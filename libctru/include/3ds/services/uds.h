/**
 * @file uds.h
 * @brief UDS(NWMUDS) local-WLAN service. WARNING: This code is not ready to be used by applications yet. https://3dbrew.org/wiki/NWM_Services
 */
#pragma once

/// Maximum number of nodes(devices) that can be connected to the network.
#define UDS_MAXNODES 16

/// Broadcast value for NetworkNodeID / alias for all NetworkNodeIDs.
#define UDS_BROADCAST_NETWORKNODEID 0xFFFF

/// NetworkNodeID for the host(the first node).
#define UDS_HOST_NETWORKNODEID 0x1

/// Default value that can be used for udsSendTo() input8.
#define UDS_SEND_INPUT8_DEFAULT 0xF3

/// Node info struct.
typedef struct {
	u64 uds_friendcodeseed;//UDS version of the FriendCodeSeed.
	u8 usercfg[0x18];//This is the first 0x18-bytes from this config block: https://www.3dbrew.org/wiki/Config_Savegame#0x000A0000_Block

	//The rest of this is initialized by NWM-module.
	u16 NetworkNodeID;
	u16 pad_x22;
	u32 word_x24;
} udsNodeInfo;

/// Network struct stored as big-endian.
typedef struct {
	u8 host_macaddress[6];
	u8 hostmacaddr_flag;//"This flag being set to non-zero presumably indicates that the MAC address is set."
	u8 unk_x7;

	u8 initialized_flag;//Must be non-zero otherwise NWM-module will use zeros internally instead of the actual field data, for most/all(?) of the fields in this struct.

	u8 unk_x9[3];

	u8 oui_value[3];//"This is the OUI value for use with the beacon tags. Normally this is 001F32. "
	u8 oui_type;//"OUI type (21/0x15)"

	u32 wlancommID;//Unique local-WLAN communications ID for each application.
	u8 id8;//Additional ID that can be used by the application for different types of networks.
	u8 unk_x15;

	u16 attributes;//See the UDSNETATTR enum values below.
	u8 unk_x18[5];

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
	u32 unk_x4;
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
	u32 total_nodes;//Total number of nodes actually connected to the network, including the host.
	udsNodeInfo nodes[UDS_MAXNODES];
} udsNetworkScanInfo;

