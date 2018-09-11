/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : TcpIp_SoAd.c
*   Module Name     : 
*   Project         : Autosar
*   Processor       : 
*   Description		: AUTOSAR NDK legacy basic socket structure support
*   Component		: 
************************************************************************************************
*   Revision History:
*
*   Version     Date          Initials      CR#          Descriptions
*   ---------   ----------    -----------   ----------   ---------------
*   1.0         2018/05/21    YaoZhiqiang   N/A          Original
************************************************************************************************
*	END_FILE_HDR*/

/*---------------------------------------------------------------------------*\
|                                Header Files                                 |
\*---------------------------------------------------------------------------*/

#include <src/eth_diagnosis/include/Network_DoIP_SoAd_Sys.h>

/* NDK Dependencies */
#include <ti/ndk/inc/netmain.h> 

/* Utils dynamic heap memory. */
#include <src/utils_common/include/utils_mem_if.h> 
//#include <ti/drv/vps/include/vps_dataTypes.h> //for 16 macro

/*---------------------------------------------------------------------------*\
|                        Global extern Declarations                    		  |
\*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*\
|                  Local Macros/Defines/Typedefs/Enums/Variable               |
\*---------------------------------------------------------------------------*/

/* Global lock. */
#define UnLocked 	FALSE
#define Locked  	TRUE

/* The default timeout  for accept client. */
#define DEF_TIMEOUT	 2000

/* The Global enbale block mode or not .*/
#define ENABLE_BLOCK   1

/*The global enable no copy API. */
#define ENABLE_NOCOPY  0

/* ============= Map the LocalAddr. ================*/

#define DEF_TCPIP_IPADDR_ASSIGNMENT_STATIC 			"192.168.1.201"
#define DEF_TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP 	"192.168.1.202"
#define DEF_TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL		"192.168.1.203"

#define TCPIP_STATIC_IPADDR 	inet_addr(DEF_TCPIP_IPADDR_ASSIGNMENT_STATIC)
#define TCPIP_DOIP_IPADDR 		inet_addr(DEF_TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP)
#define TCPIP_LINKLOCAL_IPADDR  inet_addr(DEF_TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL)

/* Define the global LocalAddr object. */
static TcpIp_SockAddrInetType LocalAddr[DEF_IPADDR_MAX];

/* Global LocalAddr lock. */
static BOOL LocalAddr_Init_Lock = UnLocked;

static const sint8 *LocalIpAddrLeft[DEF_IPADDR_REMAIN] = {
	"192.168.1.205", "192.168.1.206", "192.168.1.207", \
	"192.168.1.208", "192.168.1.209", "192.168.1.210", \
	"192.168.1.211", "192.168.1.212", "192.168.1.213", \
	"192.168.1.214", "192.168.1.215", "192.168.1.216", \
	"192.168.1.217", "192.168.1.218", "192.168.1.219", \
	"192.168.1.220", "192.168.1.221" };

/* ===============Map the SocketId.================== */

#define DEF_SOCKET_FIRST      	 1

/* The global SocketId status. */
static BOOL Local_SocketId_Status = UnLocked;

static TcpIp_SocketIdType *Local_SocketId[DEF_IPADDR_MAX];
static TcpIp_SocketIdType Local_Fd_Cnt = DEF_SOCKET_FIRST;

/* The default port for ethernet diagnosis. */
#define ETH_DIAGNOSIS_PORT 		13400

/* Information related to network socket */
typedef struct {

    SOCKET sockFd;
    /**< Server socket handle */

    uint16 port;
    /**< port on which server is listening */

    SOCKET connectedSockFd;
    /**< socket handle of client that is connected */

    FDPOLLITEM pollitem[1U];
    /**< Polling structure to use with fdPoll() */
	
} TcpIp_Network_SockObj;

/* ==================Global buffer. ===================*/

/* The global TxBuffer. */
static uint8 *TxBuffer;
static uint8 *TempTxBuffer = NULL;
static uint16 TxBufferLength = 0;

/* The global TxBuffer Lock. */
static BOOL Local_TxBuffer_Lock = UnLocked;

/* The global RxBuffer. */
static uint8 *RxBuffer;
static uint8 *TempRxBuffer = NULL;
static uint16 RxBufferLength = 0;

/* The global RxBuffer lock. */
static BOOL Local_RxBuffer_Lock = UnLocked;

/*---------------------------------------------------------------------------*\
|                         Local Function Declarations                         |
\*---------------------------------------------------------------------------*/

static Std_ReturnType TcpIp_static_socket_init(void);
static Std_ReturnType TcpIp_Network_WaitConnect(TcpIp_SocketIdType SocketId, \
														TcpIp_Network_SockObj *pObj, \
														const TcpIp_SockAddrType* RemoteAddrPtr, \
														uint32 timeout);
static void TcpIp_static_ipaddr_init(void);

static void TcpIp_static_tx_buffer_init(void);
static void TcpIp_static_rx_buffer_init(void);

static BufReq_ReturnType SoAd_CopyRxData(uint8* BufPtr, uint16 BufLength);
static BufReq_ReturnType SoAd_CopyTxData(uint8* BufPtr, uint16 BufLength );

/*---------------------------------------------------------------------------*\
|                          Global Function Definitions                        |
\*---------------------------------------------------------------------------*/

 /**********************  Core Communication Control *******************/

void TcpIp_Init(void )
{
	PR_DEBUG(TRUE, "TcpIp_Init start !!!\n");

	/* The Local_SocketId mapping. */
	if(TcpIp_static_socket_init() == E_OK)
		PR_DEBUG(DEBUG_SWITCH, "Local_SocketId_Status is Locked!\n");

	/* Init the LocalAddr. */
	TcpIp_static_ipaddr_init();

	/* Init the TxBuffer. */
	TcpIp_static_tx_buffer_init();
	
	/* Init the RxBuffer. */
	TcpIp_static_rx_buffer_init();
	
	PR_DEBUG(TRUE, "TcpIp_Init done !!!\n");
}

