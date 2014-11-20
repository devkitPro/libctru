#include "soc_common.h"

long gethostid(void)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = 0x00160000;

	if((ret = svcSendSyncRequest(SOCU_handle))!=0)return ret;

	ret = (int)cmdbuf[1];
	if(ret==0)ret = cmdbuf[2];

	return ret;
}
