// TODO(Aurel): This should not be in here, but rather in a syscall header

getchar:
	/* NOTE(Aurel): Whatever is in r0 is being discarded in the handler on the
	 * system side.
	 */
	push { r0 }
	// TODO(Aurel): Maybe replace by: add sp, #4

	// This might be described by: `(syscall(GET_CHAR))(*ret);`
	mov r0, sp
	svc #GET_CHAR

	pop { r0 }
