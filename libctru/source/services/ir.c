#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ir.h>
#include <3ds/ipc.h>

static Handle iru_handle;
static Handle iru_sharedmem_handle;
static u32 *iru_sharedmem;
static u32 iru_sharedmem_size;
static int iru_refcount;

Result irucmd_Initialize(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,0,0); // 0x10000

	if(R_FAILED(ret = svcSendSyncRequest(iru_handle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result irucmd_Shutdown(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

	if(R_FAILED(ret = svcSendSyncRequest(iru_handle)))return ret;
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

	if(R_FAILED(ret = svcSendSyncRequest(iru_handle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result irucmd_WaitSendTransfer(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,0,0); // 0x40000

	if(R_FAILED(ret = svcSendSyncRequest(iru_handle)))return ret;
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

	if(R_FAILED(ret = svcSendSyncRequest(iru_handle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result irucmd_WaitRecvTransfer(u32 *transfercount)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,0,0); // 0x60000

	if(R_FAILED(ret = svcSendSyncRequest(iru_handle)))return ret;
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

	if(R_FAILED(ret = svcSendSyncRequest(iru_handle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result IRU_GetBitRate(u8 *out)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,0,0); // 0xA0000

	if(R_FAILED(ret = svcSendSyncRequest(iru_handle)))return ret;
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

	if(R_FAILED(ret = svcSendSyncRequest(iru_handle)))return ret;
	ret = (Result)cmdbuf[1];

	return ret;
}

Result IRU_GetIRLEDRecvState(u32 *out)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC,0,0); // 0xC0000

	if(R_FAILED(ret = svcSendSyncRequest(iru_handle)))return ret;
	ret = (Result)cmdbuf[1];

	*out = cmdbuf[2];

	return ret;
}

Result iruInit(u32 *sharedmem_addr, u32 sharedmem_size)
{
	Result ret = 0;

	if(AtomicPostIncrement(&iru_refcount)) return 0;

	ret = srvGetServiceHandle(&iru_handle, "ir:u");
	if(R_FAILED(ret))goto cleanup0;

	ret = irucmd_Initialize();
	if(R_FAILED(ret))goto cleanup1;

	ret = svcCreateMemoryBlock(&iru_sharedmem_handle, (u32)sharedmem_addr, sharedmem_size, 1, 3);
	if(R_FAILED(ret))goto cleanup2;

	iru_sharedmem = sharedmem_addr;
	iru_sharedmem_size = sharedmem_size;

	return ret;

cleanup2:
	irucmd_Shutdown();
cleanup1:
	svcCloseHandle(iru_handle);
cleanup0:
	AtomicDecrement(&iru_refcount);
	return ret;
}

void iruExit(void)
{
	if(AtomicDecrement(&iru_refcount)) return;

	irucmd_Shutdown();
	svcCloseHandle(iru_handle);
	svcCloseHandle(iru_sharedmem_handle);

	iru_handle = 0;
	iru_sharedmem_handle = 0;
}

Handle iruGetServHandle(void)
{
	return iru_handle;
}

Result iruSendData(u8 *buf, u32 size, bool wait)
{
	Result ret = 0;

	ret = irucmd_StartSendTransfer(buf, size);
	if(R_FAILED(ret))return ret;

	if(!wait)return 0;

	return irucmd_WaitSendTransfer();
}

Result iruRecvData(u8 *buf, u32 size, u8 flag, u32 *transfercount, bool wait)
{
	Result ret = 0;

	*transfercount = 0;

	ret = irucmd_StartRecvTransfer(size, flag);
	if(R_FAILED(ret))return ret;

	if(wait)
	{
		ret = irucmd_WaitRecvTransfer(transfercount);
		if(R_FAILED(ret))return ret;

		if(buf)memcpy(buf, iru_sharedmem, size);
	}

	return 0;
}

