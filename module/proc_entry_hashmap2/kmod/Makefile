

MODULE := proc_entry_hashmap2
obj-m += $(MODULE).o
CFLAGS_$(MODULE).o := -DDEBUG
KERNDIR	:= /lib/modules/$(shell uname -r)/build


.PHONY: insmod rmmod build clean
default: insmod

build: $(MODULE).ko
$(MODULE).ko: $(MODULE).c
	$(MAKE) -C $(KERNDIR) M=$(CURDIR) modules

clean:
#	$(MAKE) -C $(KERNDIR) M=$(CURDIR) modules clean
	-rm  *.o *.mod.o *.mod.c modules.order Module.symvers

insmod: rmmod build
	sudo insmod $(MODULE).ko

rmmod:
	-sudo rmmod $(MODULE)
