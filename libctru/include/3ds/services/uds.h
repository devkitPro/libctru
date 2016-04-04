/**
 * @file uds.h
 * @brief UDS(NWMUDS) local-WLAN service. https://3dbrew.org/wiki/NWM_Services
 */
#pragma once

/// Node info struct.
typedef struct {
	u64 uds_friendcodeseed;//UDS version of the FriendCodeSeed.
	u8 usercfg[0x18];//This is the first 0x18-bytes from this config block: https://www.3dbrew.org/wiki/Config_Savegame#0x000A0000_Block
	u32 words_x20[2];//Not initialized by DLP-sysmodule.
} udsNodeInfo;

/// Network struct stored as big-endian.
typedef struct {
	u8 host_macaddress[6];
	u8 unk_x6[2];

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

enum {
	UDSNETATTR_DisableConnectClients = BIT(1), //When set new Clients are not allowed to connect.
	UDSNETATTR_DisableConnectSpectators = BIT(2), //When set new Spectators are (probably) not allowed to connect.
	UDSNETATTR_Default = BIT(15), //Unknown what this bit is for.
};

enum {
	UDS_SENDFLAG_Default = BIT(0), //Unknown what this bit is for.
	UDS_SENDFLAG_Broadcast = BIT(1) //When set, broadcast the data frame even when UDS_BROADCAST_NETWORKNODEID isn't used. Needs verified.
};

typedef enum {
	UDSCONTYPE_Client = 0x1,
	UDSCONTYPE_Spectator = 0x2
} udsConnectionType;

/// Maximum number of nodes(devices) that can be connected to the network.
#define UDS_MAXNODES 16

/// Broadcast value for NetworkNodeID / alias for all NetworkNodeIDs.
#define UDS_BROADCAST_NETWORKNODEID 0xFFFF

/// Default value that can be used for udsSendTo() input8.
#define UDS_SEND_INPUT8_DEFAULT 0x2

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
 * @param username If set, this is the UTF-8 string to convert for use in the struct. Max len is 10 characters without NUL-terminator.
 */
Result udsGenerateNodeInfo(udsNodeInfo *nodeinfo, const uint8_t *username);

/**
 * @brief Loads the UTF-16 username stored in the input NodeInfo struct, converted to UTF-8.
 * @param username This is the output UTF-8 string. Max len is 10 characters without NUL-terminator.
 */
Result udsGetNodeInfoUsername(udsNodeInfo *nodeinfo, uint8_t *username);

/**
 * @brief Generates a default NetworkStruct for creating networks.
 * @param network The output struct.
 * @param wlancommID Unique local-WLAN communications ID for each application.
 * @param id8 Additional ID that can be used by the application for different types of networks.
 * @param max_nodes Maximum number of nodes(devices) that can be connected to the network, including the host.
 */
void udsGenerateDefaultNetworkStruct(udsNetworkStruct *network, u32 wlancommID, u8 id8, u8 max_nodes);

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
 * @brief Sends data over the network.
 * @param dst_NetworkNodeID Destination NetworkNodeID.
 * @param input8 UDS_SEND_INPUT8_DEFAULT can be used for this. It's unknown what this field is actually for.
 * @param flags Send flags, see the UDS_SENDFLAG enum values.
 * @param buf Input send buffer.
 * @param size Size of the buffer.
 */
Result udsSendTo(u16 dst_NetworkNodeID, u8 input8, u8 flags, void* buf, size_t size);

/**
 * @brief Starts hosting a new network.
 * @param network The NetworkStruct, you can use udsGenerateDefaultNetworkStruct() for generating this.
 * @param passphrase Raw input passphrase buffer.
 * @param passphrase_size Size of the passphrase buffer.
 * @param bindcontext Output bind context which will be created for this host, with NetworkNodeID=UDS_BROADCAST_NETWORKNODEID.
 */
Result udsCreateNetwork(udsNetworkStruct *network, void* passphrase, size_t passphrase_size, udsBindContext *bindcontext);

/**
 * @brief Stop hosting the network.
 */
Result udsDestroyNetwork(void);

