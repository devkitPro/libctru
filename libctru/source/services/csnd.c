#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/mappable.h>
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

Result CSND_Reset(void)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000C0000;

	if((ret = svcSendSyncRequest(csndHandle))!=0)return ret;

	return (Result)cmdbuf[1];
}

Result csndInit(void)
{
	Result ret=0;

	// TODO: proper error handling!

	ret = srvGetServiceHandle(&csndHandle, "csnd:SND");
	if (ret != 0) return ret;

	// Calculate offsets and sizes required by the CSND module
	csndOffsets[0] = csndCmdBlockSize;                         // Offset to DSP semaphore and irq disable flags
	csndOffsets[1] = csndOffsets[0] + 8;                       // Offset to sound channel information
	csndOffsets[2] = csndOffsets[1] + 32*sizeof(CSND_ChnInfo); // Offset to capture unit information
	csndOffsets[3] = csndOffsets[2] + 2*8;                     // Offset to the input of command 0x00040080
	csndSharedMemSize = csndOffsets[3] + 0x3C;                 // Total size of the CSND shared memory

	ret = CSND_Initialize();
	if (ret != 0) goto cleanup1;

	csndSharedMem = (vu32*)mappableAlloc(csndSharedMemSize);
	if(!csndSharedMem)
	{
		ret = -1;
		goto cleanup1;
	}

	ret = svcMapMemoryBlock(csndSharedMemBlock, (u32)csndSharedMem, 3, 0x10000000);
	if (ret != 0) goto cleanup2;

	memset((void*)csndSharedMem, 0, csndSharedMemSize);

	ret = CSND_AcquireSoundChannels(&csndChannels);
	if (!ret) return 0;

cleanup2:
	svcCloseHandle(csndSharedMemBlock);
	if(csndSharedMem != NULL)
	{
		mappableFree((void*) csndSharedMem);
		csndSharedMem = NULL;
	}
cleanup1:
	svcCloseHandle(csndHandle);
	return ret;
}

Result csndExit(void)
{
	Result ret;

	//ret = CSND_Reset();
	//if (ret != 0) return ret;

	ret = CSND_ReleaseSoundChannels();
	if (ret != 0) return ret;

	svcUnmapMemoryBlock(csndSharedMemBlock, (u32)csndSharedMem);
	svcCloseHandle(csndSharedMemBlock);

	ret = CSND_Shutdown();
	svcCloseHandle(csndHandle);
	
	if(csndSharedMem != NULL)
	{
		mappableFree((void*) csndSharedMem);
		csndSharedMem = NULL;
	}

	return ret;
}

static Result CSND_ExecuteCommands(u32 offset)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00030040;
	cmdbuf[1] = offset;

	if((ret = svcSendSyncRequest(csndHandle))!=0)return ret;

	return (Result)cmdbuf[1];
}

u32* csndAddCmd(int cmdid)
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
	u32* ret = (u32*)&ptr[4];

	csndCmdCurOff += 0x20;
	if (csndCmdCurOff >= csndCmdBlockSize)
		csndCmdCurOff = 0;

	svcReleaseMutex(csndMutex);
	return ret;
}

void csndWriteCmd(int cmdid, u8 *cmdparams)
{
	memcpy(csndAddCmd(cmdid), cmdparams, 0x18);
}

Result csndExecCmds(bool waitDone)
{
	Result ret=0;

	// Check we actually wrote commands
	if (csndCmdStartOff == csndCmdCurOff)
		return 0;

	vu8* flag = (vu8*)&csndSharedMem[(csndCmdStartOff + 4) >> 2];

	ret = CSND_ExecuteCommands(csndCmdStartOff);
	csndCmdStartOff = csndCmdCurOff;
	if (ret != 0) return ret;

	// FIXME: This is a really ugly busy waiting loop!
	while (waitDone && *flag == 0);

	return ret;
}

void CSND_SetPlayStateR(u32 channel, u32 value)
{
	u32* cmdparams = csndAddCmd(0x000);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;
}

