#!/bin/sh

EXTRA_VERSION=$1
GETDP=${HOME}/src/getdp
LOG=${GETDP}/nightly64.log
WEB_BIN=geuzaine@geuz.org:/home/www/geuz.org/getdp/bin/Windows

export CC=/usr/bin/x86_64-w64-mingw32-gcc.exe
export CXX=/usr/bin/x86_64-w64-mingw32-g++.exe
export FC=/usr/bin/x86_64-w64-mingw32-gfortran.exe
export PETSC_DIR=${HOME}/src/petsc-3.1-p8
export SLEPC_DIR=${HOME}/src/slepc-3.1-p6
export PREFIXES="--with-blas-lapack-prefix=/usr/local --with-gmsh-prefix=/usr/local --with-gsl-prefix=/usr/local"

rm -f ${LOG}

echo "BUILD BEGIN: `date`" > ${LOG}

rm -f ${GETDP}/Makefile*
rm -rf ${GETDP}/getdp-*
cd ${GETDP} && svn update >> ${LOG} 2>&1
cd ${GETDP} && ./configure ${PREFIXES} --enable-sparskit >> ${LOG} 2>&1
cd ${GETDP} && make clean >> ${LOG} 2>&1
cd ${GETDP} && make distrib-win${EXTRA_VERSION} >> ${LOG} 2>&1
FILE=`ls ${GETDP}/getdp-*.zip`
if [ -f ${FILE} ]; then
  if [[ ${EXTRA_VERSION} == "-svn" ]]; then
    scp -C ${FILE} ${WEB_BIN}/getdp-svn-Win64s.zip
  else
    NAME=`basename ${FILE} .zip`
    scp -C ${FILE} ${WEB_BIN}/${NAME}64s.zip
  fi
fi

export PETSC_ARCH=win64_real_mumps_seq
rm -f ${GETDP}/Makefile*
rm -rf ${GETDP}/getdp-*
cd ${GETDP} && svn update >> ${LOG} 2>&1
cd ${GETDP} && ./configure ${PREFIXES} >> ${LOG} 2>&1
cd ${GETDP} && make clean >> ${LOG} 2>&1
cd ${GETDP} && make distrib-win${EXTRA_VERSION} >> ${LOG} 2>&1
FILE=`ls ${GETDP}/getdp-*.zip`
if [ -f ${FILE} ]; then
  if [[ ${EXTRA_VERSION} == "-svn" ]]; then
    scp -C ${FILE} ${WEB_BIN}/getdp-svn-Win64r.zip
  else
    NAME=`basename ${FILE} .zip`
    scp -C ${FILE} ${WEB_BIN}/${NAME}64r.zip
  fi
fi

export PETSC_ARCH=win64_complex_mumps_seq
rm -f ${GETDP}/Makefile*
rm -rf ${GETDP}/getdp-*
cd ${GETDP} && svn update >> ${LOG} 2>&1
cd ${GETDP} && ./configure ${PREFIXES} >> ${LOG} 2>&1
cd ${GETDP} && make clean >> ${LOG} 2>&1
cd ${GETDP} && make distrib-win${EXTRA_VERSION} >> ${LOG} 2>&1
FILE=`ls ${GETDP}/getdp-*.zip`
if [ -f ${FILE} ]; then
  if [[ ${EXTRA_VERSION} == "-svn" ]]; then
    scp -C ${FILE} ${WEB_BIN}/getdp-svn-Win64c.zip
  else
    NAME=`basename ${FILE} .zip`
    scp -C ${FILE} ${WEB_BIN}/${NAME}64c.zip
  fi
fi

echo "BUILD END: `date`" >> ${LOG}

scp -C ${LOG} ${WEB_BIN}/

### special versions for Peter
LOG=${GETDP}/nightly64_NX.log
WEB_BIN=geuzaine@geuz.org:/home/www/geuz.org/getdp/bin_nx
rm -f ${LOG}
echo "BUILD BEGIN: `date`" > ${LOG}

export PETSC_ARCH=win64_real_mumps_seq
rm -f ${GETDP}/Makefile*
rm -rf ${GETDP}/getdp-*
cd ${GETDP} && svn update >> ${LOG} 2>&1
cd ${GETDP} && ./configure ${PREFIXES} --enable-nx >> ${LOG} 2>&1
cd ${GETDP} && make clean >> ${LOG} 2>&1
cd ${GETDP} && make distrib-win${EXTRA_VERSION} >> ${LOG} 2>&1
scp -C ${GETDP}/getdp-*.zip ${WEB_BIN}/getdp${EXTRA_VERSION}-NX-Win64r.zip

export PETSC_ARCH=win64_complex_mumps_seq
rm -f ${GETDP}/Makefile*
rm -rf ${GETDP}/getdp-*
cd ${GETDP} && svn update >> ${LOG} 2>&1
cd ${GETDP} && ./configure ${PREFIXES} --enable-nx >> ${LOG} 2>&1
cd ${GETDP} && make clean >> ${LOG} 2>&1
cd ${GETDP} && make distrib-win${EXTRA_VERSION} >> ${LOG} 2>&1
scp -C ${GETDP}/getdp-*.zip ${WEB_BIN}/getdp${EXTRA_VERSION}-NX-Win64c.zip

echo "BUILD END: `date`" >> ${LOG}
scp -C ${LOG} ${WEB_BIN}/
