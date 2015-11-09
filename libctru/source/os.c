#include <3ds/types.h>
#include <3ds/os.h>
#include <3ds/svc.h>
#include <3ds/romfs.h>
#include <3ds/services/ptm.h>
#include <3ds/services/fs.h>
#include <3ds/services/cfgu.h>

#include <sys/time.h>
#include <reent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define TICKS_PER_USEC 268.123480
#define TICKS_PER_MSEC 268123.480

// Work around the VFP not supporting 64-bit integer <--> floating point conversion
static inline double u64_to_double(u64 value) {
	return (((double)(u32)(value >> 32))*0x100000000ULL+(u32)value);
}

typedef struct {
	u64 date_time;
	u64 update_tick;
	//...
} datetime_t;

static volatile u32* __datetime_selector =
	(u32*) 0x1FF81000;
static volatile datetime_t* __datetime0 =
	(datetime_t*) 0x1FF81020;
static volatile datetime_t* __datetime1 =
	(datetime_t*) 0x1FF81040;

__attribute__((weak)) bool __ctru_speedup = false;

static u32 __NVer_tidlow_regionarray[7] = {//See here regarding regions: http://3dbrew.org/wiki/Nandrw/sys/SecureInfo_A
0x00016202, //JPN
0x00016302, //USA
0x00016102, //EUR
0x00016202, //"AUS"
0x00016402, //CHN
0x00016502, //KOR
0x00016602, //TWN
};

static u32 __CVer_tidlow_regionarray[7] = {
0x00017202, //JPN
0x00017302, //USA
0x00017102, //EUR
0x00017202, //"AUS"
0x00017402, //CHN
0x00017502, //KOR
0x00017602 //TWN
};

//---------------------------------------------------------------------------------
u32 osConvertVirtToPhys(u32 vaddr) {
//---------------------------------------------------------------------------------
	if(vaddr >= 0x14000000 && vaddr < 0x1c000000)
		return vaddr + 0x0c000000; // LINEAR heap
	if(vaddr >= 0x1F000000 && vaddr < 0x1F600000)
		return vaddr - 0x07000000; // VRAM
	if(vaddr >= 0x1FF00000 && vaddr < 0x1FF80000)
		return vaddr + 0x00000000; // DSP memory
	if(vaddr >= 0x30000000 && vaddr < 0x40000000)
		return vaddr - 0x10000000; // Only available under FIRM v8+ for certain processes.
	return 0;
}

//---------------------------------------------------------------------------------
u32 osConvertOldLINEARMemToNew(u32 vaddr) {
//---------------------------------------------------------------------------------
	if(vaddr >= 0x30000000 && vaddr < 0x40000000)return vaddr;
	if(vaddr >= 0x14000000 && vaddr < 0x1c000000)return vaddr+=0x1c000000;
	return 0;
}

//---------------------------------------------------------------------------------
static datetime_t getSysTime() {
//---------------------------------------------------------------------------------
	u32 s1, s2 = *__datetime_selector & 1;
	datetime_t dt;

	do {
		s1 = s2;
		if(!s1)
			dt = *__datetime0;
		else
			dt = *__datetime1;
		s2 = *__datetime_selector & 1;
	} while(s2 != s1);

	return dt;
}

//---------------------------------------------------------------------------------
int __libctru_gtod(struct _reent *ptr, struct timeval *tp, struct timezone *tz) {
//---------------------------------------------------------------------------------
	if (tp != NULL) {

		datetime_t dt = getSysTime();

		u64 delta = svcGetSystemTick() - dt.update_tick;

		u32 offset =  (u32)(u64_to_double(delta)/TICKS_PER_USEC);

		// adjust from 1900 to 1970
		u64 now = ((dt.date_time - 2208988800000ULL) * 1000) + offset;

		tp->tv_sec =  u64_to_double(now)/1000000.0;
		tp->tv_usec = now - ((tp->tv_sec) * 1000000);

	}

	if (tz != NULL) {
		tz->tz_minuteswest = 0;
		tz->tz_dsttime = 0;
	}

	return 0;

}


// Returns number of milliseconds since 1st Jan 1900 00:00.
//---------------------------------------------------------------------------------
u64 osGetTime() {
//---------------------------------------------------------------------------------
	datetime_t dt = getSysTime();

	u64 delta = svcGetSystemTick() - dt.update_tick;

	return dt.date_time + (u32)(u64_to_double(delta)/TICKS_PER_MSEC);
}

//---------------------------------------------------------------------------------
u32 osGetFirmVersion() {
//---------------------------------------------------------------------------------
	return (*(u32*)0x1FF80060) & ~0xFF;
}

