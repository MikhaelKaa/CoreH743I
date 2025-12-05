// vfs.c - Минимальная реализация VFS
#include "vfs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Глобальное состояние VFS
vfs_state_t vfs_state;

// Простейшие функции RAM-FS
static int ramfs_open(vfs_inode_t *inode, vfs_file_t *file) {
    file->inode = inode;
    file->pos = 0;
    file->flags = 0;
    return 0;
}

static int ramfs_close(vfs_file_t *file) {
    return 0;
}

static ssize_t ramfs_read(vfs_file_t *file, void *buf, size_t len) {
    return 0; // Для директорий чтение не поддерживается
}

static ssize_t ramfs_write(vfs_file_t *file, const void *buf, size_t len) {
    return 0; // Для директорий запись не поддерживается
}

static int ramfs_ioctl(vfs_file_t *file, int cmd, void *arg) {
    return -1; // Не поддерживается
}

static vfs_inode_t *ramfs_lookup(vfs_inode_t *dir, const char *name);

static int ramfs_mkdir(vfs_inode_t *parent, const char *name) {
    vfs_inode_t *new_dir = vfs_create_inode(VFS_TYPE_DIR, name);
    if (!new_dir) return -1;
    
    new_dir->parent = parent;
    new_dir->ops.open = ramfs_open;
    new_dir->ops.close = ramfs_close;
    new_dir->ops.read = ramfs_read;
    new_dir->ops.write = ramfs_write;
    new_dir->ops.ioctl = ramfs_ioctl;
    new_dir->ops.mkdir = ramfs_mkdir;
    new_dir->ops.lookup = ramfs_lookup; 
    
    // Привязываем новый каталог к родительскому
    return vfs_link_inode(parent, new_dir);
}

static vfs_inode_t *ramfs_lookup(vfs_inode_t *dir, const char *name) {
    // Простой поиск по списку дочерних inode
    vfs_inode_t *child = (vfs_inode_t*)dir->data;
    while (child) {
        if (strcmp(child->name, name) == 0) {
            return child;
        }
        child = child->next;
    }
    return NULL;
}

// Создание нового inode
vfs_inode_t *vfs_create_inode(vfs_type_t type, const char *name) {
    vfs_inode_t *inode = (vfs_inode_t*)malloc(sizeof(vfs_inode_t));
    if (!inode) return NULL;
    
    memset(inode, 0, sizeof(vfs_inode_t));
    inode->type = type;
    inode->name = strdup(name); // Нужна реализация strdup или замена
    
    return inode;
}

// Связывание inode с родительским каталогом
int vfs_link_inode(vfs_inode_t *parent, vfs_inode_t *child) {
    if (!parent || !child || parent->type != VFS_TYPE_DIR) return -1;
    
    child->parent = parent;
    child->next = (vfs_inode_t*)parent->data;
    parent->data = child;
    
    return 0;
}

int vfs_link(const char *parent_path, vfs_inode_t *inode) {
    // Пока что упрощённая реализация - только для корня
    if (strcmp(parent_path, "/") == 0 || strcmp(parent_path, "/dev") == 0) {
        vfs_inode_t *parent = (strcmp(parent_path, "/") == 0) ? 
                              vfs_state.root : ramfs_lookup(vfs_state.root, "dev");
        if (!parent) return -1;
        return vfs_link_inode(parent, inode);
    }
    return -1;
}

// Инициализация VFS
int vfs_init(void) {
    printf("[VFS] Initializing...\r\n");
    memset(&vfs_state, 0, sizeof(vfs_state_t));
    strcpy(vfs_state.cwd, "/");
    
    // Создаем корневой каталог
    vfs_state.root = vfs_create_inode(VFS_TYPE_DIR, "/");
    if (!vfs_state.root) {
        printf("[VFS] ERROR: Failed to create root inode\r\n");
        return -1;
    }
    
    vfs_state.root->ops.open = ramfs_open;
    vfs_state.root->ops.close = ramfs_close;
    vfs_state.root->ops.read = ramfs_read;
    vfs_state.root->ops.write = ramfs_write;
    vfs_state.root->ops.ioctl = ramfs_ioctl;
    vfs_state.root->ops.mkdir = ramfs_mkdir;
    vfs_state.root->ops.lookup = ramfs_lookup;
    
    printf("[VFS] Initialized successfully\r\n");
    return 0;
}

// Создание директории
int vfs_mkdir(const char *path) {
    printf("[VFS] mkdir: %s\r\n", path);
    if (path[0] != '/' || strchr(path + 1, '/') != NULL) {
        printf("[VFS] ERROR: Invalid path or nested directories not supported yet\r\n");
        return -1;
    }
    
    const char *name = path + 1;
    int result = ramfs_mkdir(vfs_state.root, name);
    printf("[VFS] mkdir result: %d\r\n", result);
    return result;
}

