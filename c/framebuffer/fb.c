// arm-linux-gnueabihf-gcc fb.c -o init --static
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/fb.h>
#include <stdio.h>
#include <linux/kd.h>
#include <sys/mount.h>
#include <string.h>
#include "font8x8.h"

// Create fs
static void mkdir_if_not_exists(const char *target, mode_t mode) {
	int res = 0;
	struct stat sb;

	if (stat(target, &sb) != 0)
	{
		res =  mkdir(target, mode);
		if (res < 0) {
			printf("can't create '%s' dir for mount\n", target);
		} else {
			printf("New dir for %s mount\n", target);
		}
	} else if (!S_ISDIR(sb.st_mode)) {
		printf("Exist but not directory %s.\n", target);
	}
}

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

	// create and mount all dirs
	res =  mount("sysfs", "/sys", "sysfs", 0, "nodev,noexec,nosuid");
	if (res < 0) {
		printf("mount sys\n");
	}

	res =  mount("proc", "/proc", "proc", 0, "nodev,noexec,nosuid");
	if (res < 0) {
		printf("mount proc\n");
	}
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
		printf("no graphics\n");
		return -1;
	}

	if (ioctl(fb->fd, FBIOGET_FSCREENINFO, &fb->fi) < 0) {
		printf("no FBIOGET_FSCREENINFO\n");
		goto fail;
	}
	if (ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vi) < 0) {
		printf("no FBIOGET_FSCREENINFO\n");
		goto fail;
	}

	fb->bits = mmap(0, fb_size(fb), PROT_READ | PROT_WRITE,
			MAP_SHARED, fb->fd, 0);
	if (fb->bits == MAP_FAILED) {
		printf("no MMAP\n");
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
		mknod("/dev/tty0", 8624, makedev(4, 0));
	} else {
		close(fd);
	}

	fd = open("/dev/graphics/fb0", O_RDWR);
	if (fd < 0) {
		mkdir("/dev/graphics/", 0755);
		mknod("/dev/graphics/fb0", 8624, makedev(29, 0));
	} else {
		close(fd);
	}
    }

static int vt_set_mode(int graphics)
{
	int fd, r;
	fd = open("/dev/tty0", O_RDWR | O_SYNC);
	if (fd < 0) {
		printf("no tty\n");
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

	printf("screen: %s", fn);

	if (vt_set_mode(1)) {
		printf("no mode\n");
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
	write_text("Hello\n world!");
	// check scroll
	for (i=0; i<10; i++) {
		char string_buf[128];
		snprintf(string_buf, 127, "Step %d\n", i);
		write_text(string_buf);
	}
	// wait for result
	sleep(600);
	vt_set_mode(0);
	return 0;
}
