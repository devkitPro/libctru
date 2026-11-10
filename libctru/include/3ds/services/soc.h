/**
 * @file soc.h
 * @brief SOC service for sockets communications.
 *
 * After initializing this service you will be able to use system calls from netdb.h, sys/socket.h etc.
 */
#pragma once
#include <netinet/in.h>
#include <sys/socket.h>

/// The config level to be used with @ref SOCU_GetNetworkOpt.
#define SOL_CONFIG 0xfffe

/// Options to be used with @ref SOCU_GetNetworkOpt.
typedef enum
{
	//Stack layer.
	NETOPT_STACK_RX_ALLOW_CB_ADDR     = 0x1001, ///< Internal function address used to check if RX is allowed (void*) [get].
	NETOPT_STACK_TX_ALLOW_CB_ADDR     = 0x1002, ///< Internal function address used to check if TX is allowed (void*) [get].
	NETOPT_STACK_DHCP_ERROR           = 0x1003, ///< DHCP error state (@see DHCP code) [get/set].
	NETOPT_STACK_MAC_ADDRESS          = 0x1004, ///< The MAC address of the interface (u32 [6]) [get].
	NETOPT_STACK_IS_READY             = 0x1005, ///< Whether stack is ready to be used (u32) [get].
	NETOPT_STACK_STATISTICS           = 0x1006, ///< Stack statistics (@see SOCU_StackStatistics) [get/set].
	NETOPT_STACK_IS_RX_TX_DISALLOW    = 0x1007, ///< Whether stack is disallowed to RX/TX (u32) [get/set].

	//ARP layer.
	NETOPT_ARP_TABLE                  = 0x3002, ///< The ARP table (@see SOCU_ARPTableEntry) [get].
	NETOPT_ARP_CLEAR_ALL              = 0x3003, ///< Remove all entries from the ARP table (no args) [set].
	NETOPT_ARP_ADD                    = 0x3004, ///< Add entry to the ARP table (@see SOCU_ARPTableEntry (only ip and mac fields are used)) [set].
	NETOPT_ARP_REMOVE                 = 0x3005, ///< Remove entry from the ARP table (@see SOCU_ARPTableEntry (only ip field is used)) [set].

	//IP layer.
	NETOPT_IP_STATISTICS              = 0x4001, ///< IP statistics (@see SOCU_IPStatistics) [get/set].
	NETOPT_IP_VERSION                 = 0x4002, ///< IP version (sa_family) in use (u32) [get].
	NETOPT_IP_INFO                    = 0x4003, ///< The current IP setup (@see SOCU_IPInfo) [get].
	NETOPT_IP_MTU                     = 0x4004, ///< The value of the IP MTU (s32) [get/set].
	NETOPT_IP_ROUTING_CAPACITY        = 0x4005, ///< Maximum number of routing table entries (u32) [get].
	NETOPT_IP_ROUTING_TABLE           = 0x4006, ///< The routing table (@see SOCU_RoutingTableEntry) [get].
	NETOPT_IP_ROUTING_ADD             = 0x4007, ///< Add entry to the routing table (@see SOCU_RoutingTableEntry (only dest_ip, netmask and gateway fields are used)) [set].
	NETOPT_IP_ROUTING_REMOVE          = 0x4008, ///< Remove entry from the routing table (@see SOCU_RoutingTableEntry (only dest_ip, netmask and gateway fields are used)) [set].
	NETOPT_IP_MY_ADDRESS              = 0x400A, ///< My IPv4 address (struct in_addr) [set].
	NETOPT_IP_GATEWAY_ADDRESS         = 0x400B, ///< Gateway IPv4 address (struct in_addr) [set].
	NETOPT_IP_SUBNET_MASK             = 0x400C, ///< Subnet mask in current network (struct in_addr) [set].
	NETOPT_IP_BROADCAST_MY_ADDRESS    = 0x400D, ///< Broadcast IPv4 address to the network (no args) [set].

	//ICMP layer.
	NETOPT_ICMP_STATISTICS            = 0x5001, ///< ICMP statistics (@see SOCU_ICMPStatistics) [get/set].

	//UDP layer.
	NETOPT_UDP_STATISTICS             = 0x8001, ///< UDP statistics (@see SOCU_UDPStatistics) [get/set].
	NETOPT_UDP_NUMBER                 = 0x8002, ///< The number of sockets in the UDP table (u32) [get].
	NETOPT_UDP_TABLE                  = 0x8003, ///< The table of opened UDP sockets (@see SOCU_UDPTableEntry) [get].

	//TCP layer.
	NETOPT_TCP_STATISTICS             = 0x9001, ///< TCP statistics (@see SOCU_TCPStatistics) [get/set].
	NETOPT_TCP_NUMBER                 = 0x9002, ///< The number of sockets in the TCP table (u32) [get].
	NETOPT_TCP_TABLE                  = 0x9003, ///< The table of opened TCP sockets (@see SOCU_TCPTableEntry) [get].
	NETOPT_TCP_TIME_WAIT              = 0x9006, ///< Time wait duration in 2 seconds (s32) [get/set].
	NETOPT_TCP_RETRANSMISSION_INITIAL = 0x9007, ///< Initial TCP retransmission interval in ms (s32) [get/set].
	NETOPT_TCP_RETRANSMISSION_MAX     = 0x9008, ///< Maximum TCP retransmission interval in seconds (s32) [get/set].
	NETOPT_TCP_RETRANSMISSION_MIN     = 0x9009, ///< Minimum TCP retransmission interval in seconds (s32) [get/set].

	//IGMP layer.
	NETOPT_IGMP_VERSION               = 0xA001, ///< IGMP version in use, this can't be downgraded from v2 to v1 (@see IGMP version) [get/set].

	//DNS layer.
	NETOPT_DNS_NAME                   = 0xB001, ///< DNS name (char [256]) [get/set].
	NETOPT_DNS_TABLE_CAPACITY         = 0xB002, ///< Maximum number of DNS table entries (u32) [get].
	NETOPT_DNS_TABLE                  = 0xB003, ///< The table of the DNS servers (@see SOCU_DNSTable) [get/set].

	//DHCP layer.
	NETOPT_DHCP_LEASE_TIME            = 0xC001, ///< The DHCP lease time remaining in seconds (s32) [get].

	//For backward compatibility.
	NETOPT_MAC_ADDRESS                = NETOPT_STACK_MAC_ADDRESS,
	NETOPT_ROUTING_TABLE              = NETOPT_IP_ROUTING_TABLE,
} NetworkOpt;

