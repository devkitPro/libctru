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

static Result CSND_ExecCommand20(u32 offset)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00030040;
	cmdbuf[1] = offset;

	if((ret = svcSendSyncRequest(csndHandle))!=0)return ret;

	return (Result)cmdbuf[1];
}

static void csndWriteCmdType0(int cmdid, u8 *cmdparams)
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

	ptr[0] = 0xffff;
	ptr[1] = cmdid;
	ptr[2] = 0;
	ptr[3] = 0;
	memcpy(&ptr[8>>1], cmdparams, 0x18);

	csndCmdCurOff += 0x20;
	if (csndCmdCurOff >= csndCmdBlockSize)
		csndCmdCurOff = 0;

	svcReleaseMutex(csndMutex);
}

static Result csndExecCmdType0()
{
	Result ret=0;

	// Check we actually wrote commands
	if (csndCmdStartOff == csndCmdCurOff)
		return 0;

	ret = CSND_ExecCommand20(csndCmdStartOff);
	csndCmdStartOff = csndCmdCurOff;

	return ret;
}

void csndSharedMemtype0_cmd0(u32 channel, u32 value)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;

	csndWriteCmdType0(0x0, (u8*)&cmdparams);
}

void CSND_setchannel_playbackstate(u32 channel, u32 value)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;

	csndWriteCmdType0(0x1, (u8*)&cmdparams);
}

void csndSharedMemtype0_cmd3(u32 channel, u32 physaddr, u32 size)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = physaddr;
	cmdparams[2] = size;

	csndWriteCmdType0(0x3, (u8*)&cmdparams);
}

void csndSharedMemtype0_cmd9(u32 channel, u16 value)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value | (value<<16);

	csndWriteCmdType0(0x9, (u8*)&cmdparams);
}

void csndSharedMemtype0_cmd8(u32 channel, u32 samplerate)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = CSND_TIMER(samplerate);

	csndWriteCmdType0(0x8, (u8*)&cmdparams);
}

void csndSharedMemtype0_cmde(u32 channel, u32 looping, u32 encoding, u32 samplerate, u32 unk0, u32 unk1, u32 physaddr0, u32 physaddr1, u32 totalbytesize)
{
	u32 val;
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[0] |= (unk0 & 0xf) << 6;
	if(!looping)cmdparams[0] |= 2 << 10;
	if(looping)cmdparams[0] |= 1 << 10;
	cmdparams[0] |= (encoding & 3) << 12;
	cmdparams[0] |= (unk1 & 3) << 14;

	val = CSND_TIMER(samplerate);
	if(val<0x42)val = 0x42;
	if(val>0xffff)val = 0xffff;
	cmdparams[0] |= val<<16;

	cmdparams[3] = physaddr0;
	cmdparams[4] = physaddr1;
	cmdparams[5] = totalbytesize;

	csndWriteCmdType0(0xe, (u8*)&cmdparams);
}

Result csndSharedMemtype0_cmdupdatestate(int waitdone)
{
	u8 *ptr;
	int ret=0;

	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	ptr = (u8*)&csndSharedMem[csndCmdStartOff>>2];

	csndWriteCmdType0(0x300, (u8*)&cmdparams);

	ret = csndExecCmdType0();
	if(ret!=0)return ret;

	if(waitdone)
	{
		while(*ptr == 0);
	}

	return 0;
}

Result CSND_playsound(u32 channel, u32 looping, u32 encoding, u32 samplerate, u32 *vaddr0, u32 *vaddr1, u32 totalbytesize, u32 unk0, u32 unk1)
{
	u32 physaddr0 = 0;
	u32 physaddr1 = 0;

	physaddr0 = osConvertVirtToPhys((u32)vaddr0);
	physaddr1 = osConvertVirtToPhys((u32)vaddr1);

	csndSharedMemtype0_cmde(channel, looping, encoding, samplerate, unk0, unk1, physaddr0, physaddr1, totalbytesize);
	csndSharedMemtype0_cmd8(channel, samplerate);
	if(looping)
	{
		if(physaddr1>physaddr0)totalbytesize-= (u32)physaddr1 - (u32)physaddr0;
		csndSharedMemtype0_cmd3(channel, physaddr1, totalbytesize);
	}
	csndSharedMemtype0_cmd8(channel, samplerate);
	csndSharedMemtype0_cmd9(channel, 0xffff);
	CSND_setchannel_playbackstate(channel, 1);

	return csndSharedMemtype0_cmdupdatestate(0);
}

Result CSND_getchannelstate(u32 entryindex, struct CSND_CHANNEL_STATUS *out)
{
	Result ret=0;

	if((ret = csndSharedMemtype0_cmdupdatestate(1))!=0)return ret;

	memcpy(out, &csndSharedMem[(csndCmdBlockSize+8 + entryindex*0xc) >> 2], 0xc);
	out[2] -= 0x0c000000;

	return 0;
}

Result CSND_getchannelstate_isplaying(u32 entryindex, u8 *status)
{
	Result ret;
	struct CSND_CHANNEL_STATUS entry;

	ret = CSND_getchannelstate(entryindex, &entry);
	if(ret!=0)return ret;

	*status = entry.state;

	return 0;
}

