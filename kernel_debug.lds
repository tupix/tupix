ENTRY(_start)
SECTIONS
{
	. = (0x00008000);
	.init : { *(.init) }

	. = (0x00100000);
	.kcode : {
		data/?*.o_d    (.text*)
		driver/?*.o_d  (.text*)
		lib/?*.o_d     (.text*)
		system/?*.o_d  (.text*)

		data/?*.o_d    (.rodata*)
		driver/?*.o_d  (.rodata*)
		lib/?*.o_d     (.rodata*)
		system/?*.o_d  (.rodata*)
	}

	. = (0x00200000);
	.kdata : {
		data/?*.o_d    (.data*)
		driver/?*.o_d  (.data*)
		lib/?*.o_d     (.data*)
		system/?*.o_d  (.data*)

		data/?*.o_d    (.bss* COMMON)
		driver/?*.o_d  (.bss* COMMON)
		lib/?*.o_d     (.bss* COMMON)
		system/?*.o_d  (.bss* COMMON)
	}

	. = ALIGN(16K);
	.kl1 : {
		_l1_start = .;
	}

	. += 16K;
	.kstacks : ALIGN(4K) {
		_kstacks_start = .;
	}

	. = (0x00300000);
	.udata_cpy : {
		_udata_cpy_begin = .;
	}

	. = (0x00400000);
	.ucode : {
		*(.ucode)
	}

	. = (0x00500000);
	.udata : {
		_udata_begin = .;
		*(.udata)
		_udata_end = .;
	}
}
