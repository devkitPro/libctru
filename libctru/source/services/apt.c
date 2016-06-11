/*
  apt.c _ Applet/NS shell interaction
*/

#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/apt.h>
#include <3ds/services/gspgpu.h>
#include <3ds/ipc.h>
#include <3ds/env.h>
#include <3ds/thread.h>

#define APT_HANDLER_STACKSIZE (0x1000)

NS_APPID currentAppId;

Handle aptLockHandle;
Handle aptEvents[3];

Thread aptEventHandlerThread;

LightLock aptStatusMutex = 1;
Handle aptStatusEvent;
APT_AppStatus aptStatus = APP_NOTINITIALIZED;
APT_AppStatus aptStatusBeforeSleep = APP_NOTINITIALIZED;
u32 aptStatusPower;
Handle aptSleepSync;
static bool aptSleepAllowed = true;

u32 aptParameters[0x1000/4]; //TEMP

static u32 __ns_capinfo[0x20>>2];

static NS_APPID __apt_launchapplet_appID;
static Handle __apt_launchapplet_inhandle;
static u32 *__apt_launchapplet_parambuf;
static u32 __apt_launchapplet_parambufsize;

static aptHookCookie aptFirstHook;

static void aptCallHook(APT_HookType hookType)
{
	aptHookCookie* c;
	for (c = &aptFirstHook; c && c->callback; c = c->next)
		c->callback(hookType, c->param);
}

// The following function can be overriden in order to log APT signals and notifications for debugging purposes
__attribute__((weak)) void _aptDebug(int a, int b)
{
}

void __ctru_speedup_config(void);

static void aptAppStarted(void);

static bool aptIsReinit(void)
{
	return (envGetSystemRunFlags() & RUNFLAG_APTREINIT) != 0;
}

static bool aptIsCrippled(void)
{
	return (envGetSystemRunFlags() & RUNFLAG_APTWORKAROUND) != 0 && !aptIsReinit();
}

static Result aptGetServiceHandle(Handle* aptuHandle)
{
	static const char* serviceName;
	static const char* const serviceNameTable[3] = {"APT:S", "APT:A", "APT:U"};

	if (serviceName)
		return srvGetServiceHandleDirect(aptuHandle, serviceName);

	Result ret;
	int i;
	for (i = 0; i < 3; i ++)
	{
		ret = srvGetServiceHandleDirect(aptuHandle, serviceNameTable[i]);
		if (R_SUCCEEDED(ret))
		{
			serviceName = serviceNameTable[i];
			break;
		}
	}

	return ret;
}

static inline int countPrmWords(u32 hdr)
{
	return 1 + (hdr&0x3F) + ((hdr>>6)&0x3F);
}

Result aptSendCommand(u32* aptcmdbuf)
{
	Handle aptuHandle;

	if (aptLockHandle) svcWaitSynchronization(aptLockHandle, U64_MAX);
	Result res = aptGetServiceHandle(&aptuHandle);
	if (R_SUCCEEDED(res))
	{
		u32* cmdbuf = getThreadCommandBuffer();
		memcpy(cmdbuf, aptcmdbuf, 4*countPrmWords(aptcmdbuf[0]));
		res = svcSendSyncRequest(aptuHandle);
		if (R_SUCCEEDED(res))
		{
			memcpy(aptcmdbuf, cmdbuf, 4*16);//4*countPrmWords(cmdbuf[0])); // Workaround for Citra failing to emulate response cmdheaders
			res = aptcmdbuf[1];
		}
		svcCloseHandle(aptuHandle);
	}
	if (aptLockHandle) svcReleaseMutex(aptLockHandle);
	return res;
}

void aptInitCaptureInfo(u32 *ns_capinfo)
{
	u32 tmp=0;
	u32 main_pixsz, sub_pixsz;
	GSPGPU_CaptureInfo gspcapinfo;

	memset(&gspcapinfo, 0, sizeof(GSPGPU_CaptureInfo));

	// Get display-capture info from GSP.
	GSPGPU_ImportDisplayCaptureInfo(&gspcapinfo);

	// Fill in display-capture info for NS.
	if(gspcapinfo.screencapture[0].framebuf0_vaddr != gspcapinfo.screencapture[0].framebuf1_vaddr)ns_capinfo[1] = 1;
	
	ns_capinfo[4] = gspcapinfo.screencapture[0].format & 0x7;
	ns_capinfo[7] = gspcapinfo.screencapture[1].format & 0x7;

	main_pixsz = (ns_capinfo[4] < 2) ? 3 : 2;
	sub_pixsz  = (ns_capinfo[7] < 2) ? 3 : 2;

	ns_capinfo[2] = sub_pixsz * 0x14000;
	ns_capinfo[3] = ns_capinfo[2];

	if(ns_capinfo[1])ns_capinfo[3] = main_pixsz * 0x19000 + ns_capinfo[2];

	tmp = main_pixsz * 0x19000 + ns_capinfo[3];
	ns_capinfo[0] = main_pixsz * 0x7000 + tmp;
}

