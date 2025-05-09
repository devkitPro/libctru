/**
 * @file act.h
 * @brief ACT (Account) Services
 */
#pragma once
#include <3ds/services/frd.h>
#include <3ds/types.h>
#include <3ds/mii.h>

#define ACCOUNT_ID_LEN         16+1  ///< 16-character NULL-terminated ASCII Account ID
#define ACCOUNT_EMAIL_LEN      256+1 ///< 256-character NULL-terminated ASCII email address
#define ACCOUNT_PASSWORD_LEN   17+1  ///< 17-character NULL-terminated ASCII password

#define ACT_NNAS_SUBDOMAIN_LEN 32+1  ///< 32-character NULL-terminated ASCII NNAS (Nintendo Network Authentication Server) subdomain
#define ACT_UUID_LEN           16    ///< RFC9562 Version 1 UUID

#define ACT_DEFAULT_ACCOUNT 0xFE

#define ACT_UUID_REGULAR         -1 ///< The final UUID will not be based on any unique ID.
#define ACT_UUID_CURRENT_PROCESS -2 ///< The final UUID will be based on the unique ID of the current process.

#define ACT_TRANSFERABLE_ID_BASE_COMMON          0xFF ///< Use a common transferable ID base.
#define ACT_TRANSFERABLE_ID_BASE_CURRENT_ACCOUNT 0xFE ///< Use the transferable ID base of the currently loaded account.

typedef u8 ActUuid[ACT_UUID_LEN];

typedef char ActNnasSubdomain[ACT_NNAS_SUBDOMAIN_LEN];

typedef char AccountId[ACCOUNT_ID_LEN];
typedef char AccountPassword[ACCOUNT_PASSWORD_LEN];
typedef char AccountMailAddress[ACCOUNT_EMAIL_LEN];

/// Enum for common / account specific info types
typedef enum
{
	INFO_TYPE_COMMON_NUM_ACCOUNTS = 1, ///< u8
	INFO_TYPE_COMMON_CURRENT_ACCOUNT_SLOT, ///< u8
	INFO_TYPE_COMMON_DEFAULT_ACCOUNT_SLOT, ///< u8
	INFO_TYPE_COMMON_NETWORK_TIME_DIFF, ///< s64, difference between server time and device time in nanoseconds.
	INFO_TYPE_PERSISTENT_ID, ///< u32
	INFO_TYPE_COMMON_TRANSFERABLE_ID_BASE, ///< u64
	INFO_TYPE_TRANSFERABLE_ID_BASE = INFO_TYPE_COMMON_TRANSFERABLE_ID_BASE, ///< u64
	INFO_TYPE_MII, ///< CFLStoreData
	INFO_TYPE_ACCOUNT_ID, ///< AccountId
	INFO_TYPE_MAIL_ADDRESS, ///< AccountMailAddress
	INFO_TYPE_BIRTH_DATE, ///< BirthDate structure
	INFO_TYPE_COUNTRY_NAME, ///< char[2+1]
	INFO_TYPE_PRINCIPAL_ID, ///< u32
	INFO_TYPE_STUB_0xD, /* not handled */
	INFO_TYPE_IS_PASSWORD_CACHE_ENABLED, ///< bool
	INFO_TYPE_STUB_0xF, /* not handled */
	INFO_TYPE_STUB_0x10, /* not handled */
	INFO_TYPE_ACCOUNT_INFO, ///< AccountInfo structure
	INFO_TYPE_ACCOUNT_SERVER_TYPES, /// AccountServerTypes
	INFO_TYPE_GENDER, ///< u8, F = 0, M = 1
	INFO_TYPE_LAST_AUTHENTICATION_RESULT, ///< Result
	INFO_TYPE_ASSIGNED_ACCOUNT_ID, ///< AccountId
	INFO_TYPE_PARENTAL_CONTROL_SLOT_NUMBER, ///< u8
	INFO_TYPE_SIMPLE_ADDRESS_ID, ///< u32
	INFO_TYPE_STUB_0x18, /* not handled */
	INFO_TYPE_UTC_OFFSET, ///< s64
	INFO_TYPE_IS_COMMITTED, ///< bool
	INFO_TYPE_MII_NAME, ///< MiiScreenName
	INFO_TYPE_NFS_PASSWORD, ///< char[0x10+1]
	INFO_TYPE_HAS_ECI_VIRTUAL_ACCOUNT, ///< bool
	INFO_TYPE_TIMEZONE_ID, ///< char[0x40+1]
	INFO_TYPE_IS_MII_UPDATED, ///< bool
	INFO_TYPE_IS_MAIL_ADDRESS_VALIDATED, ///< bool
	INFO_TYPE_ACCOUNT_ACCESS_TOKEN, ///< AccountAccessToken structure
	INFO_TYPE_COMMON_IS_APPLICATION_UPDATE_REQUIRED, ///< bool
	INFO_TYPE_COMMON_DEFAULT_ACCOUNT_SERVER_TYPES, ///< AccountServerTypes
	INFO_TYPE_IS_SERVER_ACCOUNT_DELETED, ///< bool
	INFO_TYPE_MII_IMAGE_URL, ///< char[0x100+1]
	INFO_TYPE_ASSIGNED_PRINCIPAL_ID, ///< u32
	INFO_TYPE_ACCOUNT_ACCESS_TOKEN_STATE, ///< u32, AccountAccessTokenState enum
	INFO_TYPE_ACCOUNT_SERVER_ENVIRONMENT, ///< AccountServerTypesStr structure
	INFO_TYPE_COMMON_DEFAULT_ACCOUNT_SERVER_ENVIRONMENT, ///< AccountServerTypesStr structure
	INFO_TYPE_COMMON_DEVICE_HASH, ///< u8[8]
	INFO_TYPE_FP_LOCAL_ACCOUNT_ID, ///< u8
	INFO_TYPE_AGE, ///< u16
	INFO_TYPE_IS_ENABLED_RECEIVE_ADS, ///< bool
	INFO_TYPE_IS_OFF_DEVICE_ENABLED, ///< bool
	INFO_TYPE_TRANSLATED_SIMPLE_ADDRESS_ID, ///< u32
} ACT_InfoType;

