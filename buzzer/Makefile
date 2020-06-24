KERNEL_VER := $(shell uname -r)
KERNEL_DIR = '/lib/modules/$(KERNEL_VER)/build'

obj-m := buzzer_dev.o

PWD := $(shell pwd)

all :
	make -C $(KERNEL_DIR) M=$(PWD) modules

clean :
	make -C $(KERNEL_DIR) M=$(PWD) modules

in :
	sudo insmod buzzer_dev.ko
	sudo mknod -m 666 /dev/buzzer c 501 0

out : 
	sudo rmmod buzzer_dev

in2 :
	sudo insmod buzzer_dev.ko
