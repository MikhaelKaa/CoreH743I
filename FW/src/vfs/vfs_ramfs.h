#ifndef VFS_RAMFS_H
#define VFS_RAMFS_H

#include "vfs_core.h"

// RAM-FS операции
vfs_inode_t *ramfs_root_create(void);
// int ramfs_mkdir(vfs_inode_t *parent, const char *name);
// vfs_inode_t *ramfs_lookup(vfs_inode_t *dir, const char *name);
// int ramfs_open(vfs_inode_t *inode, vfs_file_t *file);
// int ramfs_close(vfs_file_t *file);
// ssize_t ramfs_read(vfs_file_t *file, void *buf, size_t len);
// ssize_t ramfs_write(vfs_file_t *file, const void *buf, size_t len);
// int ramfs_ioctl(vfs_file_t *file, int cmd, void *arg);

#endif /* VFS_RAMFS_H */