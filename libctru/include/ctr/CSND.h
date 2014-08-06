#ifndef CSND_H
#define CSND_H

//See here regarding CSND shared-mem commands, etc: http://3dbrew.org/wiki/CSND_Shared_Memory

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

Result CSND_initialize(u32* sharedMem);
Result CSND_shutdown();

Result CSND_playsound(u32 channel, CSND_LOOPING looping, CSND_ENCODING encoding, u32 samplerate, u32 *vaddr0, u32 *vaddr1, u32 totalbytesize, u32 unk0, u32 unk1);//vaddr0 is the initial virtual-address of the audio-data. vaddr1 is the audio-data virtual-address used for looping, when playback is restarted for looping.
void CSND_setchannel_playbackstate(u32 channel, u32 value);//value0 = pause playback, value1 = resume playback.
void CSND_sharedmemtype0_cmd0(u32 channel, u32 value);//value1 = start playback. value0 = stop playback, and reset the CSND registers for this channel.
void CSND_writesharedmem_cmdtype0(u16 cmdid, u8 *cmdparams);//This can be used to use arbitary CSND shared-memory commands.
Result CSND_sharedmemtype0_cmdupdatestate(int waitdone);//This must be used after using CSND shared-memory commands in order for those commands to be processed. CSND_playsound() and CSND_getchannelstate() use this automatically.

Result CSND_getchannelstate(u32 entryindex, u32 *out);
Result CSND_getchannelstate_isplaying(u32 entryindex, u8 *status);

#endif

