#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/synchronization.h>
#include <3ds/services/uds.h>
#include <3ds/services/cfgu.h>
#include <3ds/services/ndm.h>
#include <3ds/ipc.h>
#include <3ds/util/utf.h>

Handle __uds_servhandle;
static int __uds_refcount;

u32 *__uds_sharedmem_addr;
static u32 __uds_sharedmem_size;
static Handle __uds_sharedmem_handle;

static Handle __uds_connectionstatus_event;

static u32 bind_allocbitmask;

static Result uds_Initialize(u32 sharedmem_size, const char *username);
static Result udsipc_InitializeWithVersion(udsNodeInfo *nodeinfo, Handle sharedmem_handle, u32 sharedmem_size, Handle *eventhandle);
static Result udsipc_Shutdown(void);

static Result udsipc_BeginHostingNetwork(const udsNetworkStruct *network, const void *passphrase, size_t passphrase_size);
static Result udsipc_ConnectToNetwork(const udsNetworkStruct *network, const void *passphrase, size_t passphrase_size, udsConnectionType connection_type);
static Result udsipc_SetProbeResponseParam(u32 oui, s8 data);

static Result udsipc_RecvBeaconBroadcastData(u8 *outbuf, u32 maxsize, nwmScanInputStruct *scaninput, u32 wlancommID, u8 id8, Handle event);
static Result udsipc_ScanOnConnection(u8 *outbuf, u32 maxsize, nwmScanInputStruct *scaninput, u32 wlancommID, u8 id8);

static Result udsipc_Bind(udsBindContext *bindcontext, u32 input0, u8 data_channel, u16 NetworkNodeID);
static Result udsipc_Unbind(udsBindContext *bindcontext);

static Result udsipc_DecryptBeaconData(udsNetworkStruct *network, u8 *tag0, u8 *tag1, udsNodeInfo *out);

static Result usd_parsebeacon(u8 *buf, u32 size, udsNetworkScanInfo *networkscan);

Result udsInit(size_t sharedmem_size, const char *username)
{
	Result ret=0;
	u32 ndm_state = 0;

	if (AtomicPostIncrement(&__uds_refcount)) return 0;

	ret = ndmuInit();
	if(R_SUCCEEDED(ret))
	{
		ndm_state = 1;
		ret = NDMU_EnterExclusiveState(EXCLUSIVE_STATE_LOCAL_COMMUNICATIONS);
		if(R_SUCCEEDED(ret))
		{
			ndm_state = 2;
		}
	}

	if(R_SUCCEEDED(ret))
	{
		ret = srvGetServiceHandle(&__uds_servhandle, "nwm::UDS");
		if(R_SUCCEEDED(ret))
		{
			ret = uds_Initialize(sharedmem_size, username);
			if (R_FAILED(ret))
			{
				svcCloseHandle(__uds_servhandle);
				__uds_servhandle = 0;
			}
		}
	}

	if (R_FAILED(ret))
	{
		if(ndm_state)
		{
			if(ndm_state==2)NDMU_LeaveExclusiveState();
			ndmuExit();
		}

		AtomicDecrement(&__uds_refcount);
	}

	bind_allocbitmask = 0;

	return ret;
}

void udsExit(void)
{
	if (AtomicDecrement(&__uds_refcount)) return;

	udsipc_Shutdown();

	svcCloseHandle(__uds_servhandle);
	__uds_servhandle = 0;

	svcCloseHandle(__uds_sharedmem_handle);
	__uds_sharedmem_handle = 0;
	__uds_sharedmem_size = 0;

	free(__uds_sharedmem_addr);
	__uds_sharedmem_addr = NULL;

	svcCloseHandle(__uds_connectionstatus_event);
	__uds_connectionstatus_event = 0;

	NDMU_LeaveExclusiveState();
	ndmuExit();
}

