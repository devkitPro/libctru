#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/qtm.h>
#include <3ds/ipc.h>

Handle qtmHandle;
static int qtmRefCount;

Result qtmInit(void)
{
	Result ret=0;

	if (AtomicPostIncrement(&qtmRefCount)) return 0;

	ret = srvGetServiceHandle(&qtmHandle, "qtm:u");
	if (R_FAILED(ret)) ret = srvGetServiceHandle(&qtmHandle, "qtm:s");
	if (R_FAILED(ret)) ret = srvGetServiceHandle(&qtmHandle, "qtm:sp");
	if (R_FAILED(ret)) AtomicDecrement(&qtmRefCount);
	return ret;
}

void qtmExit(void)
{
	if (AtomicDecrement(&qtmRefCount)) return;
	svcCloseHandle(qtmHandle);
}

bool qtmCheckInitialized(void)
{
	return qtmRefCount>0;
}

bool qtmCheckHeadFullyDetected(QTM_HeadTrackingInfo *info)
{
	if(info==NULL)return false;

	if(info->flags[0] && info->flags[1] && info->flags[2])return true;
	return false;
}

Result qtmConvertCoordToScreen(QTM_HeadTrackingInfoCoord *coord, float *screen_width, float *screen_height, u32 *x, u32 *y)
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

Result QTM_GetHeadTrackingInfo(u64 val, QTM_HeadTrackingInfo* out)
{
	if(!qtmCheckInitialized())return -1;

	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,2,0); // 0x20080
	cmdbuf[1] = val&0xFFFFFFFF;
	cmdbuf[2] = val>>32;

	if(R_FAILED(ret=svcSendSyncRequest(qtmHandle)))return ret;

	if(out) memcpy(out, &cmdbuf[2], sizeof(QTM_HeadTrackingInfo));

	return cmdbuf[1];
}