void aptWaitStatusEvent(void)
{
	svcWaitSynchronization(aptStatusEvent, U64_MAX);
	svcClearEvent(aptStatusEvent);
}

void aptAppletUtility_Exit_RetToApp(u32 type)
{
	APT_UnlockTransition(0x10);
	APT_SleepIfShellClosed();
	APT_UnlockTransition(0x01);
	APT_SleepIfShellClosed();
	APT_SleepIfShellClosed();
	if (type)
		APT_SleepIfShellClosed();
}

NS_APPID aptGetMenuAppID(void)
{
	NS_APPID menu_appid = 0;
	APT_GetAppletManInfo(APTPOS_NONE, NULL, NULL, &menu_appid, NULL);
	return menu_appid;
}

void aptReturnToMenu(void)
{
	NS_APPID menu_appid;

	if(aptIsCrippled())
	{
		svcClearEvent(aptStatusEvent);
		aptSetStatus(APP_EXITING);
		return;
	}

	// This is only executed when ret-to-menu was triggered via the home-button, not the power-button.
	if(aptGetStatusPower() == 0)
	{
		bool temp;
		APT_TryLockTransition(0x01, &temp);
	}

	// Set status to SUSPENDED.
	__apt_launchapplet_appID = 0;
	svcClearEvent(aptStatusEvent);
	aptSetStatus(APP_SUSPENDED);

	// Prepare for return to menu
	APT_PrepareToJumpToHomeMenu();

	// Save Vram
	GSPGPU_SaveVramSysArea();

	// Capture screen.
	memset(__ns_capinfo, 0, 0x20);

	aptInitCaptureInfo(__ns_capinfo);

	menu_appid = aptGetMenuAppID();

	// Send capture-screen info to menu.
	APT_SendParameter(currentAppId, menu_appid, APTCMD_SYSAPPLET_REQUEST, __ns_capinfo, sizeof(__ns_capinfo), 0);
	APT_SendCaptureBufferInfo((aptCaptureBufInfo*)__ns_capinfo);

	// Release GSP module.
	GSPGPU_ReleaseRight();

	// Jump to menu!
	APT_JumpToHomeMenu(NULL, 0, 0);

	// Wait for return to application.
	APT_NotifyToWait(currentAppId);

	// This is only executed when ret-to-menu was triggered via the home-button, not the power-button.
	if(aptGetStatusPower() == 0)
		APT_SleepIfShellClosed();

	aptWaitStatusEvent();
}

void aptAppletStarted(void)
{
	// Set status to SUSPENDED.
	svcClearEvent(aptStatusEvent);
	aptSetStatus(APP_SUSPENDED);

	APT_SendCaptureBufferInfo((aptCaptureBufInfo*)__ns_capinfo);
	APT_ReplySleepQuery(currentAppId, APTREPLY_REJECT);
	APT_StartLibraryApplet(__apt_launchapplet_appID, __apt_launchapplet_parambuf, __apt_launchapplet_parambufsize, __apt_launchapplet_inhandle);
	APT_SleepIfShellClosed();
	APT_NotifyToWait(currentAppId);
	APT_SleepIfShellClosed();
}

void aptAppletClosed(void)
{
	aptAppletUtility_Exit_RetToApp(1);

	GSPGPU_AcquireRight(0x0);
	GSPGPU_RestoreVramSysArea();

	svcClearEvent(aptStatusEvent);
	aptSetStatus(APP_RUNNING);
	svcClearEvent(aptStatusEvent);
}

