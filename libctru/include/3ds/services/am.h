#pragma once

#include <3ds/types.h>

/*
	Requires access to "am:net" or "am:u" service
*/


typedef struct
{
	u64 titleID;
	u64 unknown;
	u16 titleVersion;
	u8 unknown2[6];
} TitleList;



Result amInit();
Result amExit();


/* AM_TitleIDListGetTotal()
About: Gets the number of titles for a given mediatype

  mediatype  mediatype to get titles from
  count      ptr to store title count
*/
Result AM_TitleIdListGetTotal(u8 mediatype, u32 *count);


/* AM_GetTitleIdList()
About: Writes a titleid list for a mediatype to a buffer

  mediatype  mediatype to get list from
  count      number of titleids to get
  titleIDs   ptr to buffer to write title IDs to
*/
Result AM_GetTitleIdList(u8 mediatype, u32 count, u64 *titleIDs);


/* AM_GetDeviceId()
About: Gets a 32bit device id, it's used for some key slot inits

  deviceID  ptr to where the device id is written to
*/
Result AM_GetDeviceId(u32 *deviceID);


/* AM_GetTitleProductCode()
About: Get the 16 bytes product code of a title

  mediatype       mediatype of title
  titleID         title ID of title
	outProductCode  pointer to string to store the output product code
*/
Result AM_GetTitleProductCode(u8 mediatype, u64 titleID, char *outProductCode);


/* AM_ListTitles()
About: Get a list with details about the installed titles

  mediatype    mediatype of title
  titleCount   number of titles to list
	titleIdList  pointer to a title ID list
	titleList    pointer for the output TitleList array
*/
Result AM_ListTitles(u8 mediatype, u32 titleCount, u64 *titleIdList, TitleList *titleList);



/**** Title Install Methods ****/
/* AM_StartInstallCIADB0()
About: Inits CIA install process, the returned ciahandle is where the data for CIA should be written to
Note: This is for title DB 0 (normal title install)

  mediatype  mediatype to install CIA to
  ciaHandle  ptr to where the handle should be written to
*/
Result AM_StartInstallCIADB0(u8 mediatype, Handle *ciaHandle);


/* AM_StartInstallCIADB1()
About: Inits CIA install process, the returned ciahandle is where the data for CIA should be written to
Note: This is for installing DLP CIAs only, mediatype is hardcoded to be NAND

  ciaHandle  ptr to where the handle should be written to
*/
Result AM_StartInstallCIADB1(Handle *ciaHandle);


/* AM_AbortCIAInstall()
About: Abort CIA install process

  ciaHandle  ptr to cia Handle provided by AM
*/
Result AM_AbortCIAInstall(Handle *ciaHandle);


/* AM_CloseCIAFinalizeInstall()
About: Once all data is written to the cia handle, this command signals AM to proceed with CIA install.
Note: AM closes the cia handle provided here

  mediatype  same mediatype specified ciaHandle was obtained
  ciaHandle  ptr to cia Handle provided by AM
*/
Result AM_CloseCIAFinalizeInstall(u8 mediatype, Handle *ciaHandle);



/**** Title Delete Methods ****/
/* AM_DeleteTitle()
About: Deletes any title on NAND/SDMC
Note: AM closes the cia handle provided here

  mediatype  mediatype of title
  titleID    title id of title
*/
Result AM_DeleteTitle(u8 mediatype, u64 titleID);


/* AM_DeleteAppTitle()
About: Deletes any title on NAND/SDMC
Note: If the title has the system category bit set, this will fail

  mediatype  mediatype of title
  titleID    title ID of title
*/
Result AM_DeleteAppTitle(u8 mediatype, u64 titleID);


/* AM_InstallFIRM()
About: Installs FIRM to NAND (firm0:/ & firm1:/) from a CXI
Note: The title must have the uniqueid: 0x00000, otherwise this will fail.

  titleID  title id of title
*/
Result AM_InstallFIRM(u64 titleid);
