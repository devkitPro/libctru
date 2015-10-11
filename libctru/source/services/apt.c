/*
  apt.c _ Applet/NS shell interaction
*/

#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/apt.h>
#include <3ds/services/gsp.h>


#define APT_HANDLER_STACKSIZE (0x1000)

//TODO : better place to put this ?
extern u32 __apt_appid;
extern u32 __system_runflags;

NS_APPID currentAppId;

static char *__apt_servicestr = NULL;
static char *__apt_servicenames[3] = {"APT:U", "APT:S", "APT:A"};

static u32 __apt_new3dsflag_initialized = 0;
static u8 __apt_new3dsflag = 0;

Handle aptLockHandle;
Handle aptuHandle;
Handle aptEvents[3];

Handle aptEventHandlerThread;
u64 aptEventHandlerStack[APT_HANDLER_STACKSIZE/8]; // u64 so that it's 8-byte aligned

Handle aptStatusMutex;
Handle aptStatusEvent = 0;
APP_STATUS aptStatus = APP_NOTINITIALIZED;
APP_STATUS aptStatusBeforeSleep = APP_NOTINITIALIZED;
u32 aptStatusPower = 0;
Handle aptSleepSync = 0;

u32 aptParameters[0x1000/4]; //TEMP

static u32 __ns_capinfo[0x20>>2];

static NS_APPID __apt_launchapplet_appID;
static Handle __apt_launchapplet_inhandle;
static u32 *__apt_launchapplet_parambuf;
static u32 __apt_launchapplet_parambufsize;

static aptHookCookie aptFirstHook;

static void aptCallHook(int hookType)
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
	return (__system_runflags & RUNFLAG_APTREINIT) != 0;
}

static bool aptIsCrippled(void)
{
	return (__system_runflags & RUNFLAG_APTWORKAROUND) != 0 && !aptIsReinit();
}

static Result __apt_initservicehandle()
{
	Result ret=0;
	u32 i;

	if(__apt_servicestr)
	{
		return srvGetServiceHandle(&aptuHandle, __apt_servicestr);
	}

	for(i=0; i<3; i++)
	{
		ret = srvGetServiceHandle(&aptuHandle, __apt_servicenames[i]);
		if(ret==0)
		{
			__apt_servicestr = __apt_servicenames[i];
			return ret;
		}
	}

	return ret;
}

void aptInitCaptureInfo(u32 *ns_capinfo)
{
	u32 tmp=0;
	u32 main_pixsz, sub_pixsz;
	GSP_CaptureInfo gspcapinfo;

	memset(&gspcapinfo, 0, sizeof(GSP_CaptureInfo));

	// Get display-capture info from GSP.
	GSPGPU_ImportDisplayCaptureInfo(NULL, &gspcapinfo);

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

void aptWaitStatusEvent()
{
	svcWaitSynchronization(aptStatusEvent, U64_MAX);
	svcClearEvent(aptStatusEvent);
}

void aptAppletUtility_Exit_RetToApp(u32 type)
{
	u8 buf1[4], buf2[4];

	memset(buf1, 0, 4);
	
	buf1[0]=0x10;
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x7, 0x4, buf1, 0x1, buf2);
	aptCloseSession();

	buf1[0]=0x00;
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
	aptCloseSession();

	buf1[0]=0x01;
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x7, 0x4, buf1, 0x1, buf2);
	aptCloseSession();

	buf1[0]=0x00;
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
	aptCloseSession();

	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
	aptCloseSession();

	if(type)
	{
		aptOpenSession();
		APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
		aptCloseSession();
	}
}

NS_APPID aptGetMenuAppID()
{
	NS_APPID menu_appid;

	aptOpenSession();
	APT_GetAppletManInfo(NULL, 0xff, NULL, NULL, &menu_appid, NULL);
	aptCloseSession();

	return menu_appid;
}

