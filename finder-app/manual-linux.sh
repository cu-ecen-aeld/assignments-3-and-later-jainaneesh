#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.15.163
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    # Deep cleaning
    echo -e "Build step 1. Deep cleaning with mrproper"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} mrproper
    # Doing a default config using defconfig
    echo -e "Build step 2. Configuring for defconfig"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} defconfig
    # Building our vmlinux target
    echo -e "Build step 3. Building the vmlinux targer"
    make -j4 ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} all
    # Building modules
    echo -e "Build step 4. Building modules"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules
    # Building the device tree
    echo -e "Build step 5. Building the device tree"
    make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
fi

echo "Adding the Image in outdir"
# Copying the generated files to outdir
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}

echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"
if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir -p ${OUTDIR}/rootfs
cd ${OUTDIR}/rootfs
mkdir -p bin dev etc home lib lib64 proc sbin sys tmp usr var
mkdir -p usr/bin usr/lib usr/sbin
mkdir -p var/log

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox
    # Debug message
    # Cleaning any residuals
    echo -e "Cleaning up busybox"
    make distclean
    # Make with default config
    echo -e "Adding default config to busybox"
    make defconfig
else
    cd busybox
fi

# TODO: Make and install busybox
# Debug message
# Cross compiling busybox
echo -e "Cross compiling Busybox"
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
# Make and install busybox
echo -e "Make and install busybox"
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install



echo "Library dependencies"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "program interpreter"
${CROSS_COMPILE}readelf -a ${OUTDIR}/rootfs/bin/busybox | grep "Shared library"

# TODO: Add library dependencies to rootfs
SYSROOT=$(${CROSS_COMPILE}gcc -print-sysroot)
mkdir -p "${OUTDIR}/rootfs/lib"
mkdir -p "${OUTDIR}/rootfs/lib64"
cp -a $SYSROOT/lib/ld-linux-aarch64.so.1 "${OUTDIR}/rootfs/lib"
cp -a $SYSROOT/lib64/libm.so.6 "${OUTDIR}/rootfs/lib64"
cp -a $SYSROOT/lib64/libresolv.so.2 "${OUTDIR}/rootfs/lib64"
cp -a $SYSROOT/lib64/libc.so.6 "${OUTDIR}/rootfs/lib64"

# TODO: Make device nodes
cd ${OUTDIR}/rootfs
mkdir -p "${OUTDIR}/rootfs/dev"
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1

# TODO: Clean and build the writer utility
cd $FINDER_APP_DIR
make clean
make CROSS_COMPILE=${CROSS_COMPILE} all

# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
echo -e "Copying the finder related scripts and executables to rootfs home directory..."
mkdir -p "${OUTDIR}/rootfs/home"
mkdir -p "${OUTDIR}/rootfs/home/conf"
cp "${FINDER_APP_DIR}/writer" "${OUTDIR}/rootfs/home"
cp "${FINDER_APP_DIR}/finder.sh" "${OUTDIR}/rootfs/home"
cp "${FINDER_APP_DIR}/finder-test.sh" "${OUTDIR}/rootfs/home"
cp "${FINDER_APP_DIR}/start-qemu-terminal.sh" "${OUTDIR}/rootfs/home"
cp "${FINDER_APP_DIR}/start-qemu-app.sh" "${OUTDIR}/rootfs/home"
cp "${FINDER_APP_DIR}/autorun-qemu.sh" "${OUTDIR}/rootfs/home"
cp "${FINDER_APP_DIR}/conf/assignment.txt" "${OUTDIR}/rootfs/home/conf"
cp "${FINDER_APP_DIR}/conf/username.txt" "${OUTDIR}/rootfs/home/conf"

# TODO: Chown the root directory

# TODO: Create initramfs.cpio.gz
