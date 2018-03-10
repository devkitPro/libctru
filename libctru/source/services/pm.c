#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/pm.h>
#include <3ds/ipc.h>

static Handle pmHandle;
static int pmRefCount;

Result pmInit(void)
{
	Result res;
	if (AtomicPostIncrement(&pmRefCount)) return 0;
	res = srvGetServiceHandle(&pmHandle, "pm:app");
	if (R_FAILED(res)) AtomicDecrement(&pmRefCount);
	return res;
}

void pmExit(void)
{
	if (AtomicDecrement(&pmRefCount)) return;
	svcCloseHandle(pmHandle);
}

Result PM_LaunchTitle(u8 mediatype, u64 titleid, u32 launch_flags)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,5,0); // 0x10140
	cmdbuf[1] = titleid & 0xffffffff;
	cmdbuf[2] = (titleid >> 32) & 0xffffffff;
	cmdbuf[3] = mediatype;
	cmdbuf[4] = 0x0;
	cmdbuf[5] = launch_flags;

	if(R_FAILED(ret = svcSendSyncRequest(pmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PM_LaunchFIRMSetParams(u32 firm_titleid_low, u32 size, u8* in)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,2,2); // 0x20082
	cmdbuf[1] = firm_titleid_low;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size,IPC_BUFFER_R);
	cmdbuf[4] = (u32)in;

	if(R_FAILED(ret = svcSendSyncRequest(pmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PM_TerminateCurrentApplication(u64 timeout)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,2,0); // 0x30080
	cmdbuf[1] = timeout & 0xffffffff;
	cmdbuf[2] = (timeout >> 32) & 0xffffffff;

	if(R_FAILED(ret = svcSendSyncRequest(pmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PM_TerminateProcess(u8 pid, u64 timeout)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,3,0); // 0x500C0
	cmdbuf[1] = pid;
	cmdbuf[2] = timeout & 0xffffffff;
	cmdbuf[3] = (timeout >> 32) & 0xffffffff;

	if(R_FAILED(ret = svcSendSyncRequest(pmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PM_GetFIRMLaunchParams(u32 size, u8* out)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7,1,2); // 0x70042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_Buffer(size,IPC_BUFFER_W);
	cmdbuf[3] = (u32)out;

	if(R_FAILED(ret = svcSendSyncRequest(pmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PM_GetTitleExheaderFlags(u8 mediatype, u64 titleid, u8* out)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8,4,0); // 0x80100
	cmdbuf[1] = titleid & 0xffffffff;
	cmdbuf[2] = (titleid >> 32) & 0xffffffff;
	cmdbuf[3] = mediatype;
	cmdbuf[4] = 0x0;

	if(R_FAILED(ret = svcSendSyncRequest(pmHandle)))return ret;

	memcpy(out, (u8*)(&cmdbuf[2]), 8);

	return (Result)cmdbuf[1];
}

Result PM_SetFIRMLaunchParams(u32 size, u8* in)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,1,2); // 0x90042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_Buffer(size,IPC_BUFFER_R);
	cmdbuf[3] = (u32)in;

	if(R_FAILED(ret = svcSendSyncRequest(pmHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PM_UnregisterProcess(u64 tid)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC,0,0); // 0xC0000
	cmdbuf[1] = tid & 0xffffffff;
	cmdbuf[2] = (tid >> 32) & 0xffffffff;

	if(R_FAILED(ret = svcSendSyncRequest(pmHandle)))return ret;

	return (Result)cmdbuf[1];
}