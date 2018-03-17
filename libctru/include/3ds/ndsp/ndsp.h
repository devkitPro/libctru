/**
 * @file ndsp.h
 * @brief Interface for Nintendo's default DSP component.
 */
#pragma once

#include <3ds/os.h>

#define NDSP_SAMPLE_RATE (SYSCLOCK_SOC / 512.0)

///@name Data types
///@{
/// Sound output modes.
typedef enum
{
	NDSP_OUTPUT_MONO     = 0, ///< Mono sound
	NDSP_OUTPUT_STEREO   = 1, ///< Stereo sound
	NDSP_OUTPUT_SURROUND = 2, ///< 3D Surround sound
} ndspOutputMode;

// Clipping modes.
typedef enum
{
	NDSP_CLIP_NORMAL = 0, ///< "Normal" clipping mode (?)
	NDSP_CLIP_SOFT   = 1, ///< "Soft" clipping mode (?)
} ndspClippingMode;

// Surround speaker positions.
typedef enum
{
	NDSP_SPKPOS_SQUARE = 0, ///<?
	NDSP_SPKPOS_WIDE   = 1, ///<?
	NDSP_SPKPOS_NUM    = 2, ///<?
} ndspSpeakerPos;

/// ADPCM data.
typedef struct
{
	u16 index;    ///< Current predictor index
	s16 history0; ///< Last outputted PCM16 sample.
	s16 history1; ///< Second to last outputted PCM16 sample.
} ndspAdpcmData;

/// Wave buffer type.
typedef struct tag_ndspWaveBuf ndspWaveBuf;

/// Wave buffer status.
enum
{
	NDSP_WBUF_FREE    = 0, ///< The wave buffer is not queued.
	NDSP_WBUF_QUEUED  = 1, ///< The wave buffer is queued and has not been played yet.
	NDSP_WBUF_PLAYING = 2, ///< The wave buffer is playing right now.
	NDSP_WBUF_DONE    = 3, ///< The wave buffer has finished being played.
};

/// Wave buffer struct.
struct tag_ndspWaveBuf
{
	union
	{
		s8*         data_pcm8;  ///< Pointer to PCM8 sample data.
		s16*        data_pcm16; ///< Pointer to PCM16 sample data.
		u8*         data_adpcm; ///< Pointer to DSPADPCM sample data.
		const void* data_vaddr; ///< Data virtual address.
	};
	u32 nsamples;              ///< Total number of samples (PCM8=bytes, PCM16=halfwords, DSPADPCM=nibbles without frame headers)
	ndspAdpcmData* adpcm_data; ///< ADPCM data.

	u32  offset;  ///< Buffer offset. Only used for capture.
	bool looping; ///< Whether to loop the buffer.
	u8   status;  ///< Queuing/playback status.

	u16 sequence_id;   ///< Sequence ID. Assigned automatically by ndspChnWaveBufAdd.
	ndspWaveBuf* next; ///< Next buffer to play. Used internally, do not modify.
};

/// Sound frame callback function. (data = User provided data)
typedef void (*ndspCallback)(void* data);
/// Auxiliary output callback function. (data = User provided data, nsamples = Number of samples, samples = Sample data)
typedef void (*ndspAuxCallback)(void* data, int nsamples, void* samples[4]);
///@}

///@name Initialization and basic operations
///@{
/**
 * @brief Sets up the DSP component.
 * @param binary DSP binary to load.
 * @param size Size of the DSP binary.
 * @param progMask Program RAM block mask to load the binary to.
 * @param dataMask Data RAM block mask to load the binary to.
 */
void   ndspUseComponent(const void* binary, u32 size, u16 progMask, u16 dataMask);

/// Initializes NDSP.
Result ndspInit(void);

/// Exits NDSP.
void   ndspExit(void);

/**
 * @brief Gets the number of dropped sound frames.
 * @return The number of dropped sound frames.
 */
u32    ndspGetDroppedFrames(void);

/**
 * @brief Gets the total sound frame count.
 * @return The total sound frame count.
 */
u32    ndspGetFrameCount(void);
///@}

///@name General parameters
///@{
/**
 * @brief Sets the master volume.
 * @param volume Volume to set. Defaults to 1.0f.
 */
void ndspSetMasterVol(float volume);

/**
 * @brief Sets the output mode.
 * @param mode Output mode to set. Defaults to NDSP_OUTPUT_STEREO.
 */
void ndspSetOutputMode(ndspOutputMode mode);

/**
 * @brief Sets the clipping mode.
 * @param mode Clipping mode to set. Defaults to NDSP_CLIP_SOFT.
 */
void ndspSetClippingMode(ndspClippingMode mode);

/**
 * @brief Sets the output count.
 * @param count Output count to set. Defaults to 2.
 */
void ndspSetOutputCount(int count);

/**
 * @brief Sets the wave buffer to capture audio to.
 * @param capture Wave buffer to capture to.
 */
void ndspSetCapture(ndspWaveBuf* capture);

/**
 * @brief Sets the sound frame callback.
 * @param callback Callback to set.
 * @param data User-defined data to pass to the callback.
 */
void ndspSetCallback(ndspCallback callback, void* data);
///@}

///@name Surround
///@{
/**
 * @brief Sets the surround sound depth.
 * @param depth Depth to set. Defaults to 0x7FFF.
 */
void ndspSurroundSetDepth(u16 depth);

/**
 * @brief Sets the surround sound position.
 * @param pos Position to set. Defaults to NDSP_SPKPOS_SQUARE.
 */
void ndspSurroundSetPos(ndspSpeakerPos pos);

/**
 * @brief Sets the surround sound rear ratio.
 * @param ratio Rear ratio to set. Defaults to 0x8000.
 */
void ndspSurroundSetRearRatio(u16 ratio);
///@}

///@name Auxiliary output
///@{
/**
 * @brief Configures whether an auxiliary output is enabled.
 * @param id ID of the auxiliary output.
 * @param enable Whether to enable the auxiliary output.
 */
void ndspAuxSetEnable(int id, bool enable);

/**
 * @brief Configures whether an auxiliary output should use front bypass.
 * @param id ID of the auxiliary output.
 * @param bypass Whether to use front bypass.
 */
void ndspAuxSetFrontBypass(int id, bool bypass);

/**
 * @brief Sets the volume of an auxiliary output.
 * @param id ID of the auxiliary output.
 * @param volume Volume to set.
 */
void ndspAuxSetVolume(int id, float volume);

/**
 * @brief Sets the callback of an auxiliary output.
 * @param id ID of the auxiliary output.
 * @param callback Callback to set.
 * @param data User-defined data to pass to the callback.
 */
void ndspAuxSetCallback(int id, ndspAuxCallback callback, void* data);
///@}
