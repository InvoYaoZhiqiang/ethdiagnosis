/*  BEGIN_FILE_HDR
 ************************************************************************************************
 *   NOTICE
 *   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
 *   doc should not be reproduced, or used, or disclosed without the written authorization from
 *   IntelliDrive Co.,Ltd..
 ************************************************************************************************
 *   File Name       : soAd.c
 *   Module Name     : 
 *   Project         : Autosar
 *   Processor       : 
 *   Description	 : AUTOSAR NDK legacy socket adapter support
 *   Component		 : 
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

/*---------------------------------------------------------------------------*\
|                         Global extern Declarations                       	  |
\*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*\
|                  Local Macros/Defines/Typedefs/Enums/Variable               |
\*---------------------------------------------------------------------------*/

/* The global errorlog. */
#define errorlog(...)   do {\
								Vps_printf(__VA_ARGS__);\
								fdCloseSession(TaskSelf());\
							} while(0)

/* The global lock. */
#define UnLocked  FALSE
#define Locked    TRUE

/* The MAX SoCon NUM. */
#define DEF_MAX_SOCONID_NUM   DEF_SOCKET_MAX_NUM

/* The gobal DEBUG ON/OFF. */
#define DEBUG_TCP   		0
#define DEBUG_UDP 			0
#define ENABLE_NOWAIT       1
#define ENABLE_100MS        1

/*===================== SoAd Main Connection ====================*/

static BOOL SoAd_Main_Connection_Lock = UnLocked;

/*===================== Map the Pdu2SoCon. ======================*/

/* The static SoAd_Pdu2SoCon_Type. */
typedef struct 
{
	PduIdType PduId;
	SoAd_SoConIdType SoConId;
	TcpIp_LocalAddrIdType LocalAddrId;
	TcpIp_ProtocolType Protocol;
	SoAd_SoConModeType SoConMode;
	BOOL GlobalStarOfMain;
} SoAd_Pdu2SoCon_Type;

/* Map the PduId and SoConId. */
#define DEF_PDU2SOCON_TX_NUM  2U
#define DEF_PDU2SOCON_RX_NUM  2U

#define DEF_TXPDUID0     0x00
#define DEF_TXPDUID1     0x01

#define DEF_RXPDUID0     0x00
#define DEF_RXPDUID1     0x01

#define DEF_SOCONID		 1U

/* Config the SoAd_Pdu2SoCon_Tx. */
static const SoAd_Pdu2SoCon_Type  SoAd_Pdu2SoCon_Tx_cfg[DEF_PDU2SOCON_TX_NUM] = {

	{DEF_TXPDUID0, DEF_SOCONID, 1, TCPIP_IPPROTO_TCP, SOAD_SOCON_OFFLINE, FALSE},
	{DEF_TXPDUID1, DEF_SOCONID, 1, TCPIP_IPPROTO_UDP, SOAD_SOCON_OFFLINE, FALSE},
};

/* Config the SoAd_Pdu2SoCon_Rx. */	
static const SoAd_Pdu2SoCon_Type  SoAd_Pdu2SoCon_Rx_cfg[DEF_PDU2SOCON_RX_NUM] = {

	{DEF_RXPDUID0, DEF_SOCONID, 1, TCPIP_IPPROTO_TCP, SOAD_SOCON_OFFLINE, FALSE},
	{DEF_RXPDUID1, DEF_SOCONID, 1, TCPIP_IPPROTO_UDP, SOAD_SOCON_OFFLINE, FALSE},
};

/* Define the object. */
static SoAd_Pdu2SoCon_Type  SoAd_Pdu2SoCon_Tx[DEF_PDU2SOCON_TX_NUM];
static SoAd_Pdu2SoCon_Type  SoAd_Pdu2SoCon_Rx[DEF_PDU2SOCON_RX_NUM];


/* The global Pdu2SoCon init lock. */
static BOOL SoAd_Pdu2SoCon_Tx_Lock = UnLocked;
static BOOL SoAd_Pdu2SoCon_Rx_Lock = UnLocked;

/* Define the first Pdu Length. */
#define DEF_PDU_LENGTH  5000U

/* The Global SoConID lock. */
static BOOL SoAd_Get_Lock = UnLocked;

/*================ Global configuration. ================*/

/* Config the default port. */
#define DEF_PORT 13400

/* Define the Max Channel. */
#define DEF_MAX_CHANNEL  5U

/* The Global LocalAddr. */
static TcpIp_SockAddrType NewLocalIpAddr;

/* The Global Lock of Assign IpAddr. */
static BOOL SoAd_RequestIpAddrAssignment_Lock = UnLocked;

/* The Global Lock of TcpStartofReception. */
static BOOL SoAd_TcpStartOfReception_Lock = UnLocked;

/* The Global Lock of TcpStartofReception. */
static BOOL SoAd_UdpStartOfReception_Lock = UnLocked;

/* The Global Lock of TcpTransmition. */
static BOOL SoAd_TcpTransmition_Lock = Locked;
static BOOL SoAd_TcpTransmition_Local_Lock = Locked;

/* The Global Lock of UdpTransmition. */
static BOOL SoAd_UdpTransmition_Lock = Locked;
static BOOL SoAd_UdpTransmition_Local_Lock = Locked;

/* ==================Global TcpTxBuffer. ====================*/

/* The global Tcp TxBuffer. */
static uint8 *TcpTxBuffer;
static uint8 *TempTcpTxBuffer = NULL;
static uint16 TcpTxBufferLength = 0;

/* The global Udp TxBuffer. */
static uint8 *UdpTxBuffer;
static uint8 *TempUdpTxBuffer = NULL;
static uint16 UdpTxBufferLength = 0;

/* The global TcpTxBuffer lock. */
static BOOL Local_TxBuffer_Lock = UnLocked;

/*============== Communication global parameters ===========*/

/* For connection parameters */
static SoAd_SoConIdType Tcp_SoConId_Main = DEF_SOCONID;
static SoAd_SoConIdType Udp_SoConId_Main = DEF_SOCONID;
static SoAd_SoConIdType ConnectedId_Main = DEF_SOCONID;
static TcpIp_LocalAddrIdType LocalAddrId = 1U;
static uint8 GlobalIndex = 0U;

/* For the communication. */
static uint8  BufPtr_Main[DEF_PDU_LENGTH] = {'\0'};
static uint8  BufPtr_TcpRecv[DEF_PDU_LENGTH] = {'\0'};
static uint8  BufPtr_UdpRecv[DEF_PDU_LENGTH] = {'\0'};
static TcpIp_SockAddrType GlobalRemoteAddr;
		
/* The parameter of Pdu. */
static PduInfoType Info_Main;
static PduLengthType BufferSize_Main = 0;

/* The Global SoConId state machine. */
typedef enum
{
	DIAG_TX_SERVER_CLOSED = 0U,        
	DIAG_TX_SERVER_LISTEN = 1U,       
	DIAG_TX_SERVER_CONNECTED = 2U    
}TCP_STATE;

static TCP_STATE TcpState;

/*---------------------------------------------------------------------------*\
|                         Local Function Declarations                         |
\*---------------------------------------------------------------------------*/

/* Access to the TCP/IP socket layer. */
static Std_ReturnType SoAd_GetSoConId_Local(PduIdType TxPduId, SoAd_SoConIdType* SoConIdPtr );

/* The local Pdu2SoCon defination. */
static void  SoAd_Pdu2SoCon_Tx_Init(void );
static void SoAd_Pdu2SoCon_Rx_Init(void );

/* Gobal buffer defination. */
static void SoAd_static_tx_buffer_init(void);

/* The TCP/UDP Complete job. */
static void SoAd_DoPreparationForConnectionJob(SoAd_SoConIdType *TcpSoConIdPtr,
														  SoAd_SoConIdType *UdpSoConIdPtr,
														  SoAd_SoConIdType *SocketIdConnectedPtr, 
														  TcpIp_LocalAddrIdType *LocalAddrIdPtr, 
														  TcpIp_SockAddrType *RemoteAddrPtr,
														  uint8 *BufPtr);

static Std_ReturnType SoAd_DoTcpConnectJob(SoAd_SoConIdType *SoConIdPtr,
												 SoAd_SoConIdType *SocketIdConnectedPtr, 
												 TcpIp_LocalAddrIdType *LocalAddrIdPtr, 
											     TcpIp_SockAddrType *RemoteAddrPtr,
											     uint8 *BufPtr);

static Std_ReturnType SoAd_DoUdpConnectJob(SoAd_SoConIdType *SoConIdPtr, 
												 TcpIp_LocalAddrIdType *LocalAddrIdPtr, 
											     TcpIp_SockAddrType *RemoteAddrPtr,
											     uint8 *BufPtr);
static void SoAd_TcpTransmitionJob(SoAd_SoConIdType *AcceptIdPtr, TcpIp_SockAddrType *RemoteAddrPtr);
static void SoAd_UdpTransmitionJob(SoAd_SoConIdType *SoConIdPtr, TcpIp_SockAddrType *RemoteAddrPtr);

static Std_ReturnType SoAd_TcpCommunicationJob(SoAd_SoConIdType *AcceptIdPtr, 
													   TcpIp_SockAddrType *RemoteAddrPtr,
													   PduInfoType *info, 
													   PduLengthType* bufferSizePtr);

static Std_ReturnType SoAd_UdpCommunicationJob(SoAd_SoConIdType *SoConIdPtr, 
													   TcpIp_SockAddrType *RemoteAddrPtr, 
													   PduInfoType *info, 
													   PduLengthType* bufferSizePtr);

