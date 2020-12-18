#include <system/calls.h>

#include <system/ivt.h>

#include <data/types.h>

uint16
get_syscall_id(uint32 lr)
{
	// Software interrupts are recognized later in the pipeline
	lr -= 4;
	// Get last two bytes from `svc` instruction at `lr` for the immediate value
	return (*(uint32*)lr & 0xFF);
}

void
exec_syscall(uint16 lr)
{
	// TODO: enum and macro or a switch case for different values.
}
