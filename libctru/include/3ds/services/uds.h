/**
 * @file uds.h
 * @brief UDS(NWMUDS) local-WLAN service. https://3dbrew.org/wiki/NWM_Services
 */
#pragma once

/// Maximum number of nodes(devices) that can be connected to the network.
#define UDS_MAXNODES 16

/// Broadcast value for NetworkNodeID / alias for all NetworkNodeIDs.
#define UDS_BROADCAST_NETWORKNODEID 0xFFFF

/// NetworkNodeID for the host(the first node).
#define UDS_HOST_NETWORKNODEID 0x1

/// Default recv_buffer_size that can be used for udsBind() input / code which uses udsBind() internally.
#define UDS_DEFAULT_RECVBUFSIZE 0x2E30

/// Max size of user data-frames.
#define UDS_DATAFRAME_MAXSIZE 0x5C6

/// Check whether a fatal udsSendTo error occured(some error(s) from udsSendTo() can be ignored, but the frame won't be sent when that happens).
#define UDS_CHECK_SENDTO_FATALERROR(x) (R_FAILED(x) && x!=0xC86113F0)

/// Node info struct.
typedef struct {
	u64 uds_friendcodeseed;//UDS version of the FriendCodeSeed.

	union {
		u8 usercfg[0x18];//This is the first 0x18-bytes from this config block: https://www.3dbrew.org/wiki/Config_Savegame#0x000A0000_Block

		struct {
			u16 username[10];

			u16 unk_x1c;//Unknown, normally zero. Set to 0x0 with the output from udsScanBeacons().
			u8 flag;//"u8 flag, unknown. Originates from the u16 bitmask in the beacon node-list header. This flag is normally 0 since that bitmask is normally 0?"
			u8 pad_x1f;//Unknown, normally zero.
		};
	};

	//The rest of this is initialized by NWM-module.
	u16 NetworkNodeID;
	u16 pad_x22;//Unknown, normally zero?
	u32 word_x24;//Normally zero?
} udsNodeInfo;

/// Connection status struct.
typedef struct {
	u32 status;
	u32 unk_x4;
	u16 cur_NetworkNodeID;//"u16 NetworkNodeID for this device."
	u16 unk_xa;
	u32 unk_xc[0x20>>2];

	u8 total_nodes;
	u8 max_nodes;
	u16 node_bitmask;//"This is a bitmask of NetworkNodeIDs: bit0 for NetworkNodeID 0x1(host), bit1 for NetworkNodeID 0x2(first original client), and so on."
} udsConnectionStatus;

/// Network struct stored as big-endian.
typedef struct {
	u8 host_macaddress[6];
	u8 channel;//Wifi channel for this network. If you want to create a network on a specific channel instead of the system selecting it, you can set this to a non-zero channel value.
	u8 pad_x7;

	u8 initialized_flag;//Must be non-zero otherwise NWM-module will use zeros internally instead of the actual field data, for most/all(?) of the fields in this struct.

	u8 unk_x9[3];

	u8 oui_value[3];//"This is the OUI value for use with the beacon tags. Normally this is 001F32."
	u8 oui_type;//"OUI type (21/0x15)"

	u32 wlancommID;//Unique local-WLAN communications ID for each application.
	u8 id8;//Additional ID that can be used by the application for different types of networks.
	u8 unk_x15;

	u16 attributes;//See the UDSNETATTR enum values below.

	u32 networkID;

	u8 total_nodes;
	u8 max_nodes;
	u8 unk_x1e;
	u8 unk_x1f;
	u8 unk_x20[0x1f];

	u8 appdata_size;
	u8 appdata[0xc8];
} udsNetworkStruct;

typedef struct {
	u32 BindNodeID;
	Handle event;
	bool spectator;
} udsBindContext;

/// General NWM input structure used for AP scanning.
typedef struct {
	u16 unk_x0;
	u16 unk_x2;
	u16 unk_x4;
	u16 unk_x6;

	u8 mac_address[6];

	u8 unk_xe[0x26];//Not initialized by dlp.
} nwmScanInputStruct;

/// General NWM output structure from AP scanning.
typedef struct {
	u32 maxsize;//"Max output size, from the command request."
	u32 size;//"Total amount of output data written relative to struct+0. 0xC when there's no entries."
	u32 total_entries;//"Total entries, 0 for none. "

	//The entries start here.
} nwmBeaconDataReplyHeader;

