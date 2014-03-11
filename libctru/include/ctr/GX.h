#ifndef GX_H
#define GX_H

#define GX_BUFFER_DIM(w, h) (((h)<<16)|((w)&0xFFFF))

Result GX_RequestDma(u32* gxbuf, u32* src, u32* dst, u32 length);
Result GX_SetCommandList_Last(u32* gxbuf, u32* buf0a, u32 buf0s, u8 flags);
Result GX_SetMemoryFill(u32* gxbuf, u32* buf0a, u32 buf0s, u32 buf0d, u16 width0, u32* buf1a, u32 buf1s, u32 buf1d, u16 width1);
Result GX_SetDisplayTransfer(u32* gxbuf, u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 flags);
Result GX_SetTextureCopy(u32* gxbuf, u32* inadr, u32 indim, u32* outadr, u32 outdim, u32 size, u32 flags);
Result GX_SetCommandList_First(u32* gxbuf, u32* buf0a, u32 buf0s, u32* buf1a, u32 buf1s, u32* buf2a, u32 buf2s);

#endif
