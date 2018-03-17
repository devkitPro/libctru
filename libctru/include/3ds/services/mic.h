/**
 * @file mic.h
 * @brief MIC (Microphone) service.
 */
#pragma once

/// Microphone audio encodings.
typedef enum
{
	MICU_ENCODING_PCM8 = 0,         ///< Unsigned 8-bit PCM.
	MICU_ENCODING_PCM16 = 1,        ///< Unsigned 16-bit PCM.
	MICU_ENCODING_PCM8_SIGNED = 2,  ///< Signed 8-bit PCM.
	MICU_ENCODING_PCM16_SIGNED = 3, ///< Signed 16-bit PCM.
} MICU_Encoding;

/// Microphone audio sampling rates.
typedef enum
{
	MICU_SAMPLE_RATE_32730 = 0, ///< 32728.498 Hz
	MICU_SAMPLE_RATE_16360 = 1, ///< 16364.479 Hz
	MICU_SAMPLE_RATE_10910 = 2, ///< 10909.499 Hz
	MICU_SAMPLE_RATE_8180 =  3, ///< 8182.1245 Hz
} MICU_SampleRate;

/**
 * @brief Initializes MIC.
 * @param size Shared memory buffer to write audio data to. Must be aligned to 0x1000 bytes.
 * @param handle Size of the shared memory buffer.
 */
Result micInit(u8* buffer, u32 bufferSize);

/// Exits MIC.
void micExit(void);

/**
 * @brief Gets the size of the sample data area within the shared memory buffer.
 * @return The sample data's size.
 */
u32 micGetSampleDataSize(void);

/**
 * @brief Gets the offset within the shared memory buffer of the last sample written.
 * @return The last sample's offset.
 */
u32 micGetLastSampleOffset(void);

/**
 * @brief Maps MIC shared memory.
 * @param size Size of the shared memory.
 * @param handle Handle of the shared memory.
 */
Result MICU_MapSharedMem(u32 size, Handle handle);

/// Unmaps MIC shared memory.
Result MICU_UnmapSharedMem(void);

/**
 * @brief Begins sampling microphone input.
 * @param encoding Encoding of outputted audio.
 * @param sampleRate Sample rate of outputted audio.
 * @param sharedMemAudioOffset Offset to write audio data to in the shared memory buffer.
 * @param sharedMemAudioSize Size of audio data to write to the shared memory buffer. This should be at most "bufferSize - 4".
 * @param loop Whether to loop back to the beginning of the buffer when the end is reached.
 */
Result MICU_StartSampling(MICU_Encoding encoding, MICU_SampleRate sampleRate, u32 offset, u32 size, bool loop);

/**
 * @brief Adjusts the configuration of the current sampling session.
 * @param sampleRate Sample rate of outputted audio.
 */
Result MICU_AdjustSampling(MICU_SampleRate sampleRate);

/// Stops sampling microphone input.
Result MICU_StopSampling(void);

/**
 * @brief Gets whether microphone input is currently being sampled.
 * @param sampling Pointer to output the sampling state to.
 */
Result MICU_IsSampling(bool* sampling);

/**
 * @brief Gets an event handle triggered when the shared memory buffer is full.
 * @param handle Pointer to output the event handle to.
 */
Result MICU_GetEventHandle(Handle* handle);

/**
 * @brief Sets the microphone's gain.
 * @param gain Gain to set.
 */
Result MICU_SetGain(u8 gain);

/**
 * @brief Gets the microphone's gain.
 * @param gain Pointer to output the current gain to.
 */
Result MICU_GetGain(u8* gain);

/**
 * @brief Sets whether the microphone is powered on.
 * @param power Whether the microphone is powered on.
 */
Result MICU_SetPower(bool power);

/**
 * @brief Gets whether the microphone is powered on.
 * @param power Pointer to output the power state to.
 */
Result MICU_GetPower(bool* power);

/**
 * @brief Sets whether to clamp microphone input.
 * @param clamp Whether to clamp microphone input.
 */
Result MICU_SetClamp(bool clamp);

/**
 * @brief Gets whether to clamp microphone input.
 * @param clamp Pointer to output the clamp state to.
 */
Result MICU_GetClamp(bool* clamp);

/**
 * @brief Sets whether to allow sampling when the shell is closed.
 * @param allowShellClosed Whether to allow sampling when the shell is closed.
 */
Result MICU_SetAllowShellClosed(bool allowShellClosed);
