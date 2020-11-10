#
# Kurzanleitung
# =============
#
# make			-- Baut den Kernel.
#
# make all		-- Wie make
#
# make debug		-- Baut den Kernel mit debug flags
#
# make install		-- Baut den Kernel und transferiert ihn auf den Server.
# 			   Das Board holt sich diesen Kernel beim nächsten Reset.
#
# make qemu		-- Baut den Kernel und führt ihn unter QEMU aus
#
# make qemu_debug	-- Baut den Kernel und führt ihn unter QEMU mit debug
# 			   Optionen aus. Zum debuggen in einem zweiten Terminal
# 			   folgendes ausführen:
# 			   $ arm-none-eabi-gdb kernel_debug
# 			   $ target remote localhost:1234
#
# make clean		-- Löscht alle erzeugten Dateien.
#
# make submission	-- Packt alles im Verzeichnis für die Abgabe zusammen
#
# make home		-- kopiert das fertige image nach $TFTP_PATH, für die
#			   Arbeit zuhause einfach den Pfad eintragen


# Binäre Lsg (falls verwendet, ansonsten leer lassen)
# bsp: BIN_LSG = e1
BIN_LSG =

# Ab Aufgabe 4 auf true
KERNEL_USER_SPLIT = false

# Hier eure Dateien hinzufügen
# System
#OBJ := $(wildcard system/*.o)
#OBJ += $(wildcard system/std/*.o)
OBJ = system/entry.o
OBJ += system/start.o
OBJ += system/io.o
OBJ += system/std/util.o
OBJ += system/std/strings.o

# Driver
OBJ += driver/led.o
OBJ += driver/serial.o

# Wenn ihr zuhause arbeitet, hier das TFTP-Verzeichnis eintragen
TFTP_PATH = /srv/tftp

# --- Ab hier sollte nichts mehr angepasst werden müssen ;D ---

BIN_LSG_DIR = binlsg
BIN_LSG_FILE = $(BIN_LSG_DIR)/lib$(BIN_LSG).a

# Quellen
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

# Seperates Target für Debugging
OBJ_DEBUG = $(OBJ:.o=.o_d)

# Abgabe Dateien
SUBMISSION_FILES = $(shell find . -name '*' -not -name '*.tar.gz' -not -name '.*' -not -type d -not -name '*.pdf' -not -name '*.o' -not -name '*.o_d*' -not -path './.*')

# Konfiguration
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump

CFLAGS = -Wall -Wextra -ffreestanding -mcpu=cortex-a7 -O2
CFLAGS_DEBUG = $(CFLAGS) -ggdb
CPPFLAGS = -Iinclude
#CPPFLAGS += -Iuser/include
LDFLAGS = -T$(LSCRIPT)
ifneq ($(BIN_LSG), )
	LDFLAGS += -L$(BIN_LSG_DIR) -l$(BIN_LSG)
endif


DEP = $(OBJ:.o=.d) $(OBJ_LSG:.o=.d)

#
# Regeln
#
.PHONY: all install clean

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

dump:
	$(OBJDUMP) -D kernel > kernel_dump.s
ifeq ($(KERNEL_USER_SPLIT),true)
	$(MAKE) -C user dump
endif

install: kernel.img
	arm-install-image $<

qemu: kernel
	qemu-system-arm -M raspi2 -nographic -kernel $<

qemu_debug: kernel_debug
	qemu-system-arm -M raspi2 -nographic -s -S -kernel $<

clean:
	rm -f kernel kernel_debug kernel.bin kernel.img kernel_dump.s
	rm -f $(OBJ)
	rm -f $(OBJ_DEBUG)
	rm -f $(DEP)
	rm -f "$(MATRIKEL_NR).tar.gz"
	$(MAKE) -C user clean

submission: clean abgabe_check
	tar -czf "$(MATRIKEL_NR).tar.gz" $(SUBMISSION_FILES)

home: kernel.img
	cp -v kernel.img $(TFTP_PATH)

# Abgabe check
.PHONY: abgabe_check
abgabe_check:
	ifeq ($(shell test "$$(wc -l < matrikel_nr.txt)" -gt 2; echo $$?),0)
	$(error "matrikel_nr.txt ist fehlerhaft oder leer!")
	endif

	ifeq ($(shell egrep -vq '^[0-9]{6}$$' matrikel_nr.txt; echo $$?),0)
	$(error "matrikel_nr.txt ist fehlerhaft oder leer!")
	endif

	MATRIKEL_NR = $(shell awk '(NR > 1) && (NR < 3)  {ORS="+"; print prev} {prev=$$1} END { ORS=""; print $$1 }' matrikel_nr.txt )

	ifeq ($(MATRIKEL_NR), )
	$(error "matrikel_nr.txt ist fehlerhaft oder leer!")
	endif
