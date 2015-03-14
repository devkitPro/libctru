#pragma once

/*
	Requires access to "am:net" or "am:u" service
*/


typedef struct
{
	u64 titleID;
	u64 size;
	u16 titleVersion;
	u8 unknown2[6];
} TitleList;


Result amInit();
Result amExit();

/* AM_GetTitleCount()
About: Gets the number of titles for a given mediatype

  mediatype		mediatype to get titles from
  count			ptr to store title count
*/
Result AM_GetTitleCount(u8 mediatype, u32 *count);

/* AM_GetTitleList()
About: Writes a titleid list for a mediatype to a buffer

  mediatype		mediatype to get list from
  count			number of titleids to get
  titleIDs	buffer to write titleids to
*/
Result AM_GetTitleIdList(u8 mediatype, u32 count, u64 *titleIDs);

/* AM_GetDeviceId()
About: Gets a 32bit device id, it's used for some key slot inits

  device_id		ptr to where the device id is written to
*/
Result AM_GetDeviceId(u32 *deviceID);

/* AM_ListTitles()
About: Get a list with details about the installed titles

  mediatype    mediatype of title
  titleCount   number of titles to list
	titleIdList  pointer to a title ID list
	titleList    pointer for the output TitleList array
*/
Result AM_ListTitles(u8 mediatype, u32 titleCount, u64 *titleIdList, TitleList *titleList);

/**** Title Install Methods ****/
/* AM_StartCiaInstall()
About: Inits CIA install process, the returned ciahandle is where the data for CIA should be written to

  mediatype		mediatype to install CIA to
  ciahandle		ptr to where the handle should be written to
*/
Result AM_StartCiaInstall(u8 mediatype, Handle *ciaHandle);

/* AM_StartDlpChildCiaInstall()
About: Inits CIA install process, the returned ciahandle is where the data for CIA should be written to
Note: This is for installing DLP CIAs only, mediatype is hardcoded to be NAND

  ciahandle		ptr to where the handle should be written to
*/
Result AM_StartDlpChildCiaInstall(Handle *ciaHandle);

/* AM_CancelCIAInstall()
About: Abort CIA install process

  ciahandle		ptr to cia Handle provided by AM
*/
Result AM_CancelCIAInstall(Handle *ciaHandle);

/* AM_FinishCiaInstall()
About: Once all data is written to the cia handle, this command signals AM to proceed with CIA install.
Note: AM closes the cia handle provided here

  mediatype		same mediatype specified ciahandle was obtained
  ciahandle		ptr to cia Handle provided by AM
*/
Result AM_FinishCiaInstall(u8 mediatype, Handle *ciaHandle);

/**** Title Delete Methods ****/
/* AM_DeleteTitle()
About: Deletes any title on NAND/SDMC
Note: AM closes the cia handle provided here

  mediatype		mediatype of title
  titleid		title id of title
*/
Result AM_DeleteTitle(u8 mediatype, u64 titleID);

/* AM_DeleteAppTitle()
About: Deletes any title on NAND/SDMC
Note: If the title has the system category bit set, this will fail

  mediatype		mediatype of title
  titleid		title id of title
*/
Result AM_DeleteAppTitle(u8 mediatype, u64 titleID);

/* AM_InstallFIRM()
About: Installs FIRM to NAND (firm0:/ & firm1:/) from a CXI
Note: The title must have the uniqueid: 0x00000, otherwise this will fail.

  titleid		title id of title
*/
Result AM_InstallFIRM(u64 titleID);

/* AM_GetTitleProductCode()
About: Gets the product code of a title based on its title id.

  mediatype		mediatype of title
  titleid		title id of title
  productcode           buffer to output the product code to (should have a length of 16) 
*/
Result AM_GetTitleProductCode(u8 mediatype, u64 titleID, char* productCode);
