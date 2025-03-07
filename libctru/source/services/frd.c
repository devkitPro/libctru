#include <3ds/synchronization.h>
#include <3ds/services/frd.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>
#include <string.h>

static Handle frdHandle;
static int frdRefCount;

#define MIN(x,y) ((x) > (y) ? (y) : (x))

Result frdInit(bool forceUser)
{
	Result ret = 0;

	if (AtomicPostIncrement(&frdRefCount)) return 0;

	if (forceUser)
		ret = srvGetServiceHandle(&frdHandle, "frd:u");
	else
		ret = srvGetServiceHandle(&frdHandle, "frd:a");

	if (R_FAILED(ret))
		AtomicDecrement(&frdRefCount);

	return ret;
}

void frdExit(void)
{
	if (AtomicDecrement(&frdRefCount)) return;
	svcCloseHandle(frdHandle);
}

Handle *frdGetSessionHandle(void)
{
	return &frdHandle;
}

Result FRD_HasLoggedIn(bool *state)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,0,0); // 0x10000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*state = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result FRD_IsOnline(bool *state)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*state = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result FRD_Login(Handle event)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,0,2); // 0x30002
	cmdbuf[1] = IPC_Desc_SharedHandles(1);
	cmdbuf[2] = event;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_Logout(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,0,0); // 0x40000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetMyFriendKey(FriendKey *key)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,0,0); // 0x50000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	memcpy(key, &cmdbuf[2], sizeof(FriendKey));

	return (Result)cmdbuf[1];
}

Result FRD_GetMyPreference(bool *isPublicMode, bool *isShowGameName, bool *isShowPlayedGame)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,0,0); // 0x60000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*isPublicMode = (u8)cmdbuf[2]; // Public mode
	*isShowGameName = (u8)cmdbuf[3]; // Show current game
	*isShowPlayedGame = (u8)cmdbuf[4]; // Show game history.

	return (Result)cmdbuf[1];
}

Result FRD_GetMyProfile(Profile *profile)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7, 0, 0); // 0x70000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	memcpy(profile, &cmdbuf[2], sizeof(Profile));

	return (Result)cmdbuf[1];
}

Result FRD_GetMyPresence(MyPresence *presence)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x8, 0, 0); // 0x80000

	staticbufs[0] = IPC_Desc_StaticBuffer(sizeof(MyPresence), 0);
	staticbufs[1] = (u32)presence;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetMyScreenName(MiiScreenName *name)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,0,0); // 0x90000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	memcpy(name, &cmdbuf[2], sizeof(MiiScreenName));

	return (Result)cmdbuf[1];
}

Result FRD_GetMyMii(FriendMii *mii)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,0,0); // 0xA0000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	memcpy(mii, &cmdbuf[2], sizeof(FriendMii));

	return (Result)cmdbuf[1];
}

Result FRD_GetMyLocalAccountId(u8 *localAccountId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,0,0); // 0xB0000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*localAccountId = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result FRD_GetMyPlayingGame(GameKey *playingGame)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC,0,0); // 0xC0000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	memcpy(playingGame, &cmdbuf[2], sizeof(GameKey));

	return (Result)cmdbuf[1];
}

Result FRD_GetMyFavoriteGame(GameKey *favoriteGame)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD,0,0); // 0xD0000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	memcpy(favoriteGame, &cmdbuf[2], sizeof(GameKey));

	return (Result)cmdbuf[1];
}

Result FRD_GetMyNcPrincipalId(u32 *ncPrincipalId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xE,0,0); // 0xE0000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*ncPrincipalId = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result FRD_GetMyComment(FriendComment *comment)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xF,0,0); // 0xF0000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	memcpy(comment, &cmdbuf[2], sizeof(FriendComment));

	return (Result)cmdbuf[1];
}

