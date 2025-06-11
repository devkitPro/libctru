/**
 * @file frd.h
 * @brief Friend Services
 */
#pragma once
#include <3ds/mii.h>

#define FRIEND_COMMENT_LEN               16+1  ///< 16-character NULL-terminated UTF-16 comment
#define FRIEND_GAME_MODE_DESCRIPTION_LEN 127+1 ///< 127-character NULL-terminated UTF-16 game mode description

#define NASC_INGAMESN_LEN                11+1  ///< 11-character NULL-terminated UTF-16 in-game nickname
#define NASC_KEYHASH_LEN                 8+1   ///< 8-character NULL-terminated ASCII NASC `keyhash` value
#define NASC_SVC_LEN                     4+1   ///< 4-character NULL-terminated ASCII NASC `svc` value

#define FRIEND_LIST_SIZE                 100   ///< 100 (maximum number of friends)

#define NFS_TYPESTR_LEN                  2+1   ///< 2-character NULL-terminated ASCII NFS (Nintendo Friend Server) type string

typedef u16 FriendComment[FRIEND_COMMENT_LEN];
typedef u16 FriendGameModeDescription[FRIEND_GAME_MODE_DESCRIPTION_LEN];

typedef u16 ScrambledFriendCode[6];

typedef char NfsTypeStr[NFS_TYPESTR_LEN];

#pragma pack(push, 1)

/// Friend key data
typedef struct
{
	u32 principalId;
	u32 padding;
	u64 localFriendCode;
} FriendKey;

/// Game key data
typedef struct
{
	u64 titleId;
	u16 version;
	u8 reserved[6];
} GameKey;

/// Base profile data
typedef struct
{
	u8 region;      ///< The region code for the hardware.
	u8 country;     ///< Country code.
	u8 area;        ///< Area code.
	u8 language;    ///< Language code.
	u8 platform;    ///< Platform code.
	u8 padding[3];
} Profile;

/// Friend profile data
typedef struct
{
	Profile profile; ///< Base profile data of this friend.
	GameKey favoriteGame; ///< Favorite game of this friend.
	u32 ncPrincipalId; ///< NC PrincipalID of this friend.
	FriendComment personalMessage; ///< Personal message (comment) of this friend.
	u8 pad[2];
	s64 lastOnlineTimestamp; ///< NEX timestamp of when this friend was last seen online.
} FriendProfile;

/// Base presence data
typedef struct
{
	u32 joinAvailabilityFlag;
	u32 matchmakeSystemType;
	u32 joinGameId;
	u32 joinGameMode;
	u32 ownerPrincipalId;
	u32 joinGroupId;
	u8 applicationArg[20];
} Presence;

/// Current user's presence data
typedef struct
{
	Presence presence; ///< The actual presence data.
	FriendGameModeDescription gameModeDescription; ///< The game mode description of the current user.
} MyPresence;

/// Friend presence data
typedef struct
{
	Presence presence; ///< The actual presence data.
	bool isPresenceLoaded; ///< Whether or not the presence data for this user has been loaded from the server.
	bool hasSentInvitation; ///< Whether or not this friend has sent the current user an invitation.
	bool found; ///< Whether or not this friend was found.
	u8 pad;
} FriendPresence;

/// Friend playing game structure
typedef struct
{
	GameKey game; ///< Game key of the game.
	FriendGameModeDescription gameModeDescription;
} FriendPlayingGame;

typedef CFLStoreData FriendMii;

/// Friend info structure
typedef struct
{
	FriendKey friendKey; ///< FriendKey of this friend.
	s64 addedTimestamp; ///< NEX timestamp of when this friend was added to the current user's friend list.
	u8 relationship; ///< The type of the relationship with this friend.
	u8 pad[7];
	FriendProfile friendProfile; ///< Friend profile data of this friend.
	MiiScreenName screenName; ///< The screen name of this friend.
	u8 characterSet; ///< The character set used for the text parts of the data of this friend.
	u8 pad2;
	FriendMii mii; ///< The Mii of this friend.
} FriendInfo;

