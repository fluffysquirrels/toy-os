# TODO: set default if no config
CURRENT_CONFIG_FILE=config/current.mk

# Doesn't work with empty checkout as rest of makefile won't load without a config/current.mk
ifeq ("$(wildcard $(CURRENT_CONFIG_FILE))","")
$(error Missing config file "$(CURRENT_CONFIG_FILE)". Create it, e.g. just containing "include config/versatilepb.mk")
endif
include $(CURRENT_CONFIG_FILE)

MAKEFILES += $(wildcard config/*.mk) Makefile

ARCH_DIR=arch/$(CONFIG_ARCH)
SOURCES.c := $(wildcard *.c)
SOURCES.c += $(wildcard $(ARCH_DIR)/*.c)
SOURCES.c += third_party/FreeRTOS/heap_4.c
SOURCES.h := $(wildcard *.h)
SOURCES.S := $(wildcard *.S)

OBJECTS = $(SOURCES.c:%.c=$(OBJ_DIR)/%.o)
OBJECTS += $(SOURCES.S:%.S=$(OBJ_DIR)/%.o)

CFLAGS_ARCH=-std=c99 -march=armv7-a -msoft-float -fPIC -mapcs-frame -marm -fno-stack-protector -ggdb -DCONFIG_ARCH_$(CONFIG_ARCH)=1 -ffunction-sections -fdata-sections
CFLAGS_ERRORS=-pedantic -Wall -Wextra -Werror
CFLAGS_INCLUDES=-I$(ARCH_DIR) -I.
CFLAGS+=$(CFLAGS_ARCH) $(CFLAGS_ERRORS) $(CFLAGS_INCLUDES)

LDFLAGS+= --section-start=.text.startup=0x10000 --section-start=.text=0x10000 --fatal-warnings --gc-sections --omagic
GCC_LIBS=/usr/lib/gcc-cross/arm-linux-gnueabi/5
LDLIBS+=$(GCC_LIBS)/libgcc.a

GCC_PREFIX=arm-none-eabi-

include common.mk

.PHONY: config
config:
	@echo
	cat config/current.mk

.PHONY: config-raspi2
config-raspi2:
	echo "include config/raspi2.mk" > config/current.mk

.PHONY: config-versatilepb
config-versatilepb:
	echo "include config/versatilepb.mk" > config/current.mk

.PHONY: config-stm32f4
config-stm32f4:
	echo "include config/stm32f4.mk" > config/current.mk

OUT_ELF := $(OUT_DIR)/kernel.elf
OUT_SLIM_ELF := $(OUT_DIR)/kernel.slim.elf
OUT_RAW := $(OUT_DIR)/kernel.raw

QEMU_SYSTEM_ARM = ../qemu/build/arm-softmmu/qemu-system-arm
QEMU_CMD = $(QEMU_SYSTEM_ARM) -M $(CONFIG_QEMU_MACHINE) -cpu cortex-a8 -nographic -kernel $(OUT_ELF)

.PHONY: qemu
qemu: build
	$(QEMU_CMD)

.PHONY: qemu-debug-listen
qemu-debug-listen: build
	$(QEMU_CMD) -s -S

.PHONY: gdb-attach
gdb-attach: build
	$(GDB) -ex 'target remote localhost:1234' -ex 'symbol-file $(OUT_ELF)' -tui

.PHONY: gdb-attach-new-term
gdb-attach-new-term: build
	gnome-terminal -e 'make gdb-attach'

.PHONY: qemu-debug
qemu-debug: gdb-attach-new-term qemu-debug-listen

SERIAL_DEV=/dev/ttyUSB0
PICOCOM_CMD:=picocom --baud 115200 --imap lfcrlf --send-cmd "sb -vv" $(SERIAL_DEV)

.PHONY: run-serial
run-serial: $(OUT_RAW).gz build
	du -ha --apparent-size $(OUT_DIR)/kernel*

	echo loady 0x10000 > $(SERIAL_DEV)
	sleep 1
	sb target/kernel.raw < $(SERIAL_DEV) > $(SERIAL_DEV)

	#echo loady 0x200000 > $(SERIAL_DEV)
	#sleep 1
	#sb target/kernel.raw.gz < $(SERIAL_DEV) > $(SERIAL_DEV)
	#sleep 1
	#echo unzip 0x200000 0x10000 > $(SERIAL_DEV)

	sleep 1
	echo go 0x10000 > $(SERIAL_DEV)
	$(PICOCOM_CMD)

.PHONY: term-serial
term-serial:
	$(PICOCOM_CMD)

.PHONY: build
build: $(OUT_ELF) $(OUT_RAW) $(OUT_RAW).gz $(OUT_SLIM_ELF) $(OUT_SLIM_ELF).gz

$(OUT_RAW): $(OUT_ELF) $(MAKEFILES)
	$(OBJCOPY) --strip-all --strip-debug -O binary $< $@

$(OUT_SLIM_ELF): $(OUT_ELF)
	$(OBJCOPY) --strip-all $< $@

$(OUT_ELF): $(OBJECTS) $(ENV_DIR)/LDFLAGS $(MAKEFILES)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS) $(LDLIBS)