static void __handle_notification(void) {
	APT_Signal type;
	Result ret=0;

	// Get notification type.
	ret = APT_InquireNotification(currentAppId, &type);
	if(R_FAILED(ret)) return;
	_aptDebug(1, type);

	switch(type)
	{
	case APTSIGNAL_HOMEBUTTON:
	case APTSIGNAL_POWERBUTTON:
		// The main thread should call aptReturnToMenu() when the status gets set to this.
		if(aptGetStatus() == APP_RUNNING)
		{
			APT_ReplySleepQuery(currentAppId, APTREPLY_REJECT);

			if(type == APTSIGNAL_HOMEBUTTON)  aptSetStatusPower(0);
			if(type == APTSIGNAL_POWERBUTTON) aptSetStatusPower(1);
			aptSetStatus(APP_SUSPENDING);
		}
		break;

	case APTSIGNAL_SLEEP_QUERY:
		// Reply to sleep-request.
		if(aptIsSleepAllowed())
		{
			aptStatusBeforeSleep = aptGetStatus();
			aptSetStatus(APP_PREPARE_SLEEPMODE);
			svcWaitSynchronization(aptSleepSync, U64_MAX);
			svcClearEvent(aptSleepSync);
			APT_ReplySleepQuery(currentAppId, APTREPLY_ACCEPT);
		} else
			APT_ReplySleepQuery(currentAppId, APTREPLY_REJECT);
		break;

	case APTSIGNAL_SLEEP_ENTER:
		if(aptGetStatus() == APP_PREPARE_SLEEPMODE)
		{
			// Report into sleep-mode.
			aptSetStatus(APP_SLEEPMODE);
			APT_ReplySleepNotificationComplete(currentAppId);
		}
		break;

	// Leaving sleep-mode.
	case APTSIGNAL_SLEEP_WAKEUP:
		if(aptGetStatus() == APP_SLEEPMODE)
		{
			if(aptStatusBeforeSleep == APP_RUNNING)GSPGPU_SetLcdForceBlack(0);

			// Restore old aptStatus.
			aptSetStatus(aptStatusBeforeSleep);
		}
		break;

	default:
		break;
	}
}

static bool __handle_incoming_parameter(void) {
	APT_Command cmd;
	APT_ReceiveParameter(currentAppId, aptParameters, sizeof(aptParameters), &cmd, NULL, NULL);
	_aptDebug(2, cmd);

	switch(cmd)
	{
	case APTCMD_WAKEUP: // Application just started.
		aptAppStarted();
		return true;

	case APTCMD_RESPONSE: // "Launched library applet finished loading"
		if (aptGetStatus() != APP_SUSPENDED || __apt_launchapplet_appID==0) return true;
		aptSetStatus(APP_APPLETSTARTED);
		return true;
	case APTCMD_WAKEUP_EXIT: // "Launched library applet closed"
		if (aptGetStatus() != APP_SUSPENDED || __apt_launchapplet_appID==0) return true;
		if(__apt_launchapplet_parambuf && __apt_launchapplet_parambufsize)memcpy(__apt_launchapplet_parambuf, aptParameters, __apt_launchapplet_parambufsize);
		aptSetStatus(APP_APPLETCLOSED);
		return true;
	case APTCMD_WAKEUP_PAUSE: // Just returned from menu.
		if (aptGetStatus() != APP_NOTINITIALIZED)
		{
			GSPGPU_AcquireRight(0x0);
			GSPGPU_RestoreVramSysArea();
			aptAppletUtility_Exit_RetToApp(0);
			aptSetStatus(APP_RUNNING);
		} else
			aptAppStarted();
		return true;

	case APTCMD_WAKEUP_CANCEL: // Exiting application.
		aptSetStatus(APP_EXITING);
		return false;
	default:
		break;
	}

	return true;
}

void aptEventHandler(void *arg)
{
	bool runThread = true;

	while(runThread)
	{
		s32 syncedID = 0;
		svcWaitSynchronizationN(&syncedID, aptEvents, 3, 0, U64_MAX);
		svcClearEvent(aptEvents[syncedID]);
		switch(syncedID)
		{
			// Event 0 means we got a signal from NS (home button, power button etc).
			case 0x0:
				__handle_notification();
				break;
			// Event 1 means we got an incoming parameter.
			case 0x1:
				runThread = __handle_incoming_parameter();
				break;
			// Event 2 means we should exit the thread.
			case 0x2:
				runThread = false;
				break;
		}
	}
}

static int aptRefCount = 0;