void TcpIp_DeInit(void)
{
	uint8 index = 0;

	/* DeInit the SoAd. */
	for(index = 0; index < DEF_SOCKET_MAX_NUM; index++)
	{
		if(Local_SocketId[index]) {
			Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR, 
					Local_SocketId[index], 
					sizeof(TcpIp_SocketIdType *)*DEF_SOCKET_MAX_NUM);
			Local_SocketId[index] = NULL;
		}
	}

	if(Local_SocketId_Status)
		Local_SocketId_Status = UnLocked;
	if(Local_Fd_Cnt)
		Local_Fd_Cnt = DEF_SOCKET_FIRST;

	/* DeInit the LocalAddr. */
	for(index = 0; index < DEF_IPADDR_MAX; index++)
		bzero(&LocalAddr[index], sizeof(TcpIp_SockAddrInetType));
	if(LocalAddr_Init_Lock)
		LocalAddr_Init_Lock = UnLocked;

	/* DeInit the TxBuffer. */
	if(TxBuffer) {
		Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR, 
					TxBuffer, 
					GLOBAL_DATASRV_BUFSIZE);
		TxBuffer = NULL;
	}	
	if(TempTxBuffer) 
		TempTxBuffer = NULL;

	if(Local_TxBuffer_Lock)
		Local_TxBuffer_Lock = UnLocked;

	/* DeInit the RxBuffer. */
	if(RxBuffer) {
		Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR, 
					RxBuffer, 
					GLOBAL_DATASRV_BUFSIZE);
		RxBuffer = NULL;
	}	
	if(TempRxBuffer) 
		TempRxBuffer = NULL;

	if(Local_RxBuffer_Lock)
		Local_RxBuffer_Lock = UnLocked;
}

Std_ReturnType TcpIp_SoAdGetSocket 
(
 TcpIp_DomainType Domain, 
 TcpIp_ProtocolType Protocol,
 TcpIp_SocketIdType* SocketIdPtr 
) 
{
	SOCKET s = TCPIP_SOCKETID_INVALID;
	uint32 type = 0;

	if(Local_Fd_Cnt >= DEF_SOCKET_MAX_NUM) {
		PR_DEBUG(TRUE, "%s->SocketId assigned reached full !!!\n", __func__);
		return E_NOT_OK;
	}

	/* Check the Domain valid. */
	if((Domain == TCPIP_AF_INET) && (Protocol == TCPIP_IPPROTO_TCP))
	#if ENABLE_NOCOPY
		type = SOCK_STREAMNC;
	#else
		type = SOCK_STREAM;
	#endif
	else if((Domain == TCPIP_AF_INET) && (Protocol == TCPIP_IPPROTO_UDP)) 
		type = SOCK_DGRAM;
	else 
		return E_NOT_OK;

	if((s = socket(Domain, type, Protocol)) == TCPIP_SOCKETID_INVALID)
	{
		PR_DEBUG(TRUE, "TcpIp_SoAdGetSocket errno:%d\n", fdError());
		return E_NOT_OK;
	}

	/* Fill the pointer array. */
	Local_SocketId[Local_Fd_Cnt - 1] = (TcpIp_SocketIdType *)s;

	/* Return the Local_Fd_Cnt. */
	*SocketIdPtr = Local_Fd_Cnt;

	/* Increase the fd num. */
	Local_Fd_Cnt++;

	if(Local_Fd_Cnt == DEF_SOCKET_MAX_NUM) {
		PR_DEBUG(TRUE, "%s->SocketId assigned reached full !!!\n", __func__);
		return E_NOT_OK;
	}

	if(!SocketIdPtr)
		return E_NOT_OK;

	return E_OK;
}

Std_ReturnType TcpIp_Bind
(
 TcpIp_SocketIdType SocketId, 
 TcpIp_LocalAddrIdType LocalAddrId,
 uint16* PortPtr 
)
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	uint32 len = 0;
	PSA	   pName;
	
	/* Check the PortPtr. */
	if(!PortPtr)
		return E_NOT_OK;

	LocalAddr[LocalAddrId].port = *PortPtr;
    
	#if 0
	pName = (PSA)&LocalAddr[LocalAddrId];
	len = sizeof(pName);
	Vps_printf("sizeof(SA_IN)=%d,len=%d\n",sizeof(SA_IN),len);
    if(bind(s, pName, len) == TCPIP_SOCKETID_ERROR)
	#else
	//Vps_printf("sizeof(SA_IN)=%d,sizeof(LocalAddr[LocalAddrId])=%d\n",sizeof(SA_IN),sizeof(LocalAddr[LocalAddrId]));
    if(bind(s, (struct sockaddr *)&LocalAddr[LocalAddrId], sizeof(LocalAddr[LocalAddrId])) == TCPIP_SOCKETID_ERROR) 
    #endif
	{
		PR_DEBUG(TRUE, "TcpIp_Bind errno:%d\n", fdError());
		return E_NOT_OK;
	}
		
	return E_OK;	
}

Std_ReturnType TcpIp_TcpConnect
(
 TcpIp_SocketIdType SocketId, 
 const TcpIp_SockAddrType* RemoteAddrPtr
) 
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	PSA pName;
	uint32 len = 0;
    #if 0
	pName = (PSA)RemoteAddrPtr;
	len = sizeof(*pName);
	if(connect(s, pName, len) == TCPIP_SOCKETID_ERROR)
	#else
	if(connect(s, (struct sockaddr *)RemoteAddrPtr, sizeof(*RemoteAddrPtr)) == TCPIP_SOCKETID_ERROR)
	#endif
	{	
		PR_DEBUG(TRUE, "TcpIp_TcpConnect:%d\n", fdError());
		return E_NOT_OK;
	}

	return E_OK;	
}

Std_ReturnType TcpIp_TcpListen
(
 TcpIp_SocketIdType SocketId,
 uint16 MaxChannels 
) 
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	if(listen(s, MaxChannels) == TCPIP_SOCKETID_ERROR)
	{
		PR_DEBUG(TRUE, "TcpIp_TcpListen errno:%d\n", fdError());
		return E_NOT_OK;
	}

	return E_OK;
}

