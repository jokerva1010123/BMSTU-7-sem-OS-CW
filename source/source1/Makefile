CONFIG_MODULE_SIG=n

ifneq ($(KERNELRELEASE),)
	obj-m += module_dev.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	rm -rf .tmp_versions
	rm .module_dev.*
	rm *.o
	rm *.mod.c
	rm *.symvers
	rm *.order
endif