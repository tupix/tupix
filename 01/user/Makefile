
# Kurzanleitung
# =============
#
# make			-- Baut den User Teil.
#
# make all		-- Wie make
#
# make debug		-- Baut den User Teil mit debug flags
#
# make user_test	-- Baut den User Teil als komplettes image zum testen
#
# make dump		-- Dumpt den User Teil
#
# make clean		-- Löscht alle erzeugten Dateien.

# Hier eure Dateien hinzufügen
OBJ =
# Bsp für weitere:
# OBJ += driver/uart.o


# --- Ab hier sollte nichts mehr angepasst werden müssen ;D ---

# Quellen
LSCRIPT = user.lds

# Seperates Target für Debugging
OBJ_DEBUG = $(OBJ:.o=.o_d)

CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump

CFLAGS = -Wall -Wextra -ffreestanding -mcpu=cortex-a7 -O2
CFLAGS_DEBUG = $(CFLAGS) -ggdb
CPPFLAGS = -Iinclude

DEP = $(OBJ:.o=.d) $(OBJ_LSG:.o=.d)

#
# Regeln
#
.PHONY: all clean

all: user user.bin dump

-include $(DEP)

%.o: %.S
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -o $@ -c $<

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -o $@ -c $<

%.o_d: %.S
	$(CC) $(CPPFLAGS) $(CFLAGS_DEBUG) -MMD -MP -o $@ -c $<

%.o_d: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS_DEBUG) -MMD -MP -o $@ -c $<

user: $(LSCRIPT) $(OBJ)
	$(LD) -r -T$(LSCRIPT) -o $@ $(OBJ) $(OBJ_LSG)

user_debug: $(LSCRIPT) $(OBJ_DEBUG)
	$(LD) -r -T$(LSCRIPT) -o $@ $(OBJ_DEBUG) $(OBJ_LSG)

user.bin: user
	$(OBJCOPY) -Obinary --set-section-flags .bss=contents,alloc,load,data $< $@

user.img: user.bin
	mkimage -A arm -T standalone -C none -a 0x8000 -d $< $@

dump: user
	$(OBJDUMP) -D user > user_dump.s

clean:
	rm -f user user.bin user.img user_dump.s
	rm -f $(OBJ)
	rm -f $(OBJ_DEBUG)
	rm -f $(DEP)
