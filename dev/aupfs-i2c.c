#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <aupfs.h>

/* Aupfs-i2c open */
static int aupfs_i2c_open(struct aupfs_dev *dev, 
                            const char *pathname, int flag)
{
    int retry = 6;
    char *name;

retry:
    if (ap->u.tdev.bus == 0)
        name = "/dev/i2c-0";
    else if (ap->u.tdev.bus == 1)
        name = "/dev/i2c-1";
    else if (ap->u.tdev.bus == 2)
        name = "/dev/i2c-2";
    else {
        printf("AUPFS: %s invalide on bus %d\n", dev->name, 
                                                 dev->u.tdev.bus);
        return -EINVAL;
    }

    dev->fd = open(name, flag);
    if (dev->fd < 0) {
        if (--retry == 0) {
            printf("AUPFS: %s retry %d faild to open I2C%d\n",
                  dev->name, retry, dev->u.tdev.bus);
            return -EBUSY;
        }
        sleep(2);
        goto retry;
    }

    return 0;
}

/* Aupfs-i2c close */
static int aupfs_i2c_close(struct aupfs_dev *dev)
{
    close(dev->fd);
    dev->fd = -1;
    return 0;
}

/* Aupfs-i2c release */
static int aupfs_i2c_release(struct aupfs_dev *dev)
{
    dev->open = NULL;
    dev->close = NULL;
    dev->read = NULL;
    dev->write = NULL;

    return 0;
}

/* Aupfs-i2c read */
static int aupfs_i2c_read(struct aupfs_dev *dev, unsigned char *buf, 
                        unsigned long len, unsigned long offset)
{
    int err;
    int i;

    for (i = 0; i < len; i++) {
        int local_offset = offset + i;

        err = dev->u.tdev.i2c_read(dev->fd, dev->u.tdev.slave, 
                                      local_offset, &buf[i], 1);
        if (err != 2) {
            printf("AUPFS-I2C: %s read error on %02x\n", dev->name, 
                                              local_offset);
            return -EINVAL;
        }
    }
    return 0;
}

/* Aupfs-i2c write */
static int aupfs_i2c_write(struct aupfs_dev *dev, const char *buf, 
                        unsigned long len, unsigned long offset)
{
    int err;
    int i;

    for (i = 0; i < len; i++) {
        int local_offset = offset + i;

        err = dev->u.tdev.i2c_write(dev->fd, dev->u.tdev.slave, 
                                  local_offset, &buf[i], 1);
        usleep(4000);
        if (err != 1) {
            printf("AUPFS-I2C: %s write error on %02x", dev->name, 
                                              local_offset);
            return  -1;
        }
    }
    return 0;
}

/* Aupfs-i2c entry */
struct aupfs_dev aupfs_i2c = {
    .open  = aupfs_i2c_open,
    .close = aupfs_i2c_close,
    .read  = aupfs_i2c_read,
    .write = aupfs_i2c_write,
    .release = aupfs_i2c_release,
};
