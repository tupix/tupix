.section .text

.global kendless_loop
kendless_loop:
	WFI // wait for interrupt
	b kendless_loop
