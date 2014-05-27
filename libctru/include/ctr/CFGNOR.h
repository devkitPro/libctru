#ifndef CFGNOR_H
#define CFGNOR_H

Result CFGNOR_Initialize(u8 value);
Result CFGNOR_Shutdown();
Result CFGNOR_ReadData(u32 offset, u32 *buf, u32 size);
Result CFGNOR_WriteData(u32 offset, u32 *buf, u32 size);
Result CFGNOR_DumpFlash(u32 *buf, u32 size);
Result CFGNOR_WriteFlash(u32 *buf, u32 size);

#endif

