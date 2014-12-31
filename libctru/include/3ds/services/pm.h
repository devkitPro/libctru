#pragma once

/*
	Requires access to "pm:app" service
*/

Result pmInit();
Result pmExit();

/* PM_LaunchTitle()
About: Launches a title

  mediatype		mediatype of title
  titleid		TitleId of title to launch
  launch_flags	use if you know of any
*/
Result PM_LaunchTitle(u8 mediatype, u64 titleid, u32 launch_flags);

/* PM_GetTitleExheaderFlags()
About: Writes to a buffer the launch flags (8 bytes) from a title exheader.

  mediatype		mediatype of title
  titleid		TitleId of title
  out			ptr to where the flags should be written to
*/
Result PM_GetTitleExheaderFlags(u8 mediatype, u64 titleid, u8* out);

/* PM_SetFIRMLaunchParams()
About: Sets the FIRM launch params from in

  size			size of FIRM launch params
  in			ptr to location of FIRM launch params
*/
Result PM_SetFIRMLaunchParams(u32 size, u8* in);

/* PM_GetFIRMLaunchParams()
About: Sets the FIRM launch params from in

  size			size of buffer to store FIRM launch params
  out			ptr to location to write FIRM launch params
*/
Result PM_GetFIRMLaunchParams(u32 size, u8* out);

/* PM_SetFIRMLaunchParams()
About: Same as PM_SetFIRMLaunchParams(), but also triggers a FIRM launch

  firm_titleid_low	TitleID low of firm title to launch
  size				size of FIRM launch params
  in				ptr to location of FIRM launch params
*/
Result PM_LaunchFIRMSetParams(u32 firm_titleid_low, u32 size, u8* in);
