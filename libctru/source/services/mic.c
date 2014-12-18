#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/mic.h>

//See also: http://3dbrew.org/wiki/MIC_Services

Handle MIC_handle;

static u8 *MIC_sharedmem;
static u32 MIC_sharedmem_size;
static u32 *MIC_sharedmem_offsetfield, MIC_sharedmem_offsetfield_location;
static Handle MIC_sharedmem_handle;
static Handle MIC_event;

static u32 MIC_prev_endpos, MIC_cur_endpos;

Result MIC_Initialize(u32 *sharedmem, u32 sharedmem_size, u8 control, u8 recording, u8 unk0, u8 unk1, u8 unk2)
{
	Result ret=0;

	MIC_sharedmem = (u8*)sharedmem;
	MIC_sharedmem_size = sharedmem_size;
	MIC_sharedmem_offsetfield_location = MIC_sharedmem_size - 4;
	MIC_sharedmem_offsetfield = (u32*)&MIC_sharedmem[MIC_sharedmem_offsetfield_location];
	MIC_event = 0;
	MIC_prev_endpos = 0;
	MIC_cur_endpos = 0;

	ret = srvGetServiceHandle(&MIC_handle, "mic:u");
	if(ret!=0)return ret;

	ret = svcCreateMemoryBlock(&MIC_sharedmem_handle, (u32)MIC_sharedmem, MIC_sharedmem_size, 3, 3);
	if(ret!=0)return ret;

	ret = MIC_SetControl(control);
	if(ret!=0)return ret;

	ret = MIC_MapSharedMem(MIC_sharedmem_handle, sharedmem_size);
	if(ret!=0)return ret;

	ret = MIC_SetRecording(recording);
	if(ret!=0)return ret;

	ret = MIC_cmd3_Initialize(unk0, unk1, 0, MIC_sharedmem_size-4, unk2);
	if(ret!=0)return ret;

	ret = MIC_GetEventHandle(&MIC_event);
	if(ret!=0)return ret;
	svcClearEvent(MIC_event);

	return 0;
}

Result MIC_Shutdown()
{
	Result ret=0;

	MIC_cmd5();
	MIC_SetRecording(0);

	ret = MIC_UnmapSharedMem();
	if(ret!=0)return ret;

	MIC_cmd5();

	ret = svcCloseHandle(MIC_sharedmem_handle);
	if(ret!=0)return ret;

	ret = svcCloseHandle(MIC_event);
	if(ret!=0)return ret;

	ret = svcCloseHandle(MIC_handle);
	if(ret!=0)return ret;

	MIC_sharedmem_offsetfield = NULL;
	MIC_sharedmem = NULL;
	MIC_sharedmem_size = 0;
	MIC_handle = 0;
	MIC_event = 0;

	return 0;
}

u32 MIC_GetSharedMemOffsetValue()
{
	u32 pos = 0;

	if(MIC_sharedmem_offsetfield==NULL)return pos;
	pos = *MIC_sharedmem_offsetfield;
	if(pos > MIC_sharedmem_offsetfield_location)pos = MIC_sharedmem_offsetfield_location;

	return pos;
}

u32 MIC_ReadAudioData(u8 *outbuf, u32 readsize, u32 waitforevent)
{
	u32 pos = 0, bufpos = 0;
	
	if(waitforevent)
	{
		svcClearEvent(MIC_event);
		svcWaitSynchronization(MIC_event, U64_MAX);
	}

	MIC_prev_endpos = MIC_cur_endpos;
	MIC_cur_endpos = MIC_GetSharedMemOffsetValue();
	pos = MIC_prev_endpos;

	while(pos != MIC_cur_endpos)
	{
		if(pos >= MIC_sharedmem_offsetfield_location)pos = 0;
		if(bufpos>=readsize)break;

		outbuf[bufpos] = MIC_sharedmem[pos];
		bufpos++;
		pos++;
	}

	return bufpos;
}

Result MIC_MapSharedMem(Handle handle, u32 size)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00010042;
	cmdbuf[1] = size;
	cmdbuf[2] = 0;
	cmdbuf[3] = handle;

	if((ret = svcSendSyncRequest(MIC_handle))!=0)return ret;

	return (Result)cmdbuf[1];
}

Result MIC_UnmapSharedMem()
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00020000;

	if((ret = svcSendSyncRequest(MIC_handle))!=0)return ret;

	return (Result)cmdbuf[1];
}

Result MIC_cmd3_Initialize(u8 unk0, u8 unk1, u32 sharedmem_baseoffset, u32 sharedmem_endoffset, u8 unk2)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00030140;
	cmdbuf[1] = unk0;
	cmdbuf[2] = unk1;
	cmdbuf[3] = sharedmem_baseoffset;
	cmdbuf[4] = sharedmem_endoffset;
	cmdbuf[5] = unk2;

	if((ret = svcSendSyncRequest(MIC_handle))!=0)return ret;

	return (Result)cmdbuf[1];
}

Result MIC_cmd5()
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00050000;

	if((ret = svcSendSyncRequest(MIC_handle))!=0)return ret;

	return (Result)cmdbuf[1];
}

Result MIC_GetCNTBit15(u8 *out)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00060000;

	if((ret = svcSendSyncRequest(MIC_handle))!=0)return ret;

	if(out)*out = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MIC_GetEventHandle(Handle *handle)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	if(MIC_event)
	{
		*handle = MIC_event;
		return 0;
	}

	cmdbuf[0] = 0x00070000;

	if((ret = svcSendSyncRequest(MIC_handle))!=0)return ret;

	if(handle)*handle = cmdbuf[3];

	return (Result)cmdbuf[1];
}

Result MIC_SetControl(u8 value)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00080040;
	cmdbuf[1] = value;

	if((ret = svcSendSyncRequest(MIC_handle))!=0)return ret;

	return (Result)cmdbuf[1];
}

Result MIC_GetControl(u8 *value)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00090000;

	if((ret = svcSendSyncRequest(MIC_handle))!=0)return ret;

	if(value)*value = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result MIC_SetRecording(u8 value)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000A0040;
	cmdbuf[1] = value;

	if((ret = svcSendSyncRequest(MIC_handle))!=0)return ret;

	if(value==1)MIC_cur_endpos = MIC_GetSharedMemOffsetValue();

	return (Result)cmdbuf[1];
}

Result MIC_IsRecoding(u8 *value)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000B0000;

	if((ret = svcSendSyncRequest(MIC_handle))!=0)return ret;

	if(value)*value = cmdbuf[2];

	return (Result)cmdbuf[1];
}