Std_ReturnType TcpIp_TcpAccept
(
 TcpIp_SocketIdType SocketId,
 TcpIp_SocketIdType* SocketIdConnectPtr, 
 const TcpIp_SockAddrType* RemoteAddrPtr 
)
{
	uint8 Status = E_NOT_OK;
	TcpIp_Network_SockObj Network_SockObj;
	SOCKET acceptPtr = TCPIP_SOCKETID_INVALID;

	Network_SockObj.port = ETH_DIAGNOSIS_PORT;
	
	if(Local_Fd_Cnt >= DEF_SOCKET_MAX_NUM) {
		PR_DEBUG(TRUE, "%s->AcceptId assigned reached full!\n", __func__);
		return E_NOT_OK;
	}

	Status = TcpIp_Network_WaitConnect(SocketId, &Network_SockObj, RemoteAddrPtr, DEF_TIMEOUT);
	
	if(Status == NETWORK_CONNECT_SUCCESS)
	{
		acceptPtr = Network_SockObj.connectedSockFd;
	
		/* Value the SocketIdConnected. */
		Local_SocketId[Local_Fd_Cnt - 1] = (TcpIp_SocketIdType *)acceptPtr;
		*SocketIdConnectPtr = Local_Fd_Cnt;

		/* Increase the Local_Fd_Cnt. */
		Local_Fd_Cnt++;

		if(Local_Fd_Cnt == DEF_SOCKET_MAX_NUM) {
			PR_DEBUG(TRUE, "%s->AcceptId assigned reached full!\n", __func__);
			return E_NOT_OK;
		}

		if(!SocketIdConnectPtr)
			return E_NOT_OK;
	}
	return Status;		
}

Std_ReturnType TcpIp_TcpReceived
(
 TcpIp_SocketIdType SocketId, 
 uint8* DataPtr,
 uint32 Length
) 
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	
#if ENABLE_BLOCK	
	uint32 flags = MSG_WAITALL;
#else
	uint32 flags = MSG_DONTWAIT;
#endif

	sint32 nbytes = 0;

	nbytes = recv(s, (void *)DataPtr, Length, flags);	
	if(nbytes < 0)
	{
		PR_DEBUG(TRUE, "TcpIp_TcpReceived errno:%d\n", fdError());
		return E_NOT_OK;
	}
	
	/* Return the received bytes. */
	Length = nbytes;

	return E_OK;		
}

Std_ReturnType TcpIp_TcpReceive
(
 TcpIp_SocketIdType SocketId, 
 uint8* DataPtr,
 uint16 *LengthPtr 
) 
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	
#if ENABLE_BLOCK	
	uint32 flags = MSG_WAITALL;
#else
	uint32 flags = MSG_DONTWAIT;
#endif

#if ENABLE_NOCOPY
	HANDLE hBuffer;
#endif

	sint32 nbytes = 0;

	for(;;)
	{
	#if ENABLE_NOCOPY
		nbytes = recvnc(s, (void **)&DataPtr, flags, hBuffer);
	#else
		nbytes = recv(s, (void *)DataPtr, *LengthPtr, flags);
	#endif
	
		if(nbytes < 0)
		{
			PR_DEBUG(DEBUG_SWITCH, "TcpIp_TcpReceive errno:%d\n", fdError());
			return E_NOT_OK;
		}
		
	#if ENABLE_NOCOPY
		recvncfree(hBuffer);
	#endif

		*LengthPtr = nbytes;
		break;
	}
	
	PR_DEBUG(DEBUG_SWITCH, "nbytes = %u\n", nbytes);
	
	/* Copy for database. */
	if(RxBufferLength < GLOBAL_DATASRV_BUFSIZE)
	{
		if(SoAd_CopyRxData(DataPtr, *LengthPtr) == BUFREQ_E_NOT_OK)
				return E_NOT_OK;
	}
	else {
		/* Flush the RxBuffer after full. */
		bzero(RxBuffer, GLOBAL_DATASRV_BUFSIZE);
		TempRxBuffer = RxBuffer;
		RxBufferLength = 0;
	}
	return E_OK;
}

Std_ReturnType TcpIp_Reception
(
 TcpIp_SocketIdType SocketId, 
 uint8* DataPtr,
 const TcpIp_SockAddrType* RemoteAddrPtr,
 uint16 *LengthPtr 
) 
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	
#if ENABLE_BLOCK	
	uint32 flags = MSG_WAITALL;
#else
	uint32 flags = MSG_DONTWAIT;
#endif

#if ENABLE_NOCOPY
	HANDLE hBuffer;
#endif

	PSA pName = (PSA)RemoteAddrPtr;
	sint32 nbytes = 0;
	sint32 len = sizeof(SA);

	for(;;)
	{
	#if ENABLE_NOCOPY
		nbytes = recvncfrom(s, (void **)&DataPtr, flags, pName, &len, hBuffer);
	#else
		nbytes = recvfrom(s, (void *)DataPtr, *LengthPtr, flags, pName, &len);
	#endif
	
		if(nbytes < 0)
		{
			PR_DEBUG(DEBUG_SWITCH, "TcpIp_Reception errno:%d\n", fdError());
			return E_NOT_OK;
		}
		
	#if ENABLE_NOCOPY
		recvncfree(hBuffer);
	#endif
	
		*LengthPtr = nbytes;
	
		/* Copy the client remoteaddr to SoAd layer. */
		memcpy((void *)RemoteAddrPtr, (void *)pName, len);
		break;
	}

	/* Copy for database. */
	if(RxBufferLength < GLOBAL_DATASRV_BUFSIZE)
	{
		if(SoAd_CopyRxData(DataPtr, *LengthPtr) == BUFREQ_E_NOT_OK)
				return E_NOT_OK;
	}
	else {
		/* Flush the RxBuffer after full. */
		bzero(RxBuffer, GLOBAL_DATASRV_BUFSIZE);
		TempRxBuffer = RxBuffer;
		RxBufferLength = 0;
	}	
	return E_OK;
}