/// General NWM output structure from AP scanning, for each entry.
typedef struct {
	u32 size;//"Size of this entire entry. The next entry starts at curentry_startoffset+curentry_size."
	u8 unk_x4;
	u8 channel;//Wifi channel for the AP.
	u8 unk_x6;
	u8 unk_x7;
	u8 mac_address[6];//"AP MAC address."
	u8 unk_xe[6];
	u32 unk_x14;
	u32 val_x1c;//"Value 0x1C(size of this header and/or offset to the actual beacon data)."

	//The actual beacon data starts here.
} nwmBeaconDataReplyEntry;

/// Output structure generated from host scanning output.
typedef struct {
	nwmBeaconDataReplyEntry datareply_entry;
	udsNetworkStruct network;
	udsNodeInfo nodes[UDS_MAXNODES];
} udsNetworkScanInfo;

enum {
	UDSNETATTR_DisableConnectSpectators = BIT(0), //When set new Spectators are not allowed to connect.
	UDSNETATTR_DisableConnectClients = BIT(1), //When set new Clients are not allowed to connect.
	UDSNETATTR_x4 = BIT(2), //Unknown what this bit is for.
	UDSNETATTR_Default = BIT(15), //Unknown what this bit is for.
};

enum {
	UDS_SENDFLAG_Default = BIT(0), //Unknown what this bit is for.
	UDS_SENDFLAG_Broadcast = BIT(1) //When set, broadcast the data frame via the destination MAC address even when UDS_BROADCAST_NETWORKNODEID isn't used.
};

typedef enum {
	UDSCONTYPE_Client = 0x1,
	UDSCONTYPE_Spectator = 0x2
} udsConnectionType;

/**
 * @brief Initializes UDS.
 * @param sharedmem_size This must be 0x1000-byte aligned.
 * @param username Optional custom UTF-8 username(converted to UTF-16 internally) that other nodes on the UDS network can use. If not set the username from system-config is used. Max len is 10 characters without NUL-terminator.
 */
Result udsInit(size_t sharedmem_size, const char *username);

/// Exits UDS.
void udsExit(void);

/**
 * @brief Generates a NodeInfo struct with data loaded from system-config.
 * @param nodeinfo Output NodeInfo struct.
 * @param username If set, this is the UTF-8 string to convert for use in the struct. Max len is 10 characters without NUL-terminator.
 */
Result udsGenerateNodeInfo(udsNodeInfo *nodeinfo, const char *username);

/**
 * @brief Loads the UTF-16 username stored in the input NodeInfo struct, converted to UTF-8.
 * @param nodeinfo Input NodeInfo struct.
 * @param username This is the output UTF-8 string. Max len is 10 characters without NUL-terminator.
 */
Result udsGetNodeInfoUsername(const udsNodeInfo *nodeinfo, char *username);

/**
 * @brief Checks whether a NodeInfo struct was initialized by NWM-module(not any output from udsGenerateNodeInfo()).
 * @param nodeinfo Input NodeInfo struct.
 */
bool udsCheckNodeInfoInitialized(const udsNodeInfo *nodeinfo);

/**
 * @brief Generates a default NetworkStruct for creating networks.
 * @param network The output struct.
 * @param wlancommID Unique local-WLAN communications ID for each application.
 * @param id8 Additional ID that can be used by the application for different types of networks.
 * @param max_nodes Maximum number of nodes(devices) that can be connected to the network, including the host.
 */
void udsGenerateDefaultNetworkStruct(udsNetworkStruct *network, u32 wlancommID, u8 id8, u8 max_nodes);

/**
 * @brief Scans for networks via beacon-scanning.
 * @param outbuf Buffer which will be used by the beacon-scanning command and for the data parsing afterwards. Normally there's no need to use the contents of this buffer once this function returns.
 * @param maxsize Max size of the buffer.
 * @Param networks Ptr where the allocated udsNetworkScanInfo array buffer is written. The allocsize is sizeof(udsNetworkScanInfo)*total_networks.
 * @Param total_networks Total number of networks stored under the networks buffer.
 * @param wlancommID Unique local-WLAN communications ID for each application.
 * @param id8 Additional ID that can be used by the application for different types of networks.
 * @param host_macaddress When set, this code will only return network info from the specified host MAC address.
 * @connected When not connected to a network this *must* be false. When connected to a network this *must* be true.
 */
