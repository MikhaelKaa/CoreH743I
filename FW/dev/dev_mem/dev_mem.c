/* SPDX-License-Identifier: MIT */
/*
 * memory.c - Memory device interface implementation for STM32H743
 */

#include <string.h>
#include <errno.h>
#include "dev_mem.h"
#include "stm32h743xx.h"

#define FLASH_BASE_ADDR          (0x08000000U)
#ifndef FLASH_SECTOR_SIZE
#define FLASH_SECTOR_SIZE        (128 * 1024)  // 128KB sectors for STM32H743
#endif // FLASH_SECTOR_SIZE
#define FLASH_KEY1               (0x45670123U)
#define FLASH_KEY2               (0xCDEF89ABU)

// STM32H743IIT6 memory regions
static const mem_region_t memory_regions[] = {
    {0x20000000U, 128U  * 1024U, "DTCMRAM"},
    {0x24000000U, 512U  * 1024U, "RAM_D1"},    
    {0x30000000U, 288U  * 1024U, "RAM_D2"},    
    {0x38000000U, 64U   * 1024U, "RAM_D3"},    
    {0x00000000U, 64U   * 1024U, "ITCMRAM"},    
    {FLASH_BASE_ADDR, 2048U * 1024U, "FLASH"},    
    {0xd0000000U, 8192U * 1024U, "FMCBank2"}
};

static const dev_memory_t memory_info = {
    .num_of_regions = (sizeof(memory_regions) / sizeof(memory_regions[0])),
    .regions        = (mem_region_t*)memory_regions
};

// Current memory access address
static uintptr_t current_address = 0;

// Flash operation buffer (aligned for Flash writing)
static RAM_D1 uint8_t flash_buffer[FLASH_SECTOR_SIZE] __attribute__((aligned(8)));

// Forward declarations
static int memory_read(void* buf, size_t len);
static int memory_write(const void* buf, size_t len);
static int memory_ioctrl(int cmd, void* arg);
static int memory_set_address(uintptr_t address);
static int memory_get_info(dev_memory_t** info);
static int memory_flash_unlock(void);
static int memory_flash_lock(void);
static int memory_flash_erase_sector(uint32_t sector);
static int memory_flash_write_page(uint32_t address, const uint8_t* data, size_t len);
static int memory_flash_program(uint32_t address, const void* data, size_t len);

// Read from memory 
static int memory_read(void* buf, size_t len) {
    if (buf == NULL || len == 0) {
        return -EINVAL;
    }
    
    // Check if address is in valid region
    for (uint32_t i = 0; i < memory_info.num_of_regions; i++) {
        const mem_region_t* region = &memory_regions[i];
        if (current_address >= region->start && 
            current_address < region->start + region->len) {
            
            // For Flash and RAM regions - read directly
            const uint8_t* src = (const uint8_t*)current_address;
            memcpy(buf, src, len);
            current_address += len;
            return (int)len;
        }
    }
    
    return -EINVAL; // Address out of valid range
}

// Write to memory (interface implementation)
static int memory_write(const void* buf, size_t len) {
    if (buf == NULL || len == 0) {
        return -EINVAL;
    }
    
    // Check if address is in flash region
    for (uint32_t i = 0; i < memory_info.num_of_regions; i++) {
        const mem_region_t* region = &memory_regions[i];
        if (current_address >= region->start && 
            current_address < region->start + region->len) {
            
            // Check if we're trying to write to flash
            if (region->start == FLASH_BASE_ADDR) {
                return memory_flash_program(current_address, buf, len);
            }
            
            // For RAM regions
            uint8_t* dst = (uint8_t*)current_address;
            memcpy(dst, buf, len);
            current_address += len;
            return (int)len;
        }
    }
    
    return -EINVAL; // Address out of valid range
}

// Set current memory address
static int memory_set_address(uintptr_t address) {
    // Validate address is within known memory regions
    for (uint32_t i = 0; i < memory_info.num_of_regions; i++) {
        const mem_region_t* region = &memory_regions[i];
        if (address >= region->start && address < region->start + region->len) {
            current_address = address;
            return 0;
        }
    }
    return -EINVAL;
}

