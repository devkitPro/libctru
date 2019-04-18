#pragma once

#include <3ds/types.h>

typedef struct
{
	u8 magic; 

	struct
	{
		bool allow_copying : 1;
		bool is_private_name : 1;
		u8 region_lock : 2;
		u8 char_set : 2;
	} mii_options;

	struct
	{
		u8 page_index : 4;
		u8 slot_index : 4;
	} mii_pos;

	struct
	{
		u8 unknown0 : 4;
		u8 origin_console : 4;
	} console_identity;

	u64 system_id;
	u32 mii_id;
	u8 mac[6];
	u8 pad[2];

	struct {
		u16 sex : 1;
		u16 month : 4;
		u16 day : 5;
		u16 color : 4;
		u16 favorite : 1;
	} mii_details;

	u16 mii_name[10];
	u8 height;
	u8 width;

	struct
	{
		bool disable_sharing : 1;
		u8 shape : 4;
		u8 skinColor : 3;
	} face_style;

	struct
	{
		u8 wrinkles : 4;
		u8 makeup : 4;
	} face_details;

	u8 hair_style;

	struct
	{
		u8 color : 3;
		bool flip : 1;
	} hair_details;

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

	struct
	{
		u32 style : 6;
		u32 color : 3;
		u32 scale : 4;
		u32 yscale : 3;
		u32 rotation : 5;
		u32 xspacing : 4;
		u32 yposition : 5;
	} eyebrow_details;

	struct
	{
		u16 style : 5;
		u16 scale : 4;
		u16 yposition : 5;
	} nose_details;

	struct
	{
		u16 style : 6;
		u16 color : 3;
		u16 scale : 4;
		u16 yscale : 3;
	} mouse_details;

	struct
	{
		u16 mouse_yposition : 5;
		u16 mustach_style : 3;
		u16 pad : 2;
	} mustache_details;

	struct
	{
		u16 style : 3;
		u16 color : 3;
		u16 scale : 4;
		u16 ypos : 5;
	} beard_details;

	struct
	{
		u16 style : 4;
		u16 color : 3;
		u16 scale : 4;
		u16 ypos : 5;
	} glasses_details;

	struct 
	{
		bool enable : 1;
		u16 scale : 5;
		u16 xpos : 5;
		u16 ypos : 5;
	} mole_details;

	u16 author_name[10];
} PACKED MiiData;
