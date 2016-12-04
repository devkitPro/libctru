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

static int aptRefCount = 0;
static Handle aptLockHandle;
static Handle aptEvents[3];
static LightEvent aptSleepEvent;
static Thread aptEventHandlerThread;
static aptHookCookie aptFirstHook;
static aptMessageCb aptMessageFunc;
static void* aptMessageFuncData;

enum
{
	FLAG_ACTIVE       = BIT(0),
	FLAG_ALLOWSLEEP   = BIT(1),
	FLAG_ORDERTOCLOSE = BIT(2),
	FLAG_SHUTDOWN     = BIT(3),
	FLAG_POWERBUTTON  = BIT(4),
	FLAG_WKUPBYCANCEL = BIT(5),
	FLAG_WANTSTOSLEEP = BIT(6),
	FLAG_SLEEPING     = BIT(7),
	FLAG_EXITED       = BIT(31),
};

static u8 aptHomeButtonState;
static u32 aptFlags = FLAG_ALLOWSLEEP;
static u32 aptParameters[0x1000/4];

typedef enum
{
	TR_ENABLE     = 0x62,
	TR_JUMPTOMENU = 0x0E,
	TR_SYSAPPLET  = 0x05,
	TR_LIBAPPLET  = 0x04,
	TR_CANCELLIB  = 0x03,
	TR_CLOSEAPP   = 0x09,
	TR_APPJUMP    = 0x12,
} APT_Transition;

static void aptEventHandler(void *arg);
static APT_Command aptWaitForWakeUp(APT_Transition transition);

// The following function can be overriden in order to log APT signals and notifications for debugging purposes
__attribute__((weak)) void _aptDebug(int a, int b)
{
}

static void aptCallHook(APT_HookType hookType)
{
	aptHookCookie* c;
	for (c = &aptFirstHook; c && c->callback; c = c->next)
		c->callback(hookType, c->param);
}

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

static void aptClearParamQueue(void)
{
	// Check for parameters?
	for (;;)
	{
		APT_Command cmd;
		Result res = APT_GlanceParameter(envGetAptAppId(), aptParameters, sizeof(aptParameters), NULL, &cmd, NULL, NULL);
		if (R_FAILED(res) || cmd==APTCMD_NONE) break;
		_aptDebug(2, cmd);
		svcClearEvent(aptEvents[2]);
		APT_CancelParameter(APPID_NONE, envGetAptAppId(), NULL);
	}
}

static void aptInitCaptureInfo(aptCaptureBufInfo* capinfo)
{
	GSPGPU_CaptureInfo gspcapinfo;
	memset(&gspcapinfo, 0, sizeof(gspcapinfo));

	// Get display-capture info from GSP.
	GSPGPU_ImportDisplayCaptureInfo(&gspcapinfo);

	// Fill in display-capture info for NS.
	capinfo->is3D = (gspcapinfo.screencapture[0].format & 0x20) != 0;

	capinfo->top.format    = gspcapinfo.screencapture[0].format & 0x7;
	capinfo->bottom.format = gspcapinfo.screencapture[1].format & 0x7;

	u32 __get_bytes_per_pixel(u32 format);
	u32 main_pixsz = __get_bytes_per_pixel(capinfo->top.format);
	u32 sub_pixsz  = __get_bytes_per_pixel(capinfo->bottom.format);

	capinfo->bottom.leftOffset  = 0;
	capinfo->bottom.rightOffset = 0;
	capinfo->top.leftOffset  = sub_pixsz * 0x14000;
	capinfo->top.rightOffset = capinfo->top.leftOffset;

	if (capinfo->is3D)
		capinfo->top.rightOffset += main_pixsz * 0x19000;

	capinfo->size = main_pixsz * 0x7000 + main_pixsz * 0x19000 + capinfo->top.rightOffset;
}

