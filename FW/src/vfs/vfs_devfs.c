#include "vfs_devfs.h"
#include "vfs_api.h"
#include <string.h>
#include <stdlib.h>

#ifdef VFS_DEBUG
#include <stdio.h>
#define DEVFS_LOG(fmt, ...) printf("[DevFS] " fmt, ##__VA_ARGS__)
#else
#define DEVFS_LOG(fmt, ...)
#endif

// Внутренние функции - static
static ssize_t devfs_read(vfs_file_t *file, void *buf, size_t len) {
    interface_t* dev = (interface_t*)(file->private_data);
    if (!dev || !dev->read) return VFS_ERR_IO;
    return dev->read(buf, len);
}

static ssize_t devfs_write(vfs_file_t *file, const void *buf, size_t len) {
    interface_t* dev = (interface_t*)(file->private_data);
    if (!dev || !dev->write) return VFS_ERR_IO;
    return dev->write(buf, len);
}

static int devfs_ioctl(vfs_file_t *file, int cmd, void *arg) {
    interface_t* dev = (interface_t*)(file->private_data);
    if (!dev || !dev->ioctl) return VFS_ERR_IO;
    return dev->ioctl(cmd, arg);
}

// Функция для open устройства (если нужна)
static int devfs_open(vfs_inode_t *inode, vfs_file_t *file) {
    DEVFS_LOG("Opening device: %s\r\n", inode->name);
    
    // Сохраняем интерфейс устройства в private_data файла
    file->private_data = inode->fs_data;
    
    return VFS_OK;
}

// Публичная функция
int devfs_register_device(const char *name, const interface_t *dev_interface) {
    DEVFS_LOG("Registering device: %s\r\n", name);
    
    // Находим каталог /dev
    vfs_inode_t *dev_dir = vfs_lookup("/dev");
    if (!dev_dir) {
        DEVFS_LOG("ERROR: /dev directory not found\r\n");
        return VFS_ERR_NOT_FOUND;
    }
    
    // Создаем inode для устройства
    vfs_inode_t *dev_inode = vfs_inode_create(VFS_TYPE_DEVICE, name);
    if (!dev_inode) {
        DEVFS_LOG("ERROR: Failed to create inode\r\n");
        return VFS_ERR_NO_MEM;
    }
    
    // Сохраняем интерфейс устройства в fs_data
    dev_inode->fs_data = (void*)dev_interface;
    
    // Устанавливаем операции устройства
    dev_inode->ops.open = devfs_open;
    dev_inode->ops.read = devfs_read;
    dev_inode->ops.write = devfs_write;
    dev_inode->ops.ioctl = devfs_ioctl;
    
    // Привязываем устройство в каталог /dev
    int result = vfs_inode_link(dev_dir, dev_inode);
    if (result != VFS_OK) {
        free(dev_inode->name);
        free(dev_inode);
        DEVFS_LOG("ERROR: Failed to link inode\r\n");
        return result;
    }
    
    DEVFS_LOG("Device registered successfully\r\n");
    return VFS_OK;
}