int vfs_open(const char *path, int flags, vfs_file_t **file) {
    printf("[VFS] open: %s, flags: 0x%x\r\n", path, flags);
    
    // Проверяем, что путь начинается с '/'
    if (path[0] != '/') {
        printf("[VFS] ERROR: Path must be absolute\r\n");
        return -1;
    }
    
    // Пропускаем начальный '/'
    const char *current = path + 1;
    
    // Начинаем с корневого каталога
    vfs_inode_t *dir = vfs_state.root;
    vfs_inode_t *inode = NULL;
    
    // Если путь только "/", то открываем корень (директорию)
    if (*current == '\0') {
        inode = dir;
    } else {
        // Разбираем путь по компонентам
        while (1) {
            // Находим следующий разделитель
            const char *next = strchr(current, '/');
            size_t len = next ? (size_t)(next - current) : strlen(current);
            
            // Копируем имя компонента во временный буфер
            char component[VFS_NAME_MAX];
            if (len >= VFS_NAME_MAX) {
                printf("[VFS] ERROR: Component too long\r\n");
                return -1;
            }
            strncpy(component, current, len);
            component[len] = '\0';
            
            printf("[VFS] Looking up component: '%s' in dir '%s'\r\n", component, dir->name);
            
            // Ищем компонент в текущей директории
            if (!dir->ops.lookup) {
                printf("[VFS] ERROR: Directory does not support lookup\r\n");
                return -1;
            }
            inode = dir->ops.lookup(dir, component);
            if (!inode) {
                printf("[VFS] ERROR: Component not found: '%s'\r\n", component);
                return -1;
            }
            
            // Если это последний компонент, выходим из цикла
            if (!next) {
                break;
            }
            
            // Если есть ещё компоненты, проверяем, что текущий inode - директория
            if (inode->type != VFS_TYPE_DIR) {
                printf("[VFS] ERROR: '%s' is not a directory\r\n", component);
                return -1;
            }
            
            // Переходим в следующую директорию
            dir = inode;
            current = next + 1;
        }
    }
    
    // Теперь inode указывает на целевой узел
    printf("[VFS] Found inode: type=%d, name=%s\r\n", inode->type, inode->name);
    
    // Создаём структуру файла
    vfs_file_t *f = (vfs_file_t*)malloc(sizeof(vfs_file_t));
    if (!f) {
        printf("[VFS] ERROR: Out of memory\r\n");
        return -1;
    }
    
    memset(f, 0, sizeof(vfs_file_t));
    f->inode = inode;
    f->flags = flags;
    f->pos = 0;
    
    // Для устройств устанавливаем private_data
    if (inode->type == VFS_TYPE_DEVICE) {
        f->private_data = inode->data; // Это указатель на interface_t
        printf("[VFS] Device opened, interface at: %p\r\n", inode->data);
    }
    
    // Вызываем операцию open у inode, если она есть
    if (inode->ops.open) {
        int result = inode->ops.open(inode, f);
        if (result != 0) {
            printf("[VFS] ERROR: open operation failed: %d\r\n", result);
            free(f);
            return result;
        }
    }
    
    *file = f;
    printf("[VFS] File opened successfully\r\n");
    return 0;
}

int vfs_close(vfs_file_t *file) {
    if (file && file->inode && file->inode->ops.close) {
        return file->inode->ops.close(file);
    }
    return -1;
}

ssize_t vfs_read(vfs_file_t *file, void *buf, size_t len) {
    if (file && file->inode && file->inode->ops.read) {
        return file->inode->ops.read(file, buf, len);
    }
    return -1;
}

ssize_t vfs_write(vfs_file_t *file, const void *buf, size_t len) {
    if (file && file->inode && file->inode->ops.write) {
        return file->inode->ops.write(file, buf, len);
    }
    return -1;
}

int vfs_ioctl(vfs_file_t *file, int cmd, void *arg) {
    if (file && file->inode && file->inode->ops.ioctl) {
        return file->inode->ops.ioctl(file, cmd, arg);
    }
    return -1;
}

int vfs_chdir(const char *path) {
    // Упрощённая реализация - только корень
    if (strcmp(path, "/") == 0) {
        strcpy(vfs_state.cwd, "/");
        return 0;
    }
    return -1;
}

char *vfs_getcwd(char *buf, size_t size) {
    if (!buf || size < strlen(vfs_state.cwd) + 1) return NULL;
    strcpy(buf, vfs_state.cwd);
    return buf;
}

// Остальные функции пока заглушки
int vfs_mount(const char *source, const char *target, const char *fs_type) {
    return -1; // TODO
}