Result udsGenerateNodeInfo(udsNodeInfo *nodeinfo, const char *username)
{
	Result ret=0;
	ssize_t units=0;
	size_t len;
	u8 tmp[0x1c];

	memset(nodeinfo, 0, sizeof(udsNodeInfo));
	memset(tmp, 0, sizeof(tmp));

	ret = cfguInit();
	if (R_FAILED(ret))return ret;

	ret = CFGU_GetConfigInfoBlk2(sizeof(nodeinfo->uds_friendcodeseed), 0x00090000, (u8*)&nodeinfo->uds_friendcodeseed);
	if (R_FAILED(ret))
	{
		cfguExit();
		return ret;
	}

	ret = CFGU_GetConfigInfoBlk2(sizeof(tmp), 0x000A0000, tmp);
	if (R_FAILED(ret))
	{
		cfguExit();
		return ret;
	}

	memcpy(nodeinfo->usercfg, tmp, sizeof(nodeinfo->usercfg));

	if(username)
	{
		len = 10;

		memset(nodeinfo->username, 0, sizeof(nodeinfo->username));

		units = utf8_to_utf16((uint16_t*)nodeinfo->username, (uint8_t*)username, len);

		if(units < 0 || units > len)ret = -2;
	}

	cfguExit();

	return ret;
}

Result udsGetNodeInfoUsername(const udsNodeInfo *nodeinfo, char *username)
{
	ssize_t units=0;
	size_t len = 10;

	units = utf16_to_utf8((uint8_t*)username, (uint16_t*)nodeinfo->username, len);

	if(units < 0 || units > len)return -2;
	return 0;
}

bool udsCheckNodeInfoInitialized(const udsNodeInfo *nodeinfo)
{
	if(nodeinfo->NetworkNodeID)return true;
	return false;
}

void udsGenerateDefaultNetworkStruct(udsNetworkStruct *network, u32 wlancommID, u8 id8, u8 max_nodes)
{
	u8 oui_value[3] = {0x00, 0x1f, 0x32};

	memset(network, 0, sizeof(udsNetworkStruct));

	network->initialized_flag = 1;

	memcpy(network->oui_value, oui_value, 3);
	network->oui_type = 21;

	network->wlancommID = htonl(wlancommID);
	network->id8 = id8;

	network->attributes = htons(UDSNETATTR_Default);

	if(max_nodes > UDS_MAXNODES)max_nodes = UDS_MAXNODES;
	network->max_nodes = max_nodes;

	network->unk_x1f = 1;
}

static Result uds_Initialize(u32 sharedmem_size, const char *username)
{
	Result ret=0;
	udsNodeInfo nodeinfo;

	ret = udsGenerateNodeInfo(&nodeinfo, username);
	if (R_FAILED(ret))return ret;

	__uds_sharedmem_size = sharedmem_size;
	__uds_sharedmem_handle = 0;

	__uds_sharedmem_addr = memalign(0x1000, __uds_sharedmem_size);
	if(__uds_sharedmem_addr==NULL)ret = -1;

	if (R_SUCCEEDED(ret))
	{
		memset(__uds_sharedmem_addr, 0, __uds_sharedmem_size);
		ret = svcCreateMemoryBlock(&__uds_sharedmem_handle, (u32)__uds_sharedmem_addr, __uds_sharedmem_size, 0x0, MEMPERM_READ | MEMPERM_WRITE);
	}

	if (R_SUCCEEDED(ret))ret = udsipc_InitializeWithVersion(&nodeinfo, __uds_sharedmem_handle, __uds_sharedmem_size, &__uds_connectionstatus_event);

	if (R_FAILED(ret) && __uds_sharedmem_handle)
	{
		svcCloseHandle(__uds_sharedmem_handle);
		__uds_sharedmem_handle = 0;
		__uds_sharedmem_size = 0;
	}

	if(R_FAILED(ret) && __uds_sharedmem_addr)
	{
		free(__uds_sharedmem_addr);
		__uds_sharedmem_addr = NULL;
	}

	if(R_FAILED(ret) && __uds_connectionstatus_event)
	{
		svcCloseHandle(__uds_connectionstatus_event);
		__uds_connectionstatus_event = 0;
	}

	return ret;
}

Result udsCreateNetwork(const udsNetworkStruct *network, const void *passphrase, size_t passphrase_size, udsBindContext *context, u8 data_channel, u32 recv_buffer_size)
{
	Result ret=0;

	if(context)ret = udsBind(context, UDS_BROADCAST_NETWORKNODEID, false, data_channel, recv_buffer_size);
	if(R_FAILED(ret))return ret;

	ret = udsipc_SetProbeResponseParam(0x00210080, 0);
	if(R_FAILED(ret))
	{
		if(context)udsUnbind(context);
		return ret;
	}

	ret = udsipc_BeginHostingNetwork(network, passphrase, passphrase_size);
	if(R_FAILED(ret))
	{
		if(context)udsUnbind(context);
		return ret;
	}

	return ret;
}