void aptReturnToMenu()
{
	NS_APPID menu_appid;
	u32 tmp0 = 1, tmp1 = 0;

	if(aptIsCrippled())
	{
		svcClearEvent(aptStatusEvent);
		aptSetStatus(APP_EXITING);
		return;
	}

	// This is only executed when ret-to-menu was triggered via the home-button, not the power-button.
	if(aptGetStatusPower() == 0)
	{
		aptOpenSession();
		APT_AppletUtility(NULL, NULL, 0x6, 0x4, (u8*)&tmp0, 0x1, (u8*)&tmp1);
		aptCloseSession();
	}

	// Set status to SUSPENDED.
	__apt_launchapplet_appID = 0;
	svcClearEvent(aptStatusEvent);
	aptSetStatus(APP_SUSPENDED);

	// Prepare for return to menu
	aptOpenSession();
	APT_PrepareToJumpToHomeMenu(NULL);
	aptCloseSession();

	// Save Vram
	GSPGPU_SaveVramSysArea(NULL);

	// Capture screen.
	memset(__ns_capinfo, 0, 0x20);

	aptInitCaptureInfo(__ns_capinfo);

	menu_appid = aptGetMenuAppID();

	// Send capture-screen info to menu.
	aptOpenSession();
	APT_SendParameter(NULL, currentAppId, menu_appid, 0x20, __ns_capinfo, 0x0, 0x10);
	aptCloseSession();

	aptOpenSession();
	APT_SendCaptureBufferInfo(NULL, 0x20, __ns_capinfo);
	aptCloseSession();

	// Release GSP module.
	GSPGPU_ReleaseRight(NULL);

	// Jump to menu!
	aptOpenSession();
	APT_JumpToHomeMenu(NULL, 0x0, 0x0, 0x0);
	aptCloseSession();

	// Wait for return to application.
	aptOpenSession();
	APT_NotifyToWait(NULL, currentAppId);
	aptCloseSession();

	// This is only executed when ret-to-menu was triggered via the home-button, not the power-button.
	if(aptGetStatusPower() == 0)
	{
		tmp0 = 0;
		aptOpenSession();
		APT_AppletUtility(NULL, NULL, 0x4, 0x1, (u8*)&tmp0, 0x1, (u8*)&tmp1);
		aptCloseSession();
	}

	aptWaitStatusEvent();
}

void aptAppletStarted()
{
	u8 buf1[4], buf2[4];

	memset(buf1, 0, 4);

	// Set status to SUSPENDED.
	svcClearEvent(aptStatusEvent);
	aptSetStatus(APP_SUSPENDED);

	aptOpenSession();
	APT_SendCaptureBufferInfo(NULL, 0x20, __ns_capinfo);
	aptCloseSession();

	aptOpenSession();
	APT_ReplySleepQuery(NULL, currentAppId, 0x0);
	aptCloseSession();

	aptOpenSession();
	APT_StartLibraryApplet(NULL, __apt_launchapplet_appID, __apt_launchapplet_inhandle, __apt_launchapplet_parambuf, __apt_launchapplet_parambufsize);
	aptCloseSession();

	buf1[0]=0x00;
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
	aptCloseSession();

	aptOpenSession();
	APT_NotifyToWait(NULL, currentAppId);
	aptCloseSession();

	buf1[0]=0x00;
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
	aptCloseSession();
}

void aptAppletClosed()
{
	aptAppletUtility_Exit_RetToApp(1);

	GSPGPU_AcquireRight(NULL, 0x0);
	GSPGPU_RestoreVramSysArea(NULL);

	svcClearEvent(aptStatusEvent);
	aptSetStatus(APP_RUNNING);
	svcClearEvent(aptStatusEvent);
}

static void __handle_notification() {
	u8 type;
	Result ret=0;

	// Get notification type.
	aptOpenSession();
	ret = APT_InquireNotification(NULL, currentAppId, &type);
	aptCloseSession();
	if(ret!=0) return;

	_aptDebug(1, type);

	switch(type)
	{
	case APTSIGNAL_HOMEBUTTON:
	case APTSIGNAL_POWERBUTTON:
		// The main thread should call aptReturnToMenu() when the status gets set to this.
		if(aptGetStatus() == APP_RUNNING)
		{
			aptOpenSession();
			APT_ReplySleepQuery(NULL, currentAppId, 0x0);
			aptCloseSession();
		
			if(type == APTSIGNAL_HOMEBUTTON)  aptSetStatusPower(0);
			if(type == APTSIGNAL_POWERBUTTON) aptSetStatusPower(1);
			aptSetStatus(APP_SUSPENDING);
		}
		break;

	case APTSIGNAL_PREPARESLEEP:
		// Reply to sleep-request.
		aptStatusBeforeSleep = aptGetStatus();
		aptSetStatus(APP_PREPARE_SLEEPMODE);
		svcWaitSynchronization(aptSleepSync, U64_MAX);
		svcClearEvent(aptSleepSync);
		
		aptOpenSession();
		APT_ReplySleepQuery(NULL, currentAppId, 0x1);
		aptCloseSession();
		break;

	case APTSIGNAL_ENTERSLEEP:
		if(aptGetStatus() == APP_PREPARE_SLEEPMODE)
		{
			// Report into sleep-mode.
			aptSetStatus(APP_SLEEPMODE);
			
			aptOpenSession();
			APT_ReplySleepNotificationComplete(NULL, currentAppId);
			aptCloseSession();
		}
		break;

	// Leaving sleep-mode.
	case APTSIGNAL_WAKEUP:
		if(aptGetStatus() == APP_SLEEPMODE)
		{
			if(aptStatusBeforeSleep == APP_RUNNING)GSPGPU_SetLcdForceBlack(NULL, 0);

			// Restore old aptStatus.
			aptSetStatus(aptStatusBeforeSleep);
		}
		break;
	}
}