Result FRD_GetMyPassword(char *password, u32 bufsize)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x10,1,0); // 0x100040
	cmdbuf[1] = bufsize;

	staticbufs[0] = IPC_Desc_StaticBuffer(bufsize, 0);
	staticbufs[1] = (u32)password;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetFriendKeyList(FriendKey *friendKeyList, u32 *num, u32 offset, u32 size)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x11,2,0); // 0x110080
	cmdbuf[1] = offset;
	cmdbuf[2] = size;

	staticbufs[0] = IPC_Desc_StaticBuffer(size, 0);
	staticbufs[1] = (u32)friendKeyList;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	*num = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result FRD_GetFriendPresence(FriendPresence *friendPresences, const FriendKey *friendKeyList, u32 count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x12,1,2); // 0x120042
	cmdbuf[1] = count;
	// input
	cmdbuf[2] = IPC_Desc_StaticBuffer(count * sizeof(FriendKey), 0);
	cmdbuf[3] = (u32)friendKeyList;

	// output
	staticbufs[0] = IPC_Desc_StaticBuffer(count * sizeof(FriendPresence), 0);
	staticbufs[1] = (u32)friendPresences;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetMiiScreenName(MiiScreenName *screenNames, u32 screenNamesLen, u8 *characterSets, u32 characterSetsLen, const FriendKey *friendKeyList, u32 count, bool maskNonAscii, bool profanityFlag)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x13,5,2); // 0x130142
	cmdbuf[1] = screenNamesLen;
	cmdbuf[2] = characterSetsLen;
	cmdbuf[3] = count;
	cmdbuf[4] = (u32)maskNonAscii;
	cmdbuf[5] = (u32)profanityFlag;
	// input
	cmdbuf[6] = IPC_Desc_StaticBuffer(count * sizeof(FriendKey), 0);
	cmdbuf[7] = (u32)friendKeyList;

	// output
	staticbufs[0] = IPC_Desc_StaticBuffer(count * sizeof(MiiScreenName), 0);
	staticbufs[1] = (u32)screenNames;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetFriendMii(FriendMii *miiList, const FriendKey *friendKeyList, u32 count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x14,1,4); // 0x140044
	cmdbuf[1] = count;
	cmdbuf[2] = IPC_Desc_StaticBuffer(count * sizeof(FriendKey), 0);
	cmdbuf[3] = (u32)friendKeyList;
	cmdbuf[4] = IPC_Desc_Buffer(count * sizeof(FriendMii), IPC_BUFFER_W);
	cmdbuf[5] = (u32)miiList;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetFriendProfile(Profile *profiles, const FriendKey *friendKeyList, u32 count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x15,1,2); // 0x150042
	cmdbuf[1] = count;
	// input
	cmdbuf[2] = IPC_Desc_StaticBuffer(count * sizeof(FriendKey), 0);
	cmdbuf[3] = (u32)friendKeyList;

	// output
	staticbufs[0] = IPC_Desc_StaticBuffer(count * sizeof(Profile), 0);
	staticbufs[1] = (u32)profiles;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetFriendRelationship(u8 *relationships, const FriendKey *friendKeyList, u32 count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x16,1,2); // 0x160042
	cmdbuf[1] = count;
	// input
	cmdbuf[2] = IPC_Desc_StaticBuffer(count * sizeof(FriendKey), 0);
	cmdbuf[3] = (u32)friendKeyList;

	// output
	staticbufs[0] = IPC_Desc_StaticBuffer(count * sizeof(u8), 0);
	staticbufs[1] = (u32)relationships;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetFriendAttributeFlags(u32 *attributes, const FriendKey *friendKeyList, u32 count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x17,1,2); // 0x170042
	cmdbuf[1] = count;
	// input
	cmdbuf[2] = IPC_Desc_StaticBuffer(count * sizeof(FriendKey), 0);
	cmdbuf[3] = (u32)friendKeyList;

	// output
	staticbufs[0] = IPC_Desc_StaticBuffer(count * sizeof(u32), 0);
	staticbufs[1] = (u32)attributes;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetFriendPlayingGame(FriendPlayingGame *playingGames, const FriendKey *friendKeyList, u32 count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x18,1,4); // 0x180044
	cmdbuf[1] = count;
	cmdbuf[2] = IPC_Desc_StaticBuffer(count * sizeof(FriendKey), 0);
	cmdbuf[3] = (u32)friendKeyList;
	cmdbuf[4] = IPC_Desc_Buffer(count * sizeof(FriendPlayingGame), IPC_BUFFER_W);
	cmdbuf[5] = (u32)playingGames;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetFriendFavoriteGame(GameKey *favoriteGames, const FriendKey *friendKeyList, u32 count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x19,1,2); // 0x190042
	cmdbuf[1] = count;
	// input
	cmdbuf[2] = IPC_Desc_StaticBuffer(count * sizeof(FriendKey), 0);
	cmdbuf[3] = (u32)friendKeyList;

	// output
	staticbufs[0] = IPC_Desc_StaticBuffer(count * sizeof(GameKey), 0);
	staticbufs[1] = (u32)favoriteGames;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetFriendInfo(FriendInfo *infos, const FriendKey *friendKeyList, u32 count, bool maskNonAscii, bool profanityFlag)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1A,3,4); // 0x1A00C4
	cmdbuf[1] = count;
	cmdbuf[2] = (u32)maskNonAscii;
	cmdbuf[3] = (u32)profanityFlag;
	cmdbuf[4] = IPC_Desc_StaticBuffer(count * sizeof(FriendKey), 0);
	cmdbuf[5] = (u32)friendKeyList;
	cmdbuf[6] = IPC_Desc_Buffer(count * sizeof(FriendInfo), IPC_BUFFER_W);
	cmdbuf[7] = (u32)infos;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_IsInFriendList(u64 friendCode, bool *isFromList)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1B,2,0); // 0x1B0080
	*((u64 *)&cmdbuf[1]) = friendCode;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*isFromList = (bool)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result FRD_UnscrambleLocalFriendCode(u64 *unscrambled, ScrambledFriendCode *scrambled, u32 count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x1C,1,2); // 0x1C0042
	cmdbuf[1] = count;
	// input
	cmdbuf[2] = IPC_Desc_StaticBuffer(count * sizeof(ScrambledFriendCode), 1);
	cmdbuf[3] = (u32)scrambled;

	// output
	staticbufs[0] = IPC_Desc_StaticBuffer(count * sizeof(u64), 0);
	staticbufs[1] = (u32)unscrambled;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_UpdateGameModeDescription(FriendGameModeDescription *desc)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1D,0,2); // 0x1D0002
	cmdbuf[1] = IPC_Desc_StaticBuffer(sizeof(FriendGameModeDescription), 2);
	cmdbuf[2] = (u32)desc;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_UpdateMyPresence(Presence *presence, FriendGameModeDescription *desc)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1E,11,2); // 0x1E02C2
	memcpy(&cmdbuf[1], presence, sizeof(Presence));
	cmdbuf[12] = IPC_Desc_StaticBuffer(sizeof(FriendGameModeDescription), 2);
	cmdbuf[13] = (u32)desc;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_SendInvitation(const FriendKey *friendKeyList, u32 count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1F,1,2); // 0x1F0042
	cmdbuf[1] = IPC_Desc_StaticBuffer(count * sizeof(FriendKey), 0);
	cmdbuf[2] = (u32)friendKeyList;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_AttachToEventNotification(Handle event)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x20,0,2); // 0x200002
	cmdbuf[1] = IPC_Desc_SharedHandles(1);
	cmdbuf[2] = (u32)event;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_SetNotificationMask(FriendNotificationMask mask)
{
	Result ret = 0;

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x21,1,0); // 0x210040
	cmdbuf[1] = (u32)mask;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetEventNotification(NotificationEvent *event, u32 count, u32 *recievedNotifCount)
{
	Result ret = 0;

	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x22,1,0); // 0x220040
	cmdbuf[1] = count;

	staticbufs[0] = IPC_Desc_StaticBuffer(count * sizeof(NotificationEvent), 0);
	staticbufs[1] = (u32)event;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	*recievedNotifCount = cmdbuf[3];

	return (Result)cmdbuf[1];
}

