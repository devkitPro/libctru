#include <stdlib.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/cfgu.h>

static Handle CFGU_handle = 0;

Result initCfgu()
{
	return srvGetServiceHandle(&CFGU_handle, "cfg:u");
}

Result exitCfgu()
{
	Result ret = svcCloseHandle(CFGU_handle);
	CFGU_handle = 0;

	return ret;
}

Result CFGU_SecureInfoGetRegion(u8* region)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00020000;

	if((ret = svcSendSyncRequest(CFGU_handle))!=0)return ret;

	*region = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result CFGU_GenHashConsoleUnique(u32 appIDSalt, u64* hash)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00030040;
	cmdbuf[1] = appIDSalt;

	if((ret = svcSendSyncRequest(CFGU_handle))!=0)return ret;

	*hash = (u64)cmdbuf[2];
	*hash |= ((u64)cmdbuf[3])<<32;

	return (Result)cmdbuf[1];
}

Result CFGU_GetRegionCanadaUSA(u8* value)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00040000;

	if((ret = svcSendSyncRequest(CFGU_handle))!=0)return ret;

	*value = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result CFGU_GetSystemModel(u8* model)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00050000;

	if((ret = svcSendSyncRequest(CFGU_handle))!=0)return ret;

	*model = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result CFGU_GetModelNintendo2DS(u8* value)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00060000;

	if((ret = svcSendSyncRequest(CFGU_handle))!=0)return ret;

	*value = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result CFGU_GetCountryCodeString(u16 code, u16* string)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00090040;
	cmdbuf[1] = (u32)code;

	if((ret = svcSendSyncRequest(CFGU_handle))!=0)return ret;

	*string = (u16)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result CFGU_GetCountryCodeID(u16 string, u16* code)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x000A0040;
	cmdbuf[1] = (u32)string;

	if((ret = svcSendSyncRequest(CFGU_handle))!=0)return ret;

	*code = (u16)cmdbuf[2];

	return (Result)cmdbuf[1];
}

// See here for block IDs:
// http://3dbrew.org/wiki/Config_Savegame#Configuration_blocks
Result CFGU_GetConfigInfoBlk2(u32 size, u32 blkID, u8* outData)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00010082;
	cmdbuf[1] = size;
	cmdbuf[2] = blkID;
	cmdbuf[3] = (size<<4)|12;
	cmdbuf[4] = (u32)outData;

	if((ret = svcSendSyncRequest(CFGU_handle))!=0)return ret;

	return (Result)cmdbuf[1];
}

Result CFGU_GetSystemLanguage(u8* language)
{
	return CFGU_GetConfigInfoBlk2(1, 0xA0002, language);
}
