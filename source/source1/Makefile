CONFIG_MODULE_SIG=n

ifneq ($(KERNELRELEASE),)
	obj-m += module_dev.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) modules clean
	rm -rf abc
endif
