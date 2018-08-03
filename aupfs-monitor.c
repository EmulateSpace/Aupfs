#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "monitor.h"
#include "aupfs.h"


/*
 *
 * +--------+            +----------+             +----------+
 * |       -|----------->|         -|------------>|          |
 * |  FIFO  |            |  Buffer  |             |  24CL64  |
 * |        |            |          |<------------|-         |
 * +--------+            +----------+             +----------+
 *
 */

/*
 * Write operation
 */
static int cmd_write(const char *message)
{
    /* write data into AUPFS */
    aupfs_write(message);
}

/*
 * Read Operation
 */
static int cmd_read(int nr)
{
    char *buffer;
    int left, right, i;
    int total = 0;
    struct aupfs_head *head;

    buffer = malloc(ALIGN(aupfs_buffer_count(nr)));
    if (!buffer) {
        printf("Unable to allocate memory on cmd_read()\n");
        return -1;
    }

    /* Read data from AUPFS */
    aupfs_read(buffer, nr);

    printf("\n*********************************\n");
    printf("Log Message:\n");
    if (current - nr < 0) {
        left = nr - current;
        right = current;
    } else {
        left = 0;
        right = nr;
    }
    /* Parse message */
    for (i = 0; i < right; i++) {
        char *tstr;
        int read = 0;
        int length = 0;
        char buft[32];
        char *tmpbuf;

        head = &aupfs_header[current - 1 - i];

        /* OVER: Reovlay */
        if (total  > AUPFS_DATA_VOLUME) {
            total = AUPFS_DATA_VOLUME;
            goto out;
        }
        if ((total + head->len) > AUPFS_DATA_VOLUME)
            read = AUPFS_DATA_VOLUME - total;
        else
            read = head->len;

        /* Establish time */
        tstr = ctime((time_t)&head->time);
        strcpy(buft, tstr);
        buft[strlen(tstr) - 1] = '\0';

        tmpbuf = (char *)malloc(read + 1);
        strncpy(tmpbuf, buffer + total, read);
        tmpbuf[read] = '\0';
        /* How to deal with dump -> file or stdout */
        printf("%s: %s\n", buft, tmpbuf);

        free(tmpbuf);
        free(tstr);
        /* Next offset */
        total += head->len;
    }
    if (left) {
        for (i = 0; i < left; i++) {
            char *tstr;
            int read = 0;
            int length = 0;
            char buft[32];
            char *tmpbuf;

            head = &aupfs_header[AUPFS_HEAD_COUNT - 1 - i];
            /* OVER: Reovlay */
            if (total  > AUPFS_DATA_VOLUME) {
                total = AUPFS_DATA_VOLUME;
                goto out;
            }
            if ((total + head->len) > AUPFS_DATA_VOLUME)
                read = AUPFS_DATA_VOLUME - total;
            else
                read = head->len;

            /* Establish time */
            tstr = ctime((time_t)&head->time);
            strcpy(buft, tstr);
            buft[strlen(tstr) - 1] = '\0';

            tmpbuf = (char *)malloc(read + 1);
            strncpy(tmpbuf, buffer + total, read);
            tmpbuf[read] = '\0';
            /* How to deal with dump -> file or stdout */
            printf("%s: %s\n", buft, tmpbuf);

            free(tmpbuf);
            free(tstr);
            /* Next offset */
            total += head->len;
        }
    }

out:
    printf("*********************************\n");
    free(buffer);
    return 0;
}

/*
 * parse cmd
 */
static int parse_cmd(const char *message, const char *cmd)
{
    if (strcmp(cmd, CMD_WRITE) == 0) {
        /* Write operation */
        cmd_write(message);
        return 0;
    } else if (strcmp(cmd, CMD_READ) == 0) {
        /* READ Operation */
        int nr;

        sscanf(message, "%d", &nr);
        cmd_read(nr);
        return 0;
    } else if (strcmp(cmd, CMD_SET) == 0) {
        /* Set current */
        printf("Setup position\n");
        return 0;
    } else {
        printf("Unknow Comand\n");
        return -1;
    }
}

/*
 * Message Layout:
 *
 *    MSG_HEAD cmd MSG_CMD message MSG_TAIL
 */
int parse_message(const char *message, int nr)
{
    char *head = message;
    char cmd[CMD_BUF_LEN];
    char *buffer;

    buffer = (char *)malloc(((nr + BUFFER_BOUND - 1) / BUFFER_BOUND) *
                          BUFFER_BOUND + BUFFER_BOUND);
    if (!buffer) {
        printf("No free memory for buffer\n");
        return -1;
    }

    while (head != NULL) {
        char *msg_head = strstr(head, MSG_HEAD);

        if (msg_head) {
            char *msg_cmd, *msg_tail;
            int len;

            msg_cmd  = strstr(msg_head, MSG_CMD);
            msg_tail = strstr(msg_head, MSG_TAIL);
            if (msg_cmd == NULL || msg_tail == NULL) {
                printf("Unknow-string for parsing\n");
                return -1;
            }

            /* Obtain cmdline */
            len = msg_cmd - msg_head - strlen(MSG_HEAD);
            strncpy(cmd, msg_head + strlen(MSG_HEAD), len);
            cmd[len] = '\0';

            /* Obtian message */
            len = msg_tail - msg_cmd - strlen(MSG_CMD);
            strncpy(buffer, msg_cmd + strlen(MSG_CMD), len);
            buffer[len] = '\0';           

            /* Parse cmd */
            parse_cmd(buffer, cmd);

            msg_head += strlen(MSG_HEAD);
        }
        head = msg_head; 
    }
    free(buffer);
}