Result udsScanBeacons(void *outbuf, size_t maxsize, udsNetworkScanInfo **networks, size_t *total_networks, u32 wlancommID, u8 id8, const u8 *host_macaddress, bool connected);

/**
 * @brief This can be used by the host to set the appdata contained in the broadcasted beacons.
 * @param buf Appdata buffer.
 * @param size Size of the input appdata.
 */
Result udsSetApplicationData(const void *buf, size_t size);

/**
 * @brief This can be used while on a network(host/client) to get the appdata from the current beacon.
 * @param buf Appdata buffer.
 * @param size Max size of the output buffer.
 * @param actual_size If set, the actual size of the appdata written into the buffer is stored here.
 */
Result udsGetApplicationData(void *buf, size_t size, size_t *actual_size);

/**
 * @brief This can be used with a NetworkStruct, from udsScanBeacons() mainly, for getting the appdata.
 * @param buf Appdata buffer.
 * @param size Max size of the output buffer.
 * @param actual_size If set, the actual size of the appdata written into the buffer is stored here.
 */
Result udsGetNetworkStructApplicationData(const udsNetworkStruct *network, void *buf, size_t size, size_t *actual_size);

/**
 * @brief Create a bind.
 * @param bindcontext The output bind context.
 * @param NetworkNodeID This is the NetworkNodeID which this bind can receive data from.
 * @param spectator False for a regular bind, true for a spectator.
 * @param data_channel This is an arbitrary value to use for data-frame filtering. This bind will only receive data frames which contain a matching data_channel value, which was specified by udsSendTo(). The data_channel must be non-zero.
 * @param recv_buffer_size Size of the buffer under sharedmem used for temporarily storing received data-frames which are then loaded by udsPullPacket(). The system requires this to be >=0x5F4. UDS_DEFAULT_RECVBUFSIZE can be used for this.
 */
Result udsBind(udsBindContext *bindcontext, u16 NetworkNodeID, bool spectator, u8 data_channel, u32 recv_buffer_size);

/**
 * @brief Remove a bind.
 * @param bindcontext The bind context.
 */
Result udsUnbind(udsBindContext *bindcontext);

/**
 * @brief Waits for the bind event to occur, or checks if the event was signaled. This event is signaled every time new data is available via udsPullPacket().
 * @return Always true. However if wait=false, this will return false if the event wasn't signaled.
 * @param bindcontext The bind context.
 * @param nextEvent Whether to discard the current event and wait for the next event.
 * @param wait When true this will not return until the event is signaled. When false this checks if the event was signaled without waiting for it.
 */
bool udsWaitDataAvailable(const udsBindContext *bindcontext, bool nextEvent, bool wait);

/**
 * @brief Receives data over the network. This data is loaded from the recv_buffer setup by udsBind(). When a node disconnects, this will still return data from that node until there's no more frames from that node in the recv_buffer.
 * @param bindcontext Bind context.
 * @param buf Output receive buffer.
 * @param size Size of the buffer.
 * @param actual_size If set, the actual size written into the output buffer is stored here. This is zero when no data was received.
 * @param src_NetworkNodeID If set, the source NetworkNodeID is written here. This is zero when no data was received.
 */
Result udsPullPacket(const udsBindContext *bindcontext, void *buf, size_t size, size_t *actual_size, u16 *src_NetworkNodeID);

/**
 * @brief Sends data over the network.
 * @param dst_NetworkNodeID Destination NetworkNodeID.
 * @param data_channel See udsBind().
 * @param flags Send flags, see the UDS_SENDFLAG enum values.
 * @param buf Input send buffer.
 * @param size Size of the buffer.
 */
Result udsSendTo(u16 dst_NetworkNodeID, u8 data_channel, u8 flags, const void *buf, size_t size);

/**
 * @brief Gets the wifi channel currently being used.
 * @param channel Output channel.
 */
Result udsGetChannel(u8 *channel);