/*---------------------------------------------------------------------------*\
|                          Global Function Definitions                        |
\*---------------------------------------------------------------------------*/

void SoAd_Init(void)
{
	PR_DEBUG(TRUE, "SoAd_Init start !!!\n");
    
	/* bzero the golbal LocalAddr. */
	bzero(&NewLocalIpAddr, sizeof(TcpIp_SockAddrType));

	/* The local Pdu2SoCon Tx initialization. */
	SoAd_Pdu2SoCon_Tx_Init();

	/* The local Pdu2SoCon Rx initialization. */
	SoAd_Pdu2SoCon_Rx_Init();
	
	/* Gobal buffer initialization.*/
	SoAd_static_tx_buffer_init();

	TcpState = DIAG_TX_SERVER_CLOSED;

    PR_DEBUG(TRUE, "SoAd_Init done !!!\n");	
}

void SoAd_DeInit(void )
{
	uint8 index = 0;
	
	/* bzero the golbal LocalAddr. */
	bzero(&NewLocalIpAddr, sizeof(TcpIp_SockAddrType));

    TcpState = DIAG_TX_SERVER_CLOSED;
    
	/* DeInit the SoAd_Pdu2SoCon_Tx. */
	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
		bzero(&(SoAd_Pdu2SoCon_Tx[index]), sizeof(SoAd_Pdu2SoCon_Type));
	if(SoAd_Pdu2SoCon_Tx_Lock)
		SoAd_Pdu2SoCon_Tx_Lock = UnLocked;

	/* DeInit the SoAd_Pdu2SoCon_Rx. */
	for(index = 0; index < DEF_PDU2SOCON_RX_NUM; index++)
		bzero(&(SoAd_Pdu2SoCon_Rx[index]), sizeof(SoAd_Pdu2SoCon_Type));
	if(SoAd_Pdu2SoCon_Rx_Lock)
		SoAd_Pdu2SoCon_Rx_Lock = UnLocked;

	/* DeInit the SoAdLock. */
	if(SoAd_Get_Lock)
		SoAd_Get_Lock = UnLocked;

	if(SoAd_Main_Connection_Lock)
		SoAd_Main_Connection_Lock = UnLocked;
	
	if(SoAd_TcpStartOfReception_Lock)
		SoAd_TcpStartOfReception_Lock = UnLocked;

	if(SoAd_UdpStartOfReception_Lock)
		SoAd_UdpStartOfReception_Lock = UnLocked;
	
	if(SoAd_TcpTransmition_Lock == UnLocked)
		SoAd_TcpTransmition_Lock = Locked;

	if(SoAd_UdpTransmition_Lock == UnLocked)
		SoAd_UdpTransmition_Lock = Locked;

	if(SoAd_TcpTransmition_Local_Lock == UnLocked)
		SoAd_TcpTransmition_Local_Lock = Locked;

	if(SoAd_UdpTransmition_Local_Lock == UnLocked)
		SoAd_UdpTransmition_Local_Lock = Locked;
	
	if(SoAd_RequestIpAddrAssignment_Lock)
		SoAd_RequestIpAddrAssignment_Lock = UnLocked;
	
	/* DeInit the Tcp TxBuffer. */
	if(TcpTxBuffer) {
		Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR, 
   					 TcpTxBuffer, 
					 GLOBAL_DATASRV_BUFSIZE);
		TcpTxBuffer = NULL;
	}
	if(TempTcpTxBuffer)
		TempTcpTxBuffer = NULL;

	/* DeInit the Udp TxBuffer. */
	if(UdpTxBuffer) {
		Utils_memFree(UTILS_HEAPID_DDR_CACHED_SR, 
   					 UdpTxBuffer, 
					 GLOBAL_DATASRV_BUFSIZE);
		UdpTxBuffer = NULL;
	}
	if(TempUdpTxBuffer)
		TempUdpTxBuffer = NULL;
	
	if(Local_TxBuffer_Lock)
		Local_TxBuffer_Lock = UnLocked;
}

void SoAd_MainFunction(void)
{
	uint8 index = 0;
	PduLengthType TpSduLength;
	BufReq_ReturnType RetVal;
	
	if(SoAd_Main_Connection_Lock == UnLocked)
	{
		bzero(&GlobalRemoteAddr, sizeof(TcpIp_SockAddrType));
		
		SoAd_DoPreparationForConnectionJob(&Tcp_SoConId_Main, &Udp_SoConId_Main, &ConnectedId_Main, &LocalAddrId, &GlobalRemoteAddr, BufPtr_Main);
		
		for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++)
		{
			if(Tcp_SoConId_Main == SoAd_Pdu2SoCon_Tx[index].SoConId \
				&& TRUE == SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain \
				&& SoAd_Pdu2SoCon_Tx[index].Protocol == TCPIP_IPPROTO_TCP \
				&& SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_ONLINE)
			{	
				SoAd_Main_Connection_Lock = Locked;
				SoAd_Get_Lock = Locked;
				PR_DEBUG(TRUE, "TcpConnection online !!!\n");
				break;
			}
		}
		
		for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++)
		{
			if(Udp_SoConId_Main == SoAd_Pdu2SoCon_Tx[index].SoConId \
				&& TRUE == SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain \
				&& SoAd_Pdu2SoCon_Tx[index].Protocol == TCPIP_IPPROTO_UDP \
				&& SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_ONLINE)
			{
				SoAd_Main_Connection_Lock = Locked;
				SoAd_Get_Lock = Locked;
				PR_DEBUG(TRUE, "UdpConnection online !!!\n");
				break;
			}
		}
	}

	/*=============================== TCP COMMUNICATION ============================*/
	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
	{
		if(TRUE == SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain)
		{
			if(Tcp_SoConId_Main == SoAd_Pdu2SoCon_Tx[index].SoConId \
					&& SoAd_Pdu2SoCon_Tx[index].Protocol == TCPIP_IPPROTO_TCP \
						&& SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_ONLINE)
			{	
				if(SoAd_TcpStartOfReception_Lock == UnLocked)
				{
					/* Init the PduInfo. */
					bzero(&Info_Main, sizeof(PduInfoType));
					TpSduLength = 0; /* SWS_DoIP_00189 */

					RetVal = DoIP_SoAdTpStartOfReception(DEF_RXPDUID0, &Info_Main, TpSduLength, &BufferSize_Main);
					PR_DEBUG(DEBUG_TCP, "Tcp: BufferSize_Main: %u\n", BufferSize_Main);
					
					switch(RetVal)
					{
						case BUFREQ_OK:
							if(SoAd_TcpCommunicationJob(&ConnectedId_Main, NULL, &Info_Main,  &BufferSize_Main) == E_NOT_OK)
							{
								PR_DEBUG(TRUE, "SoAd_TcpCommunication fail, try again automatically !!!\n");
								return;
							}
							break;
						case BUFREQ_E_NOT_OK:
							PR_DEBUG(TRUE, "Failed to DoIP_SoAdTpStartOfReception, BUFREQ_E_NOT_OK!\n");
							SoAd_CloseSoCon(Tcp_SoConId_Main, TRUE);
							SoAd_CloseSoCon(ConnectedId_Main, TRUE);		
							SoAd_TcpStartOfReception_Lock = UnLocked;
							TaskSleep(1000);
							return;
						case BUFREQ_E_OVFL:
							PR_DEBUG(TRUE, "Failed to DoIP_SoAdTpStartOfReception, BUFREQ_E_OVFL!\n");
							break;
					}
					SoAd_TcpStartOfReception_Lock = Locked;
					PR_DEBUG(DEBUG_TCP, "SoAd_StartOfReception_Lock is locked at %s !!!\n", __TIME__);
				}
				else {
					if(SoAd_TcpCommunicationJob(&ConnectedId_Main, NULL, &Info_Main,  &BufferSize_Main) == E_NOT_OK)
					{
						PR_DEBUG(TRUE, "SoAd_TcpCommunication fail, try again automatically !!!\n");
						return;
					}
				}
				break;
			}
		}
	} /* END OF TCP COMMUNICATION BLOCK */

	/*=============================== UDP COMMUNICATION ============================*/
	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
	{
		if(TRUE == SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain)
		{		
			if(Udp_SoConId_Main == SoAd_Pdu2SoCon_Tx[index].SoConId \
					&& SoAd_Pdu2SoCon_Tx[index].Protocol == TCPIP_IPPROTO_UDP \
						&& SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_ONLINE)
			{
				/* Do the preparation for Udp communication. */
				PR_DEBUG(DEBUG_UDP, "LINE %u: Do the UDP preparation for communication !!!\n", __LINE__);

				if(SoAd_UdpStartOfReception_Lock == UnLocked)
				{
					/* Init the PduInfo. */
					bzero(&Info_Main, sizeof(PduInfoType));
					TpSduLength = 0;
					
					RetVal = DoIP_SoAdTpStartOfReception(DEF_RXPDUID1, &Info_Main, TpSduLength, &BufferSize_Main);

					switch(RetVal)
					{
					case BUFREQ_OK:
						if(SoAd_UdpCommunicationJob(&Udp_SoConId_Main , &GlobalRemoteAddr, &Info_Main, &BufferSize_Main) == E_NOT_OK)
						{
							PR_DEBUG(TRUE, "SoAd_UdpCommunication fail, try again automatically !!!\n");
							return;
						}
						break;
					case BUFREQ_E_NOT_OK:
						PR_DEBUG(TRUE, "Failed to DoIP_SoAdTpStartOfReception, BUFREQ_E_NOT_OK!\n");
						SoAd_CloseSoCon(Udp_SoConId_Main, TRUE);
						SoAd_UdpStartOfReception_Lock = UnLocked;
						TaskSleep(1000);
						return;
					case BUFREQ_E_OVFL:
						PR_DEBUG(TRUE, "Failed to DoIP_SoAdTpStartOfReception, BUFREQ_E_OVFL!\n");
						break;
					}
					SoAd_UdpStartOfReception_Lock = Locked;
					PR_DEBUG(DEBUG_UDP, "SoAd_StartOfReception_Lock is locked at %s !!!\n", __TIME__);
				}
				else {
					if(SoAd_UdpCommunicationJob(&Udp_SoConId_Main , &GlobalRemoteAddr, &Info_Main, &BufferSize_Main) == E_NOT_OK)
					{
						PR_DEBUG(TRUE, "SoAd_UdpCommunication fail, try again automatically !!!\n");
						return;
					}
				}
				break;
			}
		}
	} /* END OF UDP COMMUNICATION BLOCK */
}/* END OF SOAD_MAINFUNCTION */