/// Friend Notification Event structure
typedef struct
{
	u8 type; ///< Type of event.
	u8 padding[7];
	FriendKey sender; ///< Friend key of friend who caused this notification event to be sent.
} NotificationEvent;

/// Game Authentication Data structure
typedef struct
{
	u32 nascResult; ///< NASC result code for the LOGIN operation.
	u32 httpStatusCode; ///< HTTP status code for the NASC LOGIN operation.
	char serverAddress[0x20]; ///< Address of the game server.
	u16 serverPort; ///< Port of the game server.
	u8 pad[6];
	char authToken[0x100]; ///< Game server authentication token.
	u64 serverTime; ///< NEX timestamp for current server time.
} GameAuthenticationData;

/// Service Locator Data strcture
typedef struct
{
	u32 nascResult; ///< NASC result code for the SVCLOC operation.
	u32 httpStatusCode; ///< HTTP status code for the NASC LOGIN operation.
	char serviceHost[0x80]; ///< Host address of the target service.
	char serviceToken[0x100]; ///< Token for the target service.
	u8 statusData; ///< `statusdata` value from the NASC response data.
	u8 padding[7];
	u64 serverTime; ///< NEX timestamp for current server time.
} ServiceLocatorData;

/// Encrypted inner Approach Context structure
typedef struct
{
	FriendProfile friendProfile;
	bool hasMii;
	bool profanityFlag;
	u8 characterSet;
	u8 wrappedMii[0x70];
	MiiScreenName screenName;
	u8 reserved[0x10F];
} ApproachContext;

/// Encrypted Approach Context structure
typedef struct
{
	u8 unknown0;
	u8 unknown1;
	u8 unknown2;
	u8 unknown3;
	struct {
		u32 principalId;
		u64 friendCode;
	} CTR_PACKED nonce;
	ApproachContext encryptedPayload;
	u8 ccmMac[16];
} EncryptedApproachContext;

/// Decrypted Approach Context structure
typedef struct
{
	u8 unknown0;
	u8 unknown1;
	u8 unknown2;
	u8 unknown3;
	bool hasMii; ///< Whether or not this friend has a Mii.
	bool profanityFlag;
	u8 characterSet; ///< Character set for text data.
	u8 pad;
	FriendKey friendKey; ///< Friend key of this friend.
	FriendProfile friendProfile; ///< Friend profile of this friend.
	FriendMii mii; ///< Mii data of this friend.
	MiiScreenName screenName; ///< UTF-16 screen name of this friend.
	u8 reserved[0x12A];
} DecryptedApproachContext;

#pragma pack(pop)

/// Enum for character set
typedef enum
{
	CHARSET_JPN_USA_EUR = 0, ///< Character set for JPN, USA, and EUR(+AUS).
	CHARSET_CHN,             ///< Character set for CHN.
	CHARSET_KOR,             ///< Character set for KOR.
	CHARSET_TWN,             ///< Character set for TWN.
} CharacterSet;

/// Enum for NASC Result
typedef enum
{
	NASC_SUCCESS                    = 001,
	NASC_SERVER_UNDER_MAINTENANCE   = 101,
	NASC_DEVICE_BANNED              = 102,
	NASC_INVALID_PRODUCT_CODE       = 107,
	NASC_INVALID_REQUEST_PARAM      = 109,
	NASC_SERVER_NO_LONGER_AVAILABLE = 110,
	NASC_INVALID_SVC                = 112,
	NASC_INVALID_FPD_VERSION        = 119,
	NASC_INVALID_TITLE_VERSION      = 120,
	NASC_INVALID_DEVICE_CERTIFICATE = 121,
	NASC_INVALID_PID_HMAC           = 122,
	NASC_BANNED_ROM_ID              = 123,
	NASC_INVALID_GAME_ID            = 125,
	NASC_INVALID_KEY_HASH           = 127,
} NASCResult;

