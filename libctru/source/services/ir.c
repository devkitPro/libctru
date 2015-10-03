#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/ir.h>
#include <3ds/ipc.h>

static Handle iru_handle=0;
static Handle iru_sharedmem_handle=0;
static u32 *iru_sharedmem = NULL;
static u32 iru_sharedmem_size = 0;

Result irucmd_Initialize(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,0,0); // 0x10000

	if((ret = svcSendSyncRequest(iru_handle))!=0)return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result irucmd_Shutdown(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

	if((ret = svcSendSyncRequest(iru_handle))!=0)return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result irucmd_StartSendTransfer(u8 *buf, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,1,2); // 0x30042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_Buffer(size,IPC_BUFFER_R);
	cmdbuf[3] = (u32)buf;

	if((ret = svcSendSyncRequest(iru_handle))!=0)return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result irucmd_WaitSendTransfer(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,0,0); // 0x40000

	if((ret = svcSendSyncRequest(iru_handle))!=0)return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result irucmd_StartRecvTransfer(u32 size, u8 flag)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,3,2); // 0x500C2
	cmdbuf[1] = iru_sharedmem_size;
	cmdbuf[2] = size;
	cmdbuf[3] = (u8)flag;
	cmdbuf[4] = IPC_Desc_SharedHandles(1);
	cmdbuf[5] = iru_sharedmem_handle;

	if((ret = svcSendSyncRequest(iru_handle))!=0)return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result irucmd_WaitRecvTransfer(u32 *transfercount)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,0,0); // 0x60000

	if((ret = svcSendSyncRequest(iru_handle))!=0)return ret;
	ret = (Result)cmdbuf[1];

	*transfercount = cmdbuf[2];

	return ret;
}

Result IRU_SetBitRate(u8 value)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,1,0); // 0x90040
	cmdbuf[1] = (u32)value;

	if((ret = svcSendSyncRequest(iru_handle))!=0)return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result IRU_GetBitRate(u8 *out)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,0,0); // 0xA0000

	if((ret = svcSendSyncRequest(iru_handle))!=0)return ret;
	ret = (Result)cmdbuf[1];

	*out = (u8)cmdbuf[2];

	return ret;
}

Result IRU_SetIRLEDState(u32 value)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,1,0); // 0xB0040
	cmdbuf[1] = value;

	if((ret = svcSendSyncRequest(iru_handle))!=0)return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result IRU_GetIRLEDRecvState(u32 *out)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC,0,0); // 0xC0000

	if((ret = svcSendSyncRequest(iru_handle))!=0)return ret;
	ret = (Result)cmdbuf[1];

	*out = cmdbuf[2];

	return ret;
}

Result IRU_Initialize(u32 *sharedmem_addr, u32 sharedmem_size)
{
	Result ret = 0;

	if(iru_handle)return 0;

	ret = srvGetServiceHandle(&iru_handle, "ir:u");
	if(ret!=0)return ret;

	ret = irucmd_Initialize();
	if(ret!=0)return ret;

	ret = svcCreateMemoryBlock(&iru_sharedmem_handle, (u32)sharedmem_addr, sharedmem_size, 1, 3);
	if(ret!=0)return ret;

	iru_sharedmem = sharedmem_addr;
	iru_sharedmem_size = sharedmem_size;

	return ret;
}

Result IRU_Shutdown(void)
{
	Result ret = 0;

	if(iru_handle==0)return 0;

	ret = irucmd_Shutdown();
	if(ret!=0)return ret;

	svcCloseHandle(iru_handle);
	svcCloseHandle(iru_sharedmem_handle);

	iru_handle = 0;
	iru_sharedmem_handle = 0;

	return 0;
}

Handle IRU_GetServHandle(void)
{
	return iru_handle;
}

Result IRU_SendData(u8 *buf, u32 size, u32 wait)
{
	Result ret = 0;

	ret = irucmd_StartSendTransfer(buf, size);
	if(ret!=0)return ret;

	if(wait==0)return 0;

	return irucmd_WaitSendTransfer();
}

Result IRU_RecvData(u8 *buf, u32 size, u8 flag, u32 *transfercount, u32 wait)
{
	Result ret = 0;

	*transfercount = 0;

	ret = irucmd_StartRecvTransfer(size, flag);
	if(ret!=0)return ret;

	if(wait)
	{
		ret = irucmd_WaitRecvTransfer(transfercount);
		if(ret!=0)return ret;

		if(buf)memcpy(buf, iru_sharedmem, size);
	}

	return 0;
}

