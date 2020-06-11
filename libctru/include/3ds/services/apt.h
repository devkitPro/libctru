/**
 * @file apt.h
 * @brief APT (Applet) service.
 */
#pragma once

/**
 * @brief NS Application IDs.
 *
 * Retrieved from http://3dbrew.org/wiki/NS_and_APT_Services#AppIDs
 */
typedef enum {
	APPID_NONE = 0,
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
} NS_APPID;

/// APT applet position.
typedef enum {
	APTPOS_NONE     = -1, ///< No position specified.
	APTPOS_APP      = 0,  ///< Application.
	APTPOS_APPLIB   = 1,  ///< Application library (?).
	APTPOS_SYS      = 2,  ///< System applet.
	APTPOS_SYSLIB   = 3,  ///< System library (?).
	APTPOS_RESIDENT = 4,  ///< Resident applet.
} APT_AppletPos;

typedef u8 APT_AppletAttr;

struct PtmWakeEvents;

/// Create an APT_AppletAttr bitfield from its components.
static inline APT_AppletAttr aptMakeAppletAttr(APT_AppletPos pos, bool manualGpuRights, bool manualDspRights)
{
	return (pos&7) | (manualGpuRights ? BIT(3) : 0) | (manualDspRights ? BIT(4) : 0);
}

/// APT query reply.
typedef enum {
	APTREPLY_REJECT = 0,
	APTREPLY_ACCEPT = 1,
	APTREPLY_LATER  = 2,
} APT_QueryReply;

/// APT signals.
typedef enum {
	APTSIGNAL_NONE         = 0,  ///< No signal received.
	APTSIGNAL_HOMEBUTTON   = 1,  ///< HOME button pressed.
	APTSIGNAL_HOMEBUTTON2  = 2,  ///< HOME button pressed (again?).
	APTSIGNAL_SLEEP_QUERY  = 3,  ///< Prepare to enter sleep mode.
	APTSIGNAL_SLEEP_CANCEL = 4,  ///< Triggered when ptm:s GetShellStatus() returns 5.
	APTSIGNAL_SLEEP_ENTER  = 5,  ///< Enter sleep mode.
	APTSIGNAL_SLEEP_WAKEUP = 6,  ///< Wake from sleep mode.
	APTSIGNAL_SHUTDOWN     = 7,  ///< Shutdown.
	APTSIGNAL_POWERBUTTON  = 8,  ///< POWER button pressed.
	APTSIGNAL_POWERBUTTON2 = 9,  ///< POWER button cleared (?).
	APTSIGNAL_TRY_SLEEP    = 10, ///< System sleeping (?).
	APTSIGNAL_ORDERTOCLOSE = 11, ///< Order to close (such as when an error happens?).
} APT_Signal;

/// APT commands.
typedef enum {
	APTCMD_NONE               = 0,  ///< No command received.
	APTCMD_WAKEUP             = 1,  ///< Applet should wake up.
	APTCMD_REQUEST            = 2,  ///< Source applet sent us a parameter.
	APTCMD_RESPONSE           = 3,  ///< Target applet replied to our parameter.
	APTCMD_EXIT               = 4,  ///< Exit (??)
	APTCMD_MESSAGE            = 5,  ///< Message (??)
	APTCMD_HOMEBUTTON_ONCE    = 6,  ///< HOME button pressed once.
	APTCMD_HOMEBUTTON_TWICE   = 7,  ///< HOME button pressed twice (double-pressed).
	APTCMD_DSP_SLEEP          = 8,  ///< DSP should sleep (manual DSP rights related?).
	APTCMD_DSP_WAKEUP         = 9,  ///< DSP should wake up (manual DSP rights related?).
	APTCMD_WAKEUP_EXIT        = 10, ///< Applet wakes up due to a different applet exiting.
	APTCMD_WAKEUP_PAUSE       = 11, ///< Applet wakes up after being paused through HOME menu.
	APTCMD_WAKEUP_CANCEL      = 12, ///< Applet wakes up due to being cancelled.
	APTCMD_WAKEUP_CANCELALL   = 13, ///< Applet wakes up due to all applets being cancelled.
	APTCMD_WAKEUP_POWERBUTTON = 14, ///< Applet wakes up due to POWER button being pressed (?).
	APTCMD_WAKEUP_JUMPTOHOME  = 15, ///< Applet wakes up and is instructed to jump to HOME menu (?).
	APTCMD_SYSAPPLET_REQUEST  = 16, ///< Request for sysapplet (?).
	APTCMD_WAKEUP_LAUNCHAPP   = 17, ///< Applet wakes up and is instructed to launch another applet (?).
} APT_Command;

