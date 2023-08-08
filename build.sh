#!/bin/bash
LANG=C
CROSS_COMPILE_AARCH64=aarch64-linux-gnu-
CROSS_COMPILE_ARM=arm-linux-gnueabihf-

PLAT=rk3568
TARGET=bl31
BINARY_PATH=bl31/bl31.elf
ARCH=aarch64

#ATF_DIR="$(extract "$ATF_SOURCE_FILENAME")"
BINARY_FORMAT="${BINARY_PATH#*.}"

case "$ARCH" in
    aarch64) CROSS_COMPILE="$CROSS_COMPILE_AARCH64";;
    arm) CROSS_COMPILE="$CROSS_COMPILE_ARM";;
    *) die Invalid arch "$ARCH";;
esac

#-C "$ATF_DIR" \

#  make -C . \
#    CROSS_COMPILE="$CROSS_COMPILE" \
#    M0_CROSS_COMPILE="$CROSS_COMPILE_ARM" \
#    CROSS_CM3="$CROSS_COMPILE_ARM" \
#    PLAT="$PLAT" \
#    SOURCE_DATE_EPOCH="$ATF_BUILD_EPOCH" \
#    BUILD_STRING="v$ATF_SOURCE_VERSION" \
#    LC_ALL=C \
#    $MAKE_FLAGS \
#    "$TARGET"

function build()
{
	set -x
	make CROSS_COMPILE="$CROSS_COMPILE" PLAT="$PLAT" "$TARGET"
	set +x
}

case $1 in
	"clean")
		make CROSS_COMPILE="$CROSS_COMPILE" clean
	;;
	"build")
		build;
	;;
	"")
		build;
	;;
	*)
		echo "unsupported action"
	;;
esac
#  cp ${ATF_DIR}/build/${PLAT}/release/${BINARY_PATH} output_dir/${PLAT}_${TARGET}.${BINARY_FORMAT}
