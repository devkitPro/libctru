#pragma once

// TODO : find a better place to put this
#define RUNFLAG_APTWORKAROUND (BIT(0))

typedef enum{
	APPID_HOMEMENU = 0x101, // Home Menu
	APPID_CAMERA = 0x110, // Camera applet
	APPID_WEB = 0x114, // Internet Browser
	APPID_APPLICATION = 0x300, // Application
}NS_APPID; // cf http://3dbrew.org/wiki/NS#AppIDs

typedef enum{
	APP_NOTINITIALIZED,
	APP_RUNNING,
	APP_SUSPENDED,
	APP_EXITING,
	APP_SUSPENDING,
	APP_SLEEPMODE,
	APP_PREPARE_SLEEPMODE,
	APP_APPLETSTARTED,
	APP_APPLETCLOSED
}APP_STATUS;

enum {
	APTSIGNAL_HOMEBUTTON   = 1,
	// 2: sleep-mode related?
	APTSIGNAL_PREPARESLEEP = 3,
	// 4: triggered when ptm:s GetShellStatus() returns 5.
	APTSIGNAL_ENTERSLEEP   = 5,
	APTSIGNAL_WAKEUP       = 6,
	APTSIGNAL_ENABLE       = 7,
	APTSIGNAL_POWERBUTTON  = 8,
	APTSIGNAL_UTILITY      = 9,
	APTSIGNAL_SLEEPSYSTEM  = 10,
	APTSIGNAL_ERROR        = 11
};


extern Handle aptEvents[3];

Result aptInit();
void aptExit();
void aptOpenSession();
void aptCloseSession();
void aptSetStatus(APP_STATUS status);
APP_STATUS aptGetStatus();
u32 aptGetStatusPower();//This can be used when the status is APP_SUSPEND* to check how the return-to-menu was triggered: 0 = home-button, 1 = power-button.
void aptSetStatusPower(u32 status);
void aptReturnToMenu();//This should be called by the user application when aptGetStatus() returns APP_SUSPENDING, not calling this will result in return-to-menu being disabled with the status left at APP_SUSPENDING. This function will not return until the system returns to the application, or when the status was changed to APP_EXITING.
void aptWaitStatusEvent();
void aptSignalReadyForSleep();
NS_APPID aptGetMenuAppID();
bool aptMainLoop(); // Use like this in your main(): while (aptMainLoop()) { your code here... }

Result APT_GetLockHandle(Handle* handle, u16 flags, Handle* lockHandle);
Result APT_Initialize(Handle* handle, NS_APPID appId, Handle* eventHandle1, Handle* eventHandle2);
Result APT_HardwareResetAsync(Handle* handle);
Result APT_Enable(Handle* handle, u32 a);
Result APT_GetAppletManInfo(Handle* handle, u8 inval, u8 *outval8, u32 *outval32, NS_APPID *menu_appid, NS_APPID *active_appid);
Result APT_PrepareToJumpToHomeMenu(Handle* handle);
Result APT_JumpToHomeMenu(Handle* handle, u32 a, u32 b, u32 c);
Result APT_IsRegistered(Handle* handle, NS_APPID appID, u8* out);
Result APT_InquireNotification(Handle* handle, u32 appID, u8* signalType);
Result APT_NotifyToWait(Handle* handle, NS_APPID appID);
Result APT_AppletUtility(Handle* handle, u32* out, u32 a, u32 size1, u8* buf1, u32 size2, u8* buf2);
Result APT_GlanceParameter(Handle* handle, NS_APPID appID, u32 bufferSize, u32* buffer, u32* actualSize, u8* signalType);
Result APT_ReceiveParameter(Handle* handle, NS_APPID appID, u32 bufferSize, u32* buffer, u32* actualSize, u8* signalType);
Result APT_SendParameter(Handle* handle, NS_APPID src_appID, NS_APPID dst_appID, u32 bufferSize, u32* buffer, Handle paramhandle, u8 signalType);
Result APT_SendCaptureBufferInfo(Handle* handle, u32 bufferSize, u32* buffer);
Result APT_ReplySleepQuery(Handle* handle, NS_APPID appID, u32 a);
Result APT_ReplySleepNotificationComplete(Handle* handle, NS_APPID appID);
Result APT_PrepareToCloseApplication(Handle* handle, u8 a);
Result APT_CloseApplication(Handle* handle, u32 a, u32 b, u32 c);
Result APT_SetAppCpuTimeLimit(Handle* handle, u32 percent);
Result APT_GetAppCpuTimeLimit(Handle* handle, u32 *percent);
Result APT_CheckNew3DS_Application(Handle* handle, u8 *out);//*Application and *System use APT commands 0x01010000 and 0x01020000. Using APT_CheckNew3DS() is recommended, this determines which of those two funcs to use automatically. When this is first called(this calls aptOpenSession/aptCloseSession internally), this initializes an internal flag, which is then used for the out val for all future calls.
Result APT_CheckNew3DS_System(Handle* handle, u8 *out);
Result APT_CheckNew3DS(Handle* handle, u8 *out);
Result APT_PrepareToDoAppJump(Handle* handle, u8 flags, u64 programID, u8 mediatype);
Result APT_DoAppJump(Handle* handle, u32 NSbuf0Size, u32 NSbuf1Size, u8 *NSbuf0Ptr, u8 *NSbuf1Ptr);
Result APT_PrepareToStartLibraryApplet(Handle* handle, NS_APPID appID);
Result APT_StartLibraryApplet(Handle* handle, NS_APPID appID, Handle inhandle, u32 *parambuf, u32 parambufsize);
Result APT_LaunchLibraryApplet(NS_APPID appID, Handle inhandle, u32 *parambuf, u32 parambufsize);//This should be used for launching library applets, this uses the above APT_StartLibraryApplet/APT_PrepareToStartLibraryApplet funcs + apt*Session(). parambuf is used for APT params input, when the applet closes the output param block is copied here. This is not usable from the homebrew launcher. This is broken: when the applet does get launched at all, the applet process doesn't actually get terminated when the applet gets closed.
Result APT_PrepareToStartSystemApplet(Handle* handle, NS_APPID appID);
Result APT_StartSystemApplet(Handle* handle, NS_APPID appID, u32 bufSize, Handle applHandle, u8 *buf);

