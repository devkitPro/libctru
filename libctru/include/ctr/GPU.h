#ifndef GPU_H
#define GPU_H

void GPU_Init(Handle *gsphandle);
void GPU_SetCommandBuffer(u32* adr, u32 size, u32 offset);
void GPU_RunCommandBuffer(u32* gxbuf);
void GPU_AddCommand(u32* cmd, u32 length);

#endif
