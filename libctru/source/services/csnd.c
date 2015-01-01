#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/os.h>
#include <3ds/services/csnd.h>

// See here regarding CSND shared-mem commands, etc: http://3dbrew.org/wiki/CSND_Shared_Memory

static Handle csndHandle = 0;
static Handle csndMutex = 0;
static Handle csndSharedMemBlock = 0;
static u32* csndSharedMem = NULL;
static u32 csndBitmask = 0;

static u32 csndCmdBlockSize = 0x2000;
static u32 csndCmdStartOff = 0;
static u32 csndCmdCurOff = 0;

static Result CSND_Initialize(u32 sharedmem_size, u32 off0, u32 off1, u32 off2, u32 off3)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00010140;
	cmdbuf[1] = sharedmem_size;
	cmdbuf[2] = off0;
	cmdbuf[3] = off1;
	cmdbuf[4] = off2;
	cmdbuf[5] = off3;

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

static Result CSND_GetBitmask(u32* bitmask)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00050000;

	if((ret = svcSendSyncRequest(csndHandle))!=0)return ret;

	*bitmask = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result csndInit(void)
{
	Result ret=0;
	csndSharedMem = (u32*)CSND_SHAREDMEM_DEFAULT;

	// TODO: proper error handling!

	ret = srvGetServiceHandle(&csndHandle, "csnd:SND");
	if (ret != 0) return ret;

	u32 sharedMemSize = csndCmdBlockSize+0x114;

	ret = CSND_Initialize(sharedMemSize, csndCmdBlockSize, csndCmdBlockSize+8, csndCmdBlockSize+0xc8, csndCmdBlockSize+0xd8);
	if (ret != 0) return ret;

	ret = svcMapMemoryBlock(csndSharedMemBlock, (u32)csndSharedMem, 3, 0x10000000);
	if (ret != 0) return ret;

	memset(csndSharedMem, 0, sharedMemSize);

	ret = CSND_GetBitmask(&csndBitmask);
	if (ret != 0) return ret;

	return 0;
}

Result csndExit(void)
{
	Result ret;

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
	u16* ptr;
	u32 prevoff;
	s32 outindex=0;

	svcWaitSynchronizationN(&outindex, &csndMutex, 1, 0, ~0);

	if (csndCmdStartOff != csndCmdCurOff)
	{
		if (csndCmdCurOff>=0x20)
			prevoff = csndCmdCurOff-0x20;
		else
			prevoff = csndCmdBlockSize-0x20;

		ptr = (u16*)&csndSharedMem[prevoff>>2];
		*ptr = csndCmdCurOff;
	}

	ptr = (u16*)&csndSharedMem[csndCmdCurOff>>2];

	ptr[0] = 0xFFFF;
	ptr[1] = cmdid & 0xFFFF;
	ptr[2] = 0;
	ptr[3] = 0;
	memcpy(&ptr[8>>1], cmdparams, 0x18);

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

	vu8* flag = (vu8*)&csndSharedMem[csndCmdStartOff>>2];

	ret = CSND_ExecChnCmds(csndCmdStartOff);
	csndCmdStartOff = csndCmdCurOff;

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
	if(!looping)cmdparams[0] |= 2 << 10;
	if(looping)cmdparams[0] |= 1 << 10;
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

	return CSND_UpdateChnInfo(false);
}

Result csndChnGetState(u32 entryindex, u32 *out)
{
	Result ret = 0;

	if((ret = CSND_UpdateChnInfo(true)) != 0)return ret;

	memcpy(out, &csndSharedMem[(csndCmdBlockSize+8 + entryindex*0xc) >> 2], 0xc);
	out[2] -= 0x0c000000;

	return 0;
}

Result csndChnIsPlaying(u32 entryindex, u8 *status)
{
	Result ret;
	struct CSND_CHANNEL_STATUS entry;

	ret = csndChnGetState(entryindex, entry);
	if(ret!=0)return ret;

	*status = entry.state;

	return 0;
}