Result udsConnectNetwork(const udsNetworkStruct *network, const void *passphrase, size_t passphrase_size, udsBindContext *context, u16 recv_NetworkNodeID, udsConnectionType connection_type, u8 data_channel, u32 recv_buffer_size)
{
	Result ret=0;
	bool spectator=false;

	if(connection_type==UDSCONTYPE_Spectator)spectator=true;

	if(context)ret = udsBind(context, recv_NetworkNodeID, spectator, data_channel, recv_buffer_size);
	if(R_FAILED(ret))return ret;

	ret = udsipc_ConnectToNetwork(network, passphrase, passphrase_size, connection_type);
	if(R_FAILED(ret))
	{
		udsDisconnectNetwork();
		if(context)udsUnbind(context);
	}

	return ret;
}

static Result udsipc_InitializeWithVersion(udsNodeInfo *nodeinfo, Handle sharedmem_handle, u32 sharedmem_size, Handle *eventhandle)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1B,12,2); // 0x1B0302
	cmdbuf[1]=sharedmem_size;
	memcpy(&cmdbuf[2], nodeinfo, sizeof(udsNodeInfo));
	cmdbuf[12] = 0x400;//version
	cmdbuf[13] = IPC_Desc_SharedHandles(1);
	cmdbuf[14] = sharedmem_handle;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))
	{
		if(eventhandle)*eventhandle = cmdbuf[3];
	}

	return ret;
}

static Result udsipc_Shutdown(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x3,0,0); // 0x30000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsEjectClient(u16 NetworkNodeID)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x5,1,0); // 0x50040
	cmdbuf[1]=NetworkNodeID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsEjectSpectator(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x6,0,0); // 0x60000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsUpdateNetworkAttribute(u16 bitmask, bool flag)
{
	u32* cmdbuf=getThreadCommandBuffer();

	if(flag)flag = 1;

	cmdbuf[0]=IPC_MakeHeader(0x7,2,0); // 0x70080
	cmdbuf[1]=bitmask;
	cmdbuf[2]=flag;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsSetNewConnectionsBlocked(bool block, bool clients, bool flag)
{
	u16 bitmask = 0;

	if(clients)bitmask |= UDSNETATTR_DisableConnectClients;
	if(flag)bitmask |= UDSNETATTR_x4;

	return udsUpdateNetworkAttribute(bitmask, block);
}

Result udsAllowSpectators(void)
{
	return udsUpdateNetworkAttribute(UDSNETATTR_DisableConnectSpectators, false);
}

Result udsDestroyNetwork(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x8,0,0); // 0x80000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsDisconnectNetwork(void)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xA,0,0); // 0xA0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsGetConnectionStatus(udsConnectionStatus *output)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xB,0,0); // 0xB0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))
	{
		if(output)memcpy(output, &cmdbuf[2], sizeof(udsConnectionStatus));
	}

	return ret;
}

bool udsWaitConnectionStatusEvent(bool nextEvent, bool wait)
{
	bool ret = true;
	u64 delayvalue = U64_MAX;

	if(!wait)delayvalue = 0;

	if(nextEvent)svcClearEvent(__uds_connectionstatus_event);

	if(svcWaitSynchronization(__uds_connectionstatus_event, delayvalue)!=0 && !wait)ret = false;

	if(!nextEvent)svcClearEvent(__uds_connectionstatus_event);

	return ret;
}

Result udsGetNodeInformation(u16 NetworkNodeID, udsNodeInfo *output)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xD,1,0); // 0xD0040
	cmdbuf[1]=NetworkNodeID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))
	{
		if(output)memcpy(output, &cmdbuf[2], sizeof(udsNodeInfo));
	}

	return ret;
}

