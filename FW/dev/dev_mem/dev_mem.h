/* SPDX-License-Identifier: MIT */
/*
 * memory.h - Memory device interface for STM32H743
 * 
 * Copyright (c) 2025 Michael Kaa
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include "dev_interface.h"

/* Memory region descriptor */
typedef struct {
    uintptr_t  start;   /* Start address of memory region */
    size_t len;     /* Length of memory region in bytes */
    char name[10];
} mem_region_t;

/* Memory device descriptor */
typedef struct {
    uint32_t num_of_regions;    /* Number of memory regions */
    mem_region_t* regions;      /* Pointer to array of memory regions */
} dev_memory_t;

/* Memory-specific ioctrl commands */
#define MEMORY_GET_INFO       (INTERFACE_CMD_DEVICE + 0)  /* Get memory device info */
#define MEMORY_SET_ADDRESS    (INTERFACE_CMD_DEVICE + 1)  /* Set current memory address */
#define MEMORY_GET_ADDRESS    (INTERFACE_CMD_DEVICE + 2)  /* Get current memory address */

/* Global memory device instance accessor */
const interface_t* dev_memory_get(void);

static inline void dev_memory_print_info(void) {
    const interface_t* mem_dev = dev_memory_get();
    if (mem_dev == NULL) {
        printf("Memory device not available\r\n");
        return;
    }
    
    dev_memory_t* mem_info = NULL;
    int result = mem_dev->ioctrl(MEMORY_GET_INFO, &mem_info);
    if (result != 0 || mem_info == NULL) {
        printf("Failed to get memory info: %d\r\n", result);
        return;
    }
    
    printf("Memory: num of regions: %lu\r\n", mem_info->num_of_regions);
    for(size_t i = 0; i < mem_info->num_of_regions; i++) {
        printf("%u %s \tstart 0x%08" PRIxPTR "  size 0x%08x\r\n", 
               i, 
               mem_info->regions[i].name, 
               mem_info->regions[i].start, 
               mem_info->regions[i].len);
    }
}

#endif /* MEMORY_H */
