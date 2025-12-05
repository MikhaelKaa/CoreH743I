// vfs.h - Основные структуры
#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include "dev_interface.h"

#define VFS_PATH_MAX 256
#define VFS_NAME_MAX 32
#define VFS_MAX_FDS 16

// Флаги открытия файлов 
#define O_RDONLY    0x0000
#define O_WRONLY    0x0001
#define O_RDWR      0x0002
#define O_CREAT     0x0100
#define O_TRUNC     0x0200
#define O_APPEND    0x0400

typedef struct vfs_inode vfs_inode_t;
typedef struct vfs_file vfs_file_t;
typedef struct vfs_mount vfs_mount_t;

// Типы узлов (inode)
typedef enum {
    VFS_TYPE_INVALID = 0,
    VFS_TYPE_DIR,
    VFS_TYPE_FILE,
    VFS_TYPE_DEVICE, 
} vfs_type_t;

// Структура виртуального узла (inode)
struct vfs_inode {
    vfs_type_t type;            // Что это: файл, директория, устройство?
    const char *name;           // Имя в родительской директории
    uint32_t size;              // Размер (актуально для файлов)
    
    // ОПЕРАЦИИ - сердце VFS
    struct {
        int (*open)(vfs_inode_t *inode, vfs_file_t *file);
        int (*close)(vfs_file_t *file);
        ssize_t (*read)(vfs_file_t *file, void *buf, size_t len);
        ssize_t (*write)(vfs_file_t *file, const void *buf, size_t len);
        int (*ioctl)(vfs_file_t *file, int cmd, void *arg);
        // Для директорий:
        int (*mkdir)(vfs_inode_t *dir, const char *name);
        vfs_inode_t *(*lookup)(vfs_inode_t *dir, const char *name);
    } ops;
    
    void *data; // Приватные данные
    vfs_inode_t *parent;  // Добавить: указатель на родительский каталог
    vfs_inode_t *next;    // Добавить: следующий inode в том же каталоге
};

// Структура открытого файла
struct vfs_file {
    vfs_inode_t *inode;   // Связанный inode
    off_t pos;            // Текущая позиция
    int flags;            // Флаги открытия
    void *private_data;   // Данные для конкретного драйвера
};

// Структура точки монтирования
struct vfs_mount {
    const char *path;           // Путь, куда смонтировано
    vfs_inode_t *root;          // Корневой inode смонтированной ФС
    vfs_mount_t *next;          // Следующая точка монтирования
};

// Глобальное состояние VFS
typedef struct {
    vfs_inode_t *root;          // Корневая директория системы
    vfs_mount_t *mounts;        // Список точек монтирования
    char cwd[VFS_PATH_MAX];     // Текущий путь
} vfs_state_t;

// Основной API VFS
int vfs_init(void);                            // Инициализация VFS
int vfs_mkdir(const char *path);               // Создание директории
int vfs_open(const char *path, int flags, vfs_file_t **file);
int vfs_close(vfs_file_t *file);
ssize_t vfs_read(vfs_file_t *file, void *buf, size_t len);
ssize_t vfs_write(vfs_file_t *file, const void *buf, size_t len);
int vfs_ioctl(vfs_file_t *file, int cmd, void *arg);
int vfs_mount(const char *source, const char *target, const char *fs_type);
int vfs_chdir(const char *path);
char *vfs_getcwd(char *buf, size_t size);

// Функции для работы с inode
vfs_inode_t *vfs_create_inode(vfs_type_t type, const char *name);
int vfs_link(const char *parent_path, vfs_inode_t *inode);
int vfs_link_inode(vfs_inode_t *parent, vfs_inode_t *child); // Новая функция

// Функция для регистрации устройств (объявим здесь)
int devfs_register_device(const char *name, const interface_t *dev_interface);
void vfs_bootstrap(void);
int cmd_test_vfs(int argc, char *argv[]);
extern vfs_state_t vfs_state;

#endif