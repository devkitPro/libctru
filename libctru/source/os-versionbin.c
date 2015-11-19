#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/os.h>
#include <3ds/svc.h>
#include <3ds/romfs.h>
#include <3ds/services/ptmsysm.h>
#include <3ds/services/fs.h>
#include <3ds/services/cfgu.h>

#include <sys/time.h>
#include <reent.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

//See here regarding regions: http://3dbrew.org/wiki/Nandrw/sys/SecureInfo_A

static u32 __NVer_tidlow_regionarray[7] = {
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


static Result __read_versionbin(FS_Archive archive, FS_Path fileLowPath, OS_VersionBin *versionbin)
{
	Result ret = 0;
	Handle filehandle = 0;
	FILE *f = NULL;

	ret = FSUSER_OpenFileDirectly(&filehandle, archive, fileLowPath, FS_OPEN_READ, 0x0);
	if(R_FAILED(ret))return ret;

	ret = romfsInitFromFile(filehandle, 0x0);
	if(R_FAILED(ret))return ret;

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

	FS_Archive archive;
	FS_Path fileLowPath;

	memset(archive_lowpath_data, 0, sizeof(archive_lowpath_data));
	memset(file_lowpath_data,    0, sizeof(file_lowpath_data));

	archive.id = 0x2345678a;
	archive.lowPath.type = PATH_BINARY;
	archive.lowPath.size = 0x10;
	archive.lowPath.data = archive_lowpath_data;

	fileLowPath.type = PATH_BINARY;
	fileLowPath.size = 0x14;
	fileLowPath.data = file_lowpath_data;

	archive_lowpath_data[1] = 0x000400DB;

	ret = cfguInit();
	if(R_FAILED(ret))return ret;

	ret = CFGU_SecureInfoGetRegion(&region);
	if(R_FAILED(ret))return ret;

	if(region>=7)return -9;

	cfguExit();

	archive_lowpath_data[0] = __NVer_tidlow_regionarray[region];
	ret = __read_versionbin(archive, fileLowPath, nver_versionbin);
	if(R_FAILED(ret))return ret;

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
	if(R_FAILED(ret))return ret;

	snprintf(sysverstr, sysverstr_maxsize, "%u.%u.%u-%u%c\n", cver_versionbin->mainver, cver_versionbin->minor, cver_versionbin->build, nver_versionbin->mainver, nver_versionbin->region);

	return 0;
}