Result udsScanBeacons(void *buf, size_t maxsize, udsNetworkScanInfo **networks, size_t *total_networks, u32 wlancommID, u8 id8, const u8 *host_macaddress, bool connected)
{
	Result ret=0;
	Handle event=0;
	u8 *outbuf = (u8*)buf;
	u32 entpos, curpos;
	nwmScanInputStruct scaninput;
	nwmBeaconDataReplyHeader *hdr;
	nwmBeaconDataReplyEntry *entry;
	udsNetworkScanInfo *networks_ptr;

	if(total_networks)*total_networks = 0;
	if(networks)*networks = NULL;

	memset(&scaninput, 0, sizeof(nwmScanInputStruct));

	scaninput.unk_x0 = 0x1;
	scaninput.unk_x2 = 0x2;
	scaninput.unk_x4 = 0x0421;
	scaninput.unk_x6 = 0x6e;

	memset(scaninput.mac_address, 0xff, sizeof(scaninput.mac_address));
	if(host_macaddress)memcpy(scaninput.mac_address, host_macaddress, sizeof(scaninput.mac_address));

	if(maxsize < sizeof(nwmBeaconDataReplyHeader))return -2;

	ret = svcCreateEvent(&event, RESET_ONESHOT);
	if(R_FAILED(ret))return ret;

	if(!connected)ret = udsipc_RecvBeaconBroadcastData(outbuf, maxsize, &scaninput, wlancommID, id8, event);
	if(connected)ret = udsipc_ScanOnConnection(outbuf, maxsize, &scaninput, wlancommID, id8);
	svcCloseHandle(event);
	if(R_FAILED(ret))return ret;

	hdr = (nwmBeaconDataReplyHeader*)outbuf;
	curpos = sizeof(nwmBeaconDataReplyHeader);

	if(hdr->maxsize != maxsize)return -2;
	if(hdr->size > maxsize)return -2;

	if(hdr->total_entries)
	{
		if(networks)
		{
			networks_ptr = malloc(sizeof(udsNetworkScanInfo) * hdr->total_entries);
			if(networks_ptr == NULL)return -1;
			if(total_networks)*total_networks = hdr->total_entries;
			memset(networks_ptr, 0, sizeof(udsNetworkScanInfo) * hdr->total_entries);
			*networks = networks_ptr;

			for(entpos=0; entpos<hdr->total_entries; entpos++)
			{
				if(curpos >= hdr->size)
				{
					ret = -2;
					break;
				}

				entry = (nwmBeaconDataReplyEntry*)&outbuf[curpos];
				if(entry->size > hdr->size || curpos + entry->size > hdr->size || entry->size <= sizeof(nwmBeaconDataReplyEntry))
				{
					ret = -2;
					break;
				}

				memcpy(&networks_ptr[entpos].datareply_entry, entry, sizeof(nwmBeaconDataReplyEntry));

				ret = usd_parsebeacon(&outbuf[curpos + sizeof(nwmBeaconDataReplyEntry)], entry->size - sizeof(nwmBeaconDataReplyEntry), &networks_ptr[entpos]);
				if(R_FAILED(ret))break;

				curpos+= entry->size;
			}
		}

		if(R_FAILED(ret))
		{
			if(networks)
			{
				free(*networks);
				*networks = NULL;
			}
			if(total_networks)*total_networks = 0;
		}
	}

	return ret;
}

Result udsBind(udsBindContext *bindcontext, u16 NetworkNodeID, bool spectator, u8 data_channel, u32 recv_buffer_size)
{
	u32 pos;

	memset(bindcontext, 0, sizeof(udsBindContext));

	if(spectator)
	{
		pos = 0;
		if((bind_allocbitmask & BIT(pos)))return -1;
	}
	else
	{
		for(pos=1; pos<UDS_MAXNODES+1; pos++)
		{
			if((bind_allocbitmask & BIT(pos)) == 0)break;
		}
		if(pos==UDS_MAXNODES)return -1;
	}

	bind_allocbitmask |= BIT(pos);

	bindcontext->BindNodeID = (pos<<1);
	if(spectator)bindcontext->BindNodeID |= spectator;

	bindcontext->spectator = spectator;

	return udsipc_Bind(bindcontext, recv_buffer_size, data_channel, NetworkNodeID);
}