/// APT capture buffer information.
typedef struct
{
	u32 size;
	u32 is3D;
	struct
	{
		u32 leftOffset;
		u32 rightOffset;
		u32 format;
	} top, bottom;
} aptCaptureBufInfo;

/// APT hook types.
typedef enum {
	APTHOOK_ONSUSPEND = 0, ///< App suspended.
	APTHOOK_ONRESTORE,     ///< App restored.
	APTHOOK_ONSLEEP,       ///< App sleeping.
	APTHOOK_ONWAKEUP,      ///< App waking up.
	APTHOOK_ONEXIT,        ///< App exiting.

	APTHOOK_COUNT,         ///< Number of APT hook types.
} APT_HookType;

/// APT hook function.
typedef void (*aptHookFn)(APT_HookType hook, void* param);

/// APT hook cookie.
typedef struct tag_aptHookCookie
{
	struct tag_aptHookCookie* next; ///< Next cookie.
	aptHookFn callback;             ///< Hook callback.
	void* param;                    ///< Callback parameter.
} aptHookCookie;

/// APT message callback.
typedef void (*aptMessageCb)(void* user, NS_APPID sender, void* msg, size_t msgsize);

/// Initializes APT.
Result aptInit(void);

/// Exits APT.
void aptExit(void);

/**
 * @brief Sends an APT command through IPC, taking care of locking, opening and closing an APT session.
 * @param aptcmdbuf Pointer to command buffer (should have capacity for at least 16 words).
 */
Result aptSendCommand(u32* aptcmdbuf);

/// Returns true if the application is currently in the foreground.
bool aptIsActive(void);

/// Returns true if the system has told the application to close.
bool aptShouldClose(void);

/// Returns true if the system can enter sleep mode while the application is active.
bool aptIsSleepAllowed(void);

/// Configures whether the system can enter sleep mode while the application is active.
void aptSetSleepAllowed(bool allowed);

/// Handles incoming sleep mode requests.
void aptHandleSleep(void);

/// Returns true if the user can press the HOME button to jump back to the HOME menu while the application is active.
bool aptIsHomeAllowed(void);

/// Configures whether the user can press the HOME button to jump back to the HOME menu while the application is active.
void aptSetHomeAllowed(bool allowed);

/// Returns true if the system requires the application to jump back to the HOME menu.
bool aptShouldJumpToHome(void);

/// Returns true if there is an incoming HOME button press rejected by the policy set by \ref aptSetHomeAllowed (use this to show a "no HOME allowed" icon).
bool aptCheckHomePressRejected(void);

/// \deprecated Alias for \ref aptCheckHomePressRejected.
static inline DEPRECATED bool aptIsHomePressed(void)
{
	return aptCheckHomePressRejected();
}

/// Jumps back to the HOME menu.
void aptJumpToHomeMenu(void);

/// Handles incoming jump-to-HOME requests.
static inline void aptHandleJumpToHome(void)
{
	if (aptShouldJumpToHome())
		aptJumpToHomeMenu();
}

/**
 * @brief Main function which handles sleep mode and HOME/power buttons - call this at the beginning of every frame.
 * @return true if the application should keep running, false otherwise (see \ref aptShouldClose).
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

/**
 * @brief Sets the function to be called when an APT message from another applet is received.
 * @param callback Callback function.
 * @param user User-defined data to be passed to the callback.
 */
void aptSetMessageCallback(aptMessageCb callback, void* user);

