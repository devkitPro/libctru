#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <ctr/types.h>
#include <ctr/svc.h>
#include <ctr/srv.h>

#include <ctr/CFGNOR.h>

Handle CFGNOR_handle = 0;

Result CFGNOR_Initialize(u8 value)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	ret = srv_getServiceHandle(NULL, &CFGNOR_handle, "cfg:nor");
	if(ret!=0)return ret;

	cmdbuf[0] = 0x00010040;
	cmdbuf[1] = (u32)value;

	if((ret = svc_sendSyncRequest(CFGNOR_handle))!=0)return ret;

	ret = (Result)cmdbuf[1];
	return ret;
}

Result CFGNOR_Shutdown()
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00020000;

	if((ret = svc_sendSyncRequest(CFGNOR_handle))!=0)return ret;
	ret = (Result)cmdbuf[1];

	svc_closeHandle(CFGNOR_handle);
	CFGNOR_handle = 0;

	return ret;
}

Result CFGNOR_ReadData(u32 offset, u32 *buf, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00050082;
	cmdbuf[1] = offset;
	cmdbuf[2] = size;
	cmdbuf[3] = (size<<4) | 12;
	cmdbuf[4] = (u32)buf;

	if((ret = svc_sendSyncRequest(CFGNOR_handle))!=0)return ret;

	ret = (Result)cmdbuf[1];
	return ret;
}

Result CFGNOR_WriteData(u32 offset, u32 *buf, u32 size)
{
	u32 ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00060082;
	cmdbuf[1] = offset;
	cmdbuf[2] = size;
	cmdbuf[3] = (size<<4) | 10;
	cmdbuf[4] = (u32)buf;

	if((ret = svc_sendSyncRequest(CFGNOR_handle))!=0)return ret;

	ret = (Result)cmdbuf[1];
	return ret;
}

Result CFGNOR_DumpFlash(u32 *buf, u32 size)
{
	Result ret = 0;
	u32 pos=0;
	u32 chunksize = 0x100;

	for(pos=0; pos<size; pos+=chunksize)
	{
		if(size-pos < chunksize)chunksize = size-pos;

		ret = CFGNOR_ReadData(pos, &buf[pos>>2], chunksize);
		if(ret!=0)break;
	}

	return ret;
}

Result CFGNOR_WriteFlash(u32 *buf, u32 size)
{
	Result ret = 0;
	u32 pos=0;
	u32 chunksize = 0x100;

	for(pos=0; pos<size; pos+=chunksize)
	{
		if(size-pos < chunksize)chunksize = size-pos;

		ret = CFGNOR_WriteData(pos, &buf[pos>>2], chunksize);
		if(ret!=0)break;
	}

	return ret;
}

