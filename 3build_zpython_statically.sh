#!/bin/bash

SCRIPT=$(readlink -f "$0")
SCRIPT_PATH=`dirname "$SCRIPT"`
NACL_TOOLCHAIN=${NACL_SDK_ROOT}/toolchain/linux_x86_glibc

export PATH=${PATH}:${NACL_TOOLCHAIN}/bin:

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

#CROSS_COMPILE=ppc_6xx-

#make install HOSTPYTHON prefix=~/soft/ZPython2
INSTALL_PATH=${SCRIPT_PATH}/_install
ZRT_PYTHONFILES_FOLDER=${ZRT_ROOT}/mounts/pythonfiles
ZRT_TAR_NAME_TO_INSTALL=${ZRT_PYTHONFILES_FOLDER}/python-install2.tar

echo install zpython files
make install prefix=${INSTALL_PATH} -n


# if [ -d ${INSTALL_PATH} ]
# then
# #prepare folder and remove old installed archive
#     rm -f ${ZRT_TAR_NAME_TO_INSTALL}
#     mkdir ${ZRT_PYTHONFILES_FOLDER} -p
#     cd ${INSTALL_PATH}

# #add files into tar archive and copy it to the zrt/mounts
#     tar -cf ${ZRT_TAR_NAME_TO_INSTALL} *
#     echo created ${ZRT_TAR_NAME_TO_INSTALL}
# fi

