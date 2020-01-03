

MODULE := irq_test
obj-m += $(MODULE).o
#CFLAGS_$(MODULE).o := -DDEBUG
KERNDIR	:= /lib/modules/$(shell uname -r)/build


.PHONY: insmod rmmod build clean
default: insmod

build: $(MODULE).ko
$(MODULE).ko: $(MODULE).c
	$(MAKE) -C $(KERNDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KERNDIR) M=$(CURDIR) modules clean

insmod: rmmod build
	sudo insmod $(MODULE).ko irq=19

rmmod:
	-sudo rmmod $(MODULE)