Result FRD_GetLastResponseResult()
{
	Result ret = 0;

	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x23,0,0);

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

	*friendCode = *((u64 *)&cmdbuf[2]);

	return (Result)cmdbuf[1];
}

Result FRD_FriendCodeToPrincipalId(u64 friendCode, u32 *principalId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x25,2,0); // 0x250080
	*((u64 *)&cmdbuf[2]) = friendCode;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*principalId = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result FRD_IsValidFriendCode(u64 friendCode, bool *isValid)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x26,2,0); // 0x260080
	*((u64 *)&cmdbuf[1]) = friendCode;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*isValid = cmdbuf[2] & 0xFF;

	return (Result)cmdbuf[1];
}

Result FRD_ResultToErrorCode(u32 *errorCode, Result res)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x27,1,0); // 0x270040
	cmdbuf[1] = (u32)res;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*errorCode = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result FRD_RequestGameAuthentication(u32 serverId, u16 *ingamesn, u32 ingamesnSize, u8 majorSdkVersion, u8 minorSdkVersion, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x28,9,4); // 0x280244
	cmdbuf[1] = serverId;
	memset(&cmdbuf[2], 0, NASC_INGAMESN_LEN * 2);
	memcpy(&cmdbuf[2], ingamesn, MIN(ingamesnSize, NASC_INGAMESN_LEN * 2));
	cmdbuf[8] = (u32)majorSdkVersion;
	cmdbuf[9] = (u32)minorSdkVersion;
	cmdbuf[10] = IPC_Desc_CurProcessId();
	// cmdbuf[11] = process id
	cmdbuf[12] = IPC_Desc_SharedHandles(1);
	cmdbuf[13] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetGameAuthenticationData(GameAuthenticationData *data)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x29,0,0); // 0x290000

	staticbufs[0] = IPC_Desc_StaticBuffer(sizeof(GameAuthenticationData), 0);
	staticbufs[1] = (u32)data;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_RequestServiceLocator(u32 serverId, char *keyhash, char *svc, u8 majorSdkVersion, u8 minorSdkVersion, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2A,8,4); // 0x2A0204
	cmdbuf[1] = serverId;
	memset(&cmdbuf[2], 0, NASC_KEYHASH_LEN);
	memcpy(&cmdbuf[2], keyhash, MIN(strnlen(keyhash, NASC_KEYHASH_LEN), NASC_KEYHASH_LEN));
	memset(&cmdbuf[5], 0, NASC_SVC_LEN);
	memcpy(&cmdbuf[5], svc, MIN(strnlen(svc, NASC_SVC_LEN), NASC_SVC_LEN));
	cmdbuf[7] = (u32)majorSdkVersion;
	cmdbuf[8] = (u32)minorSdkVersion;
	cmdbuf[9] = IPC_Desc_CurProcessId();
	// cmdbuf[10] = process id
	cmdbuf[11] = IPC_Desc_SharedHandles(1);
	cmdbuf[12] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetServiceLocatorData(ServiceLocatorData *data)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x2B,0,0); // 0x2B0000

	staticbufs[0] = IPC_Desc_StaticBuffer(sizeof(ServiceLocatorData), 0);
	staticbufs[1] = (u32)data;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_DetectNatProperties(Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2C,0,2); // 0x2C0002
	cmdbuf[1] = IPC_Desc_SharedHandles(1);
	cmdbuf[2] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetNatProperties(u32 *natMappingType, u32 *natFilteringType)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2D,0,0); // 0x2D0000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*natMappingType = cmdbuf[2];
	*natFilteringType = cmdbuf[3];

	return (Result)cmdbuf[1];
}

