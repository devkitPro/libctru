#include <3ds/synchronization.h>
#include <3ds/result.h>
#include <sys/socket.h>
#include <3ds/ipc.h>

#include <stdlib.h>
#include <errno.h>

#include "soc_common.h"

static int     soc_close(struct _reent *r, void *fd);
static ssize_t soc_write(struct _reent *r, void *fd, const char *ptr, size_t len);
static ssize_t soc_read(struct _reent *r, void *fd, char *ptr, size_t len);

typedef struct socSessionItem {
    struct socSessionItem *next;
    Handle session;
} socSessionItem;

static Handle          socMemhandle = 0;
static socSessionItem* soc_sessions = NULL;
static socSessionItem* soc_sessions_freelist = NULL;
static socSessionItem* soc_sessions_usedlist = NULL;
static RecursiveLock   soc_sessions_lock = { 0 };
static LightEvent      soc_session_avail_event = { 0 };


static Result socSessionsInit(u32 sessionCount)
{
    if (!sessionCount) {
        return MAKERESULT(RL_USAGE, RS_INVALIDARG, RM_SOC, RD_INVALID_SELECTION);
    }
    RecursiveLock_Init(&soc_sessions_lock);
    LightEvent_Init(&soc_session_avail_event, RESET_ONESHOT);
    
    RecursiveLock_Lock(&soc_sessions_lock);
    soc_sessions = (socSessionItem *)malloc(sessionCount * sizeof(socSessionItem));
    
    if (!soc_sessions)
    {
        RecursiveLock_Unlock(&soc_sessions_lock);
        return MAKERESULT(RL_FATAL, RS_OUTOFRESOURCE, RM_SOC, RD_OUT_OF_MEMORY);
    }
    
    socSessionItem* next_link = NULL;
    for (u32 i = 0; i < sessionCount; i++)
    {
        Result res = srvGetServiceHandle(&soc_sessions[i].session, "soc:U");
        if (R_FAILED(res))
        {
            for (u32 j = 0; j < i; j++)
                svcCloseHandle(soc_sessions[j].session);
            free(soc_sessions);
            soc_sessions = NULL;
            RecursiveLock_Unlock(&soc_sessions_lock);
            return res;
        }
        
        soc_sessions[i].next = next_link;
        next_link = &soc_sessions[i];
    }
    
    soc_sessions_freelist = next_link;
    
    RecursiveLock_Unlock(&soc_sessions_lock);
    return 0;
}

static Handle socSessionsTake() {
    while (1) {
        RecursiveLock_Lock(&soc_sessions_lock);
        if (soc_sessions_freelist) {
            // pop session from freelist
            socSessionItem *popped_item = soc_sessions_freelist;
            soc_sessions_freelist = popped_item->next;
            
            const Handle session = popped_item->session;
            // empty the holder, prevent any mismanagement from giving this session to another thread
            popped_item->session = 0;
            
            // push empty holder to usedlist
            popped_item->next = soc_sessions_usedlist;
            soc_sessions_usedlist = popped_item;
            
            RecursiveLock_Unlock(&soc_sessions_lock);
            return session;
        } else {
            RecursiveLock_Unlock(&soc_sessions_lock);
            // no session available, wait until there is one
            LightEvent_Wait(&soc_session_avail_event);
        }
    }
}

static void socSessionsGive(Handle session) {
    RecursiveLock_Lock(&soc_sessions_lock);
    
    // we must assume that the usedlist always contains at least one node
    // because it is impossible for this function to be called otherwise

    // pop empty holder from usedlist
    socSessionItem *node = soc_sessions_usedlist;
    soc_sessions_usedlist = node->next;

    // push session to freelist
    node->session = session;
    node->next = soc_sessions_freelist;
    soc_sessions_freelist = node;
    
    LightEvent_Signal(&soc_session_avail_event);
    RecursiveLock_Unlock(&soc_sessions_lock);
}

Result socSendSyncRequest() {
    Handle soc = socSessionsTake();
    Result res = svcSendSyncRequest(soc);
    socSessionsGive(soc);
    return res;
}

static void socSessionsFree() {
    RecursiveLock_Lock(&soc_sessions_lock);
    
    // we have to assume that all sessions are free in this state
    for (socSessionItem *i = soc_sessions_freelist; i; i = i->next) {
        svcCloseHandle(i->session);
    }
    
    free(soc_sessions);
    soc_sessions = NULL;
    soc_sessions_freelist = NULL;
    soc_sessions_usedlist = NULL;
    
    RecursiveLock_Unlock(&soc_sessions_lock);
}

static devoptab_t
soc_devoptab =
{
  .name         = "soc",
  .structSize   = sizeof(Handle),
  .open_r       = NULL,
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
	cmdbuf[2] = IPC_Desc_CurProcessId();
	cmdbuf[4] = IPC_Desc_SharedHandles(1);
	cmdbuf[5] = memhandle;

	ret = socSendSyncRequest();
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

	ret = socSendSyncRequest();
	if(ret != 0) {
		errno = SYNC_ERROR;
		return ret;
	}

	return cmdbuf[1];
}

Result socInitMulti(u32* context_addr, u32 context_size, u32 num_sessions)
{
	Result ret = 0;

	/* check that the "soc" device doesn't already exist */
	int dev = FindDevice("soc:");
	if(dev >= 0)
		return -1;

	ret = svcCreateMemoryBlock(&socMemhandle, (u32)context_addr, context_size, 0, 3);
	if(ret != 0) return ret;

	ret = socSessionsInit(num_sessions);
	if(ret != 0)
	{
		svcCloseHandle(socMemhandle);
		socMemhandle = 0;
		return ret;
	}

	ret = SOCU_Initialize(socMemhandle, context_size);
	if(ret != 0)
	{
		socSessionsFree();
		svcCloseHandle(socMemhandle);
		socMemhandle = 0;
		return ret;
	}

	/* add the "soc" device */
	dev = AddDevice(&soc_devoptab);
	if(dev < 0)
	{
		socSessionsFree();
		svcCloseHandle(socMemhandle);
		socMemhandle = 0;
		return dev;
	}

	return 0;
}

Result socInit(u32* context_addr, u32 context_size)
{
	return socInitMulti(context_addr, context_size, SOC_DEFAULT_NUM_SESSIONS);
}

Result socExit(void)
{
	// shut down sockets before closing the memory block
	Result ret = SOCU_Shutdown();
	
	svcCloseHandle(socMemhandle);
	socMemhandle = 0;

	socSessionsFree();

	int dev = FindDevice("soc:");
	if(dev >= 0)
		RemoveDevice("soc:");

	return ret;
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
	cmdbuf[2] = IPC_Desc_CurProcessId();

	ret = socSendSyncRequest();
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
