#include <system/calls.h>

#include <arch/armv7/registers.h>

#include <driver/uart.h>

#include <system/isr.h>
#include <system/ivt.h>
#include <system/scheduler.h>
#include <system/thread.h>

#include <data/types.h>
#include <std/log.h>

bool
verify_pointer(const void* p)
{
	// Verify if pointer is aligned
	// TODO: Is 4 correct?
	if ((uint32)p % 4)
		return false;

	// Verify if pointer lays inside threads stack
	void* sp     = get_stack_pointer(get_curr_thread_index());
	void* max_sp = get_max_stack_pointer(get_curr_thread_index());
	// TODO: lesser or lesser-equal?
	if (!(p < sp && p >= max_sp))
		return false;

	return true;
}

bool
verify_func_pointer(const void(* p)(void))
{
	// Verify if pointer is aligned
	// TODO: Is 4 correct?
	if ((uint32)p % 4)
		return false;

	// TODO: Verify if valid location

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

// Put pointer to memory when char should be placed.
static void
exec_syscall_get_char(struct registers* regs)
{
	char* c = (char*)regs->gr.r0;
	if (!verify_pointer(c)) {
		log(WARNING, "Thread passed invalid pointer. Killing.");
		kill_current_thread(regs);
		return;
	}
	*c = uart_pop_char();
}

// Put pointer to char that should be printed in r0.
static void
exec_syscall_put_char(struct registers* regs)
{
	char* c = (char*)regs->gr.r0;
	if (!verify_pointer(c)) {
		log(WARNING, "Thread passed invalid pointer. Killing.");
		kill_current_thread(regs);
		return;
	}
	uart_put_char(*c);
}

static void
exec_syscall_wait(struct registers* regs)
{
	// TODO
}

static void
exec_syscall_create_thread(struct registers* regs)
{
	// TODO: When incrementing sp, do we need to respect alignment?
	void* sp = (void*)regs->gr.r0;
	if (!verify_pointer(sp)) {
		log(WARNING, "Thread passed invalid pointer. Killing.");
		kill_current_thread(regs);
		return;
	}
	void (*func)(void*) = (void (*)(void*))sp;
	sp += sizeof(func);
	if (!verify_func_pointer(func) || !verify_pointer(sp)) {
		log(WARNING, "Thread passed invalid pointer. Killing.");
		kill_current_thread(regs);
		return;
	}
	const void* args = (void*)sp;
	sp += sizeof(args);
	if (!verify_pointer(args) || !verify_pointer(sp)) {
		log(WARNING, "Thread passed invalid pointer. Killing.");
		kill_current_thread(regs);
		return;
	}
	size_t args_size = *(size_t*)sp;
	sp += sizeof(args);

	thread_create(func, args, args_size);
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
		exec_syscall_get_char(regs);
		break;
	case PUT_CHAR:
		exec_syscall_put_char(regs);
		break;
	case WAIT:
		exec_syscall_wait(regs);
		break;
	case CREATE_THREAD:
		exec_syscall_create_thread(regs);
		break;
	default:
		print_registers(regs, "Software Interrupt", "Killing thread.", "");
		kill_current_thread(regs);
		break;
	}
}
