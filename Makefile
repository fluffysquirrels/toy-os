CC=arm-linux-gnueabi-gcc
CFLAGS=-std=c99 -pedantic -Wall -Wextra -march=armv6k -msoft-float -fPIC -mapcs-frame -marm -fno-stack-protector -g
LD=arm-linux-gnueabi-ld
LDFLAGS=-N -Ttext=0x10000

.PHONY: build clean qemu qemu-debug gdb-attach

%.elf: %.o
	$(LD) $(LDFLAGS) -o $@ $^

%.o: %.s
	$(CC) $(CFLAGS) -o $@ -c $^

kernel.o: asm.h asm_constants.h versatilepb.h
kernel.elf: bootstrap.o kernel.o context_switch.o syscalls.o
context_switch.o: context_switch.s
context_switch.s: context_switch.S
QEMU_CMD = qemu-system-arm -M versatilepb -cpu arm1176 -nographic -kernel kernel.elf

qemu: build
	$(QEMU_CMD)

qemu-debug: build
	$(QEMU_CMD) -s -S

gdb-attach:
	arm-none-eabi-gdb -ex 'target remote localhost:1234' -ex 'symbol-file kernel.elf'

clean:
	rm -f *.o *.elf

build: kernel.elf
