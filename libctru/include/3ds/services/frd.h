/**
 * @file frd.h
 * @brief Friend Services
 */
#pragma once
#include <3ds.h>

#define FRIENDS_SCREEN_NAME_SIZE 0x16   // 11 (0x16 because UTF-16)
#define FRIENDS_COMMENT_SIZE 0x22       // 16 (0x21 because UTF-16 + null character)
#define FRIEND_LIST_SIZE 0x64           // 100 (Number of Friends)
#define FRIEND_MII_STORE_DATA_SIZE 0x60 // 96 (Mii data)

/// Friend key data
typedef struct
{
	u32 principalId;
	u32 padding;
	u64 localFriendCode;
} FriendKey;

/// Friend Title data
typedef struct
{
	u64 tid;
	u32 version;
	u32 unk;
} TitleData;
/// Structure containing basic Mii information.
typedef struct 
{
	u32 mii_id;
	u64 system_id;
	u32 cdate;
	u8 mac[0x6];
	u16 padding;
	u16 misc1;
	u16 mii_name[0xB];
	u8 width;
	u8 height;
	u32 misc2;
	u32 unknown1;
	u32 misc3;
	u32 unknown2;
	u8 allow_copy;
	u8 unknown3[0x7];
	u16 author[0xB];
} MiiData;

/// Friend profile data
typedef struct
{
	u8 region;      // The region code for the hardware.
	u8 country;     // Country code.
	u8 area;        // Area code.
	u8 language;    // Language code.
	u8 platform;    // Platform code.
	u32 padding;
} Profile;

/// Game Description structure
typedef struct
{
	TitleData data;
	u16 desc[128];
} GameDescription;

/// Friend Notification Event structure
typedef struct
{
	u8 type;
	u8 padding3[3];
	u32 padding;
	FriendKey key;
}NotificationEvent;

/// Enum to use with FRD_GetNotificationEvent
typedef enum 
{
	selfOnline = 1, // Self went online
	selfOffline, // Self went offline
	friendOnline, // Friend Went Online 
	friendPresence, // Friend Presence changed
	friendMii, // Friend Mii changed
	friendProfile, // Friend Profile changed
	friendOffline, // Friend went offline
	friendBecameFriend, // Friend registered self as friend
	friendInvitaton // Friend Sent invitation
}NotificationTypes;

/// Initializes FRD service.
Result frdInit(void);

/// Exists FRD.
void frdExit(void);

/**
 * @brief Gets the login status of the current user.
 * @param state Pointer to write the current user's login status to.
 */
Result FRDU_HasLoggedIn(bool *state);

/**
 * @brief Gets the online status of the current user.
 * @param state Pointer to write the current user's online status to.
 */
Result FRDU_IsOnline(bool *state);

/// Logs out of Nintendo's friend server.
Result FRD_Logout(void);

/**
 * @brief Log in to Nintendo's friend server.
 * @param event Event to signal when Login is done.
 */
Result FRD_Login(Handle event);

/**
 * @brief Gets the current user's friend key.
 * @param key Pointer to write the current user's friend key to.
 */
Result FRD_GetMyFriendKey(FriendKey *key);

/**
 * @brief Gets the current user's privacy information.
 * @param isPublicMode Determines whether friends are notified of the current user's online status.
 * @param isShowGameName Determines whether friends are notified of the application that the current user is running.
 * @param isShowPlayedGame Determiens whether to display the current user's game history.
 */
Result FRD_GetMyPreference(bool *isPublicMode, bool *isShowGameName, bool *isShowPlayedGame);

/**
 * @brief Gets the current user's profile information.
 * @param profile Pointer to write the current user's profile information to.
 */
Result FRD_GetMyProfile(Profile *profile);

/**
 * @brief Gets the current user's screen name.
 * @param name Pointer to write the current user's screen name to.
 * @param max_size Max size of the screen name.
 */
Result FRD_GetMyScreenName(char *name, size_t max_size);

/**
 * @brief Gets the current user's Mii data.
 * @param mii Pointer to write the current user's mii data to.
 */
Result FRD_GetMyMii(MiiData *mii);

/**
 * @brief Gets the current user's playing game.
 * @param titleId Pointer to write the current user's playing game to.
 */
Result FRD_GetMyPlayingGame(u64 *titleId);

/**
 * @brief Gets the current user's favourite game.
 * @param titleId Pointer to write the title ID of current user's favourite game to.
 */
