#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "vfs_api.h"
#include "vfs_core.h"

#ifdef VFS_DEBUG
#include <stdio.h>
#define UCMD_LOG(fmt, ...) printf("[UCMD] " fmt, ##__VA_ARGS__)
#else
#define UCMD_LOG(fmt, ...)
#endif

extern vfs_state_t vfs_state;

int cmd_test_vfs(int argc, char *argv[]) {
    printf("VFS Test Commands:\r\n");
    printf("  vfs ls       - List root directory\r\n");
    printf("  vfs cwd      - Show current directory\r\n");
    printf("  vfs open     - Test opening /dev/uart1\r\n");
    
    if (argc < 2) return 0;
    
    if (strcmp(argv[1], "ls") == 0) {
        // Простая реализация листинга корневой директории
        printf("Root directory contents:\r\n");
        vfs_dirent_t *dirent = vfs_state.root->children;
        while (dirent) {
            printf("  %s (type: %d)\r\n", dirent->inode->name, dirent->inode->type);
            dirent = dirent->next;
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
            const char *msg = "Test from CLI command\r\n";
            vfs_write(file, msg, strlen(msg));
            vfs_close(file);
            printf("Write test completed\r\n");
        } else {
            printf("Error: %s\r\n", vfs_strerror(result));
        }
    }
    
    return 0;
}