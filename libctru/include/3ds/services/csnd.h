#pragma once
#include <3ds/types.h>

#define CSND_NUM_CHANNELS 32
#define CSND_SHAREDMEM_DEFAULT 0x10004000

#define CSND_TIMER(n) (0x3FEC3FC / ((u32)(n)))

enum
{
	CSND_ENCODING_PCM8 = 0,
	CSND_ENCODING_PCM16,
	CSND_ENCODING_ADPCM, // IMA-ADPCM
	CSND_ENCODING_PSG, // Similar to DS?
};

enum
{
	CSND_LOOPMODE_MANUAL = 0,
	CSND_LOOPMODE_NORMAL,
	CSND_LOOPMODE_ONESHOT,
	CSND_LOOPMODE_NORELOAD,
};

#define SOUND_CHANNEL(n) ((u32)(n) & 0x1F)
#define SOUND_FORMAT(n) ((u32)(n) << 12)
#define SOUND_LOOPMODE(n) ((u32)(n) << 10)

enum
{
	SOUND_LINEAR_INTERP = BIT(6),
	SOUND_REPEAT = SOUND_LOOPMODE(CSND_LOOPMODE_NORMAL),
	SOUND_ONE_SHOT = SOUND_LOOPMODE(CSND_LOOPMODE_ONESHOT),
	SOUND_FORMAT_8BIT = SOUND_FORMAT(CSND_ENCODING_PCM8),
	SOUND_FORMAT_16BIT = SOUND_FORMAT(CSND_ENCODING_PCM16),
	SOUND_FORMAT_ADPCM = SOUND_FORMAT(CSND_ENCODING_ADPCM),
	SOUND_FORMAT_PSG = SOUND_FORMAT(CSND_ENCODING_PSG),
	SOUND_ENABLE = BIT(14),
};

// Duty cycles for a PSG channel
enum
{
	DutyCycle_0  = 7, /*!<  0.0% duty cycle */
	DutyCycle_12 = 0, /*!<  12.5% duty cycle */
	DutyCycle_25 = 1, /*!<  25.0% duty cycle */
	DutyCycle_37 = 2, /*!<  37.5% duty cycle */
	DutyCycle_50 = 3, /*!<  50.0% duty cycle */
	DutyCycle_62 = 4, /*!<  62.5% duty cycle */
	DutyCycle_75 = 5, /*!<  75.0% duty cycle */
	DutyCycle_87 = 6  /*!<  87.5% duty cycle */
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

Result CSND_AcquireCapUnit(u32* capUnit);
Result CSND_ReleaseCapUnit(u32 capUnit);

Result csndInit(void);
Result csndExit(void);

void csndWriteCmd(int cmdid, u8 *cmdparams);
Result csndExecCmds(bool waitDone);

void CSND_SetPlayStateR(u32 channel, u32 value);
void CSND_SetPlayState(u32 channel, u32 value);
void CSND_SetBlock(u32 channel, int block, u32 physaddr, u32 size);
void CSND_SetVol(u32 channel, u16 left, u16 right);
void CSND_SetTimer(u32 channel, u32 timer);
void CSND_SetDuty(u32 channel, u32 duty);
void CSND_SetAdpcmState(u32 channel, int block, int sample, int index);
void CSND_SetAdpcmReload(u32 channel, bool reload);
void CSND_SetChnRegs(u32 flags, u32 physaddr0, u32 physaddr1, u32 totalbytesize);

void CSND_CapEnable(u32 capUnit, bool enable);
void CSND_CapSetBit(u32 capUnit, int bit, bool state); // Sets bit0..2 in the CNT register, purpose currently unknown
void CSND_CapSetTimer(u32 capUnit, u32 timer);
void CSND_CapSetBuffer(u32 capUnit, u32 paddr, u32 size);

Result CSND_UpdateInfo(bool waitDone);

Result csndPlaySound(int chn, u32 flags, u32 sampleRate, void* data0, void* data1, u32 size);

CSND_ChnInfo* csndGetChnInfo(u32 channel); // Requires previous CSND_UpdateInfo()

Result csndGetState(u32 channel, CSND_ChnInfo* out);
Result csndIsPlaying(u32 channel, u8* status);