//---------------------------------------------------------------------------------
u32 osGetKernelVersion() {
//---------------------------------------------------------------------------------
	return (*(u32*)0x1FF80000) & ~0xFF;
}

//---------------------------------------------------------------------------------
const char* osStrError(u32 error) {
//---------------------------------------------------------------------------------
	switch((error>>26) & 0x3F) {
	case 0:
		return "Success.";
	case 1:
		return "Nothing happened.";
	case 2:
		return "Would block.";
	case 3:
		return "Not enough resources.";
	case 4:
		return "Not found.";
	case 5:
		return "Invalid state.";
	case 6:
		return "Unsupported.";
	case 7:
		return "Invalid argument.";
	case 8:
		return "Wrong argument.";
	case 9:
		return "Interrupted.";
	case 10:
		return "Internal error.";
	default:
		return "Unknown.";
	}
}

//---------------------------------------------------------------------------------
u8 osGetWifiStrength(void) {
//---------------------------------------------------------------------------------
	return *((u8*)0x1FF81066);
}

void __ctru_speedup_config(void)
{
	if (ptmSysmInit()==0)
	{
		PTMSYSM_ConfigureNew3DSCPU(__ctru_speedup ? 3 : 0);
		ptmSysmExit();
	}
}

void osSetSpeedupEnable(bool enable)
{
	__ctru_speedup = enable;
	__ctru_speedup_config();
}

static Result __read_versionbin(FS_archive archive, FS_path fileLowPath, OS_VersionBin *versionbin)
{
	Result ret = 0;
	Handle filehandle = 0;
	FILE *f = NULL;

	ret = FSUSER_OpenFileDirectly(NULL, &filehandle, archive, fileLowPath, FS_OPEN_READ, 0x0);
	if(ret<0)return ret;

	ret = romfsInitFromFile(filehandle, 0x0);
	if(ret<0)return ret;

	f = fopen("romfs:/version.bin", "r");
	if(f==NULL)
	{
		ret = errno;
	}
	else
	{
		if(fread(versionbin, 1, sizeof(OS_VersionBin), f) != sizeof(OS_VersionBin))ret = -10;
		fclose(f);
	}

	romfsExit();

	return ret;
}

Result osGetSystemVersionData(OS_VersionBin *nver_versionbin, OS_VersionBin *cver_versionbin)
{
	Result ret=0;
	u8 region=0;

	u32 archive_lowpath_data[0x10>>2];
	u32 file_lowpath_data[0x14>>2];

	FS_archive archive;
	FS_path fileLowPath;

	memset(archive_lowpath_data, 0, sizeof(file_lowpath_data));
	memset(file_lowpath_data, 0, sizeof(file_lowpath_data));

	archive.id = 0x2345678a;
	archive.lowPath.type = PATH_BINARY;
	archive.lowPath.size = 0x10;
	archive.lowPath.data = (u8*)archive_lowpath_data;

	fileLowPath.type = PATH_BINARY;
	fileLowPath.size = 0x14;
	fileLowPath.data = (u8*)file_lowpath_data;

	archive_lowpath_data[1] = 0x000400DB;

	ret = initCfgu();
	if(ret<0)return ret;

	ret = CFGU_SecureInfoGetRegion(&region);
	if(ret<0)return ret;

	if(region>=7)return -9;

	exitCfgu();

	archive_lowpath_data[0] = __NVer_tidlow_regionarray[region];
	ret = __read_versionbin(archive, fileLowPath, nver_versionbin);
	if(ret<0)return ret;

	archive_lowpath_data[0] = __CVer_tidlow_regionarray[region];
	ret = __read_versionbin(archive, fileLowPath, cver_versionbin);
	return ret;
}

Result osGetSystemVersionDataString(OS_VersionBin *nver_versionbin, OS_VersionBin *cver_versionbin, char *sysverstr, u32 sysverstr_maxsize)
{
	Result ret=0;
	OS_VersionBin nver_versionbin_tmp, cver_versionbin_tmp;

	if(nver_versionbin==NULL)nver_versionbin = &nver_versionbin_tmp;
	if(cver_versionbin==NULL)cver_versionbin = &cver_versionbin_tmp;

	ret = osGetSystemVersionData(nver_versionbin, cver_versionbin);
	if(ret!=0)return ret;

	snprintf(sysverstr, sysverstr_maxsize, "%u.%u.%u-%u%c\n", cver_versionbin->mainver, cver_versionbin->minor, cver_versionbin->build, nver_versionbin->mainver, nver_versionbin->region);

	return 0;
}

