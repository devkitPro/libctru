#include <3ds/synchronization.h>
#include <3ds/services/act.h>
#include <3ds/result.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/ipc.h>
#include <3ds/srv.h>
#include <string.h>

static Handle actHandle;
static int actRefCount;

#define MIN(x,y) ((x) > (y) ? (y) : (x))

Result actInit(bool forceUser)
{
	Result ret = 0;

	if (AtomicPostIncrement(&actRefCount)) return 0;

	if (forceUser)
		ret = srvGetServiceHandle(&actHandle, "act:u");
	else
		ret = srvGetServiceHandle(&actHandle, "act:a");

	if (R_FAILED(ret))
		AtomicDecrement(&actHandle);

	return ret;
}

void actExit()
{
	if (AtomicDecrement(&actRefCount)) return;
	svcCloseHandle(actHandle);
}

Handle *actGetSessionHandle(void)
{
	return &actHandle;
}

Result ACT_Initialize(u32 sdkVersion, u32 sharedMemSize, Handle sharedMem)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,2,4); // 0x10084
	cmdbuf[1] = sdkVersion;
	cmdbuf[2] = sharedMemSize;
	cmdbuf[3] = IPC_Desc_CurProcessId();
	// cmdbuf[4] = process id
	cmdbuf[5] = IPC_Desc_SharedHandles(1);
	cmdbuf[6] = sharedMem;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_ResultToErrorCode(Result code)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,1,0); // 0x20040
	cmdbuf[1] = (u32)code;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_GetLastResponseResult()
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,0,0); // 0x30000

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_Cancel()
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,0,0); // 0x40000

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_GetCommonInfo(void *output, u32 outputSize, u32 dataType)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,2,2); // 0x50042
	cmdbuf[1] = outputSize;
	cmdbuf[2] = dataType;
	cmdbuf[3] = IPC_Desc_Buffer(outputSize, IPC_BUFFER_W);
	cmdbuf[4] = (u32)output;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_GetAccountInfo(void *output, u32 outputSize, u8 accountSlot, u32 infoType)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,3,2); // 0x600C2
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = outputSize;
	cmdbuf[3] = infoType;
	cmdbuf[4] = IPC_Desc_Buffer(outputSize, IPC_BUFFER_W);
	cmdbuf[5] = (u32)output;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_GetAsyncResult(u32 *outReadSize, void *output, u32 outputSize, u32 requestType)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x7,2,2); // 0x70082
	cmdbuf[1] = outputSize;
	cmdbuf[2] = requestType;
	cmdbuf[3] = IPC_Desc_Buffer(outputSize, IPC_BUFFER_W);
	cmdbuf[4] = (u32)output;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	*outReadSize = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACT_GetMiiImage(u32 *outSize, void *output, u32 outputSize, u8 accountSlot, u8 miiImageType)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x8,3,2); // 0x800C2
	cmdbuf[1] = outputSize;
	cmdbuf[2] = (u32)miiImageType;
	cmdbuf[3] = (u32)accountSlot;
	cmdbuf[4] = IPC_Desc_Buffer(outputSize, IPC_BUFFER_W);
	cmdbuf[5] = (u32)output;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	*outSize = cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACT_SetNfsPassword(u8 accountSlot, char *password)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,6,0); // 0x90180
	cmdbuf[1] = (u32)accountSlot;
	memset(&cmdbuf[2], 0, 0x14);
	memcpy(&cmdbuf[2], password, MIN(strnlen(password, 16), 16));

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_SetIsApplicationUpdateRequired(bool required)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,1,0); // 0xA0040
	cmdbuf[1] = (u32)required;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_AcquireEulaList(u8 countryCode, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,1,2); // 0xB0042
	cmdbuf[1] = (u32)countryCode;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_AcquireTimezoneList(u8 countryCode, u8 languageCode, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC,2,2); // 0xC0082
	cmdbuf[1] = (u32)countryCode;
	cmdbuf[2] = (u32)languageCode;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_GenerateUuid(ActUuid *uuid, u32 uniqueId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xD,1,0); // 0xD0040
	cmdbuf[1] = uniqueId;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	memcpy(uuid, &cmdbuf[2], sizeof(ActUuid));

	return (Result)cmdbuf[1];
}

Result ACT_GetUuid(ActUuid *uuid, u8 accountSlot, u32 uniqueId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xE,2,0); // 0xE0080
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = uniqueId;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	memcpy(uuid, &cmdbuf[2], sizeof(ActUuid));

	return (Result)cmdbuf[1];
}

