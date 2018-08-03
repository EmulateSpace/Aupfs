#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <aupfs.h>

/* Aupfs-block open */
static int aupfs_block_open(struct aupfs_dev *dev, 
                            const char *pathname, int flag)
{
    dev->fd = open(pathname, flag);
    if (dev->fd < 0) {
        printf("AUPFS: %s faild to open block\n", dev->name);
        return -EINVAL;
    }

    return 0;
}

/* Aupfs-block close */
static int aupfs_block_close(struct aupfs_dev *dev)
{
    close(dev->fd);
    dev->fd = -1;
    return 0;
}

/* Aupfs-block release */
static int aupfs_block_release(struct aupfs_dev *dev)
{
    dev->open  = NULL;
    dev->close = NULL;
    dev->read  = NULL;
    dev->write = NULL;

    return 0;
}

/* Aupfs-block read */
static int aupfs_block_read(struct aupfs_dev *dev, unsigned char *buf, 
                        unsigned long len, unsigned long offset)
{
    int err;

    err = dev->u.tdev.block_read(dev->fd, 
                              dev->u.bdev.size, offset, buf, &len);
    if (err != len) {
        printf("AUPFS-Block: %s read error on %02x\n", dev->name, err); 
        return -EINVAL;
    }
    return 0;
}

/* Aupfs-block write */
static int aupfs_block_write(struct aupfs_dev *dev, const char *buf, 
                        unsigned long len, unsigned long offset)
{
    int err;

    err = dev->u.tdev.i2c_write(dev->fd, dev->u.bdeb.size, 
                                             offset, buf, &len); 
    if (err != len) {
        printf("AUPFS-Block: %s write error on %02x", dev->name, err);
        return  -1;
    }
    return 0;
}

/* Aupfs-i2c entry */
struct aupfs_dev aupfs_block = {
    .open  = aupfs_block_open,
    .close = aupfs_block_close,
    .read  = aupfs_block_read,
    .write = aupfs_block_write,
    .release = aupfs_block_release,
};
