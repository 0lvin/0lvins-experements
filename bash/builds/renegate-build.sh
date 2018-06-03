sudo apt-get install -y git repo gnupg flex bison gperf build-essential zip tar curl libc6-dev gcc-arm-linux-gnueabihf gcc-aarch64-linux-gnu device-tree-compiler lzop libncurses5-dev libssl1.0.0 libssl-dev mtools swig libpython-dev dosfstools binfmt-support qemu-user-static  qemu qemu-user-static binfmt-support debootstrap

# create project dir
if [ ! -d roc-rk3328-cc ]; then
    mkdir roc-rk3328-cc
fi;

cd roc-rk3328-cc

PWD=`pwd`
echo "I am in ${PWD}"

# U-Boot
if [ ! -d "${PWD}/u-boot" ]; then
    git clone -b release https://github.com/FireflyTeam/u-boot
fi;
cd "${PWD}/u-boot" && git pull
cd "${PWD}"

# Kernel
if [ ! -d "${PWD}/kernel" ]; then
    git clone -b release-4.4 https://github.com/FireflyTeam/kernel
fi;
cd "${PWD}/kernel" && git pull
cd "${PWD}"

# Build
if [ ! -d "${PWD}/build" ]; then
    git clone -b debian https://github.com/FireflyTeam/build
fi;
cd "${PWD}/build" && git pull
cd "${PWD}"

# Rkbin
if [ ! -d "${PWD}/rkbin" ]; then
    git clone -b master https://github.com/FireflyTeam/rkbin
fi;
cd "${PWD}/rkbin" && git pull
cd "${PWD}"

# Root fs
if [ ! -d "${PWD}/rk-rootfs-build" ]; then
    git clone https://github.com/FireflyTeam/rk-rootfs-build.git
fi;
cd "${PWD}/rk-rootfs-build" && git pull
cd "${PWD}"

# build
./build/board_configs.sh roc-rk3328-cc
./build/mk-uboot.sh roc-rk3328-cc
./build/mk-kernel.sh roc-rk3328-cc

sudo dpkg -i ${PWD}/ubuntu-build-service/packages/*
sudo apt-get install -f -y

cd "${PWD}/rk-rootfs-build"
VERSION=stretch TARGET=desktop ARCH=armhf ./mk-base-debian.sh
cd "${PWD}"

#### http://wiki.t-firefly.com/ROC-RK3328-CC/linux_build_rootfilesystem.html
