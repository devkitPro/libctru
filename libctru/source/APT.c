#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctr/types.h>
#include <ctr/srv.h>
#include <ctr/APT.h>
#include <ctr/GSP.h>
#include <ctr/svc.h>

#define APT_HANDLER_STACKSIZE (0x1000)

NS_APPID currentAppId;

Handle aptLockHandle;
Handle aptuHandle;
Handle aptEvents[3];

Handle aptEventHandlerThread;
u64 aptEventHandlerStack[APT_HANDLER_STACKSIZE/8]; //u64 so that it's 8-byte aligned

Handle aptStatusMutex;
u32 aptStatus;

u32 aptParameters[0x1000/4]; //TEMP

void aptEventHandler(u32 arg)
{
	bool runThread=true;
	while(runThread)
	{
		s32 syncedID=0x0;
		svc_waitSynchronizationN(&syncedID, aptEvents, 2, 0, U64_MAX);
		svc_clearEvent(aptEvents[syncedID]);
	
		switch(syncedID)
		{
			case 0x0: //event 0 means we got a signal from NS (home button, power button etc)
				{
					u8 signalType;

					aptOpenSession();
					APT_InquireNotification(NULL, currentAppId, &signalType); //check signal type
					aptCloseSession();
	
					switch(signalType)
					{
						case 0x1: //home menu button got pressed
							aptOpenSession();
							APT_PrepareToJumpToHomeMenu(NULL); //prepare for return to menu
							aptCloseSession();

							aptSetStatus(APP_SUSPENDED);
			
							GSPGPU_ReleaseRight(NULL); //disable GSP module access
			
							aptOpenSession();
							APT_JumpToHomeMenu(NULL, 0x0, 0x0, 0x0); //jump !
							aptCloseSession();
							break;
					}
				}
				break;
			case 0x1: //event 1 means app just started, we're returning to app, exiting app etc.
				{
					u8 signalType;
					aptOpenSession();
					APT_ReceiveParameter(NULL, currentAppId, 0x1000, aptParameters, NULL, &signalType);
					aptCloseSession();
	
					switch(signalType)
					{
						case 0x1: //application just started
							break;
						case 0xB: //just returned from menu
							GSPGPU_AcquireRight(NULL, 0x0);
							aptSetStatus(APP_RUNNING);
							break;
						case 0xC: //exiting application
							aptOpenSession();
							APT_ReplySleepQuery(NULL, currentAppId, 0x0);
							aptCloseSession();

							runThread=false;
							aptSetStatus(APP_EXITING); //app exit signal
							break;
					}
				}
				break;
			case 0x2: //event 2 means we should exit the thread (event will be added later)
				runThread=false;
				break;
		}
	}
	svc_exitThread();
}

Result aptInit(NS_APPID appID)
{
	Result ret=0;

	//initialize APT stuff, escape load screen
	srv_getServiceHandle(NULL, &aptuHandle, "APT:U");
	if((ret=APT_GetLockHandle(&aptuHandle, 0x0, &aptLockHandle)))return ret;
	svc_closeHandle(aptuHandle);

	currentAppId=appID;

	aptOpenSession();
	if((ret=APT_Initialize(NULL, currentAppId, &aptEvents[0], &aptEvents[1])))return ret;
	aptCloseSession();
	
	aptOpenSession();
	if((ret=APT_Enable(NULL, 0x0)))return ret;
	aptCloseSession();
	
	aptOpenSession();
	if((ret=APT_NotifyToWait(NULL, currentAppId)))return ret;
	aptCloseSession();
	
	return 0;
}

void aptExit()
{
	u8 buf1[4], buf2[4];

	buf1[0]=0x02; buf1[1]=0x00; buf1[2]=0x00; buf1[3]=0x00;
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x7, 0x4, buf1, 0x1, buf2);
	aptCloseSession();
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
	aptCloseSession();

	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x7, 0x4, buf1, 0x1, buf2);
	aptCloseSession();
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
	aptCloseSession();
	aptOpenSession();
	APT_AppletUtility(NULL, NULL, 0x4, 0x1, buf1, 0x1, buf2);
	aptCloseSession();


	aptOpenSession();
	APT_PrepareToCloseApplication(NULL, 0x1);
	aptCloseSession();
	
	aptOpenSession();
	APT_CloseApplication(NULL, 0x0, 0x0, 0x0);
	aptCloseSession();

	svc_closeHandle(aptStatusMutex);
	// svc_closeHandle(aptLockHandle);
}

void aptSetupEventHandler()
{
	u8 buf1[4], buf2[4];

	buf1[0]=0x02; buf1[1]=0x00; buf1[2]=0x00; buf1[3]=0x04;
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
	aptCloseSession();

	svc_createMutex(&aptStatusMutex, true);
	aptStatus=0;
	svc_releaseMutex(aptStatusMutex);

	//create thread for stuff handling APT events
	svc_createThread(&aptEventHandlerThread, aptEventHandler, 0x0, (u32*)(&aptEventHandlerStack[APT_HANDLER_STACKSIZE/8]), 0x31, 0xfffffffe);
}

APP_STATUS aptGetStatus()
{
	APP_STATUS ret;
	svc_waitSynchronization1(aptStatusMutex, U64_MAX);
	ret=aptStatus;
	svc_releaseMutex(aptStatusMutex);
	return ret;
}

void aptSetStatus(APP_STATUS status)
{
	svc_waitSynchronization1(aptStatusMutex, U64_MAX);
	aptStatus=status;
	svc_releaseMutex(aptStatusMutex);
}

void aptOpenSession()
{
	svc_waitSynchronization1(aptLockHandle, U64_MAX);
	srv_getServiceHandle(NULL, &aptuHandle, "APT:U");
}

void aptCloseSession()
{
	svc_closeHandle(aptuHandle);
	svc_releaseMutex(aptLockHandle);
}

Result APT_GetLockHandle(Handle* handle, u16 flags, Handle* lockHandle)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x10040; //request header code
	cmdbuf[1]=flags;
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;
	
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
	if((ret=svc_sendSyncRequest(*handle)))return ret;
	
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
	if((ret=svc_sendSyncRequest(*handle)))return ret;
	
	return cmdbuf[1];
}

Result APT_InquireNotification(Handle* handle, u32 appID, u8* signalType)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0xB0040; //request header code
	cmdbuf[1]=appID;
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	if(signalType)*signalType=cmdbuf[2];
	
	return cmdbuf[1];
}

Result APT_PrepareToJumpToHomeMenu(Handle* handle)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x2b0000; //request header code
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;
	
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
	if((ret=svc_sendSyncRequest(*handle)))return ret;
	
	return cmdbuf[1];
}

Result APT_NotifyToWait(Handle* handle, NS_APPID appID)
{
	if(!handle)handle=&aptuHandle;
	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x430040; //request header code
	cmdbuf[1]=appID;
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

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
	if((ret=svc_sendSyncRequest(*handle)))return ret;

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
	if((ret=svc_sendSyncRequest(*handle)))return ret;

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
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	if(signalType)*signalType=cmdbuf[3];
	if(actualSize)*actualSize=cmdbuf[4];

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
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}

Result APT_PrepareToCloseApplication(Handle* handle, u8 a)
{
	if(!handle)handle=&aptuHandle;

	u32* cmdbuf=getThreadCommandBuffer();
	cmdbuf[0]=0x220040; //request header code
	cmdbuf[1]=a;
	
	Result ret=0;
	if((ret=svc_sendSyncRequest(*handle)))return ret;

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
	if((ret=svc_sendSyncRequest(*handle)))return ret;

	return cmdbuf[1];
}