Result aptInit(void)
{
	Result ret=0;

	if (AtomicPostIncrement(&aptRefCount)) return 0;

	// Initialize APT stuff, escape load screen.
	ret = APT_GetLockHandle(0x0, &aptLockHandle);
	if(R_FAILED(ret)) goto _fail;

	currentAppId = envGetAptAppId();

	svcCreateEvent(&aptStatusEvent, 0);
	svcCreateEvent(&aptSleepSync, 0);
	aptStatus=0;

	if(!aptIsCrippled())
	{
		APT_AppletAttr attr = aptMakeAppletAttr(APTPOS_APP, false, false);
		if(R_FAILED(ret=APT_Initialize(currentAppId, attr, &aptEvents[0], &aptEvents[1])))return ret;
		if(R_FAILED(ret=APT_Enable(attr))) goto _fail;

		// create APT close event
		svcCreateEvent(&aptEvents[2], 0);

		// After a cycle of APT_Finalize+APT_Initialize APT thinks the
		// application is suspended, so we need to tell it to unsuspend us.
		if (aptIsReinit())
		{
			APT_PrepareToJumpToApplication(false);
			APT_JumpToApplication(NULL, 0, 0);
		}

		if(R_FAILED(ret=APT_NotifyToWait(currentAppId)))goto _fail;

		// create APT event handler thread
		aptEventHandlerThread = threadCreate(aptEventHandler, 0x0, APT_HANDLER_STACKSIZE, 0x31, -2, true);

		// Wait for the state to become APT_RUNNING
		aptWaitStatusEvent();
	} else
		aptAppStarted();

	return 0;

_fail:
	AtomicDecrement(&aptRefCount);
	return ret;
}

void aptExit(void)
{
	if (AtomicDecrement(&aptRefCount)) return;

	if(!aptIsCrippled())aptAppletUtility_Exit_RetToApp(0);

	// This is only executed when application-termination was triggered via the home-menu power-off screen.
	if(aptGetStatusPower() == 1)
		APT_ReplySleepQuery(currentAppId, APTREPLY_REJECT);

	if(!aptIsCrippled())
	{
		bool isReinit = aptIsReinit();
		if (aptGetStatus() == APP_EXITING || !isReinit)
		{
			APT_PrepareToCloseApplication(true);
			APT_CloseApplication(NULL, 0, 0);

			if (isReinit)
			{
				extern void (*__system_retAddr)(void);
				__system_retAddr = NULL;
			}
		} else if (isReinit)
			APT_Finalize(currentAppId);
	}

	svcSignalEvent(aptEvents[2]);
	threadJoin(aptEventHandlerThread, U64_MAX);
	svcCloseHandle(aptEvents[2]);

	svcCloseHandle(aptSleepSync);

	svcCloseHandle(aptLockHandle);
	svcCloseHandle(aptStatusEvent);
}

bool aptMainLoop(void)
{
	while(1)
	{
		switch(aptGetStatus())
		{
			case APP_RUNNING:
				return true;
			case APP_EXITING:
				aptCallHook(APTHOOK_ONEXIT);
				return false;
			case APP_SUSPENDING:
				aptCallHook(APTHOOK_ONSUSPEND);
				aptReturnToMenu();
				if (aptGetStatus() == APP_RUNNING)
					aptCallHook(APTHOOK_ONRESTORE);
				break;
			case APP_APPLETSTARTED:
				aptAppletStarted();
				break;
			case APP_APPLETCLOSED:
				aptAppletClosed();
				aptCallHook(APTHOOK_ONRESTORE);
				break;
			case APP_PREPARE_SLEEPMODE:
				aptCallHook(APTHOOK_ONSLEEP);
				aptSignalReadyForSleep();
				// Fall through
			default:
			//case APP_NOTINITIALIZED:
			//case APP_SLEEPMODE:
				aptWaitStatusEvent();
				aptCallHook(APTHOOK_ONWAKEUP);
				break;
		}
	}
}

void aptHook(aptHookCookie* cookie, aptHookFn callback, void* param)
{
	if (!callback) return;

	aptHookCookie* hook = &aptFirstHook;
	*cookie = *hook; // Structure copy.
	hook->next = cookie;
	hook->callback = callback;
	hook->param = param;
}

void aptUnhook(aptHookCookie* cookie)
{
	aptHookCookie* hook;
	for (hook = &aptFirstHook; hook; hook = hook->next)
	{
		if (hook->next == cookie)
		{
			*hook = *cookie; // Structure copy.
			break;
		}
	}
}

void aptAppStarted(void)
{
	aptSetStatus(APP_RUNNING);
	svcClearEvent(aptStatusEvent);

	if(!aptIsCrippled())
	{
		APT_UnlockTransition(0x10);
		APT_SleepIfShellClosed();
		APT_SleepIfShellClosed();
	}
}