typedef enum
{
	REQUEST_INQUIRE_BINDING_TO_EXISTENT_SERVER_ACCOUNT = 1, ///< ExistentServerAccountData struct
	REQUEST_BIND_TO_EXISTENT_SERVER_ACCOUNT, ///< u32, parentalConsentApprovalId
	REQUEST_ACQUIRE_EULA, ///< EulaList structure (dynamically sized)
	REQUEST_ACQUIRE_EULA_LIST = REQUEST_ACQUIRE_EULA, ///< EulaList structure (dynamically sized)
	REQUEST_ACQUIRE_EULA_LANGUAGE_LIST = REQUEST_ACQUIRE_EULA, ///< EulaList structure with only the languageNameOffsets populated (dynamically sized)
	REQUEST_ACQUIRE_TIMEZONE_LIST, ///< TimezoneList structure
	REQUEST_ACQUIRE_ACCOUNT_INFO, ///< INFO_TYPE_MAIL_ADDRESS: AccountMailAddress
	REQUEST_ACQUIRE_ACCOUNT_ID_BY_PRINCIPAL_ID_MULTI, ///< AccountId[count]
	REQUEST_ACQUIRE_ACCOUNT_ID_BY_PRINCIPAL_ID, ///< AccountId
	REQUEST_ACQUIRE_PRINCIPAL_ID_BY_ACCOUNT_ID_MULTI, ///< u32[count]
	REQUEST_ACQUIRE_PRINCIPAL_ID_BY_ACCOUNT_ID, ///< u32
	REQUEST_APPROVE_BY_CREDIT_CARD, ///< u32, approvalId
	REQUEST_SEND_COPPA_CODE_MAIL, ///< CoppaCodeMailData structure
	REQUEST_ACQUIRE_MII, ///< CFLStoreData[count]
	REQUEST_ACQUIRE_ACCOUNT_INFO_RAW, ///< char[0xC00+1], NULL-terminate ASCII raw profile XML data
} ACT_AsyncRequestType;

/// Enum for Mii image type
typedef enum
{
	MII_IMAGE_PRIMARY = 0, ///< The user's primary Mii image.
	MII_IMAGE_1,
	MII_IMAGE_2,
	MII_IMAGE_3,
	MII_IMAGE_4,
	MII_IMAGE_5,
	MII_IMAGE_6,
	MII_IMAGE_7,
	MII_IMAGE_8,
} MiiImageType;

/// Enum for NNAS (Nintendo Network Authentication Server) type
typedef enum
{
	NNAS_PRODUCTION = 0,
	NNAS_GAME_DEVELOPMENT,
	NNAS_SYSTEM_DEVELOPMENT,
	NNAS_LIBRARY_DEVELOPMENT,
	NNAS_STAGING,
} NnasServerType;

/// Enum for account access token state
typedef enum
{
	ACCESS_TOKEN_UNINITIALIZED = 0,
	ACCESS_TOKEN_EXPIRED,
	ACCESS_TOKEN_VALID,
} AccountAccessTokenState;

/// Enum for account access token invalidation action
typedef enum
{
	INVALIDATE_ACCESS_TOKEN  = BIT(0), ///< Invalidates only the account token itself (and the expiry date).
	INVALIDATE_REFRESH_TOKEN = BIT(1), ///< Invalidates only the refresh token.
} InvalidateAccessTokenAction;

/// Coppa Code Mail Data Structure
typedef struct
{
	char coppaCode[5+1];
	AccountMailAddress parentEmail;
} CoppaCodeMailData;

#pragma pack(push, 1)

/// Mii CFLStoreData (CTR Face Library Store Data) structure
typedef struct
{
	MiiData miiData;
	u8 pad[2];
	u16 crc16;
} CFLStoreData;

/// Birth date structure
typedef struct
{
	u16 year;
	u8 month;
	u8 day;
} BirthDate;

/// Account info structure
typedef struct
{
	u32 persistentId;
	u8 pad[4];
	u64 transferableIdBase;
	CFLStoreData mii;
	MiiScreenName screenName;
	char accountId[ACCOUNT_ID_LEN];
	u8 pad2;
	BirthDate birthDate;
	u32 principalId;
} AccountInfo;

/// Account Timezone structure
typedef struct
{
	char timezoneArea[0x40+1];
	char pad[3];
	char timezoneId[0x40+1];
	char pad2[3];
	s64 utcOffset;
} AccountTimezone;

/// Timezone List structure
typedef struct
{
	u32 capacity;
	u32 count;
	AccountTimezone timezones[32];
} TimezoneList;

/// EULA Info structure
typedef struct
{
	char countryCode[2+1]; ///< ISO 3166-1 A-2 country code
	char languageCode[2+1]; ///< ISO 639 Set 1 language code
	u16 eulaVersion;
} EulaInfo;

/// Existent Server Account Data structure
typedef struct
{
	bool hasMii;
	u8 pad[3];
	CFLStoreData miiData;
	u32 principalId;
	bool coppaRequiredFlag;
	u8 pad2[3];
	CoppaCodeMailData coppaMailData;
	u8 pad3;
	BirthDate birthDate;
} ExistentServerAccountData;

