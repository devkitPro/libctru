.section ".init"
.arm
.align 4
.global _init
.global _start

_start:
	blx __libc_init_array
	blx main

_init:
	bx lr
