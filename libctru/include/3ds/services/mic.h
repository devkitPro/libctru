#pragma once

//See also: http://3dbrew.org/wiki/MIC_Services

Result MIC_Initialize(u32 *sharedmem, u32 sharedmem_size, u8 control, u8 recording, u8 unk0, u8 unk1, u8 unk2);//sharedmem_size = audiodata size + 4, aligned to 0x1000-bytes. The sharedmem ptr must be 0x1000-bytes aligned. The offical 3ds-sound app uses the following values for unk0-unk2: 3, 1, and 1.
Result MIC_Shutdown();
u32 MIC_GetSharedMemOffsetValue();
u32 MIC_ReadAudioData(u8 *outbuf, u32 readsize, u32 waitforevent);//Reads MIC audio data. When waitforevent is non-zero, this clears the event, then waits for MIC-module to signal it again when audio data is written to shared-mem. The return value is the actual byte-size of the read data.

Result MIC_MapSharedMem(Handle handle, u32 size);
Result MIC_UnmapSharedMem();
Result MIC_cmd3_Initialize(u8 unk0, u8 unk1, u32 sharedmem_baseoffset, u32 sharedmem_endoffset, u8 unk2);
Result MIC_cmd5();
Result MIC_GetCNTBit15(u8 *out);
Result MIC_GetEventHandle(Handle *handle);
Result MIC_SetControl(u8 value);//See here: http://3dbrew.org/wiki/MIC_Services
Result MIC_GetControl(u8 *value);
Result MIC_SetRecording(u8 value);
Result MIC_IsRecoding(u8 *value);