static bool __handle_incoming_parameter() {
	u8 type;

	aptOpenSession();
	APT_ReceiveParameter(NULL, currentAppId, 0x1000, aptParameters, NULL, &type);
	aptCloseSession();

	_aptDebug(2, type);

	switch(type)
	{
	case 0x1: // Application just started.
		aptAppStarted();
		return true;

	case 0x3: // "Launched library applet finished loading"
		if (aptGetStatus() != APP_SUSPENDED || __apt_launchapplet_appID==0) return true;
		aptSetStatus(APP_APPLETSTARTED);
		return true;
	case 0xA: // "Launched library applet closed"
		if (aptGetStatus() != APP_SUSPENDED || __apt_launchapplet_appID==0) return true;
		if(__apt_launchapplet_parambuf && __apt_launchapplet_parambufsize)memcpy(__apt_launchapplet_parambuf, aptParameters, __apt_launchapplet_parambufsize);
		aptSetStatus(APP_APPLETCLOSED);
		return true;
	case 0xB: // Just returned from menu.
		if (aptGetStatus() != APP_NOTINITIALIZED)
		{
			GSPGPU_AcquireRight(NULL, 0x0);
			GSPGPU_RestoreVramSysArea(NULL);
			aptAppletUtility_Exit_RetToApp(0);
			aptSetStatus(APP_RUNNING);
		} else
			aptAppStarted();
		return true;

	case 0xC: // Exiting application.
		aptSetStatus(APP_EXITING);
		return false;
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

	svcExitThread();
}

static bool aptInitialised = false;

Result aptInit(void)
{
	Result ret=0;

	if (aptInitialised) return ret;

	aptStatusMutex = 0;

	// Initialize APT stuff, escape load screen.
	ret = __apt_initservicehandle();
	if(ret!=0)return ret;
	if((ret=APT_GetLockHandle(&aptuHandle, 0x0, &aptLockHandle)))return ret;
	svcCloseHandle(aptuHandle);

	currentAppId = __apt_appid;

	svcCreateEvent(&aptStatusEvent, 0);
	svcCreateEvent(&aptSleepSync, 0);
	svcCreateMutex(&aptStatusMutex, false);
	aptStatus=0;

	if(!aptIsCrippled())
	{
		aptOpenSession();
		if((ret=APT_Initialize(NULL, currentAppId, &aptEvents[0], &aptEvents[1])))return ret;
		aptCloseSession();
		
		aptOpenSession();
		if((ret=APT_Enable(NULL, 0x0)))return ret;
		aptCloseSession();
		
		// create APT close event
		svcCreateEvent(&aptEvents[2], 0);

		// After a cycle of APT_Finalize+APT_Initialize APT thinks the
		// application is suspended, so we need to tell it to unsuspend us.
		if (aptIsReinit())
		{
			aptOpenSession();
			APT_PrepareToJumpToApplication(NULL, 0x0);
			aptCloseSession();

			aptOpenSession();
			APT_JumpToApplication(NULL, 0x0, 0x0, 0x0);
			aptCloseSession();
		}
		
		aptOpenSession();
		if((ret=APT_NotifyToWait(NULL, currentAppId)))return ret;
		aptCloseSession();

		// create APT event handler thread
		svcCreateThread(&aptEventHandlerThread, aptEventHandler, 0x0,
			(u32*)(&aptEventHandlerStack[APT_HANDLER_STACKSIZE/8]), 0x31, 0xfffffffe);
	} else
		aptAppStarted();

	aptInitialised = true;

	return 0;
}

void aptExit()
{
	if (!aptInitialised) return;

	if(!aptIsCrippled())aptAppletUtility_Exit_RetToApp(0);

	// This is only executed when application-termination was triggered via the home-menu power-off screen.
	if(aptGetStatusPower() == 1)
	{
		aptOpenSession();
		APT_ReplySleepQuery(NULL, currentAppId, 0x0);
		aptCloseSession();
	}

	if(!aptIsCrippled())
	{
		bool isReinit = aptIsReinit();
		if (aptGetStatus() == APP_EXITING || !isReinit)
		{
			aptOpenSession();
			APT_PrepareToCloseApplication(NULL, 0x1);
			aptCloseSession();
		
			aptOpenSession();
			APT_CloseApplication(NULL, 0x0, 0x0, 0x0);
			aptCloseSession();

			if (isReinit)
			{
				extern void (*__system_retAddr)(void);
				__system_retAddr = NULL;
			}
		} else if (isReinit)
		{
			aptOpenSession();
			APT_Finalize(NULL, currentAppId);
			aptCloseSession();
		}
	}

	svcSignalEvent(aptEvents[2]);
	svcWaitSynchronization(aptEventHandlerThread, U64_MAX);
	svcCloseHandle(aptEventHandlerThread);
	svcCloseHandle(aptEvents[2]);
	
	svcCloseHandle(aptSleepSync);

	svcCloseHandle(aptStatusMutex);
	svcCloseHandle(aptLockHandle);
	svcCloseHandle(aptStatusEvent);
	
	aptInitialised = false;
}

bool aptMainLoop()
{
	while(1)
	{
		//if(aptIsCrippled())__handle_notification();

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

void aptAppStarted()
{
	u8 buf1[4], buf2[4];

	aptSetStatus(APP_RUNNING);

	if(!aptIsCrippled())
	{
		memset(buf1, 0, 4);

		buf1[0] = 0x10;
		aptOpenSession();
		APT_AppletUtility(NULL, NULL, 0x7, 0x4, buf1, 0x1, buf2);
		aptCloseSession();

		buf1[0] = 0x00;
		aptOpenSession();
		APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
		aptCloseSession();

		aptOpenSession();
		APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
		aptCloseSession();
	}
}

APP_STATUS aptGetStatus()
{
	APP_STATUS ret;
	svcWaitSynchronization(aptStatusMutex, U64_MAX);
	ret = aptStatus;
	svcReleaseMutex(aptStatusMutex);
	return ret;
}

void aptSetStatus(APP_STATUS status)
{
	svcWaitSynchronization(aptStatusMutex, U64_MAX);

	aptStatus = status;

	//if(prevstatus != APP_NOTINITIALIZED)
	//{
		if(status == APP_RUNNING)
			__ctru_speedup_config();
		if(status == APP_RUNNING || status == APP_EXITING || status == APP_APPLETSTARTED || status == APP_APPLETCLOSED)
			svcSignalEvent(aptStatusEvent);
	//}

	svcReleaseMutex(aptStatusMutex);
}

u32 aptGetStatusPower()
{
	u32 ret;
	svcWaitSynchronization(aptStatusMutex, U64_MAX);
	ret = aptStatusPower;
	svcReleaseMutex(aptStatusMutex);
	return ret;
}

void aptSetStatusPower(u32 status)
{
	svcWaitSynchronization(aptStatusMutex, U64_MAX);
	aptStatusPower = status;
	svcReleaseMutex(aptStatusMutex);
}

void aptOpenSession()
{
	//Result ret;

	svcWaitSynchronization(aptLockHandle, U64_MAX);
	__apt_initservicehandle();
}

void aptCloseSession()
{
	svcCloseHandle(aptuHandle);
	svcReleaseMutex(aptLockHandle);
}

void aptSignalReadyForSleep()
{
	svcSignalEvent(aptSleepSync);
}

Result APT_GetLockHandle(Handle* handle, u16 flags, Handle* lockHandle)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x10040; //request header code
	cmdbuf[1]=flags;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;
	
	if(lockHandle)*lockHandle=cmdbuf[5];
	
	return cmdbuf[1];
}

Result APT_Initialize(Handle* handle, NS_APPID appId, Handle* eventHandle1, Handle* eventHandle2)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x20080; //request header code
	cmdbuf[1]=appId;
	cmdbuf[2]=0x0;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;
	
	if(eventHandle1)*eventHandle1=cmdbuf[3]; //return to menu event ?
	if(eventHandle2)*eventHandle2=cmdbuf[4];

	return cmdbuf[1];
}

Result APT_Finalize(Handle* handle, NS_APPID appId)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x40040; //request header code
	cmdbuf[1]=appId;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;
	return cmdbuf[1];
}

