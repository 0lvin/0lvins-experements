#!/bin/bash

if [ ! -d mesa ]; then
    git clone git://anongit.freedesktop.org/mesa/mesa
fi;

cd mesa
git pull
autoreconf -vfi
make distclean

CXX='ccache g++' CC='ccache gcc' ./configure --enable-texture-float --with-llvm-prefix=/usr/lib/llvm-5.0/ --enable-gallium-osmesa --with-sha1=libgcrypt --enable-llvm --with-gallium-drivers=swrast,swr --with-dri-drivers=swrast
CXX='ccache g++' CC='ccache gcc' make

cd ..

rm -rf lib
mkdir lib
cp -rv mesa/lib/* lib/
cp -rv mesa/lib/gallium/* lib/

LIBGL_DRIVERS_PATH=`pwd`/lib glxinfo
echo `pwd`/lib

if [ ! -d piglit ]; then
    git clone git://anongit.freedesktop.org/piglit
fi;

cd piglit

git pull

CXX='ccache g++' CC='ccache gcc' cmake .

make

LIBGL_ALWAYS_SOFTWARE=true GALLIUM_DRIVER=swr LIBGL_DRIVERS_PATH=`pwd`/lib piglit/bin/bptc-modes
LIBGL_ALWAYS_SOFTWARE=true GALLIUM_DRIVER=swr LIBGL_DRIVERS_PATH=`pwd`/lib piglit/bin/bptc-float-modes
