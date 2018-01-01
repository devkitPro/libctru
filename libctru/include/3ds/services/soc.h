/**
 * @file soc.h
 * @brief SOC service for sockets communications
 *
 * After initializing this service you will be able to use system calls from netdb.h, sys/socket.h etc.
 */
#pragma once
#include <netinet/in.h>
#include <sys/socket.h>

/// The config level to be used with @ref SOCU_GetNetworkOpt
#define SOL_CONFIG 0xfffe

/// Options to be used with @ref SOCU_GetNetworkOpt
typedef enum
{
	NETOPT_MAC_ADDRESS     = 0x1004, ///< The mac address of the interface (u32 mac[6])
	NETOPT_ARP_TABLE       = 0x3002, ///< The ARP table @see SOCU_ARPTableEntry
	NETOPT_IP_INFO         = 0x4003, ///< The cureent IP setup @see SOCU_IPInfo
	NETOPT_IP_MTU          = 0x4004, ///< The value of the IP MTU (u32)
	NETOPT_ROUTING_TABLE   = 0x4006, ///< The routing table @see SOCU_RoutingTableEntry
	NETOPT_UDP_NUMBER      = 0x8002, ///< The number of sockets in the UDP table (u32)
	NETOPT_UDP_TABLE       = 0x8003, ///< The table of opened UDP sockets @see SOCU_UDPTableEntry
	NETOPT_TCP_NUMBER      = 0x9002, ///< The number of sockets in the TCP table (u32)
	NETOPT_TCP_TABLE       = 0x9003, ///< The table of opened TCP sockets @see SOCU_TCPTableEntry
	NETOPT_DNS_TABLE       = 0xB003, ///< The table of the DNS servers @see SOCU_DNSTableEntry -- Returns a buffer of size 336 but only 2 entries are set ?
	NETOPT_DHCP_LEASE_TIME = 0xC001, ///< The DHCP lease time remaining, in seconds
} NetworkOpt;

/// One entry of the ARP table retrieved by using @ref SOCU_GetNetworkOpt and @ref NETOPT_ARP_TABLE
typedef struct
{
	u32 unk0; // often 2 ? state ?
	struct in_addr ip; ///< The IPv4 address associated to the entry
	u8 mac[6];         ///< The MAC address of associated to the entry
	u8 padding[2];
} SOCU_ARPTableEntry;

/// Structure returned by @ref SOCU_GetNetworkOpt when using @ref NETOPT_IP_INFO
typedef struct
{
	struct in_addr ip;        ///< Current IPv4 address
	struct in_addr netmask;   ///< Current network mask
	struct in_addr broadcast; ///< Current network broadcast address
} SOCU_IPInfo;

// Linux netstat flags
// NOTE : there are probably other flags supported, if you can forge ICMP requests please check for D and M flags

/** The route uses a gateway */
#define ROUTING_FLAG_G 0x01

/// One entry of the routing table retrieved by using @ref SOCU_GetNetworkOpt and @ref NETOPT_ROUTING_TABLE
typedef struct
{
	struct in_addr dest_ip; ///< Destination IP address of the route
	struct in_addr netmask; ///< Mask used for this route
	struct in_addr gateway; ///< Gateway address to reach the network
	u32 flags;              ///< Linux netstat flags @see ROUTING_FLAG_G
	u64 time;               ///< number of milliseconds since 1st Jan 1900 00:00.
} SOCU_RoutingTableEntry;

/// One entry of the UDP sockets table retrieved by using @ref SOCU_GetNetworkOpt and @ref NETOPT_UDP_TABLE
typedef struct
{
	struct sockaddr_storage local;  ///< Local address information
	struct sockaddr_storage remote; ///< Remote address information
} SOCU_UDPTableEntry;

///@name TCP states
///@{
#define TCP_STATE_CLOSED      1
#define TCP_STATE_LISTEN      2
#define TCP_STATE_ESTABLISHED 5
#define TCP_STATE_FINWAIT1    6
#define TCP_STATE_FINWAIT2    7
#define TCP_STATE_CLOSE_WAIT  8
#define TCP_STATE_LAST_ACK    9
#define TCP_STATE_TIME_WAIT   11
///@}

/// One entry of the TCP sockets table retrieved by using @ref SOCU_GetNetworkOpt and @ref NETOPT_TCP_TABLE
typedef struct
{
	u32 state;                      ///< @see TCP states defines
	struct sockaddr_storage local;  ///< Local address information
	struct sockaddr_storage remote; ///< Remote address information
} SOCU_TCPTableEntry;

/// One entry of the DNS servers table retrieved by using @ref SOCU_GetNetworkOpt and @ref NETOPT_DNS_TABLE
typedef struct
{
	u32 family;        /// Family of the address of the DNS server
	struct in_addr ip; /// IP of the DNS server
	u8 padding[12];    // matches the length required for IPv6 addresses
} SOCU_DNSTableEntry;


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

// this is supposed to be in unistd.h but newlib only puts it for cygwin, waiting for newlib patch from dkA
/**
 * @brief Gets the system's host ID.
 * @return The system's host ID.
 */
long gethostid(void);

// this is supposed to be in unistd.h but newlib only puts it for cygwin, waiting for newlib patch from dkA
int gethostname(char *name, size_t namelen);

int SOCU_ShutdownSockets(void);

int SOCU_CloseSockets(void);

/**
 * @brief Retrieves information from the network configuration. Similar to getsockopt().
 * @param level   Only value allowed seems to be @ref SOL_CONFIG
 * @param optname The option to be retrieved
 * @param optval  Will contain the output of the command
 * @param optlen  Size of the optval buffer, will be updated to hold the size of the output
 * @return 0 if successful. -1 if failed, and errno will be set accordingly. Can also return a system error code.
 */
int SOCU_GetNetworkOpt(int level, NetworkOpt optname, void * optval, socklen_t * optlen);

/**
 * @brief Gets the system's IP address, netmask, and subnet broadcast
 * @return error
 */
int SOCU_GetIPInfo(struct in_addr *ip, struct in_addr *netmask, struct in_addr *broadcast);

/**
 * @brief Adds a global socket.
 * @param sockfd   The socket fd.
 * @return error
 */
int SOCU_AddGlobalSocket(int sockfd);
