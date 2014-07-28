#ifndef IR_H
#define IR_H

Result IRU_Initialize(u32 *sharedmem_addr, u32 sharedmem_size);//The permissions for the specified memory is set to RO. This memory must be already mapped.
Result IRU_Shutdown();
Handle IRU_GetServHandle();
Result IRU_SendData(u8 *buf, u32 size, u32 wait);
Result IRU_RecvData(u8 *buf, u32 size, u8 flag, u32 *transfercount, u32 wait);
Result IRU_SetBitRate(u8 value);
Result IRU_GetBitRate(u8 *out);
Result IRU_SetIRLEDState(u32 value);
Result IRU_GetIRLEDRecvState(u32 *out);

#endif

