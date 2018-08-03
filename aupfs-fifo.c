#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "fifo.h"
#include "monitor.h"

int fifo_write(const char *message)
{
    int pipe_fd;
    int res;
    int open_mode = O_WRONLY;
    int bytes_sent = 0;

    /* Check buffer exist? if not and create it */
    if (access(FIFO_NAME, F_OK) == -1) {
         /* mkfifo create a fifo */
        res = mkfifo(FIFO_NAME, 0777);
        if (res != 0) {
            printf("Could not create fifo %s\n", FIFO_NAME);
            return -1;
        }
    }

    pipe_fd = open(FIFO_NAME, open_mode);

    if (pipe_fd != -1) {
        while(bytes_sent < strlen(message)) {
            res = write(pipe_fd, message, strlen(message));
            sleep(1);
            if (res == -1) {
                printf("Write error on pipe\n");
                return -1;
            }
            bytes_sent += res;
        }
        close(pipe_fd);
    } else { 
        return -1;
    }
    return 0;
}

int fifo_read(void)
{
    int pipe_fd;
    int res;
    int open_mode = O_RDONLY;
    char buffer[BUFFER_SIZE + 1];
    int bytes_read = 0;

    /* Check buffer exist? if not and create it */
    if (access(FIFO_NAME, F_OK) == -1) {
         /* mkfifo create a fifo */
        res = mkfifo(FIFO_NAME, 0777);
        if (res != 0) {
            printf("Could not create fifo %s\n", FIFO_NAME);
            return -1;
        }
    }

    do {
        memset(buffer, '\0', sizeof(buffer));
        pipe_fd = open(FIFO_NAME, open_mode);
        if (pipe_fd != -1) {
            do {
                res = read(pipe_fd, buffer, BUFFER_SIZE);
                bytes_read += res;
                if (res > 0) {
                    /* Parse message */
                    parse_message(buffer, res);
                }
            } while (res > 0);
            close(pipe_fd);
        } else {
            return -1;
        }
    } while (1);
    return -1;
}