/**
 * @brief Launches a library applet.
 * @param appId ID of the applet to launch.
 * @param buf Input/output buffer that contains launch parameters on entry and result data on exit.
 * @param bufsize Size of the buffer.
 * @param handle Handle to pass to the library applet.
 */
void aptLaunchLibraryApplet(NS_APPID appId, void* buf, size_t bufsize, Handle handle);

/// Clears the chainloader state.
void aptClearChainloader(void);

/**
 * @brief Configures the chainloader to launch a specific application.
 * @param programID ID of the program to chainload to.
 * @param mediatype Media type of the program to chainload to.
 */
void aptSetChainloader(u64 programID, u8 mediatype);

/// Configures the chainloader to relaunch the current application (i.e. soft-reset)
void aptSetChainloaderToSelf(void);

/**
 * @brief Gets an APT lock handle.
 * @param flags Flags to use.
 * @param lockHandle Pointer to output the lock handle to.
 */
Result APT_GetLockHandle(u16 flags, Handle* lockHandle);

/**
 * @brief Initializes an application's registration with APT.
 * @param appId ID of the application.
 * @param attr Attributes of the application.
 * @param signalEvent Pointer to output the signal event handle to.
 * @param resumeEvent Pointer to output the resume event handle to.
 */
Result APT_Initialize(NS_APPID appId, APT_AppletAttr attr, Handle* signalEvent, Handle* resumeEvent);

/**
 * @brief Terminates an application's registration with APT.
 * @param appID ID of the application.
 */
Result APT_Finalize(NS_APPID appId);

/// Asynchronously resets the hardware.
Result APT_HardwareResetAsync(void);

/**
 * @brief Enables APT.
 * @param attr Attributes of the application.
 */
Result APT_Enable(APT_AppletAttr attr);

/**
 * @brief Gets applet management info.
 * @param inpos Requested applet position.
 * @param outpos Pointer to output the position of the current applet to.
 * @param req_appid Pointer to output the AppID of the applet at the requested position to.
 * @param menu_appid Pointer to output the HOME menu AppID to.
 * @param active_appid Pointer to output the AppID of the currently active applet to.
 */
Result APT_GetAppletManInfo(APT_AppletPos inpos, APT_AppletPos* outpos, NS_APPID* req_appid, NS_APPID* menu_appid, NS_APPID* active_appid);

/**
 * @brief Gets the menu's app ID.
 * @return The menu's app ID.
 */
static inline NS_APPID aptGetMenuAppID(void)
{
	NS_APPID menu_appid = APPID_NONE;
	APT_GetAppletManInfo(APTPOS_NONE, NULL, NULL, &menu_appid, NULL);
	return menu_appid;
}

/**
 * @brief Gets an applet's information.
 * @param appID AppID of the applet.
 * @param pProgramID Pointer to output the program ID to.
 * @param pMediaType Pointer to output the media type to.
 * @param pRegistered Pointer to output the registration status to.
 * @param pLoadState Pointer to output the load state to.
 * @param pAttributes Pointer to output the applet atrributes to.
 */
Result APT_GetAppletInfo(NS_APPID appID, u64* pProgramID, u8* pMediaType, bool* pRegistered, bool* pLoadState, APT_AppletAttr* pAttributes);

/**
 * @brief Gets an applet's program information.
 * @param id ID of the applet.
 * @param flags Flags to use when retreiving the information.
 * @param titleversion Pointer to output the applet's title version to.
 *
 * Flags:
 * - 0x01: Use AM_ListTitles with NAND media type.
 * - 0x02: Use AM_ListTitles with SDMC media type.
 * - 0x04: Use AM_ListTitles with GAMECARD media type.
 * - 0x10: Input ID is an app ID. Must be set if 0x20 is not.
 * - 0x20: Input ID is a program ID. Must be set if 0x10 is not.
 * - 0x100: Sets program ID high to 0x00040000, else it is 0x00040010. Only used when 0x20 is set.
 */
Result APT_GetAppletProgramInfo(u32 id, u32 flags, u16 *titleversion);

/**
 * @brief Gets the current application's program ID.
 * @param pProgramID Pointer to output the program ID to.
 */
Result APT_GetProgramID(u64* pProgramID);

