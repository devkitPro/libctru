.section ".text"
.arm
.align 4
.global _vboMemcpy50

# r0 : dst
# r1 : src
# fixed size 0x50
_vboMemcpy50:
	push {r4-r11}
	ldmia r1!, {r2-r12}
	stmia r0!, {r2-r12}
	ldmia r1!, {r2-r12}
	stmia r0!, {r2-r12}
	pop {r4-r11}
	bx lr
