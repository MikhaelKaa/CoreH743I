// Например, в файле ucmd.c или аналогичном:
#include "vfs.h"
#include <stdio.h>

int cmd_test_vfs(int argc, char *argv[]) {
    printf("VFS Test Commands:\r\n");
    printf("  vfs ls       - List root directory\r\n");
    printf("  vfs cwd      - Show current directory\r\n");
    printf("  vfs open     - Test opening /dev/uart1\r\n");
    
    if (argc < 2) return 0;
    
    if (strcmp(argv[1], "ls") == 0) {
        // Простая реализация листинга корневой директории
        printf("Root directory contents:\r\n");
        vfs_inode_t *child = (vfs_inode_t*)vfs_state.root->data;
        while (child) {
            printf("  %s (type: %d)\r\n", child->name, child->type);
            child = child->next;
        }
    }
    else if (strcmp(argv[1], "cwd") == 0) {
        char cwd[VFS_PATH_MAX];
        if (vfs_getcwd(cwd, sizeof(cwd))) {
            printf("Current directory: %s\r\n", cwd);
        }
    }
    else if (strcmp(argv[1], "open") == 0) {
        vfs_file_t *file;
        int result = vfs_open("/dev/uart1", O_WRONLY, &file);
        printf("Open result: %d\r\n", result);
        if (result == 0) {
            vfs_write(file, "Test from CLI command\r\n", 23);
            vfs_close(file);
            printf("Write test completed\r\n");
        }
    }
    
    return 0;
}
