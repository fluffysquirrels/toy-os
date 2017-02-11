# Disable built-in rules
.SUFFIXES:

.PHONY: always-rebuild

# TODO: set default if no config
CURRENT_CONFIG_FILE=config/current.mk

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

# Doesn't work with empty checkout as rest of makefile won't load without a config/current.mk
ifeq ("$(wildcard $(CURRENT_CONFIG_FILE))","")
$(error Missing config file "$(CURRENT_CONFIG_FILE)". Create it, e.g. just containing "include config/versatilepb.mk")
endif
include $(CURRENT_CONFIG_FILE)

MAKEFILES = Makefile $(wildcard config/*.mk)

OUT_DIR:= target
OBJ_DIR:= $(OUT_DIR)/obj
DEP_DIR:= $(OUT_DIR)/deps
ENV_DIR:= $(OUT_DIR)/env
PREPROCESSED_DIR:= $(OUT_DIR)/preprocessed

$(shell mkdir -p $(OUT_DIR) $(ENV_DIR))

ARCH_DIR=arch/$(CONFIG_ARCH)
SOURCES.c := $(wildcard *.c)
SOURCES.c += $(wildcard $(ARCH_DIR)/*.c)
SOURCES.h := $(wildcard *.h)
SOURCES.S := $(wildcard *.S)

OUT_ELF := $(OUT_DIR)/kernel.elf
OUT_RAW := $(OUT_DIR)/kernel.raw
OBJECTS := $(SOURCES.c:%.c=$(OBJ_DIR)/%.o)
OBJECTS += $(SOURCES.S:%.S=$(OBJ_DIR)/%.o)
OBJECTS += $(OBJ_DIR)/FreeRTOS_heap_4.o

CC=arm-none-eabi-gcc
CFLAGS_ARCH=-std=c99 -march=armv7-a -msoft-float -fPIC -mapcs-frame -marm -fno-stack-protector -ggdb -DCONFIG_ARCH_$(CONFIG_ARCH)=1 -Os -ffunction-sections -fdata-sections
CFLAGS_ERRORS=-pedantic -Wall -Wextra -Werror
CFLAGS_INCLUDES=-I$(ARCH_DIR) -I.
CFLAGS+=$(CFLAGS_ARCH) $(CFLAGS_ERRORS) $(CFLAGS_INCLUDES)
GCC_LIBS=/usr/lib/gcc-cross/arm-linux-gnueabi/5
LD=arm-none-eabi-ld
LDFLAGS+= --section-start=.text.startup=0x10000 --section-start=.text=0x10000 --fatal-warnings --gc-sections --omagic
CONFIG_COMPILE_PREPROCESSED?=0
ifeq "$(CONFIG_COMPILE_PREPROCESSED)" "1"
COMPILE_SOURCE_PREFIX:=$(PREPROCESSED_DIR)/
else
COMPILE_SOURCE_PREFIX=
endif
OBJCOPY=arm-none-eabi-objcopy

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
	arm-none-eabi-gdb -ex 'target remote localhost:1234' -ex 'symbol-file $(OUT_ELF)' -tui

.PHONY: gdb-attach-new-term
gdb-attach-new-term: build
	gnome-terminal -e 'make gdb-attach'

.PHONY: qemu-debug
qemu-debug: gdb-attach-new-term qemu-debug-listen

SERIAL_DEV=/dev/ttyUSB0
PICOCOM_CMD:=picocom --baud 115200 --imap lfcrlf --send-cmd "sb -vv" $(SERIAL_DEV)

.PHONY: run-serial
run-serial: $(OUT_RAW).gz build
	du -ha --apparent-size $(OUT_RAW) $(OUT_RAW).gz
	echo loady 0x200000 > $(SERIAL_DEV)
	sleep 1
	sb target/kernel.raw.gz < $(SERIAL_DEV) > $(SERIAL_DEV)
	sleep 1
	echo unzip 0x200000 0x10000 > $(SERIAL_DEV)
	sleep 1
	echo go 0x10000 > $(SERIAL_DEV)
	$(PICOCOM_CMD)

.PHONY: term-serial
term-serial:
	$(PICOCOM_CMD)

.PHONY: clean
clean:
	rm -f TAGS
	rm -rf $(OUT_DIR)/*

.PHONY: build
build: TAGS $(OUT_ELF) $(OUT_RAW) $(OUT_RAW).gz

TAGS: $(SOURCES.c) $(SOURCES.h) $(SOURCES.S)
	etags --declarations -o TAGS *.c *.h *.S

$(OUT_RAW): $(OUT_ELF) $(MAKEFILES)
	$(OBJCOPY) --strip-all --strip-debug -O binary $< $@

$(OUT_RAW).gz: $(OUT_RAW) $(MAKEFILES)
	gzip -c $(OUT_RAW) > $(OUT_RAW).gz

$(OUT_ELF): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ $(GCC_LIBS)/libgcc.a

$(OBJ_DIR)/%.o: $(OBJ_DIR)/%.s $(DEP_DIR)/%.S.d $(MAKEFILES) $(ENV_DIR)/CFLAGS
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o: $(COMPILE_SOURCE_PREFIX)%.c $(DEP_DIR)/%.c.d $(MAKEFILES) $(ENV_DIR)/CFLAGS
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

.PRECIOUS: $(OBJ_DIR)/%.s
$(OBJ_DIR)/%.s: %.S $(DEP_DIR)/%.S.d $(MAKEFILES) $(ENV_DIR)/CFLAGS
	$(CC) -E $(CFLAGS) -o $@ -c $<

define MAKE-DEPS =
	echo MAKE-DEPS $@ : $<
	@$(eval TEMP_DEP!=echo $@.tmp.$$$$$$$$)
	@echo using TEMP_DEP = $(TEMP_DEP)
	@mkdir -p $(@D)
	@# Move existing file to *.old to ensure if we fail make does not
	@# see a stale file.
	@if test -f $@; then mv $@ $(TEMP_DEP).old; fi
	@$(CC) $(CFLAGS_INCLUDES) -E -MM -MP $< > $(TEMP_DEP).pp || (rm -f $(TEMP_DEP)* && false)
	@sed --expression 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g' < $(TEMP_DEP).pp > $(TEMP_DEP).sed || (rm -f $(TEMP_DEP)* && false)

	@cp $(TEMP_DEP).sed $@;

	@# Comment out this rm to view the temp files output at each step.
	@rm $(TEMP_DEP)*
	@echo
endef

$(DEP_DIR)/%.c.d: %.c $(MAKEFILES)
	$(MAKE-DEPS)

$(DEP_DIR)/%.S.d: %.S $(MAKEFILES)
	$(MAKE-DEPS)

.PHONY: preprocess-c
preprocess-c: $(SOURCES.c:%.c=$(PREPROCESSED_DIR)/%.c)

.PRECIOUS: $(PREPROCESSED_DIR)/%.c
$(PREPROCESSED_DIR)/%.c: %.c $(DEP_DIR)/%.c.d $(MAKEFILES)
	@echo
	mkdir -p $(@D)
	@# sed command replaces gcc directives showing which source lines code came from,
	@# which would make the debugger show the original un-preprocessed source.
	$(CC) $(CFLAGS_INCLUDES) -E $< |\
	  indent |\
	  sed -re 's,^(#[0-9]+),// gcc line \1,' > $@

$(OBJ_DIR)/FreeRTOS_heap_4.o: third_party/FreeRTOS/heap_4.c $(MAKEFILES)
	$(CC) $(CFLAGS) -I. -o $@ -c $<

# Depend on this to re-run when environment variable CFLAGS changes
$(ENV_DIR)/CFLAGS: always-rebuild
	@echo Checking variable CFLAGS
	@$(eval ENV_FILE!=echo "$(ENV_DIR)/CFLAGS")
	@$(eval TEMP_ENV!=echo $(ENV_FILE).tmp.$$$$$$$$)
	@#echo ENV_FILE $(ENV_FILE)
	@#echo TEMP_ENV $(TEMP_ENV)
	@echo $(CFLAGS) > $(TEMP_ENV)
	@if ! diff $(TEMP_ENV) $(ENV_FILE); then \
		echo variable CFLAGS changed; \
		mv $(TEMP_ENV) $(ENV_FILE); \
	else \
		echo variable CFLAGS unchanged; \
	fi
	@rm -f $(TEMP_ENV)
	@echo

# Include dependency files
-include $(SOURCES.c:%.c=$(DEP_DIR)/%.c.d)
-include $(SOURCES.S:%.S=$(DEP_DIR)/%.S.d)
