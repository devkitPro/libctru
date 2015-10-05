/**
 * @file apt.h
 * @brief APT service.
 */
#pragma once

// TODO : find a better place to put this
#define RUNFLAG_APTWORKAROUND (BIT(0))
#define RUNFLAG_APTREINIT (BIT(1))

/**
 * @brief NS Application IDs.
 *
 * Retrieved from http://3dbrew.org/wiki/NS_and_APT_Services#AppIDs
 */
typedef enum{
	APPID_HOMEMENU = 0x101,           ///< Home Menu
	APPID_CAMERA = 0x110,             ///< Camera applet
	APPID_FRIENDS_LIST = 0x112,       ///< Friends List applet
	APPID_GAME_NOTES = 0x113,         ///< Game Notes applet
	APPID_WEB = 0x114,                ///< Internet Browser
	APPID_INSTRUCTION_MANUAL = 0x115, ///< Instruction Manual applet
	APPID_NOTIFICATIONS = 0x116,      ///< Notifications applet
	APPID_MIIVERSE = 0x117,           ///< Miiverse applet (olv)
	APPID_MIIVERSE_POSTING = 0x118,   ///< Miiverse posting applet (solv3)
	APPID_AMIIBO_SETTINGS = 0x119,    ///< Amiibo settings applet (cabinet)
	APPID_APPLICATION = 0x300,        ///< Application
	APPID_ESHOP = 0x301,              ///< eShop (tiger)
	APPID_SOFTWARE_KEYBOARD = 0x401,  ///< Software Keyboard
	APPID_APPLETED = 0x402,           ///< appletEd
	APPID_PNOTE_AP = 0x404,           ///< PNOTE_AP
	APPID_SNOTE_AP = 0x405,           ///< SNOTE_AP
	APPID_ERROR = 0x406,              ///< error
	APPID_MINT = 0x407,               ///< mint
	APPID_EXTRAPAD = 0x408,           ///< extrapad
	APPID_MEMOLIB = 0x409,            ///< memolib
}NS_APPID;

/// App status values.
typedef enum{
	APP_NOTINITIALIZED,    ///< App not initialized.
	APP_RUNNING,           ///< App running.
	APP_SUSPENDED,         ///< App suspended.
	APP_EXITING,           ///< App exiting.
	APP_SUSPENDING,        ///< App suspending.
	APP_SLEEPMODE,         ///< App in sleep mode.
	APP_PREPARE_SLEEPMODE, ///< App preparing to enter sleep mode.
	APP_APPLETSTARTED,     ///< Applet started.
	APP_APPLETCLOSED       ///< Applet closed.
}APP_STATUS;

/// APT signals.
enum {
	APTSIGNAL_HOMEBUTTON   = 1,  ///< Home button pressed.
	// 2: sleep-mode related?
	APTSIGNAL_PREPARESLEEP = 3,  ///< Prepare to enter sleep mode.
	// 4: triggered when ptm:s GetShellStatus() returns 5.
	APTSIGNAL_ENTERSLEEP   = 5,  ///< Enter sleep mode.
	APTSIGNAL_WAKEUP       = 6,  ///< Wake from sleep mode.
	APTSIGNAL_ENABLE       = 7,  ///< Enable.
	APTSIGNAL_POWERBUTTON  = 8,  ///< Power button pressed.
	APTSIGNAL_UTILITY      = 9,  ///< Utility called.
	APTSIGNAL_SLEEPSYSTEM  = 10, ///< System sleeping.
	APTSIGNAL_ERROR        = 11  ///< Error occurred.
} APT_Signal;

/// APT hook types.
enum {
	APTHOOK_ONSUSPEND = 0, ///< App suspended.
	APTHOOK_ONRESTORE,     ///< App restored.
	APTHOOK_ONSLEEP,       ///< App sleeping.
	APTHOOK_ONWAKEUP,      ///< App waking up.
	APTHOOK_ONEXIT,        ///< App exiting.

	APTHOOK_COUNT,         ///< Number of APT hook types.
} APT_HookType;

/// APT hook function.
typedef void (*aptHookFn)(int hook, void* param);

