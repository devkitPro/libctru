#pragma once

#include <3ds/types.h>

#define CSND_SHAREDMEM_DEFAULT 0x10004000

typedef enum{
	CSND_LOOP_DISABLE,
	CSND_LOOP_ENABLE
} CSND_LOOPING;

typedef enum{
	CSND_ENCODING_PCM8,
	CSND_ENCODING_PCM16,
	CSND_ENCODING_IMA_ADPCM,
	CSND_ENCODING_PSG//"3 = PSG, similar to DS?"
} CSND_ENCODING;

struct  CSND_CHANNEL_STATUS {
	u8  state;
	u8  pad[3];
	u32 unknown;
	u32 position;
} ALIGN(4);


//See here regarding CSND shared-mem commands, etc: http://3dbrew.org/wiki/CSND_Shared_Memory

Result CSND_cmd1(Handle *mutexhandle, Handle *sharedmemhandle, u32 sharedmem_size, u32 off0, u32 off1, u32 off2, u32 off3);
Result CSND_cmd2();
Result CSND_cmd3(u32 offset);void CSND_writesharedmem_cmdtype0(u16 cmdid, u8 *cmdparams);
Result CSND_cmd5(u32 *bitmask);

Result CSND_initialize(u32* sharedMem);
Result CSND_shutdown();

Result CSND_processtype0cmds();
u32 CSND_convertsamplerate(u32 samplerate);
Result CSND_playsound(u32 channel, u32 looping, u32 encoding, u32 samplerate, u32 *vaddr0, u32 *vaddr1, u32 totalbytesize, u32 unk0, u32 unk1);
void CSND_setchannel_playbackstate(u32 channel, u32 value);
void CSND_sharedmemtype0_cmd0(u32 channel, u32 value);
void CSND_sharedmemtype0_cmd3(u32 channel, u32 physaddr, u32 size);
void CSND_sharedmemtype0_cmd8(u32 channel, u32 samplerate);
void CSND_sharedmemtype0_cmd9(u32 channel, u16 value);
void CSND_sharedmemtype0_cmde(u32 channel, u32 looping, u32 encoding, u32 samplerate, u32 unk0, u32 unk1, u32 physaddr0, u32 physaddr1, u32 totalbytesize);
void CSND_writesharedmem_cmdtype0(u16 cmdid, u8 *cmdparams);
Result CSND_sharedmemtype0_cmdupdatestate(int waitdone);

Result CSND_getchannelstate(u32 entryindex, struct CSND_CHANNEL_STATUS *out);
Result CSND_getchannelstate_isplaying(u32 entryindex, u8 *status);
