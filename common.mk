# Disable built-in rules
.SUFFIXES:

.PHONY: always-rebuild

OUT_DIR=target
OBJ_DIR=$(OUT_DIR)/obj
DEP_DIR=$(OUT_DIR)/deps
ENV_DIR=$(OUT_DIR)/env
PREPROCESSED_DIR=$(OUT_DIR)/preprocessed

MAKEFILES+=common.mk

CONFIG_COMPILE_C_PREPROCESSED?=0
ifeq "$(CONFIG_COMPILE_C_PREPROCESSED)" "1"
COMPILE_C_SOURCE_PREFIX:=$(PREPROCESSED_DIR)/
else
COMPILE_C_SOURCE_PREFIX=
endif

GCC_PREFIX?=

CC=$(GCC_PREFIX)gcc
LD=$(GCC_PREFIX)ld
OBJCOPY=$(GCC_PREFIX)objcopy
GDB=$(GCC_PREFIX)gdb

$(OBJ_DIR)/%.o: $(PREPROCESSED_DIR)/%.s $(DEP_DIR)/%.S.d $(MAKEFILES) $(ENV_DIR)/CFLAGS
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.o: $(COMPILE_C_SOURCE_PREFIX)%.c $(DEP_DIR)/%.c.d $(MAKEFILES) $(ENV_DIR)/CFLAGS
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ -c $<

.PRECIOUS: $(PREPROCESSED_DIR)/%.s
$(PREPROCESSED_DIR)/%.s: %.S $(DEP_DIR)/%.S.d $(MAKEFILES) $(ENV_DIR)/CFLAGS
	@mkdir -p $(@D)
	$(CC) -E $(CFLAGS) -o $@ -c $<

define MAKE_DEPS =
	echo MAKE_DEPS $@ : $<
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
	$(MAKE_DEPS)

$(DEP_DIR)/%.S.d: %.S $(MAKEFILES)
	$(MAKE_DEPS)

# Include dependency files
-include $(SOURCES.c:%.c=$(DEP_DIR)/%.c.d)
-include $(SOURCES.S:%.S=$(DEP_DIR)/%.S.d)

.PHONY: preprocess-c
preprocess-c: $(SOURCES.c:%.c=$(PREPROCESSED_DIR)/%.c)

.PRECIOUS: $(PREPROCESSED_DIR)/%.c
$(PREPROCESSED_DIR)/%.c: %.c $(DEP_DIR)/%.c.d $(MAKEFILES)
	@echo
	@mkdir -p $(@D)
	@# sed command replaces gcc directives showing which source lines code came from,
	@# which would make the debugger show the original un-preprocessed source.
	$(CC) $(CFLAGS_INCLUDES) -E $< |\
	  indent |\
	  sed -re 's,^(#[0-9]+),// gcc line \1,' > $@

define MAKE_ENV_FILE
	@$(eval VAR = $(1))
	@mkdir -p $(@D)
	@$(eval ENV_FILE!=echo "$(ENV_DIR)/$(VAR)")
	@$(eval TEMP_ENV!=echo $(ENV_FILE).tmp.$$$$$$$$)
	@#echo ENV_FILE $(ENV_FILE)
	@#echo TEMP_ENV $(TEMP_ENV)
	@echo $($(VAR)) > $(TEMP_ENV)
	@if ! diff $(TEMP_ENV) $(ENV_FILE); then \
		echo Variable $(VAR) changed; \
		mv $(TEMP_ENV) $(ENV_FILE); \
	else \
		echo Variable $(VAR) unchanged; \
	fi
	@rm -f $(TEMP_ENV)
	@echo
endef

# Depend on this to re-run when environment variable CFLAGS changes
$(ENV_DIR)/CFLAGS: always-rebuild
	$(call MAKE_ENV_FILE,CFLAGS)

# Depend on this to re-run when environment variable LDFLAGS changes
$(ENV_DIR)/LDFLAGS: always-rebuild
	$(call MAKE_ENV_FILE,LDFLAGS)

%.gz: %
	gzip -c $< > $@

TAGS: $(SOURCES.c) $(SOURCES.h) $(SOURCES.S)
	etags --declarations -o TAGS $^

.PHONY: clean
clean:
	rm -f TAGS
	rm -rf $(OUT_DIR)/*

.PHONY: build
build: TAGS
