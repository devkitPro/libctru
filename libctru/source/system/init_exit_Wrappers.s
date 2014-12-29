	.text
	.arm
	.cpu mpcore

	.global	initSystem
	.type	initSystem STT_FUNC
@---------------------------------------------------------------------------------
initSystem:
@---------------------------------------------------------------------------------
	adr	r0, saved_lr
	str	lr, [r0]
	str	sp, [r0,#4]
	bl	__ctru_initSystem
	ldr	r0,=fake_heap_start
	ldr	sp, [r0]
	ldr	pc, saved_lr

saved_lr:
	.word	0
saved_stack:
	.word	0


	.global	__ctru_exit
	.type	__ctru_exit STT_FUNC
@---------------------------------------------------------------------------------
__ctru_exit:
@---------------------------------------------------------------------------------
	ldr	sp, saved_stack
	b	__libctru_exit
