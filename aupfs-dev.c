
#ifdef CONFIG_AUPFS_I2C
#include <aupfs-i2c.h>
#endif

int aupfs_dev_probe(struct aupfs_dev *dev)
{
#ifdef CONFIG_AUPFS_I2C
    strncpy((char *)dev, (char *)&aupfs_i2c, sizeof(struct aupfs_dev));
    dev->name = AUPFS_DEV_NAME;
    dev->u.tdev.bus   = AUPFS_I2C_BUS;
    dev->u.tdev.slave = AUPFS_I2C_SLAVE; 
    dev->u.tdev.i2c_read  = AUPFS_I2C_READ;
    dev->u.tdev.i2c_write = AUPFS_I2C_WRITE;
#endif

#ifdef CONFIG_AUPFS_BLOCK
    strncpy((char *)dev, (char *)&aupfs_block, sizeof(struct aupfs_dev));
    dev->name = AUPFS_DEV_NAME;
    dev->u.bdev.size = AUPFS_BLOCK_VOLUME; 
    dev->u.bdev.block_read  = AUPFS_BLOCK_READ;
    dev->u.bdev.block_write = AUPFS_BLOCK_WRITE;
#endif

    return 0;
}

int aupfs_dev_remove(struct aupfs_dev *dev)
{
    dev->fd = -1;
    dev->name = NULL;
    dev->release(dev);
    return 0;
}
