#KERNEL_SRC = /home/takashi/ax335x_work/KERNEL
#KERNEL_SRC = /home/taiki/Desktop/kernel/linux-dev2/linux-dev/KERNEL
KERNEL_SRC = /home/taiki/export/kernel/kernel
BUILD_DIR := $(shell pwd)

CROSS_COMPILE = arm-linux-gnueabi-
ARCH = arm
MAKEARCH = $(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE)
DRV_NAME = oneshot_pulse_dev
CFILES = one_shot_timer.c one_shot_driver.c gpio_func.c
obj-m            := $(DRV_NAME).o
$(DRV_NAME)-objs := $(CFILES:.c=.o)

all: 
	$(MAKEARCH) -C $(KERNEL_SRC) SUBDIRS=$(BUILD_DIR) modules

clean:
	$(MAKEARCH) -C $(KERNEL_SRC) SUBDIRS=$(BUILD_DIR) clean
