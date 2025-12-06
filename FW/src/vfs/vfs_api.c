#include <string.h>
#include <stdlib.h>

#include "vfs_api.h"
#include "vfs_core.h"
#include "vfs_path.h"

#ifdef VFS_DEBUG
#include <stdio.h>
#define VFS_LOG(fmt, ...) printf("[VFS] " fmt, ##__VA_ARGS__)
#else
#define VFS_LOG(fmt, ...)
#endif

extern vfs_state_t vfs_state;

int vfs_open(const char *path, int flags, vfs_file_t **file) {
    VFS_LOG("open: %s, flags: 0x%x\r\n", path, flags);
    
    vfs_inode_t *inode = vfs_lookup(path);
    if (!inode) {
        VFS_LOG("Not found: %s\r\n", path);
        return VFS_ERR_NOT_FOUND;
    }
    
    vfs_file_t *f = malloc(sizeof(vfs_file_t));
    if (!f) return VFS_ERR_NO_MEM;
    
    memset(f, 0, sizeof(vfs_file_t));
    f->inode = inode;
    f->flags = flags;
    f->pos = 0;
    
    if (inode->ops.open) {
        int result = inode->ops.open(inode, f);
        if (result != VFS_OK) {
            free(f);
            return result;
        }
    }
    
    *file = f;
    return VFS_OK;
}


int vfs_close(vfs_file_t *file) {
    if (!file) return VFS_ERR_INVALID;
    
    if (file->inode && file->inode->ops.close) {
        file->inode->ops.close(file);
    }
    
    free(file);
    return VFS_OK;
}

ssize_t vfs_read(vfs_file_t *file, void *buf, size_t len) {
    if (!file || !file->inode) return VFS_ERR_INVALID;
    
    if (file->inode->ops.read) {
        return file->inode->ops.read(file, buf, len);
    }
    
    return VFS_ERR_NO_IMPL;
}

ssize_t vfs_write(vfs_file_t *file, const void *buf, size_t len) {
    if (!file || !file->inode) return VFS_ERR_INVALID;
    
    if (file->inode->ops.write) {
        return file->inode->ops.write(file, buf, len);
    }
    
    return VFS_ERR_NO_IMPL;
}

int vfs_ioctl(vfs_file_t *file, int cmd, void *arg) {
    if (!file || !file->inode) return VFS_ERR_INVALID;
    
    if (file->inode->ops.ioctl) {
        return file->inode->ops.ioctl(file, cmd, arg);
    }
    
    return VFS_ERR_NO_IMPL;
}

int vfs_mkdir(const char *path) {
    VFS_LOG("mkdir: %s\r\n", path);
    
    vfs_inode_t *parent = vfs_path_parent(path);
    if (!parent) return VFS_ERR_NOT_FOUND;
    
    if (!parent->ops.mkdir) {
        return VFS_ERR_NO_IMPL;
    }
    
    const char *name = vfs_path_basename(path);
    return parent->ops.mkdir(parent, name);
}

int vfs_chdir(const char *path) {
    vfs_inode_t *dir = vfs_lookup(path);
    if (!dir || dir->type != VFS_TYPE_DIR) {
        return VFS_ERR_NOT_DIR;
    }
    
    // Здесь должна быть реализация преобразования inode в путь
    // Для простоты пока только корень
    if (strcmp(path, "/") == 0) {
        strcpy(vfs_state.cwd, "/");
        return VFS_OK;
    }
    
    return VFS_ERR_NO_IMPL;
}

char *vfs_getcwd(char *buf, size_t size) {
    if (!buf || size < strlen(vfs_state.cwd) + 1) {
        return NULL;
    }
    strcpy(buf, vfs_state.cwd);
    return buf;
}

int vfs_mount(const char *source, const char *target, const char *fs_type) {
    (void)source;
    (void)target;
    (void)fs_type;
    return VFS_ERR_NO_IMPL; // TODO
}
