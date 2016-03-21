export ARCH:=arm
export CROSS_COMPILE:=arm-linux-gnueabihf-
CFLAGS_hello.o := -DDEBUG
KERNELDIR ?= /home/untitled/linux-bbb

obj-m+=hello.o

all:
	make -C $(KERNELDIR) M=$(PWD) modules
clean:
	make -C $(KERNELDIR) M=$(PWD) clean