Std_ReturnType SoAd_GetSoConId(PduIdType TxPduId, SoAd_SoConIdType* SoConIdPtr)
{
	uint8 index = 0;

	if(TxPduId != DEF_TXPDUID0 && TxPduId != DEF_TXPDUID1)
		return E_NOT_OK;

	if(SoAd_Get_Lock == UnLocked)
	{
		/* Open the fdsession. */
		fdOpenSession(TaskSelf());

		if(SoAd_GetSoConId_Local(TxPduId, SoConIdPtr) == E_NOT_OK)
		{
			errorlog("%s LINE %u: Failed to get socket!\n", __func__, __LINE__);
			if(*SoConIdPtr)
				SoAd_CloseSoCon(*SoConIdPtr, TRUE);
			return E_NOT_OK;
		}

		GlobalIndex++;
		if(GlobalIndex == 2) 
		{
			SoAd_Get_Lock = Locked;
			GlobalIndex = 0;
		}
		
		/* Raise the state DOIP_LINK_UP !!!*/
		DoIP_LocalIpAddrAssignmentChg(*SoConIdPtr, TCPIP_IPADDR_STATE_ASSIGNED);
	}
	else {
		for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++)
		{
			if(TxPduId == SoAd_Pdu2SoCon_Tx[index].PduId)
			{
				*SoConIdPtr = SoAd_Pdu2SoCon_Tx[index].SoConId;
				if((SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_OFFLINE \
						|| SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_RECONNECT)
						&& FALSE == SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain)
				{
					SoAd_Pdu2SoCon_Tx[index].SoConMode = SOAD_SOCON_READY;
				} 
				else
					break;
			}
		}
	}
	return E_OK;
}

Std_ReturnType SoAd_OpenSoCon(SoAd_SoConIdType SoConId ) 
{	
	uint8 index = 0;
	TcpIp_IpAddrAssignmentType Type;
	
	/* Check the SoConId. */
	if(SoConId >= DEF_MAX_SOCONID_NUM)
		return E_NOT_OK;
	
	/* Assign the LocalAddr before established. */
	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
	{
		if(SoConId == SoAd_Pdu2SoCon_Tx[index].SoConId \
				&& (SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY \
					|| SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_RECONNECT)
							&& SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain == FALSE \
							&& SoAd_RequestIpAddrAssignment_Lock == UnLocked)
		{
			if(SoAd_Pdu2SoCon_Tx[index].LocalAddrId < DEF_STATIC_IPADDR_NUM)
				Type = TCPIP_IPADDR_ASSIGNMENT_STATIC;
			if(SoAd_Pdu2SoCon_Tx[index].LocalAddrId >= DEF_STATIC_IPADDR_NUM && SoAd_Pdu2SoCon_Tx[index].LocalAddrId < DEF_DOIP_IPADDR_NUM)
				Type = TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP;
			if(SoAd_Pdu2SoCon_Tx[index].LocalAddrId >= DEF_DOIP_IPADDR_NUM && SoAd_Pdu2SoCon_Tx[index].LocalAddrId < DEF_STATIC_IPADDR_MAX)
				Type = TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL;

			/* Assign the IpAddr. */
			if(SoAd_RequestIpAddrAssignment(SoConId, Type, &NewLocalIpAddr, 0, NULL) == E_NOT_OK)
			{
				PR_DEBUG(TRUE, "Failed to request LocalAddrPtr.\n");
				return E_NOT_OK;
			}
			break;
		}
	}
	
	if(SoAd_RequestIpAddrAssignment_Lock == UnLocked)
	{
 		/* DEBUG PRINT THE IPADDR. */
 		{
 			char String[20] = "";
			TcpIp_SockAddrInetType *IPADDR = (TcpIp_SockAddrInetType *)&NewLocalIpAddr;
 			IPN NetIpaddr = (IPN)IPADDR->addr.s_addr;

			NetIpaddr = htonl(NetIpaddr);
			snprintf(String, sizeof(String), "%d.%d.%d.%d",
					(UINT8)((NetIpaddr>>24)&0xFF), (UINT8)((NetIpaddr>>16)&0xFF),
					(UINT8)((NetIpaddr>>8)&0xFF), (UINT8)(NetIpaddr&0xFF));			
			PR_DEBUG(DEBUG_TCP, "Server request IPADDR:%s !!!\n", String);
 		}
		SoAd_RequestIpAddrAssignment_Lock = Locked;
	} 
	else
	{
		PR_DEBUG(DEBUG_SWITCH, "SoAd_RequestIpAddrAssignment_Lock = %s\n", \
					SoAd_RequestIpAddrAssignment_Lock == Locked ? "Locked" : "UnLocked");
	}
	
	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
	{
		if(SoConId == SoAd_Pdu2SoCon_Tx[index].SoConId \
				&& (SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY
				||  SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_RECONNECT)) 
		{	
			PR_DEBUG(DEBUG_SWITCH, "REENTRANT THE SOAD_OPEN_FUNC AT %s !!!\n", __TIME__);
            
			/* Set the Global start of main. */
			SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain = TRUE;
			SoAd_Pdu2SoCon_Tx[index].SoConMode = SOAD_SOCON_READY;
			break;
		}
	}
	
	return E_OK;
}

Std_ReturnType SoAd_CloseSoCon(SoAd_SoConIdType SoConId, BOOL abort) 
{
	uint8 index = 0;

	if(SoConId >= DEF_MAX_SOCONID_NUM)
		return E_NOT_OK;
	
	if(abort)
	{
		PR_DEBUG(TRUE, "REENTRANT THE SOAD_CLOSE_FUNC AT %s ARORT IS TRUE!!!\n", __TIME__);

		for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
		{
			if(SoConId == SoAd_Pdu2SoCon_Tx[index].SoConId \
					&& (SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_ONLINE \
					|| SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY \
					|| SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_RECONNECT)) 
			{
				SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain = FALSE;
				SoAd_Pdu2SoCon_Tx[index].SoConMode = SOAD_SOCON_OFFLINE;
				SoAd_Pdu2SoCon_Tx[index].SoConId = DEF_SOCONID;
				break;
			}
		}

		for(index = 0; index < DEF_PDU2SOCON_RX_NUM; index++) 
		{
			if(SoConId == SoAd_Pdu2SoCon_Rx[index].SoConId \
					&& (SoAd_Pdu2SoCon_Rx[index].SoConMode == SOAD_SOCON_ONLINE \
					|| SoAd_Pdu2SoCon_Rx[index].SoConMode == SOAD_SOCON_READY \
					|| SoAd_Pdu2SoCon_Rx[index].SoConMode == SOAD_SOCON_RECONNECT)) 
			{
				SoAd_Pdu2SoCon_Rx[index].GlobalStarOfMain = FALSE;
				SoAd_Pdu2SoCon_Rx[index].SoConMode = SOAD_SOCON_OFFLINE;
				SoAd_Pdu2SoCon_Rx[index].SoConId = DEF_SOCONID;
				break;
			}
		}

		TcpIp_Close(SoConId);
		SoAd_Get_Lock = UnLocked;
		SoAd_Main_Connection_Lock = UnLocked;
		TcpState=DIAG_TX_SERVER_CLOSED;
		fdCloseSession(TaskSelf());
	}
	else 
	{
		PR_DEBUG(TRUE, "REENTRANT THE SOAD_CLOSE_FUNC AT %s ARORT IS FALSE!!!\n", __TIME__);
		for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
		{
			if(SoConId == SoAd_Pdu2SoCon_Tx[index].SoConId \
					&& (SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_ONLINE \
					|| SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY)) 
			{
				SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain = FALSE;
				SoAd_Pdu2SoCon_Tx[index].SoConMode = SOAD_SOCON_RECONNECT;
				break;
			}
		}

		for(index = 0; index < DEF_PDU2SOCON_RX_NUM; index++) 
		{
			if(SoConId == SoAd_Pdu2SoCon_Rx[index].SoConId \
					&& (SoAd_Pdu2SoCon_Rx[index].SoConMode == SOAD_SOCON_ONLINE \
					|| SoAd_Pdu2SoCon_Rx[index].SoConMode == SOAD_SOCON_READY)) 
			{
				SoAd_Pdu2SoCon_Rx[index].GlobalStarOfMain = FALSE;
				SoAd_Pdu2SoCon_Rx[index].SoConMode = SOAD_SOCON_RECONNECT;
				break;
			}
		}
	}	
	return E_OK;
}

