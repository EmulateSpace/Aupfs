#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "aupfs.h"


/*
 *
 * +---------+       +--------+       +--------+
 * |        -|------>|       -|------>|        |
 * |  FIFO   |       | Buffer |       | 24CL64 |
 * |         |<------|-       |<------|-       |
 * +---------+       +--------+       +--------+
 *
 *
 * Message layout
 *
 * +------+---------------------------------------------+
 * |      |                                             |
 * | time | Message                                     |
 * |      |                                             |
 * +------+---------------------------------------------+
 */

/* Current point */
unsigned int current;
/* aupfs_head table */
struct aupfs_head *aupfs_header;
/* aupfs super block */
struct aupfs_sb aupfs_sb = {
    .magic = AUPFS_MAGIC,
    .head_nr = AUPFS_HEAD_COUNT,
};

/*
 * Format aupfs via i2c
 */
static int format_aupfs_all(int i2c_fd)
{
    int i;
    int err;
    char buf[2];

    memset(buf, 0, 2);

    /* Format AUPFS */
    for (i = 0; i < AUPFS_VOLUME; i += 1024) {
        int j;

        for (j = 0; j < 1024; j++) {
            err = aup_write_common(i2c_fd, AUPFS_I2C_ADDR, i + j,  buf, 1);
            if (err < 0) {
                printf("Faild to Format block\n");
                return -1;
            }
        }
        printf("Initilize %d 1024-block on AUPFS.....\n", i / 1024);
    }
    /* Setup aupfs superblock */
    err = aup_write_common(i2c_fd, AUPFS_I2C_ADDR, AUPFS_SUPER0,
                             (char *)&aupfs_sb, AUPFS_SUPER_LEN);
    if (err < 0) {
        printf("Unable to write MAGIC\n");
        return -1;
    }
    /* Backup aupfs superblock */
    err = aup_write_common(i2c_fd, AUPFS_I2C_ADDR, AUPFS_SUPER1,
                             (char *)&aupfs_sb, AUPFS_SUPER_LEN);
    if (err < 0) {
        printf("Unable to write MAGIC\n");
        return -1;
    }
    printf("Setup Aupfs Magic: %#x\n", aupfs_sb.magic);
    return 0;
}

/*
 * Export function to format aupfs.
 */
int format_aupfs(void)
{
    int i2c_fd;

    /* Open I2C bus */
    i2c_fd = open_i2c(AUPFS_I2C_BUS);
    if (i2c_fd < 0) {
        printf("Unable open i2c bus\n");
        return -1;
    }

    format_aupfs_all(i2c_fd);
    close_i2c(i2c_fd);
    return 0;
}

/*
 * check_magic
 *  check the aupfs magic.
 */
static int check_magic(int i2c_fd)
{
    int err;
    struct aupfs_sb sb;

    err = aup_read_common(i2c_fd, AUPFS_I2C_ADDR, AUPFS_SUPER0,
                    (unsigned char *)&sb, AUPFS_SUPER_LEN);
    if (err < 0) {
        printf("Faild to get superblock\n");
        return -1;
    }
    if (sb.magic != AUPFS_MAGIC) {
        /* ?superbloc0 is bad block */
        err = aup_read_common(i2c_fd, AUPFS_I2C_ADDR, AUPFS_SUPER1,
                    (unsigned char *)&sb, AUPFS_SUPER_LEN);
        if (err < 0) {
            printf("Faild to get superblock2\n");
            return -1;
        }
        if (sb.magic != AUPFS_MAGIC) {
            printf("AUPFS disk has bad\n");
            return -1;
        } else
            return 0;
    } else
        return 0;
}

/*
 * load_aupfs_header_table
 *   Load aupfs into memory.
 */
static int load_aupfs_header_table(int i2c_fd)
{
    int i;
    unsigned int tmp = 0;
    int err;

    if (!aupfs_header) {
        printf("Error: Bad aupfs_header table\n");
        return -1;
    }

    current = 0;
    for (i = 0; i < AUPFS_HEAD_COUNT; i++) { 
        err = aup_read_common(i2c_fd, AUPFS_I2C_ADDR, ID_TO_PHY(i), 
                  (unsigned char *)&aupfs_header[i], 
                  sizeof(struct aupfs_head));
        if (err < 0) {
            printf("Faild to obtain aupfs_head table[%d]\n", i);
            return -1;
        }
        if (aupfs_header[i].time > tmp) {
            tmp = aupfs_header[i].time;
            current = i + 1;
        }
    }
    return 0;
}

