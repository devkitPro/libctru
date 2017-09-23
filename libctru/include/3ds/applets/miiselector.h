/**
 * @file miiselector.h
 * @brief Mii Selector Applet (appletEd).
 */

#pragma once

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
	char enable_cancel_button;        ///< Enables canceling of selection if nonzero.
	char enable_selecting_guests;     ///< Makes Guets Miis selectable if nonzero.
	char show_on_top_screen;          ///< Shows applet on top screen if nonzero,
	                                  ///< otherwise show it on the bottom screen.
	char _unk0x3[5];                  ///< @private
	u16 title[MIISELECTOR_TITLE_LEN]; ///< UTF16-LE string displayed at the top of the applet. If
	                                  ///< set to the empty string, a default title is displayed.
	char _unk0x88[4];                 ///< @private
	char show_guest_page;             ///< If nonzero, the applet shows a page with Guest
	                                  ///< Miis on launch.
	char _unk0x8D[3];                 ///< @private
	u32 initial_index;                ///< Index of the initially selected Mii. If
	                                  ///< @ref MiiSelectorConf.show_guest_page is
	                                  ///< set, this is the index of a Guest Mii,
	                                  ///< otherwise that of a user Mii.
	char mii_guest_whitelist[MIISELECTOR_GUESTMII_SLOTS]; ///< Each byte set to a nonzero value
	                                                      ///< enables its corresponding Guest
	                                                      ///< Mii to be enabled for selection.
	char mii_whitelist[MIISELECTOR_USERMII_SLOTS]; ///< Each byte set to a nonzero value enables
	                                               ///< its corresponding user Mii to be enabled
	                                               ///< for selection.
	u16 _unk0xFE;                                  ///< @private
	u32 magic; ///< Will be set to @ref MIISELECTOR_MAGIC before launching the
	           ///< applet.
} MiiSelectorConf;

/// Size of the data describing a single Mii
#define MIISELECTOR_MIIDATA_SIZE 0x5c

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
	char mii[MIISELECTOR_MIIDATA_SIZE]; ///< Data of selected Mii.
	u16 _pad0x68;                       ///< @private
	u16 checksum;                       ///< Checksum of the returned Mii data.
	                                    ///< Stored as a big-endian value; use
	                                    ///< @ref miiSelectorChecksumIsValid to
	                                    ///< verify.
	u16 guest_mii_name[MIISELECTOR_GUESTMII_NAME_LEN]; ///< Localized name of a Guest Mii,
	                                                   ///< if one was selected (UTF16-LE
	                                                   ///< string). Zeroed otherwise.
} MiiSelectorReturn;

/**
 * @brief Launch the Mii selector library applet
 *
 * @param conf Configuration determining how the applet should behave
 * @param returnbuf Data returned by the applet
 */
Result miiSelectorLaunch(const MiiSelectorConf *conf, MiiSelectorReturn* returnbuf);

/**
 * @brief Verifies that the Mii data returned from the applet matches its
 * checksum
 *
 * @param returnbuffer Buffer filled by Mii selector applet
 *
 * @return `true` if `returnbuf->checksum` is the same as the one computed from `returnbuf`
 */
bool miiSelectorChecksumIsValid(const MiiSelectorReturn *returnbuf);