void TcpIp_Close(TcpIp_SocketIdType SocketId)	{

	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];

	if(s != TCPIP_SOCKETID_INVALID) {
		fdClose(s);
		s = (SOCKET)TCPIP_SOCKETID_INVALID;
		Local_SocketId[SocketId - 1] = NULL;
	}
}

Std_ReturnType TcpIp_SetSockOpt
(
 TcpIp_SocketIdType SocketId, 
 sint32 Level, sint32 Option, 
 void *pBuf, sint32 BufferSize
)
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	if(setsockopt(s, Level, Option, pBuf, BufferSize) < 0)
	{
		PR_DEBUG(TRUE, "TcpIp_SetSockOpt errno:%d\n", fdError());
		return E_NOT_OK;
	}
	return E_OK;
}

Std_ReturnType TcpIp_GetSockOpt
(
 TcpIp_SocketIdType SocketId, 
 sint32 Level, sint32 Option, 
 void *pBuf, sint32 *BufferPtr
)
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	if(getsockopt(s, Level, Option, pBuf, BufferPtr) < 0)
	{
		PR_DEBUG(TRUE, "TcpIp_GetSockOpt errno:%d\n", fdError());
		return E_NOT_OK;
	}
	return E_OK;
}

Std_ReturnType TcpIp_RequestComMode
(
 uint8 CtrlIdx, 
 TcpIp_StateType State 
 )
{
	return E_OK;
}

/* Transmission */
Std_ReturnType TcpIp_UdpTransmit
(
 TcpIp_SocketIdType SocketId, 
 const uint8* DataPtr, 
 const TcpIp_SockAddrType* RemoteAddrPtr, 
 uint16 TotalLength 
)
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	
#if ENABLE_BLOCK	
	uint32 flags = MSG_WAITALL;
#else
	uint32 flags = MSG_DONTWAIT;
#endif

	PSA pName;
	sint32 retVal = 0;
	uint32 len = 0;

	pName = (PSA)RemoteAddrPtr;
	len = sizeof(*pName);
	
	if(!DataPtr) {
		PR_DEBUG(TRUE, "%s : %u DataPtr is NULL !!!\n", \
												__func__, __LINE__);
		return E_NOT_OK;
	}

	if(TotalLength > GLOBAL_DATASRV_BUFSIZE) {
		PR_DEBUG(TRUE, "%s: %u TotalLength is larger than GLOBAL_DATASRV_BUFSIZE !!!\n", \
											   __func__, __LINE__);
		return E_NOT_OK;
	}
	
	retVal = sendto(s, (void *)DataPtr, (uint32)TotalLength, flags, pName, len);	
	if(retVal < 0)
	{
		PR_DEBUG(TRUE, "TcpIp_UdpTransmit errno:%d\n", fdError());
		return E_NOT_OK;
	}
	return E_OK;	
}

Std_ReturnType TcpIp_TcpTransmit
(
 TcpIp_SocketIdType SocketId, 
 const uint8* DataPtr, 
 uint32 AvailableLength,
 const TcpIp_SockAddrType* RemoteAddrPtr,
 BOOL ForceRetrieve 
)
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	uint8* TempDataPtr = (uint8 *)DataPtr;
	
#if ENABLE_BLOCK	
	uint32 flags = MSG_WAITALL;
#else
	uint32 flags = MSG_DONTWAIT;
#endif

	sint32 retVal = 0;
	(void)RemoteAddrPtr;

	if(!DataPtr) {
		PR_DEBUG(TRUE, "%s : %u DataPtr is NULL !!!\n", \
												__func__, __LINE__);
		return E_NOT_OK;
	}

	if(!AvailableLength) {
		PR_DEBUG(TRUE, "%s: %u AvailableLength is zero !!!\n", \
											   __func__, __LINE__);
		return E_NOT_OK;
	}
	
	if(ForceRetrieve) 
	{
		for(;;)
		{
			bzero(TxBuffer, GLOBAL_DATASRV_BUFSIZE);
			TempTxBuffer = TxBuffer;
			TxBufferLength = 0;

			if(TempDataPtr && AvailableLength)
			{
				if(SoAd_CopyTxData((uint8 *)TempDataPtr, GLOBAL_DATASRV_BUFSIZE) == BUFREQ_E_NOT_OK)
					return E_NOT_OK;
				else {
					retVal = send(s, TxBuffer, GLOBAL_DATASRV_BUFSIZE, flags);
					if(retVal < 0)
					{
						PR_DEBUG(TRUE, "LINE %u: TcpIp_TcpTransmit errno:%d\n", __LINE__, fdError());
						continue;			
					}
				}
				
				if( (AvailableLength - GLOBAL_DATASRV_BUFSIZE) > 0) {
					TempDataPtr += GLOBAL_DATASRV_BUFSIZE;
					AvailableLength -= GLOBAL_DATASRV_BUFSIZE;
					continue;
				} 
				else {
					retVal = send(s, (void *)TempDataPtr, AvailableLength, flags);
					if(retVal < 0)
					{
						PR_DEBUG(TRUE, "LINE %u: TcpIp_TcpTransmit errno:%d\n", __LINE__, fdError());
						return E_NOT_OK;			
					}
					AvailableLength = 0;
					break;
				}
			}					
		}
	}
	else {
		retVal = send(s, (void *)DataPtr, AvailableLength, flags);
		if(retVal < 0)
		{
			PR_DEBUG(TRUE, "TcpIp_TcpTransmit errno:%d\n", fdError());
			return E_NOT_OK;			
		}
	}
	return E_OK;
}

/**********************************************************************
 *************************  Configurable interfaces *******************
 **********************************************************************/

