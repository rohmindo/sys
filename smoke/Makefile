KERNEL_DIR = /lib/modules/4.19.97-v7+/build

obj-m := smoke_dev.o

PWD := $(shell pwd)

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean
