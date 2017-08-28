/*
  gpu.c _ Advanced GPU commands.
*/

#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/gpu/gpu.h>
#include <3ds/gpu/gx.h>
#include <3ds/gpu/shbin.h>

u32* gpuCmdBuf;
u32 gpuCmdBufSize;
u32 gpuCmdBufOffset;

void GPUCMD_AddRawCommands(const u32* cmd, u32 size)
{
	if(!cmd || !size)return;

	memcpy(&gpuCmdBuf[gpuCmdBufOffset], cmd, size*4);
	gpuCmdBufOffset+=size;
}

void GPUCMD_Add(u32 header, const u32* param, u32 paramlength)
{
	if(!paramlength)paramlength=1;
	if(!gpuCmdBuf || gpuCmdBufOffset+paramlength+1>gpuCmdBufSize)
		svcBreak(USERBREAK_PANIC); // Shouldn't happen.

	paramlength--;
	header|=(paramlength&0x7ff)<<20;

	gpuCmdBuf[gpuCmdBufOffset]=param ? param[0] : 0;
	gpuCmdBuf[gpuCmdBufOffset+1]=header;

	if(paramlength)
	{
		if(param)memcpy(&gpuCmdBuf[gpuCmdBufOffset+2], &param[1], paramlength*4);
		else     memset(&gpuCmdBuf[gpuCmdBufOffset+2], 0, paramlength*4);
	}

	gpuCmdBufOffset+=paramlength+2;

	if(paramlength&1)gpuCmdBuf[gpuCmdBufOffset++]=0x00000000; //alignment
}

void GPUCMD_Split(u32** addr, u32* size)
{
	GPUCMD_AddWrite(GPUREG_FINALIZE, 0x12345678);
	if (gpuCmdBufOffset & 3)
		GPUCMD_AddWrite(GPUREG_FINALIZE, 0x12345678); // 16-byte align the buffer

	if (addr) *addr = gpuCmdBuf;
	if (size) *size = gpuCmdBufOffset;

	gpuCmdBuf       += gpuCmdBufOffset;
	gpuCmdBufSize   -= gpuCmdBufOffset;
	gpuCmdBufOffset  = 0;
}

static inline u32 floatrawbits(float f)
{
	union { float f; u32 i; } s;
	s.f = f;
	return s.i;
}

// f16 has:
//  - 1 sign bit
//  - 5 exponent bits
//  - 10 mantissa bits
u32 f32tof16(float f)
{
	u32 i = floatrawbits(f);

	u32 mantissa = (i << 9) >>  9;
	s32 exponent = (i << 1) >> 24;
	u32 sign     = (i << 0) >> 31;

	// Truncate mantissa
	mantissa >>= 13;

	// Re-bias exponent
	exponent = exponent - 127 + 15;
	if (exponent < 0)
	{
		// Underflow: flush to zero
		return sign << 15;
	}
	else if (exponent > 0x1F)
	{
		// Overflow: saturate to infinity
		return sign << 15 | 0x1F << 10;
	}

	return sign << 15 | exponent << 10 | mantissa;
}

// f20 has:
//  - 1 sign bit
//  - 7 exponent bits
//  - 12 mantissa bits
u32 f32tof20(float f)
{
	u32 i = floatrawbits(f);

	u32 mantissa = (i << 9) >>  9;
	s32 exponent = (i << 1) >> 24;
	u32 sign     = (i << 0) >> 31;

	// Truncate mantissa
	mantissa >>= 11;

	// Re-bias exponent
	exponent = exponent - 127 + 63;
	if (exponent < 0)
	{
		// Underflow: flush to zero
		return sign << 19;
	}
	else if (exponent > 0x7F)
	{
		// Overflow: saturate to infinity
		return sign << 19 | 0x7F << 12;
	}

	return sign << 19 | exponent << 12 | mantissa;
}

// f24 has:
//  - 1 sign bit
//  - 7 exponent bits
//  - 16 mantissa bits
u32 f32tof24(float f)
{
	u32 i = floatrawbits(f);

	u32 mantissa = (i << 9) >>  9;
	s32 exponent = (i << 1) >> 24;
	u32 sign     = (i << 0) >> 31;

	// Truncate mantissa
	mantissa >>= 7;

	// Re-bias exponent
	exponent = exponent - 127 + 63;
	if (exponent < 0)
	{
		// Underflow: flush to zero
		return sign << 23;
	}
	else if (exponent > 0x7F)
	{
		// Overflow: saturate to infinity
		return sign << 23 | 0x7F << 16;
	}

	return sign << 23 | exponent << 16 | mantissa;
}

// f31 has:
//  - 1 sign bit
//  - 7 exponent bits
//  - 23 mantissa bits
u32 f32tof31(float f)
{
	u32 i = floatrawbits(f);

	u32 mantissa = (i << 9) >>  9;
	s32 exponent = (i << 1) >> 24;
	u32 sign     = (i << 0) >> 31;

	// Re-bias exponent
	exponent = exponent - 127 + 63;
	if (exponent < 0)
	{
		// Underflow: flush to zero
		return sign << 30;
	}
	else if (exponent > 0x7F)
	{
		// Overflow: saturate to infinity
		return sign << 30 | 0x7F << 23;
	}

	return sign << 30 | exponent << 23 | mantissa;
}