/// EULA entry header structure
typedef struct
{
	char countryCode[2+1]; ///< ISO 3166-1 A-2 country code
	u8 pad;
	char languageCode[2+1]; ///< ISO 639 Set 1 language code
	u8 pad2;
	u16 eulaVersion;
	u8 pad3[2];
	u32 nextEntryOffset; ///< Offset of next EULA entry, relative to full data blob.
	u32 eulaTypeOffset; ///< Offset of the EulaType within textData.
	u32 agreeTextOffset; ///< Offset of the AgreeText within textData.
	u32 nonAgreeTextOffset; ///< Offset of the NonAgreeText within textData.
	u32 languageNameOffset; ///< Offset of the LanguageName within textData.
	u32 mainTitleOffset; ///< Offset of the MainTitle within textData.
	u32 mainTextOffset; ///< Offset of the MainText within textData.
	u32 subTitleOffset; ///< Offset of the SubTitle within textData.
	u32 subTextOffset; ///< Offset of the SubText within textData.
	char textData[];
} EulaEntry;

/// Support Context structure
typedef struct
{
	AccountId accountId; ///< Account ID of the account.
	u8 pad[3];
	u32 serialNumber; ///< Serial number of the console (only digits).
	u32 principalId;
	u16 randomNumber; ///< Random number based on the principalId and serialNumber.
	u8 pad2[2];
} SupportContext;

#pragma pack(pop)

/// EULA list structure
typedef struct
{
	u8 numEntries; ///< Number of entries within the list.
	u8 entries[]; ///< EULA Entries (dynamically sized)
} EulaList;

/// Device Info structure
typedef struct
{
	u32 deviceId;
	char serialNumber[0xF+1];
} DeviceInfo;

/// NEX service token structure
typedef struct
{
	char serviceToken[0x200+1];
	u8 pad[3];
	char password[0x40+1];
	u8 pad2[3];
	char serverHost[0x10];
	u16 serverPort;
	u8 pad3[2];
} NexServiceToken;

/// Credit Card Info structure
typedef struct
{
	u8 cardType;
	char cardNumber[0x10+1];
	char securityCode[3+1];
	u8 expirationMonth;
	u8 expirationYear;
	char postalCode[6+1];
	AccountMailAddress mailAddress;
} CreditCardInfo;

/// V1 Independent service token structure
typedef struct
{
	char token[0x200+1]; ///< base64
} IndependentServiceTokenV1;

/// V2 Independent service token structure
typedef struct
{
	char token[0x200+1]; ///< base64
	char iv[0x18+1]; ///< base64
	char signature[0x158+1]; ///< base64
	NfsTypeStr nfsTypeStr;
} IndependentServiceTokenV2;

/// Account server types structure (raw format)
typedef struct
{
	u8 nnasType; ///< NNAS (Nintendo Network Authentication Server) type
	u8 nfsType; ///< NFS (Nintendo Friend Server) type
	u8 nfsNo; ///< NFS (Nintendo Friend Server) number
	u8 pad;
} AccountServerTypes;

/// Account server types structure (string format)
typedef struct
{
	ActNnasSubdomain nnasSubdomain; ///< NNAS (Nintendo Network Authentication Server) subdomain
	NfsTypeStr nfsTypeStr; ///< NFS (Nintendo Friend Server) type string (letter + number)
} AccountServerTypesStr;

/// Account access token structure
typedef struct
{
	u8 state; ///< AccountAccessTokenState enum
	char accessToken[0x20+1];
	char refreshToken[0x28+1];
	u8 pad;
} AccountAccessToken;

/**
 * @brief Initializes ACT services.
 * @param forceUser Whether or not to force using the user service act:u instead of the default (admin service act:a).
 */
Result actInit(bool forceUser);

/// Exits ACT services.
void actExit(void);

/// Get the ACT user/admin service handle.
Handle *actGetSessionHandle(void);

/**
 * @brief Initializes the current ACT session.
 * @param sdkVersion The SDK version of the client process.
 * @param sharedMemSize The size of the shared memory block.
 * @param sharedMem Handle to the shared memory block.
 */
Result ACT_Initialize(u32 sdkVersion, u32 sharedMemSize, Handle sharedMem);

/**
 * @brief Returns a support error code (XXX-YYYY) for the given ACT result code.
 * @param code The result code to convert.
 */
Result ACT_ResultToErrorCode(Result code);

/**
 * @brief Gets the result of the last internal operation.
 */
Result ACT_GetLastResponseResult();

/**
 * @brief Cancels any currently running async operation.
 */
Result ACT_Cancel();

/**
 * @brief Retrieves information not specific to any one account.
 * @param output Pointer to buffer to output the data to.
 * @param outputSize Size of the output buffer.
 * @param infoType The type of data to retrieve.
 */
Result ACT_GetCommonInfo(void *output, u32 outputSize, u32 infoType);

/**
 * @brief Retrieves information of a certain account.
 * @param output Pointer to buffer to output the data to.
 * @param outputSize Size of the output buffer.
 * @param accountSlot The account slot number of the account to retrieve information for.
 * @param infoType The type of data to retrieve.
 */
Result ACT_GetAccountInfo(void *output, u32 outputSize, u8 accountSlot, u32 infoType);

/**
 * @brief Returns the data resulting from an async request.
 * @param outReadSize Pointer to output the number of retrieved bytes to.
 * @param output Pointer to buffer to output the data to.
 * @param outputSize Size of the output buffer.
 * @param requestType The type of async request to retrieve data for.
 */
