/**
 * @file exheader.h
 * @brief NCCH extended header definitions.
 */
#pragma once

#include <3ds/types.h>

/// ARM9 descriptor flags
enum
{
	ARM9DESC_MOUNT_NAND         = BIT(0), ///< Mount "nand:/"
	ARM9DESC_MOUNT_NANDRO_RW    = BIT(1), ///< Mount nand:/ro/ as read-write
	ARM9DESC_MOUNT_TWLN         = BIT(2), ///< Mount "twln:/"
	ARM9DESC_MOUNT_WNAND        = BIT(3), ///< Mount "wnand:/"
	ARM9DESC_MOUNT_CARDSPI      = BIT(4), ///< Mount "cardspi:/"
	ARM9DESC_USE_SDIF3          = BIT(5), ///< Use SDIF3
	ARM9DESC_CREATE_SEED        = BIT(6), ///< Create seed (movable.sed)
	ARM9DESC_USE_CARD_SPI       = BIT(7), ///< Use card SPI, required by multiple pxi:dev commands
	ARM9DESC_SD_APPLICATION     = BIT(8), ///< SD application (not checked)
	ARM9DESC_MOUNT_SDMC_RW      = BIT(9), ///< Mount "sdmc:/" as read-write
};

/// Filesystem access flags
enum
{
	FSACCESS_CATEGORY_SYSTEM_APPLICATION    = BIT(0),   ///< Category "system application"
	FSACCESS_CATEGORY_HARDWARE_CHECK        = BIT(1),   ///< Category "hardware check"
	FSACCESS_CATEGORY_FILESYSTEM_TOOL       = BIT(2),   ///< Category "filesystem tool"
	FSACCESS_DEBUG                          = BIT(3),   ///< Debug
	FSACCESS_TWLCARD_BACKUP                 = BIT(4),   ///< TWLCARD backup
	FSACCESS_TWLNAND_DATA                   = BIT(5),   ///< TWLNAND data
	FSACCESS_BOSS                           = BIT(6),   ///< BOSS (SpotPass)
	FSACCESS_SDMC_RW                        = BIT(7),   ///< SDMC (read-write)
	FSACCESS_CORE                           = BIT(8),   ///< Core
	FSACCESS_NANDRO_RO                      = BIT(9),   ///< nand:/ro/ (read-only)
	FSACCESS_NANDRW                         = BIT(10),  ///< nand:/rw/
	FSACCESS_NANDRO_RW                      = BIT(11),  ///< nand:/ro/ (read-write)
	FSACCESS_CATEGORY_SYSTEM_SETTINGS       = BIT(12),  ///< Category "System Settings"
	FSACCESS_CARDBOARD                      = BIT(13),  ///< Cardboard (System Transfer)
	FSACCESS_EXPORT_IMPORT_IVS              = BIT(14),  ///< Export/Import IVs (movable.sed)
	FSACCESS_SDMC_WO                        = BIT(15),  ///< SDMC (write-only)
	FSACCESS_SWITCH_CLEANUP                 = BIT(16),  ///< "Switch cleanup" (3.0+)
	FSACCESS_SAVEDATA_MOVE                  = BIT(17),  ///< Savedata move (5.0+)
	FSACCESS_SHOP                           = BIT(18),  ///< Shop (5.0+)
	FSACCESS_SHELL                          = BIT(19),  ///< Shop (5.0+)
	FSACCESS_CATEGORY_HOME_MENU             = BIT(20),  ///< Category "Home Menu" (6.0+)
	FSACCESS_SEEDDB                         = BIT(21),  ///< Seed DB (9.6+)
};

/// The resource limit category of a title
typedef enum
{
	RESLIMIT_CATEGORY_APPLICATION = 0,  ///< Regular application
	RESLIMIT_CATEGORY_SYS_APPLET  = 1,  ///< System applet
	RESLIMIT_CATEGORY_LIB_APPLET  = 2,  ///< Library applet
	RESLIMIT_CATEGORY_OTHER       = 3,  ///< System modules running inside the BASE memregion
} ResourceLimitCategory;

/// The system mode a title should be launched under
typedef enum
{
	SYSMODE_O3DS_PROD = 0,  ///< 64MB of usable application memory
	SYSMODE_N3DS_PROD = 1,  ///< 124MB of usable application memory. Unusable on O3DS
	SYSMODE_DEV1      = 2,  ///< 97MB/178MB of usable application memory
	SYSMODE_DEV2      = 3,  ///< 80MB/124MB of usable application memory
	SYSMODE_DEV3      = 4,  ///< 72MB of usable application memory. Same as "Prod" on N3DS
	SYSMODE_DEV4      = 5,  ///< 32MB of usable application memory. Same as "Prod" on N3DS
} SystemMode;


/// The system info flags and remaster version of a title
typedef struct
{
	u8 reserved[5];                 ///< Reserved
	bool compress_exefs_code  : 1;  ///< Whether the ExeFS's .code section is compressed
	bool is_sd_application    : 1;  ///< Whether the title is meant to be used on an SD card
	u16 remaster_version;           ///< Remaster version
} ExHeader_SystemInfoFlags;

/// Information about a title's section
typedef struct
{
	u32 address;    ///< The address of the section
	u32 num_pages;  ///< The number of pages the section occupies
	u32 size;       ///< The size of the section
} ExHeader_CodeSectionInfo;