/*
 * get current
 */
int get_super(void)
{
    int i2c_fd;
  
    /* Establish msg_buffer and hold on Memory */
    aupfs_header = (struct aupfs_head *)malloc(sizeof(struct aupfs_head) 
                                      * AUPFS_HEAD_COUNT);
    if (!aupfs_header) {
        printf("Unable to loaded msg-head table\n");
        return -1;
    }

    /* Open I2C bus */
    i2c_fd = open_i2c(AUPFS_I2C_BUS);
    if (i2c_fd < 0) {
        printf("Unable to open i2c bus\n");
        return -1;
    }
    
    if (check_magic(i2c_fd) != 0) {
        /* Format aupfs */
        format_aupfs_all(i2c_fd);
    } else {
        /* Load aupfs_head table */
        load_aupfs_header_table(i2c_fd);
    }
    close_i2c(i2c_fd);
    return 0;
}

int put_super(void)
{
    if (aupfs_header)
        free(aupfs_header);
}

int aupfs_buffer_count(int nr)
{
    int left, right, i;
    int total = 0;
    struct aupfs_head *head;

    if (current - nr < 0) {
        left = nr - current;
        right = current;
    } else {
        left = 0;
        right = nr;
    }
    for (i = 0; i < right; i++) {
        head = &aupfs_header[current - 1 - i];
        total += head->len;

        if (total > AUPFS_DATA_VOLUME) {
            total = AUPFS_DATA_VOLUME;
            return total;
        }
    }
    if (left) {
        for (i = 0; i < left; i++) {
            head = &aupfs_header[AUPFS_HEAD_COUNT - 1 - i];
            total += head->len;

            if (total > AUPFS_DATA_VOLUME) {
                total = AUPFS_DATA_VOLUME;
                return total;
            }
        }
    }
    return total;
}


static int aupfs_data_write(int i2c_fd,
                struct aupfs_head *head, const char *message)
{
    int err;

    if ((head->offset + head->len) > (AUPFS_DATA_VOLUME)) {
        unsigned len0, len1;

        len0 = AUPFS_DATA_VOLUME - head->offset;
        len1 = (head->offset + head->len) - AUPFS_DATA_VOLUME;

        /* Write last tail */
        err = aup_write_common(i2c_fd, AUPFS_I2C_ADDR,
                   VIRT_TO_PHY(head->offset), message, len0);
        if (err < 0) {
            printf("Unable to write data to left0\n");
            return -1;
        }
        /* Write head */
        err = aup_write_common(i2c_fd, AUPFS_I2C_ADDR,
                   VIRT_TO_PHY(0), message + len0, len1);
        if (err < 0) {
            printf("Unable to write data\n");
            return -1;
        }
    } else {
        /* Write last tail */
        err = aup_write_common(i2c_fd, AUPFS_I2C_ADDR,
                    VIRT_TO_PHY(head->offset), message, head->len);
        if (err < 0) {
            printf("Unable to write data\n");
            return -1;
        }
    }
    return 0;
}

/*
 * aupfs_write()
 *  Write operation
 */
int aupfs_write(char *message)
{
    int err, i2c_fd;
    int preid;
    struct aupfs_head *head, *pre_head;
    struct timespec ts;

    i2c_fd = open_i2c(AUPFS_I2C_BUS);
    if (i2c_fd < 0) {
        printf("Unable to open i2c bus\n");
        return -1;
    }

    if (current == 0) {
        preid = AUPFS_HEAD_COUNT - 1;
    } else {
        preid = current - 1;
    }

    pre_head = &aupfs_header[preid];
    head = &aupfs_header[current];

    /* First read */
    if (pre_head->offset == 0 && pre_head->len == 0 &&
                                    pre_head->time == 0) {
        head->offset = 0;
    } else {
        /* setup current */
        head->offset = (pre_head->offset + pre_head->len) % 
                              AUPFS_DATA_VOLUME;
    }
    head->len = strlen(message);

    /* write into data */
    aupfs_data_write(i2c_fd, head, message);

    /* sync aupfs_head */

    /* Format time */
    clock_gettime(CLOCK_REALTIME, &ts);
    head->time = ts.tv_sec;
    err = aup_write_common(i2c_fd, AUPFS_I2C_ADDR,
                    ID_TO_PHY(current), (char *)head, AUPFS_HEAD_LEN);
    if (err < 0) {
        printf("Unable to write data\n");
        return -1;
    }

    current++;
    if (current >= AUPFS_HEAD_COUNT)
        current = 0;

    close_i2c(i2c_fd);
}

