#include <system/calls.h>

#include <arch/armv7/registers.h>

#include <driver/uart.h>

#include <system/isr.h>
#include <system/ivt.h>
#include <system/scheduler.h>
#include <system/thread.h>

#include <data/types.h>
#include <std/log.h>

#include <system/assert.h>

static bool
verify_pointer(const void* p)
{
	// Verify if pointer is aligned
	if ((uint32)p % 4)
		return false;

	// Verify if pointer lays inside threads stack
	void* sp     = get_stack_pointer(get_curr_thread_index());
	void* max_sp = get_max_stack_pointer(get_curr_thread_index());
	if (!(p < sp && p >= max_sp))
		return false;

	return true;
}

static bool
verify_func_pointer(void (*p)())
{
	// Verify if pointer is aligned
	if ((uint32)p % 4)
		return false;

	// TODO: Verify if valid location

	return true;
}

#define SET_SYSCALL_RETURN_VALUE(value) regs->gr.r0 = (value)

// Kill current thread
static void
exec_syscall_exit(struct registers* regs)
{
	kill_current_thread(regs);
}

// Put char into r0.
static void
exec_syscall_get_char(struct registers* regs)
{
	if (uart_queue_is_emtpy()) {
		scheduler_push_uart_read(regs);
		ASSERTM(get_cur_thread_state() == READY,
		        "Thread has not yet received a char.");
	} else {
		SET_SYSCALL_RETURN_VALUE(uart_pop_char());
	}
}

// Print char store in r0
static void
exec_syscall_put_char(struct registers* regs)
{
	uart_put_char(regs->gr.r0);
}

static void
exec_syscall_wait(struct registers* regs)
{
	pause_cur_thread(regs->gr.r0, regs);
}

static void
exec_syscall_create_thread(struct registers* regs)
{
	void (*func)(void*) = (void (*)(void*))regs->gr.r0;
	if (!verify_func_pointer(func)) {
		klog(WARNING, "Thread passed invalid pointer. Killing.");
		kill_current_thread(regs);
		return;
	}
	void* args = (void*)regs->gr.r1;
	if (!verify_pointer(args)) {
		klog(WARNING, "Thread passed invalid pointer. Killing.");
		kill_current_thread(regs);
		return;
	}
	size_t args_size = regs->gr.r2;

	thread_create(func, args, args_size);
}

/***************************
 * END OF STATIC FUNCTIONS *
 ***************************/

uint16
get_syscall_id(uint32 lr)
{
	// Software interrupts are recognized later in the pipeline
	lr -= 4;
	// Get last two bytes from `svc` instruction at `lr` for the immediate value
	return (*(uint32*)lr & 0xFF);
}

// See user/include/sys/calls.h and user/src/sys/calls.S
void
exec_syscall(uint16 id, struct registers* regs)
{
	klog(LOG, "Syscall with id %i called.", id);
	switch (id) {
	case EXIT:
		exec_syscall_exit(regs);
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
