mkimage -D "-I dts -O dtb -p 2048" -f kernel.its vmlinux.uimg

dd if=/dev/zero of=bootloader.bin bs=512 count=1

echo "console=ttyS2,115200n8 earlyprintk=ttyS2,115200n8 console=tty1 init=/sbin/init root=PARTUUID=%U/PARTNROFF=1 rootwait rw noinitrd loglevel=4" > cmdline

vbutil_kernel --pack vmlinux.kpart --version 1 --vmlinuz vmlinux.uimg --arch aarch64 --keyblock /usr/share/vboot/devkeys/kernel.keyblock --signprivate /usr/share/vboot/devkeys/kernel_data_key.vbprivk --config cmdline --bootloader bootloader.bin 

sudo dd if=/dev/zero of=/dev/mmcblk1p1

sudo dd if=vmlinux.kpart of=/dev/mmcblk1p1
