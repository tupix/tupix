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
# make clean      -- Delete all created files
#
# make submission -- Create an archive for the submission
#
# make home       -- Copy the image to $TFTP_PATH, for the work at home simply
#                    enter the path


# Binary solution in format `e[0-9]` (Leave empty if not used)
BIN_LSG =

# User space programs
KERNEL_USER_SPLIT = false

# Source files
ASS_OBJ := $(shell find . -name "*.S" | sed 's/S$$/o/')
SRC_OBJ := $(shell find . -name "*.c" | sed 's/c$$/o/')
OBJ := $(ASS_OBJ) $(SRC_OBJ)

# Enter TFTP path here for work at home
TFTP_PATH = /srv/tftp

# Binary solution
BIN_LSG_DIR = binlsg
BIN_LSG_FILE = $(BIN_LSG_DIR)/lib$(BIN_LSG).a

# Sources
LSCRIPT = kernel.lds
UOBJ = user/user
UOBJ_DEBUG = $(UOBJ)_debug

KERNEL_TARGETS = $(LSCRIPT) $(OBJ)
ifeq ($(KERNEL_USER_SPLIT),true)
	KERNEL_TARGETS += $(UOBJ)
endif

KERNEL_TARGETS_DEBUG = $(LSCRIPT) $(OBJ_DEBUG)
ifeq ($(KERNEL_USER_SPLIT),true)
	KERNEL_TARGETS_DEBUG += $(UOBJ_DEBUG)
endif

# Debug sources
OBJ_DEBUG = $(OBJ:.o=.o_d)

# Clean patch files on `clean` target
OBJ_PATCH = $(OBJ:.o=.orig) $(OBJ:.o=.rej)

# Submission files (Everything tracked by git minus hidden files, tars, pdfs and
# the format.sh
SUBMISSION_FILES = $(shell git ls-files | grep -Ev "(^|/)\." | grep -Ev "\.(tar\.gz|pdf)$$" | grep -v "format.sh")
MATRIKEL_NR := $(shell awk '(NR > 1) && (NR < 3)  {ORS="+"; print prev} {prev=$$1} END { ORS=""; print $$1 }' matrikel_nr.txt )

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
LDFLAGS = -T$(LSCRIPT)
ifneq ($(BIN_LSG), )
	LDFLAGS += -L$(BIN_LSG_DIR) -l$(BIN_LSG)
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
	$(LD) -o $@ $(OBJ_DEBUG) $(UOBJ_DEBUG) $(OBJ_LSG) $(LDFLAGS)
else
	$(LD) -o $@ $(OBJ_DEBUG) $(OBJ_LSG) $(LDFLAGS)
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
	qemu-system-arm -M raspi2 -nographic -kernel $<

.PHONY: qemu_debug
qemu_debug: kernel_debug
	qemu-system-arm -M raspi2 -nographic -s -S -kernel $<

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
	tar -czf "$(MATRIKEL_NR).tar.gz" $(SUBMISSION_FILES)

.PHONY: home
home: kernel.img
	cp -v kernel.img $(TFTP_PATH)

#
# Submission checks
# =============

MATRIKEL_NR_ROWS := $(shell test "$$(wc -l < matrikel_nr.txt)" -gt 2; echo $$?)
MATRIKEL_NR_DIGITS := $(shell egrep -vq '^[0-9]{6}$$' matrikel_nr.txt; echo $$?)

GIT_REPO_DIRTY := $(shell test -n "$$(git status --porcelain --untracked-files)"; echo $$?)

.PHONY: submission_check
submission_check:
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
