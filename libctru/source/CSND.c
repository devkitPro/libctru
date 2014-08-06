#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>

#include <ctr/OS.h>
#include <ctr/svc.h>
#include <ctr/srv.h>

#include <ctr/CSND.h>

//See here regarding CSND shared-mem commands, etc: http://3dbrew.org/wiki/CSND_Shared_Memory

Handle CSND_handle = 0;
Handle CSND_mutexhandle = 0;
Handle CSND_sharedmemhandle = 0;
u32 *CSND_sharedmem = NULL;
static u32 CSND_bitmask=0;

static u32 CSND_sharedmem_cmdblocksize = 0x2000;
static u32 CSND_sharedmem_startcmdoff = 0;
static u32 CSND_sharedmem_currentcmdoff = 0;

Result CSND_cmd1(Handle *mutexhandle, Handle *sharedmemhandle, u32 sharedmem_size, u32 off0, u32 off1, u32 off2, u32 off3)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00010140;
	cmdbuf[1] = sharedmem_size;
	cmdbuf[2] = off0;
	cmdbuf[3] = off1;
	cmdbuf[4] = off2;
	cmdbuf[5] = off3;

	if((ret = svc_sendSyncRequest(CSND_handle))!=0)return ret;

	*mutexhandle = cmdbuf[3];
	*sharedmemhandle = cmdbuf[4];

	return (Result)cmdbuf[1];
}

Result CSND_cmd2()
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00020000;

	if((ret = svc_sendSyncRequest(CSND_handle))!=0)return ret;

	return (Result)cmdbuf[1];
}

Result CSND_cmd5(u32 *bitmask)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00050000;

	if((ret = svc_sendSyncRequest(CSND_handle))!=0)return ret;

	*bitmask = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result CSND_initialize(u32* sharedMem)
{
	Result ret=0;

	if(sharedMem==NULL)sharedMem = (u32*)CSND_SHAREDMEM_DEFAULT;
	CSND_sharedmem = sharedMem;

	ret = srv_getServiceHandle(NULL, &CSND_handle, "csnd:SND");
	if(ret!=0)return ret;

	ret = CSND_cmd1(&CSND_mutexhandle, &CSND_sharedmemhandle, CSND_sharedmem_cmdblocksize+0x114, CSND_sharedmem_cmdblocksize, CSND_sharedmem_cmdblocksize+8, CSND_sharedmem_cmdblocksize+0xc8, CSND_sharedmem_cmdblocksize+0xd8);
	if(ret!=0)return ret;

	ret = svc_mapMemoryBlock(CSND_sharedmemhandle, (u32)CSND_sharedmem, 3, 0x10000000);
	if(ret!=0)return ret;

	memset(CSND_sharedmem, 0, 0x2114);

	ret = CSND_cmd5(&CSND_bitmask);
	if(ret!=0)return ret;

	return 0;
}

Result CSND_shutdown()
{
	Result ret;

	svc_unmapMemoryBlock(CSND_sharedmemhandle, (u32)CSND_sharedmem);
	svc_closeHandle(CSND_sharedmemhandle);

	ret = CSND_cmd2();
	if(ret!=0)return ret;

	return svc_closeHandle(CSND_handle);
}

Result CSND_cmd3(u32 offset)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00030040;
	cmdbuf[1] = offset;

	if((ret = svc_sendSyncRequest(CSND_handle))!=0)return ret;

	return (Result)cmdbuf[1];
}

void CSND_writesharedmem_cmdtype0(u16 cmdid, u8 *cmdparams)
{
	u16 *ptr;
	u32 prevoff;
	s32 outindex=0;

	svc_waitSynchronizationN(&outindex, &CSND_mutexhandle, 1, 0, ~0);

	if(CSND_sharedmem_startcmdoff != CSND_sharedmem_currentcmdoff)
	{
		if(CSND_sharedmem_currentcmdoff>=0x20)
		{
			prevoff = CSND_sharedmem_currentcmdoff-0x20;
		}
		else
		{
			prevoff = CSND_sharedmem_cmdblocksize-0x20;
		}

		ptr = (u16*)&CSND_sharedmem[prevoff>>2];
		*ptr = CSND_sharedmem_currentcmdoff;
	}

	ptr = (u16*)&CSND_sharedmem[CSND_sharedmem_currentcmdoff>>2];

	ptr[0] = 0xffff;
	ptr[1] = cmdid;
	ptr[2] = 0;
	ptr[3] = 0;
	memcpy(&ptr[8>>1], cmdparams, 0x18);

	CSND_sharedmem_currentcmdoff+= 0x20;
	if(CSND_sharedmem_currentcmdoff >= CSND_sharedmem_cmdblocksize)CSND_sharedmem_currentcmdoff = 0;

	svc_releaseMutex(CSND_mutexhandle);
}