typedef struct
{
	u8             size;
	u8             sa_family;
	in_port_t      port;
	struct in_addr ip;
	u8             pad_0x08[20];
} SOCU_SockAddr;

///@name DHCP code
///@{
#define DHCP_CODE_SUCCESS       0x00000000 ///< Everything is OK.
#define DHCP_CODE_ABORTED       0xFFFF800D ///< Stack initialization was aborted.
#define DHCP_CODE_DUPLICATED    0xFFFF8026 ///< IP address is taken.
#define DHCP_CODE_LEASE_EXPIRED 0xFFFF8033 ///< DHCP lease has expired.
#define DHCP_CODE_TIMEOUT       0xFFFF8034 ///< DHCP request has timed out.
#define DHCP_CODE_REJECTED      0xFFFF8035 ///< Rejected by the DHCP server.
///@}

typedef struct
{
	u64 r_total_size;          ///< Total received data size in bytes (unicast + broadcast + multicast).
	u32 r_success_non_unicast; ///< Number of broadcast or multicast packets received.
	u32 r_success_unicast;     ///< Number of unicast packets received.
	u32 r_disallowed;          ///< Number of packets dropped because stack was disallowed to RX.
	u32 unused_0x14;
	u32 r_unknown_buffer_id;   ///< Number of packets dropped due to internal buffer ID mismatch.
	u32 s_success_unicast;     ///< Number of unicast packets sent.
	u64 s_total_size;          ///< Total sent data size in bytes (unicast + broadcast + multicast).
	u32 s_success_non_unicast; ///< Number of broadcast or multicast packets sent.
	u32 s_disallowed;          ///< Number of packets dropped because stack was disallowed to TX.
	u32 unused_0x30;
	u32 unused_0x34;
} SOCU_StackStatistics;

