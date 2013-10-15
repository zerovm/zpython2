#!/bin/bash

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`

export PYTHONHOME=${ZPYTHON_ROOT}:${ZPYTHON_ROOT}/Lib
export HOSTPYTHON=./hostpython
export HOSTPGEN=./Parser/hostpgen
export PATH=${PATH}:${PLAT}"/bin"
export CROSS_COMPILE="x86_64-nacl-"
export CROSS_COMPILE_TARGET=yes
export HOSTARCH=amd64-linux
export BUILDARCH=x86_64-linux-gnu

#make 

#copy python files into _install directory, all installed files should be accessible
#in filesystem in order to run python on zerovm+zrt

make python
# update tar file
echo "Creating python.tar.."
tar cf python.tar -C install/ include/ lib/ --exclude=libpython2.7.a
echo "Done!"