void CSND_SetPlayState(u32 channel, u32 value)
{
	u32* cmdparams = csndAddCmd(0x001);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;
}

void CSND_SetEncoding(u32 channel, u32 value)
{
	u32* cmdparams = csndAddCmd(0x002);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;
}

void CSND_SetBlock(u32 channel, int block, u32 physaddr, u32 size)
{
	u32* cmdparams = csndAddCmd(block ? 0x003 : 0x00A);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = physaddr;
	cmdparams[2] = size;
}

void CSND_SetLooping(u32 channel, u32 value)
{
	u32* cmdparams = csndAddCmd(0x004);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;
}

void CSND_SetBit7(u32 channel, bool set)
{
	u32* cmdparams = csndAddCmd(0x005);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = set ? 1 : 0;
}

void CSND_SetInterp(u32 channel, bool interp)
{
	u32* cmdparams = csndAddCmd(0x006);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = interp ? 1 : 0;
}

void CSND_SetDuty(u32 channel, u32 duty)
{
	u32* cmdparams = csndAddCmd(0x007);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = duty;
}

void CSND_SetTimer(u32 channel, u32 timer)
{
	u32* cmdparams = csndAddCmd(0x008);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = timer;
}

void CSND_SetVol(u32 channel, u32 chnVolumes, u32 capVolumes)
{
	u32* cmdparams = csndAddCmd(0x009);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = chnVolumes;
	cmdparams[2] = capVolumes;
}

void CSND_SetAdpcmState(u32 channel, int block, int sample, int index)
{
	u32* cmdparams = csndAddCmd(block ? 0x00C : 0x00B);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = sample & 0xFFFF;
	cmdparams[2] = index & 0x7F;
}

void CSND_SetAdpcmReload(u32 channel, bool reload)
{
	u32* cmdparams = csndAddCmd(0x00D);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = reload ? 1 : 0;
}

void CSND_SetChnRegs(u32 flags, u32 physaddr0, u32 physaddr1, u32 totalbytesize, u32 chnVolumes, u32 capVolumes)
{
	u32* cmdparams = csndAddCmd(0x00E);

	cmdparams[0] = flags;
	cmdparams[1] = chnVolumes;
	cmdparams[2] = capVolumes;
	cmdparams[3] = physaddr0;
	cmdparams[4] = physaddr1;
	cmdparams[5] = totalbytesize;
}

void CSND_SetChnRegsPSG(u32 flags, u32 chnVolumes, u32 capVolumes, u32 duty)
{
	u32* cmdparams = csndAddCmd(0x00F);

	cmdparams[0] = flags;
	cmdparams[1] = chnVolumes;
	cmdparams[2] = capVolumes;
	cmdparams[3] = duty;
}

void CSND_SetChnRegsNoise(u32 flags, u32 chnVolumes, u32 capVolumes)
{
	u32* cmdparams = csndAddCmd(0x010);

	cmdparams[0] = flags;
	cmdparams[1] = chnVolumes;
	cmdparams[2] = capVolumes;
}

void CSND_CapEnable(u32 capUnit, bool enable)
{
	u32* cmdparams = csndAddCmd(0x100);

	cmdparams[0] = capUnit;
	cmdparams[1] = enable ? 1 : 0;
}

void CSND_CapSetRepeat(u32 capUnit, bool repeat)
{
	u32* cmdparams = csndAddCmd(0x101);

	cmdparams[0] = capUnit;
	cmdparams[1] = repeat ? 0 : 1;
}

void CSND_CapSetFormat(u32 capUnit, bool eightbit)
{
	u32* cmdparams = csndAddCmd(0x102);

	cmdparams[0] = capUnit;
	cmdparams[1] = eightbit ? 1 : 0;
}

void CSND_CapSetBit2(u32 capUnit, bool set)
{
	u32* cmdparams = csndAddCmd(0x103);

	cmdparams[0] = capUnit;
	cmdparams[1] = set ? 1 : 0;
}