APT_AppStatus aptGetStatus(void)
{
	APT_AppStatus ret;
	LightLock_Lock(&aptStatusMutex);
	ret = aptStatus;
	LightLock_Unlock(&aptStatusMutex);
	return ret;
}

void aptSetStatus(APT_AppStatus status)
{
	LightLock_Lock(&aptStatusMutex);

	aptStatus = status;

	//if(prevstatus != APP_NOTINITIALIZED)
	//{
		if(status == APP_RUNNING)
			__ctru_speedup_config();
		if(status == APP_RUNNING || status == APP_EXITING || status == APP_APPLETSTARTED || status == APP_APPLETCLOSED)
			svcSignalEvent(aptStatusEvent);
	//}

	LightLock_Unlock(&aptStatusMutex);
}

u32 aptGetStatusPower(void)
{
	u32 ret;
	LightLock_Lock(&aptStatusMutex);
	ret = aptStatusPower;
	LightLock_Unlock(&aptStatusMutex);
	return ret;
}

void aptSetStatusPower(u32 status)
{
	LightLock_Lock(&aptStatusMutex);
	aptStatusPower = status;
	LightLock_Unlock(&aptStatusMutex);
}

void aptSignalReadyForSleep(void)
{
	svcSignalEvent(aptSleepSync);
}

bool aptIsSleepAllowed(void)
{
	return aptSleepAllowed;
}

void aptSetSleepAllowed(bool allowed)
{
	aptSleepAllowed = allowed;
}

Result APT_GetLockHandle(u16 flags, Handle* lockHandle)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x1,1,0); // 0x10040
	cmdbuf[1]=flags;
	
	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret))
		*lockHandle = cmdbuf[5];

	return ret;
}

Result APT_Initialize(NS_APPID appId, APT_AppletAttr attr, Handle* signalEvent, Handle* resumeEvent)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x2,2,0); // 0x20080
	cmdbuf[1]=appId;
	cmdbuf[2]=attr;
	
	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret))
	{
		if(signalEvent) *signalEvent=cmdbuf[3];
		if(resumeEvent) *resumeEvent=cmdbuf[4];
	}

	return ret;
}

Result APT_Finalize(NS_APPID appId)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x4,1,0); // 0x40040
	cmdbuf[1]=appId;
	
	return aptSendCommand(cmdbuf);
}

Result APT_HardwareResetAsync(void)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x4E,0,0); // 0x4E0000
	
	return aptSendCommand(cmdbuf);
}

Result APT_Enable(APT_AppletAttr attr)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x3,1,0); // 0x30040
	cmdbuf[1]=attr;
	
	return aptSendCommand(cmdbuf);
}

Result APT_GetAppletManInfo(APT_AppletPos inpos, APT_AppletPos* outpos, NS_APPID* req_appid, NS_APPID* menu_appid, NS_APPID* active_appid)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x5,1,0); // 0x50040
	cmdbuf[1]=inpos;
	
	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret))
	{
		if (outpos)       *outpos      =cmdbuf[2];
		if (req_appid)    *req_appid   =cmdbuf[3];
		if (menu_appid)   *menu_appid  =cmdbuf[4];
		if (active_appid) *active_appid=cmdbuf[5];
	}
	
	return ret;
}

Result APT_GetAppletInfo(NS_APPID appID, u64* pProgramID, u8* pMediaType, bool* pRegistered, bool* pLoadState, APT_AppletAttr* pAttributes)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x6,1,0); // 0x60040
	cmdbuf[1]=appID;

	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret))
	{
		if (pProgramID)  *pProgramID =(u64)cmdbuf[2]|((u64)cmdbuf[3]<<32);
		if (pMediaType)  *pMediaType =cmdbuf[4];
		if (pRegistered) *pRegistered=cmdbuf[5];
		if (pLoadState)  *pLoadState =cmdbuf[6];
		if (pAttributes) *pAttributes=cmdbuf[7];
	}

	return ret;
}

Result APT_GetAppletProgramInfo(u32 id, u32 flags, u16 *titleversion)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x4D,2,0); // 0x4D0080
	cmdbuf[1]=id;
	cmdbuf[2]=flags;

	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret))
		*titleversion=cmdbuf[2];

	return ret;
}