Result aptInit(void)
{
	Result ret=0;

	if (AtomicPostIncrement(&aptRefCount)) return 0;

	// Retrieve APT lock
	ret = APT_GetLockHandle(0x0, &aptLockHandle);
	if (R_FAILED(ret)) goto _fail;
	if (aptIsCrippled()) return 0;

	// Initialize APT
	APT_AppletAttr attr = aptMakeAppletAttr(APTPOS_APP, false, false);
	ret = APT_Initialize(envGetAptAppId(), attr, &aptEvents[1], &aptEvents[2]);
	if (R_FAILED(ret)) goto _fail2;

	// Enable APT
	ret = APT_Enable(attr);
	if (R_FAILED(ret)) goto _fail3;

	// Create APT close event
	ret = svcCreateEvent(&aptEvents[0], RESET_STICKY);
	if (R_FAILED(ret)) goto _fail3;

	// Initialize APT sleep event
	LightEvent_Init(&aptSleepEvent, RESET_ONESHOT);

	// Create APT event handler thread
	aptEventHandlerThread = threadCreate(aptEventHandler, 0x0, APT_HANDLER_STACKSIZE, 0x31, -2, true);
	if (!aptEventHandlerThread) goto _fail4;

	// Special handling for aptReinit (aka hax)
	APT_Transition transition = TR_ENABLE;
	if (aptIsReinit())
	{
		transition = TR_JUMPTOMENU;

		// Clear out any pending parameters
		bool success = false;
		do
			ret = APT_CancelParameter(APPID_NONE, envGetAptAppId(), &success);
		while (success);

		// APT thinks the application is suspended, so we need to tell it to unsuspend us.
		APT_PrepareToJumpToApplication(false);
		APT_JumpToApplication(NULL, 0, 0);
	}

	// Wait for wakeup
	aptWaitForWakeUp(transition);
	return 0;

_fail4:
	svcCloseHandle(aptEvents[0]);
_fail3:
	svcCloseHandle(aptEvents[1]);
	svcCloseHandle(aptEvents[2]);
_fail2:
	svcCloseHandle(aptLockHandle);
_fail:
	AtomicDecrement(&aptRefCount);
	return ret;
}

bool aptIsSleepAllowed(void)
{
	return (aptFlags & FLAG_ALLOWSLEEP) != 0;
}

void aptSetSleepAllowed(bool allowed)
{
	bool cur = aptIsSleepAllowed();
	if (allowed && !cur)
	{
		aptFlags |= FLAG_ALLOWSLEEP;
		APT_SleepIfShellClosed();
	}
	else if (!allowed && cur)
	{
		aptFlags &= ~FLAG_ALLOWSLEEP;
		APT_ReplySleepQuery(envGetAptAppId(), APTREPLY_REJECT);
	}
}

static void aptExitProcess(void)
{
	APT_CloseApplication(NULL, 0, 0);
	svcExitProcess();
}

void aptExit(void)
{
	if (AtomicDecrement(&aptRefCount)) return;

	bool closeAptLock = true;

	if (!aptIsCrippled())
	{
		if ((aptFlags & FLAG_EXITED) || !aptIsReinit())
		{
			APT_PrepareToCloseApplication(true);

			extern void (*__system_retAddr)(void);
			__system_retAddr = aptExitProcess;
			closeAptLock = false;
			srvInit(); // Keep srv initialized
		} else
		{
			APT_Finalize(envGetAptAppId());
			aptClearParamQueue();
		}

		svcSignalEvent(aptEvents[0]);
		threadJoin(aptEventHandlerThread, U64_MAX);
		int i;
		for (i = 0; i < 3; i ++)
			svcCloseHandle(aptEvents[i]);
	}

	if (closeAptLock)
		svcCloseHandle(aptLockHandle);
}

void aptEventHandler(void *arg)
{
	for (;;)
	{
		s32 id = 0;
		svcWaitSynchronizationN(&id, aptEvents, 2, 0, U64_MAX);
		svcClearEvent(aptEvents[id]);
		if (id != 1) break;

		APT_Signal signal;
		Result res = APT_InquireNotification(envGetAptAppId(), &signal);
		if (R_FAILED(res)) break;
		switch (signal)
		{
			case APTSIGNAL_HOMEBUTTON:
				if (!aptHomeButtonState) aptHomeButtonState = 1;
				break;
			case APTSIGNAL_HOMEBUTTON2:
				if (!aptHomeButtonState) aptHomeButtonState = 2;
				break;
			case APTSIGNAL_SLEEP_QUERY:
				APT_ReplySleepQuery(envGetAptAppId(), aptIsSleepAllowed() ? APTREPLY_ACCEPT : APTREPLY_REJECT);
				break;
			case APTSIGNAL_SLEEP_CANCEL:
				// Do something maybe?
				break;
			case APTSIGNAL_SLEEP_ENTER:
				aptFlags |= FLAG_WANTSTOSLEEP;
				break;
			case APTSIGNAL_SLEEP_WAKEUP:
				if (aptFlags & FLAG_SLEEPING)
					LightEvent_Signal(&aptSleepEvent);
				else
					aptFlags &= ~FLAG_WANTSTOSLEEP;
				break;
			case APTSIGNAL_SHUTDOWN:
				aptFlags |= FLAG_ORDERTOCLOSE | FLAG_SHUTDOWN;
				break;
			case APTSIGNAL_POWERBUTTON:
				aptFlags |= FLAG_POWERBUTTON;
				break;
			case APTSIGNAL_POWERBUTTON2:
				aptFlags &= ~FLAG_POWERBUTTON;
				break;
			case APTSIGNAL_TRY_SLEEP:
				break;
			case APTSIGNAL_ORDERTOCLOSE:
				aptFlags |= FLAG_ORDERTOCLOSE;
				break;
			default:
				break;
		}
	}
}

