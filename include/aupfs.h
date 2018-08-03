#ifndef _MESSAGE_H
#define _MESSAGE_H

#include <aupfs-dev.h>

struct aupfs_head {
    unsigned int time;
    unsigned short len;
    unsigned short offset;
};

struct aupfs_sb {
    unsigned short magic;
    unsigned short head_nr;
};

/* Custom configure area */
#define AUPFS_HEAD_COUNT      64

/* Don't edit area */

/* MAGIC for AUPFS */
#define AUPFS_MAGIC           0x1016
#define AUPFS_I2C_BUS         "i2c0"
#define AUPFS_I2C_ADDR        0x50
#define AUPFS_VOLUME          (8 * 1024)
#define AUPFS_BLOCK_SIZE      1024
#define AUPFS_BLOCK_COUNT     (AUPFS_VOLUME / AUPFS_BLOCK_SIZE)

/*
 * 0------------+------------+------------+------+-----------+------+
 * |            |            |            |      |           |      |
 * | Superblock | head table | Superblock | hole | data zone | hole |
 * |            |            |            |      |           |      |
 * +------------+------------+------------+------+-----------+------+
 */

/* Alignment setup */
#define AUPFS_ALIGN           (4)
#define ALIGN(addr)           (((addr + AUPFS_ALIGN - 1) / AUPFS_ALIGN) * \
                                                           AUPFS_ALIGN)

/* AUPFS msg_header */
#define AUPFS_HEAD_LEN        (sizeof(struct aupfs_head))
#define AUPFS_HEAD_VOLUME     ALIGN(AUPFS_HEAD_COUNT * AUPFS_HEAD_LEN)


/* AUPFS Superblock */
#define AUPFS_SUPER_LEN       (sizeof(struct aupfs_sb))
#define AUPFS_SUPER_VOLUME    ALIGN(AUPFS_SUPER_LEN * 2)
#define AUPFS_SUPER0          ALIGN(0x00)
#define AUPFS_SUPER1          ALIGN(AUPFS_SUPER_LEN + AUPFS_HEAD_VOLUME)

/* AUPFS hole */
#define AUPFS_HOLE_LEN        ALIGN(AUPFS_ALIGN)
#define AUPFS_HOLE_VOLUME     ALIGN(AUPFS_HOLE_LEN * 2)

/* AUPFS Data Zone */
#define AUPFS_DATA_VOLUME     ALIGN(AUPFS_VOLUME - AUPFS_SUPER_VOLUME - \
                                       AUPFS_HEAD_VOLUME - AUPFS_HOLE_VOLUME)
#define AUPFS_DATA_START      ALIGN(AUPFS_SUPER_VOLUME + AUPFS_HEAD_VOLUME + \
                                       AUPFS_HOLE_LEN)
#define AUPFS_DATA_END        ALIGN(AUPFS_DATA_START + AUPFS_DATA_VOLUME)
 
/* Cover msg_header address */
#define ID_TO_PHY(id)         ALIGN((id) * AUPFS_HEAD_LEN + AUPFS_SUPER_LEN)

/* Cover data address */
#define VIRT_TO_PHY(addr)     ALIGN(addr + AUPFS_DATA_START)
#define PHY_TO_VIRT(addr)     ALIGN(addr - AUPFS_DATA_START)

/* Bound safe */
#define AUPFS_DATA_BOUND      ALIGN(AUPFS_DATA_START + AUPFS_DATA_VOLUME)     

extern int get_super(void);
extern int aupfs_write(char *message);
extern int aupfs_read(char *message, int nr);
extern int format_aupfs(void);

/* Current point */
extern unsigned int current;
/* aupfs_head table */
extern struct aupfs_head *aupfs_header;
/* Calculate buffer count */
extern int aupfs_buffer_count(int nr);

#define MESSAGE_HOLE 0
#define MESSAGE_LEN  0
#endif