/// Enum for NASC Server Environment
typedef enum
{
	NASC_PRODUCTION = 0,
	NASC_TESTING,
	NASC_DEVELOPMENT
} NASCEnvironment;

/// Enum for notification event types
typedef enum
{
	USER_WENT_ONLINE = 1,                       ///< Self went online
	USER_WENT_OFFLINE,                          ///< Self went offline
	FRIEND_WENT_ONLINE,                         ///< Friend Went Online
	FRIEND_UPDATED_PRESENCE,                    ///< Friend Presence changed (with matching GameJoinID)
	FRIEND_UPDATED_MII,                         ///< Friend Mii changed
	FRIEND_UPDATED_PROFILE,                     ///< Friend Profile changed
	FRIEND_WENT_OFFLINE,                        ///< Friend went offline
	FRIEND_REGISTERED_USER,                     ///< Friend registered self as friend
	FRIEND_SENT_JOINABLE_INVITATION,            ///< Friend sent invitation (with matching GameJoinID)
	FRIEND_CHANGED_GAME_MODE_DESCRIPTION = 145, ///< Friend changed game mode description
	FRIEND_CHANGED_FAVORITE_GAME         = 146, ///< Friend changed favorite game
	FRIEND_CHANGED_COMMENT               = 147, ///< Friend changed comment
	FRIEND_CHANGED_ANY_PRESENCE          = 148, ///< Friend Presence changed (with nonmatching GameJoinID)
	FRIEND_SENT_ANY_INVITATION           = 149, ///< Friend sent invitiation (with nonmatching GameJoinID)
} FriendNotificationTypes;

/// Enum for notification event mask
typedef enum
{
	MASK_USER_WENT_ONLINE                     = BIT(USER_WENT_ONLINE - 1),
	MASK_USER_WENT_OFFLINE                    = BIT(USER_WENT_OFFLINE - 1),
	MASK_FRIEND_WENT_ONLINE                   = BIT(FRIEND_WENT_ONLINE - 1),
	MASK_FRIEND_UPDATED_PRESENCE              = BIT(FRIEND_UPDATED_PRESENCE - 1),
	MASK_FRIEND_UPDATED_MII                   = BIT(FRIEND_UPDATED_MII - 1),
	MASK_FRIEND_UPDATED_PROFILE               = BIT(FRIEND_UPDATED_PROFILE - 1),
	MASK_FRIEND_WENT_OFFLINE                  = BIT(FRIEND_WENT_OFFLINE - 1),
	MASK_FRIEND_REGISTERED_USER               = BIT(FRIEND_REGISTERED_USER - 1),
	MASK_FRIEND_SENT_JOINABLE_INVITATION      = BIT(FRIEND_SENT_JOINABLE_INVITATION - 1),
	// values >= 145 are not exposed to sessions.
} FriendNotificationMask;

/// Enum for friend relationship type
typedef enum
{
	RELATIONSHIP_INCOMPLETE = 0, ///< Provisionally registered friend.
	RELATIONSHIP_COMPLETE,       ///< Fully registered friend.
	RELATIONSHIP_NOT_FOUND,      ///< Friend not registered at all.
	RELATIONSHIP_DELETED,        ///< Relationship was deleted.
	RELATIONSHIP_LOCAL,          ///< Provisionally registered friend (but this relationship has not been sent to the server yet).
} RelationshipType;

/// Enum for friend attributes according to relationship type
typedef enum
{
	FRIEND_ATTRIBUTE_EVER_REGISTERED       = BIT(0), ///< Whether or not the current user has ever been in a friend relationship with the friend. This is set when the relationship type is either incomplete, complete, local, or deleted.
	FRIEND_ATTRIBUTE_REGISTRATION_COMPLETE = BIT(1)  ///< Whether or not the current user has been fully registered by this friend. Set only when the relationship type is complete.
} FriendAttributes;

/// Enum for NAT mapping type
typedef enum
{
	NAT_MAPPING_UNKNOWN = 0,
	NAT_MAPPING_ENDPOINT_INDEPENDENT,
	NAT_MAPPING_ENDPOINT_DEPENDENT
} NatMappingType;

