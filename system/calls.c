#include <system/calls.h>

#include <arch/armv7/registers.h>

#include <system/isr.h>
#include <system/ivt.h>
#include <system/scheduler.h>
#include <system/thread.h>

#include <data/types.h>

bool
verify_pointer(void* p, struct tcb* curr_thread)
{
	// Verify if pointer is aligned
	// TODO: Is 4 correct?
	if ((uint32)p % 4)
		return false;

	// Verify if pointer lays inside threads stack
	void* sp = get_stack_pointer(curr_thread->index);
	void* max_sp = get_max_stack_pointer(curr_thread->index);
	// TODO: lesser or lesser-equal?
	if (!(p < sp && p >= max_sp))
		return false;

	return true;
}

uint16
get_syscall_id(uint32 lr)
{
	// Software interrupts are recognized later in the pipeline
	lr -= 4;
	// Get last two bytes from `svc` instruction at `lr` for the immediate value
	return (*(uint32*)lr & 0xFF);
}

static void
exec_syscall_kill_me(struct registers* regs)
{
	kill_current_thread(regs);
}

void
exec_syscall(uint16 id, struct registers* regs)
{
	log(LOG, "Syscall with id %i called.", id);
	switch (id) {
	case KILL_ME:
		exec_syscall_kill_me(regs);
		break;
	case GET_CHAR:
		// TODO
	case PUT_CHAR:
		// TODO
	case WAIT:
		// TODO
	case CREATE_THREAD:
		// TODO
	default:
		print_registers(regs, "Software Interrupt", "Killing thread.", "");
		kill_current_thread(regs);
		break;
	}
}
