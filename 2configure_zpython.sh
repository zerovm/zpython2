#!/bin/bash

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`

export ZPYTHON_ROOT=${SCRIPT_PATH}
export PYTHONPATH=${ZPYTHON_ROOT}/Lib:${ZPYTHON_ROOT}
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
