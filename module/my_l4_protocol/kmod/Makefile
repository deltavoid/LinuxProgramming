

MODULE := myprot
obj-m += $(MODULE).o
CFLAGS_$(MODULE).o := -DDEBUG -I$(src)
KERNDIR	:= /lib/modules/$(shell uname -r)/build


.PHONY: insmod rmmod build clean
default: insmod

build: $(MODULE).ko
$(MODULE).ko: $(MODULE).c $(MODULE).h
	$(MAKE) -C $(KERNDIR) M=$(CURDIR) modules

clean:
	rm -f *.ko *.o  *.mod *.mod.o *.mod.c *.order *.symvers .$(MODULE).*.cmd

insmod: rmmod build
	sudo insmod $(MODULE).ko

rmmod:
	-sudo rmmod $(MODULE)