/// APT hook cookie.
typedef struct tag_aptHookCookie
{
	struct tag_aptHookCookie* next; ///< Next cookie.
	aptHookFn callback;             ///< Hook callback.
	void* param;                    ///< Callback parameter.
} aptHookCookie;

/// APT events.
extern Handle aptEvents[3];

/// Initializes APT.
Result aptInit(void);

/// Exits APT.
void aptExit(void);

/// Opens an APT session.
void aptOpenSession(void);

/// Closes an APT session.
void aptCloseSession(void);

/**
 * @brief Sets the app's status.
 * @param status Status to set.
 */
void aptSetStatus(APP_STATUS status);

/**
 * @brief Gets the app's status.
 * @return The app's status.
 */
APP_STATUS aptGetStatus(void);

/**
 * @brief Gets the app's power status.
 * When the status is APT_SUSPEND, this can be used to check what triggered a return-to-menu.
 * @return The app's power status. (0 = normal, 1 = power button pressed)
 */
u32 aptGetStatusPower(void);

/**
 * @brief Sets the app's power status.
 * @param status Power status to set.
 */
void aptSetStatusPower(u32 status);

/**
 * @brief Triggers a return to the home menu.
 *
 * This should be called by the user application when aptGetStatus() returns APP_SUSPENDING, not calling this will result in return-to-menu being disabled with the status left at APP_SUSPENDING. This function will not return until the system returns to the application, or when the status was changed to APP_EXITING.
 */
void aptReturnToMenu(void);

/// Waits for an APT status event.
void aptWaitStatusEvent(void);

/// Signals that the app is ready to sleep.
void aptSignalReadyForSleep(void);

/**
 * @brief Gets the menu's app ID.
 * @return The menu's app ID.
 */
NS_APPID aptGetMenuAppID(void);

/**
 * @brief Processes the current APT status. Generally used within a main loop.
 * @return Whether the application is closing.
 */
bool aptMainLoop(void);

/**
 * @brief Sets up an APT status hook.
 * @param cookie Hook cookie to use.
 * @param callback Function to call when APT's status changes.
 * @param param User-defined parameter to pass to the callback.
 */
void aptHook(aptHookCookie* cookie, aptHookFn callback, void* param);

/**
 * @brief Removes an APT status hook.
 * @param cookie Hook cookie to remove.
 */
void aptUnhook(aptHookCookie* cookie);

Result APT_GetLockHandle(u16 flags, Handle* lockHandle);
Result APT_Initialize(NS_APPID appId, Handle* eventHandle1, Handle* eventHandle2);
Result APT_Finalize(NS_APPID appId);
Result APT_HardwareResetAsync(void);
Result APT_Enable(u32 a);
Result APT_GetAppletManInfo(u8 inval, u8 *outval8, u32 *outval32, NS_APPID *menu_appid, NS_APPID *active_appid);
Result APT_GetAppletInfo(NS_APPID appID, u64* pProgramID, u8* pMediaType, u8* pRegistered, u8* pLoadState, u32* pAttributes);
Result APT_GetAppletProgramInfo(u32 id, u32 flags, u16 *titleversion);
Result APT_GetProgramID(u64* pProgramID);
Result APT_PrepareToJumpToHomeMenu(void);
Result APT_JumpToHomeMenu(const u8 *param, size_t paramSize, Handle handle);
Result APT_PrepareToJumpToApplication(u32 a);
Result APT_JumpToApplication(const u8 *param, size_t paramSize, Handle handle);
Result APT_IsRegistered(NS_APPID appID, u8* out);
Result APT_InquireNotification(u32 appID, u8* signalType);
Result APT_NotifyToWait(NS_APPID appID);
Result APT_AppletUtility(u32* out, u32 a, u32 size1, u8* buf1, u32 size2, u8* buf2);
Result APT_GlanceParameter(NS_APPID appID, u32 bufferSize, u32* buffer, u32* actualSize, u8* signalType);
Result APT_ReceiveParameter(NS_APPID appID, u32 bufferSize, u32* buffer, u32* actualSize, u8* signalType);
Result APT_SendParameter(NS_APPID src_appID, NS_APPID dst_appID, u32 bufferSize, u32* buffer, Handle paramhandle, u8 signalType);
Result APT_SendCaptureBufferInfo(u32 bufferSize, u32* buffer);
Result APT_ReplySleepQuery(NS_APPID appID, u32 a);
Result APT_ReplySleepNotificationComplete(NS_APPID appID);

