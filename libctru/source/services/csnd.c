#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/os.h>
#include <3ds/services/csnd.h>

// See here regarding CSND shared-mem commands, etc: http://3dbrew.org/wiki/CSND_Shared_Memory

vu32* csndSharedMem = NULL;
u32 csndSharedMemSize;
u32 csndChannels = 0;
u32 csndOffsets[4];

static Handle csndHandle = 0;
static Handle csndMutex = 0;
static Handle csndSharedMemBlock = 0;

static u8 csndChnIdx[CSND_NUM_CHANNELS];
static u32 csndCmdBlockSize = 0x2000;
static u32 csndCmdStartOff = 0;
static u32 csndCmdCurOff = 0;

static Result CSND_Initialize(void)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00010140;
	cmdbuf[1] = csndSharedMemSize;
	memcpy(&cmdbuf[2], &csndOffsets[0], 4*sizeof(u32));

	if((ret = svcSendSyncRequest(csndHandle))!=0)return ret;

	csndMutex = cmdbuf[3];
	csndSharedMemBlock = cmdbuf[4];

	return (Result)cmdbuf[1];
}

static Result CSND_Shutdown()
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00020000;

	if((ret = svcSendSyncRequest(csndHandle))!=0)return ret;

	return (Result)cmdbuf[1];
}

static Result CSND_AcquireSoundChannels(u32* channelMask)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00050000;

	if((ret = svcSendSyncRequest(csndHandle))!=0)return ret;

	*channelMask = cmdbuf[2];

	return (Result)cmdbuf[1];
}

static Result CSND_ReleaseSoundChannels(void)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00060000;

	if((ret = svcSendSyncRequest(csndHandle))!=0)return ret;

	return (Result)cmdbuf[1];
}

Result csndInit(void)
{
	Result ret=0;
	csndSharedMem = (vu32*)CSND_SHAREDMEM_DEFAULT;

	// TODO: proper error handling!

	ret = srvGetServiceHandle(&csndHandle, "csnd:SND");
	if (ret != 0) return ret;

	// Calculate offsets and sizes required by the CSND module
	csndOffsets[0] = csndCmdBlockSize;                         // Offset to some unknown DSP status flags
	csndOffsets[1] = csndOffsets[0] + 8;                       // Offset to sound channel information
	csndOffsets[2] = csndOffsets[1] + 32*sizeof(CSND_ChnInfo); // Offset to information for an 'unknown' sort of channels
	csndOffsets[3] = csndOffsets[2] + 2*8;                     // Offset to more unknown information
	csndSharedMemSize = csndOffsets[3] + 0x3C;                 // Total size of the CSND shared memory

	ret = CSND_Initialize();
	if (ret != 0) return ret;

	ret = svcMapMemoryBlock(csndSharedMemBlock, (u32)csndSharedMem, 3, 0x10000000);
	if (ret != 0) return ret;

	memset((void*)csndSharedMem, 0, csndSharedMemSize);

	ret = CSND_AcquireSoundChannels(&csndChannels);
	if (ret != 0) return ret;

	// Build channel indices for the sound channel information table
	int i, j = 0;
	for (i = 0; i < CSND_NUM_CHANNELS; i ++)
	{
		csndChnIdx[i] = j;
		if (csndChannels & BIT(i))
			j ++;
	}

	return 0;
}

Result csndExit(void)
{
	Result ret;

	ret = CSND_ReleaseSoundChannels();
	if (ret != 0) return ret;

	svcUnmapMemoryBlock(csndSharedMemBlock, (u32)csndSharedMem);
	svcCloseHandle(csndSharedMemBlock);

	ret = CSND_Shutdown();
	svcCloseHandle(csndHandle);
	return ret;
}

static Result CSND_ExecChnCmds(u32 offset)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00030040;
	cmdbuf[1] = offset;

	if((ret = svcSendSyncRequest(csndHandle))!=0)return ret;

	return (Result)cmdbuf[1];
}

void csndWriteChnCmd(int cmdid, u8 *cmdparams)
{
	vu16* ptr;
	u32 prevoff;
	s32 outindex=0;

	svcWaitSynchronizationN(&outindex, &csndMutex, 1, 0, ~0);

	if (csndCmdStartOff != csndCmdCurOff)
	{
		if (csndCmdCurOff>=0x20)
			prevoff = csndCmdCurOff-0x20;
		else
			prevoff = csndCmdBlockSize-0x20;

		ptr = (vu16*)&csndSharedMem[prevoff>>2];
		*ptr = csndCmdCurOff;
	}

	ptr = (vu16*)&csndSharedMem[csndCmdCurOff>>2];

	ptr[0] = 0xFFFF;
	ptr[1] = cmdid;
	ptr[2] = 0;
	ptr[3] = 0;
	memcpy((void*)&ptr[4], cmdparams, 0x18);

	csndCmdCurOff += 0x20;
	if (csndCmdCurOff >= csndCmdBlockSize)
		csndCmdCurOff = 0;

	svcReleaseMutex(csndMutex);
}