Result APT_HardwareResetAsync(Handle* handle)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x4E0000; //request header code
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;
	
	return cmdbuf[1];
}

Result APT_Enable(Handle* handle, u32 a)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x30040; //request header code
	cmdbuf[1]=a;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;
	
	return cmdbuf[1];
}

Result APT_GetAppletManInfo(Handle* handle, u8 inval, u8 *outval8, u32 *outval32, NS_APPID *menu_appid, NS_APPID *active_appid)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00050040; //request header code
	cmdbuf[1]=inval;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(outval8)*outval8=cmdbuf[2];
	if(outval32)*outval32=cmdbuf[3];
	if(menu_appid)*menu_appid=cmdbuf[4];
	if(active_appid)*active_appid=cmdbuf[5];
	
	return cmdbuf[1];
}

Result APT_GetAppletInfo(Handle* handle, NS_APPID appID, u64* pProgramID, u8* pMediaType, u8* pRegistered, u8* pLoadState, u32* pAttributes)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00060040; //request header code
	cmdbuf[1]=appID;

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(pProgramID)*pProgramID=(u64)cmdbuf[2]|((u64)cmdbuf[3]<<32);
	if(pMediaType)*pMediaType=cmdbuf[4];
	if(pRegistered)*pRegistered=cmdbuf[5];
	if(pLoadState)*pLoadState=cmdbuf[6];
	if(pAttributes)*pAttributes=cmdbuf[7];

	return cmdbuf[1];
}