Result FRD_GetServerTimeDifference(u64 *diff)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2E,0,0); // 0x2E0000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*diff = *((u64 *)&cmdbuf[2]);

	return (Result)cmdbuf[1];
}

Result FRD_AllowHalfAwake(bool allow)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2F,1,0); // 0x2F0040
	cmdbuf[1] = (u32)allow;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetServerTypes(u8 *nascEnvironment, u8 *nfsType, u8 *nfsNo)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x30,0,0); // 0x300000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*nascEnvironment = (u8)cmdbuf[2];
	*nfsType = (u8)cmdbuf[3];
	*nfsNo = (u8)cmdbuf[4];

	return (Result)cmdbuf[1];
}

Result FRD_GetFriendComment(FriendComment *comments, u32 commentsLen, const FriendKey *friendKeyList, u32 count)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x31,2,2); // 0x310082
	cmdbuf[1] = commentsLen;
	cmdbuf[2] = count;
	// input
	cmdbuf[3] = IPC_Desc_StaticBuffer(count * sizeof(FriendKey), 0);
	cmdbuf[4] = (u32)friendKeyList;

	// output
	staticbufs[0] = IPC_Desc_StaticBuffer(count * sizeof(FriendComment), 0);
	staticbufs[1] = (u32)comments;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_SetClientSdkVersion(u32 sdkVer)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x32,1,2); // 0x320042
	cmdbuf[1] = sdkVer;
	cmdbuf[2] = IPC_Desc_CurProcessId();
	// cmdbuf[3] = current process id

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetMyApproachContext(EncryptedApproachContext *ctx)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x33,0,0); // 0x330000

	staticbufs[0] = IPC_Desc_StaticBuffer(sizeof(EncryptedApproachContext), 0);
	staticbufs[1] = (u32)ctx;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_AddFriendWithApproach(u8 *unkbuf, u32 unkbufSize, EncryptedApproachContext *ctx, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x34,1,6); // 0x340046
	cmdbuf[1] = unkbufSize;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;
	cmdbuf[4] = IPC_Desc_StaticBuffer(sizeof(EncryptedApproachContext), 3);
	cmdbuf[5] = (u32)ctx;
	cmdbuf[6] = IPC_Desc_StaticBuffer(MIN(unkbufSize, 0x600), 4);
	cmdbuf[7] = (u32)unkbuf;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_DecryptApproachContext(DecryptedApproachContext *decryptedContext, EncryptedApproachContext *encryptedContext, bool maskNonAscii, u8 characterSet)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 *staticbufs = getThreadStaticBuffers();

	u32 save0 = staticbufs[0];
	u32 save1 = staticbufs[1];

	cmdbuf[0] = IPC_MakeHeader(0x35,2,2); // 0x350082
	cmdbuf[1] = (u32)maskNonAscii;
	cmdbuf[2] = (u32)characterSet;
	// input
	cmdbuf[3] = IPC_Desc_StaticBuffer(sizeof(EncryptedApproachContext), 3);
	cmdbuf[4] = (u32)encryptedContext;

	staticbufs[0] = IPC_Desc_StaticBuffer(sizeof(DecryptedApproachContext), 0);
	staticbufs[1] = (u32)decryptedContext;

	ret = svcSendSyncRequest(frdHandle);

	staticbufs[0] = save0;
	staticbufs[1] = save1;

	if (R_FAILED(ret)) return ret;

	return (Result)cmdbuf[1];
}

