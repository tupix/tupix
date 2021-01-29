ENTRY(_start)
SECTIONS
{
	. = (0x00100000);
	kcode : {
		system/entry.o (.init)

		data/?*.o    (.text*)
		driver/?*.o  (.text*)
		lib/?*.o     (.text*)
		system/?*.o  (.text*)

		data/?*.o    (.rodata*)
		driver/?*.o  (.rodata*)
		lib/?*.o     (.rodata*)
		system/?*.o  (.rodata*)
	}

	. = (0x00200000);
	kdata : {
		data/?*.o    (.data*)
		driver/?*.o  (.data*)
		lib/?*.o     (.data*)
		system/?*.o  (.data*)

		data/?*.o    (.bss* COMMON)
		driver/?*.o  (.bss* COMMON)
		lib/?*.o     (.bss* COMMON)
		system/?*.o  (.bss* COMMON)
	}
}
