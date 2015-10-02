#pragma once

enum
{
	NDSP_ENCODING_PCM8 = 0,
	NDSP_ENCODING_PCM16,
	NDSP_ENCODING_ADPCM, // DSPADPCM (GameCube format)
};

#define NDSP_CHANNELS(n)  ((u32)(n) & 3)
#define NDSP_ENCODING(n) (((u32)(n) & 3) << 2)

enum
{
	NDSP_FORMAT_MONO_PCM8    = NDSP_CHANNELS(1) | NDSP_ENCODING(NDSP_ENCODING_PCM8),
	NDSP_FORMAT_MONO_PCM16   = NDSP_CHANNELS(1) | NDSP_ENCODING(NDSP_ENCODING_PCM16),
	NDSP_FORMAT_MONO_ADPCM   = NDSP_CHANNELS(1) | NDSP_ENCODING(NDSP_ENCODING_ADPCM),
	NDSP_FORMAT_STEREO_PCM8  = NDSP_CHANNELS(2) | NDSP_ENCODING(NDSP_ENCODING_PCM8),
	NDSP_FORMAT_STEREO_PCM16 = NDSP_CHANNELS(2) | NDSP_ENCODING(NDSP_ENCODING_PCM16),

	NDSP_FORMAT_PCM8  = NDSP_FORMAT_MONO_PCM8,
	NDSP_FORMAT_PCM16 = NDSP_FORMAT_MONO_PCM16,
	NDSP_FORMAT_ADPCM = NDSP_FORMAT_MONO_ADPCM,

	// Flags
	NDSP_FRONT_BYPASS             = BIT(4),
	NDSP_3D_SURROUND_PREPROCESSED = BIT(6), //?
};

// Basic channel operation
void ndspChnReset(int id);
void ndspChnInitParams(int id);
bool ndspChnIsPlaying(int id);
u32  ndspChnGetSamplePos(int id);
u16  ndspChnGetWaveBufSeq(int id);

// Configuration
void ndspChnSetFormat(int id, u16 format);
void ndspChnSetInterp(int id, int type);
void ndspChnSetRate(int id, float rate);
void ndspChnSetMix(int id, float mix[12]);
void ndspChnSetAdpcmCoefs(int id, u16 coefs[16]);

// Wave buffers
void ndspChnWaveBufClear(int id);
void ndspChnWaveBufAdd(int id, ndspWaveBuf* buf);

// IIR filters
void ndspChnIirMonoSetEnable(int id, bool enable);
//   ndspChnIirMonoSetParams
void ndspChnIirBiquadSetEnable(int id, bool enable);
//   ndspChnIirBiquadSetParams
