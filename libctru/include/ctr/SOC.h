#ifndef SOC_H
#define SOC_H

Result SOC_Initialize(u32 *context_addr, u32 context_size);
Result SOC_Shutdown();
int SOC_GetErrno();

#endif
