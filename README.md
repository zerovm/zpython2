# Port of Python 2.7.3 to ZeroVM

List of supported modules/features 
is [here](https://github.com/zerovm/zpython2/blob/master/status.md)

## Nightly-built distribution

Nightly builds of zpython2 (tarball) are available
[here](http://ci.zerovm.org/latest-packages/zpython2.7.3.tar).

## Building python for ZeroVM

### Prerequisites

You will need a working toolchain from 
[zerovm-toolchain](https://github.com/zerovm/toolchain)  

Build and install from [zerovm-ports](https://github.com/zerovm/zerovm-ports)

    zlib
    bzip2
    
Build and install `libffi` from [libffi](https://github.com/zerovm/libffi)

### Build host pgen

Host python binary and host pgen binary will be used for cross-compilation

Run helper script `./1build_pgen.sh`

or run:

    cd zpython2
    ./configure
    make host
    
    export ZPYTHON_ROOT=`pwd`

 
Now you can either build a debug version or a release one

### Building debug version

Run helper script `./2build_zpython.sh`

or run:

    cd zpython2
    
    ./configure \
    --host=x86_64-nacl \
    --prefix=/ \
    --without-threads \
    --enable-shared=no \
    --disable-shared \
    --with-system-ffi \
    --with-pydebug \
    PYTHONPATH=${ZPYTHON_ROOT}/Lib:${ZPYTHON_ROOT} \
    PYTHONHOME=${ZPYTHON_ROOT}/install \
    LDFLAGS="-m64 -Wl,--no-export-dynamic -static-libgcc"


    HOSTPYTHON=/usr/bin/python \
    HOSTPGEN=${ZPYTHON_ROOT}/Parser/hostpgen \
    PYTHONPATH=${ZPYTHON_ROOT}/Lib:${ZPYTHON_ROOT} \
    PYTHONHOME=${ZPYTHON_ROOT}/install \
    make python install \
    DESTDIR=${ZPYTHON_ROOT}/install

Make sure that `HOSTPYTHON` variable above 
points to your host `python` binary

### Building release version

Run helper script `./2build_zpython.sh release`

or run:

    cd zpython2
    
    ./configure \
    --host=x86_64-nacl \
    --prefix=/ \
    --without-threads \
    --enable-shared=no \
    --disable-shared \
    --with-system-ffi \
    PYTHONPATH=${ZPYTHON_ROOT}/Lib:${ZPYTHON_ROOT} \
    PYTHONHOME=${ZPYTHON_ROOT}/install \
    LDFLAGS="-m64 -Wl,--no-export-dynamic -static-libgcc"


    HOSTPYTHON=/usr/bin/python \
    HOSTPGEN=${ZPYTHON_ROOT}/Parser/hostpgen \
    PYTHONPATH=${ZPYTHON_ROOT}/Lib:${ZPYTHON_ROOT} \
    PYTHONHOME=${ZPYTHON_ROOT}/install \
    make python install \
    DESTDIR=${ZPYTHON_ROOT}/install
    
    x86_64-nacl-strip python

Make sure that `HOSTPYTHON` variable above 
points to your host `python` binary

### Creating python.tar image

The helper script `./2build_zpython.sh` will create a `python.tar` 
file in `zpython2` dir.

or run:

    tar cf python.tar -C ${ZPYTHON_ROOT} python \
    -C ${ZPYTHON_ROOT}/install include/ lib/ \
    --exclude=libpython2.7.a
    
We can create tar image from either debug or release version, 
debug version will print some stuff to `stderr` on each run.
