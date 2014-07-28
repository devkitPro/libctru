#ifndef SOC_H
#define SOC_H

Result SOC_Initialize(u32 *context_addr, u32 context_size);//Example context_size: 0x48000. The specified context buffer can no longer be accessed by the process which called this function, since the userland permissions for this block are set to no-access.
Result SOC_Shutdown();
int SOC_GetErrno();

#endif
