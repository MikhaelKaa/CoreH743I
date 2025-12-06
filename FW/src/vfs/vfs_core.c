#include "vfs_core.h"
#include "vfs_ramfs.h"
#include "vfs_devfs.h"
#include "vfs_path.h"
#include <string.h>
#include <stdlib.h>

#ifdef VFS_DEBUG
#include <stdio.h>
#define VFS_LOG(fmt, ...) printf("[VFS] " fmt, ##__VA_ARGS__)
#else
#define VFS_LOG(fmt, ...) 
#endif

// Глобальное состояние (без static)
vfs_state_t vfs_state;

// Функции управления inode
vfs_inode_t *vfs_inode_create(vfs_type_t type, const char *name) {
    vfs_inode_t *inode = malloc(sizeof(vfs_inode_t));
    if (!inode) return NULL;
    
    memset(inode, 0, sizeof(vfs_inode_t));
    inode->type = type;
    
    // Копируем имя
    inode->name = malloc(strlen(name) + 1);
    if (!inode->name) {
        free(inode);
        return NULL;
    }
    strcpy(inode->name, name);
    
    inode->children = NULL;
    
    return inode;
}

int vfs_inode_link(vfs_inode_t *parent, vfs_inode_t *child) {
    if (!parent || !child || parent->type != VFS_TYPE_DIR) {
        return VFS_ERR_INVALID;
    }
    
    child->parent = parent;
    
    vfs_dirent_t *dirent = malloc(sizeof(vfs_dirent_t));
    if (!dirent) return VFS_ERR_NO_MEM;
    
    dirent->inode = child;
    dirent->next = parent->children;
    parent->children = dirent;
    
    return VFS_OK;
}

// Основная инициализация VFS
int vfs_init(void) {
    VFS_LOG("Initializing...\r\n");
    memset(&vfs_state, 0, sizeof(vfs_state_t));
    strcpy(vfs_state.cwd, "/");
    
    // Создаем корневой каталог с помощью RAM-FS
    vfs_state.root = ramfs_root_create();
    if (!vfs_state.root) {
        VFS_LOG("Failed to create root inode\r\n");
        return VFS_ERR_NO_MEM;
    }
    
    VFS_LOG("Initialized successfully\r\n");
    return VFS_OK;
}

// Поиск inode по пути
vfs_inode_t *vfs_lookup(const char *path) {
    return vfs_path_lookup(vfs_state.root, path);
}

const char *vfs_strerror(int err) {
    switch (err) {
        case VFS_OK: return "Success";
        case VFS_ERR_NOT_FOUND: return "Not found";
        case VFS_ERR_NOT_DIR: return "Not a directory";
        case VFS_ERR_NO_MEM: return "Out of memory";
        case VFS_ERR_EXISTS: return "Already exists";
        case VFS_ERR_INVALID: return "Invalid argument";
        case VFS_ERR_IO: return "I/O error";
        case VFS_ERR_NO_IMPL: return "Not implemented";
        case VFS_ERR_BAD_FD: return "Bad file descriptor";
        case VFS_ERR_PERM: return "Permission denied";
        default: return "Unknown error";
    }
}