Result FRD_GetExtendedNatProperties(u32 *natMappingType, u32 *natFilteringType, u32 *natMappingPortIncrement)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x36,0,0); // 0x360000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	*natMappingType = cmdbuf[2];
	*natFilteringType = cmdbuf[3];
	*natMappingPortIncrement = cmdbuf[4];

	return (Result)cmdbuf[1];
}

Result FRDA_CreateLocalAccount(u8 localAccountId, u8 nascEnvironment, u8 nfsType, u8 nfsNo)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x401,4,0); // 0x4010100
	cmdbuf[1] = (u32)localAccountId;
	cmdbuf[2] = (u32)nascEnvironment;
	cmdbuf[3] = (u32)nfsType;
	cmdbuf[4] = (u32)nfsNo;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_DeleteLocalAccount(u8 localAccountId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x402,1,0); // 0x4020040
	cmdbuf[1] = (u32)localAccountId;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_LoadLocalAccount(u8 localAccountId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x403,1,0); // 0x4030040
	cmdbuf[1] = (u32)localAccountId;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_UnloadLocalAccount()
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x404,0,0); // 0x4040000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_Save()
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x405,0,0); // 0x4050000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_AddFriendOnline(Handle event, u32 principalId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x406,1,2); // 0x4060042
	cmdbuf[1] = principalId;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = event;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_AddFriendOffline(FriendKey *friendKey, FriendMii *mii, FriendProfile *friendProfile, MiiScreenName *screenName, bool profanityFlag, u8 characterSet)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x407,54,0); // 0x4070D80
	memcpy(&cmdbuf[1], friendKey, sizeof(FriendKey));
	memcpy(&cmdbuf[5], mii, sizeof(FriendMii));
	memcpy(&cmdbuf[29], friendProfile, sizeof(FriendProfile));
	memcpy(&cmdbuf[47], screenName, sizeof(MiiScreenName));
	cmdbuf[53] = (u32)profanityFlag;
	cmdbuf[54] = (u32)characterSet;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_UpdateMiiScreenName(FriendKey *friendKey, MiiScreenName *screenName, u8 characterSet)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x408,11,0); // 0x40802C0
	memcpy(&cmdbuf[1], friendKey, sizeof(FriendKey));
	memcpy(&cmdbuf[5], screenName, sizeof(MiiScreenName));
	cmdbuf[11] = (u32)characterSet;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_RemoveFriend(u32 principalId, u64 localFriendCode)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x409,4,0); // 0x4090100
	cmdbuf[1] = principalId;
	*((u64 *)&cmdbuf[2]) = localFriendCode;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return cmdbuf[1];
}

