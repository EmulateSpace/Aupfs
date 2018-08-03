#ifndef _AUPFS_DEV_H
#define _AUPFS_DEV_H

struct aupfs_i2c_dev {
    unsigned char bus;
    unsigned char slave;
    int (*i2c_read)(int fd, unsigned char addr, unsigned long offset,
                         unsigned char *buf, unsigned long len);
    int (*i2c_write)(int fd, unsigned char addr, unsigned long offset,
                         const char *buf, unsigned long len);
};

struct aupfs_block_dev {
    unsigned long size;
    int (*block_read)(int fd, unsigned long size, unsigned long offset,
                         unsigned char *buf, unsigned long len);
    int (*block_write)(int fd, unsigned long size, unsigned long offset,
                         const char *buf, unsigned long len);
};

struct aupfs_dev {
    int fd;
    char *name;
    int (*open)(struct aupfs_dev *dev, const char *pathname, int flag);
    int (*close)(struct aupfs_dev *dev);
    int (*release)(struct aupfs_dev *dev);
    int (*read)(struct aupfs_dev *dev, unsigned char *buf, 
                        unsigned long len, unsigned long offset);
    int (*write)(struct aupfs_dev *dev, const char *buf, 
                        unsigned long len, unsigned long offset);
    union {
        struct aupfs_i2c_dev tdev;
        struct aupfs_block_dev bdev;
    } u;
};

#endif