static int aupfs_data_read(int i2c_fd, char *message, int *offset,
           struct aupfs_head *tmp)
{
    int err;

    /* Over data zone */
    if ((tmp->offset + tmp->len) > AUPFS_DATA_VOLUME) {
        /* Read tail of message */
        err =  aup_read_common(i2c_fd, AUPFS_I2C_ADDR,
                    VIRT_TO_PHY(tmp->offset),
                         (unsigned char *)message + *offset,
                             AUPFS_DATA_VOLUME - tmp->offset);
        if (err < 0) {
            printf("Unable to read data from AUPFS\n");
            return -1;
        }
        *offset += AUPFS_DATA_VOLUME - tmp->offset;

        /* Read head of message */
        err = aup_read_common(i2c_fd, AUPFS_I2C_ADDR,
                      VIRT_TO_PHY(0),
                      (unsigned char *)message + *offset,
                        tmp->offset + tmp->len - AUPFS_DATA_VOLUME);
        if (err < 0) {
            printf("Unable to read data2 from AUPFS\n");
            return -1;
        }
        *offset += tmp->offset + tmp->len - AUPFS_DATA_VOLUME;
    } else {
        err =  aup_read_common(i2c_fd, AUPFS_I2C_ADDR,
                 VIRT_TO_PHY(tmp->offset),
                    (unsigned char *)message + *offset,
                        tmp->len);
        if (err < 0) {
            printf("Unable to read data3 from AUPFS\n");
            return -1;
        }
        *offset += tmp->len;
    }
    return 0;
}

/*
 * Read a message from 24CL64
 */
int aupfs_read(char *message, int nr)
{
    int err, i;
    int i2c_fd;
    int left, right;
    int safe_addr, offset = 0;
    struct aupfs_head *head, *tmp;

    i2c_fd = open_i2c(AUPFS_I2C_BUS);
    if (i2c_fd < 0) {
        printf("Unable open i2c bus\n");
        return -1;
    }

    /*
     *
     *  FIFO
     *
     *  +-------------------0-------------------------+
     *  |                   |                         | 
     *  |                   |                         |
     *  |                   |                         |
     *  +-------------------+-------------------------+
     *                      A
     *                      |
     *      current---------o
     *
     * The current points to first item. So first valid aupfs_head locates
     * on tail of FIFO.
     *
     * The ID of tail of FIFO is : MSG_HEAD_COUNT - 1
     */
    if (current == 0) {
        head = &aupfs_header[AUPFS_HEAD_COUNT - 1];
    } else
        head = &aupfs_header[current - 1];

    /*
     *  FIFO
     *
     *  +----+--------------0-----------+-------------+
     *  |    |              |           |             | 
     *  |    |              |           |             |
     *  |    |              |           |             |
     *  +----+--------------+-----------+-------------+
     *       |<-nr-current->|<-current->|
     *                      
     *      
     *  0----------+--------------------+-------------+
     *  |          |                    |             |                   
     *  |          |                    |             |                   
     *  |          |                    |             |                   
     *  0----------+--------------------+-------------+
     *             | <- (current-nr) -> | 
     *                                  A
     *                                  |
     *                                  |
     *              current-------------o
     *
     *
     */
    if (current - nr < 0) {
        left  = nr - current;
        right = current;
    } else {
        left = 0;
        right = nr;
    }

    /* Ideficate safe address */
    safe_addr = 0;

    /* Read right */
    for (i = 0; i < right; i++) {
        tmp = &aupfs_header[current - 1 - i];
        if ((safe_addr + tmp->len) > AUPFS_DATA_VOLUME) {
            strncpy(message + offset, "AUPFSOUT", 
                          (AUPFS_DATA_VOLUME - safe_addr));
            /* Invalid data */
            break;
        } else
            safe_addr += tmp->len;

        /* Write data */
        aupfs_data_read(i2c_fd, message, &offset, tmp);
    }

    /* Read left */
    if (left) {
        for (i = 0; i < left; i++) {
            tmp = &aupfs_header[AUPFS_HEAD_COUNT - 1 - i];
            if ((safe_addr + tmp->len) > AUPFS_DATA_VOLUME) {
                strncpy(message + offset, "AUPFSOUT", 
                           (AUPFS_DATA_VOLUME - safe_addr));
                /* Invalid data */
                break;
            } else
                safe_addr += tmp->len;
            /* Data Read */
            aupfs_data_read(i2c_fd, message, &offset, tmp);
        }
    }

    close_i2c(i2c_fd);
    return 0;
}