Result APT_GetProgramID(u64* pProgramID)
{
	u32 cmdbuf[16];
	cmdbuf[0] = IPC_MakeHeader(0x58,0,2); // 0x580002
	cmdbuf[1] = IPC_Desc_CurProcessHandle();
	
	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret))
		*pProgramID=((u64)cmdbuf[3]<<32)|cmdbuf[2];

	return ret;
}

Result APT_IsRegistered(NS_APPID appID, bool* out)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x9,1,0); // 0x90040
	cmdbuf[1]=appID;
	
	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret))
		*out=cmdbuf[2];

	return ret;
}

Result APT_InquireNotification(u32 appID, APT_Signal* signalType)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0xB,1,0); // 0xB0040
	cmdbuf[1]=appID;
	
	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret))
		*signalType=cmdbuf[2];

	return ret;
}

Result APT_PrepareToJumpToHomeMenu(void)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x2B,0,0); // 0x2B0000
	
	return aptSendCommand(cmdbuf);
}

Result APT_JumpToHomeMenu(const void* param, size_t paramSize, Handle handle)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x2C,1,4); // 0x2C0044
	cmdbuf[1]=paramSize;
	cmdbuf[2]=IPC_Desc_SharedHandles(1);
	cmdbuf[3]=handle;
	cmdbuf[4]=IPC_Desc_StaticBuffer(cmdbuf[1],0);
	cmdbuf[5]=(u32) param;

	return aptSendCommand(cmdbuf);
}

Result APT_PrepareToJumpToApplication(bool exiting)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x23,1,0); // 0x230040
	cmdbuf[1]=exiting;

	return aptSendCommand(cmdbuf);
}

Result APT_JumpToApplication(const void* param, size_t paramSize, Handle handle)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x24,1,4); // 0x240044
	cmdbuf[1]=paramSize;
	cmdbuf[2]=IPC_Desc_SharedHandles(1);
	cmdbuf[3]=handle;
	cmdbuf[4]=IPC_Desc_StaticBuffer(cmdbuf[1],0);
	cmdbuf[5]= (u32) param;
	
	return aptSendCommand(cmdbuf);
}

Result APT_NotifyToWait(NS_APPID appID)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x43,1,0); // 0x430040
	cmdbuf[1]=appID;
	
	return aptSendCommand(cmdbuf);
}

Result APT_AppletUtility(int id, void* out, size_t outSize, const void* in, size_t inSize)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x4B,3,2); // 0x4B00C2
	cmdbuf[1]=id;
	cmdbuf[2]=inSize;
	cmdbuf[3]=outSize;
	cmdbuf[4]=IPC_Desc_StaticBuffer(cmdbuf[2],1);
	cmdbuf[5]=(u32)in;

	u32 saved_threadstorage[2];
	u32* staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0]=staticbufs[0];
	saved_threadstorage[1]=staticbufs[1];
	staticbufs[0]=IPC_Desc_StaticBuffer(cmdbuf[3],0);
	staticbufs[1]=(u32)out;
	
	Result ret = aptSendCommand(cmdbuf);
	staticbufs[0]=saved_threadstorage[0];
	staticbufs[1]=saved_threadstorage[1];

	return R_SUCCEEDED(ret) ? cmdbuf[2] : ret;
}

Result APT_SleepIfShellClosed(void)
{
	u8 out, in = 0;
	return APT_AppletUtility(4, &out, sizeof(out), &in, sizeof(in));
}

Result APT_TryLockTransition(u32 transition, bool* succeeded)
{
	return APT_AppletUtility(6, &succeeded, sizeof(succeeded), &transition, sizeof(transition));
}

Result APT_UnlockTransition(u32 transition)
{
	u8 out;
	return APT_AppletUtility(7, &out, sizeof(out), &transition, sizeof(transition));
}

Result APT_GlanceParameter(NS_APPID appID, void* buffer, size_t bufferSize, APT_Command* command, size_t* actualSize, Handle* parameter)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0xE,2,0); // 0xE0080
	cmdbuf[1]=appID;
	cmdbuf[2]=bufferSize;

	u32 saved_threadstorage[2];
	u32* staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0]=staticbufs[0];
	saved_threadstorage[1]=staticbufs[1];
	staticbufs[0]=IPC_Desc_StaticBuffer(cmdbuf[2],0);
	staticbufs[1]=(u32)buffer;
	
	Result ret = aptSendCommand(cmdbuf);
	staticbufs[0]=saved_threadstorage[0];
	staticbufs[1]=saved_threadstorage[1];

	if (R_SUCCEEDED(ret))
	{
		if (command)    *command   =cmdbuf[3];
		if (actualSize) *actualSize=cmdbuf[4];
		if (parameter)  *parameter =cmdbuf[6];
	}

	return ret;
}

