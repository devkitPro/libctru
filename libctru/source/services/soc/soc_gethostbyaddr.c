#include "soc_common.h"
#include <netdb.h>

#define MAX_HOSTENT_RESULTS 16
static struct hostent SOC_hostent;
static char           *SOC_hostent_results[MAX_HOSTENT_RESULTS+1];
static char           *SOC_hostent_alias = NULL;

struct hostent* gethostbyaddr(const void *addr, socklen_t len, int type)
{
	int ret = 0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];
	static u8 outbuf[0x1A88];

	h_errno = 0;

	cmdbuf[0] = 0x000E00C2;
	cmdbuf[1] = len;
	cmdbuf[2] = type;
	cmdbuf[3] = sizeof(outbuf);
	cmdbuf[4] = (len << 14) | 0x1002;
	cmdbuf[5] = (u32)addr;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = (sizeof(outbuf) << 14) | 2;
	cmdbuf[0x104>>2] = (u32)outbuf;

	ret = svcSendSyncRequest(SOCU_handle);
	if(ret != 0) {
		h_errno = NO_RECOVERY;
		return NULL;
	}


	cmdbuf[0x100>>2] = saved_threadstorage[0];
	cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret == 0)
		ret = _net_convert_error(cmdbuf[2]);

	if(ret < 0) {
		/* TODO: set h_errno based on ret */
		h_errno = HOST_NOT_FOUND;
		return NULL;
	}

	u32 num_results, i;
	memcpy(&num_results, (char*)outbuf+4, sizeof(num_results));
	if(num_results > MAX_HOSTENT_RESULTS)
		num_results = MAX_HOSTENT_RESULTS;

	SOC_hostent.h_name      = (char*)outbuf + 8;
	SOC_hostent.h_aliases   = &SOC_hostent_alias;
	SOC_hostent.h_addrtype  = AF_INET;
	SOC_hostent.h_length    = 4;
	SOC_hostent.h_addr_list = SOC_hostent_results;

	SOC_hostent_alias = NULL;

	for(i = 0; i < num_results; ++i)
		SOC_hostent_results[i] = (char*)outbuf + 0x1908 + i*0x10;
	SOC_hostent_results[num_results] = NULL;

	SOC_hostent.h_addr = SOC_hostent.h_addr_list[0];

	return &SOC_hostent;
}

