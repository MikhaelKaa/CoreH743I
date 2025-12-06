#ifndef VFS_PATH_H
#define VFS_PATH_H

#include "vfs_core.h"

// Функции для работы с путями
vfs_inode_t *vfs_path_lookup(vfs_inode_t *root, const char *path);
vfs_inode_t *vfs_path_parent(const char *path);
const char *vfs_path_basename(const char *path);
int vfs_path_normalize(char *path);
char *vfs_path_join(const char *dir, const char *name);

#endif /* VFS_PATH_H */