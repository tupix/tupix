// TODO(Aurel): This should not be in here, but rather in a syscall header

getchar:
	push { r0 }
	mov r0, sp

	//syscall(GET_CHAR);
	svc #GET_CHAR

	pop { r0 }
