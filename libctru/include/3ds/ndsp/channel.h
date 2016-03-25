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

/**
 * @brief Checks whether a channel is currently paused.
 * @param id ID of the channel (0..23).
 * @return Whether the channel is currently paused.
 */
bool ndspChnIsPaused(int id);

/**
 * @brief Sets the pause status of a channel.
 * @param id ID of the channel (0..23).
 * @param paused Whether the channel is to be paused (true) or unpaused (false).
 */
void ndspChnSetPaused(int id, bool paused);

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
/**
 * @brief Manually sets up the parameters on monopole filter
 * @param id ID of the channel (0..23).
 * @param enable Whether to enable the IIR monopole filter.
 */
bool ndspChnIirMonoSetParamsCustomFilter(int id, float a0, float a1, float b0);
/**
 * @brief Sets the monopole to be a low pass filter. (Note: This is a lower-quality filter than the biquad one.)
 * @param id ID of the channel (0..23).
 * @param f0 Low pass cut-off frequency.
 */
bool ndspChnIirMonoSetParamsLowPassFilter(int id, float f0);
/**
 * @brief Sets the monopole to be a high pass filter. (Note: This is a lower-quality filter than the biquad one.)
 * @param id ID of the channel (0..23).
 * @param f0 High pass cut-off frequency.
 */
bool ndspChnIirMonoSetParamsHighPassFilter(int id, float f0);
/**
 * @brief Configures whether the IIR biquad filter of a channel is enabled.
 * @param id ID of the channel (0..23).
 * @param enable Whether to enable the IIR biquad filter.
 */
void ndspChnIirBiquadSetEnable(int id, bool enable);
/**
 * @brief Manually sets up the parameters of the biquad filter
 * @param id ID of the channel (0..23).
 */
bool ndspChnIirBiquadSetParamsCustomFilter(int id, float a0, float a1, float a2, float b0, float b1, float b2);
/**
 * @brief Sets the biquad to be a low pass filter.
 * @param id ID of the channel (0..23).
 * @param f0 Low pass cut-off frequency.
 * @param Q "Quality factor", typically should be sqrt(2)/2 (i.e. 0.7071).
 */
bool ndspChnIirBiquadSetParamsLowPassFilter(int id, float f0, float Q);
/**
 * @brief Sets the biquad to be a high pass filter.
 * @param id ID of the channel (0..23).
 * @param f0 High pass cut-off frequency.
 * @param Q "Quality factor", typically should be sqrt(2)/2 (i.e. 0.7071).
 */
bool ndspChnIirBiquadSetParamsHighPassFilter(int id, float f0, float Q);
/**
 * @brief Sets the biquad to be a band pass filter.
 * @param id ID of the channel (0..23).
 * @param f0 Mid-frequency.
 * @param Q "Quality factor", typically should be sqrt(2)/2 (i.e. 0.7071).
 */
bool ndspChnIirBiquadSetParamsBandPassFilter(int id, float f0, float Q);
/**
 * @brief Sets the biquad to be a notch filter.
 * @param id ID of the channel (0..23).
 * @param f0 Notch frequency.
 * @param Q "Quality factor", typically should be sqrt(2)/2 (i.e. 0.7071).
 */
bool ndspChnIirBiquadSetParamsNotchFilter(int id, float f0, float Q);
/**
 * @brief Sets the biquad to be a peaking equalizer.
 * @param id ID of the channel (0..23).
 * @param f0 Central frequency.
 * @param Q "Quality factor", typically should be sqrt(2)/2 (i.e. 0.7071).
 * @param gain Amount of gain (raw value = 10 ^ dB/40)
 */
bool ndspChnIirBiquadSetParamsPeakingEqualizer(int id, float f0, float Q, float gain);
///@}
