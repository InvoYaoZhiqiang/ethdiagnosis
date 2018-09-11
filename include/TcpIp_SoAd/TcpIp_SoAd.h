/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : soAd.h
*   Module Name     : 
*   Project         : Autosar
*   Processor       : 
*   Description		: AUTOSAR NDK legacy socket adapter support
*   Component		: 
************************************************************************************************
*   Revision History:
*
*   Version     Date          Initials      CR#          Descriptions
*   ---------   ----------    -----------   ----------   ---------------
*   1.0         2018/05/18    YaoZhiqiang   N/A          Original
************************************************************************************************
*	END_FILE_HDR*/

#ifndef _TCPIP_SOAD_H_
#define _TCPIP_SOAD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* include NDK's socket API */
#include "SoAd_Ndk.h"

/* Core Communication Control. */
 _extern void TcpIp_Init( void );
 _extern void TcpIp_DeInit( void );
 _extern Std_ReturnType TcpIp_SoAdGetSocket( TcpIp_DomainType Domain, TcpIp_ProtocolType Protocol, TcpIp_SocketIdType *SocketIdPtr );
 _extern Std_ReturnType TcpIp_Bind( TcpIp_SocketIdType SocketId, TcpIp_LocalAddrIdType LocalAddrId, uint16* PortPtr );
 _extern Std_ReturnType TcpIp_TcpConnect( TcpIp_SocketIdType SocketId, const TcpIp_SockAddrType* RemoteAddrPtr );
 _extern Std_ReturnType TcpIp_TcpListen( TcpIp_SocketIdType SocketId, uint16 MaxChannels );
 _extern Std_ReturnType TcpIp_TcpAccept( TcpIp_SocketIdType SocketId, TcpIp_SocketIdType* SocketIdConnectPtr, const TcpIp_SockAddrType* RemoteAddrPtr );
 _extern Std_ReturnType TcpIp_TcpReceived( TcpIp_SocketIdType SocketId, uint8* DataPtr, uint32 Length );
 _extern Std_ReturnType TcpIp_RequestComMode( uint8 CtrlIdx, TcpIp_StateType State );
 _extern void TcpIp_Close(TcpIp_SocketIdType SocketId);
 _extern Std_ReturnType TcpIp_SetSockOpt(TcpIp_SocketIdType SocketId, sint32 Level, sint32 Option, void *pBuf, sint32 BufferSize);
 _extern Std_ReturnType TcpIp_GetSockOpt(TcpIp_SocketIdType SocketId, sint32 Level, sint32 Option, void *pBuf, sint32 *BufferPtr);

 /* Communication */
  _extern Std_ReturnType TcpIp_UdpTransmit( TcpIp_SocketIdType SocketId, const uint8* DataPtr, const TcpIp_SockAddrType* RemoteAddrPtr, uint16 TotalLength );
  _extern Std_ReturnType TcpIp_TcpTransmit( TcpIp_SocketIdType SocketId, const uint8* DataPtr, uint32 AvailableLength, const TcpIp_SockAddrType* RemoteAddrPtr, BOOL ForceRetrieve );
  _extern Std_ReturnType TcpIp_TcpReceive( TcpIp_SocketIdType SocketId, uint8* DataPtr, uint16 *LengthPtr );
  _extern Std_ReturnType TcpIp_Reception(	TcpIp_SocketIdType SocketId,	 uint8* DataPtr, const TcpIp_SockAddrType* RemoteAddrPtr, uint16 *LengthPtr ); 
  
/* Extended Communication Control and Information. */
 _extern Std_ReturnType TcpIp_RequestIpAddrAssignment( TcpIp_LocalAddrIdType LocalAddrId, TcpIp_IpAddrAssignmentType Type, const TcpIp_SockAddrType* LocalIpAddrPtr, uint8 Netmask, const TcpIp_SockAddrType* DefaultRouterPtr );
 _extern Std_ReturnType TcpIp_ReleaseIpAddrAssignment( TcpIp_LocalAddrIdType LocalAddrId );
 _extern Std_ReturnType TcpIp_ResetIpAssignment( void );
 _extern Std_ReturnType TcpIp_IcmpTransmit(TcpIp_LocalAddrIdType LocalIpAddrId, const TcpIp_SockAddrType* RemoteAddrPtr, uint8 Ttl, uint8 Type, uint8 Code, uint16 DataLength, const uint8* DataPtr );
 _extern Std_ReturnType TcpIp_DhcpReadOption( TcpIp_LocalAddrIdType LocalIpAddrId, uint8 Option, uint8* DataLength, uint8* DataPtr );
 _extern Std_ReturnType TcpIp_DhcpWriteOption( TcpIp_LocalAddrIdType LocalIpAddrId, uint8 Option, uint8 DataLength, const uint8* DataPtr );
 _extern Std_ReturnType TcpIp_ChangeParameter( TcpIp_SocketIdType SocketId, TcpIp_ParamIdType ParameterId, const uint8* ParameterValue );
 _extern Std_ReturnType TcpIp_GetIpAddr( TcpIp_LocalAddrIdType LocalAddrId, TcpIp_SockAddrType* IpAddrPtr, uint8* NetmaskPtr, TcpIp_SockAddrType* DefaultRouterPtr );
 _extern Std_ReturnType TcpIp_GetPhysAddr( TcpIp_SocketIdType SocketId, uint8* PhysAddrPtr );
 _extern TcpIp_ReturnType TcpIp_GetRemotePhysAddr( uint8 CtrlIdx, const TcpIp_SockAddrType* IpAddrPtr, uint8* PhysAddrPtr, BOOL initRes );
 _extern Std_ReturnType TcpIp_GetCtrlIdx( TcpIp_SocketIdType SocketId, uint8* CtrlIdxPtr );
 _extern Std_ReturnType TcpIp_GetArpCacheEntries( uint8 ctrlIdx, uint32* numberOfElements, TcpIp_ArpCacheEntryType* entryListPtr );
 _extern Std_ReturnType TcpIp_GetAndResetMeasurementData( TcpIp_MeasurementIdxType MeasurementIdx, BOOL MeasurementResetNeeded, uint32* MeasurementDataPtr );

/* Call-back notifications. */
 _extern void TcpIp_RxIndication( uint8 CtrlIdx, Eth_FrameType FrameType, BOOL IsBroadcast, const uint8* PhysAddrPtr, uint8* DataPtr, uint16 LenByte );

/* Up layer Configurable interfaces. */
 _extern void SoAd_RxIndication( TcpIp_SocketIdType SocketId, const TcpIp_SockAddrType* RemoteAddrPtr, uint8* BufPtr, uint16 Length );
 _extern void SoAd_TcpIpEvent( TcpIp_SocketIdType SocketId, TcpIp_EventType Event );
 _extern void SoAd_TxConfirmation( TcpIp_SocketIdType SocketId, uint16 Length );
 _extern Std_ReturnType SoAd_TcpConnected( TcpIp_SocketIdType SocketId , const TcpIp_SockAddrType* RemoteAddrPtr);
 _extern void SoAd_LocalIpAddrAssignmentChg( TcpIp_LocalAddrIdType IpAddrId, TcpIp_IpAddrStateType State );
 _extern void SoAd_IcmpMsgHandler( TcpIp_LocalAddrIdType LocalAddrId, const TcpIp_SockAddrType* RemoteAddrPtr, uint8 Ttl, uint8 Type, uint8 Code, uint16 DataLength, uint8* DataPtr );

#ifdef __cplusplus
 }
#endif /* _extern "C" */

#endif
