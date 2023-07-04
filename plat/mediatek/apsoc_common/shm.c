// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 */

#include <mbedtls/memory_buffer_alloc.h>
#include <common/debug.h>

#include <shm.h>

#define DRAM_BASE		0x40000000ULL
#define DRAM_MAX_SIZE		0x200000000ULL

int set_shared_memory(uintptr_t paddr, size_t shm_size,
		      uintptr_t *shm_vaddr, uint32_t flag)
{
	int stat;

	shm_size = ALIGN_SHM_SIZE(paddr, shm_size);
	if (!shm_size || shm_size > MAX_SHM_SIZE)
		return -ALLOC_SHM_INVALID_PARAM_ERR;

	if ((paddr & TABLE_ADDR_MASK) < DRAM_BASE ||
	    (paddr & TABLE_ADDR_MASK) > DRAM_BASE + DRAM_MAX_SIZE - shm_size)
		return -ALLOC_SHM_INVALID_PARAM_ERR;

	stat = mmap_add_dynamic_region_alloc_va(paddr & TABLE_ADDR_MASK,
						shm_vaddr, shm_size, flag);
	if (stat) {
		VERBOSE("Failed to map region: %d\n", stat);
		return -ALLOC_SHARED_MEMORY_ERR;
	}

	*shm_vaddr += SHM_OFFSET(paddr);

	return 0;
}

int free_shared_memory(uintptr_t shm_vaddr, size_t shm_size)
{
	int stat;

	shm_size = ALIGN_SHM_SIZE(shm_vaddr, shm_size);
	stat = mmap_remove_dynamic_region(shm_vaddr & TABLE_ADDR_MASK, shm_size);
	if (stat) {
		VERBOSE("Failed to unmap region: %d\n", stat);
		return -FREE_SHARED_MEMORY_ERR;
	}
	return 0;
}
