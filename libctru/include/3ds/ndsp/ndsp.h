/**
 * @file ndsp.h
 * @brief Nintendo default DSP interface.
 */
#pragma once

///@name Data types
///@{
/// ADPCM data.
typedef struct
{
	u16 index;    ///< Current sample index(?)
	s16 history0; ///< First previous sample index(?)
	s16 history1; ///< Second previous sample index(?)
} ndspAdpcmData;

/// Wave buffer type.
typedef struct tag_ndspWaveBuf ndspWaveBuf;

/// Wave buffer struct.
struct tag_ndspWaveBuf
{
	union
	{
		s8*  data_pcm8;  ///< PCM8 data.
		s16* data_pcm16; ///< PCM16 data.
		u8*  data_adpcm; ///< ADPCM data.
		u32  data_vaddr; ///< Data virtual address.
	};
	u32 nsamples;              ///< Total samples.
	ndspAdpcmData* adpcm_data; ///< ADPCM data.

	u32  offset;  ///< Buffer offset. Only used for capture.
	bool looping; ///< Whether to loop the buffer.
	u8   padding; ///< Padding.

	u16 sequence_id;   ///< Sequence ID. Used internally.
	ndspWaveBuf* next; ///< Next buffer. Used internally.
};

/// NDSP callback function. (data = User provided data)
typedef void (*ndspCallback)(void* data);
/// NDSP auxiliary callback function. (data = User provided data, nsamples = Number of samples, samples = Sample data)
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
 * @brief Gets the number of dropped NDSP frames.
 * @return The number of dropped frames.
 */
u32    ndspGetDroppedFrames(void);

/**
 * @brief Gets the total NDSP frame count.
 * @return The total frame count.
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
 * @param mode Output mode to set. Defaults to 0.
 */
void ndspSetOutputMode(int mode);

/**
 * @brief Sets the clipping mode.
 * @param mode Clipping mode to set. Defaults to 1.
 */
void ndspSetClippingMode(int mode);

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
 * @brief Sets the NDSP frame callback.
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
 * @param pos Position to set. Defaults to 0.
 */
void ndspSurroundSetPos(u16 pos);

/**
 * @brief Sets the surround sound rear ratio.
 * @param ratio Rear ratio to set. Defaults to 0x8000.
 */
void ndspSurroundSetRearRatio(u16 ratio);
///@}

///@name Auxiliary output
///@{
/**
 * @brief Sets whether an auxiliary output is enabled.
 * @param id ID of the auxiliary output.
 * @param enable Whether to enable the auxiliary output.
 */
void ndspAuxSetEnable(int id, bool enable);

/**
 * @brief Sets whether an auxiliary output should use front bypass.
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
 * @brief Sets the NDSP frame callback of an auxiliary output.
 * @param id ID of the auxiliary output.
 * @param callback Callback to set.
 * @param data User-defined data to pass to the callback.
 */
void ndspAuxSetCallback(int id, ndspAuxCallback callback, void* data);
///@}
