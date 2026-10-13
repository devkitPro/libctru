#include "soc_common.h"
#include <errno.h>
#include <sys/socket.h>
#include <3ds/ipc.h>

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen)
{
	int ret = 0;
	int fd = sockfd;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	sockfd = soc_get_fd(sockfd);
	if(sockfd < 0) {
		errno = -sockfd;
		return -1;
	}

	cmdbuf[0] = IPC_MakeHeader(0x11,4,2); // 0x110102
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = (u32)level;
	cmdbuf[3] = (u32)optname;
	cmdbuf[4] = (u32)*optlen;
	cmdbuf[5] = IPC_Desc_CurProcessId();

	u32 * staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(*optlen,0);
	staticbufs[1] = (u32)optval;

	ret = svcSendSyncRequest(SOCU_handle);

	staticbufs[0] = saved_threadstorage[0];
	staticbufs[1] = saved_threadstorage[1];

	if(ret != 0) {
		errno = SYNC_ERROR;
		return ret;
	}

	ret = (int)cmdbuf[1];
	if(ret == 0)
		ret = _net_convert_error(cmdbuf[2]);

	if(ret < 0) {
		errno = -ret;
		return -1;
	}

	*optlen = cmdbuf[3];

	// For SO_ERROR the service returns the result code of an earlier operation on the socket, in its own
	// codes: EINPROGRESS after a non-blocking connect() even once it has completed, EAGAIN after a recv()
	// that found nothing, ENOTCONN once a refused connection was polled. None of those is an error of the
	// socket itself, so report whether its connection failed instead.
	if(level == SOL_SOCKET && optname == SO_ERROR && *optlen >= sizeof(int)) {
		int *error = (int*)optval;
		if(*error < 0)
			*error = -_net_convert_error(*error);
		if(*error == EINPROGRESS || *error == EALREADY || *error == ENOTCONN || *error == EAGAIN)
			*error = soc_connection_error(fd);
	}

	return ret;
}
