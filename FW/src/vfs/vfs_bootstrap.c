#include "vfs_api.h"
#include "vfs_devfs.h"
#include "dev_uart1.h"
#include "dev_mem.h"
#include <string.h>
#include <fcntl.h>  // Добавляем для O_WRONLY

#ifdef VFS_DEBUG
#include <stdio.h>
#define BOOT_LOG(fmt, ...) printf("[BOOT] " fmt, ##__VA_ARGS__)
#else
#define BOOT_LOG(fmt, ...)
#endif

void vfs_bootstrap(void) {
    BOOT_LOG("=== VFS Bootstrap ===\r\n");
    
    // 1. Инициализируем корневую VFS
    if (vfs_init() != VFS_OK) {
        BOOT_LOG("VFS init failed!\r\n");
        return;
    }
    
    // 2. Создаем стандартные директории
    BOOT_LOG("Creating directories...\r\n");
    vfs_mkdir("/dev");
    vfs_mkdir("/tmp");
    vfs_mkdir("/mnt");
    
    // 3. Регистрируем устройства
    BOOT_LOG("Registering devices...\r\n");
    devfs_register_device("uart1", dev_uart1_get());
    devfs_register_device("mem0", dev_memory_get());
    
    // 4. Тестируем
    BOOT_LOG("Testing device access...\r\n");
    vfs_file_t *uart_file;
    int result = vfs_open("/dev/uart1", O_WRONLY, &uart_file);
    
    if(result == VFS_OK) {
        const char *msg = "VFS bootstrap complete!\r\n";
        vfs_write(uart_file, msg, strlen(msg));
        vfs_close(uart_file);
    }
    
    BOOT_LOG("=== Bootstrap Complete ===\r\n");
}