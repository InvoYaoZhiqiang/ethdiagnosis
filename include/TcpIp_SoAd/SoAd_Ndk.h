/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : soAdndk.h
*   Module Name     : 
*   Project         : Autosar
*   Processor       : 
*   Description		: AUTOSAR  NDK socket APIs
*   Component		: 
************************************************************************************************
*   Revision History:
*
*   Version     Date          Initials      CR#          Descriptions
*   ---------   ----------    -----------   ----------   ---------------
*   1.0         2018/05/08    YaoZhiqiang   N/A          Original
************************************************************************************************
*	END_FILE_HDR*/

#ifndef _SOAD_NDK_H_
#define _SOAD_NDK_H_

#ifdef __cplusplus
extern "C" {
#endif

/* AUTOSAR FOR SOCKET STANDARD */
 
 /* Socket Address Families */
 #define TCPIP_AF_INET 	AF_INET  /*IPv4*/

/* TCP/IP Domain Type */
typedef uint16 TcpIp_DomainType;

/* Protocol*/
typedef enum {
	TCPIP_IPPROTO_TCP = IPPROTO_TCP,
	TCPIP_IPPROTO_UDP = IPPROTO_UDP
} TcpIp_ProtocolType;

/* local IP address */
typedef uint8 TcpIp_LocalAddrIdType; /* 1 bytes */

/* SockAddr Structure*/
typedef struct {
	TcpIp_DomainType domain;
	sint8 data[14];
} TcpIp_SockAddrType; /* 16 byets */
	
/* IPv4 Address */
typedef struct in_addr  TcpIp_InetAddrType; /* 4 bytes IP address net order*/

/* INET SockAddr Type */
typedef struct {
	TcpIp_DomainType domain;
	uint16 port;
	TcpIp_InetAddrType addr;
	sint8 sin_zero[8];
} TcpIp_SockAddrInetType; /* 16 bytes */

/* A TcpIp stack socket */
typedef uint16 TcpIp_SocketIdType;

/* TCP/IP stack state for a specific EthIf controller */
typedef enum {
	TCPIP_STATE_ONLINE = 0,  /* ONLINE:communication via at least one IP address is possible */
	TCPIP_STATE_ONHOLD = 1,  /* ONHOLD:no communication is currently possible */
	TCPIP_STATE_OFFLINE = 2, /* OFFLINE:no communication is possible */
	TCPIP_STATE_STARTUP = 3, /* STARTUP:IP address assignment in progress or ready for manual start, */
						 	 /* communication is currently not possible. */
	TCPIP_STATE_SHUTDOWN = 4 /* SHUTDOWN:release of resources using the EthIf controller, release of IP address assignment. */
} TcpIp_StateType;

/* The state of local IP address assignment */
typedef enum {
	TCPIP_IPADDR_STATE_ASSIGNED = 0, /* local IP address is assigned */
	TCPIP_IPADDR_STATE_ONHOLD = 1,   /* local IP address is assigned, but cannot be used as the network is not active */
	TCPIP_IPADDR_STATE_UNASSIGNED = 2 /* local IP address is unassigned */
} TcpIp_IpAddrStateType;

/* Events reported by TcpIp. */
typedef enum {
	TCPIP_TCP_RESET = 0x01, /* TCP connection was reset, TCP socket and all related resources have been released. */
	TCPIP_TCP_CLOSED = 0x02, /* TCP connection was closed successfully, TCP socket and all related resources have been released. */
	TCPIP_TCP_FIN_RECEIVED = 0x03, /* A FIN signal was received on the TCP connection, TCP socket is still valid. */
	TCPIP_UDP_CLOSED = 0x04 /* UDP socket and all related resources have been released. */
} TcpIp_EventType;

/* Specification of IPv4 address assignment policy. */
typedef enum {
	TCPIP_IPADDR_ASSIGNMENT_STATIC = 0, /* Static configured IPv4 address. */
	TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP = 1, /* Linklocal IPv4 address assignment using DoIP parameters */
	TCPIP_IPADDR_ASSIGNMENT_DHCP = 2, /* Dynamic configured IPv4 address by DHCP. */
	TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL = 3 /* Linklocal IPv4 address assignment. */
} TcpIp_IpAddrAssignmentType;

/* TcpIp specific return type. */
typedef enum {
	TCPIP_OK = 0, /* operation completed successfully. */
	TCPIP_E_NOT_OK = 1, /* operation failed. */
	TCPIP_E_PHYS_ADDR_MISS = 2 /* operation failed because of an ARP ache miss. */
} TcpIp_ReturnType;

/* Type for the specification of all supported Parameter IDs. */
typedef uint8 TcpIp_ParamIdType;

/* Type for the specification of all supported Parameter IDs. */
#define TCPIP_PARAMID_TCP_RXWND_MAX 0x00 /* Specifies the maximum TCP receive window for the socket. */
#define TCPIP_PARAMID_FRAMEPRIO 0x01 /* Specifies the frame priority for outgoing frames on the socket. */
#define TCPIP_PARAMID_TCP_NAGLE 0x02 /* Specifies if the Nagle Algorithm according to IETF RFC 896 is enabled or not. */
#define TCPIP_PARAMID_TCP_KEEPALIVE 0x03/* Specifies if TCP Keep Alive Probes are sent on the socket connection. */
#define TCPIP_PARAMID_TTL 0x04 /* Specifies the time to live value for outgoing frames on the socket. */
#define TCPIP_PARAMID_TCP_KEEPALIVE_TIME 0x05 /* Specifies the time in [s] between the last data packet sent */
											  /* (simple ACKs are not considered data) and the first keepalive probe. */
#define TCPIP_PARAMID_TCP_KEEPALIVE_PROBES_MAX 0x06 /* Specifies the maximum number of times that a keepalive probe is retransmitted. */
#define TCPIP_PARAMID_TCP_KEEPALIVE_INTERVAL 0x07 /* Specifies the interval in [s] between subsequent keepalive probes. */
#define TCPIP_PARAMID_TCP_OPTIONFILTER 0x08 /* Specifies which TCP option filter shall be applied on the related socket. */
#define TCPIP_PARAMID_PATHMTU_ENABLE 0x09 /* Specifies if the Path MTU Discovery shall be performed on the related socket. */
#define TCPIP_PARAMID_FLOWLABEL 0x0a /* The 20-bit Flow Label according to IETF RFC 6437 */
#define TCPIP_PARAMID_DSCP 0x0b/* The 6-bit Differentiated Service Code Point according to IETF RFC 2474. */
#define TCPIP_PARAMID_UDP_CHECKSUM 0x0c /* Specifies if UDP checksum handling shall be enabled (TRUE) or skipped (FALSE) on the related socket. */
#define TCPIP_PARAMID_VENDOR_SPECIFIC 0x80 /* Start of vendor specific range of parameter IDs. */

/* IP address wildcard. */
typedef uint32 TcpIpIpAddrWildcardType;
#define TCPIP_IPADDR_ANY INADDR_ANY; /* 0.0.0.0 */
#define TCPIP_INADDR_BROADCAST INADDR_BROADCAST; /* 255.255.255.255 */

/* Port wildcard. */
typedef uint16 TcpIpPortWildcardType;

/* LocalAddrId wildcard. */
typedef TcpIp_LocalAddrIdType TcpIpLocalAddrIdWildcardType;

/* TcpIp_ArpCacheEntries elements type. */
typedef struct {
	TcpIp_InetAddrType InetAddr; /* IPv4 address in network byte order. */
	uint8 PhysAddr[6]; /* physical address in network byte orde. */
	uint8 State; /* state of the address entry. */
#define TCPIP_ARP_ENTRY_STATIC 0x77 /* The static ARP entry. */
#define TCPIP_ARP_ENTRY_VALID  0x88 /* The valid ARP entry. */
#define TCPIP_ARP_ENTRY_STALE  0x99 /* The stale(old) ARP entry. */
} TcpIp_ArpCacheEntryType;

/* Index to select specific measurement data. */
typedef uint8 TcpIp_MeasurementIdxType;
#define TCPIP_MEAS_DROP_TCP 0x01  /* Measurement index of dropped PDUs caused by invalid destination TCP-Port. */
#define TCPIP_MEAS_DROP_UDP 0x02  /* Measurement index of dropped PDUs caused by invalid destination UDP-Port. */
#define TCPIP_MEAS_DROP_IPV4 0x03 /* Measurement index of dropped datagrams caused by invalid IPv4 address. */
#define TCPIP_MEAS_ALL 0xFF 	  /* Represents all measurement indexes. */

/* Ethframe type */
typedef uint16 Eth_FrameType;

/* LocalAddrId range. */
#define DEF_STATIC_IPADDR_MAX   	3U 	/* Must be 3. */
#define DEF_STATIC_IPADDR_NUM 		DEF_STATIC_IPADDR_MAX/3U
#define DEF_DOIP_IPADDR_NUM   		2U*DEF_STATIC_IPADDR_NUM

#define DEF_IPADDR_MAX				20U
#define DEF_IPADDR_REMAIN           (DEF_IPADDR_MAX - DEF_STATIC_IPADDR_MAX)

#define DEF_SOCKET_MAX_NUM     		10U

/* The global Buffer size. */
#define GLOBAL_DATASRV_BUFSIZE  	3000U

/* TCPIP socket return type. */
#define TCPIP_SOCKETID_INVALID 		(void *)0xFFFFFFFF
#define TCPIP_SOCKETID_ERROR   		-1

#define NETWORK_CONNECT_SUCCESS  	0xFF

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif

