// TODO:
// subcalll
// terminal codes
// redirect io
// usb tty emul
// arm-linux-gnueabihf-gcc fb.c -o init --static
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <string.h>

#include "utils.h"
#include "font8x8.h"

// Create fs
static void create_fs() {
	int res = 0;

	mkdir_if_not_exists("/dev", S_IRUSR | S_IWUSR | S_IXUSR |
				    S_IRGRP | S_IXGRP |
				    S_IROTH | S_IXOTH);

	mkdir_if_not_exists("/root", S_IRUSR | S_IWUSR | S_IXUSR);

	mkdir_if_not_exists("/sys", S_IRUSR | S_IXUSR |
				    S_IRGRP | S_IXGRP |
				    S_IROTH | S_IXOTH);

	mkdir_if_not_exists("/proc", S_IRUSR | S_IXUSR |
				     S_IRGRP | S_IXGRP |
				     S_IROTH | S_IXOTH);

	mkdir_if_not_exists("/tmp", S_IRUSR | S_IWUSR | S_IXUSR |
				    S_IRGRP | S_IWGRP | S_IXGRP |
				    S_IROTH | S_IWOTH | S_IXOTH);

	mkdir_if_not_exists("/run", S_IRUSR | S_IWUSR | S_IXUSR |
				    S_IRGRP | S_IWGRP | S_IXGRP |
				    S_IROTH | S_IWOTH | S_IXOTH);

	// create and mount all dirs
	res = mount("tmpfs", "tmp", "tmpfs", 0, "size=65536k");
	if (res < 0) {
		perror("mount tmp");
	}

	res = mount("sysfs", "/sys", "sysfs", MS_NODEV | MS_NOEXEC | MS_NOSUID, 0);
	if (res < 0) {
		perror("mount sys");
	}

	res = mount("proc", "/proc", "proc", MS_NODEV | MS_NOEXEC | MS_NOSUID, 0);
	if (res < 0) {
		perror("mount proc");
	}

	res = mount("tmpfs", "/run", "tmpfs", MS_NOSUID, "size=20%,mode=0755");
	if (res < 0) {
		perror("mount /run");
	}

	res = mount("udev", "/dev", "devtmpfs", 0, "size=65536k,mode=0755");
	if (res < 0) {
		perror("mount devtmpfs");
		res = mount("udev", "/dev", "tmpfs", 0, "size=65536k,mode=0755");
		if (res < 0) {
			perror("mount tmpfs");
		}
	}

	// dev pts
	mkdir_if_not_exists("/dev/pts", S_IRUSR | S_IWUSR | S_IXUSR |
					S_IRGRP | S_IXGRP |
					S_IROTH | S_IXOTH);

	res = mount("devpts", "/dev/pts", "devpts", MS_NOEXEC | MS_NOSUID, "gid=5,mode=0620");
	if (res < 0) {
		perror("mount /dev/pts");
	}

	mknod("/dev/console", S_IRUSR | S_IWUSR | S_IFCHR, makedev(5, 1));
	mknod("/dev/null", S_IRUSR | S_IWUSR |
			   S_IRGRP | S_IWGRP |
			   S_IROTH | S_IWOTH | S_IFCHR, makedev(1, 3));
}

// Frameuffer logic
static unsigned int lx=0, ly=0;

struct FB {
	unsigned char *bits;
	unsigned size;
	int fd;
	struct fb_fix_screeninfo fi;
	struct fb_var_screeninfo vi;
};

#define fb_width(fb) ((fb)->vi.xres)
#define fb_height(fb) ((fb)->vi.yres)
#define fb_size(fb) ((fb)->fi.line_length * (fb)->vi.yres)

static int fb_open(struct FB *fb)
{
	fb->fd = open("/dev/fb0", O_RDWR);
	if (fb->fd < 0) {
		perror("no graphics at /dev/fb0");
		fb->fd = open("/dev/graphics/fb0", O_RDWR);
		if (fb->fd < 0) {
			perror("no graphics at /dev/graphics/fb0");
			return -1;
		}
	}

	if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb->fi) < 0) {
		perror("no FBIOGET_FSCREENINFO");
		goto fail;
	}
	if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vi) < 0) {
		perror("no FBIOGET_FSCREENINFO");
		goto fail;
	}

	fb->bits = mmap(0, fb_size(fb), PROT_READ | PROT_WRITE,
			MAP_SHARED, fb->fd, 0);
	if (fb->bits == MAP_FAILED) {
		perror("no MMAP");
		goto fail;
	}

	return 0;

fail:
	close(fb->fd);
	return -1;
}