///@name ARP states
///@{
#define ARP_STATE_IDLE                0
#define ARP_STATE_REQUEST_SENT        1
#define ARP_STATE_VALID               2
#define ARP_STATE_DIRECT_REQUEST_SENT 3
///@}

/// One entry of the ARP table retrieved by using @ref SOCU_GetNetworkOpt and @ref NETOPT_ARP_TABLE.
typedef struct
{
	u32 state;         ///< @see ARP states defines.
	struct in_addr ip; ///< The IPv4 address associated to the entry.
	u8 mac[6];         ///< The MAC address associated to the entry.
	u8 padding[2];
} SOCU_ARPTableEntry;

typedef struct
{
	u32 r_attempt;          ///< Number of packets received (both success and error).
	u32 r_invalid_header;   ///< Number of packets dropped due to a header error.
	u32 r_invalid_ip;       ///< Number of packets dropped due to a destination IP error.
	u32 r_unknown_protocol; ///< Number of packets dropped because protocol was unknown (unsupported).
	u32 r_error;            ///< Number of packets dropped due to an error (including invalid header/IP and unknown protocol).
	u32 r_success;          ///< Number of packets received.
	u32 s_attempt;          ///< Number of packets attempted to send (both success and error).
	u32 s_error;            ///< Number of packets dropped due to an error (including invalid IP/size).
	u32 s_invalid_ip;       ///< Number of packets dropped due to a destination IP error.
	u32 r_fragment_timeout; ///< Number of fragmented packets dropped due to a timeout when waiting for other fragments.
	u32 r_fragment;         ///< Number of fragmented packets received.
	u32 r_fragment_last;    ///< Number of last fragmented packets received.
	u32 r_fragment_error;   ///< Number of fragmented packets dropped due to an error (including timeout).
	u32 s_fragment_last;    ///< Number of last fragmented packets sent.
	u32 s_invalid_size;     ///< Number of packets dropped due to a size error (e.g. packet was too large for MTU).
	u32 s_fragment;         ///< Number of fragmented packets sent.
} SOCU_IPStatistics;

/// Structure returned by @ref SOCU_GetNetworkOpt when using @ref NETOPT_IP_INFO.
typedef struct
{
	struct in_addr ip;        ///< Current IPv4 address.
	struct in_addr netmask;   ///< Current network mask.
	struct in_addr broadcast; ///< Current network broadcast address.
} SOCU_IPInfo;

// Linux netstat flags.
// NOTE : there are probably other flags supported, if you can forge ICMP requests please check for D and M flags.

/** The route uses a gateway */
#define ROUTING_FLAG_G 0x01

/// One entry of the routing table retrieved by using @ref SOCU_GetNetworkOpt and @ref NETOPT_ROUTING_TABLE.
typedef struct
{
	struct in_addr dest_ip; ///< Destination IP address of the route.
	struct in_addr netmask; ///< Mask used for this route.
	struct in_addr gateway; ///< Gateway address to reach the network.
	u32 flags;              ///< Linux netstat flags @see ROUTING_FLAG_G.
	u64 time;               ///< Number of milliseconds since 1st Jan 1900 00:00.
} SOCU_RoutingTableEntry;

