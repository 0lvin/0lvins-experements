# https://manpages.debian.org/unstable/depthcharge-tools/mkdepthcharge.1.en.html
mkimage -D "-I dts -O dtb -p 2048" -f kernel.its /tmp/vmlinux.uimg

dd if=/dev/zero of=/tmp/bootloader.bin bs=512 count=1

echo "console=ttyS2,115200n8 earlyprintk=ttyS2,115200n8 console=tty1 init=/sbin/init root=PARTUUID=%U/PARTNROFF=1 rootwait rw noinitrd loglevel=4 delayacct" > /tmp/cmdline

vbutil_kernel --pack /tmp/vmlinux.kpart --version 1 --vmlinuz /tmp/vmlinux.uimg --arch aarch64 --keyblock /usr/share/vboot/devkeys/kernel.keyblock --signprivate /usr/share/vboot/devkeys/kernel_data_key.vbprivk --config /tmp/cmdline --bootloader /tmp/bootloader.bin

# sudo dd if=/dev/mmcblk0p1 of=`uname -r`.img
sudo dd bs=4096 if=/dev/disk/by-partuuid/2d0afc5d-f4b8-df43-b45a-18b34d662753 of=`uname -r`.img
sudo cp /proc/config.gz `uname -r`.config.gz
lsmod | sort > `uname -r`.lsmod.txt

# sudo dd if=/dev/zero of=/dev/mmcblk0p1
sudo dd count=4096 bs=4096 if=/dev/zero of=/dev/disk/by-partuuid/2d0afc5d-f4b8-df43-b45a-18b34d662753

# sudo dd if=/tmp/vmlinux.kpart of=/dev/mmcblk0p1
sudo dd bs=4096 if=/tmp/vmlinux.kpart of=/dev/disk/by-partuuid/2d0afc5d-f4b8-df43-b45a-18b34d662753

sleep 30 && sync