Result ACT_GetAsyncResult(u32 *outReadSize, void *output, u32 outputSize, u32 requestType);

/**
 * @brief Gets one of the Mii images of a certain account.
 * @param outSize Pointer to output the raw size of the image to.
 * @param output Pointer to output the image data to.
 * @param outputSize Size of the output buffer.
 * @param accountSlot The account slot number of the account to get the Mii image for.
 * @param miiImageType The type of the Mii image to get.
 */
Result ACT_GetMiiImage(u32 *outSize, void *output, u32 outputSize, u8 accountSlot, u8 miiImageType);

/**
 * @brief Sets the NFS (Nintendo Friend Server) password for a certain account.
 * @param accountSlot The account slot number of the account to set the NfsPassword for.
 * @param password Pointer to the new NFS password to use.
 */
Result ACT_SetNfsPassword(u8 accountSlot, char *password);

/**
 * @brief Sets the `IsApplicationUpdateRequired` field in the internal account manager.
 * @param required The new value to use.
 */
Result ACT_SetIsApplicationUpdateRequired(bool required);

/**
 * @brief Acquires a list of EULA agreements for the specified country.
 * @param countryCode The country code of the country to acquire EULA agreements for.
 * @param completionEvent The event handle to signal when the request has finished.
 */
Result ACT_AcquireEulaList(u8 countryCode, Handle completionEvent);

/**
 * @brief Acquires a list of timezones for the specified country and language combination.
 * @param countryCode The country code of the country to acquire time zones for.
 * @param language code The language code of the language to acquire the time zones in.
 * @param completionEvent The event handle to signal when the request has finished.
 */
Result ACT_AcquireTimezoneList(u8 countryCode, u8 languageCode, Handle completionEvent);

/**
 * @brief Generates a UUID.
 * @param uuid Pointer to output the generated UUID to.
 * @param uniqueId The unique ID to use during generation. Special values include `ACT_UUID_REGULAR` and `ACT_UUID_CURRENT_PROCESS`.
 */
Result ACT_GenerateUuid(ActUuid *uuid, u32 uniqueId);

/**
 * @brief Gets a specific account's UUID.
 * @param uuid Pointer to output the UUID to.
 * @param uniqueId The unique ID to use during the retrieval of the UUID. Special values include `ACT_UUID_REGULAR` and `ACT_UUID_CURRENT_PROCESS`.
 */
Result ACT_GetUuid(ActUuid *uuid, u8 accountSlot, u32 uniqueId);

/**
 * @brief Finds the account slot number of the account having the specified UUID.
 * @param accountSlot Pointer to output the account slot number to.
 * @param uuid Pointer to the UUID to find the account slot number for.
 * @param uniqueId The unique ID to use during internal UUID generation. Special values include `ACT_UUID_REGULAR` and `ACT_UUID_CURRENT_PROCESS`.
 */
Result ACT_FindSlotNoByUuid(u8 *accountSlot, ActUuid *uuid, u32 uniqueId);

/**
 * @brief Saves all pending changes to the ACT system save data.
 */
Result ACT_Save();

/**
 * @brief Returns a TransferableID for a certain account.
 * @param transferableId Pointer to output the generated TransferableID to.
 * @param accountSlot The account slot number of the account to generate the TransferableID for. Special values include `ACT_TRANSFERABLE_ID_BASE_COMMON` and `ACT_TRANSFERABLE_ID_BASE_CURRENT_ACCOUNT`.
 * @param saltValue The value to use as a salt during generation.
 */
Result ACT_GetTransferableId(u64 *transferableId, u8 accountSlot, u8 saltValue);

/**
 * @brief Acquires an account-specific service token for a NEX server.
 * @param accountSlot The account slot number of the account to use for acquiring the token.
 * @param serverId The NEX server ID to request a service token for.
 * @param doParentalControlsCheck Whether or not to perform a parental controls check before requesting the token. (unused)
 * @param callerProcessId The process ID of the process requesting the token.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACT_AcquireNexServiceToken(u8 accountSlot, u32 serverId, bool doParentralControlsCheck, u32 callerProcessId, Handle completionEvent);

/**
 * @brief Gets a NEX service token requested using ACT_AcquireNexServiceToken.
 * @param token Pointer to output the NEX service token data to.
 */
Result ACT_GetNexServiceToken(NexServiceToken *token);

/**
 * @brief Requests a V1 independent service token for a specific account.
 * @param accountSlot The account slot number of the account to request the token with.
 * @param clientId The client ID to use for requesting the independent service token.
 * @param cacheDuration The duration in seconds to cache the token. If a token was requested within the past cacheDuration seconds, this command returns that token instead of requesting a new one. Passing 0 will cause ACT to always request a new token.
 * @param doParentalControlsCheck Whether or not to perform a parental controls check before requesting the token. (unused)
 * @param shared Whether or not this token should be shared with other processes. If set to false, it will only be accessible to the process with the given process ID.
 * @param callerProcessId The process ID of the process requesting the token.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACT_AcquireIndependentServiceToken(u8 accountSlot, char *clientId, u32 cacheDuration, bool doParentalControlsCheck, bool shared, u32 callerProcessId, Handle completionEvent);

/**
 * @brief Gets a V1 independent service token requested using ACT_AcquireIndependentServiceToken.
 * @param token Pointer to output the V1 independent service token to.
 */
Result ACT_GetIndependentServiceToken(IndependentServiceTokenV1 *token);

