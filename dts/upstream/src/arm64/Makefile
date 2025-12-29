# SPDX-License-Identifier: GPL-2.0+

include $(srctree)/scripts/Makefile.dts

# Add any required device tree compiler flags here
DTC_FLAGS += -a 0x8

ifdef CONFIG_RCAR_64
	DTC_FLAGS += -R 4
endif

mt7986a-bpi-r3-sd-dtbs := mediatek/mt7986a-bananapi-bpi-r3.dtb \
			  mediatek/mt7986a-bananapi-bpi-r3-sd.dtbo
mt7986a-bpi-r3-emmc-dtbs := mediatek/mt7986a-bananapi-bpi-r3.dtb \
			    mediatek/mt7986a-bananapi-bpi-r3-emmc.dtbo
dtb-$(CONFIG_ARCH_MEDIATEK) += mt7986a-bpi-r3-sd.dtb
dtb-$(CONFIG_ARCH_MEDIATEK) += mt7986a-bpi-r3-emmc.dtb
