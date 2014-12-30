	.text
	.arm
	.cpu mpcore

	.global	initSystem
	.type	initSystem STT_FUNC
@---------------------------------------------------------------------------------
initSystem:
@---------------------------------------------------------------------------------
	ldr	r0, =saved_lr
	str	lr, [r0]
	str	sp, [r0,#4]
	bl	__ctru_initSystem
	ldr	r0,=fake_heap_start
	ldr	sp, [r0]
	ldr	r0, =saved_lr
	ldr	pc, [r0]

	.global	__ctru_exit
	.type	__ctru_exit STT_FUNC
@---------------------------------------------------------------------------------
__ctru_exit:
@---------------------------------------------------------------------------------
	ldr	r1, =saved_stack
	ldr	sp, [r1]
	b	__libctru_exit

	.bss
	.align 2
saved_lr:
	.space	4
saved_stack:
	.space	4
