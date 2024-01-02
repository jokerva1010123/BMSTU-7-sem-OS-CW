CURRENT = $(shell uname -r)
KDIR = /lib/modules/$(CURRENT)/build
PWD = $(shell pwd)
TARGET = src
obj-m := $(TARGET).o

default :
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean :
	@rm -f *.o .led.o.d .*.cmd .*.flags *.mod.c *.order Module.*
	@rm -f .*.*.cmd *~ *.*~ TODO.*
	@rm -fR .tmp* *.ko *.mod
	@rm -rf .tmp_versions

disclean : clean
	@rm *.ko *.mod