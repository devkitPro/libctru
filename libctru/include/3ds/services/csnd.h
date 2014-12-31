#pragma once

#include <3ds/types.h>

#define CSND_SHAREDMEM_DEFAULT 0x10004000

#define CSND_TIMER(n) (0x3FEC3FC / ((u32)(n)))

typedef enum{
    CSND_LOOP_DISABLE,
    CSND_LOOP_ENABLE,
} CSND_LOOPING;

typedef enum{
    CSND_ENCODING_PCM8,
    CSND_ENCODING_PCM16,
    CSND_ENCODING_IMA_ADPCM,
    CSND_ENCODING_PSG, // Similar to DS?
} CSND_ENCODING;

// See here regarding CSND shared-mem commands, etc: http://3dbrew.org/wiki/CSND_Shared_Memory

Result csndInit(void);
Result csndExit(void);

u32 CSND_convertsamplerate(u32 samplerate);
Result CSND_playsound(u32 channel, u32 looping, u32 encoding, u32 samplerate, u32 *vaddr0, u32 *vaddr1, u32 totalbytesize, u32 unk0, u32 unk1);
void CSND_setchannel_playbackstate(u32 channel, u32 value);
void CSND_sharedmemtype0_cmd0(u32 channel, u32 value);
void CSND_writesharedmem_cmdtype0(u16 cmdid, u8 *cmdparams);
Result CSND_sharedmemtype0_cmdupdatestate(int waitdone);

Result CSND_getchannelstate(u32 entryindex, struct CSND_CHANNEL_STATUS *out);
Result CSND_getchannelstate_isplaying(u32 entryindex, u8 *status);