Result FRD_GetMyFavoriteGame(u64 *titleId);

/**
 * @brief Gets the current user's comment on their friend profile.
 * @param comment Pointer to write the current user's comment to.
 * @param max_size Max size of the comment.
 */
Result FRD_GetMyComment(char *comment, size_t max_size);

/**
 * @brief Gets the current user's firend key list
 * @param friendKeyList Pointer to write the friend key list to.
 * @param num Stores the number of friend keys obtained.
 * @param offset the index of the friend key to start with.
 * @param size Size of the friend key list. (FRIEND_LIST_SIZE)
 */
Result FRD_GetFriendKeyList(FriendKey *friendKeyList, size_t *num, size_t offset, size_t size);

/**
 * @brief Gets Friends Mii data.
 * @param mii Pointer to write Mii data to.
 * @param keys Pointer to FriendKeys.
 * @param numberOfKeys Number of Friendkeys.
 */
Result FRD_GetFriendMii(MiiData *mii, const FriendKey *keys, size_t numberOfKeys);

/**
 * @brief Get a friend's profile data.
 * @param profile Pointer to write profile data to.
 * @param keys Pointer to FriendKeys.
 * @param numberOfKeys Number of FriendKeys.
 */
Result FRD_GetFriendProfile(Profile *profile, const FriendKey *keys, size_t numberOfKeys);

/**
 * @brief Get a friend's playing Game.
 * @param desc Pointer to write Game Description data to.
 * @param keys Pointer to FriendKeys,
 * @param numberOfKeys Number Of FriendKeys.
 */
Result FRD_GetFriendPlayingGame(GameDescription *desc, const FriendKey *keys, size_t numberOfKeys);

/**
 * @brief Get a friend's favourite Game.
 * @param desc Pointer to write Game Description data to.
 * @param keys Pointer to FriendKeys,
 * @param numberOfKeys Number Of FriendKeys.
 */
Result FRD_GetFriendFavouriteGame(GameDescription *desc, const FriendKey *keys, size_t numberOfKeys);

/**
 * @brief Determines if the application was started using the join game option in the friends applet.
 * @param friendKeyList Pointer to a list of friend keys.
 * @param isFromList Pointer to a write the friendship status to.
 */
Result FRD_IsFromFriendList(FriendKey *friendKeyList, bool *isFromList);

/**
 * @brief Updates the game mode description string.
 * @param desc Pointer to write the game mode description to.
 */
Result FRD_UpdateGameModeDescription(const char *desc);

/**
 * @brief Event which is signaled when friend login states change.
 * @param event event which will be signaled.
 */
Result FRD_AttachToEventNotification(Handle event);

/**
 * @brief Get Latest Event Notification
 * @param event Pointer to write recieved notification event struct to.
 * @param size Number of events
 * @param recievedNotifCount Number of notification reccieved.
 */
Result FRD_GetEventNotification(NotificationEvent *event, size_t size, u32 *recievedNotifCount); 

/**
 * @brief Returns the friend code using the given principal ID.
 * @param principalId The principal ID being used.
 * @param friendCode Pointer to write the friend code to.
 */
Result FRD_PrincipalIdToFriendCode(u32 principalId, u64 *friendCode);

/**
 * @brief Returns the principal ID using the given friend code.
 * @param friendCode The friend code being used.
 * @param principalId Pointer to write the principal ID to.
 */
Result FRD_FriendCodeToPrincipalId(u64 friendCode, u32 *principalId);

/**
 * @brief Checks if the friend code is valid.
 * @param friendCode The friend code being used.
 * @param isValid Pointer to write the validity of the friend code to.
 */
Result FRD_IsValidFriendCode(u64 friendCode, bool *isValid);

/**
 * @brief Sets the Friend API to use a specific SDK version.
 * @param sdkVer The SDK version needed to be used.
 */
Result FRD_SetClientSdkVersion(u32 sdkVer);

/**
 * @brief Add a Friend online.
 * @param event Event signaled when friend is registered.
 * @param principalId PrincipalId of the friend to add.
 */
Result FRD_AddFriendOnline(Handle event, u32 principalId);

/**
 * @brief Remove a Friend.
 * @param principalId PrinipalId of the friend code to remove.
 * @param localFriendCode LocalFriendCode of the friend code to remove.
 */
Result FRD_RemoveFriend(u32 principalId, u64 localFriendCode);
