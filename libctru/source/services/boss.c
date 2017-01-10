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

Result bossInit(u64 programID, bool force_user)
{
	Result res=0;
	Handle envhandle=0;
	Handle handle=0;

	if (AtomicPostIncrement(&bossRefCount)) return 0;

	res = -1;

	if(!force_user)
	{
		res = srvGetServiceHandle(&handle, "boss:P");
		envhandle = envGetHandle("boss:P");
		bossPriv = 1;
	}

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

Result bossReinit(u64 programID)
{
	return bossipc_InitializeSession(programID);
}

void bossExit(void)
{
	if (AtomicDecrement(&bossRefCount)) return;
	svcCloseHandle(bossHandle);
	bossHandle = 0;
}

Handle bossGetSessionHandle(void)
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

Result bossSetStorageInfo(u64 extdataID, u32 boss_size, u8 mediaType)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,4,0); // 0x20100
	cmdbuf[1] = (u32) extdataID;
	cmdbuf[2] = (u32) (extdataID >> 32);
	cmdbuf[3] = boss_size;
	cmdbuf[4] = mediaType;

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result bossUnregisterStorage(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,0,0); // 0x30000

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result bossRegisterTask(const char *taskID, u8 unk0, u8 unk1)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 size = strlen(taskID)+1;

	cmdbuf[0] = IPC_MakeHeader(0xB,3,2); // 0xB00C2
	cmdbuf[1] = size;
	cmdbuf[2] = unk0;
	cmdbuf[3] = unk1;
	cmdbuf[4] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[5] = (u32)taskID;

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;

	return (Result)cmdbuf[1];
}

static Result bossipc_UnregisterTask(const char *taskID, u32 unk)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 size = strlen(taskID)+1;

	cmdbuf[0] = IPC_MakeHeader(0xC,2,2); // 0xC0082
	cmdbuf[1] = size;
	cmdbuf[2] = unk;
	cmdbuf[3] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[4] = (u32)taskID;

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result bossSendProperty(u16 PropertyID, const void* buf, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x14,2,2); // 0x140082
	cmdbuf[1] = PropertyID;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[4] = (u32)buf;

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result bossStartTaskImmediate(const char *taskID)
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

Result bossipc_CancelTask(const char *taskID)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 size = strlen(taskID)+1;

	cmdbuf[0] = IPC_MakeHeader(0x1E,1,2); // 0x1E0042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[3] = (u32)taskID;

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result bossDeleteTask(const char *taskID, u32 unk)
{
	Result ret=0;

	ret = bossipc_CancelTask(taskID);
	if(R_FAILED(ret))return ret;

	ret = bossipc_UnregisterTask(taskID, unk);

	return ret;
}

Result bossGetTaskState(const char *taskID, s8 inval, u8 *status, u32 *out1, u8 *out2)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 size = strlen(taskID)+1;

	cmdbuf[0] = IPC_MakeHeader(0x20,2,2); // 0x200082
	cmdbuf[1] = size;
	cmdbuf[2] = inval;
	cmdbuf[3] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[4] = (u32)taskID;

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;
	ret = (Result)cmdbuf[1];

	if(R_SUCCEEDED(ret))
	{
		if(status)*status = cmdbuf[2];
		if(out1)*out1 = cmdbuf[3];
		if(out2)*out2 = cmdbuf[4];
	}

	return ret;
}

Result bossDeleteNsData(u32 NsDataId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x26,1,0); // 0x260040
	cmdbuf[1] = NsDataId;

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result bossGetNsDataHeaderInfo(u32 NsDataId, u8 type, void* buffer, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x27,3,2); // 0x2700C2
	cmdbuf[1] = NsDataId;
	cmdbuf[2] = type;
	cmdbuf[3] = size;
	cmdbuf[4] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[5] = (u32)buffer;

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result bossReadNsData(u32 NsDataId, u64 offset, void* buffer, u32 size, u32 *transfer_total, u32 *unk_out)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x28,4,2); // 0x280102
	cmdbuf[1] = NsDataId;
	cmdbuf[2] = (u32) offset;
	cmdbuf[3] = (u32) (offset >> 32);
	cmdbuf[4] = size;
	cmdbuf[5] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[6] = (u32)buffer;

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;

	if(R_SUCCEEDED(ret))
	{
		if(transfer_total)*transfer_total = cmdbuf[2];
		if(unk_out)*unk_out = cmdbuf[3];
	}

	return (Result)cmdbuf[1];
}