void SoAd_GetSoConMode(SoAd_SoConIdType SoConId, SoAd_SoConModeType* ModePtr )
{
	DoIP_SoConModeChg(SoConId, *ModePtr);
}

/*============================= Address Configuaration. ==================================*/

Std_ReturnType SoAd_RequestIpAddrAssignment
(
 SoAd_SoConIdType SoConId, 
 TcpIp_IpAddrAssignmentType Type, 
 const TcpIp_SockAddrType* LocalIpAddrPtr, 
 uint8 Netmask, const TcpIp_SockAddrType* DefaultRouterPtr 
) 
{
	uint8 index = 0;

	if(SoConId >= DEF_MAX_SOCONID_NUM)
		return E_NOT_OK;
	else
	{
		for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++)
		{
			if(SoConId == SoAd_Pdu2SoCon_Tx[index].SoConId \
					&& (SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY \
						|| SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_RECONNECT))
			{			
				if(TcpIp_RequestIpAddrAssignment(SoAd_Pdu2SoCon_Tx[index].LocalAddrId, 
							Type, LocalIpAddrPtr, Netmask, 
							DefaultRouterPtr) == E_NOT_OK)
					return E_NOT_OK;
				break;
			}
		}
	}
	return E_OK;
}

Std_ReturnType SoAd_ReleaseIpAddrAssignment(SoAd_SoConIdType SoConId) 
{
	uint8 index = 0;

	if(SoConId >= DEF_MAX_SOCONID_NUM)
		return E_NOT_OK;
	else 
	{
		for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++)
		{
			if(SoConId == SoAd_Pdu2SoCon_Tx[index].SoConId \
					&& SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_OFFLINE) 
			{	
				if(TcpIp_ReleaseIpAddrAssignment(SoAd_Pdu2SoCon_Tx[index].LocalAddrId ) == E_NOT_OK)
					return E_NOT_OK;
				SoAd_RequestIpAddrAssignment_Lock = UnLocked;
				break;
			}
		}
	}
	return E_OK;
}

Std_ReturnType SoAd_GetLocalAddr
(
 SoAd_SoConIdType SoConId, TcpIp_SockAddrType* LocalAddrPtr, 
 uint8* NetmaskPtr, TcpIp_SockAddrType* DefaultRouterPtr 
)
{
	uint8 index = 0;

	if(SoConId >= DEF_MAX_SOCONID_NUM)
		return E_NOT_OK;
	else 
	{
		for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++)
		{
			if(SoConId == SoAd_Pdu2SoCon_Tx[index].SoConId \
					&& (SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY \
						|| SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_ONLINE))
			{
				if(TcpIp_GetIpAddr(SoAd_Pdu2SoCon_Tx[index].LocalAddrId, LocalAddrPtr, 
							NetmaskPtr, DefaultRouterPtr) == E_NOT_OK)
					return E_NOT_OK;
				break;
			}
		}
	}
	return E_OK;
}

Std_ReturnType SoAd_GetPhysAddr(SoAd_SoConIdType SoConId, uint8* PhysAddrPtr ) 
{
	if(TcpIp_GetPhysAddr(SoConId, PhysAddrPtr) == E_NOT_OK)
		return E_NOT_OK;

	if(!PhysAddrPtr)
		return E_NOT_OK;

	return E_OK;
}

Std_ReturnType SoAd_GetRemoteAddr(SoAd_SoConIdType SoConId, TcpIp_SockAddrType* IpAddrPtr ) 
{

	if(SoConId >= DEF_MAX_SOCONID_NUM)
		return E_NOT_OK;
	else 
	{	
		if(SoAd_TcpConnected(SoConId, IpAddrPtr) == E_NOT_OK)
			return E_NOT_OK;
	}
	return E_OK;
}

Std_ReturnType SoAd_SetRemoteAddr(SoAd_SoConIdType SoConId, const TcpIp_SockAddrType* RemoteAddrPtr )
{
	return E_OK;
}

Std_ReturnType SoAd_SetUniqueRemoteAddr
(
 SoAd_SoConIdType SoConId, const TcpIp_SockAddrType* RemoteAddrPtr,
 SoAd_SoConIdType* AssignedSoConIdPtr 
 )
{
	return E_OK;
}

void SoAd_ReleaseRemoteAddr(SoAd_SoConIdType SoConId )
{
}

/*============================== SoAd communication. ====================================*/

Std_ReturnType SoAd_IfTransmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr ) 
{
	return E_OK;
}

Std_ReturnType SoAd_IfRoutingGroupTransmit(SoAd_RoutingGroupIdType id) 
{

	return E_OK;
}

Std_ReturnType SoAd_IfSpecificRoutingGroupTransmit(SoAd_RoutingGroupIdType id, SoAd_SoConIdType SoConId ) 
{
	return E_OK;
}

Std_ReturnType SoAd_TpTransmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr ) 
{
	uint8 *DataPtr; 
	PduLengthType availableDataLength;

	if(!PduInfoPtr)
		return E_NOT_OK;
	
	if(TxPduId != DEF_TXPDUID0 && TxPduId != DEF_TXPDUID1)
		return E_NOT_OK;

	/*Initialize the param. */
	DataPtr = PduInfoPtr->SduDataPtr;
	availableDataLength = PduInfoPtr->SduLength;

	if(TxPduId == DEF_TXPDUID0)
	{
		/* Do TcpTransmition Job. */
		if(availableDataLength <= GLOBAL_DATASRV_BUFSIZE) 
		{
			memcpy(TempTcpTxBuffer, DataPtr, availableDataLength);
			TempTcpTxBuffer += availableDataLength;
			TcpTxBufferLength += availableDataLength;
			SoAd_TcpTransmition_Local_Lock = UnLocked;
		}
		else {
			memcpy(TcpTxBuffer, DataPtr, GLOBAL_DATASRV_BUFSIZE);
			TcpTxBufferLength = GLOBAL_DATASRV_BUFSIZE;
			SoAd_TcpTransmition_Lock = UnLocked;
		}
	}
	else {
		/* Do UdpTransmition Job. */
		if(availableDataLength <= GLOBAL_DATASRV_BUFSIZE) 
		{
			memcpy(TempUdpTxBuffer, DataPtr, availableDataLength);
			TempUdpTxBuffer += availableDataLength;
			UdpTxBufferLength += availableDataLength;
			SoAd_UdpTransmition_Local_Lock = UnLocked;
		}
		else {
			memcpy(UdpTxBuffer, DataPtr, GLOBAL_DATASRV_BUFSIZE);
			UdpTxBufferLength = GLOBAL_DATASRV_BUFSIZE;
			SoAd_UdpTransmition_Lock = UnLocked;
		}
	}
	return E_OK;
}

Std_ReturnType SoAd_TpCancelTransmit(PduIdType TxPduId )
{
	uint8 index = 0;

	if(TxPduId != DEF_TXPDUID0 && TxPduId != DEF_TXPDUID1)
		return E_NOT_OK;
	
	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++)
	{
		if(TxPduId == SoAd_Pdu2SoCon_Tx[index].PduId \
				&& (SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY \
				|| SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_RECONNECT\
				|| SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_ONLINE))
		{
			 if(SoAd_CloseSoCon(SoAd_Pdu2SoCon_Tx[index].SoConId, FALSE) == E_NOT_OK) 
			 	return E_NOT_OK;
			 break;
		}
	}	
	return E_OK;
}

Std_ReturnType SoAd_TpCancelReceive(PduIdType RxPduId ) 
{
	uint8 index = 0;

	if(RxPduId != DEF_RXPDUID0)
		return E_NOT_OK;
	
	for(index = 0; index < DEF_PDU2SOCON_RX_NUM; index++)
	{
		if(RxPduId == SoAd_Pdu2SoCon_Rx[index].PduId \
				& (SoAd_Pdu2SoCon_Rx[index].SoConMode == SOAD_SOCON_READY \
				|| SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_RECONNECT \
				|| SoAd_Pdu2SoCon_Rx[index].SoConMode == SOAD_SOCON_ONLINE))
		{
			if(SoAd_CloseSoCon(SoAd_Pdu2SoCon_Rx[index].SoConId, FALSE) == E_NOT_OK)
				return E_NOT_OK;
			break;
		}
	}	
	return E_OK;
}

/*============================== SoAd configuaration. ====================================*/

Std_ReturnType SoAd_TpChangeParameter(PduIdType id, TPParameterType parameter, uint16 value )
{
	return E_OK;
}

Std_ReturnType SoAd_ReadDhcpHostNameOption( SoAd_SoConIdType SoConId, uint8* length, uint8* data )
{
	return E_OK;
}

Std_ReturnType SoAd_WriteDhcpHostNameOption( SoAd_SoConIdType SoConId, uint8 length, const uint8* data )
{
	return E_OK;
}

Std_ReturnType SoAd_GetAndResetMeasurementData
(
 SoAd_MeasurementIdxType MeasurementIdx, 
 BOOL MeasurementResetNeeded, uint32* MeasurementDataPtr 
)
{
	return E_OK;
}

/*---------------------------------------------------------------------------*\
|                           Local Function Definitions                        |
\*---------------------------------------------------------------------------*/

