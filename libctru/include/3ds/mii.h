/**
 * @file mii.h
 * @brief Shared Mii struct.
 *
 * @see https://www.3dbrew.org/wiki/Mii#Mii_format
 */
#pragma once

#include <3ds/types.h>

/// Shared Mii struct
typedef struct
{
	u8 magic;     ///< Always 3?

	/// Mii options
	struct
	{
		bool allow_copying : 1;   ///< True if copying is allowed
		bool is_private_name : 1; ///< Private name?
		u8 region_lock : 2;       ///< Region lock (0=no lock, 1=JPN, 2=USA, 3=EUR)
		u8 char_set : 2;          ///< Character set (0=JPN+USA+EUR, 1=CHN, 2=KOR, 3=TWN)
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
		bool favorite : 1;    ///< Whether the Mii is one of your 10 favorite Mii's
	} mii_details;

	u16 mii_name[10];  ///< Name of Mii (Encoded using UTF16)
	u8 height;         ///< How tall the Mii is
	u8 width;          ///< How wide the Mii is

	/// Face style
	struct
	{
		bool disable_sharing : 1; ///< Whether or not Sharing of the Mii is allowed
		u8 shape : 4;             ///< Face shape
		u8 skinColor : 3;         ///< Color of skin
	} face_style;

	/// Face details
	struct
	{
		u8 wrinkles : 4;
		u8 makeup : 4;
	} face_details;

	u8 hair_style;

	/// Hair details
	struct
	{
		u8 color : 3;
		bool flip : 1;
	} hair_details;

	/// Eye details
	struct
	{
		u32 style : 6;
		u32 color : 3;
		u32 scale : 4;
		u32 yscale : 3;
		u32 rotation : 5;
		u32 xspacing : 4;
		u32 yposition : 5;
	} eye_details;

	/// Eyebrow details
	struct
	{
		u32 style : 5;
		u32 color : 3;
		u32 scale : 4;
		u32 yscale : 3;
		u32 pad : 1;
		u32 rotation : 5;
		u32 xspacing : 4;
		u32 yposition : 5;
	} eyebrow_details;

	/// Nose details
	struct
	{
		u16 style : 5;
		u16 scale : 4;
		u16 yposition : 5;
	} nose_details;

	/// Mouth details
	struct
	{
		u16 style : 6;
		u16 color : 3;
		u16 scale : 4;
		u16 yscale : 3;
	} mouth_details;

	/// Mustache details
	struct
	{
		u16 mouth_yposition : 5;
		u16 mustach_style : 3;
		u16 pad : 2;
	} mustache_details;

	/// Beard details
	struct
	{
		u16 style : 3;
		u16 color : 3;
		u16 scale : 4;
		u16 ypos : 5;
	} beard_details;

	/// Glasses details
	struct
	{
		u16 style : 4;
		u16 color : 3;
		u16 scale : 4;
		u16 ypos : 5;
	} glasses_details;

	/// Mole details
	struct 
	{
		bool enable : 1;
		u16 scale : 5;
		u16 xpos : 5;
		u16 ypos : 5;
	} mole_details;

	u16 author_name[10];    ///< Name of Mii's author (Encoded using UTF16)
} PACKED MiiData;
