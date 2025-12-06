/* SPDX-License-Identifier: MIT */
#ifndef VFS_CORE_H
#define VFS_CORE_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define VFS_PATH_MAX 256
#define VFS_NAME_MAX 32
#define VFS_MAX_FDS 16

// УБИРАЕМ переопределение стандартных констант
// Вместо этого используем стандартные из <fcntl.h>

// Типы узлов (inode)
typedef enum {
    VFS_TYPE_INVALID = 0,
    VFS_TYPE_DIR,
    VFS_TYPE_FILE,
    VFS_TYPE_DEVICE, 
} vfs_type_t;

// Структуры объявлены, определены в vfs_core.c
typedef struct vfs_inode vfs_inode_t;
typedef struct vfs_file vfs_file_t;
typedef struct vfs_mount vfs_mount_t;
typedef struct vfs_dirent vfs_dirent_t;

// Сначала определяем vfs_ops_t, потом используем его
typedef struct {
    int (*open)(vfs_inode_t *inode, vfs_file_t *file);
    int (*close)(vfs_file_t *file);
    ssize_t (*read)(vfs_file_t *file, void *buf, size_t len);
    ssize_t (*write)(vfs_file_t *file, const void *buf, size_t len);
    int (*ioctl)(vfs_file_t *file, int cmd, void *arg);
    int (*mkdir)(vfs_inode_t *dir, const char *name);
    vfs_inode_t *(*lookup)(vfs_inode_t *dir, const char *name);
} vfs_ops_t;

// Теперь определяем структуры
struct vfs_inode {
    vfs_type_t type;
    char *name;
    uint32_t size;
    vfs_ops_t ops;  // Теперь тип определен!
    void *fs_data;
    vfs_inode_t *parent;
    vfs_dirent_t *children;
};

struct vfs_file {
    vfs_inode_t *inode;
    off_t pos;
    int flags;
    void *private_data;
};

struct vfs_mount {
    const char *path;
    vfs_inode_t *root;
    vfs_mount_t *next;
};

struct vfs_dirent {
    vfs_inode_t *inode;
    vfs_dirent_t *next;
};

// Коды ошибок VFS
typedef enum {
    VFS_OK = 0,
    VFS_ERR_NOT_FOUND = -1,
    VFS_ERR_NOT_DIR = -2,
    VFS_ERR_NO_MEM = -3,
    VFS_ERR_EXISTS = -4,
    VFS_ERR_INVALID = -5,
    VFS_ERR_IO = -6,
    VFS_ERR_NO_IMPL = -7,
    VFS_ERR_BAD_FD = -8,
    VFS_ERR_PERM = -9,
} vfs_error_t;

// Глобальное состояние VFS
typedef struct {
    vfs_inode_t *root;
    vfs_mount_t *mounts;
    char cwd[VFS_PATH_MAX];
} vfs_state_t;

// Объявляем глобальное состояние
extern vfs_state_t vfs_state;

// Вспомогательные функции
const char *vfs_strerror(int err);
int vfs_init(void);

#endif /* VFS_CORE_H */