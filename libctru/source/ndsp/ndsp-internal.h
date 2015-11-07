#pragma once
#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/os.h>
#include <3ds/synchronization.h>
#include <3ds/services/dsp.h>
#include <3ds/services/apt.h>
#include <3ds/ndsp/ndsp.h>

extern u16 ndspFrameId, ndspBufferCurId, ndspBufferId;
extern void* ndspVars[16][2];

typedef struct
{
	u32 paddr, sampleCount;
	ndspAdpcmData adpcmData;
	u8 hasAdpcmData, looping;
	u16 seqId, padding;
} DspChnBuf;

typedef struct
{
	u32 flags;
	float mix[12];
	float rate;
	u8 rim[2];
	u16 iirFilterType;
	u16 iirFilter_mono[2];
	u16 iirFilter_biquad[5];
	u16 activeBuffers;
	DspChnBuf buffers[4];
	u32 _pad0;
	u16 playStatus, syncCount;
	u32 unknown;
	u32 _pad1;

	u32 paddr, sampleCount;
	u16 cntFlags;
	ndspAdpcmData adpcmData;
	u16 moreFlags;
	u16 seqId;
} DspChnStruct;

typedef struct
{
	u16 flags, syncCount;
	u32 samplePos;
	u16 curSeqId, lastSeqId;
} DspChnStatus;

typedef struct
{
	u32 flags;
	float masterVol;
	float auxReturnVol[2];
	u16 outBufCount;
	u16 _pad0[2];
	u16 outputMode;
	u16 clippingMode;
	u16 headsetConnected;
	u16 surroundDepth;
	u16 surroundSpeakerPos;
	u16 _pad1;
	u16 rearRatio;
	u16 auxFrontBypass[2];
	u16 auxBusEnable[2];
	u16 dspDelayEffect[2][10];
	u16 dspReverbEffect[2][26];
	u16 syncMode;
	u16 _pad2;
	u32 unknown;
} DspMasterStatus;

static inline u32 ndspiRotateVal(u32 x)
{
	return (x << 16) | (x >> 16);
}

static inline DspChnStruct* ndspiGetChnStruct(int id)
{
	DspChnStruct* them = (DspChnStruct*)ndspVars[1][ndspFrameId&1];
	return &them[id];
}

static inline DspChnStatus* ndspiGetChnStatus(int id)
{
	DspChnStatus* them = (DspChnStatus*)ndspVars[2][ndspBufferId];
	return &them[id];
}

static inline u16* ndspiGetChnAdpcmCoefs(int id)
{
	u16* them = (u16*)ndspVars[3][ndspBufferId];
	return &them[id*16];
}

static inline DspMasterStatus* ndspiGetMasterStatus(void)
{
	return (DspMasterStatus*)ndspVars[4][ndspBufferCurId];
}

void ndspiInitChn(void);
void ndspiDirtyChn(void);
void ndspiUpdateChn(void);
void ndspiReadChnState(void);