/// Prepares to jump to the home menu.
Result APT_PrepareToJumpToHomeMenu(void);

/**
 * @brief Jumps to the home menu.
 * @param param Parameters to jump with.
 * @param Size of the parameter buffer.
 * @param handle Handle to pass.
 */
Result APT_JumpToHomeMenu(const void* param, size_t paramSize, Handle handle);

/**
 * @brief Prepares to jump to an application.
 * @param exiting Specifies whether the applet is exiting.
 */
Result APT_PrepareToJumpToApplication(bool exiting);

/**
 * @brief Jumps to an application.
 * @param param Parameters to jump with.
 * @param Size of the parameter buffer.
 * @param handle Handle to pass.
 */
Result APT_JumpToApplication(const void* param, size_t paramSize, Handle handle);

/**
 * @brief Gets whether an application is registered.
 * @param appID ID of the application.
 * @param out Pointer to output the registration state to.
 */
Result APT_IsRegistered(NS_APPID appID, bool* out);

/**
 * @brief Inquires as to whether a signal has been received.
 * @param appID ID of the application.
 * @param signalType Pointer to output the signal type to.
 */
Result APT_InquireNotification(u32 appID, APT_Signal* signalType);

/**
 * @brief Requests to enter sleep mode, and later sets wake events if allowed to.
 * @param wakeEvents The wake events. Limited to "shell" (bit 1) for the PDN wake events part
 * and "shell opened", "shell closed" and "HOME button pressed" for the MCU interrupts part.
 */
Result APT_SleepSystem(const struct PtmWakeEvents *wakeEvents);

/**
 * @brief Notifies an application to wait.
 * @param appID ID of the application.
 */
Result APT_NotifyToWait(NS_APPID appID);

/**
 * @brief Calls an applet utility function.
 * @param id Utility function to call.
 * @param out Pointer to write output data to.
 * @param outSize Size of the output buffer.
 * @param in Pointer to the input data.
 * @param inSize Size of the input buffer.
 */
Result APT_AppletUtility(int id, void* out, size_t outSize, const void* in, size_t inSize);

/// Sleeps if shell is closed (?).
Result APT_SleepIfShellClosed(void);

/**
 * @brief Locks a transition (?).
 * @param transition Transition ID.
 * @param flag Flag (?)
 */
Result APT_LockTransition(u32 transition, bool flag);

/**
 * @brief Tries to lock a transition (?).
 * @param transition Transition ID.
 * @param succeeded Pointer to output whether the lock was successfully applied.
 */
Result APT_TryLockTransition(u32 transition, bool* succeeded);

/**
 * @brief Unlocks a transition (?).
 * @param transition Transition ID.
 */
Result APT_UnlockTransition(u32 transition);

/**
 * @brief Glances at a receieved parameter without removing it from the queue.
 * @param appID AppID of the application.
 * @param buffer Buffer to receive to.
 * @param bufferSize Size of the buffer.
 * @param sender Pointer to output the sender's AppID to.
 * @param command Pointer to output the command ID to.
 * @param actualSize Pointer to output the actual received data size to.
 * @param parameter Pointer to output the parameter handle to.
 */
Result APT_GlanceParameter(NS_APPID appID, void* buffer, size_t bufferSize, NS_APPID* sender, APT_Command* command, size_t* actualSize, Handle* parameter);

/**
 * @brief Receives a parameter.
 * @param appID AppID of the application.
 * @param buffer Buffer to receive to.
 * @param bufferSize Size of the buffer.
 * @param sender Pointer to output the sender's AppID to.
 * @param command Pointer to output the command ID to.
 * @param actualSize Pointer to output the actual received data size to.
 * @param parameter Pointer to output the parameter handle to.
 */
Result APT_ReceiveParameter(NS_APPID appID, void* buffer, size_t bufferSize, NS_APPID* sender, APT_Command* command, size_t* actualSize, Handle* parameter);

/**
 * @brief Sends a parameter.
 * @param source AppID of the source application.
 * @param dest AppID of the destination application.
 * @param command Command to send.
 * @param buffer Buffer to send.
 * @param bufferSize Size of the buffer.
 * @param parameter Parameter handle to pass.
 */
