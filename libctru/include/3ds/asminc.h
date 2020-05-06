#pragma once

#if !__ASSEMBLER__
    #error This header file is only for use in assembly files!
#endif // !__ASSEMBLER__

.macro BEGIN_ASM_FUNC name, linkage=global, section=text
    .section        .\section\().\name, "ax", %progbits
    .align          2
    .\linkage       \name
    .type           \name, %function
    .func           \name
    .cfi_sections   .debug_frame
    .cfi_startproc
    \name:
.endm

.macro END_ASM_FUNC
    .cfi_endproc
    .endfunc
.endm