Result udsUnbind(udsBindContext *bindcontext)
{
	Result ret=0;
	u32 bitpos = 0;

	if(bindcontext->event)
	{
		svcCloseHandle(bindcontext->event);
	}

	ret = udsipc_Unbind(bindcontext);

	if(!bindcontext->spectator)bitpos = bindcontext->BindNodeID>>1;
	bind_allocbitmask &= ~BIT(bitpos);

	memset(bindcontext, 0, sizeof(udsBindContext));

	return ret;
}

bool udsWaitDataAvailable(const udsBindContext *bindcontext, bool nextEvent, bool wait)
{
	bool ret = true;
	u64 delayvalue = U64_MAX;

	if(!wait)delayvalue = 0;

	if(nextEvent)svcClearEvent(bindcontext->event);

	if(svcWaitSynchronization(bindcontext->event, delayvalue)!=0 && !wait)ret = false;

	if(!nextEvent)svcClearEvent(bindcontext->event);

	return ret;
}

static Result usd_parsebeacon(u8 *buf, u32 size, udsNetworkScanInfo *networkscan)
{
	Result ret=0;

	u8 tagid, tag_datalen;
	u8 *tagptr;
	u8 oui[3] = {0x00, 0x1f, 0x32};
	u8 oui_type;
	u8 appdata_size;

	//Index0 = 21(0x15), index1=24(0x18), index2=25(0x19).
	u8 *tags_data[3] = {0};
	u32 tags_sizes[3] = {0};
	int tagindex;

	u8 tmp_tagdata[0xfe*2];

	if(size < 0xc)return -3;

	buf+=0xc;//Skip down to the tagged parameters in the beacon.
	size-=0xc;

	while(size)//Locate each of the Nintendo vendor tags which this code uses.
	{
		if(size < 2)return -3;

		tagid = buf[0];
		tag_datalen = buf[1];

		buf+= 0x2;
		size-= 0x2;

		if(tag_datalen > size)return -3;

		if(tagid==0xdd)//Vendor tag
		{
			if(tag_datalen < 4)return -3;

			if(memcmp(buf, oui, sizeof(oui))==0)
			{
				oui_type = buf[3];

				tagindex = -1;

				if(oui_type==21)
				{
					tagindex = 0;
				}
				else if(oui_type==24)
				{
					tagindex = 1;
				}
				else if(oui_type==25)
				{
					tagindex = 2;
				}

				if(tagindex>=0)
				{
					tags_data[tagindex] = buf;
					tags_sizes[tagindex] = tag_datalen;
				}
			}
		}

		buf+= tag_datalen;
		size-= tag_datalen;
	}

	for(tagindex=0; tagindex<3; tagindex++)//Verify that the required tags exist and have valid sizes.
	{
		if(tagindex!=2 && (tags_data[tagindex]==NULL || tags_sizes[tagindex]==0))return -3;

		if(tagindex && tags_sizes[tagindex] > 0xFE)return -3;
		if(tagindex==1 && tags_sizes[tagindex] < 0x12)return -3;

		if(tagindex==0 && ((tags_sizes[tagindex]<0x34) || (tags_sizes[tagindex]>0x34+0xC8)))return -3;
	}

	//Tag type21
	tagindex = 0;
	{
		tagptr = tags_data[tagindex];
		tag_datalen = tags_sizes[tagindex];

		appdata_size = tagptr[0x33];
		if((appdata_size > 0xC8) || (appdata_size > tag_datalen-0x34))return -3;//Verify the appdata size.

		memset(&networkscan->network, 0, sizeof(udsNetworkStruct));

		memcpy(&networkscan->network.oui_value, tagptr, 0x1F);

		networkscan->network.appdata_size = appdata_size;
		if(appdata_size)memcpy(networkscan->network.appdata, &tagptr[0x34], appdata_size);

		networkscan->network.initialized_flag = 1;
		networkscan->network.channel = networkscan->datareply_entry.channel;
		memcpy(networkscan->network.host_macaddress, networkscan->datareply_entry.mac_address, sizeof(networkscan->network.host_macaddress));
	}

	memset(tmp_tagdata, 0, sizeof(tmp_tagdata));
	for(tagindex=1; tagindex<3; tagindex++)
	{
		if(tags_data[tagindex])memcpy(&tmp_tagdata[0xfe * (tagindex-1)], tags_data[tagindex], tags_sizes[tagindex]);
	}

	ret = udsipc_DecryptBeaconData(&networkscan->network, tmp_tagdata, &tmp_tagdata[0xfe], networkscan->nodes);
	if(R_FAILED(ret))return ret;

	return 0;
}

