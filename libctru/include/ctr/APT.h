#ifndef APT_H
#define APT_H

typedef enum{
	APPID_HOMEMENU = 0x101, // Home Menu
	APPID_CAMERA = 0x110, // Camera applet
	APPID_WEB = 0x114, // Internet Browser
	APPID_APPLICATION = 0x300, // Application
}NS_APPID; // cf http://3dbrew.org/wiki/NS#AppIDs

typedef enum{
	APP_RUNNING,
	APP_SUSPENDED,
	APP_EXITING
}APP_STATUS;

extern Handle aptEvents[3];

Result aptInit(NS_APPID appID);
void aptExit();
void aptOpenSession();
void aptCloseSession();
void aptSetupEventHandler();
void aptSetStatus(APP_STATUS status);
APP_STATUS aptGetStatus();

Result APT_GetLockHandle(Handle* handle, u16 flags, Handle* lockHandle);
Result APT_Initialize(Handle* handle, NS_APPID appId, Handle* eventHandle1, Handle* eventHandle2);
Result APT_Enable(Handle* handle, u32 a);
Result APT_PrepareToJumpToHomeMenu(Handle* handle);
Result APT_JumpToHomeMenu(Handle* handle, u32 a, u32 b, u32 c);
Result APT_InquireNotification(Handle* handle, u32 appID, u8* signalType);
Result APT_NotifyToWait(Handle* handle, NS_APPID appID);
Result APT_AppletUtility(Handle* handle, u32* out, u32 a, u32 size1, u8* buf1, u32 size2, u8* buf2);
Result APT_GlanceParameter(Handle* handle, NS_APPID appID, u32 bufferSize, u32* buffer, u32* actualSize, u8* signalType);
Result APT_ReceiveParameter(Handle* handle, NS_APPID appID, u32 bufferSize, u32* buffer, u32* actualSize, u8* signalType);
Result APT_ReplySleepQuery(Handle* handle, NS_APPID appID, u32 a);
Result APT_PrepareToCloseApplication(Handle* handle, u8 a);
Result APT_CloseApplication(Handle* handle, u32 a, u32 b, u32 c);

#endif