/**
 * @brief Acquires account information for the specified account.
 * @param accountSlot The account slot number to acquire information for.
 * @param infoType The type of info to obtain. (only INFO_TYPE_MAIL_ADDRESS is supported.)
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACT_AcquireAccountInfo(u8 accountSlot, u32 infoType, Handle completionEvent);

/**
 * @brief Acquires account IDs from a list of principal IDs.
 * @param principalIds Pointer to the input principal IDs.
 * @param principalIdsSize Size of the input principal IDs buffer.
 * @param unk Unknown value. Must be 0.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACT_AcquireAccountIdByPrincipalId(u32 *principalIds, u32 principalIdsSize, u8 unk, Handle completionEvent);

/**
 * @brief Acquires principal IDs from a list of account IDs.
 * @param accountIds Pointer to input account IDs.
 * @param accountIdsSize Size of the input account IDs buffer.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACT_AcquirePrincipalIdByAccountId(AccountId *accountIds, u32 accountIdsSize, Handle completionHandle);

/**
 * @brief Acquires Miis corresponding to a given list of persistent IDs.
 * @param persistentIds Pointer to input persistent IDs to use.
 * @param persistentIdsSize Size of the input persistent IDs buffer.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACT_AcquireMii(u32 *persistentIds, u32 persistentIdsSize, Handle completionEvent);

/**
 * @brief Acquires raw (XML) account info for the specified account.
 * @param accountSlot The account slot number of the account to acquire raw info for.
 */
Result ACT_AcquireAccountInfoRaw(u8 accountSlot, Handle completionEvent);

/**
 * @brief Gets a cached V1 independent service token for a specific account.
 * @param accountSlot The account slot number of the account to get the token for.
 * @param clientId The client ID to use for the cache lookup.
 * @param cacheDuration The duration in seconds ago this token must have been requested in at least for it to be eligible for retrieval.
 * @param doParentalControlsCheck Whether or not to perform a parental controls check before getting the token. (unused)
 * @param shared Whether or not to only look for shared (non-process-specific) tokens in the cache.
 */
Result ACT_GetCachedIndependentServiceToken(IndependentServiceTokenV1 *token, u8 accountSlot, char *clientId, u32 cacheDuration, bool doParentralControlsCheck, bool shared);

/**
 * @brief Inquires whether or not the given email address is available for creating a new account.
 * @param mailAddress Pointer to the input email address to check.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACT_InquireMailAddressAvailability(AccountMailAddress *mailAddress, Handle completionEvent);

/**
 * @brief Acquires the EULA for the given country and language combination.
 * @param countryCode The country code of the country for the EULA.
 * @param languageCode The 2-character ISO 639 Set 1 language code for the EULA.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACT_AcquireEula(u8 countryCode, char *languageCode, Handle completionEvent);

/**
 * @brief Acquires a list of languages the EULA is available in for a given country.
 * @param countryCode The country code to acquire the EULA language list for.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACT_AcquireEulaLanguageList(u8 countryCode, Handle completionEvent);

/**
 * @brief Requests a V2 independent service token for a specific account.
 * @param accountSlot The account slot number of the account to request the token with.
 * @param clientId The client ID to use for requesting the independent service token.
 * @param cacheDuration The duration in seconds to cache the token. If a token was requested within the past cacheDuration seconds, this command returns that token instead of requesting a new one. Passing 0 will cause ACT to always request a new token.
 * @param doParentalControlsCheck Whether or not to perform a parental controls check before requesting the token. (unused)
 * @param shared Whether or not this token should be shared with other processes. If set to false, it will only be accessible to the process with the given process ID.
 * @param callerProcessId The process ID of the process requesting the token.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACT_AcquireIndependentServiceTokenV2(u8 accountSlot, char *clientId, u32 cacheDuration, bool doParentalControlsCheck, bool shared, u32 callerProcessId, Handle completionEvent);

/**
 * @brief Gets a V2 independent service token requested using ACT_AcquireIndependentServiceTokenV2.
 * @param token Pointer to output the V2 independent service token to.
 */
Result ACT_GetIndependentServiceTokenV2(IndependentServiceTokenV2 *token);

/**
 * @brief Gets a cached V2 independent service token for a specific account.
 * @param accountSlot The account slot number of the account to get the token for.
 * @param clientId The client ID to use for the cache lookup.
 * @param cacheDuration The duration in seconds ago this token must have been requested in at least for it to be eligible for retrieval.
 * @param doParentalControlsCheck Whether or not to perform a parental controls check before getting the token. (unused)
 * @param shared Whether or not to only look for shared (non-process-specific) tokens in the cache.
 */
Result ACT_GetCachedIndependentServiceTokenV2(IndependentServiceTokenV2 *token, u8 accountSlot, char *clientId, u32 cacheDuration, bool doParentralControlsCheck, bool shared);

/**
 * @brief Swaps the account slot numbers of two accounts.
 * @param accountSlot1 The first account slot number.
 * @param accountSlot2 The second account slot number.
 */
Result ACTA_SwapAccounts(u8 accountSlot1, u8 accountSlot2);

/**
 * @brief Creates a new local console account.
 */
Result ACTA_CreateConsoleAccount();

/**
 * @brief Sets a local console account as committed.
 * @param accountSlot The account slot number of the account to set as committed.
 */
Result ACTA_CommitConsoleAccount(u8 accountSlot);

/**
 * @brief Clears (but does not delete) account data for the given account slot. The FpLocalAccountId will not be cleared.
 * @param accountSlot The account slot number of the account to clear.
 * @param completely Whether or not to also clear the AssignedAccountId and AssignedPrincipalId in the account data.
 */
