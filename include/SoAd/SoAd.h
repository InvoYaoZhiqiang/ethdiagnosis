/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : SoAd.h
*   Module Name     : 
*   Project         : 
*   Processor       : 
*   Description		: Autosar socket adapter inc
*   Component		: 
************************************************************************************************
*   Revision History:
*
*   Version     Date          Initials      CR#          Descriptions
*   ---------   ----------    -----------   ----------   ---------------
*   1.0         2018/05/29    YaoZhiqiang   N/A          Original
************************************************************************************************
*	END_FILE_HDR*/

#ifndef _SOAD_H_
#define _SOAD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* SoCon identifier type for unique identification of a SoAd socket connection. */
typedef  TcpIp_SocketIdType SoAd_SoConIdType;

/* type to specify the state of a SoAd socket connection. */
typedef enum {
	SOAD_SOCON_ONLINE = 0,
	SOAD_SOCON_RECONNECT = 1,
	SOAD_SOCON_OFFLINE = 2,
	SOAD_SOCON_READY = 3
} SoAd_SoConModeType;

/* RoutingGroup identifier type for unique identification of a SoAd routing group. */
typedef uint16 SoAd_RoutingGroupIdType;

/* Index to select specific measurement data. */
typedef uint8 SoAd_MeasurementIdxType;

#define SOAD_MEAS_DROP_TCP 0x01 /* Measurement index of dropped PDUs caused by invalid destination TCP-Port. */
#define SOAD_MEAS_DROP_UDP 0x02 /* Measurement index of dropped PDUs caused by invalid destination UDP-Port. */
#define SOAD_MEAS_ALL	   0xFF /* represents all measurement indexes. */

/*====================================Function definitions================================*/

_extern void SoAd_MainFunction(void);
_extern void SoAd_Init( void );
_extern void SoAd_DeInit( void );
_extern Std_ReturnType SoAd_IfTransmit( PduIdType TxPduId, const PduInfoType* PduInfoPtr );
_extern Std_ReturnType SoAd_IfRoutingGroupTransmit( SoAd_RoutingGroupIdType id );
_extern Std_ReturnType SoAd_IfSpecificRoutingGroupTransmit( SoAd_RoutingGroupIdType id, SoAd_SoConIdType SoConId );
_extern Std_ReturnType SoAd_TpTransmit( PduIdType TxPduId, const PduInfoType* PduInfoPtr );
_extern Std_ReturnType SoAd_TpCancelTransmit( PduIdType TxPduId );
_extern Std_ReturnType SoAd_TpCancelReceive( PduIdType RxPduId );
_extern Std_ReturnType SoAd_GetSoConId( PduIdType TxPduId, SoAd_SoConIdType* SoConIdPtr );
_extern Std_ReturnType SoAd_OpenSoCon( SoAd_SoConIdType SoConId );
_extern Std_ReturnType SoAd_CloseSoCon( SoAd_SoConIdType SoConId, BOOL abort );
_extern void SoAd_GetSoConMode( SoAd_SoConIdType SoConId, SoAd_SoConModeType* ModePtr );
_extern Std_ReturnType SoAd_RequestIpAddrAssignment( SoAd_SoConIdType SoConId, 
																TcpIp_IpAddrAssignmentType Type, 
																const TcpIp_SockAddrType* LocalIpAddrPtr, 
																uint8 Netmask, const TcpIp_SockAddrType* DefaultRouterPtr );
_extern Std_ReturnType SoAd_ReleaseIpAddrAssignment( SoAd_SoConIdType SoConId );
_extern Std_ReturnType SoAd_GetLocalAddr( SoAd_SoConIdType SoConId, TcpIp_SockAddrType* LocalAddrPtr, 
													uint8* NetmaskPtr, TcpIp_SockAddrType* DefaultRouterPtr );
_extern Std_ReturnType SoAd_GetPhysAddr( SoAd_SoConIdType SoConId, uint8* PhysAddrPtr );
_extern Std_ReturnType SoAd_GetRemoteAddr( SoAd_SoConIdType SoConId, TcpIp_SockAddrType* IpAddrPtr );
_extern Std_ReturnType SoAd_EnableRouting( SoAd_RoutingGroupIdType id );
_extern Std_ReturnType SoAd_EnableSpecificRouting( SoAd_RoutingGroupIdType id, SoAd_SoConIdType SoConId );
_extern Std_ReturnType SoAd_DisableRouting( SoAd_RoutingGroupIdType id );
_extern Std_ReturnType SoAd_DisableSpecificRouting( SoAd_RoutingGroupIdType id, SoAd_SoConIdType SoConId );
_extern Std_ReturnType SoAd_SetRemoteAddr( SoAd_SoConIdType SoConId, const TcpIp_SockAddrType* RemoteAddrPtr );
_extern Std_ReturnType SoAd_SetUniqueRemoteAddr( SoAd_SoConIdType SoConId, const TcpIp_SockAddrType* RemoteAddrPtr, SoAd_SoConIdType* AssignedSoConIdPtr );
_extern void SoAd_ReleaseRemoteAddr( SoAd_SoConIdType SoConId );
_extern Std_ReturnType SoAd_TpChangeParameter( PduIdType id, TPParameterType parameter, uint16 value );
_extern Std_ReturnType SoAd_ReadDhcpHostNameOption( SoAd_SoConIdType SoConId, uint8* length, uint8* data );
_extern Std_ReturnType SoAd_WriteDhcpHostNameOption( SoAd_SoConIdType SoConId, uint8 length, const uint8* data );
_extern Std_ReturnType SoAd_GetAndResetMeasurementData( SoAd_MeasurementIdxType MeasurementIdx, BOOL MeasurementResetNeeded, uint32* MeasurementDataPtr );

#ifdef __cplusplus
}
#endif /* extern "C" */

#endif
