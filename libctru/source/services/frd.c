#include <string.h>

#include "frd.h"

static Handle frdHandle;
static int frdRefCount;

Result frdInit(void)
{
	Result ret = 0;

	if (AtomicPostIncrement(&frdRefCount)) return 0;

	ret = srvGetServiceHandle(&frdHandle, "frd:u");
	if (R_FAILED(ret)) ret = srvGetServiceHandle(&frdHandle, "frd:n");
	if (R_FAILED(ret)) ret = srvGetServiceHandle(&frdHandle, "frd:a");
	if (R_FAILED(ret)) AtomicDecrement(&frdRefCount);

	return ret;
}

void frdExit(void)
{
	if (AtomicDecrement(&frdRefCount)) return;
	svcCloseHandle(frdHandle);
}

Result FRDU_HasLoggedIn(bool *state)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x01,0,0); // 0x10000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*state = cmdbuf[2] & 0xFF;
	
	return (Result)cmdbuf[1];
}

Result FRDU_IsOnline(bool *state)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x02,0,0); // 0x20000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*state = cmdbuf[2] & 0xFF;
	
	return (Result)cmdbuf[1];
}

Result FRD_Logout(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x04,0,0); // 0x40000
    
	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetMyFriendKey(FriendKey *key)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x05,0,0); // 0x50000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	memcpy(key, &cmdbuf[2], sizeof(FriendKey));
	
	return (Result)cmdbuf[1];
}

Result FRD_GetMyPreference(bool *isPublicMode, bool *isShowGameName, bool *isShowPlayedGame)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x06,0,0); // 0x60000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*isPublicMode = cmdbuf[2] & 0xFF; // Public mode 
	*isShowGameName = cmdbuf[3] & 0xFF; // Show current game 
	*isShowPlayedGame = cmdbuf[4] & 0xFF; // Show game history.
	
	return (Result)cmdbuf[1];
}

Result FRD_GetMyProfile(Profile *profile)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x07, 0, 0); // 0x70000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	memcpy(profile, &cmdbuf[2], sizeof(Profile));
	
	return (Result)cmdbuf[1];
}

Result FRD_GetMyScreenName(u16 *name)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x09,0,0); // 0x90000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;
	
	memcpy(name, &cmdbuf[2], FRIENDS_SCREEN_NAME_SIZE); // 11-byte UTF-16 screen name (with null terminator)
	
	return (Result)cmdbuf[1];
}

Result FRD_GetMyMii(MiiData *mii)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0A,0,0); // 0xA0000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	memcpy(mii, &cmdbuf[2], FRIEND_MII_STORE_DATA_SIZE);
	
	return (Result)cmdbuf[1];
}

Result FRD_GetMyPlayingGame(u64 *titleId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0C,0,0); // 0xC0000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*titleId = (((u64)cmdbuf[3]) << 32 | (u64)cmdbuf[2]);
	
	return (Result)cmdbuf[1];
}

Result FRD_GetMyFavoriteGame(u64 *titleId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0D,0,0); // 0xD0000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*titleId = (((u64)cmdbuf[3]) << 32 | (u64)cmdbuf[2]);
	
	return (Result)cmdbuf[1];
}

Result FRD_GetMyComment(u16 *comment)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x0F,0,0); // 0xF0000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;
	
	memcpy(comment, &cmdbuf[2], FRIENDS_COMMENT_SIZE); // 16-byte UTF-16 comment
	
	return (Result)cmdbuf[1];
}

Result FRD_GetFriendKeyList(FriendKey *friendKeyList, size_t *num, size_t offset, size_t size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x11,2,0); // 0x110080
	cmdbuf[1] = 0;
	cmdbuf[2] = size;
	cmdbuf[64] = (size << 18) | 2;
	cmdbuf[65] = (u32)friendKeyList;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*num = cmdbuf[2];
	
	return (Result)cmdbuf[1];
}

Result FRD_GetFriendMii(MiiData *miiDataList, const FriendKey *friendKeyList, size_t size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x14,1,4); // 0x140044
	cmdbuf[1] = 0;
	cmdbuf[2] = (size << 18) | 2;
	cmdbuf[3] = (u32)friendKeyList;
	cmdbuf[4] = IPC_Desc_Buffer((size *sizeof(MiiData)), IPC_BUFFER_W);
	cmdbuf[5] = (u32)miiDataList;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_IsFromFriendList(FriendKey *friendKeyList, bool *isFromList)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1B,2,0); // 0x1B0080
	cmdbuf[1] = (u32)(friendKeyList->localFriendCode & 0xFFFFFFFF);
	cmdbuf[2] = (u32)(friendKeyList->localFriendCode >> 32);

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*isFromList = cmdbuf[2] & 0xFF;
	
	return (Result)cmdbuf[1];
}

Result FRD_UpdateGameModeDescription(u16 *desc)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1D,0,2); // 0x1D0002
	cmdbuf[1] = 0x400802;
	cmdbuf[2] = (uintptr_t)desc;
    
	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_PrincipalIdToFriendCode(u32 principalId, u64 *friendCode)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x24,1,0); // 0x240040
	cmdbuf[1] = principalId;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*friendCode = (((u64)cmdbuf[3]) << 32 | (u64)cmdbuf[2]);
	
	return (Result)cmdbuf[1];
}

Result FRD_FriendCodeToPrincipalId(u64 friendCode, u32 *principalId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x25,2,0); // 0x250080
	cmdbuf[1] = (u32)(friendCode & 0xFFFFFFFF);
	cmdbuf[2] = (u32)(friendCode >> 32);

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*principalId = cmdbuf[2];
	
	return (Result)cmdbuf[1];
}

Result FRD_IsValidFriendCode(u64 friendCode, bool *isValid)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x26,2,0); // 0x260080
	cmdbuf[1] = (u32)(friendCode & 0xFFFFFFFF);
	cmdbuf[2] = (u32)(friendCode >> 32);

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*isValid = cmdbuf[2] & 0xFF;
	
	return (Result)cmdbuf[1];
}

Result FRD_SetClientSdkVersion(u32 sdkVer)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x32,1,2); // 0x320042
	cmdbuf[1] = sdkVer;
	cmdbuf[2] = IPC_Desc_CurProcessHandle();

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}