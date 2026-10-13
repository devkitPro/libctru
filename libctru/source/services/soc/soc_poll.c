#include "soc_common.h"
#include <errno.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <3ds/ipc.h>
#include <3ds/os.h>

// The SOC service doesn't wake a poll() waiting for POLLOUT when a non-blocking connect() completes:
// it only notices once the poll times out. Waits that include POLLOUT are therefore split into slices.
#define POLLOUT_SLICE_MS 10

static int soc_poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	int ret = 0;
	u32 size = sizeof(struct pollfd)*nfds;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	cmdbuf[0] = IPC_MakeHeader(0x14,2,4); // 0x140084
	cmdbuf[1] = (u32)nfds;
	cmdbuf[2] = (u32)timeout;
	cmdbuf[3] = IPC_Desc_CurProcessId();
	cmdbuf[5] = IPC_Desc_StaticBuffer(size,10);
	cmdbuf[6] = (u32)fds;

	u32 * staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(size,0);
	staticbufs[1] = (u32)fds;

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

	return ret;
}

static s64 elapsed_ms(u64 start)
{
	return (svcGetSystemTick() - start) * 1000 / SYSCLOCK_ARM11;
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
	int ret = 0;
	nfds_t i;
	bool sliced = false;
	u64 start = svcGetSystemTick();

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
		if (fds[i].fd >= 0) {
			tmp_fds[i].fd = soc_get_fd(fds[i].fd);
			if(tmp_fds[i].fd < 0) {
				errno = -tmp_fds[i].fd;
				free(tmp_fds);
				return -1;
			}

			// a failed connect() is only reported as readable, so watch for that too
			if(fds[i].events & POLLOUT) {
				tmp_fds[i].events |= POLLIN;
				if(timeout != 0)
					sliced = true;
			}
		}
		// negative fds are ignored, however 3ds poll only
		// ignores fd's that are -1.
		// this forces negative fds to -1 so they are ignored without error.
		else {
			tmp_fds[i].fd = -1;
		}
	}

	for(;;) {
		int slice = timeout;
		if(sliced) {
			slice = POLLOUT_SLICE_MS;
			if(timeout > 0) {
				s64 left = timeout - elapsed_ms(start);
				if(left < slice)
					slice = left > 0 ? left : 0;
			}
		}

		for(i = 0; i < nfds; ++i)
			tmp_fds[i].revents = 0;

		ret = soc_poll(tmp_fds, nfds, slice);
		if(ret < 0)
			break;

		ret = 0;
		for(i = 0; i < nfds; ++i) {
			short revents = tmp_fds[i].revents;

			if((fds[i].events & POLLOUT) && (revents & POLLIN) && !(revents & POLLOUT)) {
				if(soc_connection_error(fds[i].fd))
					revents |= POLLERR | POLLHUP; // the connection failed or was reset
				else if(!(fds[i].events & POLLIN))
					tmp_fds[i].events &= ~POLLIN; // only data nobody asked for: stop watching for it
			}
			if(!(fds[i].events & POLLIN))
				revents &= ~POLLIN;

			fds[i].revents = revents;
			if(revents != 0)
				++ret;
		}

		if(ret != 0 || !sliced || (timeout > 0 && elapsed_ms(start) >= timeout))
			break;
	}

	free(tmp_fds);

	return ret;
}
