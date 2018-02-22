#!/bin/bash
mkdir -p  arm-check
echo "*" > arm-check/.gitignore
cd arm-check

wget -cv https://people.debian.org/~aurel32/qemu/armhf/debian_wheezy_armhf_standard.qcow2
wget -cv https://people.debian.org/~aurel32/qemu/armhf/initrd.img-3.2.0-4-vexpress
wget -cv https://people.debian.org/~aurel32/qemu/armhf/vmlinuz-3.2.0-4-vexpress

wget -cv https://busybox.net/downloads/busybox-1.28.1.tar.bz2
rm -rf busybox-1.28.1
tar -xvf busybox-1.28.1.tar.bz2

rm -rf check_image.qcow2
qemu-img create -f qcow2 -o backing_file=debian_wheezy_armhf_standard.qcow2 check_image.qcow2

rm -rf initram
mkdir initram
cd initram

gunzip -c ../initrd.img-3.2.0-4-vexpress | cpio -idmv

arm-linux-gnueabihf-gcc --pedantic --static ../../fb.c -o init

find . | cpio -o -H newc | gzip > ../initrd.img-3.2.0-4-vexpress-new

cd ..

# qemu-system-arm -M vexpress-a9 -kernel vmlinuz-3.2.0-4-vexpress -initrd initrd.img-3.2.0-4-vexpress -drive if=sd,file=check_image.qcow2 -append "root=/dev/mmcblk0p2 console=ttyAMA0" -net nic,model=lan9118,netdev=net0 -netdev user,id=net0,hostfwd=tcp::2222-:22 -serial stdio
qemu-system-arm -M vexpress-a9 -kernel vmlinuz-3.2.0-4-vexpress -initrd initrd.img-3.2.0-4-vexpress-new -drive if=sd,file=check_image.qcow2 -append "root=/dev/mmcblk0p2" -net nic,model=lan9118,netdev=net0 -netdev user,id=net0,hostfwd=tcp::2222-:22