Result APT_GetAppletProgramInfo(Handle* handle, u32 id, u32 flags, u16 *titleversion)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x004D0080; //request header code
	cmdbuf[1]=id;
	cmdbuf[2]=flags;

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(titleversion)*titleversion=cmdbuf[2];

	return cmdbuf[1];
}

Result APT_GetProgramID(Handle* handle, u64* pProgramID)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0] = 0x00580002; //request header code
	cmdbuf[1] = 0x20;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(ret==0)ret = cmdbuf[1];

	if(pProgramID)
	{
		if(ret==0) *pProgramID=((u64)cmdbuf[3]<<32)|cmdbuf[2];
	}

	return ret;
}

Result APT_IsRegistered(Handle* handle, NS_APPID appID, u8* out)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x90040; //request header code
	cmdbuf[1]=appID;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(out)*out=cmdbuf[2];
	
	return cmdbuf[1];
}

Result APT_InquireNotification(Handle* handle, u32 appID, u8* signalType)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xB0040; //request header code
	cmdbuf[1]=appID;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(signalType)*signalType=cmdbuf[2];
	
	return cmdbuf[1];
}

Result APT_PrepareToJumpToHomeMenu(Handle* handle)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x2b0000; //request header code
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;
	
	return cmdbuf[1];
}

Result APT_JumpToHomeMenu(Handle* handle, u32 a, u32 b, u32 c)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x2C0044; //request header code
	cmdbuf[1]=a;
	cmdbuf[2]=b;
	cmdbuf[3]=c;
	cmdbuf[4]=(b<<14)|2;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;
	
	return cmdbuf[1];
}

Result APT_PrepareToJumpToApplication(Handle* handle, u32 a)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x230040; //request header code
	cmdbuf[1]=a;

	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;
	
	return cmdbuf[1];
}

Result APT_JumpToApplication(Handle* handle, u32 a, u32 b, u32 c)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x240044; //request header code
	cmdbuf[1]=a;
	cmdbuf[2]=b;
	cmdbuf[3]=c;
	cmdbuf[4]=(b<<14)|2;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;
	
	return cmdbuf[1];
}

