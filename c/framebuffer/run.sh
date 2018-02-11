#!/bin/bash
mkdir -p  arm-check
echo "*" > arm-check/.gitignore
cd arm-check

wget -cv https://people.debian.org/~aurel32/qemu/armhf/debian_wheezy_armhf_standard.qcow2
wget -cv https://people.debian.org/~aurel32/qemu/armhf/initrd.img-3.2.0-4-vexpress
wget -cv https://people.debian.org/~aurel32/qemu/armhf/vmlinuz-3.2.0-4-vexpress

rm -rf check_image.qcow2
qemu-img create -f qcow2 -o backing_file=debian_wheezy_armhf_standard.qcow2 check_image.qcow2

qemu-system-arm -M vexpress-a9 -kernel vmlinuz-3.2.0-4-vexpress -initrd initrd.img-3.2.0-4-vexpress -drive if=sd,file=check_image.qcow2 -append "root=/dev/mmcblk0p2 console=ttyAMA0" -vga none
