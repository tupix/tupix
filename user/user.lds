SECTIONS
{
	. = (0x00400000);
	ucode : {
		user/src/?*.o (.text*)
		user/src/?*.o (.rodata*)
	}

	. = (0x00500000);
	udata : {
		user/src/?*.o (.data*)
		user/src/?*.o (.bss* COMMON)
	}
}

