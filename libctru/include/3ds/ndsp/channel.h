/**
 * @file channel.h
 * @brief Functions for interacting with DSP audio channels.
 */
#pragma once

///@name Data types
///@{
/// Supported sample encodings.
enum
{
	NDSP_ENCODING_PCM8 = 0, ///< PCM8
	NDSP_ENCODING_PCM16,    ///< PCM16
	NDSP_ENCODING_ADPCM,    ///< DSPADPCM (GameCube format)
};

/// Specifies the number of channels used in a sample.
#define NDSP_CHANNELS(n)  ((u32)(n) & 3)
/// Specifies the encoding used in a sample.
#define NDSP_ENCODING(n) (((u32)(n) & 3) << 2)

/// Channel format flags for use with ndspChnSetFormat.
enum
{
	NDSP_FORMAT_MONO_PCM8    = NDSP_CHANNELS(1) | NDSP_ENCODING(NDSP_ENCODING_PCM8),  ///< Buffer contains Mono   PCM8.
	NDSP_FORMAT_MONO_PCM16   = NDSP_CHANNELS(1) | NDSP_ENCODING(NDSP_ENCODING_PCM16), ///< Buffer contains Mono   PCM16.
	NDSP_FORMAT_MONO_ADPCM   = NDSP_CHANNELS(1) | NDSP_ENCODING(NDSP_ENCODING_ADPCM), ///< Buffer contains Mono   ADPCM.
	NDSP_FORMAT_STEREO_PCM8  = NDSP_CHANNELS(2) | NDSP_ENCODING(NDSP_ENCODING_PCM8),  ///< Buffer contains Stereo PCM8.
	NDSP_FORMAT_STEREO_PCM16 = NDSP_CHANNELS(2) | NDSP_ENCODING(NDSP_ENCODING_PCM16), ///< Buffer contains Stereo PCM16.

	NDSP_FORMAT_PCM8  = NDSP_FORMAT_MONO_PCM8,  ///< (Alias) Buffer contains Mono PCM8.
	NDSP_FORMAT_PCM16 = NDSP_FORMAT_MONO_PCM16, ///< (Alias) Buffer contains Mono PCM16.
	NDSP_FORMAT_ADPCM = NDSP_FORMAT_MONO_ADPCM, ///< (Alias) Buffer contains Mono ADPCM.

	// Flags
	NDSP_FRONT_BYPASS             = BIT(4), ///< Front bypass.
	NDSP_3D_SURROUND_PREPROCESSED = BIT(6), ///< (?) Unknown, under research
};

/// Interpolation types.
typedef enum
{
	NDSP_INTERP_POLYPHASE = 0, ///< Polyphase interpolation
	NDSP_INTERP_LINEAR    = 1, ///< Linear interpolation
	NDSP_INTERP_NONE      = 2, ///< No interpolation
} ndspInterpType;

///@}

///@name Basic channel operation
///@{
/**
 * @brief Resets a channel.
 * @param id ID of the channel (0..23).
 */
void ndspChnReset(int id);

/**
 * @brief Initializes the parameters of a channel.
 * @param id ID of the channel (0..23).
 */
void ndspChnInitParams(int id);

/**
 * @brief Checks whether a channel is currently playing.
 * @param id ID of the channel (0..23).
 * @return Whether the channel is currently playing.
 */
bool ndspChnIsPlaying(int id);

/**
 * @brief Gets the current sample position of a channel.
 * @param id ID of the channel (0..23).
 * @return The channel's sample position.
 */
u32  ndspChnGetSamplePos(int id);

/**
 * @brief Gets the sequence ID of the wave buffer that is currently playing in a channel.
 * @param id ID of the channel (0..23).
 * @return The sequence ID of the wave buffer.
 */
u16  ndspChnGetWaveBufSeq(int id);
///@}

///@name Configuration
///@{
/**
 * @brief Sets the format of a channel.
 * @param id ID of the channel (0..23).
 * @param format Format to use.
 */
void ndspChnSetFormat(int id, u16 format);

/**
 * @brief Sets the interpolation type of a channel.
 * @param id ID of the channel (0..23).
 * @param type Interpolation type to use.
 */
void ndspChnSetInterp(int id, ndspInterpType type);

/**
 * @brief Sets the sample rate of a channel.
 * @param id ID of the channel (0..23).
 * @param rate Sample rate to use.
 */
void ndspChnSetRate(int id, float rate);

/**
 * @brief Sets the mix parameters (volumes) of a channel.
 * @param id ID of the channel (0..23).
 * @param mix Mix parameters to use. Working hypothesis:
 *   - 0: Front left volume.
 *   - 1: Front right volume.
 *   - 2: Back left volume:
 *   - 3: Back right volume:
 *   - 4..7: Same as 0..3, but for auxiliary output 0.
 *   - 8..11: Same as 0..3, but for auxiliary output 1.
 */
void ndspChnSetMix(int id, float mix[12]);

/**
 * @brief Sets the DSPADPCM coefficients of a channel.
 * @param id ID of the channel (0..23).
 * @param coefs DSPADPCM coefficients to use.
 */
void ndspChnSetAdpcmCoefs(int id, u16 coefs[16]);
///@}

///@name Wave buffers
///@{
/**
 * @brief Clears the wave buffer queue of a channel and stops playback.
 * @param id ID of the channel (0..23).
 */
void ndspChnWaveBufClear(int id);

/**
 * @brief Adds a wave buffer to the wave buffer queue of a channel.
 * @remark If the channel's wave buffer queue was empty before the use of this function, playback is started.
 * @param id ID of the channel (0..23).
 * @param buf Wave buffer to add.
 */
void ndspChnWaveBufAdd(int id, ndspWaveBuf* buf);
///@}

///@name IIR filters
///@{
/**
 * @brief Configures whether the IIR monopole filter of a channel is enabled.
 * @param id ID of the channel (0..23).
 * @param enable Whether to enable the IIR monopole filter.
 */
void ndspChnIirMonoSetEnable(int id, bool enable);
//   ndspChnIirMonoSetParams
/**
 * @brief Configures whether the IIR biquad filter of a channel is enabled.
 * @param id ID of the channel (0..23).
 * @param enable Whether to enable the IIR biquad filter.
 */
void ndspChnIirBiquadSetEnable(int id, bool enable);
//   ndspChnIirBiquadSetParams
///@}