void CSND_CapSetTimer(u32 capUnit, u32 timer)
{
	u32* cmdparams = csndAddCmd(0x104);

	cmdparams[0] = capUnit;
	cmdparams[1] = timer & 0xFFFF;
}

void CSND_CapSetBuffer(u32 capUnit, u32 addr, u32 size)
{
	u32* cmdparams = csndAddCmd(0x105);

	cmdparams[0] = capUnit;
	cmdparams[1] = addr;
	cmdparams[2] = size;
}

void CSND_SetCapRegs(u32 capUnit, u32 flags, u32 addr, u32 size)
{
	u32* cmdparams = csndAddCmd(0x106);

	cmdparams[0] = capUnit;
	cmdparams[1] = flags;
	cmdparams[2] = addr;
	cmdparams[3] = size;
}

Result CSND_SetDspFlags(bool waitDone)
{
	csndAddCmd(0x200);
	return csndExecCmds(waitDone);
}

Result CSND_UpdateInfo(bool waitDone)
{
	csndAddCmd(0x300);
	return csndExecCmds(waitDone);
}

Result csndPlaySound(int chn, u32 flags, u32 sampleRate, float vol, float pan, void* data0, void* data1, u32 size)
{
	if (!(csndChannels & BIT(chn)))
		return 1;

	u32 paddr0 = 0, paddr1 = 0;

	int encoding = (flags >> 12) & 3;
	int loopMode = (flags >> 10) & 3;

	if (!loopMode) flags |= SOUND_ONE_SHOT;

	if (encoding != CSND_ENCODING_PSG)
	{
		if (data0) paddr0 = osConvertVirtToPhys((u32)data0);
		if (data1) paddr1 = osConvertVirtToPhys((u32)data1);

		if (data0 && encoding == CSND_ENCODING_ADPCM)
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

	u32 volumes = CSND_VOL(vol, pan);
	CSND_SetChnRegs(flags, paddr0, paddr1, size, volumes, volumes);

	if (loopMode == CSND_LOOPMODE_NORMAL && paddr1 > paddr0)
	{
		// Now that the first block is playing, configure the size of the subsequent blocks
		size -= paddr1 - paddr0;
		CSND_SetBlock(chn, 1, paddr1, size);
	}

	return csndExecCmds(true);
}

void csndGetDspFlags(u32* outSemFlags, u32* outIrqFlags)
{
	if (outSemFlags)
		*outSemFlags = csndSharedMem[(csndOffsets[0] + 0) >> 2];
	if (outIrqFlags)
		*outIrqFlags = csndSharedMem[(csndOffsets[0] + 4) >> 2];
}

static inline u32 chnGetSharedMemIdx(u32 channel)
{
	return __builtin_popcount(((1<<channel)-1) & csndChannels);
}

CSND_ChnInfo* csndGetChnInfo(u32 channel)
{
	channel = chnGetSharedMemIdx(channel);
	return (CSND_ChnInfo*)(&csndSharedMem[(csndOffsets[1] + channel*0xc) >> 2]);
}

CSND_CapInfo* csndGetCapInfo(u32 capUnit)
{
	return (CSND_CapInfo*)(&csndSharedMem[(csndOffsets[2] + capUnit*8) >> 2]);
}

Result csndGetState(u32 channel, CSND_ChnInfo* out)
{
	Result ret = 0;
	channel = chnGetSharedMemIdx(channel);

	if ((ret = CSND_UpdateInfo(true)) != 0)return ret;

	memcpy(out, (const void*)&csndSharedMem[(csndOffsets[1] + channel*0xc) >> 2], 0xc);
	//out[2] -= 0x0c000000;

	return 0;
}

Result csndIsPlaying(u32 channel, u8* status)
{
	Result ret;
	CSND_ChnInfo entry;

	ret = csndGetState(channel, &entry);
	if(ret!=0)return ret;

	*status = entry.active;

	return 0;
}
