/*
  apt.c _ Applet/NS shell interaction
*/

#include <stdlib.h>
#include <string.h>
#include <3ds.h>

#define APT_HANDLER_STACKSIZE (0x1000)

//TODO : better place to put this ?
extern u32 __apt_appid;
extern u32 __system_runflags;

NS_APPID currentAppId;

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


void aptInitCaptureInfo(u32 *ns_capinfo)
{
	u32 tmp=0;
	u32 main_pixsz, sub_pixsz;
	GSP_CaptureInfo gspcapinfo;

	memset(&gspcapinfo, 0, sizeof(GSP_CaptureInfo));

	// Get display-capture info from GSP.
	GSPGPU_ImportDisplayCaptureInfo(NULL, &gspcapinfo);

	// Fill in display-capture info for NS.
	if(gspcapinfo.screencapture[0].framebuf0_vaddr != gspcapinfo.screencapture[1].framebuf0_vaddr)ns_capinfo[1] = 1;
	
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

void aptAppletUtility_Exit_RetToApp()
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
	u32 ns_capinfo[0x20>>2];
	u32 tmp_params[0x20>>2];

	// This is only executed when ret-to-menu was triggered via the home-button, not the power-button.
	if(aptGetStatusPower() == 0)
	{
		aptOpenSession();
		APT_AppletUtility(NULL, NULL, 0x6, 0x4, (u8*)&tmp0, 0x1, (u8*)&tmp1);
		aptCloseSession();
	}

	// Prepare for return to menu
	aptOpenSession();
	APT_PrepareToJumpToHomeMenu(NULL);
	aptCloseSession();

	// Set status to SUSPENDED.
	svcClearEvent(aptStatusEvent);
	aptSetStatus(APP_SUSPENDED);

	// Save Vram
	GSPGPU_SaveVramSysArea(NULL);

	// Capture screen.
	memset(tmp_params, 0, 0x20);
	memset(ns_capinfo, 0, 0x20);

	aptInitCaptureInfo(ns_capinfo);

	menu_appid = aptGetMenuAppID();

	// Send capture-screen info to menu.
	aptOpenSession();
	APT_SendParameter(NULL, currentAppId, menu_appid, 0x20, ns_capinfo, 0x0, 0x10);
	aptCloseSession();

	aptOpenSession();
	APT_SendCaptureBufferInfo(NULL, 0x20, ns_capinfo);
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

static void __handle_notification() {
	u8 type;

	// Get notification type.
	aptOpenSession();
	APT_InquireNotification(NULL, currentAppId, &type);
	aptCloseSession();

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

	switch(type)
	{
	case 0x1: // Application just started.
		return true;

	case 0xB: // Just returned from menu.
		GSPGPU_AcquireRight(NULL, 0x0);
		GSPGPU_RestoreVramSysArea(NULL);
		aptAppletUtility_Exit_RetToApp();
		aptSetStatus(APP_RUNNING);
		return true;

	case 0xC: // Exiting application.
		aptSetStatus(APP_EXITING);
		return false;
	}

	return true;
}

void aptEventHandler(u32 arg)
{
	bool runThread = true;

	while(runThread)
	{
		s32 syncedID = 0;
		svcWaitSynchronizationN(&syncedID, aptEvents, 2, 0, U64_MAX);
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
			// Event 2 means we should exit the thread (event will be added later).
			case 0x2:
				runThread = false;
				break;
		}
	}

	svcExitThread();
}

Result aptInit(void)
{
	Result ret=0;

	// Initialize APT stuff, escape load screen.
	srvGetServiceHandle(&aptuHandle, "APT:U");
	if((ret=APT_GetLockHandle(&aptuHandle, 0x0, &aptLockHandle)))return ret;
	svcCloseHandle(aptuHandle);

	currentAppId = __apt_appid;

	if(!(__system_runflags&RUNFLAG_APTWORKAROUND))
	{
		aptOpenSession();
		if((ret=APT_Initialize(NULL, currentAppId, &aptEvents[0], &aptEvents[1])))return ret;
		aptCloseSession();
		
		aptOpenSession();
		if((ret=APT_Enable(NULL, 0x0)))return ret;
		aptCloseSession();
		
		aptOpenSession();
		if((ret=APT_NotifyToWait(NULL, currentAppId)))return ret;
		aptCloseSession();
	}

	svcCreateEvent(&aptStatusEvent, 0);
	svcCreateEvent(&aptSleepSync, 0);
	return 0;
}

void aptExit()
{
	aptAppletUtility_Exit_RetToApp();

	// This is only executed when application-termination was triggered via the home-menu power-off screen.
	if(aptGetStatusPower() == 1)
	{
		aptOpenSession();
		APT_ReplySleepQuery(NULL, currentAppId, 0x0);
		aptCloseSession();
	}

	if(!(__system_runflags&RUNFLAG_APTWORKAROUND))
	{
		aptOpenSession();
		APT_PrepareToCloseApplication(NULL, 0x1);
		aptCloseSession();
		
		aptOpenSession();
		APT_CloseApplication(NULL, 0x0, 0x0, 0x0);
		aptCloseSession();
	}
	
	svcCloseHandle(aptSleepSync);

	svcCloseHandle(aptStatusMutex);
	//svcCloseHandle(aptLockHandle);
	svcCloseHandle(aptStatusEvent);
}

void aptSetupEventHandler()
{
	u8 buf1[4], buf2[4];

	/*buf1[0]=0x02; buf1[1]=0x00; buf1[2]=0x00; buf1[3]=0x04;
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x7, 0x4, buf1, 0x1, buf2);
	aptCloseSession();

	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
	aptCloseSession();

	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
	aptCloseSession();

	buf1[0]=0x13; buf1[1]=0x00; buf1[2]=0x10; buf1[3]=0x00;
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x7, 0x4, buf1, 0x1, buf2);
	aptCloseSession();

	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
	aptCloseSession();*/

	svcCreateMutex(&aptStatusMutex, true);
	aptStatus=0;
	svcReleaseMutex(aptStatusMutex);

	aptSetStatus(APP_RUNNING);

	if(!(__system_runflags&RUNFLAG_APTWORKAROUND))
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

		// Create thread for stuff handling APT events.
		svcCreateThread(&aptEventHandlerThread, aptEventHandler, 0x0,
			(u32*)(&aptEventHandlerStack[APT_HANDLER_STACKSIZE/8]), 0x31, 0xfffffffe);
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
	u32 prevstatus;

	svcWaitSynchronization(aptStatusMutex, U64_MAX);

	prevstatus = aptStatus;
	aptStatus = status;

	if(prevstatus != APP_NOTINITIALIZED)
	{
		if(status == APP_RUNNING || status == APP_EXITING)
			svcSignalEvent(aptStatusEvent);
	}

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
	svcWaitSynchronization(aptLockHandle, U64_MAX);
	srvGetServiceHandle(&aptuHandle, "APT:U");
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
