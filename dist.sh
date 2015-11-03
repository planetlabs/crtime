#!/bin/bash
set -e

VERSION=`git describe --tags`
ARCH=`uname -m`
[ "${ARCH}" = "x86_64" ] && ARCH=amd64
[ "${ARCH}" = "i686" ] && ARCH=i386
NAME=crtime_${VERSION}_${ARCH}
rm -rf ${NAME}
PREFIX=${PWD}/${NAME}/usr/local/
mkdir -p ${PREFIX}
./configure --prefix ${PREFIX}
make
make install
DEBDIR=${PWD}/${NAME}/DEBIAN
mkdir ${DEBDIR}
sed 's/{VERSION}/'$VERSION'/g' < DEBIAN/control > ${DEBDIR}/control
sed 's/{ARCH}/'$ARCH'/g' -i ${DEBDIR}/control
dpkg-deb --build ${NAME}