static void  SoAd_Pdu2SoCon_Tx_Init(void) 
{
	uint8 index = 0;

	if(SoAd_Pdu2SoCon_Tx_Lock == UnLocked) 
	{
		for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
		{
			bzero(&(SoAd_Pdu2SoCon_Tx[index]), sizeof(SoAd_Pdu2SoCon_Type));
			SoAd_Pdu2SoCon_Tx[index] = SoAd_Pdu2SoCon_Tx_cfg[index];
		}
		SoAd_Pdu2SoCon_Tx_Lock = Locked;
	}
}

static void  SoAd_Pdu2SoCon_Rx_Init(void) 
{
	uint8 index = 0;

	if(SoAd_Pdu2SoCon_Rx_Lock == UnLocked) 
	{
		for(index = 0; index < DEF_PDU2SOCON_RX_NUM; index++)
		{
			bzero(&(SoAd_Pdu2SoCon_Rx[index]), sizeof(SoAd_Pdu2SoCon_Type));
			SoAd_Pdu2SoCon_Rx[index] = SoAd_Pdu2SoCon_Rx_cfg[index];
		}
		SoAd_Pdu2SoCon_Rx_Lock = Locked;
	}
}

static Std_ReturnType SoAd_GetSoConId_Local(PduIdType TxPduId, SoAd_SoConIdType* SoConIdPtr )
{
	TcpIp_ProtocolType Protocol;
	uint8 index = 0;

	if(TxPduId != DEF_TXPDUID0 && TxPduId != DEF_TXPDUID1)
		return E_NOT_OK;

	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++)
	{
		if(TxPduId == SoAd_Pdu2SoCon_Tx[index].PduId) 
		{
			Protocol = SoAd_Pdu2SoCon_Tx[index].Protocol;

			if(TcpIp_SoAdGetSocket(TCPIP_AF_INET, Protocol, SoConIdPtr) == E_NOT_OK) 
			{
				errorlog("%s LINE %u: TcpIp_SoAdGetSocket failed !!!\n", __func__, __LINE__);
				if(!SoConIdPtr)
					SoAd_CloseSoCon(*SoConIdPtr, TRUE);
				return E_NOT_OK;
			}

			if(*SoConIdPtr > DEF_MAX_SOCONID_NUM)
				return E_NOT_OK;
			
			/* Do as much as initialization. */
			SoAd_Pdu2SoCon_Tx[index].SoConId = *SoConIdPtr;
			SoAd_Pdu2SoCon_Tx[index].SoConMode = SOAD_SOCON_READY; 
			break;
		}
	}
	
	return E_OK;
}

static void SoAd_DoPreparationForConnectionJob
(
 SoAd_SoConIdType *TcpSoConIdPtr,
 SoAd_SoConIdType *UdpSoConIdPtr,
 SoAd_SoConIdType *SocketIdConnectedPtr, 
 TcpIp_LocalAddrIdType *LocalAddrIdPtr, 
 TcpIp_SockAddrType *RemoteAddrPtr,
 uint8 *BufPtr
)
{
	uint8 index = 0;
	
	/* Open the fdsession. */
	fdOpenSession(TaskSelf());
	
	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
	{
		if(TRUE == SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain \
				&& (SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY) \
				&& SoAd_Pdu2SoCon_Tx[index].Protocol == TCPIP_IPPROTO_TCP)
		{
			*LocalAddrIdPtr = SoAd_Pdu2SoCon_Tx[index].LocalAddrId;
			*TcpSoConIdPtr = SoAd_Pdu2SoCon_Tx[index].SoConId;
			break;
		}
	}
	
	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
	{
		if(TRUE == SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain \
				&& (SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY
				|| SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_RECONNECT) \
				&& SoAd_Pdu2SoCon_Tx[index].Protocol == TCPIP_IPPROTO_UDP)
		{
			*LocalAddrIdPtr = SoAd_Pdu2SoCon_Tx[index].LocalAddrId;
			*UdpSoConIdPtr = SoAd_Pdu2SoCon_Tx[index].SoConId;
			
			if(SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_RECONNECT)
			{
				SoAd_Pdu2SoCon_Tx[index].SoConMode = SOAD_SOCON_READY;
			}
			break;
		}
	}
	
	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
	{
		if(*TcpSoConIdPtr == SoAd_Pdu2SoCon_Tx[index].SoConId \
				&& SoAd_Pdu2SoCon_Tx[index].Protocol == TCPIP_IPPROTO_TCP \
					&& SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY)
		{
			if(SoAd_DoTcpConnectJob(TcpSoConIdPtr, SocketIdConnectedPtr, LocalAddrIdPtr, RemoteAddrPtr, BufPtr) == E_NOT_OK)
			{
				errorlog("%s LINE %u: Failed to SoAd_DoTcpConnectJob !!!\n", __func__, __LINE__);
				break;
			}
			break;
		}
	}

	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
	{
		if(*UdpSoConIdPtr == SoAd_Pdu2SoCon_Tx[index].SoConId \
				&& SoAd_Pdu2SoCon_Tx[index].Protocol == TCPIP_IPPROTO_UDP \
				&& SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY) 
		{
			if(SoAd_DoUdpConnectJob(UdpSoConIdPtr, LocalAddrIdPtr, RemoteAddrPtr, BufPtr) == E_NOT_OK) 
			{
				errorlog("%s LINE %u: Failed to SoAd_DoUdpConnectJob !!!\n", __func__, __LINE__);
				break;
			}
			break;
		}
	}	
	return;
}

static Std_ReturnType SoAd_DoTcpConnectJob
(
 SoAd_SoConIdType *SoConIdPtr,
 SoAd_SoConIdType *SocketIdConnectedPtr, 
 TcpIp_LocalAddrIdType *LocalAddrIdPtr, 
 TcpIp_SockAddrType *RemoteAddrPtr,
 uint8 *BufPtr
)
{
	uint8 index = 0, Status = E_NOT_OK;
	uint16 Port =  htons(DEF_PORT);
	TcpIp_SocketIdType SocketIdConnected = DEF_SOCONID;
	uint16 Length = DEF_PDU_LENGTH;
    
	if(TcpState==DIAG_TX_SERVER_CLOSED)	
	{	
		sint32 Option = 1;
		if(TcpIp_SetSockOpt(*SoConIdPtr, SOL_SOCKET, SO_REUSEPORT, (void *) &Option, sizeof(Option)) == E_NOT_OK)
		{
			PR_DEBUG(TRUE, "Failed to TcpIp_SetSockOpt for SoConId_%u SO_REUSEPORT !!! \n", *SoConIdPtr);
			SoAd_CloseSoCon(*SoConIdPtr, TRUE);
			return E_NOT_OK;
		}
		
		Option = 1;
		if(TcpIp_SetSockOpt(*SoConIdPtr, SOL_SOCKET, SO_REUSEADDR, (void *) &Option, sizeof(Option)) == E_NOT_OK)
		{
			PR_DEBUG(TRUE, "Failed to TcpIp_SetSockOpt for SoConId_%u SO_REUSEADDR !!! \n", *SoConIdPtr);
			SoAd_CloseSoCon(*SoConIdPtr, TRUE);
			return E_NOT_OK;
		}

		if(TcpIp_Bind(*SoConIdPtr, *LocalAddrIdPtr, &Port) == E_NOT_OK) 
		{
			PR_DEBUG(TRUE, "Failed to TcpIp_TcpBind !!!\n");
			SoAd_CloseSoCon(*SoConIdPtr, TRUE);
			return E_NOT_OK;
		}
		PR_DEBUG(TRUE, "Finish bind diagnostic port(=%u) !!!\n", ntohs(Port));

		if(TcpIp_TcpListen(*SoConIdPtr, DEF_MAX_CHANNEL) == E_NOT_OK)
		{
			PR_DEBUG(TRUE, "Failed to TcpIp_TcpListen!\n");
			SoAd_CloseSoCon(*SoConIdPtr, TRUE);
			return E_NOT_OK;
		}
		else
		{
			TcpState=DIAG_TX_SERVER_LISTEN;
		}
		PR_DEBUG(TRUE, "Listening diagnostic port(=13400)!!!\n");
	}
	/* Do the connecting job. */
	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
	{
		if(*SoConIdPtr == SoAd_Pdu2SoCon_Tx[index].SoConId \
				&& SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY)
		{	
			if(SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain == TRUE)
			{
				Status = TcpIp_TcpAccept(*SoConIdPtr, &SocketIdConnected, RemoteAddrPtr);
				
				if(Status == E_NOT_OK)
				{
					PR_DEBUG(TRUE, "Failed to accept!\n");
					SoAd_CloseSoCon(*SoConIdPtr, TRUE);
					if(SocketIdConnected) {
						SoAd_CloseSoCon(SocketIdConnected, TRUE);
					}
					break;
				}
				else if (Status == E_OK)
				{
					PR_DEBUG(DEBUG_TCP, "%s:Status is E_OK continue waiting !!! \n", __func__);
				}
				else 
				{
					TcpState=DIAG_TX_SERVER_CONNECTED;
					PR_DEBUG(TRUE, "Diagnostic port(=13400) is connected !!!\n");
					*SocketIdConnectedPtr = SocketIdConnected;
		
					for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
					{
						if(*SoConIdPtr == SoAd_Pdu2SoCon_Tx[index].SoConId \
									&& SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY)
						{					
							/* Set the SoConMod. */
							SoAd_Pdu2SoCon_Tx[index].SoConMode = SOAD_SOCON_ONLINE;
							break;
						}
					}
						
					for(index = 0; index < DEF_PDU2SOCON_RX_NUM; index++) 
					{
						/* Accept the client. */
						if(SoAd_Pdu2SoCon_Rx[index].Protocol == TCPIP_IPPROTO_TCP \
							&& SoAd_Pdu2SoCon_Rx[index].SoConMode == SOAD_SOCON_OFFLINE)
						{
							SoAd_Pdu2SoCon_Rx[index].SoConId = SocketIdConnected;
							SoAd_Pdu2SoCon_Rx[index].GlobalStarOfMain = TRUE;
							SoAd_Pdu2SoCon_Rx[index].SoConMode = SOAD_SOCON_ONLINE;				
							break;
						}
					}
	
					/* Notic the upper layer. */
					DoIP_SoConModeChg(*SoConIdPtr, SOAD_SOCON_ONLINE);
	
					if(SocketIdConnected)
					{
						PR_DEBUG(DEBUG_TCP, "SocketIdConnected = %u\n", SocketIdConnected);
						break;
					}
				}
			}
			/* Only check first available. */
			break;
		}
	}
	
	if(SoAd_TcpConnected(*SoConIdPtr, RemoteAddrPtr) == E_NOT_OK) 
	{
		PR_DEBUG(TRUE, "LINE %u£ºFailed to SoAd_TcpConnect!\n", __LINE__);
		return E_NOT_OK;
	}
	else
	{
		/* Make preparation for Reception. */

		/* Set the acceptFd action. */
		{
			static struct timeval timeout;
			
			#if ENABLE_100MS
			/* Configure our timeout to be x microseconds */
			timeout.tv_sec = 0;
			timeout.tv_usec = 100000; //100ms
			#else
			timeout.tv_sec = 1;
			timeout.tv_usec = 0;
			#endif
				
			TcpIp_SetSockOpt(SocketIdConnected, SOL_SOCKET,  SO_SNDTIMEO, &timeout, sizeof( timeout ));
			TcpIp_SetSockOpt(SocketIdConnected, SOL_SOCKET,  SO_RCVTIMEO, &timeout, sizeof( timeout ));	
		}
		
	#if ENABLE_DEBUG
		/* Try to get the server physical address. */
		{	
			static uint8 PhysAddr[6] = "";
			if(SoAd_GetPhysAddr(*SoConIdPtr, PhysAddr) == E_NOT_OK)
			{
				PR_DEBUG(TRUE, "Failed to GetPhysAddr at %u !!!\n", __LINE__);
			}
		}

		for(;;)
		{
			if(TcpIp_TcpReceived(SocketIdConnected, BufPtr, Length) == E_NOT_OK)
			{	
				continue;	
			} 		
			if(strlen((const char *)BufPtr))
			{
				PR_DEBUG(DEBUG_TCP, "TCP FIRST BUFFER: %s\n", BufPtr);
				break;
			} 
		}
	#else
		{
			uint8 count = 0;
			
			for(;;)
			{
				if(TcpIp_TcpReceived(SocketIdConnected, BufPtr, Length) == E_NOT_OK)
				{	
					if(count == 5)
					{
						break;
					}
					count++;
					continue;
				}
				if(strlen((const char *)BufPtr))
				{
					PR_DEBUG(DEBUG_TCP, "TCP FIRST BUFFER: %s\n", BufPtr);
					break;
				} 
					
			}
		}
	#endif
	}	
	return E_OK;
}