static void fb_close(struct FB *fb)
{
	munmap(fb->bits, fb_size(fb));
	close(fb->fd);
}

/* there's got to be a more portable way to do this ... */
static void fb_update(struct FB *fb)
{
	fb->vi.yoffset = 1;
	ioctl(fb->fd, FBIOPUT_VSCREENINFO, &fb->vi);
	fb->vi.yoffset = 0;
	ioctl(fb->fd, FBIOPUT_VSCREENINFO, &fb->vi);
}

void vt_create_nodes()
{
	int fd;

	printf("Create nodes\n");

	fd = open("/dev/tty0", O_RDWR | O_SYNC);
	if (fd < 0) {
		mknod("/dev/tty0", S_IRUSR | S_IWUSR |
				   S_IRGRP | S_IWGRP | S_IFCHR, makedev(4, 0));
	} else {
		close(fd);
	}

	fd = open("/dev/graphics/fb0", O_RDWR);
	if (fd < 0) {
		mkdir_if_not_exists("/dev/graphics/", S_IRUSR | S_IWUSR | S_IXUSR |
						      S_IRGRP | S_IXGRP |
						      S_IROTH | S_IXOTH);
		mknod("/dev/graphics/fb0", S_IRUSR | S_IWUSR |
					   S_IRGRP | S_IWGRP | S_IFCHR, makedev(29, 0));
	} else {
		close(fd);
	}
    }

static int vt_set_mode(int graphics)
{
	int fd, r;
	fd = open("/dev/tty0", O_RDWR | O_SYNC);
	if (fd < 0) {
		perror("no tty");
		return -1;
	}
	r = ioctl(fd, KDSETMODE, graphics ? KD_GRAPHICS : KD_TEXT);
	close(fd);
	return r;
}

void set_pixel(struct FB *fb, unsigned short r, unsigned short g, unsigned short b, short x, short y) {
	long long pixel_color = 0;
	long red = (r & 0xFFFF) >> (16 - fb->vi.red.length);
	long green = (g & 0xFFFF) >> (16 - fb->vi.green.length);
	long blue = (b & 0xFFFF) >> (16 - fb->vi.blue.length);
	long count_bytes = fb->vi.bits_per_pixel / 8;
	unsigned char * write_pos = fb->bits + x * count_bytes + y * fb->fi.line_length;
	pixel_color =
		(red << fb->vi.red.offset) |
		(green << fb->vi.green.offset) |
		(blue << fb->vi.blue.offset);
	memcpy(write_pos, &pixel_color, count_bytes);
}

void write_text(const char *fn)
{
	struct FB fb;
	unsigned int i, x, y;
	unsigned short value, mask;

	printf("=>%s", fn);

	if (vt_set_mode(1)) {
		perror("no mode");
	}

	if (fb_open(&fb))
		goto fail_unmap_data;

	for(i = 0; i < strlen(fn); i ++) {
		if (fn[i] == '\n') {
			for(x=lx; x < (fb.vi.xres); x ++) {
				for(y=0; y < 8; y ++) {
					set_pixel(&fb, 0, 0, 0, x, ly + y);
				}
			}
			ly += 8;
			lx = 0;
			if (ly < (fb.vi.yres - 8)) {
				for(x=0; x < fb.vi.xres; x ++) {
					for(y=0; y < 8; y ++) {
						set_pixel(&fb, 0, 0xffff, 0xffff, x, ly + y);
					}
				}
			}
			continue;
		}
		if (lx >= (fb.vi.xres - 8)) {
			lx = 0;
			ly += 8;
		}
		if (ly >= (fb.vi.yres - 8)) {
			ly = 0;
		}
		for (x = 0; x < 8; x++) {
			for (y = 0; y < 8; y++) {
				mask = font8x8[fn[i] * 8 + y];
				/* font pixels: 0 - right, 7 - left */
				value = (mask & (1 << (7 - x))) == 0 ? 0 : 0xffff;
				set_pixel(&fb, value, value, value, lx + x, ly + y);
			}
		}
		lx += 8;
	}
	fb_update(&fb);
	fb_close(&fb);
	return;

fail_unmap_data:
	vt_set_mode(0);
}

int main() {
	int i;
	create_fs();
	vt_create_nodes();
	write_text("Hello\n world!\n");
	// check scroll
	for (i=0; i<3; i++) {
		char string_buf[1024];
		snprintf(string_buf, 1023, "Step %d\n", i);
		write_text(string_buf);
	}
	// wait for result
	sleep(600);
	vt_set_mode(0);
	return 0;
}
