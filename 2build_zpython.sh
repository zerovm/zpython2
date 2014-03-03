#!/bin/bash

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`

export ZPYTHON_ROOT=${SCRIPT_PATH}
export PYTHONPATH=${ZPYTHON_ROOT}/Lib:${ZPYTHON_ROOT}
export PYTHONHOME=${ZPYTHON_ROOT}/install
export LDFLAGS="-m64 -Wl,--no-export-dynamic -static-libgcc"

./configure \
--host=x86_64-nacl \
--prefix=${ZPYTHON_ROOT}/install \
--without-threads \
--enable-shared=no \
--disable-shared \
--with-pydebug

export HOSTPYTHON=`which python`
export HOSTPGEN=./Parser/hostpgen

make python install

echo "Creating python.tar.."
tar cf python.tar -C install/ include/ lib/ --exclude=libpython2.7.a
echo "Done!"


