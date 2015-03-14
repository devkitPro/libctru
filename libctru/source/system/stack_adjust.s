
	.arm
	.align 2

	.global	initSystem
	.type	initSystem,	%function

initSystem:
	ldr	r2, =saved_stack
	str	sp, [r2]
	str	lr, [r2,#4]

	bl	__libctru_init

	ldr	r2, =fake_heap_start
	ldr	sp, [r2]

	ldr	r3, =__stacksize__
	ldr	r3, [r3]
	add sp, sp, r3
	add	sp, sp, #7
	bics	sp, sp, #7
	str	sp, [r2]


	bl	__appInit
	bl	__libc_init_array

	ldr	r2, =saved_stack
	ldr	lr, [r2,#4]
 	bx	lr


	.global	__ctru_exit
	.type	__ctru_exit,	%function

__ctru_exit:
	bl	__libc_fini_array
	bl	__appExit

	ldr	r2, =saved_stack
	ldr	sp, [r2]
	b	__libctru_exit

	.data
	.align 2
__stacksize__:
	.word	32 * 1024
	.weak	__stacksize__


	.bss
	.align 2
saved_stack:
	.space 8


