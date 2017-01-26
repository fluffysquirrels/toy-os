CC=arm-linux-gnueabi-gcc
CFLAGS=-ansi -pedantic -Wall -Wextra -march=armv6k -msoft-float -fPIC -mapcs-frame -marm -fno-stack-protector
LD=arm-linux-gnueabi-ld
LDFLAGS=-N -Ttext=0x10000

.PHONY: clean qemu

.SUFFIXES: .o .elf
.o.elf:
	$(LD) $(LDFLAGS) -o $@ $^

.SUFFIXES: .s .o
.s.o:
	$(CC) $(CFLAGS) -o $@ -c $^

kernel.elf: bootstrap.o kernel.o context_switch.o syscalls.o

qemu: kernel.elf
	qemu-system-arm -M versatilepb -cpu arm1176 -nographic -kernel kernel.elf

clean:
	rm -f *.o *.elf

