#!/bin/bash

DEBUG=" --with-pydebug"
SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`

export ZPYTHON_ROOT=${SCRIPT_PATH}
export PYTHONPATH=${ZPYTHON_ROOT}/Lib:${ZPYTHON_ROOT}
export PYTHONHOME=${ZPYTHON_ROOT}/install
export LDFLAGS="-m64 -Wl,--no-export-dynamic -static-libgcc"

if [[ "$1" == "release" ]]; then
	DEBUG=""
	STRIP="x86_64-nacl-strip python"
fi

./configure \
--host=x86_64-nacl \
--prefix=/ \
--without-threads \
--enable-shared=no \
--disable-shared \
$DEBUG

export HOSTPYTHON=`which python`
export HOSTPGEN=./Parser/hostpgen

make python install DESTDIR=${ZPYTHON_ROOT}/install

$STRIP

echo "Creating python.tar.."
tar cf python.tar python -C install/ include/ lib/ --exclude=libpython2.7.a
echo "Done!"