/// Enum for NAT filtering type
typedef enum
{
	NAT_FILTERING_UNKNOWN = 0,
	NAT_FILTERING_PORT_INDEPENDENT,
	NAT_FILTERING_PORT_DEPENDENT
} NatFilteringType;

/**
 * @brief Initializes friend services.
 * @param forceUser Whether or not to force using the user service frd:u instead of the default (admin service frd:a).
 */
Result frdInit(bool forceUser);

/// Exits friend services.
void frdExit(void);

/// Get the friend user/admin service handle.
Handle *frdGetSessionHandle(void);

/**
 * @brief Gets the login status of the current user.
 * @param state Pointer to write the current user's login status to.
 */
Result FRD_HasLoggedIn(bool *state);

/**
 * @brief Gets the online status of the current user.
 * @param state Pointer to write the current user's online status to.
 */
Result FRD_IsOnline(bool *state);

/**
 * @brief Log in to Nintendo's friend server.
 * @param event Event to signal when Login is done.
 */
Result FRD_Login(Handle event);

/// Logs out of Nintendo's friend server.
Result FRD_Logout(void);

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
 * @brief Gets the current user's presence information.
 * @param presence Pointer to write the current user's presence information to.
 */
Result FRD_GetMyPresence(MyPresence *presence);

/**
 * @brief Gets the current user's screen name.
 * @param name Pointer to write the current user's screen name to.
 * @param max_size Max size of the screen name.
 */
Result FRD_GetMyScreenName(MiiScreenName *name);

/**
 * @brief Gets the current user's Mii data.
 * @param mii Pointer to write the current user's mii data to.
 */
Result FRD_GetMyMii(FriendMii *mii);

/**
 * @brief Gets the ID of the current local account.
 * @param localAccountId Pointer to write the current local account ID to.\
 */
Result FRD_GetMyLocalAccountId(u8 *localAccountId);

/**
 * @brief Gets the current user's playing game.
 * @param titleId Pointer to write the current user's playing game to.
 */
Result FRD_GetMyPlayingGame(GameKey *playingGame);

/**
 * @brief Gets the current user's favourite game.
 * @param titleId Pointer to write the title ID of current user's favourite game to.
 */
Result FRD_GetMyFavoriteGame(GameKey *favoriteGame);

/**
 * @brief Gets the NcPrincipalId for the current user.
 * @param ncPrincipalId Pointer to output the NcPrincipalId to.
 */
Result FRD_GetMyNcPrincipalId(u32 *ncPrincipalId);

/**
 * @brief Gets the current user's comment on their friend profile.
 * @param comment Pointer to write the current user's comment to.
 * @param max_size Max size of the comment.
 */
Result FRD_GetMyComment(FriendComment *comment);

/**
 * @brief Gets the current friend account's NEX password.
 * @param password Pointer to write the NEX password to.
 * @param max_size Max size of the output buffer. Must not exceed 0x800.
 */
Result FRD_GetMyPassword(char *password, u32 bufsize);

/**
 * @brief Gets the current user's friend key list.
 * @param friendKeyList Pointer to write the friend key list to.
 * @param num Stores the number of friend keys obtained.
 * @param offset The index of the friend key to start with.
 * @param count Number of friend keys to retrieve. Max: FRIEND_LIST_SIZE
 */
Result FRD_GetFriendKeyList(FriendKey *friendKeyList, u32 *num, u32 offset, u32 count);

/**
 * @brief Gets friend presence data for the current user's friends.
 * @param friendPresences Pointer to write the friend presence data to.
 * @param friendKeyList The friend keys of the friends to get presence data for.
 * @param count The number of input friend keys.
 */
Result FRD_GetFriendPresence(FriendPresence *friendPresences, const FriendKey *friendKeyList, u32 count);

