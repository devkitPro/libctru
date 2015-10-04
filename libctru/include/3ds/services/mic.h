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
 * @param control Control. TODO: Document parameter.
 * @param unk0 Unknown. Typically 3.
 * @param unk1 Unknown. Typically 1.
 * @param unk2 Unknown. Typically 1.
 */
Result MIC_Initialize(u32 *sharedmem, u32 sharedmem_size, u8 control, u8 recording, u8 unk0, u8 unk1, u8 unk2);

/**
 * @brief Shuts down MIC.
 */
Result MIC_Shutdown(void);

u32 MIC_GetSharedMemOffsetValue(void);

/**
 * @brief Reads MIC audio data.
 * @param outbuf Buffer to write audio data to.
 * @param readsize Size of the buffer to write to.
 * @param waitforevent Whether to wait for the MIC service to signal that audio data is ready. (non-zero = wait)
 * @return Actual number of bytes read.
 */
u32 MIC_ReadAudioData(u8 *outbuf, u32 readsize, u32 waitforevent);

Result MIC_MapSharedMem(Handle handle, u32 size);
Result MIC_UnmapSharedMem(void);
Result MIC_cmd3_Initialize(u8 unk0, u8 unk1, u32 sharedmem_baseoffset, u32 sharedmem_endoffset, u8 unk2);
Result MIC_cmd5(void);
Result MIC_GetCNTBit15(u8 *out);
Result MIC_GetEventHandle(Handle *handle);
/**
 * See here: http://3dbrew.org/wiki/MIC_Services
 */
Result MIC_SetControl(u8 value);
Result MIC_GetControl(u8 *value);
Result MIC_SetRecording(u8 value);
Result MIC_IsRecoding(u8 *value);

