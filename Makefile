# Disable built-in rules
.SUFFIXES:

OUT_DIR :=target
OBJ_DIR :=$(OUT_DIR)/obj
DEP_DIR :=$(OUT_DIR)/deps

$(shell mkdir -p $(OUT_DIR) $(OBJ_DIR) $(DEP_DIR))

SOURCES.c := $(wildcard *.c)
SOURCES.h := $(wildcard *.h)
SOURCES.S := $(wildcard *.S)

OUT_IMG := $(OUT_DIR)/kernel.elf
OBJECTS := $(SOURCES.c:%.c=$(OBJ_DIR)/%.o)
OBJECTS += $(SOURCES.S:%.S=$(OBJ_DIR)/%.o)
OBJECTS += $(OBJ_DIR)/FreeRTOS_heap_4.o

CC=arm-linux-gnueabi-gcc
CFLAGS_ARCH=-std=c99 -march=armv6k -msoft-float -fPIC -mapcs-frame -marm -fno-stack-protector -ggdb
CFLAGS_ERRORS=-pedantic -Wall -Wextra -Werror
CFLAGS+=$(CFLAGS_ARCH) $(CFLAGS_ERRORS)
GCC_LIBS=/usr/lib/gcc-cross/arm-linux-gnueabi/5
LD=arm-linux-gnueabi-ld
LDFLAGS=-N -Ttext=0x10000

QEMU_CMD = qemu-system-arm -M versatilepb -cpu arm1176 -nographic -kernel $(OUT_IMG)

.PHONY += qemu
qemu: build
	$(QEMU_CMD)

.PHONY += qemu-debug-listen
qemu-debug-listen: build
	$(QEMU_CMD) -s -S

.PHONY += gdb-attach
gdb-attach: build
	arm-none-eabi-gdb -ex 'target remote localhost:1234' -ex 'symbol-file $(OUT_IMG)' -tui

.PHONY += gdb-attach-new-term
gdb-attach-new-term: build
	gnome-terminal -e 'make gdb-attach'

.PHONY += debug
debug: gdb-attach-new-term qemu-debug-listen

.PHONY += clean
clean:
	rm -f TAGS
	rm -rf $(OUT_DIR)

.PHONY += build
build: $(OUT_IMG) TAGS

TAGS: $(SOURCES.c) $(SOURCES.h) $(SOURCES.S)
	etags --declarations -o TAGS *.c *.h *.S

$(OUT_IMG): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^ $(GCC_LIBS)/libgcc.a

$(OBJ_DIR)/%.o: $(OBJ_DIR)/%.s $(DEP_DIR)/%.d Makefile
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o: %.c $(DEP_DIR)/%.d Makefile
	$(CC) $(CFLAGS) -o $@ -c $<

.PRECIOUS: $(OBJ_DIR)/%.s
$(OBJ_DIR)/%.s: %.S Makefile
	$(CC) -E -o $@ -c $<

define MAKE-DEPS =
	set -e; rm -f $@
	$(eval TEMP_DEP!=echo $@.tmp.$$$$$$$$)
	@echo using TEMP_DEP = $(TEMP_DEP)
	$(CC) -E -MM -MP $< > $(TEMP_DEP) || (rm -f $(TEMP_DEP) && false)
	sed -e 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $(TEMP_DEP) > $@ || (rm -f $(TEMP_DEP) $@ && false)
	rm -f $(TEMP_DEP)
endef

$(DEP_DIR)/%.d: %.c Makefile
	$(MAKE-DEPS)

$(DEP_DIR)/%.d: %.S Makefile
	$(MAKE-DEPS)

$(OBJ_DIR)/FreeRTOS_heap_4.o: third_party/FreeRTOS/heap_4.c Makefile
	$(CC) $(CFLAGS) -I. -o $@ -c $<

 # Include dependency files
-include $(SOURCES.c:%.c=$(DEP_DIR)/%.d)
-include $(SOURCES.S:%.S=$(DEP_DIR)/%.d)