Result ACTA_UnbindServerAccount(u8 accountSlot, bool completely);

/**
 * @brief Deletes a local console account.
 * @param accountSlot The account slot number of the local console account to delete.
 */
Result ACTA_DeleteConsoleAccount(u8 accountSlot);

/**
 * @brief Loads ("logs in to") a local console account.
 * @param accountSlot The account slot number of the local console account to load.
 * @param doPasswordCheck Whether or not the check the input password, or if one isn't provided, the cached password (if enabled).
 * @param password Pointer to the input password.
 * @param useNullPassword Whether or not to forcefully use NULL as the password (= no password).
 * @param dryRun Whether or not to execute this command as a "dry run," not actually changing the current account to specified one.
 */
Result ACTA_LoadConsoleAccount(u8 accountSlot, bool doPasswordCheck, AccountPassword *password, bool useNullPassword, bool dryRun);

/**
 * @brief Unloads the currently loaded local console account.
 */
Result ACTA_UnloadConsoleAccount();

/**
 * @brief Enables or disables the account password cache for a specific account. When the account password cache is enabled, entering the password is not required to log into the account.
 * @param accountSlot The account slot number to enable/disable the account password cache for.
 * @param enabled Whether or not to enable the account password cache.
 */
Result ACTA_EnableAccountPasswordCache(u8 accountSlot, bool enabled);

/**
 * @brief Sets the default account that is loaded when the ACT module is initialized.
 * @param accountSlot The account slot number of the account to set as the default.
 */
Result ACTA_SetDefaultAccount(u8 accountSlot);

/**
 * @brief Replaces the AccountId with the AssignedAccountId for a specific account.
 * @param accountSlot The account slot number of the account to perform this replacement for.
 */
Result ACTA_ReplaceAccountId(u8 accountSlot);

/**
 * @brief Creates a support context for a specific account.
 * @param supportContext Pointer to write the support context data to.
 * @param accountSlot The account slot number of the account to create the support context for.
 */
Result ACTA_GetSupportContext(SupportContext *supportContext, u8 accountSlot);

/**
 * @brief Sets server environment settings for a specific account. This will also update CFG configuration block 0x150002 accordingly.
 * @param accountSlot The account slot number of the account to set the host server settings for.
 * @param nnasType The NNAS (Nintendo Network Authentication Server) type.
 * @param nfsType The NFS (Nintendo Friend Server) type.
 * @param nfsNo The NFS (Nintendo Friend Server) number.
 */
Result ACTA_SetHostServerSettings(u8 accountSlot, u8 nnasType, u8 nfsType, u8 nfsNo);

/**
 * @brief Sets default server environment settings. This will also update CFG configuration block 0x150002 accordingly.
 * @param nnasType The NNAS (Nintendo Network Authentication Server) type.
 * @param nfsType The NFS (Nintendo Friend Server) type.
 * @param nfsNo The NFS (Nintendo Friend Server) number.
 */
Result ACTA_SetDefaultHostServerSettings(u8 nnasType, u8 nfsType, u8 nfsNo);

/**
 * @brief Sets server environment settings (in string form) for a specific account. This will also update CFG configuration block 0x150002 accordingly.
 * @param accountSlot The account slot number of the account to set the host server settings for.
 * @param nnasSubdomain Pointer to the new NNAS (Nintendo Network Authentication Server) subdomain to use.
 * @param nfsTypeStr Pointer to the new NFS (Nintendo Friend Server) type to use.
 */
Result ACTA_SetHostServerSettingsStr(u8 accountSlot, ActNnasSubdomain *nnasSubdomain, NfsTypeStr *nfsTypeStr);

/**
 * @brief Sets default server environment settings (in string form). This will also update CFG configuration block 0x150002 accordingly.
 * @param nnasSubdomain Pointer to the new NNAS (Nintendo Network Authentication Server) subdomain to use.
 * @param nfsTypeStr Pointer to the new NFS (Nintendo Friend Server) type to use.
 */
Result ACTA_SetDefaultHostServerSettingsStr(ActNnasSubdomain *nnasSubdomain, NfsTypeStr *nfsTypeStr);

/**
 * @brief Sets the internal base value for generating new persistent IDs.
 * @param head The new base value to use.
 */
Result ACTA_SetPersistentIdHead(u32 head);

/**
 * @brief Sets the internal base value for generating new transferable IDs.
 * @param counter The new base value to use.
 */
Result ACTA_SetTransferableIdCounter(u16 counter);

/**
 * @brief Updates a specific account's Mii data and screen name.
 * @param accountSlot The account slot number of the account to update the Mii and screen name of.
 * @param miiData Pointer to the new Mii data to use.
 * @param screenName Pointer to the new screen name to use.
 */
Result ACTA_UpdateMiiData(u8 accountSlot, CFLStoreData *miiData, MiiScreenName *screenName);

/**
 * @brief Updates a Mii image of a specific account.
 * @param accountSlot The account slot number of the account to update the Mii image for.
 * @param miiImageType The type of Mii image to update.
 * @param image Pointer to the Mii image data to use.
 * @param imageSize Size of the Mii image data.
 */
Result ACTA_UpdateMiiImage(u8 accountSlot, u8 miiImageType, void *image, u32 imageSize);

