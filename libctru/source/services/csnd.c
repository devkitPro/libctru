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

Result CSND_AcquireCapUnit(u32* capUnit)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00070000;

	if((ret = svcSendSyncRequest(csndHandle))!=0)return ret;

	*capUnit = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result CSND_ReleaseCapUnit(u32 capUnit)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00080040;
	cmdbuf[1] = capUnit;

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

static Result CSND_ExecCmd0(u32 offset)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00030040;
	cmdbuf[1] = offset;

	if((ret = svcSendSyncRequest(csndHandle))!=0)return ret;

	return (Result)cmdbuf[1];
}

void csndWriteCmd(int cmdid, u8 *cmdparams)
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

Result csndExecCmds(bool waitDone)
{
	Result ret=0;

	// Check we actually wrote commands
	if (csndCmdStartOff == csndCmdCurOff)
		return 0;

	vu8* flag = (vu8*)&csndSharedMem[(csndCmdStartOff + 4) >> 2];

	ret = CSND_ExecCmd0(csndCmdStartOff);
	csndCmdStartOff = csndCmdCurOff;
	if (ret != 0) return ret;

	// FIXME: This is a really ugly busy waiting loop!
	while (waitDone && *flag == 0);

	return ret;
}

void CSND_SetPlayStateR(u32 channel, u32 value)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;

	csndWriteCmd(0x0, (u8*)&cmdparams);
}

void CSND_CetPlayState(u32 channel, u32 value)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;

	csndWriteCmd(0x1, (u8*)&cmdparams);
}

void CSND_SetBlock(u32 channel, int block, u32 physaddr, u32 size)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = physaddr;
	cmdparams[2] = size;

	csndWriteCmd(block ? 0x3 : 0xA, (u8*)&cmdparams);
}

void CSND_SetVol(u32 channel, u16 left, u16 right)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = left | (right<<16);

	csndWriteCmd(0x9, (u8*)&cmdparams);
}

void CSND_SetTimer(u32 channel, u32 timer)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = timer;

	csndWriteCmd(0x8, (u8*)&cmdparams);
}

void CSND_SetDuty(u32 channel, u32 duty)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = duty;

	csndWriteCmd(0x7, (u8*)&cmdparams);
}

void CSND_SetAdpcmState(u32 channel, int block, int sample, int index)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = sample & 0xFFFF;
	cmdparams[2] = index & 0x7F;

	csndWriteCmd(block ? 0xC : 0xB, (u8*)&cmdparams);
}

void CSND_SetAdpcmReload(u32 channel, bool reload)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = reload ? 1 : 0;

	csndWriteCmd(0xD, (u8*)&cmdparams);
}

void CSND_SetChnRegs(u32 flags, u32 physaddr0, u32 physaddr1, u32 totalbytesize)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = flags;
	cmdparams[1] = 0x7FFF7FFF; // Volume
	cmdparams[2] = 0; // Unknown
	cmdparams[3] = physaddr0;
	cmdparams[4] = physaddr1;
	cmdparams[5] = totalbytesize;

	csndWriteCmd(0xe, (u8*)&cmdparams);
}

Result CSND_UpdateInfo(bool waitDone)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	csndWriteCmd(0x300, (u8*)&cmdparams);
	return csndExecCmds(waitDone);
}

void CSND_CapEnable(u32 capUnit, bool enable)
{
	u32 cmdparams[0x18>>2];
	memset(cmdparams, 0, 0x18);

	cmdparams[0] = capUnit;
	cmdparams[1] = enable ? 1 : 0;

	csndWriteCmd(0x100, (u8*)&cmdparams);
}

void CSND_CapSetBit(u32 capUnit, int bit, bool state)
{
	u32 cmdparams[0x18>>2];
	memset(cmdparams, 0, 0x18);

	cmdparams[0] = capUnit;
	cmdparams[1] = state ? 1 : 0;

	csndWriteCmd(0x101 + bit, (u8*)&cmdparams);
}

void CSND_CapSetTimer(u32 capUnit, u32 timer)
{
	u32 cmdparams[0x18>>2];
	memset(cmdparams, 0, 0x18);

	cmdparams[0] = capUnit;
	cmdparams[1] = timer & 0xFFFF;

	csndWriteCmd(0x104, (u8*)&cmdparams);
}

void CSND_CapSetBuffer(u32 capUnit, u32 paddr, u32 size)
{
	u32 cmdparams[0x18>>2];
	memset(cmdparams, 0, 0x18);

	cmdparams[0] = capUnit;
	cmdparams[1] = paddr;
	cmdparams[2] = size;

	csndWriteCmd(0x105, (u8*)&cmdparams);
}

Result csndPlaySound(int chn, u32 flags, u32 sampleRate, void* data0, void* data1, u32 size)
{
	if (!(csndChannels & BIT(chn)))
		return 1;

	u32 paddr0 = 0, paddr1 = 0;

	int encoding = (flags >> 12) & 3;
	int loopMode = (flags >> 10) & 3;

	if (encoding != CSND_ENCODING_PSG)
	{
		if (data0) paddr0 = osConvertVirtToPhys((u32)data0);
		if (data1) paddr1 = osConvertVirtToPhys((u32)data1);

		if (encoding == CSND_ENCODING_ADPCM)
		{
			int adpcmSample = ((s16*)data0)[-2];
			int adpcmIndex = ((u8*)data0)[-2];
			CSND_SetAdpcmState(chn, 0, adpcmSample, adpcmIndex);
		}
	}

	u32 timer = CSND_TIMER(sampleRate);
	if (timer < 0x0042) timer = 0x0042;
	else if (timer > 0xFFFF) timer = 0xFFFF;
	flags &= ~0xFFFF001F;
	flags |= SOUND_ENABLE | SOUND_CHANNEL(chn) | (timer << 16);

	CSND_SetChnRegs(flags, paddr0, paddr1, size);

	if (loopMode == CSND_LOOPMODE_NORMAL && paddr1 > paddr0)
	{
		// Now that the first block is playing, configure the size of the subsequent blocks
		size -= paddr1 - paddr0;
		CSND_SetBlock(chn, 1, paddr1, size);
	}

	return csndExecCmds(true);
}

CSND_ChnInfo* csndGetChnInfo(u32 channel)
{
	channel = csndChnIdx[channel];
	return (CSND_ChnInfo*)(&csndSharedMem[(csndOffsets[1] + channel*0xc) >> 2]);
}

Result csndGetState(u32 channel, CSND_ChnInfo* out)
{
	Result ret = 0;
	channel = csndChnIdx[channel];

	if ((ret = CSND_UpdateInfo(true)) != 0)return ret;

	memcpy(out, (const void*)&csndSharedMem[(csndOffsets[1] + channel*0xc) >> 2], 0xc);
	//out[2] -= 0x0c000000;

	return 0;
}

Result csndIsPlaying(u32 channel, u8* status)
{
	Result ret;
	struct CSND_CHANNEL_STATUS entry;

	ret = csndGetState(channel, &entry);
	if(ret!=0)return ret;

	*status = entry.state;

	return 0;
}
