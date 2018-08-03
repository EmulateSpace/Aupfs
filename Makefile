
####
## System Configure
SYS_CONFIG +=

### Support I2C dev
SYS_CONFIG += -DCONFIG_I2C_DEV
#   Support 24CL64
SYS_CONFIG += -DCONFIG_I2C_24CL64

### Support Block dev
# SYS_CONFIG += -DCONFIG_BLOCK_DEV 

### Cross Compile 
CONFIG_CROSS_COMPILE="/xspace/AuperaStor/MainPro/Tina2/prebuilt/gcc/linux-x86/arm/toolchain-sunxi/toolchain/bin/arm-openwrt-linux-muslgnueabi-"
CONFIG_STAGING_DIR="/xspace/AuperaStor/MainPro/Tina2/out/octopus-MGT2600/staging_dir"
export CONFIG_STAGING_DIR


CFLAGS += $(SYS_CONFIG)

CC=$(CONFIG_CROSS_COMPILE)gcc

MYCFLAGS= -I./

OBJ= main.o i2c.o aupfs.o monitor.o fifo.o

all: aupfs aupfs-log

aupfs: $(OBJ)
	$(CC) $^ $(CFLAGS) -o $@

aupfs-log: log.o 
	$(CC) $^ $(CFLAGS) -o $@

%.o: %.c
	$(CC) -c $(MYCFLAGS) $^ -o $@

$(OBJ):

clean:
	rm aupfs aupfs-log *.o