// Get memory device info
static int memory_get_info(dev_memory_t** info) {
    if (info == NULL) {
        return -EINVAL;
    }
    *info = (dev_memory_t*)&memory_info;
    return 0;
}

// Unlock Flash for writing
static int memory_flash_unlock(void) {
    // Check if Flash is already unlocked
    if (!(FLASH->CR1 & FLASH_CR_LOCK)) {
        return 0; // Already unlocked
    }
    
    // Perform unlock sequence
    FLASH->KEYR1 = FLASH_KEY1;
    FLASH->KEYR1 = FLASH_KEY2;
    
    // Verify unlock was successful
    if (FLASH->CR1 & FLASH_CR_LOCK) {
        return -EACCES; // Unlock failed
    }
    
    return 0;
}

// Lock Flash
static int memory_flash_lock(void) {
    FLASH->CR1 |= FLASH_CR_LOCK;
    return 0;
}

// Erase Flash sector
static int memory_flash_erase_sector(uint32_t sector) {
    // Wait for any ongoing operation
    while (FLASH->SR1 & FLASH_SR_BSY) {}
    
    // Check if sector is valid (0-7 for STM32H743)
    if (sector > 7) {
        return -EINVAL;
    }
    
    // Define error mask for Flash operations
    uint32_t error_mask = FLASH_SR_WRPERR | FLASH_SR_PGSERR | FLASH_SR_STRBERR | 
                         FLASH_SR_INCERR | FLASH_SR_OPERR | FLASH_SR_RDPERR | 
                         FLASH_SR_RDSERR;
    
    // Clear any pending flags
    FLASH->SR1 |= (FLASH_SR_EOP | error_mask);
    
    // Start erase operation
    FLASH->CR1 &= ~FLASH_CR_SNB;
    FLASH->CR1 |= (sector << FLASH_CR_SNB_Pos);
    FLASH->CR1 |= FLASH_CR_SER;
    FLASH->CR1 |= FLASH_CR_START;
    
    // Wait for completion
    while (FLASH->SR1 & FLASH_SR_BSY) {}
    
    // Check for errors
    if (FLASH->SR1 & error_mask) {
        FLASH->SR1 |= error_mask;
        return -EIO;
    }
    
    // Clear end of operation flag
    FLASH->SR1 |= FLASH_SR_EOP;
    
    return 0;
}

// Write Flash page (256-bit programming)
static int memory_flash_write_page(uint32_t address, const uint8_t* data, size_t len) {
    if (len != 32) { // STM32H7 uses 256-bit (32-byte) programming
        return -EINVAL;
    }
    
    // Check address alignment (must be 8-byte aligned for 64-bit writes)
    if ((address & 0x7) != 0) {
        return -EINVAL;
    }
    
    // Wait for any ongoing operation and check no errors
    while (FLASH->SR1 & FLASH_SR_BSY) {}
    
    // Define error mask for Flash operations
    uint32_t error_mask = FLASH_SR_WRPERR | FLASH_SR_PGSERR | FLASH_SR_STRBERR | 
                         FLASH_SR_INCERR | FLASH_SR_OPERR | FLASH_SR_RDPERR | 
                         FLASH_SR_RDSERR;
    
    if (FLASH->SR1 & error_mask) {
        // Clear error flags
        FLASH->SR1 |= error_mask;
        return -EIO;
    }
    
    // Set programming mode with correct parallelism
    FLASH->CR1 &= ~FLASH_CR_PSIZE;
    FLASH->CR1 |= (2U << FLASH_CR_PSIZE_Pos); // 32-bit programming (x64 parallelism)
    FLASH->CR1 |= FLASH_CR_PG;
    
    // Program 4 double words (256 bits) with proper volatile access
    for (uint32_t i = 0; i < 4; i++) {
        uint64_t value = *((uint64_t*)(data + i * 8));
        *(__IO uint64_t*)(address + i * 8) = value;
        
        // Wait for completion of the write operation
        while (FLASH->SR1 & FLASH_SR_BSY) {}
        
        // Check for errors after each write
        if (FLASH->SR1 & error_mask) {
            FLASH->CR1 &= ~FLASH_CR_PG;
            FLASH->SR1 |= error_mask;
            return -EIO;
        }
    }
    
    // Verify programming by reading back
    for (uint32_t i = 0; i < 4; i++) {
        uint64_t written = *(__IO uint64_t*)(address + i * 8);
        uint64_t expected = *((uint64_t*)(data + i * 8));
        
        if (written != expected) {
            FLASH->CR1 &= ~FLASH_CR_PG;
            return -EIO;
        }
    }
    
    // Clear programming flag
    FLASH->CR1 &= ~FLASH_CR_PG;
    
    return 0;
}