Result APT_ReceiveParameter(NS_APPID appID, void* buffer, size_t bufferSize, APT_Command* command, size_t* actualSize, Handle* parameter)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0xD,2,0); // 0xD0080
	cmdbuf[1]=appID;
	cmdbuf[2]=bufferSize;

	u32 saved_threadstorage[2];
	u32* staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0]=staticbufs[0];
	saved_threadstorage[1]=staticbufs[1];
	staticbufs[0]=IPC_Desc_StaticBuffer(cmdbuf[2],0);
	staticbufs[1]=(u32)buffer;
	
	Result ret = aptSendCommand(cmdbuf);
	staticbufs[0]=saved_threadstorage[0];
	staticbufs[1]=saved_threadstorage[1];

	if (R_SUCCEEDED(ret))
	{
		if (command)    *command   =cmdbuf[3];
		if (actualSize) *actualSize=cmdbuf[4];
		if (parameter)  *parameter =cmdbuf[6];
	}

	return ret;
}

Result APT_SendParameter(NS_APPID source, NS_APPID dest, APT_Command command, const void* buffer, u32 bufferSize, Handle parameter)
{
	u32 cmdbuf[16];

	cmdbuf[0] = IPC_MakeHeader(0xC,4,4); // 0xC0104
	cmdbuf[1] = source;
	cmdbuf[2] = dest;
	cmdbuf[3] = command;
	cmdbuf[4] = bufferSize;

	cmdbuf[5] = IPC_Desc_SharedHandles(1);
	cmdbuf[6] = parameter;
	
	cmdbuf[7] = IPC_Desc_StaticBuffer(cmdbuf[4],0);
	cmdbuf[8] = (u32)buffer;
	
	return aptSendCommand(cmdbuf);
}

Result APT_SendCaptureBufferInfo(const aptCaptureBufInfo* captureBuf)
{
	u32 cmdbuf[16];

	cmdbuf[0] = IPC_MakeHeader(0x40,1,2); // 0x400042
	cmdbuf[1] = sizeof(*captureBuf);
	cmdbuf[2] = IPC_Desc_StaticBuffer(cmdbuf[1],0);
	cmdbuf[3] = (u32)captureBuf;
	
	return aptSendCommand(cmdbuf);
}

Result APT_ReplySleepQuery(NS_APPID appID, APT_QueryReply reply)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x3E,2,0); // 0x3E0080
	cmdbuf[1]=appID;
	cmdbuf[2]=reply;
	
	return aptSendCommand(cmdbuf);
}

Result APT_ReplySleepNotificationComplete(NS_APPID appID)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x3F,1,0); // 0x3F0040
	cmdbuf[1]=appID;
	
	return aptSendCommand(cmdbuf);
}

Result APT_PrepareToCloseApplication(bool cancelPreload)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x22,1,0); // 0x220040
	cmdbuf[1]=cancelPreload;
	
	return aptSendCommand(cmdbuf);
}

Result APT_CloseApplication(const void* param, size_t paramSize, Handle handle)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x27,1,4); // 0x270044
	cmdbuf[1]=paramSize;
	cmdbuf[2]=IPC_Desc_SharedHandles(1);
	cmdbuf[3]=handle;
	cmdbuf[4]=IPC_Desc_StaticBuffer(cmdbuf[1],0);
	cmdbuf[5]= (u32) param;
	
	return aptSendCommand(cmdbuf);
}

//See http://3dbrew.org/APT:SetApplicationCpuTimeLimit
Result APT_SetAppCpuTimeLimit(u32 percent)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x4F,2,0); // 0x4F0080
	cmdbuf[1]=1;
	cmdbuf[2]=percent;
	
	return aptSendCommand(cmdbuf);
}

Result APT_GetAppCpuTimeLimit(u32 *percent)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x50,1,0); // 0x500040
	cmdbuf[1]=1;
	
	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret))
		*percent=cmdbuf[2];

	return ret;
}

static Result APT_CheckNew3DS_System(bool* out)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x102,0,0); // 0x1020000
	
	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret))
		*out = cmdbuf[2];

	return ret;
}

Result APT_CheckNew3DS(bool* out)
{
	static bool flagInit, flagValue;
	if (!flagInit)
	{
		Result ret = APT_CheckNew3DS_System(&flagValue);
		if (R_FAILED(ret)) return ret;
		flagInit = true;
	}

	*out = flagValue;
	return 0;
}

