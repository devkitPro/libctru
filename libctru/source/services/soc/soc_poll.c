#include "soc_common.h"
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	int ret = 0;
	nfds_t i;
	u32 size = sizeof(struct pollfd)*nfds;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	if(nfds == 0) {
		errno = EINVAL;
		return -1;
	}

	struct pollfd *tmp_fds = (struct pollfd*)malloc(sizeof(struct pollfd) * nfds);
	if(tmp_fds == NULL) {
		errno = ENOMEM;
		return -1;
	}

	memcpy(tmp_fds, fds, sizeof(struct pollfd) * nfds);

	for(i = 0; i < nfds; ++i) {
		tmp_fds[i].fd = soc_get_fd(fds[i].fd);
		if(tmp_fds[i].fd < 0) {
			errno = -tmp_fds[i].fd;
			free(tmp_fds);
			return -1;
		}
		tmp_fds[i].revents = 0;
	}

	cmdbuf[0] = 0x00140084;
	cmdbuf[1] = (u32)nfds;
	cmdbuf[2] = (u32)timeout;
	cmdbuf[3] = 0x20;
	cmdbuf[5] = (size<<14) | 0x2802;
	cmdbuf[6] = (u32)tmp_fds;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = (size<<14) | 2;
	cmdbuf[0x104>>2] = (u32)tmp_fds;

	ret = svcSendSyncRequest(SOCU_handle);
	if(ret != 0) {
		free(tmp_fds);
		errno = SYNC_ERROR;
		return ret;
	}

	cmdbuf[0x100>>2] = saved_threadstorage[0];
        cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret == 0)
		ret = _net_convert_error(cmdbuf[2]);

	if(ret < 0) {
		free(tmp_fds);
		errno = -ret;
		return -1;
	}

	for(i = 0; i < nfds; ++i)
		fds[i].revents = tmp_fds[i].revents;

	free(tmp_fds);

	return ret;
}
