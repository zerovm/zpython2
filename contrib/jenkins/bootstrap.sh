#!/usr/bin/env bash


export ZVM_PREFIX=$HOME/zvm-root
export ZRT_ROOT=$HOME/zrt
export LD_LIBRARY_PATH=/usr/lib64
export CPATH="/usr/x86_64-nacl/include:$HOME/libffi/x86_64-pc-nacl/include"
export PATH="$PATH:$ZVM_PREFIX/bin"
export ZPYTHON_ROOT=$HOME/zpython2

# Copy the current clone of zpython2 from the shared dir to another working
# directory
rsync -az --exclude=contrib/jenkins/.* /host-workspace/ $ZPYTHON_ROOT

#################
# Build zpython2:
ZEROVM_PORTS=$HOME/zerovm-ports
git clone https://github.com/zerovm/zerovm-ports $ZEROVM_PORTS
# First, we need a couple of libraries in order to build zpython
# (zlib, bzip2, and libffi)

# build/install zlib
cd $ZEROVM_PORTS/zlib
wget http://zlib.net/zlib-1.2.8.tar.gz
tar xf zlib-1.2.8.tar.gz
cd zlib-1.2.8
CC=x86_64-nacl-gcc ./configure --prefix=${ZVM_PREFIX}/x86_64-nacl
make
make install

# build/install bzip2:
cd $ZEROVM_PORTS/bzip2
wget http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz
tar xf bzip2-1.0.6.tar.gz
cd bzip2-1.0.6
patch -p0 < ../Makefile.patch
make
make install

# build/install libffi:
git clone https://github.com/zerovm/libffi $HOME/libffi
cd $HOME/libffi
./configure --host=x86_64-nacl --prefix=$ZVM_PREFIX/x86_64-nacl
make
make install

# finally, build zpython:
cd $ZPYTHON_ROOT
./configure
make host
./2build_zpython.sh release
echo "Built '$ZPYTHON_ROOT/python.tar'"
sudo cp python.tar /host-workspace/zpython2.7.3.tar
# TODO(larsbutler): Run `3test.py`
