#!/bin/bash

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`
export ZPYTHON_ROOT=${SCRIPT_PATH}

#to make sure copy rigth Modules/Setup
cp pyconfig.h.zin pyconfig.h.in
cp Modules/Setup.zdist Modules/Setup

#configure cpython to be built statically, also overrided LINKFORSHARED variable,
#although rest variables has been set to link it statically by nacl-gcc
export PYTHONPATH="${ZPYTHON_ROOT}/Modules:${ZPYTHON_ROOT}/Lib:${ZPYTHON_ROOT}"
export LINKFORSHARED=\
"-s -static -T ${ZVM_PREFIX}/x86_64-nacl/lib64/ldscripts/elf64_nacl.x.static \
-melf64_nacl -m64 -L${ZVM_PREFIX}/x86_64-nacl/lib -lzrt -lfs -lstdc++"
export CC="x86_64-nacl-gcc"
export CXX="x86_64-nacl-g++"
export AR="x86_64-nacl-ar"
export RANLIB="x86_64-nacl-ranlib"
export LD_LIBRARY_PATH=${ZVM_PREFIX}"/x86_64-nacl/lib"
export CFLAGS=""
export LINKFORSHARED=" "
export HOSTPYTHON=./hostpython
export HOSTPGEN=./Parser/hostpgen
export LDFLAGS="-m64 -Wl,--no-export-dynamic -static-libgcc"

./configure \
--host=x86_64-nacl \
--prefix=${ZPYTHON_ROOT}/install \
--without-threads \
--enable-shared=no \
--disable-shared \
--with-pydebug
