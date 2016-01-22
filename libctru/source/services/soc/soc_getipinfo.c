#include "soc_common.h"
#include <3ds/ipc.h>
#include <3ds/result.h>

typedef struct
{
  struct in_addr ip;
  struct in_addr netmask;
  struct in_addr broadcast;
} SOCU_IPInfo_t;

int SOCU_GetIPInfo(struct in_addr *ip, struct in_addr *netmask, struct in_addr *broadcast)
{
	int           i, ret;
	u32           *cmdbuf     = getThreadCommandBuffer();
	u32           *staticbufs = getThreadStaticBuffers();
	u32           saved_threadstorage[2];
	SOCU_IPInfo_t info;

	cmdbuf[0] = IPC_MakeHeader(0x1A,3,0); //0x1A00C0
	cmdbuf[1] = 0xFFFE;
	cmdbuf[2] = 0x4003;
	cmdbuf[3] = sizeof(info);

	// Save the thread storage values
	for(i = 0 ; i < 2 ; ++i)
		saved_threadstorage[i] = staticbufs[i];

	staticbufs[0] = IPC_Desc_StaticBuffer(sizeof(info), 0);
	staticbufs[1] = (u32)&info;

	ret = svcSendSyncRequest(SOCU_handle);

	// Restore the thread storage values
	for(i = 0 ; i < 2 ; ++i)
		staticbufs[i] = saved_threadstorage[i];

	if(R_FAILED(ret)) {
		errno = SYNC_ERROR;
		return ret;
	}

	ret = cmdbuf[1];
	if(R_FAILED(ret)) {
		errno = SYNC_ERROR;
		return ret;
	}
	if(cmdbuf[2] != 0)
	{
		return cmdbuf[2];
	}

	if(ip != NULL)
		*ip = info.ip;
	if(netmask != NULL)
		*netmask = info.netmask;
	if(broadcast != NULL)
		*broadcast = info.broadcast;

	return 0;
}
