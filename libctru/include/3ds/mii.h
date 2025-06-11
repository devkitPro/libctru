/**
 * @file mii.h
 * @brief Shared Mii struct.
 *
 * @see https://www.3dbrew.org/wiki/Mii#Mii_format
 */
#pragma once

#define MII_NAME_LEN           10+1  ///< 10-character NULL-terminated UTF-16 mii name

typedef u16 MiiScreenName[MII_NAME_LEN];

#include <3ds/types.h>

/// Shared Base Mii struct
typedef struct
{
	u8 version;     ///< Always 3

	/// Mii options
	struct
	{
		bool allow_copying : 1;   ///< True if copying is allowed
		bool is_private_name : 1; ///< Private name?
		u8 region_lock : 2;       ///< Region lock (0=no lock, 1=JPN, 2=USA, 3=EUR)
		u8 char_set : 2;          ///< Character set (0=JPN+USA+EUR, 1=CHN, 2=KOR, 3=TWN)
		u8 _pad : 2;
	} mii_options;

	/// Mii position in Mii selector or Mii maker
	struct
	{
		u8 page_index : 4;   ///< Page index of Mii
		u8 slot_index : 4;   ///< Slot offset of Mii on its Page
	} mii_pos;

	/// Console Identity
	struct
	{
		u8 unknown0 : 4;        ///< Mabye padding (always seems to be 0)?
		u8 origin_console : 3;  ///< Console that the Mii was created on (1=WII, 2=DSI, 3=3DS)
		u8 _pad : 1;
	} console_identity;

	u64 system_id;   ///< Identifies the system that the Mii was created on (Determines pants)
	u32 mii_id;      ///< ID of Mii
	u8 mac[6];       ///< Creator's system's full MAC address
	u8 pad[2];       ///< Padding

	/// Mii details
	struct {
		bool sex : 1;         ///< Sex of Mii (False=Male, True=Female)
		u16 bday_month : 4;   ///< Month of Mii's birthday
		u16 bday_day : 5;     ///< Day of Mii's birthday
		u16 shirt_color : 4;  ///< Color of Mii's shirt
		u16 favorite : 1;    ///< Whether the Mii is one of your 10 favorite Mii's
		u16 _pad : 1;
	} CTR_PACKED mii_details;

	u16 mii_name[10];  ///< Name of Mii (Encoded using UTF16)
	u8 height;         ///< How tall the Mii is
	u8 width;          ///< How wide the Mii is

	/// Face style
	struct
	{
		u16 disable_sharing : 1; ///< Whether or not Sharing of the Mii is allowed
		u16 shape : 4;             ///< Face shape
		u16 skinColor : 3;         ///< Color of skin
	} CTR_PACKED face_style;

	/// Face details
	struct
	{
		u16 wrinkles : 4;
		u16 makeup : 4;
	} CTR_PACKED face_details;

	u8 hair_style;

	/// Hair details
	struct
	{
		u16 color : 3;
		u16 flip : 1;
		u16 _pad : 4;
	} CTR_PACKED hair_details;

	/// Eye details
	struct
	{
		u16 style : 6;
		u16 color : 3;
		u16 scale : 4;
		u16 yscale : 3;
		u16 rotation : 5;
		u16 xspacing : 4;
		u16 yposition : 5;
		u16 _pad : 2;
	} CTR_PACKED eye_details;

	/// Eyebrow details
	struct
	{
		u16 style : 5;
		u16 color : 3;
		u16 scale : 4;
		u16 yscale : 3;
		u16 _pad : 1;
		u16 rotation : 4;
		u16 xspacing : 4;
		u16 yposition : 5;
		u16 _pad2 : 3;
	} CTR_PACKED eyebrow_details;

	/// Nose details
	struct
	{
		u16 style : 5;
		u16 scale : 4;
		u16 yposition : 5;
		u16 _pad : 2;
	} CTR_PACKED nose_details;

	/// Mouth details
	struct
	{
		u16 style : 6;
		u16 color : 3;
		u16 scale : 4;
		u16 yscale : 3;
	} CTR_PACKED mouth_details;

	/// Mustache details
	struct
	{
		u16 mouth_yposition : 5;
		u16 mustache_style : 3;
		u16 _pad : 8;
	} CTR_PACKED mustache_details;

	/// Beard details
	struct
	{
		u16 style : 3;
		u16 color : 3;
		u16 scale : 4;
		u16 ypos : 5;
		u16 _pad : 1;
	} CTR_PACKED beard_details;

	/// Glasses details
	struct
	{
		u16 style : 4;
		u16 color : 3;
		u16 scale : 4;
		u16 ypos : 5;
	} CTR_PACKED glasses_details;

	/// Mole details
	struct
	{
		bool enable : 1;
		u16 scale : 4;
		u16 xpos : 5;
		u16 ypos : 5;
		u16 _pad : 1;
	} CTR_PACKED mole_details;

	u16 author_name[10];    ///< Name of Mii's author (Encoded using UTF16)
} CTR_PACKED MiiData;

/// Mii CFLStoreData (CTR Face Library Store Data) structure
typedef struct
{
	MiiData miiData; ///< Common shared Mii data structure.
	u8 pad[2]; ///< Padding (usually left as zeros)
	u16 crc16; ///< CRC16 over the previous 0x5E of data
} CFLStoreData;