typedef struct
{
	u32 r_attempt;          ///< Number of packets received (both success and error).
	u32 r_error;            ///< Number of packets dropped due to an error (e.g. invalid header/message ID).
	u32 r_dst_unreachable;  ///< Number of DST_UNREACHABLE packets received.
	u32 r_time_exceeded;    ///< Number of TIME_EXCEEDED packets received.
	u32 r_bad_ip_header;    ///< Number of BAD_IP_HEADER packets received.
	u32 r_src_quench;       ///< Number of SRC_QUENCH packets received.
	u32 r_redirect;         ///< Number of REDIRECT packets received.
	u32 r_echo_req;         ///< Number of ECHO_REQ packets received.
	u32 r_echo_rsp;         ///< Number of ECHO_RSP packets received.
	u32 r_timestamp_req;    ///< Number of TIMESTAMP_REQ packets received.
	u32 r_timestamp_res;    ///< Number of TIMESTAMP_RES packets received.
	u32 r_address_mask_req; ///< Number of ADDRESS_MASK_REQ packets received.
	u32 r_address_mask_res; ///< Number of ADDRESS_MASK_RES packets received.
	u32 s_echo_req;         ///< Number of ECHO_REQ packets sent.
	u32 s_error;            ///< Number of packets dropped due to an error (e.g. insufficient buffer).
	u32 s_dst_unreachable;  ///< Number of DST_UNREACHABLE packets sent.
	u32 s_time_exceeded;    ///< Number of TIME_EXCEEDED packets sent.
	u32 s_bad_ip_header;    ///< Number of BAD_IP_HEADER packets sent.
	u32 s_src_quench;       ///< Number of SRC_QUENCH packets sent.
	u32 s_redirect;         ///< Number of REDIRECT packets sent.
	u32 s_echo_req_user;    ///< Number of ECHO_REQ packets sent (by calling ICMPPing() see https://www.3dbrew.org/wiki/Socket_Services).
	u32 s_success;          ///< Number of packets sent.
	u32 unused_0x58;
	u32 unused_0x5c;
	u32 unused_0x60;
	u32 unused_0x64;
} SOCU_ICMPStatistics;

typedef struct
{
	u32 r_success;        ///< Number of packets received.
	u32 r_invalid_header; ///< Number of packets dropped due to a header error.
	u32 r_no_association; ///< Number of packets dropped because no listening socket exists for that data.
	u32 s_success;        ///< Number of packets sent.
	u32 s_error;          ///< Number of packets dropped due to an error (e.g. invalid IP, port, size etc...).
} SOCU_UDPStatistics;

/// One entry of the UDP sockets table retrieved by using @ref SOCU_GetNetworkOpt and @ref NETOPT_UDP_TABLE.
typedef struct
{
	SOCU_SockAddr local;  ///< Local address information.
	SOCU_SockAddr remote; ///< Remote address information.
} SOCU_UDPTableEntry;

///@name TCP states
///@{
#define TCP_STATE_CLOSED       1
#define TCP_STATE_LISTEN       2
#define TCP_STATE_SYN_SENT     3
#define TCP_STATE_SYN_RECEIVED 4
#define TCP_STATE_ESTABLISHED  5
#define TCP_STATE_FINWAIT1     6
#define TCP_STATE_FINWAIT2     7
#define TCP_STATE_CLOSE_WAIT   8
#define TCP_STATE_CLOSING      9
#define TCP_STATE_LAST_ACK     10
#define TCP_STATE_TIME_WAIT    11
///@}

typedef struct
{
	u32 s_syn;            ///< Number of SYN packets sent (connection attempt).
	u32 r_syn;            ///< Number of SYN packets received (incoming connection request).
	u32 conn_timeout;     ///< Number of connection timeouts.
	u32 conn_termination; ///< Number of connection terminations.
	u32 conn_active;      ///< Number of **currently** active connections.
	u32 r_attempt;        ///< Number of packets received (both success and error).
	u32 s_success_a;      ///< Number of packets sent.
	u32 s_success_b;      ///< Number of packets sent.
	u32 r_invalid_header; ///< Number of packets dropped due to a header error.
	u32 s_error;          ///< Number of packets dropped due to an error (e.g. insufficient buffer).
	u32 s_rst;            ///< Number of RST packets sent (connection termination).
} SOCU_TCPStatistics;