static Std_ReturnType SoAd_DoUdpConnectJob
(
 SoAd_SoConIdType *SoConIdPtr,
 TcpIp_LocalAddrIdType *LocalAddrIdPtr, 
 TcpIp_SockAddrType *RemoteAddrPtr,
 uint8 *BufPtr
)
{
	uint16 Port = htons(DEF_PORT);
	uint8 index = 0;
	uint16 Length = DEF_PDU_LENGTH;  

	{
		static sint32 Option = 1;
		if(TcpIp_SetSockOpt(*SoConIdPtr, SOL_SOCKET, SO_REUSEADDR, (void *) &Option, sizeof(Option)) == E_NOT_OK)
		{
			PR_DEBUG(TRUE, "Failed to TcpIp_SetSockOpt for SoConId_%u !!! \n", *SoConIdPtr);
			SoAd_CloseSoCon(*SoConIdPtr, TRUE);
			return E_NOT_OK;
		}

		Option = 1;
		if(TcpIp_SetSockOpt(*SoConIdPtr, SOL_SOCKET, SO_REUSEPORT, (void *) &Option, sizeof(Option)) == E_NOT_OK)
		{
			PR_DEBUG(TRUE, "Failed to TcpIp_SetSockOpt for SoConId_%u SO_REUSEPORT !!! \n", *SoConIdPtr);
			SoAd_CloseSoCon(*SoConIdPtr, TRUE);
			return E_NOT_OK;
		}
	}
	
	/* Set the acceptFd action. */
	{
		static struct timeval timeout;
			
		#if ENABLE_100MS
		/* Configure our timeout to be x microseconds */
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000; //100ms
		#else
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		#endif
				
		TcpIp_SetSockOpt(*SoConIdPtr, SOL_SOCKET,  SO_SNDTIMEO, &timeout, sizeof( timeout ));
		TcpIp_SetSockOpt(*SoConIdPtr, SOL_SOCKET,  SO_RCVTIMEO, &timeout, sizeof( timeout ));	
	}

#if ENABLE_DEBUG			
	/* Try to get the server physical address. */
	{	
		static uint8 PhysAddr[6] = "";
		if(SoAd_GetPhysAddr(*SoConIdPtr, PhysAddr) == E_NOT_OK)
		{
			PR_DEBUG(TRUE, "Failed to GetPhysAddr at %u !!!\n", __LINE__);
		}
	}
#endif

	if(TcpIp_Bind(*SoConIdPtr, *LocalAddrIdPtr, &Port) == E_NOT_OK) 
	{
		PR_DEBUG(TRUE, "Failed to TcpIp_UdpBind!\n");
		SoAd_CloseSoCon(*SoConIdPtr, TRUE);
		return E_NOT_OK;
	}
	
	PR_DEBUG(TRUE, "SoAd bind with the port %u successfully !!!\n", ntohs(Port));

	/* Do the connecting job. */
	for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
	{
		if(*SoConIdPtr == SoAd_Pdu2SoCon_Tx[index].SoConId \
					&& SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY)
		{
			while(SoAd_Pdu2SoCon_Tx[index].GlobalStarOfMain == TRUE)
			{
				for(;;)
				{
					SoAd_RxIndication(*SoConIdPtr, RemoteAddrPtr, BufPtr, Length);
					
					if(strlen((const char *)BufPtr)) 
					{						
					#if ENABLE_DEBUG					
						{	
							/* Get the client ipaddr. */
							char String[20] = "";
							TcpIp_SockAddrInetType *IPADDR = (TcpIp_SockAddrInetType *)RemoteAddrPtr;
							IPN NetIpaddr = (IPN)IPADDR->addr.s_addr;
						
							NetIpaddr = htonl(NetIpaddr);
							snprintf(String, sizeof(String), "%d.%d.%d.%d",
									(UINT8)((NetIpaddr>>24)&0xFF), (UINT8)((NetIpaddr>>16)&0xFF),
									(UINT8)((NetIpaddr>>8)&0xFF), (UINT8)(NetIpaddr&0xFF)); 		
							PR_DEBUG(TRUE, "SoAd_RxIndication succeded to connect client:%s !!!\n", String);
						}
					#endif
					
						for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
						{
							if(*SoConIdPtr == SoAd_Pdu2SoCon_Tx[index].SoConId \
									&& SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY) 
							{	
								/* Set the SoConMod. */
								SoAd_Pdu2SoCon_Tx[index].SoConMode = SOAD_SOCON_ONLINE;
								break;
							}
						}
						
						for(index = 0; index < DEF_PDU2SOCON_RX_NUM; index++) 
						{
							/* Accept the client. */
							if(SoAd_Pdu2SoCon_Rx[index].Protocol == TCPIP_IPPROTO_UDP \
								&& SoAd_Pdu2SoCon_Rx[index].SoConMode == SOAD_SOCON_OFFLINE)
							{
								SoAd_Pdu2SoCon_Rx[index].GlobalStarOfMain = TRUE;
								SoAd_Pdu2SoCon_Rx[index].SoConMode = SOAD_SOCON_ONLINE;
								break;
							}	
						}      
						DoIP_SoConModeChg(*SoConIdPtr, SOAD_SOCON_ONLINE);							
						break;
					}
					else {
                        
					#if ENABLE_DEBUG
						continue;
					#else
						uint8 count = 0;

						if(count == 5)
						{
							for(index = 0; index < DEF_PDU2SOCON_TX_NUM; index++) 
							{
								if(*SoConIdPtr == SoAd_Pdu2SoCon_Tx[index].SoConId \
										&&(SoAd_Pdu2SoCon_Tx[index].SoConMode == SOAD_SOCON_READY)) 
								{	
									/* Set the SoConMod. */
									SoAd_Pdu2SoCon_Tx[index].SoConMode = SOAD_SOCON_ONLINE;
									break;
								}
							}
							break;
						}
						count++;
						continue;
					#endif				
					}
				}
				/* Only check first pdu available. */
				break;
			}
			break;
		}
	}
	return E_OK;
}