Result APT_SendParameter(NS_APPID source, NS_APPID dest, APT_Command command, const void* buffer, u32 bufferSize, Handle parameter);

/**
 * @brief Cancels a parameter which matches the specified source and dest AppIDs.
 * @param source AppID of the source application (use APPID_NONE to disable the check).
 * @param dest AppID of the destination application (use APPID_NONE to disable the check).
 * @param success Pointer to output true if a parameter was cancelled, or false otherwise.
 */
Result APT_CancelParameter(NS_APPID source, NS_APPID dest, bool* success);

/**
 * @brief Sends capture buffer information.
 * @param captureBuf Capture buffer information to send.
 */
Result APT_SendCaptureBufferInfo(const aptCaptureBufInfo* captureBuf);

/**
 * @brief Replies to a sleep query.
 * @param appID ID of the application.
 * @param reply Query reply value.
 */
Result APT_ReplySleepQuery(NS_APPID appID, APT_QueryReply reply);

/**
 * @brief Replies that a sleep notification has been completed.
 * @param appID ID of the application.
 */
Result APT_ReplySleepNotificationComplete(NS_APPID appID);

/**
 * @brief Prepares to close the application.
 * @param cancelPreload Whether applet preloads should be cancelled.
 */
Result APT_PrepareToCloseApplication(bool cancelPreload);

/**
 * @brief Closes the application.
 * @param param Parameters to close with.
 * @param paramSize Size of param.
 * @param handle Handle to pass.
 */
Result APT_CloseApplication(const void* param, size_t paramSize, Handle handle);

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
 * @param out Pointer to write the New 3DS flag to.
 */
Result APT_CheckNew3DS(bool* out);

/**
 * @brief Prepares for an applicaton jump.
 * @param flags Flags to use.
 * @param programID ID of the program to jump to.
 * @param mediatype Media type of the program to jump to.
 */
Result APT_PrepareToDoApplicationJump(u8 flags, u64 programID, u8 mediatype);

/**
 * @brief Performs an application jump.
 * @param param Parameter buffer.
 * @param paramSize Size of parameter buffer.
 * @param hmac HMAC buffer (should be 0x20 bytes long).
 */
Result APT_DoApplicationJump(const void* param, size_t paramSize, const void* hmac);

/**
 * @brief Prepares to start a library applet.
 * @param appID AppID of the applet to start.
 */
Result APT_PrepareToStartLibraryApplet(NS_APPID appID);

/**
 * @brief Starts a library applet.
 * @param appID AppID of the applet to launch.
 * @param param Buffer containing applet parameters.
 * @param paramsize Size of the buffer.
 * @param handle Handle to pass to the applet.
 */
Result APT_StartLibraryApplet(NS_APPID appID, const void* param, size_t paramSize, Handle handle);

/**
 * @brief Prepares to start a system applet.
 * @param appID AppID of the applet to start.
 */
Result APT_PrepareToStartSystemApplet(NS_APPID appID);

/**
 * @brief Starts a system applet.
 * @param appID AppID of the applet to launch.
 * @param param Buffer containing applet parameters.
 * @param paramSize Size of the parameter buffer.
 * @param handle Handle to pass to the applet.
 */
Result APT_StartSystemApplet(NS_APPID appID, const void* param, size_t paramSize, Handle handle);

/**
 * @brief Retrieves the shared system font.
 * @brief fontHandle Pointer to write the handle of the system font memory block to.
 * @brief mapAddr Pointer to write the mapping address of the system font memory block to.
 */
Result APT_GetSharedFont(Handle* fontHandle, u32* mapAddr);

/**
 * @brief Receives the deliver (launch) argument
 * @param param Parameter buffer.
 * @param paramSize Size of parameter buffer.
 * @param hmac HMAC buffer (should be 0x20 bytes long).
 * @param sender Pointer to output the sender's AppID to.
 * @param received Pointer to output whether an argument was received to.
 */
Result APT_ReceiveDeliverArg(const void* param, size_t paramSize, const void* hmac, u64* sender, bool* received);
