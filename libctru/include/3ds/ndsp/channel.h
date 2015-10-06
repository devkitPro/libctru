/**
 * @file channel.h
 * @brief NDSP channels.
 */
#pragma once

///@name Data types
///@{
/// Supported NDSP encodings.
enum
{
	NDSP_ENCODING_PCM8 = 0, ///< PCM8
	NDSP_ENCODING_PCM16,    ///< PCM16
	NDSP_ENCODING_ADPCM,    ///< DSPADPCM (GameCube format)
} NDSP_Encoding;

/// Creates a hardware channel value from a channel number.
#define NDSP_CHANNELS(n)  ((u32)(n) & 3)
/// Creates a hardware encoding value from an encoding type.
#define NDSP_ENCODING(n) (((u32)(n) & 3) << 2)

/// NDSP playback flags.
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
	NDSP_3D_SURROUND_PREPROCESSED = BIT(6), ///< Preprocessed 3D surround sound.
} NDSP_Flags;
///@}

///@name Basic channel operation
///@{
/**
 * @brief Resets a channel.
 * @param id ID of the channel.
 */
void ndspChnReset(int id);

/**
 * @brief Initializes the parameters of a channel.
 * @param id ID of the channel.
 */
void ndspChnInitParams(int id);

/**
 * @brief Checks whether a channel is currently playing.
 * @param id ID of the channel.
 * @return Whether the channel is currently playing.
 */
bool ndspChnIsPlaying(int id);

/**
 * @brief Gets the current sample position of a channel.
 * @param id ID of the channel.
 * @return The channel's sample position.
 */
u32  ndspChnGetSamplePos(int id);

/**
 * @brief Gets the current wave buffer sequence position of a channel.
 * @param id ID of the channel.
 * @return The channel's wave buffer sequence position.
 */
u16  ndspChnGetWaveBufSeq(int id);
///@}

///@name Configuration
///@{
/**
 * @brief Sets the format of a channel.
 * @sa NDSP_Encoding
 * @param id ID of the channel.
 * @param format Format to use.
 */
void ndspChnSetFormat(int id, u16 format);

/**
 * @brief Sets the linear interpolation type of a channel.
 * @param id ID of the channel.
 * @param type Linear interpolation type to use.
 */
void ndspChnSetInterp(int id, int type);

/**
 * @brief Sets the sample rate of a channel.
 * @param id ID of the channel.
 * @param rate Sample rate to use.
 */
void ndspChnSetRate(int id, float rate);

/**
 * @brief Sets the mix parameters of a channel.
 * @param id ID of the channel.
 * @param mix Mix parameters to use.
 */
void ndspChnSetMix(int id, float mix[12]);

/**
 * @brief Sets the ADPCM coefficients of a channel.
 * @param id ID of the channel.
 * @param coefs ADPCM coefficients to use.
 */
void ndspChnSetAdpcmCoefs(int id, u16 coefs[16]);
///@}

///@name Wave buffers
///@{
/**
 * @brief Clears the wave buffers of a channel.
 * @param id ID of the channel.
 */
void ndspChnWaveBufClear(int id);

/**
 * @brief Adds a wave buffer to a channel.
 * @param id ID of the channel.
 * @param buf Wave buffer to add.
 */
void ndspChnWaveBufAdd(int id, ndspWaveBuf* buf);
///@}

///@name IIR filters
///@{
/**
 * @brief Sets whether the mono filter of a channel is enabled.
 * @param id ID of the channel.
 * @param enable Whether to enable the mono filter.
 */
void ndspChnIirMonoSetEnable(int id, bool enable);
//   ndspChnIirMonoSetParams
/**
 * @brief Sets whether the biquad filter of a channel is enabled.
 * @param id ID of the channel.
 * @param enable Whether to enable the biquad filter.
 */
void ndspChnIirBiquadSetEnable(int id, bool enable);
//   ndspChnIirBiquadSetParams
///@}
