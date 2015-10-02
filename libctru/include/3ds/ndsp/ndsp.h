#pragma once

typedef struct
{
	u16 index;
	s16 history0, history1;
} ndspAdpcmData;

typedef struct tag_ndspWaveBuf ndspWaveBuf;

struct tag_ndspWaveBuf
{
	union
	{
		s8*  data_pcm8;
		s16* data_pcm16;
		u8*  data_adpcm;
		u32  data_vaddr;
	};
	u32 nsamples;
	ndspAdpcmData* adpcm_data;

	u32  offset; // only used for capture
	bool looping;
	u8   padding;

	// The following fields are used internally
	u16 sequence_id;
	ndspWaveBuf* next;
};

typedef void (*ndspCallback)(void* data);
typedef void (*ndspAuxCallback)(void* data, int nsamples, void* samples[4]);

// Initialization and basic operations
void   ndspUseComponent(const void* binary, u32 size, u16 progMask, u16 dataMask);
Result ndspInit(void);
void   ndspExit(void);
u32    ndspGetDroppedFrames(void);
u32    ndspGetFrameCount(void);

// General parameters
void ndspSetMasterVol(float volume);
void ndspSetOutputMode(int mode);
void ndspSetClippingMode(int mode);
void ndspSetOutputCount(int count);
void ndspSetCapture(ndspWaveBuf* capture);
void ndspSetCallback(ndspCallback callback, void* data);

// Surround
void ndspSurroundSetDepth(u16 depth);
void ndspSurroundSetPos(u16 pos);
void ndspSurroundSetRearRatio(u16 ratio);

// Auxiliary output
void ndspAuxSetEnable(int id, bool enable);
void ndspAuxSetFrontBypass(int id, bool bypass);
void ndspAuxSetVolume(int id, float volume);
void ndspAuxSetCallback(int id, ndspAuxCallback callback, void* data);
