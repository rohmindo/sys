KERNEL_VER := $(shell uname -r)
KERNEL_DIR = '/lib/modules/$(KERNEL_VER)/build'

obj-m := hum_dev.o

PWD := $(shell pwd)

all :
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean :
	make -C $(KERNEL_DIR) M=$(PWD) clean

in :
	sudo insmod hydro_dev.ko
	sudo mknod -m 666 /dev/hydro c 505 0

out : 
	sudo rmmod hydro_dev

in2 :
	sudo insmod hydro_dev.ko