static void SoAd_static_tx_buffer_init(void)
{
TxBuffer_Alloc:	
	if(Local_TxBuffer_Lock == UnLocked) 
	{
		TcpTxBuffer = (uint8 *)Utils_memAlloc( \
							UTILS_HEAPID_DDR_CACHED_SR, 
							GLOBAL_DATASRV_BUFSIZE,
							16);
		if(!TcpTxBuffer) {
			PR_DEBUG(TRUE, "TcpTxBuffer: allocation failed, sending constant size\n");
			goto Error_Alloc_TxBuffer;
		}

		UdpTxBuffer = (uint8 *)Utils_memAlloc( \
							UTILS_HEAPID_DDR_CACHED_SR, 
							GLOBAL_DATASRV_BUFSIZE,
							16);
		if(!UdpTxBuffer) {
			PR_DEBUG(TRUE, "UdpTxBuffer: allocation failed, sending constant size\n");
			goto Error_Alloc_TxBuffer;
		}
		
		bzero(TcpTxBuffer, GLOBAL_DATASRV_BUFSIZE);
		TempTcpTxBuffer = TcpTxBuffer;
		bzero(UdpTxBuffer, GLOBAL_DATASRV_BUFSIZE);
		TempUdpTxBuffer = UdpTxBuffer;
		
		Local_TxBuffer_Lock = Locked;
	}
	return;
	
Error_Alloc_TxBuffer:
	if(TcpTxBuffer) 
	{
		Utils_memFree( \
				UTILS_HEAPID_DDR_CACHED_SR,
				TcpTxBuffer, 
				GLOBAL_DATASRV_BUFSIZE);
		TcpTxBuffer = NULL;
	}

	if(UdpTxBuffer) 
	{
		Utils_memFree( \
				UTILS_HEAPID_DDR_CACHED_SR,
				UdpTxBuffer, 
				GLOBAL_DATASRV_BUFSIZE);
		UdpTxBuffer = NULL;
	}
	
	Local_TxBuffer_Lock = UnLocked;	
	goto TxBuffer_Alloc;
}

static void SoAd_TcpTransmitionJob
(
 SoAd_SoConIdType *AcceptIdPtr, 
 TcpIp_SockAddrType *RemoteAddrPtr
)
{
	/* DoIP_SoAdTpCopyTxData parameters. */
	PduInfoType info;
	RetryInfoType retry;
	PduLengthType availableDataLength = 0;
	PduLengthType TotalDataLength = 0;
	BufReq_ReturnType RetVal = BUFREQ_E_NOT_OK;

	bzero(&info, sizeof(info));
	bzero(&retry, sizeof(retry));
	
	/* Get the Total availableDataLength. */
	{
		info.SduDataPtr = NULL;
		info.SduLength = 0;

		RetVal = DoIP_SoAdTpCopyTxData(DEF_TXPDUID0, &info, &retry, &availableDataLength);
		
		switch(RetVal) {
			case BUFREQ_E_NOT_OK:
				return;
			case BUFREQ_OK:
				TotalDataLength = availableDataLength;
				break;
			default:
				return;
		}
	}
	
	for(;;)
	{	
		if(TcpTxBuffer)
		{
			bzero(TcpTxBuffer, GLOBAL_DATASRV_BUFSIZE);
		}
		
		if(TotalDataLength >= GLOBAL_DATASRV_BUFSIZE) 
		{
			info.SduDataPtr = TcpTxBuffer;
			info.SduLength = GLOBAL_DATASRV_BUFSIZE;
		
			if(retry.TpDataState != TP_DATARETRY)
				TotalDataLength -= GLOBAL_DATASRV_BUFSIZE;
		}
		else {
			info.SduDataPtr = TcpTxBuffer;
			info.SduLength = TotalDataLength;
			retry.TpDataState = TP_DATACONF;
		}
		
		RetVal = DoIP_SoAdTpCopyTxData(DEF_TXPDUID0, &info, &retry, &availableDataLength);

		if(availableDataLength != TotalDataLength)
		{
			TotalDataLength = availableDataLength;
		}
		
		if(RetVal ==  BUFREQ_E_NOT_OK)
		{	
			if(retry.TpDataState == TP_DATACONF) 
			{
				DoIP_SoAdTpTxConfirmation(DEF_TXPDUID0, E_OK);
				break;
			}		
			DoIP_SoAdTpTxConfirmation(DEF_TXPDUID0, E_NOT_OK);
			break;
		}
		else if(RetVal == BUFREQ_OK)
		{			
			TcpIp_TcpTransmit(*AcceptIdPtr, TcpTxBuffer, GLOBAL_DATASRV_BUFSIZE, RemoteAddrPtr, FALSE);
			if(retry.TpDataState == TP_DATACONF) 
			{
				DoIP_SoAdTpTxConfirmation(DEF_TXPDUID0, E_OK);
				break;
			}
			continue;
		}
		else if(RetVal == BUFREQ_E_BUSY)
		{	
			/* Change the state of the retry param. */
			retry.TpDataState = TP_DATARETRY;
			continue;
		}
		else {		
			/* do nothing*/
			break;
		}
	}	
}

static Std_ReturnType SoAd_TcpCommunicationJob
(
 SoAd_SoConIdType *AcceptIdPtr, TcpIp_SockAddrType *RemoteAddrPtr,
 PduInfoType *info, PduLengthType* bufferSizePtr
)
{
	PduLengthType bufferSize;
	uint8 *TcpDataPtr = BufPtr_TcpRecv;
	uint8 *TempTcpDataPtr;
	uint16 RxLength = DEF_PDU_LENGTH, TempLength = 0;
	PduLengthType TpSduLength = 0;
	uint8 count = 0;

	if(*AcceptIdPtr >= DEF_MAX_SOCONID_NUM)
		return E_NOT_OK;
	
	if(!info || !bufferSizePtr)
	{
		PR_DEBUG(TRUE, "%s LINE %u info & bufferSizePtr are NULL !!!\n", \
							__func__, __LINE__);
		return E_NOT_OK;
	}

	/*========================= Transmission. =======================*/
	if(SoAd_TcpTransmition_Local_Lock == UnLocked)
	{
		if( (TcpTxBufferLength <= GLOBAL_DATASRV_BUFSIZE) && TcpTxBuffer )
		{
			TcpIp_TcpTransmit(*AcceptIdPtr, TcpTxBuffer, TcpTxBufferLength, RemoteAddrPtr, FALSE);
			DoIP_SoAdTpTxConfirmation(DEF_TXPDUID0, E_OK);
			SoAd_TcpTransmition_Local_Lock = Locked;
		}
	}
	
	if(SoAd_TcpTransmition_Lock == UnLocked)
	{
		SoAd_TcpTransmitionJob(AcceptIdPtr, RemoteAddrPtr);
		SoAd_TcpTransmition_Lock = Locked;
	}

	/* After finish Flush the TcpTxBuffer. */
	if(TcpTxBuffer && TcpTxBufferLength)
	{
		bzero(TcpTxBuffer, GLOBAL_DATASRV_BUFSIZE);
		TempTcpTxBuffer = TcpTxBuffer;
		TcpTxBufferLength = 0;
	}

    /* Bzero the global BufPtr_TcpRecv. */
    bzero(BufPtr_TcpRecv, DEF_PDU_LENGTH);
    
	for(;;)
	{
		/*=========================Reception.=======================*/
		if(TcpDataPtr && RxLength)
		{
			/* Receive and Copy data from TcpIp stack. */
		#if ENABLE_NOWAIT
			if(TcpIp_TcpReceive(*AcceptIdPtr, TcpDataPtr, &RxLength) == E_NOT_OK)
			{	
				count++;
				if(count == 5) {
					break;
				}
				continue;
			}
		#else
			if(TcpIp_TcpReceive(*AcceptIdPtr, TcpDataPtr, &RxLength) == E_NOT_OK)
				continue;
		#endif

			/* Start reception when each time copy. */
			info->SduDataPtr = TcpDataPtr;
			info->SduLength = TpSduLength = RxLength;
			(void)DoIP_SoAdTpStartOfReception(DEF_RXPDUID0, info, TpSduLength, bufferSizePtr);

			bufferSize = *bufferSizePtr;

			if(bufferSize >= RxLength) 
			{
				if(DoIP_SoAdTpCopyRxData(DEF_RXPDUID0, info, &bufferSize) == BUFREQ_E_NOT_OK)
				{
					PR_DEBUG(TRUE, "LINE %u: DoIP_SoAdTpCopyRxData fail !!!\n", __LINE__);
				}
				else {
					PR_DEBUG(DEBUG_TCP, "LINE %u : bufferSize = %u\n", __LINE__, bufferSize);
				}
			}
			else {
				/* Each time init the TempTcpDataPtr. */
				TempTcpDataPtr = TcpDataPtr;
				TempLength = RxLength;
			
				/* Copy the data to DoIP. */
				while(TempLength)
				{				
					/* Check the illegal circumstance. */
					if(bufferSize == 0)
						break;
					
					if(TempTcpDataPtr <= TcpDataPtr + RxLength - bufferSize)
					{
						info->SduDataPtr = TempTcpDataPtr;
						info->SduLength = bufferSize;
					
						/* Copy the data to DoIP layer. */
						if(DoIP_SoAdTpCopyRxData(DEF_RXPDUID0, info, &bufferSize) == BUFREQ_E_NOT_OK)
						{
							PR_DEBUG(TRUE, "LINE %u: DoIP_SoAdTpCopyRxData fail !!!\n", __LINE__);
							break;
						}
						
						TempTcpDataPtr += bufferSize;
						TempLength -= bufferSize;
					}
					else {
						TempLength = TcpDataPtr + RxLength - TempTcpDataPtr;
						info->SduDataPtr = TempTcpDataPtr;
						info->SduLength = bufferSize = TempLength;

						if(DoIP_SoAdTpCopyRxData(DEF_RXPDUID0, info, &bufferSize) == BUFREQ_E_NOT_OK)
						{
							PR_DEBUG(TRUE, "LINE %u: DoIP_SoAdTpCopyRxData fail !!!\n", __LINE__);
							break;
						}
						break;
					}
				}					
			}			
			/* Finish receiving one time . */
			DoIP_SoAdTpRxIndication(DEF_RXPDUID0, E_OK);
		}
		
		/* Finish one time reception/transmistion . */
		break;
	}
	
	//PR_DEBUG(DEBUG_TCP, "%s reached end returned E_OK !!!\n", __func__);
	return E_OK;
}

