/**
 * @file miiselector.h
 * @brief Mii Selector Applet (appletEd).
 */

#pragma once
#include <3ds/types.h>
#include <3ds/mii.h>

/// Magic value needed to launch the applet.
#define MIISELECTOR_MAGIC 0x13DE28CF

/// Maximum length of title to be displayed at the top of the Mii selector applet
#define MIISELECTOR_TITLE_LEN 64

/// Number of Guest Miis available for selection
#define MIISELECTOR_GUESTMII_SLOTS 6

/// Maximum number of user Miis available for selection
#define MIISELECTOR_USERMII_SLOTS 100

/// Parameter structure passed to AppletEd
typedef struct
{
	u8 enable_cancel_button;          ///< Enables canceling of selection if nonzero.
	u8 enable_selecting_guests;       ///< Makes Guets Miis selectable if nonzero.
	u8 show_on_top_screen;            ///< Shows applet on top screen if nonzero,
	                                  ///< otherwise show it on the bottom screen.
	u8 _unk0x3[5];                    ///< @private
	u16 title[MIISELECTOR_TITLE_LEN]; ///< UTF16-LE string displayed at the top of the applet. If
	                                  ///< set to the empty string, a default title is displayed.
	u8 _unk0x88[4];                   ///< @private
	u8 show_guest_page;               ///< If nonzero, the applet shows a page with Guest
	                                  ///< Miis on launch.
	u8 _unk0x8D[3];                   ///< @private
	u32 initial_index;                ///< Index of the initially selected Mii. If
	                                  ///< @ref MiiSelectorConf.show_guest_page is
	                                  ///< set, this is the index of a Guest Mii,
	                                  ///< otherwise that of a user Mii.
	u8 mii_guest_whitelist[MIISELECTOR_GUESTMII_SLOTS];   ///< Each byte set to a nonzero value
	                                                      ///< enables its corresponding Guest
	                                                      ///< Mii to be enabled for selection.
	u8 mii_whitelist[MIISELECTOR_USERMII_SLOTS];   ///< Each byte set to a nonzero value enables
	                                               ///< its corresponding user Mii to be enabled
	                                               ///< for selection.
	u16 _unk0xFE;                                  ///< @private
	u32 magic; ///< Will be set to @ref MIISELECTOR_MAGIC before launching the
	           ///< applet.
} MiiSelectorConf;

/// Maximum length of the localized name of a Guest Mii
#define MIISELECTOR_GUESTMII_NAME_LEN 12

/// Structure written by AppletEd
typedef struct
{
	u32 no_mii_selected;                ///< 0 if a Mii was selected, 1 if the selection was
	                                    ///< canceled.
	u32 guest_mii_was_selected;         ///< 1 if a Guest Mii was selected, 0 otherwise.
	u32 guest_mii_index;                ///< Index of the selected Guest Mii,
	                                    ///< 0xFFFFFFFF if no guest was selected.
	MiiData mii;                        ///< Data of selected Mii.
	u16 _pad0x68;                       ///< @private
	u16 checksum;                       ///< Checksum of the returned Mii data.
	                                    ///< Stored as a big-endian value; use
	                                    ///< @ref miiSelectorChecksumIsValid to
	                                    ///< verify.
	u16 guest_mii_name[MIISELECTOR_GUESTMII_NAME_LEN]; ///< Localized name of a Guest Mii,
	                                                   ///< if one was selected (UTF16-LE
	                                                   ///< string). Zeroed otherwise.
} MiiSelectorReturn;

/// AppletEd options
enum
{
	MIISELECTOR_CANCEL     = BIT(0), ///< Show the cancel button
	MIISELECTOR_GUESTS     = BIT(1), ///< Make Guets Miis selectable
	MIISELECTOR_TOP        = BIT(2), ///< Show AppletEd on top screen
	MIISELECTOR_GUESTSTART = BIT(3), ///< Start on guest page
};

/**
 * @brief Initialize Mii selector config
 * @param conf Pointer to Miiselector config.
 */