/* RxIndication interfaces. */
void SoAd_RxIndication
(
 TcpIp_SocketIdType SocketId,
 const TcpIp_SockAddrType* RemoteAddrPtr, 
 uint8* BufPtr, 
 uint16 Length 
)
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	PSA pName = (PSA)RemoteAddrPtr;
	sint32 nbytes = 0;
	
#if ENABLE_BLOCK	
	uint32 flags = MSG_WAITALL;
#else
	uint32 flags = MSG_DONTWAIT;
#endif

	sint32 len = sizeof(SA);

	nbytes = recvfrom(s, (void *)BufPtr, Length, flags, pName, &len);
	if(nbytes < 0)
	{	
		PR_DEBUG(DEBUG_SWITCH, "SoAd_RxIndication errno:%d\n", fdError());
		return;
	}

	/* Copy the client remoteaddr to SoAd layer. */
	memcpy((void *)RemoteAddrPtr, (void *)pName, len);
	
	/* Return the received bytes. */
	Length = nbytes;
}

void SoAd_TcpIpEvent
(
 TcpIp_SocketIdType SocketId, 
 TcpIp_EventType Event
)
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	FILEDESC *pfd = (FILEDESC *)s;

	llEnter();

	for(;;)
	{
		if(FdWaitEvent(pfd, FD_EVENT_READ | FD_EVENT_WRITE | FD_EVENT_EXCEPT, 0)) {
			Event = TCPIP_TCP_CLOSED;
			break;
		}
		else if(FdWaitEvent(pfd, FD_EVENT_READ | FD_EVENT_EXCEPT, 0)) {
			Event = TCPIP_TCP_FIN_RECEIVED;
			break;
		}
		else if(FdWaitEvent(pfd, FD_EVENT_INVALID, 0)) {
			Event = TCPIP_TCP_RESET;
			break;
		}
		else 
			continue;
	}

	if(Event < TCPIP_TCP_RESET || Event > TCPIP_UDP_CLOSED) {
		llExit();
		return;
	}

	llExit();
}

void SoAd_TxConfirmation
(
 TcpIp_SocketIdType SocketId,
 uint16 Length 
)
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	uint8 *BufPtr = NULL;
	sint32 nbytes = 0;

	if(TcpIp_TcpReceived(SocketId, BufPtr, (uint32)Length) == E_OK) {
		nbytes = send(s, BufPtr, sizeof(BufPtr), MSG_DONTWAIT);
		if(nbytes < 0)
		{
			PR_DEBUG(TRUE, "SoAd_TxConfirmation errno:%d\n", fdError());
			return;
		}
	}
	else
		return;		
}

Std_ReturnType SoAd_TcpConnected
(
 TcpIp_SocketIdType SocketId, 
 const TcpIp_SockAddrType* RemoteAddrPtr
)
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	SA Name;
	sint32 len = sizeof(SA);

	bzero(&Name, sizeof(SA));
	if(getpeername(s, (PSA)&Name, &len) < 0)
	{	
		PR_DEBUG(TRUE, "SoAd_TcpConnected errno:%d\n", fdError());
		return E_NOT_OK;
	}
	
#if ENABLE_DEBUG
	{	
		/* Get the client ipaddr. */
		char String[20] = "";
		TcpIp_SockAddrInetType *IPADDR = (TcpIp_SockAddrInetType *)&Name;
		IPN NetIpaddr = (IPN)IPADDR->addr.s_addr;
	
		NetIpaddr = htonl(NetIpaddr);
		snprintf(String, sizeof(String), "%d.%d.%d.%d",
				(UINT8)((NetIpaddr>>24)&0xFF), (UINT8)((NetIpaddr>>16)&0xFF),
				(UINT8)((NetIpaddr>>8)&0xFF), (UINT8)(NetIpaddr&0xFF)); 		
		PR_DEBUG(TRUE, "SoAd_TcpConnected succeded to accept client:%s !!!\n", String);
	}
#endif

	/* Value the RemoteAddrPtr. */
	memcpy((void *)RemoteAddrPtr, (void *)&Name, sizeof(TcpIp_SockAddrType));

	if(!RemoteAddrPtr)
		return E_NOT_OK;

	return E_OK;
}

void SoAd_LocalIpAddrAssignmentChg
(
 TcpIp_LocalAddrIdType IpAddrId, 
 TcpIp_IpAddrStateType State 
)
{
	if(IpAddrId < DEF_IPADDR_MAX)
		State = TCPIP_IPADDR_STATE_ASSIGNED;
	else
		State = TCPIP_IPADDR_STATE_UNASSIGNED;

	if(State > TCPIP_IPADDR_STATE_UNASSIGNED)
		return;
}

void SoAd_IcmpMsgHandler
(	
 TcpIp_LocalAddrIdType LocalAddrId,
 const TcpIp_SockAddrType* RemoteAddrPtr, 
 uint8 Ttl, uint8 Type, uint8 Code, 
 uint16 DataLength, uint8* DataPtr 
)
{
	return;
}

/********* Extended Communication Control and Information *************/

Std_ReturnType TcpIp_RequestIpAddrAssignment
(
 TcpIp_LocalAddrIdType LocalAddrId,
 TcpIp_IpAddrAssignmentType Type, 
 const TcpIp_SockAddrType* LocalIpAddrPtr, uint8 Netmask, 
 const TcpIp_SockAddrType* DefaultRouterPtr
)
{
	if(Type == TCPIP_IPADDR_ASSIGNMENT_STATIC) {
		if(LocalAddrId < DEF_STATIC_IPADDR_NUM)
			memcpy((void *)LocalIpAddrPtr, (void *)&LocalAddr[LocalAddrId], sizeof(TcpIp_SockAddrType));
		else
			return E_NOT_OK;
	}
	else if(Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP) {
		if(LocalAddrId >= DEF_STATIC_IPADDR_NUM && LocalAddrId < DEF_DOIP_IPADDR_NUM)
			memcpy((void *)LocalIpAddrPtr, (void *)&LocalAddr[LocalAddrId], sizeof(TcpIp_SockAddrType));
		else
			return E_NOT_OK;
	}
	else if(Type == TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL) {
		if(LocalAddrId >= DEF_DOIP_IPADDR_NUM && LocalAddrId < DEF_STATIC_IPADDR_MAX)
			memcpy((void *)LocalIpAddrPtr, (void *)&LocalAddr[LocalAddrId], sizeof(TcpIp_SockAddrType));
		else
			return E_NOT_OK;
	}
	else
		return E_NOT_OK;

	(void)Netmask;
	(void)DefaultRouterPtr;

	if(!LocalIpAddrPtr)
		return E_NOT_OK;

	return E_OK;
}