static Result udsipc_RecvBeaconBroadcastData(u8 *outbuf, u32 maxsize, nwmScanInputStruct *scaninput, u32 wlancommID, u8 id8, Handle event)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0xF,16,4); // 0xF0404
	cmdbuf[1]=maxsize;
	memcpy(&cmdbuf[2], scaninput, sizeof(nwmScanInputStruct));
	cmdbuf[15]=wlancommID;
	cmdbuf[16]=id8;
	cmdbuf[17]=IPC_Desc_SharedHandles(1);
	cmdbuf[18]=event;
	cmdbuf[19]=IPC_Desc_Buffer(maxsize, IPC_BUFFER_W);
	cmdbuf[20]=(u32)outbuf;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsSetApplicationData(const void *buf, size_t size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x10,1,2); // 0x100042
	cmdbuf[1]=size;
	cmdbuf[2]=IPC_Desc_StaticBuffer(size, 4);
	cmdbuf[3]=(u32)buf;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsGetApplicationData(void *buf, size_t size, size_t *actual_size)
{
	u32* cmdbuf=getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	cmdbuf[0]=IPC_MakeHeader(0x11,1,0); // 0x110040
	cmdbuf[1]=size;

	u32 * staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(size,0);
	staticbufs[1] = (u32)buf;

	Result ret=0;
	ret=svcSendSyncRequest(__uds_servhandle);

	staticbufs[0] = saved_threadstorage[0];
	staticbufs[1] = saved_threadstorage[1];

	if(R_FAILED(ret))return ret;

	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))
	{
		if(actual_size)*actual_size = cmdbuf[2];
	}

	return ret;
}

Result udsGetNetworkStructApplicationData(const udsNetworkStruct *network, void *buf, size_t size, size_t *actual_size)
{
	if(network->appdata_size > sizeof(network->appdata))return -1;
	if(size > network->appdata_size)size = network->appdata_size;

	if(buf)memcpy(buf, network->appdata, size);

	if(actual_size)*actual_size = size;

	return 0;
}

static Result udsipc_Bind(udsBindContext *bindcontext, u32 input0, u8 data_channel, u16 NetworkNodeID)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x12,4,0); // 0x120100
	cmdbuf[1]=bindcontext->BindNodeID;
	cmdbuf[2]=input0;
	cmdbuf[3]=data_channel;
	cmdbuf[4]=NetworkNodeID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))
	{
		bindcontext->event = cmdbuf[3];
	}

	return ret;
}

static Result udsipc_Unbind(udsBindContext *bindcontext)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x13,1,0); // 0x130040
	cmdbuf[1]=bindcontext->BindNodeID;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsPullPacket(const udsBindContext *bindcontext, void *buf, size_t size, size_t *actual_size, u16 *src_NetworkNodeID)
{
	u32* cmdbuf=getThreadCommandBuffer();
	u32 saved_threadstorage[2];

	u32 aligned_size = (size+0x3) & ~0x3;

	cmdbuf[0]=IPC_MakeHeader(0x14,3,0); // 0x1400C0
	cmdbuf[1]=bindcontext->BindNodeID;
	cmdbuf[2]=aligned_size>>2;
	cmdbuf[3]=size;

	u32 * staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(aligned_size,0);
	staticbufs[1] = (u32)buf;

	Result ret=0;
	ret=svcSendSyncRequest(__uds_servhandle);

	staticbufs[0] = saved_threadstorage[0];
	staticbufs[1] = saved_threadstorage[1];

	if(R_FAILED(ret))return ret;

	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))
	{
		if(actual_size)*actual_size = cmdbuf[2];
		if(src_NetworkNodeID)*src_NetworkNodeID = cmdbuf[3];
	}

	return ret;
}

