#ifndef VFS_DEVFS_H
#define VFS_DEVFS_H

#include "dev_interface.h"
#include "vfs_core.h"

// DevFS операции (только публичные)
int devfs_register_device(const char *name, const interface_t *dev_interface);
// Функции чтения/записи используются через указатели в ops

#endif /* VFS_DEVFS_H */