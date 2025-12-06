#include "vfs_path.h"
#include <string.h>
#include <stdlib.h>

vfs_inode_t *vfs_path_lookup(vfs_inode_t *root, const char *path) {
    if (!path || !root) return NULL;
    
    // Пропускаем начальный '/'
    const char *current = (*path == '/') ? path + 1 : path;
    
    vfs_inode_t *dir = root;
    
    if (*current == '\0') {
        return dir; // Путь "/"
    }
    
    while (*current) {
        const char *next = strchr(current, '/');
        size_t len = next ? (size_t)(next - current) : strlen(current);
        
        char component[VFS_NAME_MAX];
        if (len >= VFS_NAME_MAX) return NULL;
        
        strncpy(component, current, len);
        component[len] = '\0';
        
        if (!dir->ops.lookup) return NULL;
        
        vfs_inode_t *inode = dir->ops.lookup(dir, component);
        if (!inode) return NULL;
        
        if (!next) return inode;
        
        if (inode->type != VFS_TYPE_DIR) return NULL;
        
        dir = inode;
        current = next + 1;
    }
    
    return NULL;
}

vfs_inode_t *vfs_path_parent(const char *path) {
    (void)path;
    // Упрощенная реализация - всегда возвращает корень
    // TODO: нормальная реализация
    extern vfs_state_t vfs_state;
    return vfs_state.root;
}

const char *vfs_path_basename(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? slash + 1 : path;
}

int vfs_path_normalize(char *path) {
    (void)path;
    // TODO: нормализация пути (удаление "/./", "/../" и т.д.)
    return 0;
}

char *vfs_path_join(const char *dir, const char *name) {
    size_t dir_len = strlen(dir);
    size_t name_len = strlen(name);
    size_t total = dir_len + name_len + 2;
    
    // TODO: check for mem leak
    char *result = malloc(total);
    if (!result) return NULL;
    
    strcpy(result, dir);
    
    // Добавляем '/' если его нет
    if (dir_len > 0 && result[dir_len - 1] != '/') {
        strcat(result, "/");
    }
    
    strcat(result, name);
    return result;
}
