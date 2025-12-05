#include "vfs.h"

ssize_t devfs_read(vfs_file_t *file, void *buf, size_t len) {
    interface_t* dev = (interface_t*)(file->private_data);
    if (!dev || !dev->read) return -1;
    return dev->read(buf, len);
}

ssize_t devfs_write(vfs_file_t *file, const void *buf, size_t len) {
    interface_t* dev = (interface_t*)(file->private_data);
    if (!dev || !dev->write) return -1;
    return dev->write(buf, len);
}

int devfs_ioctl(vfs_file_t *file, int cmd, void *arg) {
    interface_t* dev = (interface_t*)(file->private_data);
    if (!dev || !dev->ioctl) return -1;
    return dev->ioctl(cmd, arg);
}

int devfs_register_device(const char *name, const interface_t *dev_interface) {
    printf("[DevFS] Registering device: %s at %p\r\n", name, dev_interface);
    vfs_inode_t *dev_inode = vfs_create_inode(VFS_TYPE_DEVICE, name);
    if (!dev_inode) {
        printf("[DevFS] ERROR: Failed to create inode\r\n");
        return -1;
    }
    
    // Сохраняем интерфейс устройства
    dev_inode->data = (void*)dev_interface;
    
    // Привязываем операции VFS к драйверу
    dev_inode->ops.open = NULL;  // Можно реализовать позже
    dev_inode->ops.close = NULL;
    dev_inode->ops.read = devfs_read;
    dev_inode->ops.write = devfs_write;
    dev_inode->ops.ioctl = devfs_ioctl;
    
    int result = vfs_link("/dev", dev_inode);
    printf("[DevFS] vfs_link result: %d\r\n", result);
    return result;
}