Result FRDA_UpdatePlayingGame(GameKey *playingGame)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x40A,4,0); // 0x40A0100
	memcpy(&cmdbuf[1], playingGame, sizeof(GameKey));

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_UpdatePreference(bool isPublicMode, bool isShowGameMode, bool isShowPlayedMode)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x40B,3,0); // 0x40B00C0
	cmdbuf[1] = (u32)isPublicMode;
	cmdbuf[2] = (u32)isShowGameMode;
	cmdbuf[3] = (u32)isShowPlayedMode;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_UpdateMii(FriendMii *mii, MiiScreenName *screenName, bool profanityFlag, u8 characterSet)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x40C,32,0); // 0x40C0800
	memcpy(&cmdbuf[1], mii, sizeof(FriendMii));
	memcpy(&cmdbuf[25], screenName, sizeof(MiiScreenName));
	cmdbuf[31] = (u32)profanityFlag;
	cmdbuf[32] = (u32)characterSet;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_UpdateFavoriteGame(GameKey *favoriteGame)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x40D,4,0); // 0x40D0100
	memcpy(&cmdbuf[1], favoriteGame, sizeof(GameKey));

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_SetNcPrincipalId(u32 ncPrincipalId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x40E,1,0); // 0x40E0040
	cmdbuf[1] = ncPrincipalId;

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_UpdateComment(FriendComment *comment)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x40F,9,0); // 0x40F0240
	memcpy(&cmdbuf[1], comment, sizeof(FriendComment));

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result FRDA_IncrementMoveCount()
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x410,0,0); // 0x4100000

	if (R_FAILED(ret = svcSendSyncRequest(frdHandle))) return ret;

	return (Result)cmdbuf[1];
}

#undef MIN