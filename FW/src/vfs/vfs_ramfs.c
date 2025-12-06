#include "vfs_ramfs.h"
#include "vfs_api.h"  // Добавляем для vfs_inode_create, vfs_inode_link
#include <string.h>
#include <stdlib.h>

#ifdef VFS_DEBUG
#include <stdio.h>
#define RAMFS_LOG(fmt, ...) printf("[RAMFS] " fmt, ##__VA_ARGS__)
#else
#define RAMFS_LOG(fmt, ...)
#endif

// Внутренние функции - static
static int ramfs_open(vfs_inode_t *inode, vfs_file_t *file) {
    RAMFS_LOG("open: %s\r\n", inode->name);
    file->inode = inode;
    file->pos = 0;
    return VFS_OK;
}

static int ramfs_close(vfs_file_t *file) {
    RAMFS_LOG("close\r\n");
    (void)file; // Для подавления предупреждения о неиспользуемом параметре
    return VFS_OK;
}

static ssize_t ramfs_read(vfs_file_t *file, void *buf, size_t len) {
    // Для директорий чтение не поддерживается
    (void)file;
    (void)buf;
    (void)len;
    return VFS_ERR_NO_IMPL;
}

static ssize_t ramfs_write(vfs_file_t *file, const void *buf, size_t len) {
    // Для директорий запись не поддерживается
    (void)file;
    (void)buf;
    (void)len;
    return VFS_ERR_NO_IMPL;
}

static int ramfs_ioctl(vfs_file_t *file, int cmd, void *arg) {
    (void)file;
    (void)cmd;
    (void)arg;
    return VFS_ERR_NO_IMPL;
}

static vfs_inode_t *ramfs_lookup(vfs_inode_t *dir, const char *name);

static int ramfs_mkdir(vfs_inode_t *parent, const char *name) {
    RAMFS_LOG("mkdir: %s in %s\r\n", name, parent->name);
    
    // Проверяем, нет ли уже такого имени
    vfs_dirent_t *dirent = parent->children;
    while (dirent) {
        if (strcmp(dirent->inode->name, name) == 0) {
            return VFS_ERR_EXISTS;
        }
        dirent = dirent->next;
    }
    
    vfs_inode_t *new_dir = vfs_inode_create(VFS_TYPE_DIR, name);
    if (!new_dir) return VFS_ERR_NO_MEM;
    
    // Устанавливаем операции для новой директории
    new_dir->ops.open = ramfs_open;
    new_dir->ops.close = ramfs_close;
    new_dir->ops.read = ramfs_read;
    new_dir->ops.write = ramfs_write;
    new_dir->ops.ioctl = ramfs_ioctl;
    new_dir->ops.mkdir = ramfs_mkdir;
    new_dir->ops.lookup = ramfs_lookup;
    
    return vfs_inode_link(parent, new_dir);
}

static vfs_inode_t *ramfs_lookup(vfs_inode_t *dir, const char *name) {
    RAMFS_LOG("lookup: %s in %s\r\n", name, dir->name);
    
    vfs_dirent_t *dirent = dir->children;
    while (dirent) {
        if (strcmp(dirent->inode->name, name) == 0) {
            return dirent->inode;
        }
        dirent = dirent->next;
    }
    return NULL;
}

// Публичная функция - создание корневого каталога RAM-FS
vfs_inode_t *ramfs_root_create(void) {
    vfs_inode_t *root = vfs_inode_create(VFS_TYPE_DIR, "/");
    if (!root) return NULL;
    
    // Устанавливаем операции для корневой директории
    root->ops.open = ramfs_open;
    root->ops.close = ramfs_close;
    root->ops.read = ramfs_read;
    root->ops.write = ramfs_write;
    root->ops.ioctl = ramfs_ioctl;
    root->ops.mkdir = ramfs_mkdir;
    root->ops.lookup = ramfs_lookup;
    
    RAMFS_LOG("Root directory created\r\n");
    return root;
}