#ifndef APT_H
#define APT_H

typedef enum{
	APPID_HOMEMENU = 0x101, // Home Menu
	APPID_CAMERA = 0x110, // Camera applet
	APPID_WEB = 0x114, // Internet Browser
	APPID_APPLICATION = 0x300, // Application
}NS_APPID; // cf http://3dbrew.org/wiki/NS#AppIDs

Result APT_GetLockHandle(Handle handle, u16 flags, Handle* lockHandle);
Result APT_Initialize(Handle handle, NS_APPID appId, Handle* eventHandle1, Handle* eventHandle2);
Result APT_Enable(Handle handle, u32 a);
Result APT_PrepareToJumpToHomeMenu(Handle handle);
Result APT_JumpToHomeMenu(Handle handle, u32 a, u32 b, u32 c);
Result APT_InquireNotification(Handle handle, u32 appID, u8* signalType);
Result APT_NotifyToWait(Handle handle, u32 a);

#endif
