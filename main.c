#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "i2c.h"
#include "aupfs.h"
#include "monitor.h"
#include "fifo.h"

int main()
{
    daemon(1, 1);
    get_super();

    fifo_read();

    put_super();

    return 0;
}