Result csndExecChnCmds(bool waitDone)
{
	Result ret=0;

	// Check we actually wrote commands
	if (csndCmdStartOff == csndCmdCurOff)
		return 0;

	vu8* flag = (vu8*)&csndSharedMem[(csndCmdStartOff + 4) >> 2];

	ret = CSND_ExecChnCmds(csndCmdStartOff);
	csndCmdStartOff = csndCmdCurOff;
	if (ret != 0) return ret;

	// FIXME: This is a really ugly busy waiting loop!
	while (waitDone && *flag == 0);

	return ret;
}

void CSND_ChnSetPlayStateR(u32 channel, u32 value)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;

	csndWriteChnCmd(0x0, (u8*)&cmdparams);
}

void CSND_ChnSetPlayState(u32 channel, u32 value)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;

	csndWriteChnCmd(0x1, (u8*)&cmdparams);
}

void CSND_ChnSetLoop(u32 channel, u32 physaddr, u32 size)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = physaddr;
	cmdparams[2] = size;

	csndWriteChnCmd(0x3, (u8*)&cmdparams);
}

void CSND_ChnSetVol(u32 channel, u16 left, u16 right)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = left | (right<<16);

	csndWriteChnCmd(0x9, (u8*)&cmdparams);
}

void CSND_ChnSetTimer(u32 channel, u32 timer)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = timer;

	csndWriteChnCmd(0x8, (u8*)&cmdparams);
}

void CSND_ChnConfig(u32 channel, u32 looping, u32 encoding, u32 timer, u32 unk0, u32 unk1, u32 physaddr0, u32 physaddr1, u32 totalbytesize)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[0] |= (unk0 & 0xf) << 6;
	if (!looping) cmdparams[0] |= 2 << 10;
	if (looping) cmdparams[0] |= 1 << 10;
	cmdparams[0] |= (encoding & 3) << 12;
	cmdparams[0] |= (unk1 & 3) << 14;

	if (timer < 0x42) timer = 0x42;
	if (timer > 0xffff) timer = 0xffff;
	cmdparams[0] |= timer<<16;

	cmdparams[3] = physaddr0;
	cmdparams[4] = physaddr1;
	cmdparams[5] = totalbytesize;

	csndWriteChnCmd(0xe, (u8*)&cmdparams);
}

Result CSND_UpdateChnInfo(bool waitDone)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	csndWriteChnCmd(0x300, (u8*)&cmdparams);
	return csndExecChnCmds(waitDone);
}

Result csndChnPlaySound(u32 channel, u32 looping, u32 encoding, u32 samplerate, u32 *vaddr0, u32 *vaddr1, u32 totalbytesize, u32 unk0, u32 unk1)
{
	if (!(csndChannels & BIT(channel)))
		return 1;

	u32 physaddr0 = 0;
	u32 physaddr1 = 0;

	physaddr0 = osConvertVirtToPhys((u32)vaddr0);
	physaddr1 = osConvertVirtToPhys((u32)vaddr1);

	CSND_ChnConfig(channel, looping, encoding, CSND_TIMER(samplerate), unk0, unk1, physaddr0, physaddr1, totalbytesize);
	if(looping)
	{
		if(physaddr1>physaddr0)totalbytesize-= (u32)physaddr1 - (u32)physaddr0;
		CSND_ChnSetLoop(channel, physaddr1, totalbytesize);
	}
	CSND_ChnSetVol(channel, 0xFFFF, 0xFFFF);
	CSND_ChnSetPlayState(channel, 1);

	return csndExecChnCmds(true);
}

CSND_ChnInfo* csndChnGetInfo(u32 channel)
{
	channel = csndChnIdx[channel];
	return (CSND_ChnInfo*)(&csndSharedMem[(csndOffsets[1] + channel*0xc) >> 2]);
}

Result csndChnGetState(u32 channel, u32 *out)
{
	Result ret = 0;
	channel = csndChnIdx[channel];

	if ((ret = CSND_UpdateChnInfo(true)) != 0)return ret;

	memcpy(out, (const void*)&csndSharedMem[(csndOffsets[1] + channel*0xc) >> 2], 0xc);
	//out[2] -= 0x0c000000;

	return 0;
}

Result csndChnIsPlaying(u32 channel, u8 *status)
{
	Result ret;
	struct CSND_CHANNEL_STATUS entry;

	ret = csndChnGetState(channel, entry);
	if(ret!=0)return ret;

	*status = entry.state;

	return 0;
}
