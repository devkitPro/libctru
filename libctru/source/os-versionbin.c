#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/os.h>
#include <3ds/romfs.h>
#include <3ds/services/cfgu.h>

#include <stdio.h>
#include <string.h>

//See here regarding regions: http://3dbrew.org/wiki/Nandrw/sys/SecureInfo_A

#define TID_HIGH 0x000400DB00000000ULL

static const u32 __NVer_tidlow_regionarray[7] =
{
	0x00016202, //JPN
	0x00016302, //USA
	0x00016102, //EUR
	0x00016202, //"AUS"
	0x00016402, //CHN
	0x00016502, //KOR
	0x00016602, //TWN
};

static const u32 __CVer_tidlow_regionarray[7] =
{
	0x00017202, //JPN
	0x00017302, //USA
	0x00017102, //EUR
	0x00017202, //"AUS"
	0x00017402, //CHN
	0x00017502, //KOR
	0x00017602, //TWN
};

static Result osReadVersionBin(u64 tid, OS_VersionBin *versionbin)
{
	Result ret = romfsMountFromTitle(tid, MEDIATYPE_NAND, "ver");
	if (R_FAILED(ret))
		return ret;

	FILE* f = fopen("ver:/version.bin", "r");
	if (!f)
		ret = MAKERESULT(RL_PERMANENT, RS_NOTFOUND, RM_APPLICATION, RD_NOT_FOUND);
	else
	{
		if (fread(versionbin, 1, sizeof(OS_VersionBin), f) != sizeof(OS_VersionBin))
			ret = MAKERESULT(RL_PERMANENT, RS_INVALIDSTATE, RM_APPLICATION, RD_NO_DATA);
		fclose(f);
	}

	romfsUnmount("ver");
	return ret;
}

Result osGetSystemVersionData(OS_VersionBin *nver_versionbin, OS_VersionBin *cver_versionbin)
{
	Result ret = cfguInit();
	if(R_FAILED(ret))return ret;

	u8 region=0;
	ret = CFGU_SecureInfoGetRegion(&region);
	cfguExit();

	if(R_FAILED(ret))return ret;

	if(region>=7)return MAKERESULT(RL_PERMANENT, RS_INVALIDSTATE, RM_APPLICATION, RD_OUT_OF_RANGE);

	ret = osReadVersionBin(TID_HIGH | __NVer_tidlow_regionarray[region], nver_versionbin);
	if(R_FAILED(ret))return ret;

	ret = osReadVersionBin(TID_HIGH | __CVer_tidlow_regionarray[region], cver_versionbin);
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

	snprintf(sysverstr, sysverstr_maxsize, "%u.%u.%u-%u%c", cver_versionbin->mainver, cver_versionbin->minor, cver_versionbin->build, nver_versionbin->mainver, nver_versionbin->region);

	return 0;
}
