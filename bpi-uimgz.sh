#!/bin/bash
U=u-boot-2019.07

BOARD=$1
if [ -z $BOARD ] ; then
  echo "usage: $0 BOARD"
  exit 1
fi

echo ${BOARD}
#
UBOOTMTKBIN=out/${BOARD}/u-boot-mtk.bin
if [ -f ${UBOOTMTKBIN} ] ; then
  UBOOTBIN=out/${BOARD}/u-boot.bin
  UBOOTIMG=out/${U}-${BOARD}-2k.img
  #SPLBIN=preloader_iotg7623Np1_emmc_pad.bin
  #SPLBIN=BPI-R2-preloader-DDR1600-20190722-2k_pad_318KB.img
  SPLBIN=BPI-R2-preloader-DDR1600-20191024-2k_pad_318KB.img
  cat ${SPLBIN} ${UBOOTBIN} > ${UBOOTIMG}
  rm -f ${UBOOTIMG}.gz
  gzip $UBOOTIMG
  exit 0
fi
#
UBOOTBIN=out/${BOARD}/u-boot-sunxi-with-spl.bin
UBOOTIMG=out/${U}-${BOARD}-8k.img
if [ -f ${UBOOTBIN} ] ; then
  cp -a ${UBOOTBIN} ${UBOOTIMG}
else
  UBOOTBIN=out/${BOARD}/u-boot.itb
  SPLBIN=out/${BOARD}/spl/sunxi-spl.bin
  cat ${SPLBIN} ${UBOOTBIN} > ${UBOOTIMG}
fi
rm -f ${UBOOTIMG}.gz
gzip $UBOOTIMG
