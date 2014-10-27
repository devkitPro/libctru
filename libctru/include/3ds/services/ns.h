#pragma once

/*
	Requires access to "ns:s" service
*/

Result nsInit();
Result nsExit();

/* NS_LaunchTitle()
  titleid			TitleId of title to launch, if 0, gamecard assumed
  launch_flags		use if you know of any
  procid			ptr to where the process id of the launched title will be written to, leave a NULL, if you don't care
*/
Result NS_LaunchTitle(u64 titleid, u32 launch_flags, u32 *procid);

/* NS_RebootToTitle()
  mediatype			mediatype for title
  titleid			TitleId of title to launch
*/
Result NS_RebootToTitle(u8 mediatype, u64 titleid);