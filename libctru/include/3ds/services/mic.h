/**
 * @file mic.h
 * @brief MIC (Microphone) service.
 */
#pragma once

//See also: http://3dbrew.org/wiki/MIC_Services

/**
 * @brief Initializes MIC.
 * @param sharedmem Shared memory block to use. Must be 0x1000-bytes aligned.
 * @param sharedmem_size Size of the shared memory block to use. (audiodata size + 4, aligned to 0x1000-bytes)
 * @param control Control value. Bits 0-6 = Amplification.
 * @param unk0 Unknown. Typically 3.
 * @param unk1 Unknown. Typically 1.
 * @param unk2 Unknown. Typically 1.
 */
Result MIC_Initialize(u32 *sharedmem, u32 sharedmem_size, u8 control, u8 recording, u8 unk0, u8 unk1, u8 unk2);

/// Shuts down MIC.
Result MIC_Shutdown(void);

/**
 * @brief Gets the current shared memory offset.
 * @return The current shared memory offset.
 */
u32 MIC_GetSharedMemOffsetValue(void);

/**
 * @brief Reads MIC audio data.
 * @param outbuf Buffer to write audio data to.
 * @param readsize Size of the buffer to write to.
 * @param waitforevent Whether to wait for the MIC service to signal that audio data is ready. (non-zero = wait)
 * @return Actual number of bytes read.
 */
u32 MIC_ReadAudioData(u8 *outbuf, u32 readsize, u32 waitforevent);

/**
 * @brief Maps MIC's shared memory.
 * @param handle Handle of the shared memory.
 * @param size Size of the shared memory.
 */
Result MIC_MapSharedMem(Handle handle, u32 size);

/// Unmaps MIC's shardd memory.
Result MIC_UnmapSharedMem(void);

/**
 * @brief Initializes MIC.
 * @param unk0 Unknown.
 * @param unk1 Unknown.
 * @param sharedmem_baseoffset Base offset of shared memory.
 * @param sharedmem_endoffset End offset of shared memory.
 * @param unk2 Unknown.
 */
Result MIC_cmd3_Initialize(u8 unk0, u8 unk1, u32 sharedmem_baseoffset, u32 sharedmem_endoffset, u8 unk2);

/// Unknown MIC command.
Result MIC_cmd5(void);

/**
 * @brief Gets CNT bit 15 from MIC.
 * @param out Pointer to output the bit to.
 */
Result MIC_GetCNTBit15(u8 *out);

/**
 * @brief Gets the event handle signaled by MIC when data is ready.
 * @param handle Pointer to output the event handle to.
 */
Result MIC_GetEventHandle(Handle *handle);

/**
 * Sets the control value.
 * @note Bits 0-6 = Amplification.
 * @param value Control value to set.
 */
Result MIC_SetControl(u8 value);

/**
 * Gets the control value.
 * @note Bits 0-6 = Amplification.
 * @param value Pointer to output the control value to.
 */
Result MIC_GetControl(u8 *value);

/**
 * Sets whether the microphone is recording.
 * @param value Whether the microphone is recording.
 */
Result MIC_SetRecording(u8 value);

/**
 * Gets whether the microphone is recording.
 * @param value Pointer to output whether the microphone is recording to.
 */
Result MIC_IsRecoding(u8 *value);

