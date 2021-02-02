SECTIONS
{
	.ucode : {
		*(.text .text.*)
		*(.rodata .rodata.*)
	}

	.udata : {
		*(.data .data.*)
		*(.bss .bss.*)
	}
}
