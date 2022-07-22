#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/cfgu.h>
#include <3ds/services/ndm.h>
#include <3ds/ipc.h>

typedef struct {
   u32 uniqueId;
   u32 revision;
   u8 macAddr[6];
} dlpTitleInfo;

Result dlpClntInit(void);

void dlpClntExit(void);

bool dlpClntWaitForEvent(bool nextEvent, bool wait);

u64 dlpCreateChildTid(u32 uniqueId, u32 revision);

Result DLPCLNT_Initialize(size_t sharedMemSize, u8 maxScanTitles, size_t unk, Handle sharedmemHandle, Handle eventHandle);

Result DLPCLNT_Finalize(void);

//DLPCLNT_GetEventDesc();

Result DLPCLNT_GetChannel(u16* channel);

Result DLPCLNT_StartScan(u16 channel, u8* macAddr);

Result DLPCLNT_StopScan(void);
/*
DLPCLNT_GetServerInfo();

DLPCLNT_GetTitleInfo();
*/
Result DLPCLNT_GetTitleInfoInOrder(void* buf, size_t size, size_t* actual_size);
/*
DLPCLNT_DeleteScanInfo();
*/
Result DLPCLNT_PrepareForSystemDownload(u8* macAddr, u32 uniqueId, u32 revision);
/*
DLPCLNT_StartSystemDownload();
*/
Result DLPCLNT_StartTitleDownload(u8* macAddr, u32 uniqueId, u32 revision);

Result DLPCLNT_GetMyStatus(u32* status);
/*
DLPCLNT_GetConnectingNodes();

DLPCLNT_GetNodeInfo();
*/
Result DLPCLNT_GetWirelessRebootPassphrase(void* buf);

Result DLPCLNT_StopSession(void);
/*
DLPCLNT_GetCupVersion();

DLPCLNT_GetDupAvailability();*/