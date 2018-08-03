#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "monitor.h"
#include "fifo.h"

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


/*
 * 
 *  
 */
static const char *format_message(const char *message, const char *cmd)
{
    char *buffer;
    int len;
    int offset = 0;

    len = strlen(MSG_HEAD) + strlen(MSG_CMD) + strlen(MSG_TAIL)
             + strlen(cmd) + strlen(message);

    buffer = (char *)malloc(len + 4);

    /* String cat HEAD */
    strncpy(buffer, MSG_HEAD, strlen(MSG_HEAD));
    offset += strlen(MSG_HEAD);
    /* String cat cmd */
    strncpy(buffer + offset, cmd, strlen(cmd));
    offset += strlen(cmd);
    /* String cat MSG_CMD */
    strncpy(buffer + offset, MSG_CMD, strlen(MSG_CMD));
    offset += strlen(MSG_CMD);
    /* String cat Message */
    strncpy(buffer + offset, message, strlen(message));
    offset += strlen(message);
    /* String cat MSG_Tail */
    strncpy(buffer + offset, MSG_TAIL, strlen(MSG_TAIL));
    offset += strlen(MSG_TAIL);
    buffer[offset] = '\0';
    return buffer;
}

static int message_write(const char *message)
{
    char *buffer = format_message(message, CMD_WRITE);

    fifo_write(buffer);

    free(buffer);

    return 0;
}

static int message_read(const char *nr)
{
    char *buffer = format_message(nr, CMD_READ);

    fifo_write(buffer);

    free(buffer);
    return 0;
}

static int message_dump(const char *filepath)
{
    char *buffer = format_message(filepath, CMD_DUMP);

    fifo_write(buffer);

    free(buffer);
    return 0;
}

static int message_reset(void)
{
    char *buffer = format_message(CMD_SET, CMD_SET);

    fifo_write(buffer);

    free(buffer);
    return 0;
}

int main(int argc,char *argv[])
{
    char *option_read  = NULL;
    char *option_write = NULL;
    char *option_dump  = NULL;
    int option_reset = 0;

    int c;
    const char *short_opts = "hcr:w:";
    const struct option long_opts[] = {
        {"help", no_argument, NULL, 'h'},
        {"read", required_argument, NULL, 'r'},
        {"write", required_argument, NULL, 'w'},
        {"dump", required_argument, NULL, 'd'},
        {"reset", no_argument, NULL, 'c'},
        {0,0,0,0}
    };
    opterr = 0;
    while((c = getopt_long(argc, argv, short_opts, long_opts,NULL)) != -1) {
        switch(c) {
        case 'h':
            break;
        case 'w':
            option_write = optarg;
            break;
        case 'r':
            option_read = optarg;
            break;
        case 'd':
            option_dump = optarg;
            break;
        case 'c':
            option_reset = 1;
            break;
        case '?':
            if(optopt == 't' || optopt == 'T')
                ;
            else if(isprint(optopt))
                ;
            else
                ;
            printf("Invalid command\n");
            exit(1);
        default:
            abort();
        }
    }

    if (option_write) {
        /* Write data into fifo */
        message_write(option_write);
    }

    if (option_read) {
        /* Show or dump information */
        message_read(option_read);
    }

    if (option_dump) {
        /* Dump information into file */
        message_dump(option_dump);
    }

    if (option_reset) {
        /* Reset AUPFS */
        message_reset();
    }
    return 0;
}
