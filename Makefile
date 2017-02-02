# Disable built-in rules
.SUFFIXES:

.PHONY: build clean qemu qemu-debug gdb-attach

QEMU_CMD = qemu-system-arm -M versatilepb -cpu arm1176 -nographic -kernel kernel.elf

qemu: build
	$(QEMU_CMD)

qemu-debug: build
	$(QEMU_CMD) -s -S

gdb-attach:
	arm-none-eabi-gdb -ex 'target remote localhost:1234' -ex 'symbol-file kernel.elf' -tui

CC=arm-linux-gnueabi-gcc
CFLAGS_ARCH=-std=c99 -march=armv6k -msoft-float -fPIC -mapcs-frame -marm -fno-stack-protector -ggdb
CFLAGS_ERRORS=-pedantic -Wall -Wextra -Werror
CFLAGS+=$(CFLAGS_ARCH) $(CFLAGS_ERRORS)
GCC_LIBS=/usr/lib/gcc-cross/arm-linux-gnueabi/5
LD=arm-linux-gnueabi-ld
LDFLAGS=-N -Ttext=0x10000

SOURCES.c := $(wildcard *.c)
SOURCES.h := $(wildcard *.h)
SOURCES.S := $(wildcard *.S)
OUT_DIR=target
OBJECTS := $(SOURCES.c:.c=.o)
OBJECTS += $(SOURCES.S:.S=.o)
OBJECTS += $(OUT_DIR)/FreeRTOS_heap_4.o

clean:
	rm -f *.o *.elf *.s *.d TAGS
	rm -rf $(OUT_DIR)

build: $(OUT_DIR) kernel.elf TAGS

TAGS: $(SOURCES.c) $(SOURCES.h) $(SOURCES.S)
	etags --declarations -o TAGS *.c *.h *.S

kernel.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ $(GCC_LIBS)/libgcc.a

%.o: %.s Makefile
	$(CC) $(CFLAGS) -o $@ -c $<

%.o: %.c Makefile
	$(CC) $(CFLAGS) -o $@ -c $<

%.s: %.S Makefile
	$(CC) -E -o $@ -c $<

%.d: %.c Makefile
	set -e; rm -f $@; \
	$(CC) -E -MM -MP $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%.d: %.S Makefile
	set -e; rm -f $@; \
	$(CC) -E -MM -MP $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(OUT_DIR)/FreeRTOS_heap_4.o: third_party/FreeRTOS/heap_4.c Makefile
	$(CC) $(CFLAGS) -o $@ -c $<

$(OUT_DIR):
	mkdir -p $(OUT_DIR)

 # Include dependency files
-include $(SOURCES.c:.c=.d)
-include $(SOURCES.S:.S=.d)