/// The name of a title and infomation about its section
typedef struct
{
	char name[8];                       ///< Title name
	ExHeader_SystemInfoFlags flags;     ///< System info flags, see @ref ExHeader_SystemInfoFlags
	ExHeader_CodeSectionInfo text;      ///< .text section info, see @ref ExHeader_CodeSectionInfo
	u32 stack_size;                     ///< Stack size
	ExHeader_CodeSectionInfo rodata;    ///< .rodata section info, see @ref ExHeader_CodeSectionInfo
	u32 reserved;                       ///< Reserved
	ExHeader_CodeSectionInfo data;      ///< .data section info, see @ref ExHeader_CodeSectionInfo
	u32 bss_size;                       ///< .bss section size
} ExHeader_CodeSetInfo;

/// The savedata size and jump ID of a title
typedef struct
{
	u64 savedata_size;  ///< Savedata size
	u64 jump_id;        ///< Jump ID
	u8 reserved[0x30];  ///< Reserved
} ExHeader_SystemInfo;

/// The code set info, dependencies and system info of a title (SCI)
typedef struct
{
	ExHeader_CodeSetInfo codeset_info;  ///< Code set info, see @ref ExHeader_CodeSetInfo
	u64 dependencies[48];               ///< Title IDs of the titles that this program depends on
	ExHeader_SystemInfo system_info;    ///< System info, see @ref ExHeader_SystemInfo
} ExHeader_SystemControlInfo;

/// The ARM11 filesystem info of a title
typedef struct
{
	u64 extdata_id;                                 ///< Extdata ID
	u32 system_savedata_ids[2];                     ///< IDs of the system savedata accessible by the title
	u64 accessible_savedata_ids;                    ///< IDs of the savedata accessible by the title, 20 bits each, followed by "Use other variation savedata"
	u32 fs_access_info;                             ///< FS access flags
	u32 reserved                            : 24;   ///< Reserved
	bool no_romfs                           : 1;    ///< Don't use any RomFS
	bool use_extended_savedata_access       : 1;    ///< Use the "extdata_id" field to store 3 additional accessible savedata IDs
} ExHeader_Arm11StorageInfo;

/// The CPU-related and memory-layout-related info of a title
typedef struct
{
	u32 core_version;                           ///< The low title ID of the target firmware
	bool use_cpu_clockrate_804MHz       : 1;    ///< Whether to start the title with the 804MHz clock rate
	bool enable_l2c                     : 1;    ///< Whether to start the title with the L2C-310 enabled enabled
	u8 flag1_unused                     : 6;    ///< Unused
	SystemMode n3ds_system_mode         : 4;    ///< The system mode to use on N3DS
	u8 flag2_unused                     : 4;    ///< Unused
	u8 ideal_processor                  : 2;    ///< The ideal processor to start the title on
	u8 affinity_mask                    : 2;    ///< The affinity mask of the title
	SystemMode o3ds_system_mode         : 4;    ///< The system mode to use on N3DS
	u8 priority;                                ///< The priority of the title's main thread
} ExHeader_Arm11CoreInfo;

/// The ARM11 system-local capabilities of a title
typedef struct
{
	u64 title_id;                               ///< Title ID
	ExHeader_Arm11CoreInfo core_info;           ///< Core info, see @ref ExHeader_Arm11CoreInfo
	u16 reslimits[16];                          ///< Resource limit descriptors, only "CpuTime" (first byte) sems to be used
	ExHeader_Arm11StorageInfo storage_info;     ///< Storage info, see @ref ExHeader_Arm11StorageInfo
	char service_access[34][8];                 ///< List of the services the title has access to. Limited to 32 prior to system version 9.3
	u8 reserved[15];                            ///< Reserved
	ResourceLimitCategory reslimit_category;    ///< Resource limit category, see @ref ExHeader_Arm11SystemLocalCapabilities
} ExHeader_Arm11SystemLocalCapabilities;

/// The ARM11 kernel capabilities of a title
typedef struct
{
	u32 descriptors[28]; ///< ARM11 kernel descriptors, see 3dbrew
	u8 reserved[16];     ///< Reserved
} ExHeader_Arm11KernelCapabilities;

/// The ARM9 access control of a title
typedef struct
{
	u8 descriptors[15];     ///< Process9 FS descriptors, see 3dbrew
	u8 descriptor_version;  ///< Descriptor version
} ExHeader_Arm9AccessControl;

/// The access control information of a title
typedef struct
{
	ExHeader_Arm11SystemLocalCapabilities local_caps;   ///< ARM11 system-local capabilities, see @ref ExHeader_Arm11SystemLocalCapabilities
	ExHeader_Arm11KernelCapabilities kernel_caps;       ///< ARM11 kernel capabilities, see @ref ExHeader_Arm11SystemLocalCapabilities
	ExHeader_Arm9AccessControl access_control;          ///< ARM9 access control, see @ref ExHeader_Arm9AccessControl
} ExHeader_AccessControlInfo;

/// Main extended header data, as returned by PXIPM, Loader and FSREG service commands
typedef struct
{
	ExHeader_SystemControlInfo sci; ///< System control info, see @ref ExHeader_SystemControlInfo
	ExHeader_AccessControlInfo aci; ///< Access control info, see @ref ExHeader_AccessControlInfo
} ExHeader_Info;

/// Extended header access descriptor
typedef struct
{
	u8 signature[0x100];                ///< The signature of the access descriptor (RSA-2048-SHA256)
	u8 ncchModulus[0x100];              ///< The modulus used for the above signature, with 65537 as public exponent
	ExHeader_AccessControlInfo acli;    ///< This is compared for equality with the first ACI by Process9, see @ref ExHeader_AccessControlInfo
} ExHeader_AccessDescriptor;

/// The NCCH Extended Header of a title
typedef struct
{
	ExHeader_Info info;                             ///< Main extended header data, see @ref ExHeader_Info
	ExHeader_AccessDescriptor access_descriptor;    ///< Access descriptor, see @ref ExHeader_AccessDescriptor
} ExHeader;
