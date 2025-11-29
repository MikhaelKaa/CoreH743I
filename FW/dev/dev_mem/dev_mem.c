/* SPDX-License-Identifier: MIT */
/*
 * memory.c - Memory device interface implementation for STM32H743
 */

#include <string.h>
#include <errno.h>
#include "dev_mem.h"

#define FLASH_ADR (0x08000000U)

/* STM32H743IIT6 memory regions */
static const mem_region_t memory_regions[] = {
    {0x20000000U, 128U  * 1024U, "DTCMRAM"},
    {0x24000000U, 512U  * 1024U, "RAM_D1"},    
    {0x30000000U, 288U  * 1024U, "RAM_D2"},    
    {0x38000000U, 64U   * 1024U, "RAM_D3"},    
    {0x00000000U, 64U   * 1024U, "ITCMRAM"},    
    {FLASH_ADR,   2048U * 1024U, "FLASH"},    
    {0xd0000000U, 8192U * 1024U, "FMCBank2"}
};

static const dev_memory_t memory_info = {
    .num_of_regions = (sizeof(memory_regions) / sizeof(memory_regions[0])),
    .regions        = (mem_region_t*)memory_regions
};

/* Current memory access address */
static size_t current_address = 0;

/* Forward declarations */
static int memory_read(void* buf, size_t len);
static int memory_write(const void* buf, size_t len);
static int memory_ioctrl(int cmd, void* arg);
static int memory_set_address(size_t address);
static int memory_get_info(dev_memory_t** info);

// Read from memory 
static int memory_read(void* buf, size_t len) {
    if (buf == NULL || len == 0) {
        return -EINVAL;
    }
    
    
    /* Check if address is in flash region */
    for (uint32_t i = 0; i < memory_info.num_of_regions; i++) {
        const mem_region_t* region = &memory_regions[i];
        if (current_address >= region->start && 
            current_address < region->start + region->len) {
            
            /* Check if we're trying to read from flash */
            if (region->start == FLASH_ADR) {
                /* For flash, we can read directly */
                const uint8_t* src = (const uint8_t*)current_address;
                memcpy(buf, src, len);
                current_address += len;
                return (int)len;
            }
            
            /* For RAM regions */
            const uint8_t* src = (const uint8_t*)current_address;
            memcpy(buf, src, len);
            current_address += len;
            return (int)len;
        }
    }
    
    return -EINVAL; /* Address out of valid range */
}

/* Write to memory (interface implementation) */
static int memory_write(const void* buf, size_t len) {
    if (buf == NULL || len == 0) {
        return -EINVAL;
    }
    
    
    /* Check if address is in flash region */
    for (uint32_t i = 0; i < memory_info.num_of_regions; i++) {
        const mem_region_t* region = &memory_regions[i];
        if (current_address >= region->start && 
            current_address < region->start + region->len) {
            
            /* Check if we're trying to write to flash */
            if (region->start == FLASH_ADR) {
                return -ENOTSUP; /* Flash write not supported yet */
            }
            
            /* For RAM regions */
            uint8_t* dst = (uint8_t*)current_address;
            memcpy(dst, buf, len);
            current_address += len;
            return (int)len;
        }
    }
    
    return -EINVAL; /* Address out of valid range */
}

/* Set current memory address */
static int memory_set_address(size_t address) {
    /* Validate address is within known memory regions */
    for (uint32_t i = 0; i < memory_info.num_of_regions; i++) {
        const mem_region_t* region = &memory_regions[i];
        if (address >= region->start && address < region->start + region->len) {
            current_address = address;
            return 0;
        }
    }
    return -EINVAL;
}

/* Get memory device info */
static int memory_get_info(dev_memory_t** info) {
    if (info == NULL) {
        return -EINVAL;
    }
    *info = (dev_memory_t*)&memory_info;
    return 0;
}

/* IO Control for memory (interface implementation) */
static int memory_ioctrl(int cmd, void* arg) {
    switch (cmd) {
        case MEMORY_SET_ADDRESS:
            if (arg == NULL) return -EINVAL;
            return memory_set_address(*(size_t*)arg);
            
        case MEMORY_GET_INFO:
            return memory_get_info((dev_memory_t**)arg);

        case MEMORY_GET_ADDRESS:
            if (arg == NULL) return -EINVAL;
            *(size_t*)arg = current_address;
            return 0;
            
        default:
            return -ENOTSUP;
    }
}

/* Memory device instance (static - hidden inside module) */
static const interface_t dev_memory = {
    .read = memory_read,
    .write = memory_write,
    .ioctrl = memory_ioctrl
};

/* Memory device instance accessor */
const interface_t* dev_memory_get(void) {
    return &dev_memory;
}