static void SoAd_UdpTransmitionJob
(
 SoAd_SoConIdType *SoConIdPtr, 
 TcpIp_SockAddrType *RemoteAddrPtr
)
{
	/* DoIP_SoAdTpCopyTxData parameters. */
	PduInfoType info;
	RetryInfoType retry;
	PduLengthType availableDataLength = 0;
	PduLengthType TotalDataLength = 0;
	BufReq_ReturnType RetVal = BUFREQ_E_NOT_OK;

	bzero(&info, sizeof(info));
	bzero(&retry, sizeof(retry));
	
	/* Get the Total availableDataLength. */
	{
		info.SduDataPtr = NULL;
		info.SduLength = 0;

		RetVal = DoIP_SoAdTpCopyTxData(DEF_TXPDUID1, &info, &retry, &availableDataLength);
		
		switch(RetVal) {
			case BUFREQ_E_NOT_OK:
				return;
			case BUFREQ_OK:
				TotalDataLength = availableDataLength;
				break;
			default:
				return;
		}
	}
	
	for(;;)
	{	
		if(UdpTxBuffer)
		{
			bzero(UdpTxBuffer, GLOBAL_DATASRV_BUFSIZE);
		}
		
		if(TotalDataLength >= GLOBAL_DATASRV_BUFSIZE) 
		{
			info.SduDataPtr = UdpTxBuffer;
			info.SduLength = GLOBAL_DATASRV_BUFSIZE;
		
			if(retry.TpDataState != TP_DATARETRY)
				TotalDataLength -= GLOBAL_DATASRV_BUFSIZE;
		}
		else {
			info.SduDataPtr = UdpTxBuffer;
			info.SduLength = TotalDataLength;
			retry.TpDataState = TP_DATACONF;
		}
		
		RetVal = DoIP_SoAdTpCopyTxData(DEF_TXPDUID1, &info, &retry, &availableDataLength);

		/* Correction */
		if(availableDataLength != TotalDataLength)
		{
			TotalDataLength = availableDataLength;
		}
		
		if(RetVal ==  BUFREQ_E_NOT_OK)
		{	
			if(retry.TpDataState == TP_DATACONF) 
			{
				DoIP_SoAdTpTxConfirmation(DEF_TXPDUID1, E_OK);
				break;
			}
			
			DoIP_SoAdTpTxConfirmation(DEF_TXPDUID1, E_NOT_OK);
			break;
		}
		else if(RetVal == BUFREQ_OK)
		{			
			TcpIp_UdpTransmit(*SoConIdPtr, UdpTxBuffer, RemoteAddrPtr, UdpTxBufferLength);
			if(retry.TpDataState == TP_DATACONF) 
			{
				DoIP_SoAdTpTxConfirmation(DEF_TXPDUID1, E_OK);
				break;
			}
			continue;
		}
		else if(RetVal == BUFREQ_E_BUSY)
		{	
			/* Change the state of the retry param. */
			retry.TpDataState = TP_DATARETRY;
			continue;
		}
		else {		
			/* do nothing*/
			break;
		}
	}	
}

static Std_ReturnType SoAd_UdpCommunicationJob
(
 SoAd_SoConIdType *SoConIdPtr, TcpIp_SockAddrType *RemoteAddrPtr, 
 PduInfoType *info, PduLengthType* bufferSizePtr
)
{
	PduLengthType bufferSize;
	uint8 *UdpDataPtr = BufPtr_UdpRecv;
	uint8 *TempUdpDataPtr;
	uint16 RxLength = DEF_PDU_LENGTH, TempLength = 0;
	PduLengthType TpSduLength = 0;
	uint8 count = 0;
	
	if(*SoConIdPtr >= DEF_MAX_SOCONID_NUM)
		return E_NOT_OK;
	
	if(!info || !bufferSizePtr)
	{
		PR_DEBUG(TRUE, "%s LINE %u info & bufferSizePtr are NULL !!!\n", \
							__func__, __LINE__);
		return E_NOT_OK;
	}

	/*========================= Transmission. =======================*/
	if(SoAd_UdpTransmition_Local_Lock == UnLocked)
	{
		if( (UdpTxBufferLength <= GLOBAL_DATASRV_BUFSIZE) && UdpTxBuffer )
		{
			TcpIp_UdpTransmit(*SoConIdPtr, UdpTxBuffer, RemoteAddrPtr, UdpTxBufferLength);
			DoIP_SoAdTpTxConfirmation(DEF_TXPDUID1, E_OK);
			SoAd_UdpTransmition_Local_Lock = Locked;
		}
	}
		
	if(SoAd_UdpTransmition_Lock == UnLocked)
	{
		SoAd_UdpTransmitionJob(SoConIdPtr, RemoteAddrPtr);
		SoAd_UdpTransmition_Lock = Locked;
	}

	/* After finish Flush the UdpTxBuffer. */
	if(UdpTxBuffer && UdpTxBufferLength)
	{
		bzero(UdpTxBuffer, GLOBAL_DATASRV_BUFSIZE);
		TempUdpTxBuffer = UdpTxBuffer;
		UdpTxBufferLength = 0;
	}

    /* Bzero the global BufPtr_TcpRecv. */
    bzero(BufPtr_UdpRecv, DEF_PDU_LENGTH);
    
	for(;;)
	{
		/*=========================Reception.=======================*/
		if(UdpDataPtr && RxLength)
		{
		#if ENABLE_NOWAIT
			/* Receive and Copy data from TcpIp stack. */
			if(TcpIp_Reception(*SoConIdPtr,	UdpDataPtr, RemoteAddrPtr, &RxLength) == E_NOT_OK)
			{
				count++;
				if(count == 5) {
					break;
				}
				continue;
			}
		#else
			if(TcpIp_Reception(*SoConIdPtr,	UdpDataPtr, RemoteAddrPtr, &RxLength) == E_NOT_OK)
				continue;
		#endif
		
			/* Start reception when each time copy. */
			info->SduDataPtr = UdpDataPtr;
			info->SduLength = TpSduLength = RxLength;
			(void)DoIP_SoAdTpStartOfReception(DEF_RXPDUID1, info, TpSduLength, bufferSizePtr);

			bufferSize = *bufferSizePtr;

			if(bufferSize >= RxLength) 
			{
				if(DoIP_SoAdTpCopyRxData(DEF_RXPDUID1, info, &bufferSize) == BUFREQ_E_NOT_OK)
				{
					PR_DEBUG(TRUE, "LINE %u: DoIP_SoAdTpCopyRxData fail !!!\n", __LINE__);
				}
				else {
					PR_DEBUG(DEBUG_UDP, "LINE %u : bufferSize = %u\n", __LINE__, bufferSize);
				}
			}
			else {
				/* Each time init the TempTcpDataPtr. */
				TempUdpDataPtr = UdpDataPtr;
				TempLength = RxLength;
			
				/* Copy the data to DoIP. */
				while(TempLength)
				{				
					/* Check the illegal circumstance. */
					if(bufferSize == 0)
						break;
													
					if(TempUdpDataPtr <= UdpDataPtr + RxLength - bufferSize)
					{
						info->SduDataPtr = TempUdpDataPtr;
						info->SduLength = bufferSize;
						
						/* Copy the data to DoIP layer. */
						if(DoIP_SoAdTpCopyRxData(DEF_RXPDUID1, info, &bufferSize) == BUFREQ_E_NOT_OK)
						{
							PR_DEBUG(TRUE, "LINE %u: DoIP_SoAdTpCopyRxData fail !!!\n", __LINE__);
							break;
						}
						TempUdpDataPtr += bufferSize;
						TempLength -= bufferSize;
					}
					else {
						TempLength = UdpDataPtr + RxLength - TempUdpDataPtr;
						info->SduDataPtr = TempUdpDataPtr;
						info->SduLength = bufferSize = TempLength;

						if(DoIP_SoAdTpCopyRxData(DEF_RXPDUID1, info, &bufferSize) == BUFREQ_E_NOT_OK)
						{
							PR_DEBUG(TRUE, "LINE %u: DoIP_SoAdTpCopyRxData fail !!!\n", __LINE__);
							break;
						}
						break;
					}
				}					
			}			
			/* Finish receiving one time . */
			DoIP_SoAdTpRxIndication(DEF_RXPDUID1, E_OK);
		}
	
		/* Finish one time reception. */
		break;
	}
	
	return E_OK;
}

/*---------------------------------------------------------------------------*\
|                                 End of File                                 |
\*---------------------------------------------------------------------------*/

