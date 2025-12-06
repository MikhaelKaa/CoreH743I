#ifndef VFS_API_H
#define VFS_API_H

#include "vfs_core.h"
#include <fcntl.h>  // Добавляем для стандартных O_* констант

// Основной API VFS
int vfs_open(const char *path, int flags, vfs_file_t **file);
int vfs_close(vfs_file_t *file);
ssize_t vfs_read(vfs_file_t *file, void *buf, size_t len);
ssize_t vfs_write(vfs_file_t *file, const void *buf, size_t len);
int vfs_ioctl(vfs_file_t *file, int cmd, void *arg);
int vfs_mkdir(const char *path);
int vfs_mount(const char *source, const char *target, const char *fs_type);
int vfs_chdir(const char *path);
char *vfs_getcwd(char *buf, size_t size);
vfs_inode_t *vfs_lookup(const char *path);

// Функции для работы с inode
vfs_inode_t *vfs_inode_create(vfs_type_t type, const char *name);
int vfs_inode_link(vfs_inode_t *parent, vfs_inode_t *child);

void vfs_bootstrap(void);
int cmd_test_vfs(int argc, char *argv[]);

#endif /* VFS_API_H */