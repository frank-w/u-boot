#
# Copyright (c) 2026, MediaTek Inc. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ifeq ($(DDR3_FLYBY), 1)
BL2_CPPFLAGS		+=	-DDDR3_FLYBY
endif

# Whether to display verbose DRAM log
ifeq ($(DRAM_DEBUG_LOG), 1)
BL2_CPPFLAGS		+=	-DDRAM_DEBUG_LOG
endif # END OF DRAM_DEBUG_LOG