Result CSND_processtype0cmds()
{
	Result ret=0;

	if(CSND_sharedmem_startcmdoff == CSND_sharedmem_currentcmdoff)return 0;

	ret = CSND_cmd3(CSND_sharedmem_startcmdoff);
	CSND_sharedmem_startcmdoff = CSND_sharedmem_currentcmdoff;

	return ret;
}

u32 CSND_convertsamplerate(u32 samplerate)
{
	return (u32)(6.7027964E+07f / ((float)samplerate));
}

void CSND_sharedmemtype0_cmd0(u32 channel, u32 value)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;

	CSND_writesharedmem_cmdtype0(0x0, (u8*)&cmdparams);
}

void CSND_setchannel_playbackstate(u32 channel, u32 value)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;

	CSND_writesharedmem_cmdtype0(0x1, (u8*)&cmdparams);
}

void CSND_sharedmemtype0_cmd3(u32 channel, u32 physaddr, u32 size)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = physaddr;
	cmdparams[2] = size;

	CSND_writesharedmem_cmdtype0(0x3, (u8*)&cmdparams);
}

void CSND_sharedmemtype0_cmd9(u32 channel, u16 value)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value | (value<<16);

	CSND_writesharedmem_cmdtype0(0x9, (u8*)&cmdparams);
}

void CSND_sharedmemtype0_cmd8(u32 channel, u32 samplerate)
{
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = CSND_convertsamplerate(samplerate);

	CSND_writesharedmem_cmdtype0(0x8, (u8*)&cmdparams);
}

void CSND_sharedmemtype0_cmde(u32 channel, CSND_LOOPING looping, CSND_ENCODING encoding, u32 samplerate, u32 unk0, u32 unk1, u32 physaddr0, u32 physaddr1, u32 totalbytesize)
{
	u32 val;
	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	cmdparams[0] = channel & 0x1f;
	cmdparams[0] |= (unk0 & 0xf) << 6;
	if(looping==CSND_LOOP_DISABLE)cmdparams[0] |= 2 << 10;
	if(looping==CSND_LOOP_ENABLE)cmdparams[0] |= 1 << 10;
	cmdparams[0] |= (encoding & 3) << 12;
	cmdparams[0] |= (unk1 & 3) << 14;

	val = CSND_convertsamplerate(samplerate);
	if(val<0x42)val = 0x42;
	if(val>0xffff)val = 0xffff;
	cmdparams[0] |= val<<16;

	cmdparams[3] = physaddr0;
	cmdparams[4] = physaddr1;
	cmdparams[5] = totalbytesize;

	CSND_writesharedmem_cmdtype0(0xe, (u8*)&cmdparams);
}

Result CSND_sharedmemtype0_cmdupdatestate(int waitdone)
{
	u8 *ptr;
	int ret=0;

	u32 cmdparams[0x18>>2];

	memset(cmdparams, 0, 0x18);

	ptr = (u8*)&CSND_sharedmem[CSND_sharedmem_startcmdoff>>2];

	CSND_writesharedmem_cmdtype0(0x300, (u8*)&cmdparams);

	ret = CSND_processtype0cmds();
	if(ret!=0)return ret;

	if(waitdone)
	{
		while(*ptr == 0);
	}

	return 0;
}

Result CSND_playsound(u32 channel, CSND_LOOPING looping, CSND_ENCODING encoding, u32 samplerate, u32 *vaddr0, u32 *vaddr1, u32 totalbytesize, u32 unk0, u32 unk1)
{
	u32 physaddr0 = 0;
	u32 physaddr1 = 0;

	physaddr0 = OS_ConvertVaddr2Physaddr((u32)vaddr0);
	physaddr1 = OS_ConvertVaddr2Physaddr((u32)vaddr1);

	CSND_sharedmemtype0_cmde(channel, looping, encoding, samplerate, unk0, unk1, physaddr0, physaddr1, totalbytesize);
	CSND_sharedmemtype0_cmd8(channel, samplerate);
	if(looping==CSND_LOOP_ENABLE)CSND_sharedmemtype0_cmd3(channel, physaddr0, totalbytesize);
	CSND_sharedmemtype0_cmd8(channel, samplerate);
	CSND_sharedmemtype0_cmd9(channel, 0xffff);
	CSND_setchannel_playbackstate(channel, 1);

	return CSND_sharedmemtype0_cmdupdatestate(0);
}

Result CSND_getchannelstate(u32 entryindex, u32 *out)
{
	Result ret=0;

	if((ret = CSND_sharedmemtype0_cmdupdatestate(1))!=0)return ret;

	memcpy(out, &CSND_sharedmem[(CSND_sharedmem_cmdblocksize+8 + entryindex*0xc) >> 2], 0xc);
	out[2] -= 0x0c000000;

	return 0;
}

Result CSND_getchannelstate_isplaying(u32 entryindex, u8 *status)
{
	Result ret;
	u32 entry[0xc>>2];

	ret = CSND_getchannelstate(entryindex, entry);
	if(ret!=0)return ret;

	*status = entry[0] & 0xff;

	return 0;
}

