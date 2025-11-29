/**
 * @file memory_man.c
 * @brief Memory utilities
 * @author Mikhael Kaa (Михаил Каа)
 * @date 21.06.2025
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include "dev_interface.h"
#include "dev_mem.h"

static interface_t* mem_interface = NULL;
static void         print_usage(void);
static int          mem_dump(uintptr_t addr, uint32_t len);
static uint32_t     mem_test(uintptr_t addr, uint32_t len);
static int          mem_copy(uintptr_t dst, uintptr_t src, uint32_t len);
static int          mem_read_byte(uintptr_t addr, uint8_t* value);
static int          mem_write_byte(uintptr_t addr, uint8_t value);

// Set memory interface
int mem_set_interface(interface_t* iface) {
    if (iface == NULL) {
        return -EINVAL;
    }
    mem_interface = iface;
    return 0;
}

#ifdef BAREMETAL
int ucmd_mem(int argc, char* argv[])
#define ENDL "\r\n"
#else
int main(int argc, char* argv[])
#define ENDL "\n"
#endif // BAREMETAL
{
    uint32_t addr, data, len;
    uint32_t src, dst;
    uint8_t value;
    int ret;

    if (mem_interface == NULL) {
        printf("Memory interface not set" ENDL);
        return -ENODEV;
    }

    switch (argc) {
        case 1:
        case 2:
            print_usage();
            return -EINVAL;

        case 3:
            if (strcmp(argv[1], "read") == 0) {
                if (sscanf(argv[2], "%lx", &addr) != 1) {
                    printf("Invalid address format" ENDL);
                    return -EINVAL;
                }
                ret = mem_read_byte(addr, &value);
                if (ret == 0) {
                    printf("0x%02x" ENDL, value);
                } else {
                    printf("Read failed: %d" ENDL, ret);
                }
                return ret;
            }
            break;

        case 4:
            if (strcmp(argv[1], "test") == 0) {
                if (sscanf(argv[2], "%lx", &addr) != 1 || sscanf(argv[3], "%lx", &len) != 1) {
                    printf("Invalid arguments format" ENDL);
                    return -EINVAL;
                }
                return (int)mem_test(addr, len);
            }

            if (strcmp(argv[1], "dump") == 0) {
                if (sscanf(argv[2], "%lx", &addr) != 1 || sscanf(argv[3], "%lx", &len) != 1) {
                    printf("Invalid arguments format" ENDL);
                    return -EINVAL;
                }
                return mem_dump(addr, len);
            }

            if (strcmp(argv[1], "write") == 0) {
                if (sscanf(argv[2], "%lx", &addr) != 1 || sscanf(argv[3], "%lx", &data) != 1) {
                    printf("Invalid arguments format" ENDL);
                    return -EINVAL;
                }
                ret = mem_write_byte(addr, (uint8_t)data);
                if (ret != 0) {
                    printf("Write failed: %d" ENDL, ret);
                }
                return ret;
            }
            break;

        case 5:
            if (strcmp(argv[1], "cpy") == 0) {
                if (sscanf(argv[2], "%lx", &dst) != 1 || 
                    sscanf(argv[3], "%lx", &src) != 1 || 
                    sscanf(argv[4], "%lx", &len) != 1) {
                    printf("Invalid arguments format" ENDL);
                    return -EINVAL;
                }
                return mem_copy(dst, src, len);
            }
            break;

        default:
            print_usage();
            return -EINVAL;
    }

    print_usage();
    return -EINVAL;
}

static void print_usage(void) {
    printf("Usage: mem <command> [arguments]" ENDL);
    printf("Commands:" ENDL);
    printf("  dump <adr> <len>    - Hexdump of memory region" ENDL);
    printf("  read <adr>          - Read byte from address" ENDL);
    printf("  write <adr> <data>  - Write byte to address" ENDL);
    printf("  test <adr> <len>    - Test memory region" ENDL);
    printf("  cpy <dst> <src> <len> - Copy memory block" ENDL);
}

static int mem_dump(uintptr_t addr, uint32_t len) {
    uint8_t buffer[16];
    uint32_t total_read = 0;
    uint32_t current_addr = addr;

    while (total_read < len) {
        uint32_t chunk = (len - total_read) > sizeof(buffer) ? sizeof(buffer) : (len - total_read);
        
        // Set address for reading
        int ret = mem_interface->ioctrl(MEMORY_SET_ADDRESS, &current_addr);
        if (ret != 0) {
            printf("Failed to set address: %d" ENDL, ret);
            return ret;
        }

        // Read chunk
        int bytes_read = mem_interface->read(buffer, chunk);
        if (bytes_read != (int)chunk) {
            printf("Read failed at 0x%08lx: got %d, expected %lu" ENDL, 
                   (unsigned long)current_addr, bytes_read, chunk);
            return -EIO;
        }

        // Print address
        printf("0x%08lx: ", (unsigned long)current_addr);

        // Print hex values
        for (uint32_t i = 0; i < chunk; i++) {
            printf("%02x ", buffer[i]);
        }
        
        // Pad if incomplete line
        for (uint32_t i = chunk; i < sizeof(buffer); i++) {
            printf("   ");
        }

        // Print ASCII representation
        printf("| ");
        for (uint32_t i = 0; i < chunk; i++) {
            uint8_t c = buffer[i];
            putchar((c >= 32 && c <= 126) ? c : '.');
        }
        printf(ENDL);

        total_read += chunk;
        current_addr += chunk;
    }
    return 0;
}

static uint32_t mem_test(uintptr_t addr, uint32_t len) {
    uint32_t error_count = 0;
    uint8_t read_back;
    const uint8_t patterns[] = {0x00, 0x55, 0xAA, 0xFF};
    const int num_patterns = sizeof(patterns) / sizeof(patterns[0]);
    const uint32_t max_errors_to_print = 10;

    if (len == 0) {
        printf("Zero-length test skipped" ENDL);
        return 0;
    }

    for (uint32_t i = 0; i < len; i++) {
        uintptr_t current_addr = addr + i;
        
        // Read original value
        int ret = mem_read_byte(current_addr, &read_back);
        if (ret != 0) {
            printf("Failed to read original value at 0x%08lx: %d" ENDL, 
                   (unsigned long)current_addr, ret);
            error_count++;
            continue;
        }
        uint8_t original = read_back;

        // Test patterns
        for (int p = 0; p < num_patterns; p++) {
            uint8_t test_val = patterns[p];
            
            ret = mem_write_byte(current_addr, test_val);
            if (ret != 0) {
                if (error_count < max_errors_to_print) {
                    printf("Write failed @ 0x%08lx: %d" ENDL, 
                           (unsigned long)current_addr, ret);
                }
                error_count++;
                continue;
            }

            ret = mem_read_byte(current_addr, &read_back);
            if (ret != 0) {
                if (error_count < max_errors_to_print) {
                    printf("Read failed @ 0x%08lx: %d" ENDL, 
                           (unsigned long)current_addr, ret);
                }
                error_count++;
                continue;
            }

            if (read_back != test_val) {
                if (error_count < max_errors_to_print) {
                    printf("ERROR @ 0x%08lx: Wrote 0x%02X, Read 0x%02X" ENDL, 
                           (unsigned long)current_addr, test_val, read_back);
                }
                error_count++;
            }
        }

        // Restore original value
        ret = mem_write_byte(current_addr, original);
        if (ret != 0) {
            if (error_count < max_errors_to_print) {
                printf("Restore failed @ 0x%08lx: %d" ENDL, 
                       (unsigned long)current_addr, ret);
            }
            error_count++;
        }

        // Progress indicator
        if ((i % 8192) == 0) {
            printf("Tested %lu/%lu bytes (errors: %lu)" ENDL, i, len, error_count);
        }
    }

    // Final report
    if (error_count == 0) {
        printf("Memory test PASSED: %lu bytes" ENDL, len);
    } else {
        printf("Memory test FAILED! Errors: %lu/%lu bytes" ENDL, error_count, len);
    }

    return error_count;
}

static int mem_copy(uintptr_t dst, uintptr_t src, uint32_t len) {
    uint8_t buffer[64];
    uint32_t total_copied = 0;
    int ret;

    while (total_copied < len) {
        uint32_t chunk = (len - total_copied) > sizeof(buffer) ? sizeof(buffer) : (len - total_copied);
        
        // Set source address and read
        uintptr_t src_addr = src + total_copied;
        ret = mem_interface->ioctrl(MEMORY_SET_ADDRESS, &src_addr);
        if (ret != 0) {
            printf("Failed to set source address: %d" ENDL, ret);
            return ret;
        }

        ret = mem_interface->read(buffer, chunk);
        if (ret != (int)chunk) {
            printf("Read failed at 0x%08lx: got %d, expected %lu" ENDL, 
                   (unsigned long)src_addr, ret, chunk);
            return -EIO;
        }

        // Set destination address and write
        uintptr_t dst_addr = dst + total_copied;
        ret = mem_interface->ioctrl(MEMORY_SET_ADDRESS, &dst_addr);
        if (ret != 0) {
            printf("Failed to set destination address: %d" ENDL, ret);
            return ret;
        }

        ret = mem_interface->write(buffer, chunk);
        if (ret != (int)chunk) {
            printf("Write failed at 0x%08lx: got %d, expected %lu" ENDL, 
                   (unsigned long)dst_addr, ret, chunk);
            return -EIO;
        }

        total_copied += chunk;
    }
    return 0;
}

static int mem_read_byte(uintptr_t addr, uint8_t* value) {
    int ret;
    
    ret = mem_interface->ioctrl(MEMORY_SET_ADDRESS, &addr);
    if (ret != 0) {
        return ret;
    }
    
    ret = mem_interface->read(value, 1);
    if (ret == 1) {
        return 0; // Success
    } else if (ret >= 0) {
        return -EIO; // Wrong number of bytes read
    } else {
        return ret; // Error code
    }
}

static int mem_write_byte(uintptr_t addr, uint8_t value) {
    int ret;
    
    ret = mem_interface->ioctrl(MEMORY_SET_ADDRESS, &addr);
    if (ret != 0) {
        return ret;
    }
    
    ret = mem_interface->write(&value, 1);
    if (ret == 1) {
        return 0; // Success
    } else if (ret >= 0) {
        return -EIO; // Wrong number of bytes written
    } else {
        return ret; // Error code
    }
}

#undef ENDL