Result udsSendTo(u16 dst_NetworkNodeID, u8 data_channel, u8 flags, const void *buf, size_t size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	u32 aligned_size = (size+0x3) & ~0x3;

	cmdbuf[0]=IPC_MakeHeader(0x17,6,2); // 0x170182
	cmdbuf[1]=0x1;//Unused
	cmdbuf[2]=dst_NetworkNodeID;
	cmdbuf[3]=data_channel;
	cmdbuf[4]=aligned_size>>2;
	cmdbuf[5]=size;
	cmdbuf[6]=flags;
	cmdbuf[7]=IPC_Desc_StaticBuffer(aligned_size, 5);
	cmdbuf[8]=(u32)buf;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

Result udsGetChannel(u8 *channel)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1A,0,0); // 0x1A0000

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;
	ret = cmdbuf[1];

	if(R_SUCCEEDED(ret))
	{
		*channel = cmdbuf[2];
	}

	return ret;
}

static Result udsipc_BeginHostingNetwork(const udsNetworkStruct *network, const void *passphrase, size_t passphrase_size)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1D,1,4); // 0x1D0044
	cmdbuf[1]=passphrase_size;
	cmdbuf[2]=IPC_Desc_StaticBuffer(sizeof(udsNetworkStruct), 1);
	cmdbuf[3]=(u32)network;
	cmdbuf[4]=IPC_Desc_StaticBuffer(passphrase_size, 0);
	cmdbuf[5]=(u32)passphrase;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

static Result udsipc_ConnectToNetwork(const udsNetworkStruct *network, const void *passphrase, size_t passphrase_size, udsConnectionType connection_type)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x1E,2,4); // 0x1E0084
	cmdbuf[1]=connection_type;
	cmdbuf[2]=passphrase_size;
	cmdbuf[3]=IPC_Desc_StaticBuffer(sizeof(udsNetworkStruct), 1);
	cmdbuf[4]=(u32)network;
	cmdbuf[5]=IPC_Desc_StaticBuffer(passphrase_size, 0);
	cmdbuf[6]=(u32)passphrase;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

static Result udsipc_DecryptBeaconData(udsNetworkStruct *network, u8 *tag0, u8 *tag1, udsNodeInfo *out)
{
	u32* cmdbuf=getThreadCommandBuffer();
	u32 tagsize = 0xfe;

	u32 saved_threadstorage[2];

	cmdbuf[0]=IPC_MakeHeader(0x1F,0,6); // 0x1F0006
	cmdbuf[1]=IPC_Desc_StaticBuffer(sizeof(udsNetworkStruct), 1);
	cmdbuf[2]=(u32)network;
	cmdbuf[3]=IPC_Desc_StaticBuffer(tagsize, 2);
	cmdbuf[4]=(u32)tag0;
	cmdbuf[5]=IPC_Desc_StaticBuffer(tagsize, 3);
	cmdbuf[6]=(u32)tag1;

	u32 * staticbufs = getThreadStaticBuffers();
	saved_threadstorage[0] = staticbufs[0];
	saved_threadstorage[1] = staticbufs[1];

	staticbufs[0] = IPC_Desc_StaticBuffer(0x280,0);
	staticbufs[1] = (u32)out;

	Result ret=0;
	ret=svcSendSyncRequest(__uds_servhandle);

	staticbufs[0] = saved_threadstorage[0];
	staticbufs[1] = saved_threadstorage[1];

	if(R_FAILED(ret))return ret;

	return cmdbuf[1];
}

static Result udsipc_SetProbeResponseParam(u32 oui, s8 data)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x21,2,0); // 0x210080
	cmdbuf[1]=oui;
	cmdbuf[2]=data;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

static Result udsipc_ScanOnConnection(u8 *outbuf, u32 maxsize, nwmScanInputStruct *scaninput, u32 wlancommID, u8 id8)
{
	u32* cmdbuf=getThreadCommandBuffer();

	cmdbuf[0]=IPC_MakeHeader(0x22,16,2); // 0x220402
	cmdbuf[1]=maxsize;
	memcpy(&cmdbuf[2], scaninput, sizeof(nwmScanInputStruct));
	cmdbuf[15]=wlancommID;
	cmdbuf[16]=id8;
	cmdbuf[17]=IPC_Desc_Buffer(maxsize, IPC_BUFFER_W);
	cmdbuf[18]=(u32)outbuf;

	Result ret=0;
	if(R_FAILED(ret=svcSendSyncRequest(__uds_servhandle)))return ret;

	return cmdbuf[1];
}

