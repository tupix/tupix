SECTIONS
{
	. = (0x00400000);
	ucode : {
		user/?*.o (.text*)
		user/?*.o (.rodata*)
	}

	. = (0x00500000);
	udata : {
		user/?*.o (.data*)
		user/?*.o (.bss* COMMON)
	}
}