Std_ReturnType TcpIp_ReleaseIpAddrAssignment(TcpIp_LocalAddrIdType LocalAddrId )
{
	if(LocalAddrId < DEF_IPADDR_MAX) {
		bzero(&LocalAddr[LocalAddrId], sizeof(TcpIp_SockAddrInetType));
		if(LocalAddr_Init_Lock)
			LocalAddr_Init_Lock = UnLocked;
	}
	else
		return E_NOT_OK;

	return E_OK;
}

Std_ReturnType TcpIp_ResetIpAssignment(void ) 
{	
	if(LocalAddr_Init_Lock)
		LocalAddr_Init_Lock = UnLocked;
	TcpIp_static_ipaddr_init();
	return E_OK;
}

Std_ReturnType TcpIp_IcmpTransmit
(
 TcpIp_LocalAddrIdType LocalIpAddrId,
 const TcpIp_SockAddrType* RemoteAddrPtr, 
 uint8 Ttl, uint8 Type, uint8 Code, 
 uint16 DataLength, const uint8* DataPtr
)
{
	return E_OK;
}

Std_ReturnType TcpIp_DhcpReadOption
(
 TcpIp_LocalAddrIdType LocalIpAddrId,
 uint8 Option, uint8* DataLength, uint8* DataPtr 
)
{
	return E_OK;
}

Std_ReturnType TcpIp_DhcpWriteOption
(
 TcpIp_LocalAddrIdType LocalIpAddrId,
 uint8 Option, uint8 DataLength, 
 const uint8* DataPtr 
)
{
	return E_OK;
}

Std_ReturnType TcpIp_ChangeParameter
(
 TcpIp_SocketIdType SocketId,
 TcpIp_ParamIdType ParameterId, 
 const uint8* ParameterValue 
)
{ 
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	sint32 error, bufsize;
	struct linger linger;
	sint32 lingerSize;

	bufsize = sizeof(uint8);
	lingerSize = sizeof(struct linger);
	bzero(&linger, sizeof(struct linger));

	switch (ParameterId)
	{
	case TCPIP_PARAMID_TCP_RXWND_MAX:
		error = setsockopt(s, SOL_SOCKET, SO_RCVLOWAT, (void *)ParameterValue, bufsize);
		if(error)
			return E_NOT_OK;
		break;
	case TCPIP_PARAMID_FRAMEPRIO:
		error = setsockopt(s, SOL_SOCKET, SO_PRIORITY, (void *)ParameterValue, bufsize);
		if(error)
			return E_NOT_OK;
		break;
	case TCPIP_PARAMID_TCP_NAGLE:
		error = setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (void *)ParameterValue, bufsize);
		if(error)
			return E_NOT_OK;
		break;
	case  TCPIP_PARAMID_TCP_KEEPALIVE:
		error = setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (void *)ParameterValue, bufsize);
		if(error)
			return E_NOT_OK;
		break;
	case TCPIP_PARAMID_TTL:
		error = setsockopt(s, IPPROTO_IP, IP_TTL, (void *)ParameterValue, bufsize);
		if(error)
			return E_NOT_OK;
		break;
	case TCPIP_PARAMID_TCP_KEEPALIVE_TIME:
		linger.l_onoff = 1;
		linger.l_linger = (sint32)*ParameterValue;
		error = setsockopt(s, SOL_SOCKET, SO_LINGER, (void *)&linger, lingerSize);
		if(error)
			return E_NOT_OK;
		break;
	case TCPIP_PARAMID_PATHMTU_ENABLE:
		error = setsockopt(s, IPPROTO_TCP, TCP_MAXSEG, (void *)ParameterValue, bufsize);
		if(error)
			return E_NOT_OK;
		break;
	case TCPIP_PARAMID_TCP_KEEPALIVE_PROBES_MAX:
	case TCPIP_PARAMID_TCP_KEEPALIVE_INTERVAL:
	case TCPIP_PARAMID_TCP_OPTIONFILTER:	
	case TCPIP_PARAMID_FLOWLABEL:
	case TCPIP_PARAMID_DSCP:
	case TCPIP_PARAMID_UDP_CHECKSUM:
	case TCPIP_PARAMID_VENDOR_SPECIFIC:
		PR_DEBUG(TRUE, "These cmds don't support!\n");
		break;		
	default:
		break;
	}

	return E_OK;
}

Std_ReturnType TcpIp_GetIpAddr
(
 TcpIp_LocalAddrIdType LocalAddrId, 
 TcpIp_SockAddrType* IpAddrPtr, uint8* NetmaskPtr,
 TcpIp_SockAddrType* DefaultRouterPtr 
)
{
	if(LocalAddrId > DEF_IPADDR_MAX)
		return E_NOT_OK;
	memcpy((void *)IpAddrPtr ,(void *)&LocalAddr[LocalAddrId], sizeof(TcpIp_SockAddrType));

	(void)NetmaskPtr;
	(void)DefaultRouterPtr;

	if(!IpAddrPtr)
		return E_NOT_OK;

	return E_OK;
}