/**
 * @brief Prepares to close the application.
 * @param a Whether the jump is to the home menu.
 */
Result APT_PrepareToCloseApplication(u8 a);

/**
 * @brief Closes the application.
 * @param param Parameter to use.
 * @param paramSize Size of param.
 * @param handle Handle to use.
 */
Result APT_CloseApplication(const u8 *param, size_t paramSize, Handle handle);

/**
 * @brief Sets the application's CPU time limit.
 * @param percent CPU time limit percentage to set.
 */
Result APT_SetAppCpuTimeLimit(u32 percent);

/**
 * @brief Gets the application's CPU time limit.
 * @param percent Pointer to output the CPU time limit percentage to.
 */
Result APT_GetAppCpuTimeLimit(u32 *percent);

/**
 * @brief Checks whether the system is a New 3DS.
 * Note: this function is unreliable, see: http://3dbrew.org/wiki/APT:PrepareToStartApplication
 * @param out Pointer to write the New 3DS flag to.
 */
Result APT_CheckNew3DS_Application(u8 *out);

/**
 * @brief Checks whether the system is a New 3DS.
 * @param out Pointer to write the New 3DS flag to.
 */
Result APT_CheckNew3DS_System(u8 *out);

/**
 * @brief Checks whether the system is a New 3DS.
 * @param out Pointer to write the New 3DS flag to.
 */
Result APT_CheckNew3DS(u8 *out);

/**
 * @brief Prepares for an applicaton jump.
 * @param flags Flags to use.
 * @param programID ID of the program to jump to.
 * @param mediatype Media type of the program to jump to.
 */
Result APT_PrepareToDoAppJump(u8 flags, u64 programID, u8 mediatype);

/**
 * @brief Performs an application jump.
 * @param NSbuf0Size Size of NSbuf0Ptr.
 * @param NSbuf1Size Size of NSbuf1Ptr.
 * @param NSbuf0Ptr Launch buffer 0.
 * @param NSbuf1Ptr Launch buffer 1.
 */
Result APT_DoAppJump(u32 NSbuf0Size, u32 NSbuf1Size, u8 *NSbuf0Ptr, u8 *NSbuf1Ptr);

/**
 * @brief Prepares to start a library applet.
 * @param appID ID of the applet to start.
 */
Result APT_PrepareToStartLibraryApplet(NS_APPID appID);

/**
 * @brief Starts a library applet.
 * @param appID ID of the applet to launch.
 * @param inhandle Handle to pass to the applet.
 * @param parambuf Buffer containing applet parameters.
 * @param parambufsize Size of parambuf.
 */
Result APT_StartLibraryApplet(NS_APPID appID, Handle inhandle, u32 *parambuf, u32 parambufsize);

/**
 * @brief Launches a library applet.
 * Note: This is not usable from the homebrew launcher. This is broken: when the applet does get launched at all, the applet process doesn't actually get terminated when the applet gets closed.
 * @param appID ID of the applet to launch.
 * @param inhandle Handle to pass to the applet.
 * @param parambuf Buffer containing applet parameters.
 * @param parambufsize Size of parambuf.
 */
Result APT_LaunchLibraryApplet(NS_APPID appID, Handle inhandle, u32 *parambuf, u32 parambufsize);

/**
 * @brief Prepares to start a system applet.
 * @param appID ID of the applet to start.
 */
Result APT_PrepareToStartSystemApplet(NS_APPID appID);

/**
 * @brief Starts a system applet.
 * @param appID ID of the applet to launch.
 * @param bufSize Size of the parameter buffer.
 * @param applHandle Handle to pass to the applet.
 * @param buf Buffer containing applet parameters.
 */
Result APT_StartSystemApplet(NS_APPID appID, u32 bufSize, Handle applHandle, u8 *buf);

