#include "soc_common.h"
#include <netdb.h>
#include <3ds/ipc.h>
#include <3ds/result.h>

int getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags)
{
	int i,tmp_addrlen;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[4];
	u8 tmpaddr[0x1c]; // sockaddr size for the kernel is 0x1C (sockaddr_in6?)

	if((host == NULL || hostlen == 0) && (serv == NULL || servlen == 0))
	{
		return EAI_NONAME;
	}

	if(sa->sa_family == AF_INET)
		tmp_addrlen = 8;
	else
		tmp_addrlen = 0x1c;

	if(salen < tmp_addrlen) {
		errno = EINVAL;
		return -1;
	}

	tmpaddr[0] = tmp_addrlen;
	tmpaddr[1] = sa->sa_family;
	memcpy(&tmpaddr[2], &sa->sa_data, tmp_addrlen-2);

	cmdbuf[0] = IPC_MakeHeader(0x10,4,2); // 0x100102
	cmdbuf[1] = sizeof(tmpaddr);
	cmdbuf[2] = hostlen;
	cmdbuf[3] = servlen;
	cmdbuf[4] = flags;
	cmdbuf[5] = IPC_Desc_StaticBuffer(sizeof(tmpaddr),8);
	cmdbuf[6] = (u32)tmpaddr;

	u32 * staticbufs = getThreadStaticBuffers();

	// Save the thread storage values
	for(i = 0 ; i < 4 ; ++i)
		saved_threadstorage[i] = staticbufs[i];

	staticbufs[0] = IPC_Desc_StaticBuffer(hostlen,0);
	staticbufs[1] = (u32)host;
	staticbufs[2] = IPC_Desc_StaticBuffer(servlen,0);
	staticbufs[3] = (u32)serv;

	Result ret = svcSendSyncRequest(SOCU_handle);

	// Restore the thread storage values
	for(i = 0 ; i < 4 ; ++i)
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
	return cmdbuf[2];
}