Result ACT_FindSlotNoByUuid(u8 *accountSlot, ActUuid *uuid, u32 uniqueId)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xF,5,0); // 0xF0140
	cmdbuf[1] = uniqueId;
	memcpy(&cmdbuf[2], uuid, sizeof(ActUuid));

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	*accountSlot = (u8)cmdbuf[2];

	return (Result)cmdbuf[1];
}

Result ACT_Save()
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x10,0,0); // 0x100000

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_GetTransferableId(u64 *transferableId, u8 accountSlot, u8 saltValue)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x11,2,0); // 0x110080
	cmdbuf[1] = (u32)saltValue;
	cmdbuf[2] = (u32)accountSlot;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	*transferableId = *((u64 *)&cmdbuf[2]);

	return (Result)cmdbuf[1];
}

Result ACT_AcquireNexServiceToken(u8 accountSlot, u32 serverId, bool doParentralControlsCheck, u32 callerProcessId, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x12,4,2); // 0x120102
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = serverId;
	cmdbuf[3] = (u32)doParentralControlsCheck;
	cmdbuf[4] = callerProcessId;
	cmdbuf[5] = IPC_Desc_SharedHandles(1);
	cmdbuf[6] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_GetNexServiceToken(NexServiceToken *token)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x13,0,2); // 0x130002
	cmdbuf[1] = IPC_Desc_Buffer(sizeof(NexServiceToken), IPC_BUFFER_W);
	cmdbuf[2] = (u32)token;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_AcquireIndependentServiceToken(u8 accountSlot, char *clientId, u32 cacheDuration, bool doParentalControlsCheck, bool shared, u32 callerProcessId, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x14,14,2); // 0x140382
	cmdbuf[1] = (u32)accountSlot;
	memset(&cmdbuf[2], 0, 0x24);
	memcpy(&cmdbuf[2], clientId, MIN(strnlen(clientId, 32), 32));
	cmdbuf[11] = cacheDuration;
	cmdbuf[12] = (u32)doParentalControlsCheck;
	cmdbuf[13] = (u32)shared;
	cmdbuf[14] = callerProcessId;
	cmdbuf[15] = IPC_Desc_SharedHandles(1);
	cmdbuf[16] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_GetIndependentServiceToken(IndependentServiceTokenV1 *token)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x15,0,2); // 0x150002
	cmdbuf[1] = IPC_Desc_Buffer(sizeof(IndependentServiceTokenV1), IPC_BUFFER_W);
	cmdbuf[2] = (u32)token;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_AcquireAccountInfo(u8 accountSlot, u32 infoType, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x16,2,2); // 0x160082
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = infoType;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_AcquireAccountIdByPrincipalId(u32 *principalIds, u32 principalIdsSize, u8 unk, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x17,2,4); // 0x170084
	cmdbuf[1] = principalIdsSize;
	cmdbuf[2] = (u32)unk;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = completionEvent;
	cmdbuf[5] = IPC_Desc_StaticBuffer(principalIdsSize, 0);
	cmdbuf[6] = (u32)principalIds;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_AcquirePrincipalIdByAccountId(AccountId *accountIds, u32 accountIdsSize, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x18,1,4); // 0x180044
	cmdbuf[1] = accountIdsSize;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;
	cmdbuf[4] = IPC_Desc_StaticBuffer(accountIdsSize, 0);
	cmdbuf[5] = (u32)accountIds;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_AcquireMii(u32 *persistentIds, u32 persistentIdsSize, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x19,1,4); // 0x190044
	cmdbuf[1] = persistentIdsSize;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;
	cmdbuf[4] = IPC_Desc_StaticBuffer(persistentIdsSize, 0);
	cmdbuf[5] = (u32)persistentIds;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_AcquireAccountInfoRaw(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1A,1,2); // 0x1A0042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_GetCachedIndependentServiceToken(IndependentServiceTokenV1 *token, u8 accountSlot, char *clientId, u32 cacheDuration, bool doParentralControlsCheck, bool shared)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1C,13,2); // 0x1C0342
	cmdbuf[1] = (u32)accountSlot;
	memset(&cmdbuf[2], 0, 0x24);
	memcpy(&cmdbuf[2], clientId, MIN(strnlen(clientId, 32), 32));
	cmdbuf[11] = cacheDuration;
	cmdbuf[12] = (u32)doParentralControlsCheck;
	cmdbuf[13] = (u32)shared;
	cmdbuf[14] = IPC_Desc_Buffer(sizeof(IndependentServiceTokenV1), IPC_BUFFER_W);
	cmdbuf[15] = (u32)token;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_InquireMailAddressAvailability(AccountMailAddress *mailAddress, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1D,0,4); // 0x1D0004
	cmdbuf[1] = IPC_Desc_SharedHandles(1);
	cmdbuf[2] = completionEvent;
	cmdbuf[3] = IPC_Desc_Buffer(sizeof(AccountMailAddress), IPC_BUFFER_R);
	cmdbuf[4] = (u32)mailAddress;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_AcquireEula(u8 countryCode, char *languageCode, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1E,2,2); // 0x1E0082
	cmdbuf[1] = (u32)countryCode;
	cmdbuf[2] = 0;
	memcpy(&cmdbuf[2], languageCode, MIN(strnlen(languageCode, 2), 2));
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_AcquireEulaLanguageList(u8 countryCode, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1F,1,2); // 0x1F0042
	cmdbuf[1] = (u32)countryCode;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_AcquireIndependentServiceTokenV2(u8 accountSlot, char *clientId, u32 cacheDuration, bool doParentalControlsCheck, bool shared, u32 callerProcessId, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x20,14,2); // 0x200382
	cmdbuf[1] = (u32)accountSlot;
	memset(&cmdbuf[2], 0, 0x24);
	memcpy(&cmdbuf[2], clientId, MIN(strnlen(clientId, 32), 32));
	cmdbuf[11] = cacheDuration;
	cmdbuf[12] = (u32)doParentalControlsCheck;
	cmdbuf[13] = (u32)shared;
	cmdbuf[14] = callerProcessId;
	cmdbuf[15] = IPC_Desc_SharedHandles(1);
	cmdbuf[16] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_GetIndependentServiceTokenV2(IndependentServiceTokenV2 *token)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x21,0,2); // 0x210002
	cmdbuf[1] = IPC_Desc_Buffer(sizeof(IndependentServiceTokenV2), IPC_BUFFER_W);
	cmdbuf[2] = (u32)token;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACT_GetCachedIndependentServiceTokenV2(IndependentServiceTokenV2 *token, u8 accountSlot, char *clientId, u32 cacheDuration, bool doParentralControlsCheck, bool shared)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x22,13,2); // 0x220342
	cmdbuf[1] = (u32)accountSlot;
	memset(&cmdbuf[2], 0, 0x24);
	memcpy(&cmdbuf[2], clientId, MIN(strnlen(clientId, 32), 32));
	cmdbuf[11] = cacheDuration;
	cmdbuf[12] = (u32)doParentralControlsCheck;
	cmdbuf[13] = (u32)shared;
	cmdbuf[14] = IPC_Desc_Buffer(sizeof(IndependentServiceTokenV2), IPC_BUFFER_W);
	cmdbuf[15] = (u32)token;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SwapAccounts(u8 accountSlot1, u8 accountSlot2)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x401,2,0); // 0x4010080
	cmdbuf[1] = (u32)accountSlot1;
	cmdbuf[2] = (u32)accountSlot2;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_CreateConsoleAccount()
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x402,0,0); // 0x4020000

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_CommitConsoleAccount(u8 accountSlot)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x403,1,0); // 0x4030040
	cmdbuf[1] = (u32)accountSlot;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_UnbindServerAccount(u8 accountSlot, bool completely)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x404,2,0); // 0x4040080
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = (u32)completely;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_DeleteConsoleAccount(u8 accountSlot)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x405,1,0); // 0x4050040
	cmdbuf[1] = (u32)accountSlot;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_LoadConsoleAccount(u8 accountSlot, bool doPasswordCheck, AccountPassword *password, bool useNullPassword, bool dryRun)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x406,9,0); // 0x4060240
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = (u32)doPasswordCheck;
	memset(&cmdbuf[3], 0, 0x14);
	memcpy(&cmdbuf[3], password, sizeof(AccountPassword));
	cmdbuf[8] = (u32)useNullPassword;
	cmdbuf[9] = (u32)dryRun;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_UnloadConsoleAccount()
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x407,0,0); // 0x4070000

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_EnableAccountPasswordCache(u8 accountSlot, bool enabled)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x408,2,0); // 0x4080080
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = (u32)enabled;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SetDefaultAccount(u8 accountSlot)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x409,1,0); // 0x4090040
	cmdbuf[1] = (u32)accountSlot;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_ReplaceAccountId(u8 accountSlot)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40A,1,0); // 0x40A0040
	cmdbuf[1] = (u32)accountSlot;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_GetSupportContext(SupportContext *supportContext, u8 accountSlot)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40B,1,0); // 0x40B0040
	cmdbuf[1] = (u32)accountSlot;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	memcpy(supportContext, &cmdbuf[2], sizeof(SupportContext));

	return (Result)cmdbuf[1];
}