static Result aptReceiveParameter(APT_Command* cmd, size_t* actualSize, Handle* handle)
{
	NS_APPID sender;
	size_t temp_actualSize;
	if (!actualSize) actualSize = &temp_actualSize;

	svcWaitSynchronization(aptEvents[2], U64_MAX);
	svcClearEvent(aptEvents[2]);
	Result res = APT_ReceiveParameter(envGetAptAppId(), aptParameters, sizeof(aptParameters), &sender, cmd, actualSize, handle);
	if (R_SUCCEEDED(res) && *cmd == APTCMD_MESSAGE && aptMessageFunc)
		aptMessageFunc(aptMessageFuncData, sender, aptParameters, *actualSize);
	return res;
}

APT_Command aptWaitForWakeUp(APT_Transition transition)
{
	APT_Command cmd;
	APT_NotifyToWait(envGetAptAppId());
	if (transition != TR_ENABLE)
		APT_SleepIfShellClosed();
	aptFlags &= ~FLAG_ACTIVE;
	for (;;)
	{
		Result res = aptReceiveParameter(&cmd, NULL, NULL);
		if (R_SUCCEEDED(res)
			&& (cmd==APTCMD_WAKEUP || cmd==APTCMD_WAKEUP_PAUSE || cmd==APTCMD_WAKEUP_EXIT || cmd==APTCMD_WAKEUP_CANCEL
			|| cmd==APTCMD_WAKEUP_CANCELALL || cmd==APTCMD_WAKEUP_POWERBUTTON || cmd==APTCMD_WAKEUP_JUMPTOHOME
			|| cmd==APTCMD_WAKEUP_LAUNCHAPP)) break;
	}
	aptFlags |= FLAG_ACTIVE;

	void __ctru_speedup_config();
	__ctru_speedup_config();

	if (transition != TR_CANCELLIB && cmd != APTCMD_WAKEUP_CANCEL && cmd != APTCMD_WAKEUP)
	{
		GSPGPU_AcquireRight(0);
		GSPGPU_RestoreVramSysArea();
		aptCallHook(APTHOOK_ONRESTORE);
	}

	if (cmd == APTCMD_WAKEUP_CANCEL)
		aptFlags |= FLAG_WKUPBYCANCEL;

	if (cmd != APTCMD_WAKEUP_JUMPTOHOME)
	{
		APT_UnlockTransition(0x10);
		APT_SleepIfShellClosed();
	} else
	{
		bool dummy;
		APT_TryLockTransition(0x01, &dummy);
	}

	if (transition == TR_JUMPTOMENU || transition == TR_LIBAPPLET || transition == TR_SYSAPPLET || transition == TR_APPJUMP)
	{
		if (cmd != APTCMD_WAKEUP_JUMPTOHOME)
		{
			aptHomeButtonState = 0;
			APT_UnlockTransition(0x01);
			APT_SleepIfShellClosed();
		}
	}

	return cmd;
}

static void aptScreenTransfer(NS_APPID appId, bool sysApplet)
{
	aptCallHook(APTHOOK_ONSUSPEND);
	GSPGPU_SaveVramSysArea();

	aptCaptureBufInfo capinfo;
	aptInitCaptureInfo(&capinfo);

	for (;;)
	{
		bool tmp;
		Result res = APT_IsRegistered(appId, &tmp);
		if (R_SUCCEEDED(res) && tmp) break;
		svcSleepThread(10000000);
	}

	for (;;)
	{
		Result res = APT_SendParameter(envGetAptAppId(), appId, sysApplet ? APTCMD_SYSAPPLET_REQUEST : APTCMD_REQUEST, &capinfo, sizeof(capinfo), 0);
		if (R_SUCCEEDED(res)) break;
		svcSleepThread(10000000);
	}

	for (;;)
	{
		APT_Command cmd;
		Result res = aptReceiveParameter(&cmd, NULL, NULL);
		if (R_SUCCEEDED(res) && cmd==APTCMD_RESPONSE)
			break;
	}

	APT_SendCaptureBufferInfo(&capinfo);
	GSPGPU_ReleaseRight();
}

static void aptProcessJumpToMenu(void)
{
	bool sleep = aptIsSleepAllowed();
	aptSetSleepAllowed(false);

	aptClearParamQueue();
	APT_PrepareToJumpToHomeMenu();
	aptScreenTransfer(aptGetMenuAppID(), true);

	APT_JumpToHomeMenu(NULL, 0, 0);
	aptFlags &= ~FLAG_ACTIVE;

	aptWaitForWakeUp(TR_JUMPTOMENU);
	aptSetSleepAllowed(sleep);
}

