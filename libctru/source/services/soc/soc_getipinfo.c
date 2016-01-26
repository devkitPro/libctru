#include "soc_common.h"
#include <3ds/ipc.h>
#include <3ds/result.h>
#include <3ds/services/soc.h>

int SOCU_GetIPInfo(struct in_addr *ip, struct in_addr *netmask, struct in_addr *broadcast)
{
	SOCU_IPInfo info;
	socklen_t infolen = sizeof info;
	int ret = SOCU_GetNetworkOpt(SOL_CONFIG,NETOPT_IP_INFO,&info,&infolen);
	if(ret != 0) return ret;

	if(ip != NULL)
		*ip = info.ip;
	if(netmask != NULL)
		*netmask = info.netmask;
	if(broadcast != NULL)
		*broadcast = info.broadcast;

	return 0;
}