Std_ReturnType TcpIp_GetPhysAddr
(
 TcpIp_SocketIdType SocketId, 
 uint8* PhysAddrPtr 
)
{	
	uint8 CtrlIdx;
	sint32 retVal = 0;
	NIMU_IF_REQ  if_req;

	if(TcpIp_GetCtrlIdx(SocketId, &CtrlIdx) == E_NOT_OK)
		return E_NOT_OK;

	if_req.name[0] = 0;
	if_req.index = (uint32)CtrlIdx;

	retVal = NIMUIoctl(NIMU_GET_DEVICE_MAC, &if_req, (void*)PhysAddrPtr, sizeof(uint8)*6);
	if(retVal) {
		PR_DEBUG(TRUE, "Error: Incorrect device MAC specified\n");
		return E_NOT_OK;
	}

	if(!PhysAddrPtr)
		return E_NOT_OK;

	Vps_printf("MAC Address: %02x-%02x-%02x-%02x-%02x-%02x\n",
			PhysAddrPtr[0], PhysAddrPtr[1], PhysAddrPtr[2], 
			PhysAddrPtr[3], PhysAddrPtr[4], PhysAddrPtr[5]);

	return E_OK;
}

TcpIp_ReturnType TcpIp_GetRemotePhysAddr
(
 uint8 CtrlIdx, const TcpIp_SockAddrType* IpAddrPtr,
 uint8* PhysAddrPtr, BOOL initRes 
)
{
	return TCPIP_OK;
}

Std_ReturnType TcpIp_GetCtrlIdx
(
 TcpIp_SocketIdType SocketId,
 uint8* CtrlIdxPtr 
)
{
	SOCKET s = (SOCKET)Local_SocketId[SocketId - 1];
	sint32 ifVal = 1;
	sint32 retVal = 0;
	uint16      dev_index;
	NIMU_IF_REQ     if_req;

	retVal = setsockopt(s, SOL_SOCKET, SO_IFDEVICE, &ifVal, sizeof(ifVal));
	if (retVal)
	{
		PR_DEBUG(TRUE, "Only DGRAM sockets can use the SO_IFDEVICE option !!!\n");
	}
	
	if_req.index = 1;
	if (NIMUIoctl (NIMU_GET_DEVICE_INDEX, &if_req, &dev_index, sizeof(dev_index)) < 0) 
	{
		PR_DEBUG(TRUE, "Error: Incorrect device index specified\n");
		return E_NOT_OK;
	}

	/* Fill the object. */
	if_req.name[0] = 0;
	if_req.index = dev_index;

	memcpy((void *)CtrlIdxPtr , (void *)&dev_index, sizeof(uint8));

	if(!CtrlIdxPtr)
		return E_NOT_OK;

	return E_OK;
}

Std_ReturnType TcpIp_GetArpCacheEntries
(
 uint8 ctrlIdx, uint32* numberOfElements, 
 TcpIp_ArpCacheEntryType* entryListPtr 
)
{
	return E_OK;
}

Std_ReturnType TcpIp_GetAndResetMeasurementData
(
 TcpIp_MeasurementIdxType MeasurementIdx,
 BOOL MeasurementResetNeeded, 
 uint32* MeasurementDataPtr 
)
{
	return E_OK;
}

/*---------------------------------------------------------------------------*\
|                           Local Function Definitions                        |
\*---------------------------------------------------------------------------*/

static Std_ReturnType TcpIp_static_socket_init(void) 
{	
	static uint32 i = DEF_SOCKET_FIRST;
	uint32 TcpIp_SocketIdLen = sizeof(TcpIp_SocketIdType *);

Socket_Alloc:		
	if(Local_SocketId_Status == UnLocked) {	
		for(; i < DEF_SOCKET_MAX_NUM; i++) {		
			Local_SocketId[i] = (TcpIp_SocketIdType *)Utils_memAlloc( \
					UTILS_HEAPID_DDR_CACHED_SR, 
					TcpIp_SocketIdLen*DEF_SOCKET_MAX_NUM, 
					16);	
			if(Local_SocketId[i] == NULL) 
				goto Error_Alloc;
			bzero(Local_SocketId[i], TcpIp_SocketIdLen);
		}
		Local_SocketId_Status = Locked;	
	}
	return E_OK;

Error_Alloc:
	for(i = 0; i < DEF_SOCKET_MAX_NUM; i++) {
		if(Local_SocketId[i] != NULL) {
			Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
					Local_SocketId[i],
					sizeof(TcpIp_SocketIdType *)*DEF_SOCKET_MAX_NUM);
			Local_SocketId[i] = NULL;
		}
	}
	Local_SocketId_Status = UnLocked;
	goto Socket_Alloc;
}

static Std_ReturnType TcpIp_Network_WaitConnect
(
 TcpIp_SocketIdType SocketId, TcpIp_Network_SockObj *pObj,
 const TcpIp_SockAddrType* RemoteAddrPtr, uint32 timeout
)
{
	uint8 status = E_OK;
    pObj->pollitem[0].fd = pObj->sockFd = (SOCKET)Local_SocketId[SocketId - 1];;
    pObj->pollitem[0].eventsRequested = POLLIN;

	PSA pName = (PSA)RemoteAddrPtr;
	sint32 len = sizeof(*pName);
	
    if( fdPoll( pObj->pollitem, 1U, timeout ) == TCPIP_SOCKETID_ERROR )
    {
        Vps_printf(" NETWORK: fdPoll() failed with SOCKET_ERROR (port=%d) !!!\n", pObj->port);
        status =  E_NOT_OK;
    }
    else
    {
        if( pObj->pollitem[0].eventsDetected == FALSE)
        {
            /* NO connection, retry */
            status = E_OK;
			PR_DEBUG(DEBUG_SWITCH, "%s:Status is E_OK continue waiting !!! \n", __func__);
        }
        else
        {
            if( (uint32)(pObj->pollitem[0].eventsDetected) & (uint32)POLLNVAL )
            {
                Vps_printf(" NETWORK: fdPoll() failed with POLLNVAL (port=%d) !!!\n", pObj->port);
                status = E_NOT_OK;
            }
            else
            {

                if( (uint32)(pObj->pollitem[0].eventsDetected) & (uint32)POLLIN )
                {
                    pObj->connectedSockFd = accept( pObj->sockFd, pName, &len);
					
					/* Copy the client remoteAddr. */
					memcpy((void *)RemoteAddrPtr, (void *)pName, len); 
					
                    if( pObj->connectedSockFd != (SOCKET)TCPIP_SOCKETID_INVALID )
                    {
                        status = NETWORK_CONNECT_SUCCESS;
                    }
                }
            }
        }
    }
    /* NO connection, retry */
    return status;
}