Result APT_NotifyToWait(Handle* handle, NS_APPID appID)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x430040; //request header code
	cmdbuf[1]=appID;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_AppletUtility(Handle* handle, u32* out, u32 a, u32 size1, u8* buf1, u32 size2, u8* buf2)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x4B00C2; //request header code
	cmdbuf[1]=a;
	cmdbuf[2]=size1;
	cmdbuf[3]=size2;
	cmdbuf[4]=(size1<<14)|0x402;
	cmdbuf[5]=(u32)buf1;
	
	cmdbuf[0+0x100/4]=(size2<<14)|2;
	cmdbuf[1+0x100/4]=(u32)buf2;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(out)*out=cmdbuf[2];

	return cmdbuf[1];
}

Result APT_GlanceParameter(Handle* handle, NS_APPID appID, u32 bufferSize, u32* buffer, u32* actualSize, u8* signalType)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xE0080; //request header code
	cmdbuf[1]=appID;
	cmdbuf[2]=bufferSize;
	
	cmdbuf[0+0x100/4]=(bufferSize<<14)|2;
	cmdbuf[1+0x100/4]=(u32)buffer;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(signalType)*signalType=cmdbuf[3];
	if(actualSize)*actualSize=cmdbuf[4];

	return cmdbuf[1];
}

Result APT_ReceiveParameter(Handle* handle, NS_APPID appID, u32 bufferSize, u32* buffer, u32* actualSize, u8* signalType)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xD0080; //request header code
	cmdbuf[1]=appID;
	cmdbuf[2]=bufferSize;
	
	cmdbuf[0+0x100/4]=(bufferSize<<14)|2;
	cmdbuf[1+0x100/4]=(u32)buffer;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(signalType)*signalType=cmdbuf[3];
	if(actualSize)*actualSize=cmdbuf[4];

	return cmdbuf[1];
}

Result APT_SendParameter(Handle* handle, NS_APPID src_appID, NS_APPID dst_appID, u32 bufferSize, u32* buffer, Handle paramhandle, u8 signalType)
{
	u32* cmdbuf=getThreadCommandBuffer();

	if(!handle)handle=&aptuHandle;

	cmdbuf[0] = 0x000C0104; //request header code
	cmdbuf[1] = src_appID;
	cmdbuf[2] = dst_appID;
	cmdbuf[3] = signalType;
	cmdbuf[4] = bufferSize;

	cmdbuf[5]=0x0;
	cmdbuf[6] = paramhandle;
	
	cmdbuf[7] = (bufferSize<<14) | 2;
	cmdbuf[8] = (u32)buffer;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_SendCaptureBufferInfo(Handle* handle, u32 bufferSize, u32* buffer)
{
	u32* cmdbuf=getThreadCommandBuffer();

	if(!handle)handle=&aptuHandle;

	cmdbuf[0] = 0x00400042; //request header code
	cmdbuf[1] = bufferSize;
	cmdbuf[2] = (bufferSize<<14) | 2;
	cmdbuf[3] = (u32)buffer;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_ReplySleepQuery(Handle* handle, NS_APPID appID, u32 a)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x3E0080; //request header code
	cmdbuf[1]=appID;
	cmdbuf[2]=a;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_ReplySleepNotificationComplete(Handle* handle, NS_APPID appID)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x3F0040; //request header code
	cmdbuf[1]=appID;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_PrepareToCloseApplication(Handle* handle, u8 a)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x220040; //request header code
	cmdbuf[1]=a;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_CloseApplication(Handle* handle, u32 a, u32 b, u32 c)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x270044; //request header code
	cmdbuf[1]=a;
	cmdbuf[2]=0x0;
	cmdbuf[3]=b;
	cmdbuf[4]=(a<<14)|2;
	cmdbuf[5]=c;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

//See http://3dbrew.org/APT:SetApplicationCpuTimeLimit
Result APT_SetAppCpuTimeLimit(Handle* handle, u32 percent)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x4F0080;
	cmdbuf[1]=1;
	cmdbuf[2]=percent;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_GetAppCpuTimeLimit(Handle* handle, u32 *percent)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x500040;
	cmdbuf[1]=1;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(percent)*percent=cmdbuf[2];

	return cmdbuf[1];
}

// Note: this function is unreliable, see: http://3dbrew.org/wiki/APT:PrepareToStartApplication
Result APT_CheckNew3DS_Application(Handle* handle, u8 *out)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x01010000;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(ret==0)ret = cmdbuf[1];

	if(out)
	{
		*out = 0;
		if(ret==0)*out=cmdbuf[2];
	}

	return ret;
}

