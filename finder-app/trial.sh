#!/bin/bash

CROSS_COMPILE=aarch64-none-linux-gnu-
OUTDIR=~/aesd-autograder


SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
echo $SYSROOT


DEPENDENCIES=$(find $(aarch64-none-linux-gnu-gcc -print-sysroot) -name 'libc-2.33.so')
echo $DEPENDENCIES

#echo -e $VAR1
#echo -e $VAR2
#mkdir -p "${OUTDIR}/rootfs/lib"
#mkdir -p "${OUTDIR}/rootfs/lib64"
#cp -a $SYSROOT/lib/ld-linux-aarch64.so.1 "${OUTDIR}/rootfs/lib"
#cp -a $SYSROOT/lib64/libm.so.6 "${OUTDIR}/rootfs/lib64"
#cp -a $SYSROOT/lib64/libresolv.so.2 "${OUTDIR}/rootfs/lib64"
#cp -a $SYSROOT/lib64/libc.so.6 "${OUTDIR}/rootfs/lib64"