/**
 * @brief Gets screen names for the current user's friends.
 * @param screenNames Pointer to write the UTF-16 screen names to.
 * @param screenNamesLen Number of UTF-16 characters `screenNames` can hold. (max: 0x800)
 * @param characterSets Pointer to write the character sets for the screen names to.
 * @param characterSetsLen Size of buffer to output character sets to.
 * @param friendKeyList The friend keys for the friends to get screen names for.
 * @param count The number of input friend keys.
 * @param maskNonAscii Whether or not to replace all non-ASCII characters with question marks ('?') if the given character set doesn't match that of the corresponding friend's Mii data.
 * @param profanityFlag Setting this to true replaces the screen names with all question marks ('?') if profanityFlag is also set in the corresponding friend's Mii data.
 */
Result FRD_GetMiiScreenName(MiiScreenName *screenNames, u32 screenNamesLen, u8 *characterSets, u32 characterSetsLen, const FriendKey *friendKeyList, u32 count, bool maskNonAscii, bool profanityFlag);

/**
 * @brief Gets the current user's friends' Mii data.
 * @param miiList Pointer to write Mii data to.
 * @param friendKeyList Pointer to input friend keys.
 * @param count Number of input friend keys.
 */
Result FRD_GetFriendMii(FriendMii *miiList, const FriendKey *friendKeyList, u32 count);

/**
 * @brief Get the current user's friends' profile data.
 * @param profile Pointer to write profile data to.
 * @param friendKeyList Pointer to input friend keys.
 * @param count Number of input friend keys.
 */
Result FRD_GetFriendProfile(Profile *profiles, const FriendKey *friendKeyList, u32 count);

/**
 * @brief Get the relationship type for the current user's friends.
 * @param relationships Pointer to output relationship types to.
 * @param friendKeyList Pointer to input friend keys to query relationship types for.
 * @param count Number of input friend keys.
 */
Result FRD_GetFriendRelationship(u8 *relationships, const FriendKey *friendKeyList, u32 count);

/**
 * @brief Get attributes for the current user's friends.
 * @param attributes Pointer to output the attributes to.
 * @param friendKeyList Pointer to input friend keys to query attributes for.
 * @param count Number of input friend keys.
 */
Result FRD_GetFriendAttributeFlags(u32 *attributes, const FriendKey *friendKeyList, u32 count);

/**
 * @brief Get the current user's friends' playing game.
 * @param playingGames Pointer to write playing game data to.
 * @param friendKeyList Pointer to friend keys.
 * @param count Number of input friend keys.
 */
Result FRD_GetFriendPlayingGame(FriendPlayingGame *playingGames, const FriendKey *friendKeyList, u32 count);

/**
 * @brief Get the current user's friends' favourite games.
 * @param favoriteGames Pointer to write game key data to.
 * @param friendKeyList Pointer to friend keys.
 * @param count Number of friend keys.
 */
Result FRD_GetFriendFavoriteGame(GameKey *favoriteGames, const FriendKey *friendKeyList, u32 count);

/**
 * @brief Get info about the current user's friends.
 * @param infos Pointer to output friend info data to.
 * @param friendKeyList Pointer to input friend keys.
 * @param count Number of input friend keys.
 * @param maskNonAscii Whether or not to replace all non-ASCII characters with question marks ('?') if the given character set doesn't match that of the corresponding friend's Mii data.
 * @param profanityFlag Setting this to true replaces the screen names with all question marks ('?') if profanityFlag is also set in the corresponding friend's Mii data.
 */
Result FRD_GetFriendInfo(FriendInfo *infos, const FriendKey *friendKeyList, u32 count, bool maskNonAscii, bool profanityFlag);

/**
 * @brief Gets whether a friend code is included in the current user's friend list.
 * @param friendCode The friend code to check for.
 * @param isFromList Pointer to write whether or not the given friend code was found in the current user's friends list.
 */
Result FRD_IsInFriendList(u64 friendCode, bool *isFromList);

