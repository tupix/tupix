#
# Usage
# =============
#
# make            -- Build kernel
#
# make all        -- Build kernel
#
# make debug      -- Build kernel with debug flags
#
# make install    -- Build kernel and transfer onto server.
#                    The board gets the kernel at the next reset.
#
# make qemu       -- Build kernel and execute it in QEMU
#
# make qemu_debug -- Build kernel and execute it in QEMU with debug flags.
#                    For debugging execute in a second terminal:
#                    $ arm-none-eabi-gdb kernel_debug
#                    $ target remote localhost:1234
#
# make gdb_debug  -- Start gdb and attach it to an already running instance of
#                    qemu_debug
#
# make clean      -- Delete all created files
#
# make home       -- Copy the image to $TFTP_PATH, for the work at home simply
#                    enter the path

# Source files
ASS_OBJ := $(shell find . \( -path "./user" -prune -o -name "*.S" \) -a -type f | sed -E 's:^\./(.*\.S)$$:\1.o:')
SRC_OBJ := $(shell find . \( -path "./user" -prune -o -name "*.c" \) -a -type f | sed -E 's:^\./(.*\.c)$$:\1.o:')
OBJ := $(ASS_OBJ) $(SRC_OBJ)
OBJ_DEBUG = $(OBJ:.o=.o_d)

# Enter TFTP path here for work at home
TFTP_PATH = /srv/tftp

# Linker scripts
LSCRIPT = kernel.lds
LSCRIPT_DEBUG = kernel_debug.lds

# User space sources
UOBJ = user/user
UOBJ_DEBUG = $(UOBJ)_debug

KERNEL_TARGETS = $(LSCRIPT) $(OBJ) $(UOBJ)
KERNEL_TARGETS_DEBUG = $(LSCRIPT_DEBUG) $(OBJ_DEBUG) $(UOBJ_DEBUG)

# Clean patch files on `clean` target
OBJ_PATCH = $(OBJ:.o=.orig) $(OBJ:.o=.rej)

# Configuration
RM = rm -f
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
QEMU = qemu-system-arm
GDB = arm-none-eabi-gdb

CFLAGS = -Wall -Wextra -ffreestanding -mcpu=cortex-a7 -O2
# gcc will use last specified -O flag
CFLAGS_DEBUG = $(CFLAGS) -ggdb -Og
CPPFLAGS = -Iinclude
# ld would accumulate multiple specified linker scripts, thus separate flags are
# needed
LDFLAGS = -T$(LSCRIPT)
LDFLAGS_DEBUG = -T$(LSCRIPT_DEBUG)

DEP = $(OBJ:.o=.d)

#
# Targets
# =============

.PHONY: all
all: kernel kernel.bin dump

-include $(DEP)

%.S.o: %.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -o $@ -c $<

%.c.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -o $@ -c $<

%.S.o_d: %.S
	$(CC) $(CPPFLAGS) $(CFLAGS_DEBUG) -MMD -MP -o $@ -c $<

%.c.o_d: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS_DEBUG) -MMD -MP -o $@ -c $<

$(UOBJ):
	$(MAKE) -C user user

$(UOBJ_DEBUG):
	$(MAKE) -C user user_debug

kernel: $(KERNEL_TARGETS)
	$(LD) -o $@ $(OBJ) $(UOBJ) $(LDFLAGS)

kernel_debug: $(KERNEL_TARGETS_DEBUG)
	$(LD) -o $@ $(OBJ_DEBUG) $(UOBJ_DEBUG) $(LDFLAGS_DEBUG)

kernel.bin: kernel
	$(OBJCOPY) -Obinary --set-section-flags .bss=contents,alloc,load,data $< $@

kernel.img: kernel.bin
	mkimage -A arm -T standalone -C none -a 0x8000 -d $< $@

.PHONY: dump
dump:
	$(OBJDUMP) -D kernel > kernel_dump.s
	$(MAKE) -C user dump

.PHONY: install
install: kernel.img
	arm-install-image $<

.PHONY: qemu
qemu: kernel
	$(QEMU) -M raspi2 -nographic -kernel $<

.PHONY: qemu_debug
qemu_debug: kernel_debug
	$(QEMU) -M raspi2 -nographic -s -S -kernel $<

.PHONY: qemu_debug_monitor
qemu_debug_monitor: kernel_debug
	$(QEMU) -M raspi2 -nographic -s -S -kernel kernel_debug -monitor telnet::45454,server,nowait -serial mon:stdio

.PHONY: gdb_debug
gdb_debug: kernel_debug
	$(GDB) kernel_debug -ex "target remote localhost:1234"

.PHONY: qemu_monitor
qemu_monitor: kernel_debug
	telnet localhost 45454

.PHONY: clean
clean:
	$(RM) kernel kernel_debug kernel.bin kernel.img kernel_dump.s
	$(RM) $(OBJ)
	$(RM) $(OBJ_DEBUG)
	$(RM) $(OBJ_PATCH)
	$(RM) $(DEP)
	$(MAKE) -C user clean

.PHONY: home
home: kernel.img
	cp -v kernel.img $(TFTP_PATH)