Result APT_CheckNew3DS_System(Handle* handle, u8 *out)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x01020000;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	if(ret==0)ret = cmdbuf[1];

	if(out)
	{
		*out = 0;
		if(ret==0)*out=cmdbuf[2];
	}

	return ret;
}

Result APT_CheckNew3DS(Handle* handle, u8 *out)
{
	Result ret=0;

	if(out==NULL)return -1;

	*out = 0;

	if(__apt_new3dsflag_initialized)
	{
		*out = __apt_new3dsflag;
		return 0;
	}

	aptOpenSession();
	ret = APT_CheckNew3DS_System(NULL, out);
	aptCloseSession();

	__apt_new3dsflag_initialized = 1;
	__apt_new3dsflag = *out;

	return ret;
}

Result APT_PrepareToDoAppJump(Handle* handle, u8 flags, u64 programID, u8 mediatype)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x310100; //request header code
	cmdbuf[1]=flags;
	cmdbuf[2]=(u32)programID;
	cmdbuf[3]=(u32)(programID>>32);
	cmdbuf[4]=mediatype;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_DoAppJump(Handle* handle, u32 NSbuf0Size, u32 NSbuf1Size, u8 *NSbuf0Ptr, u8 *NSbuf1Ptr)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x320084; //request header code
	cmdbuf[1]=NSbuf0Size;
	cmdbuf[2]=NSbuf1Size;
	cmdbuf[3]=(NSbuf0Size<<14)|2;
	cmdbuf[4]=(u32)NSbuf0Ptr;
	cmdbuf[5]=(NSbuf1Size<<14)|0x802;
	cmdbuf[6]=(u32)NSbuf1Ptr;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_PrepareToStartLibraryApplet(Handle* handle, NS_APPID appID)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x180040; //request header code
	cmdbuf[1]=appID;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_StartLibraryApplet(Handle* handle, NS_APPID appID, Handle inhandle, u32 *parambuf, u32 parambufsize)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x1E0084; //request header code
	cmdbuf[1]=appID;
	cmdbuf[2]=parambufsize;
	cmdbuf[3]=0;
	cmdbuf[4]=inhandle;
	cmdbuf[5]=(parambufsize<<14)|2;
	cmdbuf[6]=(u32)parambuf;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_LaunchLibraryApplet(NS_APPID appID, Handle inhandle, u32 *parambuf, u32 parambufsize)
{
	Result ret=0;
	u8 tmp=0;

	u8 buf1[4];
	u8 buf2[4];

	aptOpenSession();
	APT_ReplySleepQuery(NULL, currentAppId, 0);
	aptCloseSession();

	aptOpenSession();
	ret=APT_PrepareToStartLibraryApplet(NULL, appID);
	aptCloseSession();
	if(ret!=0)return ret;

	memset(buf1, 0, 4);
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
	aptCloseSession();

	while(1)
	{
		aptOpenSession();
		ret=APT_IsRegistered(NULL, appID, &tmp);
		aptCloseSession();
		if(ret!=0)return ret;

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
	GSPGPU_SaveVramSysArea(NULL);

	// Capture screen.
	memset(__ns_capinfo, 0, 0x20);

	aptInitCaptureInfo(__ns_capinfo);

	// Send capture-screen info to the library applet.
	aptOpenSession();
	APT_SendParameter(NULL, currentAppId, appID, 0x20, __ns_capinfo, 0x0, 0x2);
	aptCloseSession();

	// Release GSP module.
	GSPGPU_ReleaseRight(NULL);

	return 0;
}

Result APT_PrepareToStartSystemApplet(Handle* handle, NS_APPID appID)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x00190040; //request header code
	cmdbuf[1]=appID;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_StartSystemApplet(Handle* handle, NS_APPID appID, u32 bufSize, Handle applHandle, u8 *buf)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0] = 0x001F0084; //request header code
	cmdbuf[1] = appID;
	cmdbuf[2] = bufSize;
	cmdbuf[3] = 0;
	cmdbuf[4] = applHandle;
	cmdbuf[5] = (bufSize<<14) | 2;
	cmdbuf[6] = (u32)buf;
	
	Result ret=0;
	if((ret=svcSendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