/**
 * @brief Unscrambles a scrambled friend code.
 * @param unscrambled Pointer to output the unscrambled friend codes to.
 * @param scrambled Pointer to the input scrambled friend codes.
 * @param count Number of input scrambled codes.
 */
Result FRD_UnscrambleLocalFriendCode(u64 *unscrambled, ScrambledFriendCode *scrambled, u32 count);

/**
 * @brief Updates the game mode description string.
 * @param desc Pointer to the UTF-8 game mode description to use.
 */
Result FRD_UpdateGameModeDescription(FriendGameModeDescription *desc);

/**
 * @brief Updates the current user's presence data and game mode description.
 * @param presence The new presence data to use.
 * @param desc The new game mode description to use.
 */
Result FRD_UpdateMyPresence(Presence *presence, FriendGameModeDescription *desc);

/**
 * @brief Sends an invitation to the current user's friends.
 * @param friendKeyList The friend keys to send an invitation to.
 * @param count The number of input friend keys.
 */
Result FRD_SendInvitation(const FriendKey *friendKeyList, u32 count);

/**
 * @brief Registers the event handle that will be signaled to inform the session of various status changes.
 * @param event The event handle to register for notification signaling.
 */
Result FRD_AttachToEventNotification(Handle event);

/**
 * @brief Sets the notification mask for the event notification system.
 * @param mask The notifications to subscribe to for the event notification system.
 */
Result FRD_SetNotificationMask(FriendNotificationMask mask);

/**
 * @brief Get Latest Event Notification
 * @param event Pointer to write recieved notification event struct to.
 * @param count Number of events
 * @param recievedNotifCount Number of notification reccieved.
 */
Result FRD_GetEventNotification(NotificationEvent *event, u32 count, u32 *recievedNotifCount);

/**
 * @brief Get the result of the last internal operation.
 */
Result FRD_GetLastResponseResult();

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
 * @brief Get a support error code (XXX-YYYY) for the given result code.
 * @param errorCode Pointer to write the support error code to.
 * @param res The result code to convert.
 */
Result FRD_ResultToErrorCode(u32 *errorCode, Result res);

/**
 * @brief Requests game server authentication.
 * @param serverId The ID of the NEX server to request authentication for.
 * @param ingamesn The UTF-16 nickname to use in game.
 * @param ingamesnSize Buffer size of the input ingamesn buffer. (max: FRIEND_INGAMESN_LEN * 2)
 * @param majorSdkVersion The major SDK version.
 * @param minorSdkVersion The minor SDK version.
 * @param completionEvent The event handle to signal once the operation has completed.
 */
Result FRD_RequestGameAuthentication(u32 serverId, u16 *ingamesn, u32 ingamesnSize, u8 majorSdkVersion, u8 minorSdkVersion, Handle completionEvent);

/**
 * @brief Get game server authentication data requested using FRD_RequestGameAuthentication.
 * @param data Pointer to write game server authentication data to.
 */
Result FRD_GetGameAuthenticationData(GameAuthenticationData *data);

/**
 * @brief Request service locator info for a given NEX server.
 * @param keyhash The `keyhash` value to use for the NASC request.
 * @param svc The svc `value` to use for the NASC request.
 * @param majorSdkVersion The major SDK version.
 * @param minorSdkVersion The minor SDK version.
 * @param completionEvent The event handle to signal once the operation has completed.
 * @param serverId
 */
Result FRD_RequestServiceLocator(u32 serverId, char *keyhash, char *svc, u8 majorSdkVersion, u8 minorSdkVersion, Handle completionEvent);

/**
 * @brief Get service locator data requested using FRD_RequestServiceLocator.
 * @param data Pointer to write the service locator data to.
 */
Result FRD_GetServiceLocatorData(ServiceLocatorData *data);

/**
 * @brief Starts an internal task to determine the NAT properties of the current internet connection.
 * @param completionEvent The event handle to signal once the task has completed.
 */
Result FRD_DetectNatProperties(Handle completionEvent);