Result bossStartBgImmediate(const char *taskID)
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

Result bossGetTaskProperty0(const char *taskID, u8 *out)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 size = strlen(taskID)+1;

	cmdbuf[0] = IPC_MakeHeader(0x34,1,2); // 0x340042
	cmdbuf[1] = size;
	cmdbuf[2] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[3] = (u32)taskID;

	if(R_FAILED(ret = svcSendSyncRequest(bossHandle)))return ret;
	ret = (Result)cmdbuf[1];

	if(R_SUCCEEDED(ret) && out)*out = cmdbuf[2];

	return ret;
}

void bossSetupContextDefault(bossContext *ctx, u32 seconds_interval, const char *url)
{
	memset(ctx, 0, sizeof(bossContext));

	ctx->property[0x0] = 0xaa;
	ctx->property[0x1] = 0x01;
	ctx->property[0x2] = 0x00;
	ctx->property[0x3] = seconds_interval;
	ctx->property[0x4] = 0x64;
	ctx->property[0x5] = 0x02;
	ctx->property[0x6] = 0x02;

	memset(ctx->url, 0, sizeof(ctx->url));
	strncpy(ctx->url, url, sizeof(ctx->url)-1);

	ctx->property_x8 = 0x00;

	ctx->property_x9 = 0x00;

	memset(ctx->property_xa, 0, sizeof(ctx->property_xa));
	memset(ctx->property_xd, 0, sizeof(ctx->property_xd));

	ctx->property_xe = 0x00;

	memset(ctx->property_xf, 0, sizeof(ctx->property_xf));
	ctx->property_xf[0] = 0x07;
	ctx->property_xf[1] = 0x03;

	ctx->property_x10 = 0x00;

	ctx->property_x11 = 0x00;

	ctx->property_x12 = 0x00;

	ctx->property_x13 = 0x02;

	ctx->property_x14 = 0x01;

	memset(ctx->property_x15, 0, sizeof(ctx->property_x15));

	ctx->property_x16 = 0x00;

	ctx->property_x3b = 0x00;
}

Result bossSendContextConfig(bossContext *ctx)
{
	Result ret=0;

	ret = bossSendProperty(0x0, &ctx->property[0x0], 0x1);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x1, &ctx->property[0x1], 0x1);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x2, &ctx->property[0x2], 0x4);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x3, &ctx->property[0x3], 0x4);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x4, &ctx->property[0x4], 0x4);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x5, &ctx->property[0x5], 0x4);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x6, &ctx->property[0x6], 0x4);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x7, ctx->url, 0x200);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x8, &ctx->property_x8, 0x4);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x9, &ctx->property_x9, 0x1);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0xa, &ctx->property_xa, 0x100);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0xb, &ctx->property_xb, 0x200);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0xd, ctx->property_xd, 0x360);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0xe, &ctx->property_xe, 0x4);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0xf, ctx->property_xf, 0xc);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x10, &ctx->property_x10, 0x1);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x11, &ctx->property_x11, 0x1);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x12, &ctx->property_x12, 0x1);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x13, &ctx->property_x13, 0x4);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x14, &ctx->property_x14, 0x4);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x15, ctx->property_x15, 0x40);

	ret = bossSendProperty(0x16, &ctx->property_x16, 0x4);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x3b, &ctx->property_x3b, 0x4);
	if(R_FAILED(ret))return ret;

	ret = bossSendProperty(0x3e, &ctx->property_x3e, 0x200);
	if(R_FAILED(ret))return ret;

	return ret;
}

