sudo apt-get install -y git repo gnupg flex bison gperf build-essential zip tar curl libc6-dev gcc-arm-linux-gnueabihf gcc-aarch64-linux-gnu device-tree-compiler lzop libncurses5-dev libssl1.0.0 libssl-dev mtools swig libpython-dev dosfstools binfmt-support qemu-user-static  qemu qemu-user-static binfmt-support debootstrap

# create project dir
if [ ! -d roc-rk3328-cc ]; then
    mkdir roc-rk3328-cc
fi;

cd roc-rk3328-cc

PWD=`pwd`

# U-Boot
if [ ! -d u-boot ]; then
    git clone -b release https://github.com/FireflyTeam/u-boot
fi;
cd u-boot && git pull && cd $PWD

# Kernel
if [ ! -d kernel ]; then
    git clone -b release-4.4 https://github.com/FireflyTeam/kernel
fi;
cd kernel && git pull && cd $PWD

# Build
if [ ! -d build ]; then
    git clone -b debian https://github.com/FireflyTeam/build
fi;
cd build && git pull && cd $PWD

# Rkbin
if [ ! -d rkbin ]; then
    git clone -b master https://github.com/FireflyTeam/rkbin
fi;
cd rkbin && git pull && cd $PWD

# Root fs
if [ ! -d rk-rootfs-build ]; then
    git clone https://github.com/FireflyTeam/rk-rootfs-build.git
fi;
cd rk-rootfs-build && git pull && cd $PWD

# build
./build/board_configs.sh roc-rk3328-cc
./build/mk-uboot.sh roc-rk3328-cc
./build/mk-kernel.sh roc-rk3328-cc

cd rk-rootfs-build
sudo dpkg -i ubuntu-build-service/packages/*
sudo apt-get install -f -y

VERSION=stretch TARGET=desktop ARCH=armhf ./mk-base-debian.sh

#### http://wiki.t-firefly.com/ROC-RK3328-CC/linux_build_rootfilesystem.html