/**
 * @brief Returns NAT properties for the current internet connection.
 * @param natMappingType Pointer to write the NAT mapping type of the connection to.
 * @param natFilteringType Pointer to write the NAT filtering type of the connection to.
 */
Result FRD_GetNatProperties(u32 *natMappingType, u32 *natFilteringType);

/**
 * @brief Returns the difference (in nanoseconds) between server time and device time. This difference is calculated every time the system logs into friend services.
 * @param diffMs The pointer to write the time difference (in nanoseconds) to.
 */
Result FRD_GetServerTimeDifference(u64 *diff);

/**
 * @brief Configures the current session to allow or disallow running the friends service in sleep mode (half-awake mode).
 * @param allow Whether or not to enable half-awake mode.
 */
Result FRD_AllowHalfAwake(bool allow);

/**
 * @brief Gets the server environment configuration for the current user.
 * @param nascEnvironment Pointer to write the NASC server environment type to.
 * @param nfsType Pointer to write the NFS (Nintendo Friend Server) type to.
 * @param nfsNo Pointer to write the NFS (Nintendo Friend Server) number to.
 */
Result FRD_GetServerTypes(u8 *nascEnvironment, u8 *nfsType, u8 *nfsNo);

/**
 * @brief Gets the comment (personal) message of the current user's friends.
 * @param comments Pointer to write the friend comment data to.
 * @param commentsLen Number of UTF-16 characters `screenNames` can hold. (max: 0xC00)
 * @param friendKeyList Pointer to input friend keys.
 * @param count Number of input friend keys.
 */
Result FRD_GetFriendComment(FriendComment *comments, u32 commentsLen, const FriendKey *friendKeyList, u32 count);

/**
 * @brief Sets the Friend API to use a specific SDK version.
 * @param sdkVer The SDK version needed to be used.
 */
Result FRD_SetClientSdkVersion(u32 sdkVer);

/**
 * @brief Gets the current user's encrypted approach context.
 * @param ctx Pointer to write the encrypted approach context data to.
 */
Result FRD_GetMyApproachContext(EncryptedApproachContext *ctx);

/**
 * @brief Adds a friend using their encrypted approach context.
 * @param unkbuf Pointer to unknown (and unused) data.
 * @param unkbufSize Size of unknown (and unused) data. (max: 0x600)
 * @param ctx Pointer to encrypted approach context data.
 * @param completionEvent The event handle to signal when this action is completed.
 */
Result FRD_AddFriendWithApproach(u8 *unkbuf, u32 unkbufSize, EncryptedApproachContext *ctx, Handle completionEvent);

/**
 * @brief Decrypts an encrypted approach context.
 * @param decryptedContext Pointer to write the decrypted approach context data to.
 * @param encryptedContext Pointer to input encrypted approach context.
 * @param maskNonAscii Whether or not to replace all non-ASCII characters with question marks ('?') if the given character set doesn't match that of the corresponding friend's Mii data.
 * @param characterSet The character set to use for text conversions.
 */
Result FRD_DecryptApproachContext(DecryptedApproachContext *decryptedContext, EncryptedApproachContext *encryptedContext, bool maskNonAscii, u8 characterSet);

/**
 * @brief Gets extended NAT properties. This is the same as FRD_GetNatProperties, with this version also returning the NAT Mapping Port Increment.
 * @param natMappingType Pointer to write the NAT mapping type of the connection to.
 * @param natFilteringType Pointer to write the NAT filtering type of the connection to.
 * @param natMappingPortIncrement Pointer to write the NAT mapping port increment to.
 */
Result FRD_GetExtendedNatProperties(u32 *natMappingType, u32 *natFilteringType, u32 *natMappingPortIncrement);

/**
 * @brief Creates a new local friends account.
 * @param localAccountId The local account ID to use.
 * @param nascEnvironment The NASC environment to create this account in.
 * @param nfsType The NFS (Nintendo Friend Server) type this account should use.
 * @param nfsNo The NFS (Nintendo Friend Server) number this account should use.
 */
