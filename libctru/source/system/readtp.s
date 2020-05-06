#include <3ds/asminc.h>

.arm

BEGIN_ASM_FUNC __aeabi_read_tp
	mrc p15, 0, r0, c13, c0, 3
	ldr r0, [r0, #0xC] @ Read ThreadVars.tls_tp
	bx lr
END_ASM_FUNC
