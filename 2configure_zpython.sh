#!/bin/bash

NACL_TOOLCHAIN=${NACL_SDK_ROOT}/toolchain/linux_x86_glibc

export PATH=${PATH}:${NACL_TOOLCHAIN}/bin

#to make sure copy rigth Modules/Setup
cp pyconfig.h.zin pyconfig.h.in
cp Modules/Setup.zdist Modules/Setup

#configure cpython to be built statically, also overrided LINKFORSHARED variable,
#although rest variables has been set to link it statically by nacl-gcc
export THIRDPARTYLIBS="-lbz2"
export PYTHONPATH="${ZPYTHON_ROOT}/Modules:$${ZPYTHON_ROOT}/Lib:${ZPYTHON_ROOT}"
export LINKFORSHARED=\
"-s -static -T ${NACL_TOOLCHAIN}/x86_64-nacl/lib64/ldscripts/elf64_nacl.x.static \
-melf64_nacl -m64 -L${ZRT_ROOT}/lib -L${ZRT_ROOT}/../3rdparty/lib -lzrt -lfs -lstdc++ -lzglibc ${THIRDPARTYLIBS}"
export CC="x86_64-nacl-gcc"
export CXX="x86_64-naclg++"
export AR="x86_64-nacl-ar"
export RANLIB="x86_64-nacl-ranlib"
export LD_LIBRARY_PATH=${NACL_TOOLCHAIN}"/x86_64-nacl/lib"
export CFLAGS="-I${ZRT_ROOT}/lib "
export LDFLAGS=" -s -static -static-libgcc"
export LIBC="-lzglibc"

./configure \
--host=x86_64-nacl \
--build=x86_64-linux-gnu \
--prefix=/python \
--without-threads \
--enable-shared=no \
--disable-shared
