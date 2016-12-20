#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/boss.h>
#include <3ds/ipc.h>
#include <3ds/env.h>

static Handle bossHandle;
static int bossRefCount;
static u32 bossPriv = 0;

static Result bossipc_InitializeSession(u64 programID);

Result bossInit(u64 programID)
{
	Result res=0;
	Handle envhandle=0;
	Handle handle=0;

	if (AtomicPostIncrement(&bossRefCount)) return 0;

	res = srvGetServiceHandle(&handle, "boss:P");
	envhandle = envGetHandle("boss:P");
	bossPriv = 1;
	if (R_FAILED(res))
	{
		bossPriv = 0;
		res = srvGetServiceHandle(&handle, "boss:U");
		envhandle = envGetHandle("boss:U");
	}

	if (R_FAILED(res)) AtomicDecrement(&bossRefCount);

	if (R_SUCCEEDED(res))
	{
		bossHandle = handle;

		if(envhandle==0)res = bossipc_InitializeSession(programID);

		if (R_FAILED(res))bossExit();
	}

	return res;
}

void bossExit(void)
{
	if (AtomicDecrement(&bossRefCount)) return;
	svcCloseHandle(bossHandle);
	bossHandle = 0;
}

Handle bossGetSessionHandle()
{
	return bossHandle;
}

static Result bossipc_InitializeSession(u64 programID)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	if(bossPriv==0)cmdbuf[0] = IPC_MakeHeader(0x1,2,2); // 0x10082
	if(bossPriv)cmdbuf[0] = IPC_MakeHeader(0x0401,2,2); // 0x04010082
	cmdbuf[1] = (u32) programID;
	cmdbuf[2] = (u32) (programID >> 32);
	cmdbuf[3] = IPC_Desc_CurProcessHandle();

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result bossStartTaskImmediate(char *taskID)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 size = strlen(taskID)+1;

	cmdbuf[0] = IPC_MakeHeader(0x1D,1,2); // 0x1D0042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[3] = (u32)taskID;

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result bossStartBgImmediate(char *taskID)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 size = strlen(taskID)+1;

	cmdbuf[0] = IPC_MakeHeader(0x33,1,2); // 0x330042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[3] = (u32)taskID;

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;

	return (Result)cmdbuf[1];
}

