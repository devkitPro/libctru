#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/cfgnor.h>
#include <3ds/ipc.h>

Handle cfgnorHandle;

Result cfgnorInit(u8 value)
{
	Result ret = 0;
	if(R_FAILED(ret = srvGetServiceHandle(&cfgnorHandle, "cfg:nor")))return ret;
	if(R_FAILED(ret = CFGNOR_Initialize(value))) svcCloseHandle(cfgnorHandle);
	return ret;
}

void cfgnorExit(void)
{
	if(cfgnorHandle != 0)
	{
		CFGNOR_Shutdown();
		svcCloseHandle(cfgnorHandle);
		cfgnorHandle = 0;
	}
}

Result cfgnorDumpFlash(u32 *buf, u32 size)
{
	Result ret = 0;
	u32 pos=0;
	u32 chunksize = 0x100;

	for(pos=0; pos<size; pos+=chunksize)
	{
		if(size-pos < chunksize)chunksize = size-pos;

		ret = CFGNOR_ReadData(pos, &buf[pos>>2], chunksize);
		if(R_FAILED(ret))break;
	}

	return ret;
}

Result cfgnorWriteFlash(u32 *buf, u32 size)
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

Result CFGNOR_Initialize(u8 value)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,1,0); // 0x10040
	cmdbuf[1] = (u32)value;

	if(R_FAILED(ret = svcSendSyncRequest(cfgnorHandle)))return ret;

	return cmdbuf[1];
}

Result CFGNOR_Shutdown(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

	if(R_FAILED(ret = svcSendSyncRequest(cfgnorHandle)))return ret;

	return cmdbuf[1];
}

Result CFGNOR_ReadData(u32 offset, u32 *buf, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,2,2); // 0x50082
	cmdbuf[1] = offset;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[4] = (u32)buf;

	if(R_FAILED(ret = svcSendSyncRequest(cfgnorHandle)))return ret;

	return cmdbuf[1];
}

Result CFGNOR_WriteData(u32 offset, u32 *buf, u32 size)
{
	u32 ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,2,2); // 0x60082
	cmdbuf[1] = offset;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size,IPC_BUFFER_R);
	cmdbuf[4] = (u32)buf;

	if(R_FAILED(ret = svcSendSyncRequest(cfgnorHandle)))return ret;

	return cmdbuf[1];
}