/* bzero the static ipaddr object. */
static void TcpIp_static_ipaddr_init(void) 
{	
	static uint8 index;
	static uint32 TcpIp_SockAddrInetLen = sizeof(TcpIp_SockAddrInetType);
	static uint32 Partial = DEF_STATIC_IPADDR_NUM;

	if(LocalAddr_Init_Lock == UnLocked) {
		for(index = 0; index < DEF_STATIC_IPADDR_MAX; index++) {
			bzero(&LocalAddr[index], TcpIp_SockAddrInetLen);
			LocalAddr[index].domain = TCPIP_AF_INET;
		}

		/* Entry the static ipaddr. */
		for(index = 0; index < Partial; index++)
			LocalAddr[index].addr.s_addr = TCPIP_STATIC_IPADDR;
		for(; index < 2*Partial; index++)
			LocalAddr[index].addr.s_addr = TCPIP_DOIP_IPADDR;
		for(; index < 3*Partial; index++)
			LocalAddr[index].addr.s_addr = TCPIP_LINKLOCAL_IPADDR;
		
		/*Extra LocalAddr for reconnect.*/
		{
			sint8* cursor = (sint8 *)LocalIpAddrLeft[0];
			for(; index < DEF_IPADDR_REMAIN; index++, cursor++)
			{
				LocalAddr[index].addr.s_addr = inet_addr((const char *)cursor);
			}
		}
		LocalAddr_Init_Lock = Locked;
	}
}

static void TcpIp_static_tx_buffer_init(void)
{
TxBuffer_Alloc:	
	if(Local_TxBuffer_Lock == UnLocked) {
		TxBuffer = (uint8 *)Utils_memAlloc( \
				UTILS_HEAPID_DDR_CACHED_SR, 
				GLOBAL_DATASRV_BUFSIZE, 
				16);
		if(!TxBuffer) {
			PR_DEBUG(TRUE, "TxBuffer: allocation failed, sending constant size\n");
			goto Error_Alloc_TxBuffer;
		}
		bzero(TxBuffer, GLOBAL_DATASRV_BUFSIZE);
		TempTxBuffer = TxBuffer;
		Local_TxBuffer_Lock = Locked;
	}
	return;

Error_Alloc_TxBuffer:
	if(TxBuffer) {
		Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR,
				TxBuffer,
				GLOBAL_DATASRV_BUFSIZE);
		TxBuffer = NULL;
	}
	Local_TxBuffer_Lock = UnLocked;
	goto TxBuffer_Alloc;
}

static void TcpIp_static_rx_buffer_init(void)
{
RxBuffer_Alloc:	
	if(Local_RxBuffer_Lock == UnLocked) {
		RxBuffer = (uint8 *)Utils_memAlloc( \
				UTILS_HEAPID_DDR_CACHED_SR, 
				GLOBAL_DATASRV_BUFSIZE, 
				16);
		if(!RxBuffer) {
			PR_DEBUG(TRUE, "RxBuffer: allocation failed, sending constant size\n");
			goto Error_Alloc_RxBuffer;
		}
		bzero(RxBuffer, GLOBAL_DATASRV_BUFSIZE);
		TempRxBuffer = RxBuffer;
		Local_RxBuffer_Lock = Locked;
	}
	return;

Error_Alloc_RxBuffer:
	if(RxBuffer) {
		Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR, 
				RxBuffer,
				GLOBAL_DATASRV_BUFSIZE);
		RxBuffer = NULL;
	}
	Local_RxBuffer_Lock = UnLocked;	
	goto RxBuffer_Alloc;
}

static BufReq_ReturnType SoAd_CopyRxData(uint8* BufPtr, uint16 BufLength)
{
	uint16 Length = 0;

	if(!BufPtr)
	{
		PR_DEBUG(TRUE, "%s : %u BufPtr is NULL !!!\n", \
								__func__, __LINE__);
		return BUFREQ_E_NOT_OK;
	}

	if(TempRxBuffer <= (RxBuffer + GLOBAL_DATASRV_BUFSIZE - BufLength)) 
	{
		memcpy(TempRxBuffer, BufPtr, BufLength);
		TempRxBuffer +=  BufLength;
		RxBufferLength += BufLength;
	}
	else {
		Length = RxBuffer + GLOBAL_DATASRV_BUFSIZE - TempRxBuffer;
		memcpy(TempRxBuffer, BufPtr, Length);
		RxBufferLength = GLOBAL_DATASRV_BUFSIZE;
	}

	return BUFREQ_OK;
}

static BufReq_ReturnType SoAd_CopyTxData(uint8* BufPtr, uint16 BufLength)
{
	uint16 Length = 0;

	if(!BufPtr)
	{
		PR_DEBUG(TRUE, "%s : %u BufPtr is NULL !!!\n", \
								__func__, __LINE__);
		return BUFREQ_E_NOT_OK;
	}

	if(TempTxBuffer <= (TxBuffer + GLOBAL_DATASRV_BUFSIZE - BufLength)) {
		memcpy(TempTxBuffer, BufPtr, BufLength);
		TempTxBuffer +=  BufLength;
		TxBufferLength += BufLength;
	}
	else {
		Length = TxBuffer + GLOBAL_DATASRV_BUFSIZE - TempTxBuffer;
		memcpy(TempTxBuffer, BufPtr, Length);
		TxBufferLength = GLOBAL_DATASRV_BUFSIZE;
	}

	return BUFREQ_OK;
}

/*---------------------------------------------------------------------------*\
|                                 End of File                                 |
\*---------------------------------------------------------------------------*/

