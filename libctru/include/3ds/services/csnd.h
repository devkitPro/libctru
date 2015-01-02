#pragma once
#include <3ds/types.h>

#define CSND_NUM_CHANNELS 32
#define CSND_SHAREDMEM_DEFAULT 0x10004000

#define CSND_TIMER(n) (0x3FEC3FC / ((u32)(n)))

typedef enum
{
    CSND_ENCODING_PCM8,
    CSND_ENCODING_PCM16,
    CSND_ENCODING_ADPCM, // IMA-ADPCM
    CSND_ENCODING_PSG, // Similar to DS?
} CSND_ENCODING;

#define SOUND_CHANNEL(n) ((u32)(n) & 0x1F)
#define SOUND_FORMAT(n) ((u32)(n) << 12)

enum
{
	SOUND_LINEAR_INTERP = BIT(6),
	SOUND_REPEAT = BIT(10),
	SOUND_CONST_BLOCK_SIZE = BIT(11),
	SOUND_ONE_SHOT = 0,
	SOUND_FORMAT_8BIT = SOUND_FORMAT(CSND_ENCODING_PCM8),
	SOUND_FORMAT_16BIT = SOUND_FORMAT(CSND_ENCODING_PCM16),
	SOUND_FORMAT_ADPCM = SOUND_FORMAT(CSND_ENCODING_ADPCM),
	SOUND_FORMAT_PSG = SOUND_FORMAT(CSND_ENCODING_PSG),
	SOUND_ENABLE = BIT(14),
};

typedef union
{
	u32 value[3];
	struct
	{
		u8 active;
		u8 _pad1;
		u16 _pad2;
		s16 adpcmSample;
		u8 adpcmIndex;
		u8 _pad3;
		u32 samplePAddr;
	};
} CSND_ChnInfo;

// See here regarding CSND shared-mem commands, etc: http://3dbrew.org/wiki/CSND_Shared_Memory

extern vu32* csndSharedMem;
extern u32 csndSharedMemSize;
extern u32 csndChannels; // Bitmask of channels that are allowed for usage

Result csndInit(void);
Result csndExit(void);

void csndWriteChnCmd(int cmdid, u8 *cmdparams);
Result csndExecChnCmds(bool waitDone);

void CSND_ChnSetPlayStateR(u32 channel, u32 value);
void CSND_ChnSetPlayState(u32 channel, u32 value);
void CSND_ChnSetBlock(u32 channel, int block, u32 physaddr, u32 size);
void CSND_ChnSetVol(u32 channel, u16 left, u16 right);
void CSND_ChnSetTimer(u32 channel, u32 timer);
void CSND_ChnConfig(u32 flags, u32 physaddr0, u32 physaddr1, u32 totalbytesize);

Result CSND_UpdateChnInfo(bool waitDone);

Result csndChnPlaySound(int chn, u32 flags, u32 sampleRate, void* data0, void* data1, u32 size);

CSND_ChnInfo* csndChnGetInfo(u32 channel); // Requires previous CSND_UpdateChnInfo()

Result csndChnGetState(u32 channel, u32 *out);
Result csndChnIsPlaying(u32 channel, u8 *status);
