#include "vfs.h"
#include "dev_uart1.h"
#include "dev_mem.h"
#include <string.h>

void vfs_bootstrap(void) {
    printf("\r\n=== VFS Bootstrap ===\r\n");
    
    // 1. Инициализируем корневую RAM-FS
    if (vfs_init() != 0) {
        printf("VFS init failed!\r\n");
        return;
    }
    
    // 2. Создаем стандартные директории
    printf("\r\nCreating directories:\r\n");
    vfs_mkdir("/dev");
    vfs_mkdir("/tmp");
    vfs_mkdir("/mnt");
    
    // 3. Регистрируем ваши устройства в DevFS
    printf("\r\nRegistering devices:\r\n");
    devfs_register_device("uart1", dev_uart1_get());
    printf("Registered uart1\r\n");
    devfs_register_device("mem0", dev_memory_get());
    printf("Registered mem0\r\n");
    
    // 4. Устанавливаем начальный текущий путь
    vfs_chdir("/");
    char cwd[VFS_PATH_MAX];
    vfs_getcwd(cwd, sizeof(cwd));
    printf("Current directory: %s\r\n", cwd);
    
    // 5. Тест: открываем UART как файл
    printf("\r\nTesting device access:\r\n");
    vfs_file_t *uart_file;
    int result = vfs_open("/dev/uart1", O_WRONLY, &uart_file);
    printf("vfs_open result: %d\r\n", result);
    
    if(result == 0) {
        const char *msg = "VFS is alive! Writing via VFS.\r\n";
        printf("Attempting to write: %s", msg);
        
        ssize_t write_result = vfs_write(uart_file, msg, strlen(msg));
        printf("vfs_write result: %d bytes written\r\n", write_result);
        
        vfs_close(uart_file);
        printf("File closed\r\n");
    }
    
    printf("=== VFS Bootstrap Complete ===\r\n\r\n");
}