#include "soc_common.h"
#include <errno.h>
#include <sys/socket.h>
#include <3ds/ipc.h>

static int     soc_open(struct _reent *r, void *fileStruct, const char *path, int flags, int mode);
static int     soc_close(struct _reent *r, void *fd);
static ssize_t soc_write(struct _reent *r, void *fd, const char *ptr, size_t len);
static ssize_t soc_read(struct _reent *r, void *fd, char *ptr, size_t len);

static devoptab_t
soc_devoptab =
{
  .name         = "soc",
  .structSize   = sizeof(Handle),
  .open_r       = soc_open,
  .close_r      = soc_close,
  .write_r      = soc_write,
  .read_r       = soc_read,
  .seek_r       = NULL,
  .fstat_r      = NULL,
  .stat_r       = NULL,
  .link_r       = NULL,
  .unlink_r     = NULL,
  .chdir_r      = NULL,
  .rename_r     = NULL,
  .mkdir_r      = NULL,
  .dirStateSize = 0,
  .diropen_r    = NULL,
  .dirreset_r   = NULL,
  .dirnext_r    = NULL,
  .dirclose_r   = NULL,
  .statvfs_r    = NULL,
  .ftruncate_r  = NULL,
  .fsync_r      = NULL,
  .deviceData   = 0,
  .chmod_r      = NULL,
  .fchmod_r     = NULL,
};

static Result SOCU_Initialize(Handle memhandle, u32 memsize)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,1,4); // 0x10044
	cmdbuf[1] = memsize;
	cmdbuf[2] = IPC_Desc_CurProcessHandle();
	cmdbuf[4] = IPC_Desc_SharedHandles(1);
	cmdbuf[5] = memhandle;

	ret = svcSendSyncRequest(SOCU_handle);
	if(ret != 0) {
		errno = SYNC_ERROR;
		return ret;
	}

	return cmdbuf[1];
}

static Result SOCU_Shutdown(void)
{
	Result ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x19,0,0); // 0x190000

	ret = svcSendSyncRequest(SOCU_handle);
	if(ret != 0) {
		errno = SYNC_ERROR;
		return ret;
	}

	return cmdbuf[1];
}

Result socInit(u32* context_addr, u32 context_size)
{
	Result ret = 0;

	/* check that the "soc" device doesn't already exist */
	int dev = FindDevice("soc:");
	if(dev >= 0)
		return -1;

	ret = svcCreateMemoryBlock(&socMemhandle, (u32)context_addr, context_size, 0, 3);
	if(ret != 0) return ret;

	ret = srvGetServiceHandle(&SOCU_handle, "soc:U");
	if(ret != 0)
	{
		svcCloseHandle(socMemhandle);
		socMemhandle = 0;
		return ret;
	}

	ret = SOCU_Initialize(socMemhandle, context_size);
	if(ret != 0)
	{
		svcCloseHandle(socMemhandle);
		svcCloseHandle(SOCU_handle);
		socMemhandle = 0;
		SOCU_handle = 0;
		return ret;
	}

	/* add the "soc" device */
	dev = AddDevice(&soc_devoptab);
	if(dev < 0)
	{
		svcCloseHandle(socMemhandle);
		svcCloseHandle(SOCU_handle);
		socMemhandle = 0;
		SOCU_handle = 0;
		return dev;
	}

	return 0;
}

Result socExit(void)
{
	Result ret = 0;
	int dev;

	svcCloseHandle(socMemhandle);
	socMemhandle = 0;

	ret = SOCU_Shutdown();

	svcCloseHandle(SOCU_handle);
	SOCU_handle = 0;

	dev = FindDevice("soc:");
	if(dev >= 0)
		RemoveDevice("soc:");

	return ret;
}

static int
soc_open(struct _reent *r,
         void          *fileStruct,
         const char    *path,
         int           flags,
         int           mode)
{
	r->_errno = ENOSYS;
	return -1;
}

static int
soc_close(struct _reent *r,
          void           *fd)
{
	Handle sockfd = *(Handle*)fd;

	int ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,1,2); // 0xB0042
	cmdbuf[1] = (u32)sockfd;
	cmdbuf[2] = IPC_Desc_CurProcessHandle();

	ret = svcSendSyncRequest(SOCU_handle);
	if(ret != 0) {
		errno = SYNC_ERROR;
		return ret;
	}

	ret = (int)cmdbuf[1];
	if(ret == 0)
		ret =_net_convert_error(cmdbuf[2]);

	if(ret < 0) {
		errno = -ret;
		return -1;
	}

	return 0;
}

static ssize_t
soc_write(struct _reent *r,
          void          *fd,
          const char    *ptr,
          size_t        len)
{
	Handle sockfd = *(Handle*)fd;
	return soc_sendto(sockfd, ptr, len, 0, NULL, 0);
}

static ssize_t
soc_read(struct _reent *r,
         void          *fd,
         char          *ptr,
         size_t        len)
{
	Handle sockfd = *(Handle*)fd;
	return soc_recvfrom(sockfd, ptr, len, 0, NULL, 0);
}