/// One entry of the TCP sockets table retrieved by using @ref SOCU_GetNetworkOpt and @ref NETOPT_TCP_TABLE.
typedef struct
{
	u32 state;            ///< @see TCP states defines.
	SOCU_SockAddr local;  ///< Local address information.
	SOCU_SockAddr remote; ///< Remote address information.
} SOCU_TCPTableEntry;

///@name IGMP version
///@{
#define IGMP_VERSION_1 1
#define IGMP_VERSION_2 2
///@}

/// One entry of the DNS servers table retrieved by using @ref SOCU_GetNetworkOpt and @ref NETOPT_DNS_TABLE.
typedef struct
{
	u32 family;        ///< Family of the address of the DNS server.
	struct in_addr ip; ///< IP of the DNS server.
	u8 padding[12];    ///< Matches the length required for IPv6 addresses.
} SOCU_DNSTableEntry;

/// DNS servers table retrieved by using @ref SOCU_GetNetworkOpt and @ref NETOPT_DNS_TABLE.
typedef struct
{
	SOCU_DNSTableEntry entries[4]; ///< DNS server entries (only 1 or 2 entries are set, it seems other 2 entries are intended for IPv6).
	char domain_name[256];         ///< Domain name of DNS server.
} SOCU_DNSTable;

/**
 * @brief Initializes the SOC service.
 * @param context_addr Address of a page-aligned (0x1000) buffer to be used.
 * @param context_size Size of the buffer, a multiple of 0x1000.
 * @note The specified context buffer can no longer be accessed by the process which called this function, since the userland permissions for this block are set to no-access.
 */
Result socInit(u32 *context_addr, u32 context_size);

/**
 * @brief Closes the soc service.
 * @note You need to call this in order to be able to use the buffer again.
 */
Result socExit(void);

/**
 * @brief Initializes the SOCP service.
 * @return 0 on success.
 */
Result socPInit(void);

/**
 * @brief Exits the SOCP service.
 */
void SocPExit(void);

// This is supposed to be in unistd.h but newlib only puts it for cygwin, waiting for newlib patch from dkA.
/**
 * @brief Gets the system's host ID.
 * @return The system's host ID.
 */
long gethostid(void);

// This is supposed to be in unistd.h but newlib only puts it for cygwin, waiting for newlib patch from dkA.
int gethostname(char *name, size_t namelen);

int SOCU_ShutdownSockets(void);

int SOCU_CloseSockets(void);

/**
 * @brief Retrieves information from the network configuration. Similar to getsockopt().
 * @param level   Only value allowed is @ref SOL_CONFIG.
 * @param optname The option to be retrieved.
 * @param optval  Will contain the output of the command.
 * @param optlen  Size of the optval buffer, will be updated to hold the size of the output.
 * @return 0 if successful. -1 if failed, and errno will be set accordingly. Can also return a system error code.
 */
int SOCU_GetNetworkOpt(int level, NetworkOpt optname, void *optval, socklen_t *optlen);

/**
 * @brief Gets the system's IP address, netmask, and subnet broadcast.
 * @return Result.
 */
int SOCU_GetIPInfo(struct in_addr *ip, struct in_addr *netmask, struct in_addr *broadcast);

/**
 * @brief Adds a global socket.
 * @param sockfd The socket fd.
 * @return Result.
 */
int SOCU_AddGlobalSocket(int sockfd);

/**
 * @brief Sets information to the network configuration. Similar to setsockopt().
 * @param level   Only value allowed is @ref SOL_CONFIG.
 * @param optname The option to be set.
 * @param optval  Value to set.
 * @param optlen  Size of the optval buffer.
 * @return 0 if successful. -1 if failed, and errno will be set accordingly. Can also return a system error code.
 */
int SOCP_SetNetworkOpt(int level, NetworkOpt optname, void *optval, socklen_t optlen);