/**
 * @brief Starts hosting a new network.
 * @param network The NetworkStruct, you can use udsGenerateDefaultNetworkStruct() for generating this.
 * @param passphrase Raw input passphrase buffer.
 * @param passphrase_size Size of the passphrase buffer.
 * @param context Optional output bind context which will be created for this host, with NetworkNodeID=UDS_BROADCAST_NETWORKNODEID.
 * @param data_channel This is the data_channel value which will be passed to udsBind() internally.
 * @param recv_buffer_size This is the recv_buffer_size value which will be passed to udsBind() internally.
 */
Result udsCreateNetwork(const udsNetworkStruct *network, const void *passphrase, size_t passphrase_size, udsBindContext *context, u8 data_channel, u32 recv_buffer_size);

/**
 * @brief Connect to a network.
 * @param network The NetworkStruct, you can use udsScanBeacons() for this.
 * @param passphrase Raw input passphrase buffer.
 * @param passphrase_size Size of the passphrase buffer.
 * @param context Optional output bind context which will be created for this host.
 * @param recv_NetworkNodeID This is the NetworkNodeID passed to udsBind() internally.
 * @param connection_type Type of connection, see the udsConnectionType enum values.
 * @param data_channel This is the data_channel value which will be passed to udsBind() internally.
 * @param recv_buffer_size This is the recv_buffer_size value which will be passed to udsBind() internally.
 */
Result udsConnectNetwork(const udsNetworkStruct *network, const void *passphrase, size_t passphrase_size, udsBindContext *context, u16 recv_NetworkNodeID, udsConnectionType connection_type, u8 data_channel, u32 recv_buffer_size);

/**
 * @brief Stop hosting the network.
 */
Result udsDestroyNetwork(void);

/**
 * @brief Disconnect this client device from the network.
 */
Result udsDisconnectNetwork(void);

/**
 * @brief This can be used by the host to force-disconnect client(s).
 * @param NetworkNodeID Target NetworkNodeID. UDS_BROADCAST_NETWORKNODEID can be used to disconnect all clients.
 */
Result udsEjectClient(u16 NetworkNodeID);

/**
 * @brief This can be used by the host to force-disconnect the spectators. Afterwards new spectators will not be allowed to connect until udsAllowSpectators() is used.
 */
Result udsEjectSpectator(void);

/**
 * @brief This can be used by the host to update the network attributes. If bitmask 0x4 is clear in the input bitmask, this clears that bit in the value before actually writing the value into state. Normally you should use the below wrapper functions.
 * @param bitmask Bitmask to clear/set in the attributes. See the UDSNETATTR enum values.
 * @param flag When false, bit-clear, otherwise bit-set.
 */
Result udsUpdateNetworkAttribute(u16 bitmask, bool flag);

/**
 * @brief This uses udsUpdateNetworkAttribute() for (un)blocking new connections to this host.
 * @param block When true, block the specified connection types(bitmask set). Otherwise allow them(bitmask clear).
 * @param clients When true, (un)block regular clients.
 * @param flag When true, update UDSNETATTR_x4. Normally this should be false.
 */
Result udsSetNewConnectionsBlocked(bool block, bool clients, bool flag);

/**
 * @brief This uses udsUpdateNetworkAttribute() for unblocking new spectator connections to this host. See udsEjectSpectator() for blocking new spectators.
 */
Result udsAllowSpectators(void);

/**
 * @brief This loads the current ConnectionStatus struct.
 * @param output Output ConnectionStatus struct.
 */
Result udsGetConnectionStatus(udsConnectionStatus *output);

/**
 * @brief Waits for the ConnectionStatus event to occur, or checks if the event was signaled. This event is signaled when the data from udsGetConnectionStatus() was updated internally.
 * @return Always true. However if wait=false, this will return false if the event wasn't signaled.
 * @param nextEvent Whether to discard the current event and wait for the next event.
 * @param wait When true this will not return until the event is signaled. When false this checks if the event was signaled without waiting for it.
 */
bool udsWaitConnectionStatusEvent(bool nextEvent, bool wait);

/**
 * @brief This loads a NodeInfo struct for the specified NetworkNodeID. The broadcast alias can't be used with this.
 * @param NetworkNodeID Target NetworkNodeID.
 * @param output Output NodeInfo struct.
 */
Result udsGetNodeInformation(u16 NetworkNodeID, udsNodeInfo *output);