bool aptMainLoop(void)
{
	if (aptIsCrippled()) return true;
	if (aptFlags & FLAG_EXITED) return false;

	if (aptFlags & FLAG_WANTSTOSLEEP)
	{
		aptFlags = (aptFlags &~ FLAG_WANTSTOSLEEP) | FLAG_SLEEPING;
		aptCallHook(APTHOOK_ONSLEEP);
		APT_ReplySleepNotificationComplete(envGetAptAppId());
		LightEvent_Wait(&aptSleepEvent);
		aptFlags &= ~FLAG_SLEEPING;

		if (aptFlags & FLAG_ACTIVE)
			GSPGPU_SetLcdForceBlack(0);
		aptCallHook(APTHOOK_ONWAKEUP);
	}
	else if ((aptFlags & FLAG_POWERBUTTON) || aptHomeButtonState)
		aptProcessJumpToMenu();

	if (aptFlags & (FLAG_ORDERTOCLOSE|FLAG_WKUPBYCANCEL))
	{
		aptFlags |= FLAG_EXITED;
		aptCallHook(APTHOOK_ONEXIT);
		return false;
	}

	return true;
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

void aptSetMessageCallback(aptMessageCb callback, void* user)
{
	aptMessageFunc = callback;
	aptMessageFuncData = user;
}

bool aptLaunchLibraryApplet(NS_APPID appId, void* buf, size_t bufsize, Handle handle)
{
	bool sleep = aptIsSleepAllowed();

	aptSetSleepAllowed(false);
	aptClearParamQueue();
	APT_PrepareToStartLibraryApplet(appId);
	aptSetSleepAllowed(sleep);

	aptScreenTransfer(appId, false);

	aptSetSleepAllowed(false);
	APT_StartLibraryApplet(appId, buf, bufsize, handle);
	aptFlags &= ~FLAG_ACTIVE;

	aptWaitForWakeUp(TR_LIBAPPLET);
	memcpy(buf, aptParameters, bufsize);
	aptSetSleepAllowed(sleep);

	return aptMainLoop();
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
		if (pRegistered) *pRegistered=cmdbuf[5] & 0xFF;
		if (pLoadState)  *pLoadState =cmdbuf[6] & 0xFF;
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
		*out=cmdbuf[2] & 0xFF;

	return ret;
}

Result APT_InquireNotification(u32 appID, APT_Signal* signalType)
{
	u32 cmdbuf[16];
	cmdbuf[0]=IPC_MakeHeader(0xB,1,0); // 0xB0040
	cmdbuf[1]=appID;

	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret))
	{
		_aptDebug(1, cmdbuf[2]);
		*signalType=cmdbuf[2];
	}

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

Result APT_GlanceParameter(NS_APPID appID, void* buffer, size_t bufferSize, NS_APPID* sender, APT_Command* command, size_t* actualSize, Handle* parameter)
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
		if (sender)     *sender    =cmdbuf[2];
		if (command)    *command   =cmdbuf[3];
		if (actualSize) *actualSize=cmdbuf[4];
		if (parameter)  *parameter =cmdbuf[6];
		else if (cmdbuf[6]) svcCloseHandle(cmdbuf[6]);
	}

	return ret;
}

Result APT_ReceiveParameter(NS_APPID appID, void* buffer, size_t bufferSize, NS_APPID* sender, APT_Command* command, size_t* actualSize, Handle* parameter)
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
		_aptDebug(2, cmdbuf[3]);
		if (sender)     *sender    =cmdbuf[2];
		if (command)    *command   =cmdbuf[3];
		if (actualSize) *actualSize=cmdbuf[4];
		if (parameter)  *parameter =cmdbuf[6];
		else if (cmdbuf[6]) svcCloseHandle(cmdbuf[6]);
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

Result APT_CancelParameter(NS_APPID source, NS_APPID dest, bool* success)
{
	u32 cmdbuf[16];

	cmdbuf[0] = IPC_MakeHeader(0xF,4,0); // 0xF0100
	cmdbuf[1] = source != APPID_NONE;
	cmdbuf[2] = source;
	cmdbuf[3] = dest != APPID_NONE;
	cmdbuf[4] = dest;

	Result ret = aptSendCommand(cmdbuf);
	if (R_SUCCEEDED(ret) && success)
		*success = cmdbuf[2] & 0xFF;

	return ret;
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
		*out = cmdbuf[2] & 0xFF;

	return ret;
}

Result APT_CheckNew3DS(bool* out)
{
	static bool flagInit, flagValue;
	if (!flagInit)
	{
		*out = false;
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
