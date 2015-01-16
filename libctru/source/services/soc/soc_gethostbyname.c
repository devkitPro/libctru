#include "soc_common.h"
#include <netdb.h>

#define MAX_HOSTENT_RESULTS 16
static struct hostent	SOC_hostent;
static char						*SOC_hostent_results[MAX_HOSTENT_RESULTS+1];
static char						*SOC_hostent_alias = NULL;

int h_errno = 0;

struct hostent* gethostbyname(const char *name)
{
	int ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();
	u32 saved_threadstorage[2];
	static u8 outbuf[0x1A88];

	cmdbuf[0] = 0x000D0082;
	cmdbuf[1] = strlen(name)+1;
	cmdbuf[2] = sizeof(outbuf);
	cmdbuf[3] = ((strlen(name)+1) << 14) | 0xC02;
	cmdbuf[4] = (u32)name;

	saved_threadstorage[0] = cmdbuf[0x100>>2];
	saved_threadstorage[1] = cmdbuf[0x104>>2];

	cmdbuf[0x100>>2] = (sizeof(outbuf) << 14) | 2;
	cmdbuf[0x104>>2] = (u32)outbuf;

	if(( ret = svcSendSyncRequest(SOCU_handle))!=0)return NULL;

	cmdbuf[0x100>>2] = saved_threadstorage[0];
				cmdbuf[0x104>>2] = saved_threadstorage[1];

	ret = (int)cmdbuf[1];
	if(ret==0)ret = _net_convert_error(cmdbuf[2]);
				if(ret<0)SOCU_errno = ret;
	/* TODO: set h_errno based on SOCU_errno */

	if(ret<0)return NULL;

	u32 num_results, i;
	memcpy(&num_results, (char*)outbuf+4, sizeof(num_results));
	if(num_results > MAX_HOSTENT_RESULTS)
		num_results = MAX_HOSTENT_RESULTS;

	SOC_hostent.h_name			= (char*)outbuf + 8;
	SOC_hostent.h_aliases		= &SOC_hostent_alias;
	SOC_hostent.h_addrtype	= AF_INET;
	SOC_hostent.h_length		= 4;
	SOC_hostent.h_addr_list	= SOC_hostent_results;

	SOC_hostent_alias = NULL;

	for(i = 0; i < num_results; ++i)
		SOC_hostent_results[i] = (char*)outbuf + 0x1908 + i*0x10;
	SOC_hostent_results[num_results] = NULL;

	return &SOC_hostent;
}
