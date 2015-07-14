/*
  qtm.c - New3DS head-tracking
*/
#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/services/qtm.h>

Handle qtmHandle;

static bool qtmInitialized = false;

Result qtmInit()
{
	Result ret=0;

	if(qtmInitialized)return 0;

	if((ret=srvGetServiceHandle(&qtmHandle, "qtm:u")) && (ret=srvGetServiceHandle(&qtmHandle, "qtm:s")) && (ret=srvGetServiceHandle(&qtmHandle, "qtm:sp")))return ret;

	qtmInitialized = true;

	return 0;
}

void qtmExit()
{
	if(!qtmInitialized)return;

	svcCloseHandle(qtmHandle);
	qtmInitialized = false;
}

bool qtmCheckInitialized()
{
	return qtmInitialized;
}

Result qtmGetHeadtrackingInfo(u64 val, qtmHeadtrackingInfo *out)
{
	u32* cmdbuf=getThreadCommandBuffer();

	if(!qtmInitialized)return -1;

	cmdbuf[0]=0x00020080; //request header code
	memcpy(&cmdbuf[1], &val, 8);

	Result ret=0;
	if((ret=svcSendSyncRequest(qtmHandle)))return ret;

	ret = (Result)cmdbuf[1];
	if(ret!=0)return ret;

	if(out)memcpy(out, &cmdbuf[2], sizeof(qtmHeadtrackingInfo));

	return 0;
}

bool qtmCheckHeadFullyDetected(qtmHeadtrackingInfo *info)
{
	if(info==NULL)return false;

	if(info->flags[0] && info->flags[1] && info->flags[2])return true;
	return false;
}

Result qtmConvertCoordToScreen(qtmHeadtrackingInfoCoord *coord, float *screen_width, float *screen_height, u32 *x, u32 *y)
{
	float width = 200.0f;
	float height = 160.0f;

	if(coord==NULL || x==NULL || y==NULL)return -1;

	if(screen_width)width = (*screen_width) / 2;
	if(screen_height)height = (*screen_height) / 2;

	if(coord->x > 1.0f || coord->x < -1.0f)return -2;
	if(coord->y > 1.0f || coord->y < -1.0f)return -2;

	*x = (u32)((coord->x * width) + width);
	*y = (u32)((coord->y * height) + height);

	return 0;
}