/**
 * @brief Checks whether or not the given account ID is available for creating a new server account.
 * @param accountSlot The account slot number of the account to perform the check for.
 * @param accountId Pointer to the input account ID to check.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_InquireAccountIdAvailability(u8 accountSlot, AccountId *accountId, Handle completionEvent);

/**
 * @brief Links a new server account to a local console account. In other words, this creates and links an NNID.
 * @param accountSlot The account slot number of the local console account to bind.
 * @param accountId Pointer to the account ID to use for the new server account.
 * @param mailAddress Pointer to the email address to use for the new server account.
 * @param password Pointer to the password to use for the new server account.
 * @param isParentEmail Whether or not the input email address is a parental email address.
 * @param marketingFlag Whether or not the user has consented to receiving marketing emails. ("Customized Email Offers")
 * @param offDeviceFlag Whether or not the user has allowed using the server account from other devices. ("Access from PCs and Other Devices")
 * @param birthDateTimestamp A birth date timestamp in the format milliseconds elapsed since 01.01.2000 00:00:00 UTC.
 * @param parentalConsentTimestamp When parental consent is required, the timestamp of parental consent in the format milliseconds elapsed since 01.01.2000 00:00:00 UTC.
 * @param parentalConsentId When parental consent is required, the resulting ID corresponding to the consent.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_BindToNewServerAccount(u8 accountSlot, AccountId *accountId, AccountMailAddress *mailAddress, AccountPassword *password, bool isParentEmail, bool marketingFlag, bool offDeviceFlag, s64 birthDateTimestatmp, u8 gender, u32 region, AccountTimezone *timezone, EulaInfo *eulaInfo, s64 parentalConsentTimestamp, u32 parentalConsentId, Handle completionEvent);

/**
 * @brief Links a local console account to an existing server account. In other words, this links an existing NNID.
 * @param accountSlot The account slot number of the local console account to bind.
 * @param accountId Pointer to the account ID of the existing server account.
 * @param mailAddress Pointer to the email address of the existing server account.
 * @param password Pointer to the password of the existing server account.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_BindToExistentServerAccount(u8 accountSlot, AccountId *accountId, AccountMailAddress *mailAddress, AccountPassword *password, Handle completionEvent);

/**
 * @brief Acquires information about an existing server account.
 * @param accountSlot The account slot number of the local console account to use for the request.
 * @param accountId Pointer to the account ID of the existing server account.
 * @param mailAddress Pointer to the email address of the existing server account.
 * @param password Pointer to the password of the existing server account.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_InquireBindingToExistentServerAccount(u8 accountSlot, AccountId *accountId, AccountMailAddress *mailAddress, AccountPassword *password, Handle completionEvent);

/**
 * @brief Deletes a server account. In other words, this deletes an NNID (server-side).
 * @param accountSlot The account slot number of the local console account bound to the server account to delete.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_DeleteServerAccount(u8 accountSlot, Handle completionEvent);

/**
 * @brief Acquires an account token for a specific account.
 * @param accountSlot The account slot number of the account to acquire the account token for.
 * @param password Pointer to the password of the account.
 * @param useNullPassword Whether or not to force NULL as the password (no password). This will cause the account password cache to be used instead, if it is enabled.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_AcquireAccountTokenEx(u8 accountSlot, AccountPassword *password, bool useNullPassword, Handle completionEvent);

/**
 * @brief Submits a EULA agreement to the account server.
 * @param accountSlot The account slot number of the account to use to submit the agreement.
 * @param eulaInfo Pointer to a EULA information structure describing the agreed EULA.
 * @param agreementTimestamp A timestamp in the format milliseconds elapsed since 01.01.2000 00:00:00 UTC of when the user agreed to the EULA.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_AgreeEula(u8 accountSlot, EulaInfo *eulaInfo, s64 agreementTimestamp, Handle completionEvent);

/**
 * @brief Reloads account information from the server for a specific account.
 * @param accountSlot The account slot number of the account to reload information for.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_SyncAccountInfo(u8 accountSlot, Handle completionEvent);

/**
 * @brief Invalidates a specific account's access token in different ways.
 * @param accountSlot The account slot number of the account to invalidate the access token for.
 * @param invalidationMask A bitfield of the actions to take to invalidate the access token.
 */
Result ACTA_InvalidateAccountToken(u8 accountSlot, u32 invalidationActionMask);

/**
 * @brief Updates the account password for a specific account.
 * @param accountSlot The account slot number of the account to update the password for.
 * @param password Pointer to the new password to use.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_UpdateAccountPassword(u8 accountSlot, AccountPassword *newPassword, Handle completionEvent);

/**
 * @brief Requests the issuing of a temporary password (valid for 24 hours) to the email address associated with a specific account.
 * @param accountSlot The account slot number of the account to issue the temporary password for.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_ReissueAccountPassword(u8 accountSlot, Handle completionEvent);

/**
 * @brief Sets the account password input for a specific account. This value is not stored in the save data and only resides in memory. Following up a call to this command with a call to ACTA_EnableAccountPasswordCache will lead to the account password cache being updated.
 */
Result ACTA_SetAccountPasswordInput(u8 accountSlot, AccountPassword *passwordInput);