void miiSelectorInit(MiiSelectorConf *conf);

/**
 * @brief Launch the Mii selector library applet
 *
 * @param conf Configuration determining how the applet should behave
 */
void miiSelectorLaunch(const MiiSelectorConf *conf, MiiSelectorReturn* returnbuf);

/**
 * @brief Sets title of the Mii selector library applet
 *
 * @param conf Pointer to miiSelector configuration
 * @param text Title text of Mii selector
 */
void miiSelectorSetTitle(MiiSelectorConf *conf, const char* text);

/**
 * @brief Specifies which special options are enabled in the Mii selector
 *
 * @param conf Pointer to miiSelector configuration
 * @param options Options bitmask
 */
void miiSelectorSetOptions(MiiSelectorConf *conf, u32 options);

/**
 * @brief Specifies which guest Miis will be selectable
 *
 * @param conf Pointer to miiSelector configuration
 * @param index Index of the guest Miis that will be whitelisted.
 * @ref MIISELECTOR_GUESTMII_SLOTS can be used to whitelist all the guest Miis.
 */
void miiSelectorWhitelistGuestMii(MiiSelectorConf *conf, u32 index);

/**
 * @brief Specifies which guest Miis will be unselectable
 *
 * @param conf Pointer to miiSelector configuration
 * @param index Index of the guest Miis that will be blacklisted.
 * @ref MIISELECTOR_GUESTMII_SLOTS can be used to blacklist all the guest Miis.
 */
void miiSelectorBlacklistGuestMii(MiiSelectorConf *conf, u32 index);

/**
 * @brief Specifies which user Miis will be selectable
 *
 * @param conf Pointer to miiSelector configuration
 * @param index Index of the user Miis that will be whitelisted.
 * @ref MIISELECTOR_USERMII_SLOTS can be used to whitlist all the user Miis
 */
void miiSelectorWhitelistUserMii(MiiSelectorConf *conf, u32 index);

/**
 * @brief Specifies which user Miis will be selectable
 *
 * @param conf Pointer to miiSelector configuration
 * @param index Index of the user Miis that will be blacklisted.
 * @ref MIISELECTOR_USERMII_SLOTS can be used to blacklist all the user Miis
 */
void miiSelectorBlacklistUserMii(MiiSelectorConf *conf, u32 index);

/**
 * @brief Specifies which Mii the cursor should start from
 *
 * @param conf Pointer to miiSelector configuration
 * @param index Indexed number of the Mii that the cursor will start on.
 * If there is no mii with that index, the the cursor will start at the Mii
 * with the index 0 (the personal Mii).
 */
static inline void miiSelectorSetInitialIndex(MiiSelectorConf *conf, u32 index)
{
	conf->initial_index = index;
}

/**
 * @brief Get Mii name
 *
 * @param returnbuf Pointer to miiSelector return
 * @param out String containing a Mii's name
 * @param max_size Size of string. Since UTF8 characters range in size from 1-3 bytes
 * (assuming that no non-BMP characters are used), this value should be 36 (or 30 if you are not
 * dealing with guest miis).
 */
void miiSelectorReturnGetName(const MiiSelectorReturn *returnbuf, char* out, size_t max_size);

/**
 * @brief Get Mii Author
 *
 * @param returnbuf Pointer to miiSelector return
 * @param out String containing a Mii's author
 * @param max_size Size of string. Since UTF8 characters range in size from 1-3 bytes
 * (assuming that no non-BMP characters are used), this value should be 30.
 */
void miiSelectorReturnGetAuthor(const MiiSelectorReturn *returnbuf, char* out, size_t max_size);

/**
 * @brief Verifies that the Mii data returned from the applet matches its
 * checksum
 *
 * @param returnbuf Buffer filled by Mii selector applet
 * @return `true` if `returnbuf->checksum` is the same as the one computed from `returnbuf`
 */
bool miiSelectorChecksumIsValid(const MiiSelectorReturn *returnbuf);
