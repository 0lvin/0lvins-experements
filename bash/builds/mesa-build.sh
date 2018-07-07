#!/bin/bash

if [ ! -d mesa ]; then
    git clone git://anongit.freedesktop.org/mesa/mesa
fi;

cd mesa
git pull
#autoreconf -vfi
#make distclean

CXX='ccache g++' CC='ccache gcc' ./configure --enable-texture-float --with-llvm-prefix=/usr/lib/llvm-6.0/ --enable-gallium-osmesa --with-sha1=libgcrypt --enable-llvm --with-gallium-drivers=swrast --with-dri-drivers=swrast --enable-lmsensors
CXX='ccache g++' CC='ccache gcc' make

cd ..

rm -rf lib
mkdir lib
cp -rv mesa/lib/* lib/
cp -rv mesa/lib/gallium/* lib/

LIBGL_DRIVERS_PATH=`pwd`/lib glxinfo
echo `pwd`/lib

#if [ ! -d piglit ]; then
#    git clone git://anongit.freedesktop.org/piglit
#fi;
#cd piglit
#git pull
#CXX='ccache g++' CC='ccache gcc' cmake .
#make

#LIBGL_ALWAYS_SOFTWARE=true GALLIUM_DRIVER=softpipe LIBGL_DRIVERS_PATH=`pwd`/lib PIGLIT_SOURCE_DIR=`pwd`/piglit piglit/bin/bptc-modes
#LIBGL_ALWAYS_SOFTWARE=true GALLIUM_DRIVER=softpipe LIBGL_DRIVERS_PATH=`pwd`/lib PIGLIT_SOURCE_DIR=`pwd`/piglit piglit/bin/bptc-float-modes
LIBGL_ALWAYS_SOFTWARE=true GALLIUM_DRIVER=softpipe LIBGL_DRIVERS_PATH=`pwd`/lib PIGLIT_SOURCE_DIR=`pwd`/piglit piglit/bin/khr_compressed_astc-miptree_gl