Result ACTA_SetHostServerSettings(u8 accountSlot, u8 nnasType, u8 nfsType, u8 nfsNo)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40C,4,0); // 0x40C0100
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = (u32)nnasType;
	cmdbuf[3] = (u32)nfsType;
	cmdbuf[4] = (u32)nfsNo;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SetDefaultHostServerSettings(u8 nnasType, u8 nfsType, u8 nfsNo)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40D,3,0); // 0x40C00C0
	cmdbuf[1] = (u32)nnasType;
	cmdbuf[2] = (u32)nfsType;
	cmdbuf[3] = (u32)nfsNo;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SetHostServerSettingsStr(u8 accountSlot, ActNnasSubdomain *nnasSubdomain, NfsTypeStr *nfsTypeStr)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40E,11,0); // 0x40E02C0
	memset(&cmdbuf[1], 0, 0x24);
	memcpy(&cmdbuf[1], nnasSubdomain, sizeof(ActNnasSubdomain));
	cmdbuf[10] = 0;
	memcpy(&cmdbuf[10], nfsTypeStr, sizeof(NfsTypeStr));
	cmdbuf[11] = (u32)accountSlot;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SetDefaultHostServerSettingsStr(ActNnasSubdomain *nnasSubdomain, NfsTypeStr *nfsTypeStr)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x40F,10,0); // 0x40F0280
	memset(&cmdbuf[1], 0, 0x24);
	memcpy(&cmdbuf[1], nnasSubdomain, sizeof(ActNnasSubdomain));
	cmdbuf[10] = 0;
	memcpy(&cmdbuf[10], nfsTypeStr, sizeof(NfsTypeStr));

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SetPersistentIdHead(u32 head)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x410,1,0); // 0x4100040

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SetTransferableIdCounter(u16 counter)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x411,1,0); // 0x4110040
	cmdbuf[1] = (u32)counter;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_UpdateMiiData(u8 accountSlot, CFLStoreData *miiData, MiiScreenName *screenName)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x412,31,0); // 0x41207C0
	cmdbuf[1] = (u32)accountSlot;
	memcpy(&cmdbuf[2], miiData, sizeof(CFLStoreData));
	memset(&cmdbuf[26], 0, 0x18);
	memcpy(&cmdbuf[26], screenName, sizeof(MiiScreenName));

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_UpdateMiiImage(u8 accountSlot, u8 miiImageType, void *image, u32 imageSize)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x413,3,2); // 0x41300C2
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = (u32)miiImageType;
	cmdbuf[3] = imageSize;
	cmdbuf[4] = IPC_Desc_Buffer(imageSize, IPC_BUFFER_R);
	cmdbuf[5] = (u32)image;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_InquireAccountIdAvailability(u8 accountSlot, AccountId *accountId, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x414,6,2); // 0x4140182
	cmdbuf[1] = (u32)accountSlot;
	memset(&cmdbuf[2], 0, 0x14);
	memcpy(&cmdbuf[2], accountId, sizeof(AccountId));
	cmdbuf[7] = IPC_Desc_SharedHandles(1);
	cmdbuf[8] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_BindToNewServerAccount(u8 accountSlot, AccountId *accountId, AccountMailAddress *mailAddress, AccountPassword *password, bool isParentEmail, bool marketingFlag, bool offDeviceFlag, s64 birthDateTimestatmp, u8 gender, u32 region, AccountTimezone *timezone, EulaInfo *eulaInfo, s64 parentalConsentTimestamp, u32 parentalConsentId, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x415,59,4); // 0x4150EC4
	cmdbuf[1] = (u32)accountSlot;
	memset(&cmdbuf[2], 0, 0x14 * 2);
	memcpy(&cmdbuf[2], accountId, sizeof(AccountId));
	memcpy(&cmdbuf[7], password, sizeof(AccountPassword));
	cmdbuf[12] = (u32)isParentEmail;
	cmdbuf[13] = (u32)marketingFlag;
	*((u64 *)&cmdbuf[14]) = birthDateTimestatmp;
	cmdbuf[16] = (u32)gender;
	cmdbuf[17] = region;
	memcpy(&cmdbuf[18], timezone, sizeof(AccountTimezone));
	memcpy(&cmdbuf[54], eulaInfo, sizeof(EulaInfo));
	*((u64 *)&cmdbuf[56]) = parentalConsentTimestamp;
	cmdbuf[58] = parentalConsentId;
	cmdbuf[59] = (u32)offDeviceFlag;
	cmdbuf[60] = IPC_Desc_SharedHandles(1);
	cmdbuf[61] = completionEvent;
	cmdbuf[62] = IPC_Desc_Buffer(sizeof(AccountMailAddress), IPC_BUFFER_R);
	cmdbuf[63] = (u32)mailAddress;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_BindToExistentServerAccount(u8 accountSlot, AccountId *accountId, AccountMailAddress *mailAddress, AccountPassword *password, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x416,11,4); // 0x41602C4
	cmdbuf[1] = (u32)accountSlot;
	memset(&cmdbuf[2], 0, 0x14 * 2);
	memcpy(&cmdbuf[2], accountId, sizeof(AccountId));
	memcpy(&cmdbuf[7], password, sizeof(AccountPassword));
	cmdbuf[12] = IPC_Desc_SharedHandles(1);
	cmdbuf[13] = completionEvent;
	cmdbuf[14] = IPC_Desc_Buffer(sizeof(AccountMailAddress), IPC_BUFFER_R);
	cmdbuf[15] = (u32)mailAddress;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_InquireBindingToExistentServerAccount(u8 accountSlot, AccountId *accountId, AccountMailAddress *mailAddress, AccountPassword *password, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x417,11,4); // 0x41702C4
	cmdbuf[1] = (u32)accountSlot;
	memset(&cmdbuf[2], 0, 0x14 * 2);
	memcpy(&cmdbuf[2], accountId, sizeof(AccountId));
	memcpy(&cmdbuf[7], password, sizeof(AccountPassword));
	cmdbuf[12] = IPC_Desc_SharedHandles(1);
	cmdbuf[13] = completionEvent;
	cmdbuf[14] = IPC_Desc_Buffer(sizeof(AccountMailAddress), IPC_BUFFER_R);
	cmdbuf[15] = (u32)mailAddress;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_DeleteServerAccount(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x418,1,2); // 0x4180042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_AcquireAccountTokenEx(u8 accountSlot, AccountPassword *password, bool useNullPassword, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x41A,7,2); // 0x41A01C2
	cmdbuf[1] = (u32)accountSlot;
	memset(&cmdbuf[2], 0, 0x14);
	memcpy(&cmdbuf[2], password, sizeof(AccountPassword));
	cmdbuf[7] = (u32)useNullPassword;
	cmdbuf[8] = IPC_Desc_SharedHandles(1);
	cmdbuf[9] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_AgreeEula(u8 accountSlot, EulaInfo *eulaInfo, s64 agreementTimestamp, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x41B,5,2); // 0x41B0142
	cmdbuf[1] = (u32)accountSlot;
	memcpy(&cmdbuf[2], eulaInfo, sizeof(EulaInfo));
	*((u64 *)&cmdbuf[4]) = agreementTimestamp;
	cmdbuf[6] = IPC_Desc_SharedHandles(1);
	cmdbuf[7] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SyncAccountInfo(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x41C,1,2); // 0x41C0042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_InvalidateAccountToken(u8 accountSlot, u32 invalidationActionMask)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x41D,2,0); // 0x41D0080
	cmdbuf[1] = invalidationActionMask;
	cmdbuf[2] = (u32)accountSlot;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_UpdateAccountPassword(u8 accountSlot, AccountPassword *newPassword, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x41E,6,2); // 0x41E0182
	cmdbuf[1] = (u32)accountSlot;
	memset(&cmdbuf[2], 0, 0x14);
	memcpy(&cmdbuf[2], newPassword, sizeof(AccountPassword));
	cmdbuf[7] = IPC_Desc_SharedHandles(1);
	cmdbuf[8] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_ReissueAccountPassword(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x41F,1,2); // 0x41F0042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SetAccountPasswordInput(u8 accountSlot, AccountPassword *passwordInput)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x420,6,0); // 0x4200180
	cmdbuf[1] = (u32)accountSlot;
	memset(&cmdbuf[2], 0, 0x14);
	memcpy(&cmdbuf[2], passwordInput, sizeof(AccountPassword));

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_UploadMii(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x421,1,2); // 0x4210042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_InactivateDeviceAssociation(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x422,1,2); // 0x4220042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_ValidateMailAddress(u8 accountSlot, u32 confirmationCode, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x423,2,2); // 0x4230082
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = confirmationCode;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SendPostingApprovalMail(u8 accountSlot, AccountMailAddress *parentalEmail, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x424,1,4); // 0x4240044
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;
	cmdbuf[4] = IPC_Desc_Buffer(sizeof(AccountMailAddress), IPC_BUFFER_R);
	cmdbuf[5] = (u32)parentalEmail;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SendConfirmationMail(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x425,1,2); // 0x4250042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SendConfirmationMailForPin(u8 accountSlot, AccountMailAddress *parentalEmail, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x426,1,4); // 0x4260044
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;
	cmdbuf[4] = IPC_Desc_Buffer(sizeof(AccountMailAddress), IPC_BUFFER_R);
	cmdbuf[5] = (u32)parentalEmail;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SendMasterKeyMailForPin(u8 accountSlot, u32 masterKey, AccountMailAddress *parentalEmail, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x427,2,4); // 0x4270084
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = masterKey;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = completionEvent;
	cmdbuf[5] = IPC_Desc_Buffer(sizeof(AccountMailAddress), IPC_BUFFER_R);
	cmdbuf[6] = (u32)parentalEmail;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_ApproveByCreditCard(u8 accountSlot, CreditCardInfo *cardInfo, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x428,1,4);
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;
	cmdbuf[4] = IPC_Desc_StaticBuffer(sizeof(CreditCardInfo), 0);
	cmdbuf[5] = (u32)cardInfo;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SendCoppaCodeMail(u8 accountSlot, u32 principalId, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x429,2,2); // 0x4290082
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = principalId;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SetIsMiiUpdated(u8 accountSlot, bool isDirty)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x42A,2,0); // 0x42A0080
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = (u32)isDirty;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_ReserveTransfer(u8 accountSlot, DeviceInfo *newDevice, char *operatorData, u32 operatorSize, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x42B,7,4); // 0x42B01C4
	cmdbuf[1] = (u32)accountSlot;
	memcpy(&cmdbuf[2], newDevice, sizeof(DeviceInfo));
	cmdbuf[7] = operatorSize;
	cmdbuf[8] = IPC_Desc_SharedHandles(1);
	cmdbuf[9] = completionEvent;
	cmdbuf[10] = IPC_Desc_Buffer(operatorSize, IPC_BUFFER_R);
	cmdbuf[11] = (u32)operatorData;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_CompleteTransfer(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x42C,1,2); // 0x42C0042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_InactivateAccountDeviceAssociation(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x42D,1,2); // 0x42D0042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_SetNetworkTime(s64 timestamp)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x42E,2,0); // 0x42E0080
	*((u64 *)&cmdbuf[1]) = timestamp;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_UpdateAccountInfo(u8 accountSlot, char *xmlData, u32 xmlDataSize, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x42F,2,4); // 0x42F0084
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = xmlDataSize;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = completionEvent;
	cmdbuf[5] = IPC_Desc_Buffer(xmlDataSize, IPC_BUFFER_R);
	cmdbuf[6] = (u32)xmlData;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_UpdateAccountMailAddress(u8 accountSlot, AccountMailAddress *newEmail, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x430,1,4); // 0x4300044
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;
	cmdbuf[4] = IPC_Desc_Buffer(sizeof(AccountMailAddress), IPC_BUFFER_R);
	cmdbuf[5] = (u32)newEmail;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_DeleteDeviceAssociation(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x431,1,2); // 0x4310042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_DeleteAccountDeviceAssociation(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x432,1,2); // 0x4320042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_CancelTransfer(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x433,1,2); // 0x4330042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_ReloadAndBlockSaveData(Handle unloadFinishedEvent, Handle remountAndBlockEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x434,0,3); // 0x4340003
	cmdbuf[1] = IPC_Desc_SharedHandles(2);
	cmdbuf[2] = unloadFinishedEvent;
	cmdbuf[3] = remountAndBlockEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}

Result ACTA_ReserveServerAccountDeletion(u8 accountSlot, Handle completionEvent)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x435,1,2); // 0x4350042
	cmdbuf[1] = (u32)accountSlot;
	cmdbuf[2] = IPC_Desc_SharedHandles(1);
	cmdbuf[3] = completionEvent;

	if (R_FAILED(ret = svcSendSyncRequest(actHandle))) return ret;

	return (Result)cmdbuf[1];
}
