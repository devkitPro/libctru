#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/ps.h>
#include <3ds/ipc.h>

static Handle psHandle;
static int psRefCount;

Result psInit(void)
{
	Result res;
	if (AtomicPostIncrement(&psRefCount)) return 0;
	res = srvGetServiceHandle(&psHandle, "ps:ps");
	if (R_FAILED(res)) AtomicDecrement(&psRefCount);
	return res;
}

void psExit(void)
{
	if (AtomicDecrement(&psRefCount)) return;
	svcCloseHandle(psHandle);
}

Result PS_EncryptDecryptAes(u32 size, u8* in, u8* out, PS_AESAlgorithm aes_algo, PS_AESKeyType key_type, u8* iv)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	u32 *_iv = (u32*)iv;

	cmdbuf[0] = IPC_MakeHeader(0x4,7,4); // 0x401C4
	cmdbuf[1] = size;
	cmdbuf[2] = _iv[0];
	cmdbuf[3] = _iv[1];
	cmdbuf[4] = _iv[2];
	cmdbuf[5] = _iv[3];
	cmdbuf[6] = aes_algo;
	cmdbuf[7] = key_type;
	cmdbuf[8] = IPC_Desc_PXIBuffer(size,0,false);
	cmdbuf[9] = (u32)in;
	cmdbuf[10] = IPC_Desc_PXIBuffer(size,1,false);
	cmdbuf[11] = (u32)out;

	if(R_FAILED(ret = svcSendSyncRequest(psHandle)))return ret;

	_iv[0] = cmdbuf[2] & 0xFF;
	_iv[1] = cmdbuf[3] & 0xFF;
	_iv[2] = cmdbuf[4] & 0xFF;
	_iv[3] = cmdbuf[5] & 0xFF;

	return (Result)cmdbuf[1];
}

Result PS_EncryptSignDecryptVerifyAesCcm(u8* in, u32 in_size, u8* out, u32 out_size, u32 data_len, u32 mac_data_len, u32 mac_len, PS_AESAlgorithm aes_algo, PS_AESKeyType key_type, u8* nonce)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	u32 *_nonce = (u32*)nonce;

	cmdbuf[0] = IPC_MakeHeader(0x5,10,4); // 0x50284
	cmdbuf[1] = in_size;
	cmdbuf[2] = out_size;
	cmdbuf[3] = mac_data_len;
	cmdbuf[4] = data_len;
	cmdbuf[5] = mac_len;
	cmdbuf[6] = _nonce[0];
	cmdbuf[7] = _nonce[1];
	cmdbuf[8] = _nonce[2];
	cmdbuf[9] = aes_algo;
	cmdbuf[10] = key_type;
	cmdbuf[8] = IPC_Desc_PXIBuffer(in_size,0,false);
	cmdbuf[9] = (u32)in;
	cmdbuf[10] = IPC_Desc_PXIBuffer(out_size,1,false);
	cmdbuf[11] = (u32)out;

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
