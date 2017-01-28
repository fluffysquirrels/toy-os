CC=arm-linux-gnueabi-gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -Werror -march=armv6k -msoft-float -fPIC -mapcs-frame -marm -fno-stack-protector -g
LD=arm-linux-gnueabi-ld
LDFLAGS=-N -Ttext=0x10000

.PHONY: build clean qemu qemu-debug gdb-attach

%.elf: %.o
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.s
	$(CC) $(CFLAGS) -o $@ -c $^

# Disable built-in rule to build .o from .S with the C++ compiler.
%.o: %.S

kernel.o: context_switch.h asm_constants.h versatilepb.h kernel.c synchronous_console.h
context_switch.s: context_switch.S asm_constants.h
kernel.elf: bootstrap.o kernel.o context_switch.o syscalls.o synchronous_console.o
synchronous_console.o: synchronous_console.h versatilepb.h synchronous_console.c
syscalls.s: syscalls.S asm_constants.h

QEMU_CMD = qemu-system-arm -M versatilepb -cpu arm1176 -nographic -kernel kernel.elf

qemu: build
	$(QEMU_CMD)

qemu-debug: build
	$(QEMU_CMD) -s -S

gdb-attach:
	arm-none-eabi-gdb -ex 'target remote localhost:1234' -ex 'symbol-file kernel.elf' -tui

clean:
	rm -f *.o *.elf

build: kernel.elf
