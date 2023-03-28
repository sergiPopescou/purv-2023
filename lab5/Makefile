PWD := $(shell pwd)
obj-m += driver.o

all:
	make ARCH=arm CROSS_COMPILE=$(TOOL_PATH)/$(CROSS_COMPILE) -C $(LINUX_SRC) SUBDIRS=$(PWD) modules
clean:
	rm -rf *.ko *.o *.symvers *.order *.mod.c *.mod.o
