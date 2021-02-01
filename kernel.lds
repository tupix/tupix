ENTRY(_start)
SECTIONS
{

	. = (0x00008000);
	.init : { *(.init) }


	. = (0x00100000);
	.kcode : {
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
	.kdata : {
		data/?*.o    (.data*)
		driver/?*.o  (.data*)
		lib/?*.o     (.data*)
		system/?*.o  (.data*)

		data/?*.o    (.bss* COMMON)
		driver/?*.o  (.bss* COMMON)
		lib/?*.o     (.bss* COMMON)
		system/?*.o  (.bss* COMMON)
	}

	. = (0x00400000);
	.ucode : {
		*(.ucode)
	}

	. = (0x00500000);
	.udata : {
		*(.udata)
	}


}