// High-level Flash programming with automatic sector management
static int memory_flash_program(uint32_t address, const void* data, size_t len) {
    if (address < FLASH_BASE_ADDR || data == NULL || len == 0) {
        return -EINVAL;
    }
    
    // Check address alignment for Flash operations
    if ((address & 0x7) != 0) {
        return -EINVAL;
    }
    
    // Calculate sector number
    uint32_t sector = (address - FLASH_BASE_ADDR) / FLASH_SECTOR_SIZE;
    if (sector > 7) {
        return -EINVAL;
    }
    
    // Calculate sector start address
    uint32_t sector_start = FLASH_BASE_ADDR + (sector * FLASH_SECTOR_SIZE);
    uint32_t sector_offset = address - sector_start;
    
    // Check if we need to modify existing data
    if (sector_offset + len > FLASH_SECTOR_SIZE) {
        return -EINVAL; // Cross-sector write not supported
    }
    
    // Read current sector content
    memcpy(flash_buffer, (void*)sector_start, FLASH_SECTOR_SIZE);
    
    // Check if data actually changed
    if (memcmp(flash_buffer + sector_offset, data, len) == 0) {
        current_address += len;
        return (int)len; // No change needed
    }
    
    // Modify buffer with new data
    memcpy(flash_buffer + sector_offset, data, len);
    
    // Unlock Flash
    int result = memory_flash_unlock();
    if (result != 0) {
        return result;
    }
    
    // Erase sector
    result = memory_flash_erase_sector(sector);
    if (result != 0) {
        memory_flash_lock();
        return result;
    }
    
    // Write back entire sector in 32-byte pages
    for (uint32_t i = 0; i < FLASH_SECTOR_SIZE; i += 32) {
        // Ensure address is 32-byte aligned for 256-bit programming
        uint32_t write_addr = sector_start + i;
        if ((write_addr & 0x1F) != 0) {
            memory_flash_lock();
            return -EINVAL;
        }
        
        result = memory_flash_write_page(write_addr, flash_buffer + i, 32);
        if (result != 0) {
            memory_flash_lock();
            return result;
        }
    }
    
    // Lock Flash
    memory_flash_lock();
    
    current_address += len;
    return (int)len;
}

// IO Control for memory (interface implementation)
static int memory_ioctrl(int cmd, void* arg) {
    switch (cmd) {
        case MEMORY_SET_ADDRESS:
            if (arg == NULL) return -EINVAL;
            return memory_set_address(*(uintptr_t*)arg);
            
        case MEMORY_GET_INFO:
            return memory_get_info((dev_memory_t**)arg);

        case MEMORY_GET_ADDRESS:
            if (arg == NULL) return -EINVAL;
            *(uintptr_t*)arg = current_address;
            return 0;

        // case MEMORY_FLASH_UNLOCK:
        //     return memory_flash_unlock();
            
        // case MEMORY_FLASH_LOCK:
        //     return memory_flash_lock();
            
        // case MEMORY_FLASH_ERASE_SECTOR:
        //     if (arg == NULL) return -EINVAL;
        //     return memory_flash_erase_sector(*(uint32_t*)arg);
            
        // case MEMORY_FLASH_ERASE_MASS:
        //     // Mass erase not implemented for safety
        //     return -ENOTSUP;
            
        // case MEMORY_FLASH_WRITE:
        //     if (arg == NULL) return -EINVAL;
        //     // arg should point to struct { void* data; size_t len; }
        //     // This is a direct flash write command
        //     return -ENOTSUP; // Use regular write() instead
            
        default:
            return -ENOTSUP;
    }
}

// Memory device instance (static - hidden inside module)
static const interface_t dev_memory = {
    .read = memory_read,
    .write = memory_write,
    .ioctl = memory_ioctrl
};

// Memory device instance accessor
const interface_t* dev_memory_get(void) {
    return &dev_memory;
}