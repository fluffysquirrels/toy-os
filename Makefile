.PHONY: build clean qemu qemu-debug gdb-attach

QEMU_CMD = qemu-system-arm -M versatilepb -cpu arm1176 -nographic -kernel kernel.elf

qemu: build
	$(QEMU_CMD)

qemu-debug: build
	$(QEMU_CMD) -s -S

gdb-attach:
	arm-none-eabi-gdb -ex 'target remote localhost:1234' -ex 'symbol-file kernel.elf' -tui

clean:
	rm -f *.o *.elf *.s *.d

CC=arm-linux-gnueabi-gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -Werror -march=armv6k -msoft-float -fPIC -mapcs-frame -marm -fno-stack-protector -g
LD=arm-linux-gnueabi-ld
LDFLAGS=-N -Ttext=0x10000

SOURCES.c := $(wildcard *.c)
SOURCES.S := $(wildcard *.S)
OBJECTS := $(SOURCES.c:.c=.o)
OBJECTS += $(SOURCES.S:.S=.o)

build: kernel.elf

%.elf: %.o
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.s
	$(CC) $(CFLAGS) -o $@ -c $<

# Disable built-in rule to build .o from .S with the C++ compiler.
%.o: %.S

%.d : %.c
	set -e; rm -f $@; \
	$(CC) -E -MM -MP $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d : %.S
	set -e; rm -f $@; \
	$(CC) -E -MM -MP $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

kernel.elf: $(OBJECTS)

-include $(SOURCES.c:.c=.d)
-include $(SOURCES.S:.S=.d)
