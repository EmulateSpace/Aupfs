#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <i2c.h>

/*
 * I2C read
 * @fd: file handler
 * @addr: i2c slave 7-bit address
 * @offset: read position.
 * @buf: buffer for reading data.
 * @len: length for reading.
 *
 * @return: the number of i2c_msg has read. 
 *          succeed is 2.
 */
static int aupfs_24cl64_packetRead(int fd, unsigned char addr, 
      unsigned long offset, unsigned char *buf, unsigned long len)
{
    unsigned char tmpaddr[2];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset;
    int rc;

    tmpaddr[0]     = (offset >> 8) & 0xff;
    tmpaddr[1]     = offset & 0xff;
    msgs[0].addr   = addr & 0xfe;
    msgs[0].flags  = I2C_M_WR;
    msgs[0].len    = 2;
    msgs[0].buf    = tmpaddr;

    msgs[1].addr   = addr & 0xfe;
    msgs[1].flags  = I2C_M_RD;  ;
    msgs[1].len    = len;
    msgs[1].buf    = buf;

    msgset.msgs    = msgs;
    msgset.nmsgs   = 2;

    rc = ioctl(fd, I2C_RDWR, &msgset);
    return rc;
}

/* 
 * I2C write
 * @fd: file handler.
 * @addr: i2c slave 7-bit address
 * @offset: write position
 * @buf: buffer for writuing data.
 * @len: the length for writing
 *
 * @return: the number of i2c_msg has write.
 *          succeed is 1.
 */
static int aupfs_24cl64_packetWrite(int fd, unsigned char addr, 
       unsigned long offset, unsigned char *buf, unsigned long len)
{
    unsigned char tmpaddr[3];
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data msgset;
    int rc;

    tmpaddr[0]     = (offset >> 8) & 0xff;
    tmpaddr[1]     = offset & 0xff;
    tmpaddr[2]     = buf[0];
    msgs[0].addr   = addr & 0xfe;
    msgs[0].flags  = I2C_M_WR;
    msgs[0].len    = 3;
    msgs[0].buf    = tmpaddr;

    msgset.msgs    = msgs;
    msgset.nmsgs   = 1;

    rc = ioctl(fd, I2C_RDWR, &msgset);
    return rc;
}
