#ifndef _FIFO_H
#define _FIFO_H

#define FIFO_NAME "/tmp/my_fifo"
#define BUFFER_SIZE PIPE_BUF
#define TEN_MEG (1024 * 1024 * 10)

extern int fifo_read(void);
extern int fifo_write(const char *message);

#endif