enum {
	UDSNETATTR_DisableConnectClients = BIT(1), //When set new Clients are (supposedly) not allowed to connect.
	UDSNETATTR_DisableConnectSpectators = BIT(2), //When set new Spectators are (probably) not allowed to connect.
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
Result udsInit(u32 sharedmem_size, const uint8_t *username);

/// Exits UDS.
void udsExit(void);

/**
 * @brief Generates a NodeInfo struct with data loaded from system-config.
 * @param nodeinfo Output NodeInfo struct.
 * @param username If set, this is the UTF-8 string to convert for use in the struct. Max len is 10 characters without NUL-terminator.
 */
Result udsGenerateNodeInfo(udsNodeInfo *nodeinfo, const uint8_t *username);

/**
 * @brief Loads the UTF-16 username stored in the input NodeInfo struct, converted to UTF-8.
 * @param nodeinfo Input NodeInfo struct.
 * @param username This is the output UTF-8 string. Max len is 10 characters without NUL-terminator.
 */
Result udsGetNodeInfoUsername(udsNodeInfo *nodeinfo, uint8_t *username);

/**
 * @brief Checks whether a NodeInfo struct was initialized by NWM-module(not any output from udsGenerateNodeInfo()).
 * @param nodeinfo Input NodeInfo struct.
 */
bool udsCheckNodeInfoInitialized(udsNodeInfo *nodeinfo);

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
 */
Result udsScanBeacons(u8 *outbuf, u32 maxsize, udsNetworkScanInfo **networks, u32 *total_networks, u32 wlancommID, u8 id8, u8 *host_macaddress);

/**
 * @brief This can be used by the host to set the appdata contained in the broadcasted beacons.
 * @param buf Appdata buffer.
 * @param size Size of the input appdata.
 */
Result udsSetApplicationData(u8 *buf, u32 size);

/**
 * @brief This can be used while on a network(host/client) to get the appdata from the current beacon.
 * @param buf Appdata buffer.
 * @param size Max size of the output buffer.
 * @param actual_size If set, the actual size of the appdata written into the buffer is stored here.
 */
Result udsGetApplicationData(u8 *buf, u32 size, u32 *actual_size);

/**
 * @brief This can be used with a NetworkStruct, from udsScanBeacons() mainly, for getting the appdata.
 * @param buf Appdata buffer.
 * @param size Max size of the output buffer.
 * @param actual_size If set, the actual size of the appdata written into the buffer is stored here.
 */
Result udsGetNetworkStructApplicationData(udsNetworkStruct *network, u8 *buf, u32 size, u32 *actual_size);

/**
 * @brief Create a bind.
 * @param bindcontext The output bind context.
 * @param NetworkNodeID This is the NetworkNodeID which this bind can receive data from.
 */
Result udsBind(udsBindContext *bindcontext, u16 NetworkNodeID);

/**
 * @brief Remove a bind.
 * @param bindcontext The bind context.
 */
Result udsUnbind(udsBindContext *bindcontext);

/**
 * @brief Waits for the bind event to occur, or checks if the event was signalled. This event is signalled every time new data is available via udsPullPacket().
 * @return Always true. However if wait=false, this will return false if the event wasn't signalled.
 * @param bindcontext The bind context.
 * @param nextEvent Whether to discard the current event and wait for the next event.
 * @param wait When true this will not return until the event is signalled. When false this checks if the event was signalled without waiting for it.
 */
bool udsWaitDataAvailable(udsBindContext *bindcontext, bool nextEvent, bool wait);

/**
 * @brief Receives data over the network.
 * @param bindcontext Bind context.
 * @param buf Output receive buffer.
 * @param size Size of the buffer.
 * @param actual_size If set, the actual size written into the output buffer is stored here. This is zero when no data was received.
 * @param src_NetworkNodeID If set, the source NetworkNodeID is written here. This is zero when no data was received.
 */
Result udsPullPacket(udsBindContext *bindcontext, void* buf, size_t size, size_t *actual_size, u16 *src_NetworkNodeID);

/**
 * @brief Sends data over the network.
 * @param dst_NetworkNodeID Destination NetworkNodeID.
 * @param input8 UDS_SEND_INPUT8_DEFAULT can be used for this. It's unknown what this field is actually for.
 * @param flags Send flags, see the UDS_SENDFLAG enum values.
 * @param buf Input send buffer.
 * @param size Size of the buffer.
 */
Result udsSendTo(u16 dst_NetworkNodeID, u8 input8, u8 flags, void* buf, size_t size);

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
 * @param bindcontext Output bind context which will be created for this host, with NetworkNodeID=UDS_BROADCAST_NETWORKNODEID.
 */
Result udsCreateNetwork(udsNetworkStruct *network, void* passphrase, size_t passphrase_size, udsBindContext *bindcontext);

/**
 * @brief Connect to a network.
 * @param network The NetworkStruct, you can use udsScanBeacons() for this.
 * @param passphrase Raw input passphrase buffer.
 * @param passphrase_size Size of the passphrase buffer.
 * @param bindcontext Output bind context which will be created for this host.
 * @param recv_NetworkNodeID This is the NetworkNodeID passed to udsBind() internally.
 * @param connection_type Type of connection, see the udsConnectionType enum values.
 */
Result udsConnectNetwork(udsNetworkStruct *network, void* passphrase, size_t passphrase_size, udsBindContext *context, u16 recv_NetworkNodeID, udsConnectionType connection_type);

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
 * @brief This can be used by the host to update the network attributes. If bitmask 0x4 is clear in the input bitmask, this clears that bit in the value before actually writing the value into state.
 * @param bitmask Bitmask to clear/set in the attributes. See the UDSNETATTR enum values.
 * @param flag When false, bit-clear, otherwise bit-set.
 */
Result udsUpdateNetworkAttribute(u16 bitmask, bool flag);

/**
 * @brief This uses udsUpdateNetworkAttribute() for (un)blocking new connections to this host with the specified type(s). This is what it was supposed to do, doesn't seem actually to affect new connections though.
 * @param block When true, block the specified connection types. Otherwise allow them.
 * @param clients When true, (un)block regular clients.
 * @param clients When true, (un)block spectators(?).
 */
Result udsSetNewConnectionsBlocked(bool block, bool clients, bool spectators);