Result FRDA_CreateLocalAccount(u8 localAccountId, u8 nascEnvironment, u8 nfsType, u8 nfsNo);

/**
 * @brief Deletes a local friends account.
 * @param localAccountId The ID of the local account to delete.
 */
Result FRDA_DeleteLocalAccount(u8 localAccountId);

/**
 * @brief Loads a local friends account.
 * @param localAccountId The ID of the local account to load.
 */
Result FRDA_LoadLocalAccount(u8 localAccountId);

/**
 * @brief Unloads the currently active local account.
 */
Result FRDA_UnloadLocalAccount();

/**
 * @brief Saves all data of the friends module.
 */
Result FRDA_Save();

/**
 * @brief Adds a friend online ("Internet" option).
 * @param event Event signaled when friend is registered.
 * @param principalId PrincipalId of the friend to add.
 */
Result FRDA_AddFriendOnline(Handle event, u32 principalId);

/**
 * @brief Adds a friend offline ("Local" option).
 * @param friendKey Pointer to the friend key of the friend to add.
 * @param mii Pointer to the Mii of the friend to add.
 * @param friendProfile Pointer to the friend profile of the friend to add.
 * @param screenName Pointer to the UTF-16 screen name of the friend to add.
 * @param profanityFlag Setting this to true will cause calls that return the screen name to replace it with question marks ('?') when profanityFlag is true in those calls.
 * @param characterSet The character set to use for text data of the friend.
 */
Result FRDA_AddFriendOffline(FriendKey *friendKey, FriendMii *mii, FriendProfile *friendProfile, MiiScreenName *screenName, bool profanityFlag, u8 characterSet);

/**
 * @brief Updates a friend's display name.
 * @param friendKey Pointer to friend key of the friend to update the screen name of.
 * @param screenName Pointer to the new screen name to use.
 * @param characterSet The character set of the new screen name.
 */
Result FRDA_UpdateMiiScreenName(FriendKey *friendKey, MiiScreenName *screenName, u8 characterSet);

/**
 * @brief Remove a friend.
 * @param principalId PrinipalId of the friend code to remove.
 * @param localFriendCode LocalFriendCode of the friend code to remove.
 */
Result FRDA_RemoveFriend(u32 principalId, u64 localFriendCode);

/**
 * @brief Updates the game being played by the current user.
 * @param playingGame Pointer to game key of the game being played.
 */
Result FRDA_UpdatePlayingGame(GameKey *playingGame);

/**
 * @brief Updates the current user's friend list preferences.
 * @param isPublicMode Whether or not the online status should be public.
 * @param isShowGameMode Whether or not the currently played game is shown.
 * @param isShowPlayedMode Whether or not the play history is shown.
 */
Result FRDA_UpdatePreference(bool isPublicMode, bool isShowGameMode, bool isShowPlayedMode);

/**
 * @brief Updates the current user's Mii.
 * @param mii Pointer to the new Mii data to use.
 * @param screenName Pointer to new screen name associated with the new Mii.
 * @param profanityFlag Setting this to true will cause calls that return the screen name to replace it with question marks ('?') when profanityFlag is true in those calls.
 * @param characterSet The character set to use for the screen name.
 */
Result FRDA_UpdateMii(FriendMii *mii, MiiScreenName *screenName, bool profanityFlag, u8 characterSet);

/**
 * @brief Updates the current user's favorite game.
 * @param favoriteGame Pointer to the game key of the new favorite game.
 */
Result FRDA_UpdateFavoriteGame(GameKey *favoriteGame);

/**
 * @brief Sets the NcPrincipalId of the current user.
 * @param ncPrincipalId The new NcPrincipalId.
 */
Result FRDA_SetNcPrincipalId(u32 ncPrincipalId);

/**
 * @brief Updates the current user's comment (personal message).
 * @param comment Pointer to the new comment (personal message).
 */
Result FRDA_UpdateComment(FriendComment *comment);

/**
 * @brief Increments the move count in the current local account's save data.
 */
Result FRDA_IncrementMoveCount();
