export SHELL=/bin/bash

#
# setup ARCH specific flags and variables
#
PROG_PREFIX=arm-elf-
ASFLAGS += 
CFLAGS += -mcpu=arm9 -mlittle-endian -Os -fomit-frame-pointer

#
# variables used by the implicit make rules
#
AR=$(PROG_PREFIX)ar
AS=$(PROG_PREFIX)as
CC=$(PROG_PREFIX)gcc
CPP=$(PROG_PREFIX)cpp
CXX=$(PROG_PREFIX)g++
LD=$(PROG_PREFIX)ld
OBJCOPY=$(PROG_PREFIX)objcopy
OBJDUMP=$(PROG_PREFIX)objdump
STRIP=$(PROG_PREFIX)strip

DEFINES +=
INCLUDES += -I.
ASFLAGS += -D_ASM_ $(INCLUDES) $(DEFINES)
CFLAGS += -Wall $(INCLUDES) $(DEFINES) -fno-builtin
LFLAGS = $(DEFINES) $(INCLUDES) -preproc
GCCLIB = -lgcc

.SUFFIXES:
.SUFFIXES: .a .o .c .cc .y .s .S .h

#
# use gmake's implicit rules for now, except this one:
#
%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

%.s : %.c
	$(CC) -S $(CFLAGS) $(CPPFLAGS) $< -o $@

all::	boot.elf boot.raw

boot.elf:
	$(CC) -g -c $(CFLAGS) $(CPPFLAGS) -DINFLASH=0 init.S -o init.o
	$(CC) -g -c $(CFLAGS) $(CPPFLAGS) -DINFLASH=0 boot.c -o boot.o
	$(CC) -g -c $(CFLAGS) $(CPPFLAGS) BootError.c -o BootError.o
	$(CC) -g -c $(CFLAGS) $(CPPFLAGS) printf.c -o printf.o
	$(CC) -g -nostdlib -nostartfiles -Wl,-Map=boot.map,--cref -N -o boot.elf -T boot.lds -Ttext 0x300000 init.o boot.o BootError.o printf.o $(GCCLIB)

boot.raw:
	$(OBJCOPY) --strip-debug --strip-unneeded boot.elf -O binary boot.raw
	ls -ls boot.raw
	cp boot.raw boot.bin

.PHONY: clean

clean::
	-rm $(OBJS) boot.tmp *.o

distclean::
	-rm $(OBJS) boot.elf boot.raw boot.tmp boot.map depend *~ *.o