/**
 * @brief Uploads the Mii data of a specific account to the account server.
 * @param accountSlot The account slot number of the account to upload the Mii for.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_UploadMii(u8 accountSlot, Handle completionEvent);

/**
 * @brief Inactivates the device association for a specific account.
 * @param accountSlot The account slot number of the account to inactive the device association for.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_InactivateDeviceAssociation(u8 accountSlot, Handle completionEvent);

/**
 * @brief Validates the email address of a specific account using the code received via the confirmation email.
 * @param accountSlot The account slot number of the account to validate the email address for.
 * @param confirmationCode The confirmation code received via email.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_ValidateMailAddress(u8 accountSlot, u32 confirmationCode, Handle completionEvent);

/**
 * @brief Requests parental approval for a specific account.
 * @param accountSlot The account slot number of the account to request parental approval for.
 * @param parentalEmail Pointer to a parental email to use for parental consent.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_SendPostingApprovalMail(u8 accountSlot, AccountMailAddress *parentalEmail, Handle completionEvent);

/**
 * @brief Requests the email address confirmation mail to be resent for a specific account.
 * @param accountSlot The account slot number of the account for which the confirmation mail should be resent.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_SendConfirmationMail(u8 accountSlot, Handle completionEvent);

/**
 * @brief Registers a parental email address to be used in case the parental controls PIN has been forgotten for a specific account.
 * @param accountSlot The account slot number of the account to register the fallback parental email for.
 * @param parentalEmail Pointer to the parental email to use.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_SendConfirmationMailForPin(u8 accountSlot, AccountMailAddress *parentalEmail, Handle completionEvent);

/**
 * @brief Sends the master key for resetting parental controls to a parental email for a specific account.
 * @param accountSlot The account slot number for the account to be used for this operation.
 * @param masterKey The master key to send to the parental email address.
 * @param parentalEmail Pointer to the parental email address to send the master key to.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_SendMasterKeyMailForPin(u8 accountSlot, u32 masterKey, AccountMailAddress *parentalEmail, Handle completionEvent);

/**
 * @brief Requests COPPA parental consent using credit card information.
 * @param accountSlot The account slot number for the account to request approval for.
 * @param cardInfo Pointer to the credit card information to use for the approval process.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_ApproveByCreditCard(u8 accountSlot, CreditCardInfo *cardInfo, Handle completionEvent);

/**
 * @brief Requests a COPPA code for a specific account.
 * @param accountSlot The account slot number for the account to send the request for.
 * @param principalId The principalId of the account to send the request for.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_SendCoppaCodeMail(u8 accountSlot, u32 principalId, Handle completionEvent);

/**
 * @brief Set a flag in a specifc account's data that determines whether or not it is necessary to upload the account Mii data to the account server.
 * @param accountSlot The account slot number of the account to set the flag for.
 * @param isDirty Whether or not the Mii data should be reuploaded to the account server.
 */
Result ACTA_SetIsMiiUpdated(u8 accountSlot, bool isDirty);

/**
 * @brief Initializes a server account transfer of a specific account to another device.
 * @param accountSlot The account slot number of the account to transfer the server account of.
 * @param newDevice Pointer to device info of the target device.
 * @param operatorData Pointer to operator data for the transfer.
 * @param operatorSize Size of the operator data buffer (max: 0x100)
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_ReserveTransfer(u8 accountSlot, DeviceInfo *newDevice, char *operatorData, u32 operatorSize, Handle completionEvent);

/**
 * @brief Finalizes a server account transfer of a specifc account to another device.
 * @param accountSlot The account slot number of the account to complete the transfer for.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_CompleteTransfer(u8 accountSlot, Handle completionEvent);

/**
 * @brief Inactivates the account-device association for a specific account. In other words, this deletes an NNID.
 * @param accountSlot The account slot number of the account to perform this action on.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_InactivateAccountDeviceAssociation(u8 accountSlot, Handle completionEvent);

/**
 * @brief Set the internal network time field.
 * @param timestamp The new server time timestamp to use. The timestamp format is milliseconds elapsed since 01.01.2000 00:00:00 UTC.
 */
Result ACTA_SetNetworkTime(s64 timestamp);

/**
 * @brief Updates the account info of a specific account using raw XML data.
 * @param accountSlot The account slot number of the account to update information for.
 * @param xmlData Pointer to the input XML data.
 * @param xmlDataSize Size of the input XML data.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_UpdateAccountInfo(u8 accountSlot, char *xmlData, u32 xmlDataSize, Handle completionEvent);

/**
 * @brief Updates the email address of a specific account.
 * @param accountSlot The account slot number of the account to update the email address for.
 * @param newEmail Pointer to the new email address to use.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_UpdateAccountMailAddress(u8 accountSlot, AccountMailAddress *newEmail, Handle completionEvent);

/**
 * @brief Deletes the device association for a specific account.
 * @param accountSlot The account slot of the account to perform this action on.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_DeleteDeviceAssociation(u8 accountSlot, Handle completionEvent);

/**
 * @brief Deletes the account-device association for a specific account.
 * @param accountSlot The account slot of the account to perform this action on.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_DeleteAccountDeviceAssociation(u8 accountSlot, Handle completionEvent);

/**
 * @brief Cancels a pending server account transfer of a specific account to another device.
 * @param accountSlot The account slot number of the account to cancel the transfer for.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_CancelTransfer(u8 accountSlot, Handle completionEvent);

/**
 * @brief Cancels any running HTTP requests, saves all pending changes to the system save data, then signals unloadFinishedEvent. Then, waits for remountAndBlockEvent, and once this has been signaled, remounts the system save, and blocks subsequent attempts to save the system save data (which can be bypassed by entering and exiting sleep mode).
 * @param unloadFinishedEvent The event handle for ACT to signal once it has saved pending changes and has unmounted its system save.
 * @param remountAndBlockEvent The event handle for the caller to signal once ACT should remount its save data and block subsequent save attempts.
 */
Result ACTA_ReloadAndBlockSaveData(Handle unloadFinishedEvent, Handle remountAndBlockEvent);

/**
 * @brief Initializes server-side account deletion for a specific account. In other words, this deletes an NNID.
 * @param accountSlot The account slot number of the account to perform this action on.
 * @param completionEvent The event handle to signal once the request has finished.
 */
Result ACTA_ReserveServerAccountDeletion(u8 accountSlot, Handle completionEvent);
