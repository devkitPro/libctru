#pragma once
#include <3ds/types.h>

#define CSND_NUM_CHANNELS 32

#define CSND_TIMER(n) (0x3FEC3FC / ((u32)(n)))

// Convert a vol-pan pair into a left/right volume pair used by the hardware
static inline u32 CSND_VOL(float vol, float pan)
{
	if (vol < 0.0) vol = 0.0;
	else if (vol > 1.0) vol = 1.0;

	float rpan = (pan+1) / 2;
	if (rpan < 0.0) rpan = 0.0;
	else if (rpan > 1.0) rpan = 1.0;

	u32 lvol = vol*(1-rpan) * 0x8000;
	u32 rvol = vol*rpan * 0x8000;
	return lvol | (rvol << 16);
}

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

enum
{
	CAPTURE_REPEAT = 0,
	CAPTURE_ONE_SHOT = BIT(0),
	CAPTURE_FORMAT_16BIT = 0,
	CAPTURE_FORMAT_8BIT = BIT(1),
	CAPTURE_ENABLE = BIT(15),
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
		u32 unknownZero;
	};
} CSND_ChnInfo;

typedef union
{
	u32 value[2];
	struct
	{
		u8 active;
		u8 _pad1;
		u16 _pad2;
		u32 unknownZero;
	};
} CSND_CapInfo;

// See here regarding CSND shared-mem commands, etc: http://3dbrew.org/wiki/CSND_Shared_Memory

extern vu32* csndSharedMem;
extern u32 csndSharedMemSize;
extern u32 csndChannels; // Bitmask of channels that are allowed for usage

Result CSND_AcquireCapUnit(u32* capUnit);
Result CSND_ReleaseCapUnit(u32 capUnit);

Result CSND_Reset(void); // Currently breaks sound, don't use for now!

Result csndInit(void);
Result csndExit(void);

u32* csndAddCmd(int cmdid); // Adds a command to the list and returns the buffer to which write its arguments.
void csndWriteCmd(int cmdid, u8* cmdparams); // As above, but copies the arguments from an external buffer
Result csndExecCmds(bool waitDone);

void CSND_SetPlayStateR(u32 channel, u32 value);
void CSND_SetPlayState(u32 channel, u32 value);
void CSND_SetEncoding(u32 channel, u32 value);
void CSND_SetBlock(u32 channel, int block, u32 physaddr, u32 size);
void CSND_SetLooping(u32 channel, u32 value);
void CSND_SetBit7(u32 channel, bool set);
void CSND_SetInterp(u32 channel, bool interp);
void CSND_SetDuty(u32 channel, u32 duty);
void CSND_SetTimer(u32 channel, u32 timer);
void CSND_SetVol(u32 channel, u32 chnVolumes, u32 capVolumes);
void CSND_SetAdpcmState(u32 channel, int block, int sample, int index);
void CSND_SetAdpcmReload(u32 channel, bool reload);
void CSND_SetChnRegs(u32 flags, u32 physaddr0, u32 physaddr1, u32 totalbytesize, u32 chnVolumes, u32 capVolumes);
void CSND_SetChnRegsPSG(u32 flags, u32 chnVolumes, u32 capVolumes, u32 duty);
void CSND_SetChnRegsNoise(u32 flags, u32 chnVolumes, u32 capVolumes);

void CSND_CapEnable(u32 capUnit, bool enable);
void CSND_CapSetRepeat(u32 capUnit, bool repeat);
void CSND_CapSetFormat(u32 capUnit, bool eightbit);
void CSND_CapSetBit2(u32 capUnit, bool set);
void CSND_CapSetTimer(u32 capUnit, u32 timer);
void CSND_CapSetBuffer(u32 capUnit, u32 addr, u32 size);
void CSND_SetCapRegs(u32 capUnit, u32 flags, u32 addr, u32 size);

Result CSND_SetDspFlags(bool waitDone);
Result CSND_UpdateInfo(bool waitDone);

/**
 * @param vol The volume, ranges from 0.0 to 1.0 included
 * @param pan The pan, ranges from -1.0 to 1.0 included
 */
Result csndPlaySound(int chn, u32 flags, u32 sampleRate, float vol, float pan, void* data0, void* data1, u32 size);

void csndGetDspFlags(u32* outSemFlags, u32* outIrqFlags); // Requires previous CSND_UpdateInfo()
CSND_ChnInfo* csndGetChnInfo(u32 channel); // Requires previous CSND_UpdateInfo()
CSND_CapInfo* csndGetCapInfo(u32 capUnit); // Requires previous CSND_UpdateInfo()

Result csndGetState(u32 channel, CSND_ChnInfo* out);
Result csndIsPlaying(u32 channel, u8* status);
