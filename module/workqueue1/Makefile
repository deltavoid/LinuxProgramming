

MODULE := workqueue1
obj-m += $(MODULE).o
#CFLAGS_$(MODULE).o := -DDEBUG
KERNDIR	:= /lib/modules/$(shell uname -r)/build


.PHONY: insmod rmmod clean
default: insmod

$(MODULE).ko: $(MODULE).c
	$(MAKE) -C $(KERNDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KERNDIR) M=$(CURDIR) modules clean

insmod: rmmod $(MODULE).ko
	sudo insmod $(MODULE).ko

rmmod:
	-sudo rmmod $(MODULE)
