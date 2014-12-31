#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/ps.h>

static Handle psHandle;

Result psInit()
{
	return srvGetServiceHandle(&psHandle, "ps:ps");
}

Result psExit()
{
	return svcCloseHandle(psHandle);
}

Result PS_EncryptDecryptAes(u32 size, u8* in, u8* out, u32 aes_algo, u32 key_type, u8* iv)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	u32 *_iv = (u32*)iv;

	cmdbuf[0] = 0x000401C4;
	cmdbuf[1] = size;
	cmdbuf[2] = _iv[0];
	cmdbuf[3] = _iv[1];
	cmdbuf[4] = _iv[2];
	cmdbuf[5] = _iv[3];
	cmdbuf[6] = aes_algo;
	cmdbuf[7] = key_type;
	cmdbuf[8] = (size << 0x8) | 0x4;
	cmdbuf[9] = (u32)in;
	cmdbuf[10] = (size << 0x8) | 0x14;
	cmdbuf[11] = (u32)out;

	if((ret = svcSendSyncRequest(psHandle))!=0)return ret;

	_iv[0] = cmdbuf[2];
	_iv[1] = cmdbuf[3];
	_iv[2] = cmdbuf[4];
	_iv[3] = cmdbuf[5];

	return (Result)cmdbuf[1];
}

Result PS_EncryptSignDecryptVerifyAesCcm(u8* in, u32 in_size, u8* out, u32 out_size, u32 data_len, u32 mac_data_len, u32 mac_len, u32 aes_algo, u32 key_type, u8* nonce)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	u32 *_nonce = (u32*)nonce;

	cmdbuf[0] = 0x00050284;
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
	cmdbuf[8] = (in_size << 0x8) | 0x4;
	cmdbuf[9] = (u32)in;
	cmdbuf[10] = (out_size << 0x8) | 0x14;
	cmdbuf[11] = (u32)out;

	if((ret = svcSendSyncRequest(psHandle))!=0)return ret;

	return (Result)cmdbuf[1];
}

Result PS_GetLocalFriendCodeSeed(u64* seed)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000A0000;

	if((ret = svcSendSyncRequest(psHandle))!=0)return ret;

	*seed = (u64)cmdbuf[2] | (u64)cmdbuf[3] << 32;

	return (Result)cmdbuf[1];
}

Result PS_GetDeviceId(u32* device_id)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000B0000;

	if((ret = svcSendSyncRequest(psHandle))!=0)return ret;

	*device_id = cmdbuf[2];

	return (Result)cmdbuf[1];
}
