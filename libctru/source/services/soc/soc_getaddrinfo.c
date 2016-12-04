#include "soc_common.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <3ds/ipc.h>
#include <3ds/result.h>
#include <stdlib.h>

#define DEFAULT_NUM_ADDRINFO 4

typedef struct addrinfo_3ds_t addrinfo_3ds_t;
struct addrinfo_3ds_t
{
	s32                     ai_flags;
	s32                     ai_family;
	s32                     ai_socktype;
	s32                     ai_protocol;
	u32                     ai_addrlen;
	char                    ai_canonname[256];
	struct sockaddr_storage ai_addr;
};

void freeaddrinfo(struct addrinfo *ai)
{
	struct addrinfo *next_ai = ai;
	while(ai != NULL)
	{
		next_ai = ai->ai_next;
		free(ai);
		ai = next_ai;
	}
}

static struct addrinfo * buffer2addrinfo(addrinfo_3ds_t * entry)
{
	int ai_canonname_len = strnlen(entry->ai_canonname, sizeof(entry->ai_canonname));
	struct addrinfo *ai;
	size_t          len = sizeof(*ai)
	                    + sizeof(struct sockaddr_storage)
	                    + ai_canonname_len
	                    + 1;

	ai = (struct addrinfo*)calloc(1,len);
	if(ai != NULL)
	{
		ai->ai_canonname = (char*)ai + sizeof(*ai) + sizeof(struct sockaddr_storage);
		ai->ai_addr      = (struct sockaddr*)((char*)ai + sizeof(*ai));

		ai->ai_flags    = entry->ai_flags;
		ai->ai_family   = entry->ai_family;
		ai->ai_socktype = entry->ai_socktype;
		ai->ai_protocol = entry->ai_protocol;
		ai->ai_addrlen  = entry->ai_addrlen;

		memcpy(ai->ai_canonname, entry->ai_canonname, ai_canonname_len);
		memcpy(ai->ai_addr, &entry->ai_addr, ai->ai_addrlen);
		ai->ai_addr->sa_family = ntohs(ai->ai_addr->sa_family) & 0xFF; // Clear sa_len to match the API
	}
	return ai;
}


static int getaddrinfo_detail(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res, addrinfo_3ds_t *info, s32 info_count, s32 * count)
{
	int            i;
	u32            *cmdbuf = getThreadCommandBuffer();
	u32            saved_threadstorage[2];

	if(node == NULL && service == NULL)
	{
		return EAI_NONAME;
	}

	cmdbuf[ 0] = IPC_MakeHeader(0xF,4,6); // 0x00F0106
	cmdbuf[ 1] = node    == NULL ? 0 : strlen(node)+1;
	cmdbuf[ 2] = service == NULL ? 0 : strlen(service)+1;
	cmdbuf[ 3] = hints   == NULL ? 0 : sizeof(*hints);
	cmdbuf[ 4] = sizeof(addrinfo_3ds_t) * info_count;
	cmdbuf[ 5] = IPC_Desc_StaticBuffer(cmdbuf[1], 5);
	cmdbuf[ 6] = (u32)node;
	cmdbuf[ 7] = IPC_Desc_StaticBuffer(cmdbuf[2], 6);
	cmdbuf[ 8] = (u32)service;
	cmdbuf[ 9] = IPC_Desc_StaticBuffer(cmdbuf[3], 7);
	cmdbuf[10] = (u32)hints;

	u32 * staticbufs = getThreadStaticBuffers();

	// Save the thread storage values
	for(i = 0 ; i < 2 ; ++i)
		saved_threadstorage[i] = staticbufs[i];

	staticbufs[0] = IPC_Desc_StaticBuffer(sizeof(addrinfo_3ds_t) * info_count, 0);
	staticbufs[1] = (u32)info;

	int ret = svcSendSyncRequest(SOCU_handle);

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

	*count = cmdbuf[3];
	if(*count <= 0)
		*res = NULL;
	else if(*count <= info_count)
	{
		struct addrinfo **ptr = res;
		for(i = 0; i < *count; ++i)
		{
			*ptr = buffer2addrinfo(&info[i]);
			if(*ptr == NULL)
			{
				freeaddrinfo(*res);
				*res = NULL;
				return EAI_MEMORY;
			}
			ptr = &(*ptr)->ai_next;
		}
		*ptr = NULL;
	}

	return 0;
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
	Result ret;
	addrinfo_3ds_t *info = NULL, *tmp;
	s32            count = DEFAULT_NUM_ADDRINFO, info_count;

	if(node == NULL && service == NULL)
	{
		return EAI_NONAME;
	}

	do
	{
		info_count = count;
		tmp = (addrinfo_3ds_t*)realloc(info, sizeof(addrinfo_3ds_t) * info_count);
		if(tmp == NULL)
		{
			free(info);
			return EAI_MEMORY;
		}
		info = tmp;
		ret = getaddrinfo_detail(node,service,hints,res,info,info_count,&count);
	} while(count > info_count && R_SUCCEEDED(ret));

	free(info);
	return ret;
}
