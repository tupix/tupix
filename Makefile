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
# make submission -- Create an archive for the submission
#
# make home       -- Copy the image to $TFTP_PATH, for the work at home simply
#                    enter the path


# Binary solution in format `e[0-9]` (Leave empty if not used)
BIN_LSG =

# User space programs
KERNEL_USER_SPLIT = true

# Source files
ASS_OBJ := $(shell find . \( -path "./user" -prune -o -name "*.S" \) -a -type f | sed -E 's:^\./(.*)S$$:\1o:')
SRC_OBJ := $(shell find . \( -path "./user" -prune -o -name "*.c" \) -a -type f | sed -E 's:^\./(.*)c$$:\1o:')
OBJ := $(ASS_OBJ) $(SRC_OBJ)

# Enter TFTP path here for work at home
TFTP_PATH = /srv/tftp

# Binary solution
BIN_LSG_DIR = binlsg
BIN_LSG_FILE = $(BIN_LSG_DIR)/lib$(BIN_LSG).a

# Sources
LSCRIPT = kernel.lds
LSCRIPT_DEBUG = kernel_debug.lds
UOBJ = user/user
UOBJ_DEBUG = $(UOBJ)_debug

KERNEL_TARGETS = $(LSCRIPT) $(OBJ)
ifeq ($(KERNEL_USER_SPLIT),true)
	KERNEL_TARGETS += $(UOBJ)
endif

KERNEL_TARGETS_DEBUG = $(LSCRIPT_DEBUG) $(OBJ_DEBUG)
ifeq ($(KERNEL_USER_SPLIT),true)
	KERNEL_TARGETS_DEBUG += $(UOBJ_DEBUG)
endif

# Debug sources
OBJ_DEBUG = $(OBJ:.o=.o_d)

# Clean patch files on `clean` target
OBJ_PATCH = $(OBJ:.o=.orig) $(OBJ:.o=.rej)

# Configuration
RM = rm -f
TAR = tar -czf
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
ifneq ($(BIN_LSG), )
	LDFLAGS += -L$(BIN_LSG_DIR) -l$(BIN_LSG)
	LDFLAGS_DEBUG += -L$(BIN_LSG_DIR) -l$(BIN_LSG)
endif

DEP = $(OBJ:.o=.d) $(OBJ_LSG:.o=.d)

#
# Targets
# =============

.PHONY: all
all: kernel kernel.bin dump

-include $(DEP)

%.o: %.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -o $@ -c $<

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -o $@ -c $<

%.o_d: %.S
	$(CC) $(CPPFLAGS) $(CFLAGS_DEBUG) -MMD -MP -o $@ -c $<

%.o_d: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS_DEBUG) -MMD -MP -o $@ -c $<

$(UOBJ):
	$(MAKE) -C user user

$(UOBJ_DEBUG):
	$(MAKE) -C user user_debug

kernel: $(KERNEL_TARGETS)
ifeq ($(KERNEL_USER_SPLIT),true)
	$(LD) -o $@ $(OBJ) $(UOBJ) $(OBJ_LSG) $(LDFLAGS)
else
	$(LD) -o $@ $(OBJ) $(OBJ_LSG) $(LDFLAGS)
endif

kernel_debug: $(KERNEL_TARGETS_DEBUG)
ifeq ($(KERNEL_USER_SPLIT),true)
	$(LD) -o $@ $(OBJ_DEBUG) $(UOBJ_DEBUG) $(OBJ_LSG) $(LDFLAGS_DEBUG)
else
	$(LD) -o $@ $(OBJ_DEBUG) $(OBJ_LSG) $(LDFLAGS_DEBUG)
endif

kernel.bin: kernel
	$(OBJCOPY) -Obinary --set-section-flags .bss=contents,alloc,load,data $< $@

kernel.img: kernel.bin
	mkimage -A arm -T standalone -C none -a 0x8000 -d $< $@

.PHONY: dump
dump:
	$(OBJDUMP) -D kernel > kernel_dump.s
ifeq ($(KERNEL_USER_SPLIT),true)
	$(MAKE) -C user dump
endif

.PHONY: install
install: kernel.img
	arm-install-image $<

.PHONY: qemu
qemu: kernel
	$(QEMU) -M raspi2 -nographic -kernel $<

.PHONY: qemu_debug
qemu_debug: kernel_debug
	$(QEMU) -M raspi2 -nographic -s -S -kernel $<

.PHONY: gdb_debug
gdb_debug: kernel_debug
	$(GDB) kernel_debug -ex "target remote localhost:1234"

.PHONY: clean
clean:
	$(RM) kernel kernel_debug kernel.bin kernel.img kernel_dump.s
	$(RM) $(OBJ)
	$(RM) $(OBJ_DEBUG)
	$(RM) $(OBJ_PATCH)
	$(RM) $(DEP)
	$(RM) "$(MATRIKEL_NR).tar.gz"
	$(MAKE) -C user clean

.PHONY: submission
submission: submission_check clean
	# Submission files (Everything tracked by git minus hidden files, tars, pdfs and
	# the format.sh
	submission : SUBMISSION_FILES = $(shell git ls-files | grep -Ev "(^|/)\." | grep -Ev "\.(tar\.gz|pdf)$$" | grep -v "format.sh")
	submission : MATRIKEL_NR := $(shell awk '(NR > 1) && (NR < 3)  {ORS="+"; print prev} {prev=$$1} END { ORS=""; print $$1 }' matrikel_nr.txt )
	$(TAR) "$(MATRIKEL_NR).tar.gz" $(SUBMISSION_FILES)

.PHONY: home
home: kernel.img
	cp -v kernel.img $(TFTP_PATH)

#
# Submission checks
# =============

.PHONY: submission_check
submission_check:
	submission_check : MATRIKEL_NR_ROWS := $(shell test "$$(wc -l < matrikel_nr.txt)" -gt 2; echo $$?)
	submission_check : MATRIKEL_NR_DIGITS := $(shell egrep -vq '^[0-9]{6}$$' matrikel_nr.txt; echo $$?)
	submission_check : GIT_REPO_DIRTY := $(shell test -n "$$(git status --porcelain --untracked-files)"; echo $$?)
	ifeq ($(MATRIKEL_NR), )
		$(error "matrikel_nr.txt is flawed or empty!")
	endif
	ifeq ($(MATRIKEL_NR_ROWS), 0)
		$(error "matrikel_nr.txt contains too many lines (max 2)")
	endif
	ifeq ($(MATRIKEL_NR_DIGITS),0)
		$(error "matrikel_nr.txt is flawed. Every line needs to match ^[0-9]{6}$$")
	endif
	ifeq ($(GIT_REPO_DIRTY), 0)
		$(error "Git repository not clean!")
	endif