Result APT_PrepareToDoApplicationJump(u8 flags, u64 programID, u8 mediatype)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x31,4,0); // 0x310100
	cmdbuf[1]=flags;
	cmdbuf[2]=(u32)programID;
	cmdbuf[3]=(u32)(programID>>32);
	cmdbuf[4]=mediatype;
	
	return aptSendCommand(cmdbuf);
}

Result APT_DoApplicationJump(const void* param, size_t paramSize, const void* hmac)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x32,2,4); // 0x320084
	cmdbuf[1]=paramSize;
	cmdbuf[2]=hmac ? 0x20 : 0;
	cmdbuf[3]=IPC_Desc_StaticBuffer(cmdbuf[1],0);
	cmdbuf[4]=(u32)param;
	cmdbuf[5]=IPC_Desc_StaticBuffer(cmdbuf[2],2);
	cmdbuf[6]=(u32)hmac;
	
	return aptSendCommand(cmdbuf);
}

Result APT_PrepareToStartLibraryApplet(NS_APPID appID)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x18,1,0); // 0x180040
	cmdbuf[1]=appID;
	
	return aptSendCommand(cmdbuf);
}

Result APT_StartLibraryApplet(NS_APPID appID, const void* param, size_t paramSize, Handle handle)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x1E,2,4); // 0x1E0084
	cmdbuf[1]=appID;
	cmdbuf[2]=paramSize;
	cmdbuf[3]=IPC_Desc_SharedHandles(1);
	cmdbuf[4]=handle;
	cmdbuf[5]=IPC_Desc_StaticBuffer(cmdbuf[2],0);
	cmdbuf[6]=(u32)param;
	
	return aptSendCommand(cmdbuf);
}

Result APT_LaunchLibraryApplet(NS_APPID appID, Handle inhandle, u32 *parambuf, u32 parambufsize)
{
	Result ret=0;

	APT_ReplySleepQuery(currentAppId, APTREPLY_REJECT);

	ret=APT_PrepareToStartLibraryApplet(appID);

	if(R_FAILED(ret))return ret;

	APT_SleepIfShellClosed();

	while(1)
	{
		bool tmp;
		ret=APT_IsRegistered(appID, &tmp);
		if(R_FAILED(ret))return ret;

		if(tmp!=0)break;
	}

	aptCallHook(APTHOOK_ONSUSPEND);

	__apt_launchapplet_appID = appID;
	__apt_launchapplet_inhandle = inhandle;
	__apt_launchapplet_parambuf = parambuf;
	__apt_launchapplet_parambufsize = parambufsize;

	// Set status to SUSPENDED.
	svcClearEvent(aptStatusEvent);
	aptSetStatus(APP_SUSPENDED);

	// Save Vram
	GSPGPU_SaveVramSysArea();

	// Capture screen.
	memset(__ns_capinfo, 0, 0x20);

	aptInitCaptureInfo(__ns_capinfo);

	// Send capture-screen info to the library applet.
	APT_SendParameter(currentAppId, appID, APTCMD_REQUEST, __ns_capinfo, sizeof(__ns_capinfo), 0);

	// Release GSP module.
	GSPGPU_ReleaseRight();

	return 0;
}

Result APT_PrepareToStartSystemApplet(NS_APPID appID)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0x19,1,0); // 0x190040
	cmdbuf[1]=appID;
	
	return aptSendCommand(cmdbuf);
}

Result APT_StartSystemApplet(NS_APPID appID, const void* param, size_t paramSize, Handle handle)
{
	u32 cmdbuf[16];
	cmdbuf[0] = IPC_MakeHeader(0x1F,2,4); // 0x001F0084
	cmdbuf[1] = appID;
	cmdbuf[2] = paramSize;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = handle;
	cmdbuf[5] = IPC_Desc_StaticBuffer(cmdbuf[2],0);
	cmdbuf[6] = (u32)param;
	
	return aptSendCommand(cmdbuf);
}

Result APT_GetSharedFont(Handle* fontHandle, u32* mapAddr)
{
	u32 cmdbuf[16];
	cmdbuf[0] = IPC_MakeHeader(0x44,0,0); // 0x00440000

	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret))
	{
		if(fontHandle) *fontHandle = cmdbuf[4];
		if(mapAddr)    *mapAddr    = cmdbuf[2];
	}

	return ret;
}
