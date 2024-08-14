obj-m := file_varea.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

CFLAG := -DEXPORT_SYMTAB

all:
	$(MAKE) $(CLFAG) -C $(KDIR) SUBDIRS=$(PWD) modules
	gcc -o test test.c
cleen:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
