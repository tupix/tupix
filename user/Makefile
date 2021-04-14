#
# Usage
# =============
#
# make            -- Build user space
#
# make all        -- Build user space
#
# make user_debug -- Build user space with debug flags
#
# make dump       -- Dump user space
#
# make clean      -- Delete all created files

# Source files
ASS_OBJ := $(shell find . -name "*.S" -a -type f | sed -E 's:^\./(.*.S)$$:\1.o:')
SRC_OBJ := $(shell find . -name "*.c" -a -type f | sed -E 's:^\./(.*.c)$$:\1.o:')
OBJ := $(ASS_OBJ) $(SRC_OBJ)
OBJ_DEBUG = $(OBJ:.o=.o_d)

# Linker script
LSCRIPT = user.lds

# Clean patch files on `clean` target
OBJ_PATCH = $(OBJ:.o=.orig) $(OBJ:.o=.rej)

# Configuration
RM = rm -f
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump

CFLAGS = -Wall -Wextra -ffreestanding -mcpu=cortex-a7 -O2
# gcc will use last specified -O flag
CFLAGS_DEBUG = $(CFLAGS) -ggdb -Og
CPPFLAGS = -Iinclude

DEP = $(OBJ:.o=.d) $(OBJ_LSG:.o=.d)

#
# Targets
# ===========

.PHONY: all
all: user user.bin dump

-include $(DEP)

%.S.o: %.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -o $@ -c $<

%.c.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -o $@ -c $<

%.S.o_d: %.S
	$(CC) $(CPPFLAGS) $(CFLAGS_DEBUG) -MMD -MP -o $@ -c $<

%.c.o_d: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS_DEBUG) -MMD -MP -o $@ -c $<

user: $(LSCRIPT) $(OBJ)
	$(LD) -r -T$(LSCRIPT) -o $@ $(OBJ) $(OBJ_LSG)

user_debug: $(LSCRIPT) $(OBJ_DEBUG)
	$(LD) -r -T$(LSCRIPT) -o $@ $(OBJ_DEBUG) $(OBJ_LSG)

user.bin: user
	$(OBJCOPY) -Obinary --set-section-flags .bss=contents,alloc,load,data $< $@

user.img: user.bin
	mkimage -A arm -T standalone -C none -a 0x8000 -d $< $@

.PHONY: dump
dump: user
	$(OBJDUMP) -D user > user_dump.s

.PHONY: clean
clean:
	$(RM) user user_debug user.bin user.img user_dump.s
	$(RM) $(OBJ)
	$(RM) $(OBJ_DEBUG)
	$(RM) $(OBJ_PATCH)
	$(RM) $(DEP)
