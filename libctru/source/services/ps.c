#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ps.h>
#include <3ds/ipc.h>

static Handle psHandle;
static int psRefCount;

static Result _psInit(Handle handle)
{
	Result res=0;
	if (AtomicPostIncrement(&psRefCount)) return 0;
	if(handle==0)res = srvGetServiceHandle(&handle, "ps:ps");
	if (R_FAILED(res)) AtomicDecrement(&psRefCount);
	if (R_SUCCEEDED(res)) psHandle = handle;
	return res;
}

Result psInit(void)
{
	return _psInit(0);
}

Result psInitHandle(Handle handle)
{
	return _psInit(handle);
}

void psExit(void)
{
	if (AtomicDecrement(&psRefCount)) return;
	svcCloseHandle(psHandle);
	psHandle = 0;
}

Handle psGetSessionHandle(void)
{
	return psHandle;
}

Result PS_SignRsaSha256(u8 *hash, psRSAContext *ctx, u8 *signature)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 size;

	size = ctx->rsa_bitsize>>3;

	cmdbuf[0] = IPC_MakeHeader(0x1,9,4); // 0x10244
	memcpy(&cmdbuf[1], hash, 32);
	cmdbuf[9] = size;
	cmdbuf[10] = IPC_Desc_StaticBuffer(0x208, 0);
	cmdbuf[11] = (u32)ctx;
	cmdbuf[12] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[13] = (u32)signature;

	if(R_FAILED(ret = svcSendSyncRequest(psHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PS_VerifyRsaSha256(u8 *hash, psRSAContext *ctx, u8 *signature)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 size;

	size = ctx->rsa_bitsize>>3;

	cmdbuf[0] = IPC_MakeHeader(0x2,9,4); // 0x20244
	memcpy(&cmdbuf[1], hash, 32);
	cmdbuf[9] = size;
	cmdbuf[10] = IPC_Desc_StaticBuffer(0x208, 0);
	cmdbuf[11] = (u32)ctx;
	cmdbuf[12] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[13] = (u32)signature;

	if(R_FAILED(ret = svcSendSyncRequest(psHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PS_EncryptDecryptAes(u32 size, u8* in, u8* out, PS_AESAlgorithm aes_algo, PS_AESKeyType key_type, u8* iv)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	u32 *_iv = (u32*)iv;

	cmdbuf[0] = IPC_MakeHeader(0x4,8,4); // 0x40204
	cmdbuf[1] = size;
	cmdbuf[2] = size;
	memcpy(&cmdbuf[3], _iv, 16);
	cmdbuf[7] = aes_algo;
	cmdbuf[8] = key_type;
	cmdbuf[9] = IPC_Desc_Buffer(size, IPC_BUFFER_R);
	cmdbuf[10] = (u32)in;
	cmdbuf[11] = IPC_Desc_Buffer(size, IPC_BUFFER_W);
	cmdbuf[12] = (u32)out;

	if(R_FAILED(ret = svcSendSyncRequest(psHandle)))return ret;

	memcpy(_iv, &cmdbuf[2], 16);

	return (Result)cmdbuf[1];
}

Result PS_EncryptSignDecryptVerifyAesCcm(u8* in, u32 in_size, u8* out, u32 out_size, u32 data_len, u32 mac_data_len, u32 mac_len, PS_AESAlgorithm aes_algo, PS_AESKeyType key_type, u8* nonce)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	u32 *_nonce = (u32*)nonce;

	cmdbuf[0] = IPC_MakeHeader(0x5,10,4); // 0x50284
	cmdbuf[1] = in_size;
	cmdbuf[2] = mac_data_len;
	cmdbuf[3] = data_len;
	cmdbuf[4] = out_size;
	cmdbuf[5] = mac_len;
	memcpy(&cmdbuf[6], _nonce, 12);
	cmdbuf[9] = aes_algo;
	cmdbuf[10] = key_type;
	cmdbuf[11] = IPC_Desc_Buffer(in_size, IPC_BUFFER_R);
	cmdbuf[12] = (u32)in;
	cmdbuf[13] = IPC_Desc_Buffer(out_size, IPC_BUFFER_W);
	cmdbuf[14] = (u32)out;

	if(R_FAILED(ret = svcSendSyncRequest(psHandle)))return ret;

	return (Result)cmdbuf[1];
}

Result PS_GetLocalFriendCodeSeed(u64* seed)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,0,0); // 0xA0000

	if(R_FAILED(ret = svcSendSyncRequest(psHandle)))return ret;

	*seed = (u64)cmdbuf[2] | (u64)cmdbuf[3] << 32;

	return (Result)cmdbuf[1];
}

Result PS_GetDeviceId(u32* device_id)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,0,0); // 0xB0000

	if(R_FAILED(ret = svcSendSyncRequest(psHandle)))return ret;

	*device_id = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result PS_GenerateRandomBytes(void* out, size_t len)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD,1,2); // 0xD0042
	cmdbuf[1] = len;
	cmdbuf[2] = IPC_Desc_Buffer(len, IPC_BUFFER_W);
	cmdbuf[3] = (u32)out;

	if(R_FAILED(ret = svcSendSyncRequest(psHandle)))return ret;

	return (Result)cmdbuf[1];
}
