/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : DoIP.c
*   Module Name     : Diag over IP
*   Project         : 
*   Processor       : MC9S12G128MLH
*   Description		: 
*         This file implements the CAN Transport Protocol module initialization, rx message analysis,
*         tx message transmission and fault check.
*   Component		: 
*
*
*
*
************************************************************************************************
*   Revision History:
*
*   Version     Date          Initials      CR#          Descriptions
*   ---------   ----------    -----------   ----------   ---------------
*   1.0         2018/04/19    PangXD        N/A          Original
*
************************************************************************************************
*	END_FILE_HDR*/

/*
** ===================================================================
**     Include files.
** ===================================================================
*/

#include <src/eth_diagnosis/include/Network_DoIP_SoAd_Sys.h>
#include <src/eth_diagnosis_stack/include/Dcm/Dcm.h>
#include <src/eth_diagnosis_stack/include/Dcm/Dcm_Cbk.h>

/*
** ===================================================================
**     Global declaration.
** ===================================================================
*/




/*
** ===================================================================
**     Macro definition.
** ===================================================================
*/

#define ENABLE_TCP   1

#define DOIP_SA 0x798u

#define DOIP_RX_7DF_FUNC   (0)
#define DOIP_RX_790_PHY    (1)
#define DOIP_UPLA_CHANNEL_MAX (2)
#define DOIP_TX_798 (0)

#define  DOIP_BUFFER_LENGTH_MAX  3000
#define  DOIP_UDS_BUFF_MAX	3000

#define  DOIP_ROUT_ACT_NUM   0 //g_stDoIPCfg.DoIPConfigSet[uiLoop].DoIPRoutingActivation.DoIPRoutingActivationNumber

/* Version */
#define INVO_ID					0xffff
#define DOIP_ID					50u/* -- */
#define DOIP_MAJOR_VERSION		0x01u
#define DOIP_MINOR_VERSION		0x00u
#define DOIP_PATCH_VERSION		0x00u

/*Service ID*/
#define	DOIP_TPTRANSMIT						0x49
#define	DOIP_TPCANCELTRANSMIT				0x4a
#define	DOIP_TPCANCELRECEIVE				0x4c
#define	DOIP_IFTRANSMIT						0x49
#define	DOIP_IFCANCELTRANSMIT				0x4a
#define	DOIP_INIT							0x01
#define	DOIP_GETVERSIONINFO					0x00
#define	DOIP_SOADTPCOPYTXDATA				0x43
#define	DOIP_SOADTPTXCONFIRMATION			0x48
#define	DOIP_SOADTPCOPYRXDATA				0x44
#define	DOIP_SOADTPSTARTOFRECEPTION			0x46
#define	DOIP_SOADTPRXINDICATION				0x45
#define	DOIP_SOADIFRXINDICATION				0x42
#define	DOIP_SOADIFTXCONFIRMATION			0x40
#define	DOIP_SOCONMODECHG					0x0b
#define	DOIP_LOCALIPADDRASSIGNMENTCHG		0x0c
#define	DOIP_ACTIVATIONLINESWITCHACTIVE		0x0f
#define	DOIP_ACTIVATIONLINESWITCHINACTIVE	0x0e
#define	DOIP_MAINFUNCTION					0x02
	
/*Development Errors*/
#define	DOIP_E_UNINIT				0x01
#define	DOIP_E_PARAM_POINTER		0x02
#define	DOIP_E_INVALID_PDU_SDU_ID	0x03
#define	DOIP_E_INVALID_PARAMETER	0x04
#define	DOIP_E_INIT_FAILED			0x05

// Generic doip header negative acknowledge codes
#define DOIP_E_H_INCORRECT_PATTERN_FORMAT	0x00
#define DOIP_E_H_UNKNOWN_PAYLOAD_TYPE		0x01
#define DOIP_E_H_MESSAGE_TOO_LARGE			0x02
#define DOIP_E_H_OUT_OF_MEMORY				0x03
#define DOIP_E_H_INVALID_PAYLOAD_LENGTH		0x04

// Diagnostic message negative acknowledge codes
// 0x00 0x01  Reserved by document
#define DOIP_E_DIAG_INVALID_SA			0x02	// Invalid Source Address
#define DOIP_E_DIAG_UNKNOWN_TA			0x03	// Unknown Target Address
#define DOIP_E_DIAG_MESSAGE_TO_LARGE	0x04	// Diagnostic Message too large
#define DOIP_E_DIAG_OUT_OF_MEMORY		0x05	// Out of memory
#define DOIP_E_DIAG_TARGET_UNREACHABLE	0x06	// Target unreachable
#define DOIP_E_DIAG_UNKNOWN_NETWORK		0x07	// Unknown network
#define DOIP_E_DIAG_TP_ERROR			0x08	// Transport protocol error
// 0x09 0xFF  Reserved by document

//Table 25 — Routing activation response code values
#define DOIP_RARC_UNKNOWN_SA   		0x00 //Routing activation denied due to unknown source address.Do not activate routing and close this TCP_DATA socket. mandatory
#define DOIP_RARC_SOC_REGED			0x01 //Routing activation denied because all concurrently supported TCP_DATA sockets are registered and active.Do not activate routing and close thisTCP_DATA socket. mandatory
#define DOIP_RARC_SA_DIFFRNT  		0x02 //Routing activation denied because an SA different from the table connection entry was received on the already activated TCP_DATA socket. Do not activate routing and close this TCP_DATA socket. mandatory
#define DOIP_RARC_SA_REGED   		0x03 //Routing activation denied because the SA is already registered and active on a different TCP_DATA socket. Do not activate routing and close this TCP_DATA socket. mandatory
#define DOIP_RARC_AUTHENTIC_MISS 	0x04 //Routing activation denied due to missing authentication. Do not activate routing and register. optional
#define DOIP_RARC_CONFIRM_REJCT   	0x05 //Routing activation denied due to rejected confrmation. Do not activate routing and close this TCP_DATA socket. optional
#define DOIP_RARC_UNSUPPORT_TYP   	0x06 //Routing activation denied due to unsupported routing activation type. Do not activate routing and close this TCP_DATA socket. mandatory
										 //0x07 – 0x0F //Reserved by this part of ISO 13400. — —
#define DOIP_RARC_SUCCESS   		0x10 //Routing successfully activated. Activate routing and register SA on this TCP_DATA socket. mandatory
#define DOIP_RARC_CONFRM_REQUIRED   0x11 //Routing will be activated; confrmation required. Only activate routing after confrmation from within the vehicle. optional
										 //0x12 – 0xDF //Reserved by this part of ISO 13400. — —
										 //0xE0 – 0xFE //Vehicle-manufacturer specifc. — optional
										 //0xFF //Reserved by this part of ISO 13400. — —
//Msg Index
//Generic Header
#define DOIP_GH_PROTOCOL_VERSION 			0	// Protocol version 
#define DOIP_GH_INVERSE_PROTOCOL_VERSION	1	// Inverse protocol version
#define DOIP_GH_PAYLOAD_TYPE				2	// Payload type
#define DOIP_GH_PAYLOAD_LENGTH				4	// Payload length
#define DOIP_GH_TOTAL						8	// Generic Header Total length
//Msg content  cell length
#define DOIP_OFFSET_NONE			0	// Message no content.
#define DOIP_OFFSET_ACK				1	// ack or nack or response code.
#define DOIP_OFFSET_EID				6	// EID.
#define DOIP_OFFSET_VIN				17	// VIN.
#define DOIP_OFFSET_LA				2	// Logical Address.
#define DOIP_OFFSET_GID				6	// GID.
#define DOIP_OFFSET_FAR				1	// Further action required.
#define DOIP_OFFSET_VGSS			1	// VIN/GID sync. status.
#define DOIP_OFFSET_SA				2	// Source address.
#define DOIP_OFFSET_AT				1	// Activation Type.
#define DOIP_OFFSET_ISO				4	// Reserved by the ISO.	
#define DOIP_OFFSET_OEM				4	// OEM specificGID.
#define DOIP_OFFSET_NT				1	// Node type.
#define DOIP_OFFSET_MCTS			1	// Max.concurrent TCP_DATA sockets.
#define DOIP_OFFSET_NCTS			1	// Currently open TCP_DATA sockets.
#define DOIP_OFFSET_MDS				4	// Max. data size.
#define DOIP_OFFSET_DPM				1	// Diagnostic power mode.
#define DOIP_OFFSET_TA				2	// Target address.

//Payload length
#define DOIP_PL_GNRHEADERNACK		1	//Generic DoIP header negative acknowledge
#define DOIP_PL_VHCLIDFIC_REQMSG  	0		//Vehicle identification request msg
#define DOIP_PL_VHCLIDFIC_REQEID  	6		//Vehicle identification request message with EID
#define DOIP_PL_VHCLIDFIC_REQVIN  	17 		//Vehicle identification request message with VIN
#define DOIP_PL_VHCLIDFIC_RESP		33	//Vehicle announcement message/vehicle identification response message
#define DOIP_PL_VHCLANNOUNCE		33		//Vehicle announcement message/vehicle identification response message
#define DOIP_PL_ROUTACTVTN_REQ_7	7		//Routing activation request
#define DOIP_PL_ROUTACTVTN_REQ_11	11		//Routing activation request
#define DOIP_PL_ROUTACTVTN_RESP_9	9	//Routing activation response
#define DOIP_PL_ROUTACTVTN_RESP_13	13	//Routing activation response
#define DOIP_PL_ALIVECHK_REQ		0	//Alive check request
#define	DOIP_PL_ALIVECHK_RESP		2		//Alive check reponse
#define DOIP_PL_ENTITYSTS_REQ		0		//DoIP entity status request
#define DOIP_PL_ENTITYSTS_RESP		7		//DoIP entity status response
#define DOIP_PL_PWRMODEINFO_REQ		0		//Diagnostic power mode information request
#define DOIP_PL_PWRMODEINFO_RESP	1	//Diagnostic power mode information response
#define DOIP_PL_DIAGMSG_MIN			5	//Diagnostic message	>= 5
#define DOIP_PL_DIAGMSG_ACK_MIN		6	//Diagnostic message positive acknowledgement	>= 6
#define DOIP_PL_DIAGMSG_NACK_MIN	6	//Diagnostic message negative acknowledgement	>= 6

//Payload type                      /* rcv/snd */
#define DOIP_PT_GNRHEADERNACK			0x0000	//Generic DoIP header negative acknowledge
#define DOIP_PT_VHCLIDFIC_REQMSG  	0x0001		//Vehicle identification request msg
#define DOIP_PT_VHCLIDFIC_REQEID  	0x0002		//Vehicle identification request message with EID
#define DOIP_PT_VHCLIDFIC_REQVIN  	0x0003 		//Vehicle identification request message with VIN
#define DOIP_PT_VHCLIDFIC_RESP			0x0004	//Vehicle announcement message/vehicle identification response message
#define DOIP_PT_VHCLANNOUNCE			0x0004	//Vehicle announcement message/vehicle identification response message
#define DOIP_PT_ROUTACTVTN_REQ		0x0005		//Routing activation request (TCP)
#define DOIP_PT_ROUTACTVTN_RESP			0x0006	//Routing activation response (TCP)
#define DOIP_PT_ALIVECHK_REQ			0x0007	//Alive check request (TCP)
#define	DOIP_PT_ALIVECHK_RESP		0x0008		//Alive check reponse (TCP)
#define DOIP_PT_ENTITYSTS_REQ		0x4001		//DoIP entity status request
#define DOIP_PT_ENTITYSTS_RESP			0x4002		//DoIP entity status response
#define DOIP_PT_PWRMODEINFO_REQ		0x4003		//Diagnostic power mode information request
#define DOIP_PT_PWRMODEINFO_RESP		0x4004	//Diagnostic power mode information response
#define DOIP_PT_DIAGMSG				0x8001		//Diagnostic message (TCP)
#define DOIP_PT_DIAGMSG_ACK				0x8002	//Diagnostic message positive acknowledgement (TCP)
#define DOIP_PT_DIAGMSG_NACK			0x8003	//Diagnostic message negative acknowledgement (TCP)

enum
{
	DOIP_STATES_UNINIT = 0,
	DOIP_STATES_INITED	
};
	
enum	//Sockets Connection states	ucConnectionState
{
	DOIP_CONCTS_LISTEN = 0u,
	DOIP_CONCTS_SOCKET_INITIALIZED,
	DOIP_CONCTS_REGISTERED_PEND_AUTHEN,
	DOIP_CONCTS_REGISTERED_PEND_CONFIRM,
	DOIP_CONCTS_REGISTERED_ROUTING_ALIVE,
	DOIP_CONCTS_FINALIZE
};
	
enum 	//routing activation handler
{
	DOIP_ROUTINGACTIVATION_ACCEPTED = 0,//Final socket handler- return to routing activation handler - accepted
	DOIP_ROUTINGACTIVATION_REJECTED,//Exit point to routing activation handler - rejected
	DOIP_ROUTINGACTIVATION_PENDING
};

enum
{
	DOIP_BUFFSTS_TX_IDLE = 0,
	DOIP_BUFFSTS_TX_READY_TO_SEND,
	DOIP_BUFFSTS_TX_COPY_ALLOW
};
	
enum
{
	DOIP_BUFFSTS_RX_IDLE = 0,
	DOIP_BUFFSTS_RX_START,
	DOIP_BUFFSTS_RX_COPY,
	DOIP_BUFFSTS_RX_DIRECT,
};

enum 
{
	DOIP_BUFFTYP_TX = 0,	//DOIP_BUFFTYP_PDUR_TX -> DOIP_BUFFTYP_SOAD_TX
	DOIP_BUFFTYP_RX		//DOIP_BUFFTYP_SOAD_RX -> DOIP_BUFFTYP_PDUR_RX
};

enum  
{
	DOIP_ACTIVATIONTYPE_DEFAULT = 0,
	DOIP_ACTIVATIONTYPE_WWHOBD,
	DOIP_ACTIVATIONTYPE_CENTRALSECURITYOBD = 0xE0
};

enum 
{
	DOIP_ACTIVATION_LINE_INACTIVE = 0,
	DOIP_ACTIVATION_LINE_ACTIVE
};

enum{
	DOIP_SOAD_CONNECTIONS_TCP = 0,
	DOIP_SOAD_CONNECTIONS_UDP,
	DOIP_SOAD_CONNECTIONS_MAX
};

enum
{
	DOIP_LINK_DOWN = 0,
	DOIP_LINK_UP
};

/*
** ===================================================================
**     Type definition.
** ===================================================================
*/
typedef struct 
{
	uint8 Cnt;
	DoIPPduRTxPdu_Type info[255];
}DoIP_PduRTxWaitQueue_Type;

typedef struct {
	uint8 ucPduRTxPduId;/* Addressing type */
	DoIPPduType_Enum DoIPPduType;
	uint8 ucConnectTyp;
}DoIP_PduRChannel_Typ;

typedef struct {
	uint16 uiTA;
	uint8 ucPduRRxPduId;/* Addressing type */
}DoIP_TAAddressing_Typ;

typedef struct
{
	uint8 Status;
	PduLengthType copy_idx;
	PduLengthType trans_idx;
	PduLengthType total_Len;
	uint8* pucBuff; 
}DoIP_Buffer_Type;

typedef Std_ReturnType (*SoAdTransmit_pfunc)( PduIdType TxPduId, const PduInfoType* PduInfoPtr);

typedef struct
{
	uint8  ucSoConId;					// Socket Connection ID 
	PduIdType DoIPSoAdPduId;			// describes the connection to the SocketConnection
	uint16 uiSA;						// Source Address (SA) as soon as the information is available for the DoIP module
	uint16 uiTA;						// Tester Address
	uint8  ucConnectionState;			// Status of the SocketConnection
	uint32 ulInitialInactivityTimer;
	uint32 ulGeneralInactivityTimer;
	uint32 ulAliveCheckTimer;
	uint8  ucAuthenticationInfo;
	uint8  ucConfirmationInfo;
	uint8  ucAwaitingAliveCheckResponse;
	uint8  ucActivationType;			
	uint8  ucAllRoutingActivationStatus;// All Routing activation status of this socket connection
	boolean bIsConnectionActive;        // Information if the connection is active or not
	uint8  ucGIDSyncStatus;
	uint32 ulInactivityTimer;		/* SWS_DoIP_00142 */
	uint8  ucActivationLineStatus;	
	DoIP_Buffer_Type* pstTxBuff;
	DoIP_Buffer_Type* pstRxBuff;
	SoAdTransmit_pfunc pfSoAd_Transmit;
}DoIP_ConnectionTable_Type;

typedef struct
{
	uint8 InitStatus;
	DoIP_ConnectionTable_Type Connection_Table[DOIP_SOAD_CONNECTIONS_MAX];
}DoIP_InnerConfigType;

/*
** ===================================================================
**     Marco definition.
** ===================================================================
*/

#if(TRUE == DOIP_DEVELOPMENTERRORDETECT)
	#define DET_REPORT_ERROR(API_ID, ERROR_ID) 	PR_DEBUG(DEBUG_SWITCH, "=========DoIP=========\nApiId = %u\nErrorId = %u\n",API_ID,ERROR_ID)//  DET_REPORT_ERROR(DOIP_ID, 0, API_ID, ERROR_ID)
#else
	#define DET_REPORT_ERROR(API_ID, ERROR_ID)
#endif


/* Define:Chk_Ptr. If Ptr Null,Return E_NOT_OK and DOIP_E_PARAM_POINTER */
#define Chk_Ptr(Ptr)			 										\
do{ 																	\
	if (Ptr == NULL_PTR)												\
	{																	\
		DET_REPORT_ERROR(SERVICE_ID, DOIP_E_PARAM_POINTER); 			\
		return E_NOT_OK;/* or BUFREQ_E_NOT_OK */						\
	}																	\
}while(0)

#define Chk_Ptr_Buf(Ptr)			 									\
do{ 																	\
	if (Ptr == NULL_PTR)												\
	{																	\
		DET_REPORT_ERROR(SERVICE_ID, DOIP_E_PARAM_POINTER); 			\
		return BUFREQ_E_NOT_OK;/* or BUFREQ_E_NOT_OK */					\
	}																	\
}while(0)

#define BTYE2INT(HIGH,LOW) (((HIGH & 0xFF) << 8) | (LOW & 0xFF))
#define BTYE2WORD(pB1,pB2,pB3,pB4)	((pB1 << 24) | (pB2 << 16) | (pB3 << 8) | pB4)



/*
** ===================================================================
**	   Static Variable definition.
** ===================================================================
*/
//static struct {uint8 ucSocketConnectionState;} s_UDPConnection;		/* SWS_DoIP_00001 */

/**- Tx/Rx Buffers -***************************************************************************/
static uint8 s_ucDoIP_Tx_Buffer[DOIP_SOAD_CONNECTIONS_MAX][DOIP_BUFFER_LENGTH_MAX];
static uint8 s_ucDoIP_Rx_Buffer[DOIP_SOAD_CONNECTIONS_MAX][DOIP_BUFFER_LENGTH_MAX];
static DoIP_Buffer_Type s_stDoIPTxBuffer[DOIP_SOAD_CONNECTIONS_MAX] = 
{
	{DOIP_BUFFSTS_TX_IDLE,0,0,0,&s_ucDoIP_Tx_Buffer[DOIP_SOAD_CONNECTIONS_TCP][0]},	//TCP_TX
	{DOIP_BUFFSTS_TX_IDLE,0,0,0,&s_ucDoIP_Tx_Buffer[DOIP_SOAD_CONNECTIONS_UDP][0]}	//UDP_TX
};
static DoIP_Buffer_Type s_stDoIPRxBuffer[DOIP_SOAD_CONNECTIONS_MAX] = 
{
	{DOIP_BUFFSTS_RX_IDLE,0,0,0,&s_ucDoIP_Rx_Buffer[DOIP_SOAD_CONNECTIONS_TCP][0]},	//TCP_RX
	{DOIP_BUFFSTS_RX_IDLE,0,0,0,&s_ucDoIP_Rx_Buffer[DOIP_SOAD_CONNECTIONS_UDP][0]}	//UDP_RX
};

static DoIP_InnerConfigType s_stDoIPCfg = 							/* SWS_DoIP_00002 */
{
	DOIP_STATES_UNINIT,
	{
		{		//TCP
			0,							//uint8  ucSoConId;					// Socket Connection ID 
			0,							//PduIdType DoIPSoAdPduId;Rx/Tx		// describes the connection to the SocketConnection
			DOIP_SA,					//uint16 uiSA; for rx				// Source Address (SA) as soon as the information is available for the DoIP module
			0x790,							//uint16 uiTA; Tester Addr;
			DOIP_CONCTS_LISTEN,			//uint8  ucConnectionState;			// Status of the SocketConnection
			0,							//uint32 ulInitialInactivityTimer;
			0,							//uint32 ulGeneralInactivityTimer;
			0,							//uint32 ulAliveCheckTimer;
			0,							//uint8  ucAuthenticationInfo;
			0,							//uint8  ucConfirmationInfo;
			0,							//uint8  ucAwaitingAliveCheckResponse;
			0,							//uint8  ucActivationType;			
			0,							//uint8  ucAllRoutingActivationStatus;// All Routing activation status of this socket connection
			0,							//boolean bIsConnectionActive;        // Information if the connection is active or not
			0,							//uint8  ucGIDSyncStatus;
			0,							//uint32 ulInactivityTimer;			/* SWS_DoIP_00142 */
			0,							//uint8  ucActivationLineStatus;		
			&s_stDoIPTxBuffer[DOIP_SOAD_CONNECTIONS_TCP],		//DoIP_Buffer_Type* pstTxBuff;
			&s_stDoIPRxBuffer[DOIP_SOAD_CONNECTIONS_TCP],		//DoIP_Buffer_Type* pstRxBuff;
			&SoAd_TpTransmit,           //   pfSoAd_Transmit;
		},
		{		//UDP
			0,							//uint8  ucSoConId; 				// Socket Connection ID 
			1,							//PduIdType DoIPSoAdPduId;		// describes the connection to the SocketConnection
			DOIP_SA,					//uint16 uiSA;						// Source Address (SA) as soon as the information is available for the DoIP module
			0x790,							//uint16 uiTA;
			DOIP_CONCTS_LISTEN,			//uint8  ucConnectionState; 		// Status of the SocketConnection
			0,							//uint32 ulInitialInactivityTimer;
			0,							//uint32 ulGeneralInactivityTimer;
			0,							//uint32 ulAliveCheckTimer;
			0,							//uint8  ucAuthenticationInfo;
			0,							//uint8  ucConfirmationInfo;
			0,							//uint8  ucAwaitingAliveCheckResponse;
			0,							//uint8  ucActivationType;			
			0,							//uint8  ucAllRoutingActivationStatus;// All Routing activation status of this socket connection
			0,							//boolean bIsConnectionActive;		  // Information if the connection is active or not
			0,							//uint8  ucGIDSyncStatus;
			0,							//uint32 ulInactivityTimer; 		  /* SWS_DoIP_00142 */
			0,							//uint8  ucActivationLineStatus;
			&s_stDoIPTxBuffer[DOIP_SOAD_CONNECTIONS_UDP],		//DoIP_Buffer_Type* pstTxBuff;
			&s_stDoIPRxBuffer[DOIP_SOAD_CONNECTIONS_UDP],		//DoIP_Buffer_Type* pstRxBuff;
			&SoAd_TpTransmit,           //   pfSoAd_Transmit;
		}
	}
};

static uint8 s_ucActivationLineStatus;

static DoIP_PduRTxWaitQueue_Type s_stDoIPPduRWaitQueue;

static uint8 s_ucDoIP_CurrProtocol = 0xff;





	



/* Dcm send physical addressing in TCP type */
static const DoIP_PduRChannel_Typ s_stDoIP_PduRChannel[DOIP_UPLA_CHANNEL_MAX] =
{
	{DOIP_TX_798,DOIP_TPPDU, DOIP_SOAD_CONNECTIONS_TCP},
	{DOIP_TX_798,DOIP_IFPDU, DOIP_SOAD_CONNECTIONS_TCP},
};

/* send to PduR in diffirent addressing type according to the received TA */
static const DoIP_TAAddressing_Typ s_stDoIP_TAChannel[DOIP_UPLA_CHANNEL_MAX] = 
{
	{0x7df	,DOIP_RX_7DF_FUNC},
	{0x790	,DOIP_RX_790_PHY}
};




static uint8 LinkStatus;
static uint32 DoIp_AnnouncementTimer;
static uint8 DoIp_AnnounceWait;


/**- Routingactivet -************************************************************************/
static uint16 pendingRoutingActivationSocket = 0xff;
static uint16 pendingRoutingActivationSa = 0xff;
static uint16 pendingRoutingActivationActivationType = 0xffff;
static uint8* pendingRoutingActivationTxBuffer = NULL;

/*
** ===================================================================
**	   static function declaration.
** ===================================================================
*/
static void DiscardMsg(void);
//static boolean SoAd_BufferGet(uint32 size, uint8** buffPtr);
static void SoAd_BufferFree(uint8* buffPtr);
static void DoIP_Inner_MemSet(void* s, uint8 c, uint32 ulSize);
static void DoIP_Inner_MemCpy( void* vpTgt, const void* vpSrc, const uint32 ulSize );
static boolean DoIP_Inner_MemCmp( void* vpTgt, const void* vpSrc, const uint32 ulSize );
static Std_ReturnType DoIP_Inner_FillHeader(uint8* Header,uint16 PayLoadType,uint32 ulPayLoadLength);
static void DoIP_Inner_RegisterSocket(uint8 slotIndex, uint16 SoConId, uint8 ActivationType, uint16 SA) ;
//static void DoIP_Inner_AllSocketsAliveCheck(void);
static Std_ReturnType DoIP_Inner_TCPSocketHandle(uint16 SoConId, uint8 ActivationType, uint16 SA, uint8* routingActivationResponseCode);
static void DoIP_Inner_SendHeaderNAck(uint16 SoConId, uint8 NackCode);
static void DoIP_Inner_SendVehicleAnnouncement(uint16 SoConId);
static void DoIP_Inner_SendRoutingActivationResponse(uint16 SoConId, uint32 ulPayLoadLength, uint8 *pucRxBuff);
static void DoIP_Inner_SendAliveCheckRequest(uint16 uiConnectionIndex);
static void DoIP_Inner_RcvAliveCheckResponse(uint16 SoConId, uint32 ulPayLoadLength, uint8 *pucRxBuff);
#if 0 
static void DoIP_Inner_SendDiagMsgAck(uint16 SoConId, uint32 ulPayLoadLength, uint8 *pucRxBuff);
#endif
static void DoIP_Inner_SendDiagMsgNack(uint16 SoConId, uint16 sa, uint16 ta, uint8 nackCode,uint32 ulPayLoadLength);
static void DoIP_Inner_SendEntityStatusResp(uint16 SoConId, uint32 ulPayLoadLength, uint8 *pucRxBuff);
static void DoIP_Inner_SendPowerModeCheckResp(uint16 SoConId, uint32 ulPayLoadLength, uint8 *pucRxBuff);
#if 0 
static void DoIp_Inner_RxTCPMessage(uint8 ucIndex); 
#endif
static Std_ReturnType DoIP_SocketClose(PduIdType TxPduId);
static boolean isSourceAddressKnown(uint16 SA);
static boolean isRoutingTypeSupported(uint16 ActivationType);
static boolean isAuthenticationRequired(uint16 ActivationType);
static boolean isAuthenticated(uint16 SA, uint8 SoConId);
static boolean isConfirmationRequired(uint16 ActivationType);
static boolean isConfirmed(uint16 SA, uint8 SoConId);
static void handleTimeout(uint16 connectionIndex);
static uint8 CheckSaTa(uint16 id,uint16 SA, uint16 TA, uint32 ulPayLoadLength,uint8* ucPduRIndex);


/*===================================================================
 * Function name	: DoIP_TpTransmit
 * Description		: Requests transmission of a PDU
 * Parameter(in)	: TxPduId 
 *					: PduInfoPtr 
 * Parameter(inout)	: -
 * Parameter(out)	: -			
 * Return value 	: Std_ReturnType
====================================================================*/
Std_ReturnType DoIP_TpTransmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr)
{
	uint8 SERVICE_ID = DOIP_TPTRANSMIT;
	Std_ReturnType aucRet = E_OK;
	uint8  ucPduRIndex = 0;
	uint8  ucSoAdIndex = 0;
	uint32 ulPayLoadLength = 0;
	uint16 TA;
	uint16 SA;
	PduIdType ucTxPduId;
	PduInfoType stPduInfo;
	uint8* pucTxBuffer = NULL_PTR;
	
	PR_DEBUG(DEBUG_SWITCH, "%s  id : %d.\n", __func__,TxPduId);
	
	/* SWS_DoIP_00162 */
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
		return E_NOT_OK;
	}
	/* SWS_DoIP_00202 */
	if (DOIP_ACTIVATION_LINE_INACTIVE == s_ucActivationLineStatus)
	{	
		PR_DEBUG(DEBUG_SWITCH, "%d.\n", __LINE__);
		return E_NOT_OK;
	}

	/* SWS_DoIP_00163 */
    for (ucPduRIndex = 0; ucPduRIndex < DOIP_UPLA_CHANNEL_MAX; ucPduRIndex++) 
	{
        if (s_stDoIP_PduRChannel[ucPduRIndex].ucPduRTxPduId == TxPduId) 
		{
            break;
        }
    }
    if (ucPduRIndex == DOIP_UPLA_CHANNEL_MAX) 
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_INVALID_PDU_SDU_ID);
        return E_NOT_OK;
    }
	
	/* SWS_DoIP_00164 */
	Chk_Ptr(PduInfoPtr);	
	Chk_Ptr(PduInfoPtr->SduDataPtr);

	/* get Transmit to soad index */
	ucSoAdIndex = s_stDoIP_PduRChannel[ucPduRIndex].ucConnectTyp;

	/* SWS_DoIP_00130 */
	if (DOIP_CONCTS_LISTEN == s_stDoIPCfg.Connection_Table[ucSoAdIndex].ucConnectionState)
	{
		//response shall be discarded
		PR_DEBUG(DEBUG_SWITCH, "%d.\n", __LINE__);
		return E_NOT_OK; 
	}

	/* SWS_DoIP_00230 *//* SWS_DoIP_00226 */
#if 1
	if (DOIP_SOAD_CONNECTIONS_TCP == s_ucDoIP_CurrProtocol) 	/* SWS_DoIP_00230 */
	{
		aucRet = E_OK;
	}
	else	/* SWS_DoIP_00226 */
	{
		PR_DEBUG(DEBUG_SWITCH, "%d.\n", __LINE__);
	//	return E_NOT_OK;
	}
#endif

	PR_DEBUG(DEBUG_SWITCH, "%s : %d : %d\n", __func__,__LINE__,s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status);

	/* SWS_DoIP_00230 .1 */
    if (s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status == DOIP_BUFFSTS_TX_IDLE) 
	{
		s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status = DOIP_BUFFSTS_TX_READY_TO_SEND;
		s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->trans_idx = 0;
		s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->copy_idx = 0;
		s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->total_Len = 0;
#if 0	/* todo... */
		/* SWS_DoIP_00220 */
		if (DOIP_MSGTYP_DIAG != s_stDoIPCfg.Connection_Table[ucIndex].stChannel.PduRTxPdu.DoIPPduType)
		{
			ucTxPduId = s_stDoIPCfg.Connection_Table[ucIndex].DoIPSoAdPduId;
			stPduInfo.SduLength = ulPayLoadLength;;
			stPduInfo.SduDataPtr =  PduInfoPtr->SduDataPtr;
		}
		else
#endif
		{
			/* SWS_DoIP_00131 */
	        TA = s_stDoIPCfg.Connection_Table[ucSoAdIndex].uiTA; // DoIPTesterAddress
			SA = s_stDoIPCfg.Connection_Table[ucSoAdIndex].uiSA;	//DoIPEntiy
			//SA = g_stDoIPCfg.DoIPConfigSet.DoIPConnections[ucIndex].DoIPTargetAddress.DoIPTargetAddressValue;

			pucTxBuffer = s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->pucBuff;
			//ASSERT(NULL_PTR == pucTxBuffer);

			/* SWS_DoIP_00284 */
			ulPayLoadLength = DOIP_OFFSET_SA + DOIP_OFFSET_TA + PduInfoPtr->SduLength;
			DoIP_Inner_FillHeader(pucTxBuffer, DOIP_PT_DIAGMSG, ulPayLoadLength);
	        pucTxBuffer[DOIP_GH_TOTAL+0] = SA >> 8;
	        pucTxBuffer[DOIP_GH_TOTAL+1] = SA >> 0;
	        pucTxBuffer[DOIP_GH_TOTAL+2] = TA >> 8;
	        pucTxBuffer[DOIP_GH_TOTAL+3] = TA >> 0;
			/* SWS_DoIP_00173 */
	        DoIP_Inner_MemCpy(&pucTxBuffer[DOIP_GH_TOTAL+4], PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);
			s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->copy_idx = DOIP_GH_TOTAL + ulPayLoadLength;

			ucTxPduId = s_stDoIPCfg.Connection_Table[ucSoAdIndex].DoIPSoAdPduId;
			stPduInfo.SduLength = DOIP_GH_TOTAL + ulPayLoadLength;
			s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->total_Len = stPduInfo.SduLength;
			stPduInfo.SduDataPtr =  pucTxBuffer;
		}
		PR_DEBUG(DEBUG_SWITCH, " %d :%d - %d,\n", __LINE__,stPduInfo.SduLength,stPduInfo.SduDataPtr[0]);
        if (E_OK == s_stDoIPCfg.Connection_Table[ucSoAdIndex].pfSoAd_Transmit(ucTxPduId, &stPduInfo))	/* SWS_DoIP_00220 note: */
		{
			PR_DEBUG(DEBUG_SWITCH, " %d :%d - %d,\n", __LINE__,stPduInfo.SduLength,stPduInfo.SduDataPtr[0]);
			s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status = DOIP_BUFFSTS_TX_COPY_ALLOW;
			s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->trans_idx = stPduInfo.SduLength;
        } 
		else 
		{
			PR_DEBUG(DEBUG_SWITCH, " %d :%d - %d,\n", __LINE__,stPduInfo.SduLength,stPduInfo.SduDataPtr[0]);
			/* SWS_DoIP_00223 */
			DoIP_Inner_MemSet(s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff, 0, sizeof(s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff));
			s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status = DOIP_BUFFSTS_TX_IDLE;
			//DiscardMsg();
            /* SWS_DoIP_00228 */
			//DiscardMsg();
			if (DOIP_TPPDU == s_stDoIP_PduRChannel[ucPduRIndex].DoIPPduType)
			{
				Dcm_TpTxConfirmation(TxPduId,E_NOT_OK);
			}
        }

		PR_DEBUG(DEBUG_SWITCH, "%s : %d : %d\n", __func__,__LINE__,s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status);

    }
	else	/* SWS_DoIP_00230 .2 */
	{
		if (s_stDoIPPduRWaitQueue.Cnt < 255)
		{
			s_stDoIPPduRWaitQueue.info[s_stDoIPPduRWaitQueue.Cnt].DoIPPduRTxPduId = TxPduId;
			s_stDoIPPduRWaitQueue.info[s_stDoIPPduRWaitQueue.Cnt].DoIPPduType = s_stDoIP_PduRChannel[ucPduRIndex].DoIPPduType;
			DoIP_Inner_MemCpy(&s_stDoIPPduRWaitQueue.info[s_stDoIPPduRWaitQueue.Cnt].DoIPPduRTxPduRef,PduInfoPtr,sizeof(PduInfoType));
			s_stDoIPPduRWaitQueue.Cnt++;			
		}
		else
		{
			aucRet = E_NOT_OK;
		}
	}
    return aucRet;
}

/*===================================================================
 * Function name	: DoIP_TpCancelTransmit
 * Description		: Requests cancellation of an ongoing transmission
 * 					  of a PDU in a lower layer communication module
 * Parameter(in)	: TxPduId 
 * Parameter(inout)	: -
 * Parameter(out)	: -			
 * Return value 	: Std_ReturnType
====================================================================*/
Std_ReturnType DoIP_TpCancelTransmit(PduIdType TxPduId)
{
#if 1
	uint8 SERVICE_ID = DOIP_TPCANCELTRANSMIT;
	uint8 ucPduRIndex;
	uint8 ucSoAdIndex;
	
	PR_DEBUG(DEBUG_SWITCH, "%s  id : %d.\n", __func__,TxPduId);
		
	/* SWS_DoIP_00166 */
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
		return E_NOT_OK;
	}
	/* SWS_DoIP_00202 */
	if (DOIP_ACTIVATION_LINE_INACTIVE == s_ucActivationLineStatus)
	{
		return E_NOT_OK;
	}
	
	/* SWS_DoIP_00257 */	
	for (ucPduRIndex = 0; ucPduRIndex < DOIP_UPLA_CHANNEL_MAX; ucPduRIndex++)
	{
		if (s_stDoIP_PduRChannel[ucPduRIndex].ucPduRTxPduId == TxPduId)
			break;
	}
	if (ucPduRIndex != DOIP_UPLA_CHANNEL_MAX)
	{
		ucSoAdIndex = s_stDoIP_PduRChannel[ucPduRIndex].ucConnectTyp;
		return SoAd_TpCancelTransmit(s_stDoIPCfg.Connection_Table[ucSoAdIndex].DoIPSoAdPduId);
	}
	else
	{
		return E_NOT_OK;
	}
#endif
}

/*===================================================================
 * Function name	: DoIP_TpCancelReceive
 * Description		: Requests cancellation of an ongoing reception of
 * 					  a PDU in a lower layer transport protocol module.
 * Parameter(in)	: TxPduId 
 * Parameter(inout)	: -
 * Parameter(out)	: -			
 * Return value 	: Std_ReturnType
====================================================================*/
Std_ReturnType DoIP_TpCancelReceive(PduIdType RxPduId)
{
#if 1
	uint8 SERVICE_ID = DOIP_TPCANCELRECEIVE;
//	uint8 ucIndex;
	uint8 ucPduRIndex;
	uint8 ucSoAdIndex;
	
	PR_DEBUG(DEBUG_SWITCH, "%s  id : %d.\n", __func__,RxPduId);
		
	/* SWS_DoIP_00169 */
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
		return E_NOT_OK;
	}	
	/* SWS_DoIP_00202 */
	if (DOIP_ACTIVATION_LINE_INACTIVE == s_ucActivationLineStatus)
	{
		return E_NOT_OK;
	}

	/* SWS_DoIP_00258 */

	for (ucPduRIndex = 0; ucPduRIndex < DOIP_UPLA_CHANNEL_MAX; ucPduRIndex++)
	{
		if (s_stDoIP_PduRChannel[ucPduRIndex].ucPduRTxPduId == RxPduId)
			break;
	}
	if (ucPduRIndex != DOIP_UPLA_CHANNEL_MAX)
	{
		ucSoAdIndex = s_stDoIP_PduRChannel[ucPduRIndex].ucConnectTyp;
		return SoAd_TpCancelReceive(s_stDoIPCfg.Connection_Table[ucSoAdIndex].DoIPSoAdPduId);
	}
	else
	{
		return E_NOT_OK;
	}
#endif
}

/*===================================================================
 * Function name	: DoIP_IfTransmit
 * Description		: Requests cancellation of an ongoing reception of
 * 					  a PDU in a lower layer transport protocol module.
 * Parameter(in)	: TxPduId 
 *					: PduInfoPtr 
 * Parameter(inout)	: -
 * Parameter(out)	: -			
 * Return value 	: Std_ReturnType
====================================================================*/
Std_ReturnType DoIP_IfTransmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr)
{
	uint8 SERVICE_ID = DOIP_IFTRANSMIT;
	Std_ReturnType aucRet = E_OK;
	uint8  ucPduRIndex = 0;
	uint8  ucSoAdIndex = 0;
	uint32 ulPayLoadLength = 0;
//	uint16 TA;
//	uint16 SA;
	PduIdType ucTxPduId;
	PduInfoType stPduInfo;
	uint8* pucTxBuffer = NULL_PTR;
	
	PR_DEBUG(DEBUG_SWITCH, "%s  id : %d.\n", __func__,TxPduId);
		
	/* SWS_DoIP_00202 */
	if (DOIP_ACTIVATION_LINE_INACTIVE == s_ucActivationLineStatus)
	{
		return E_NOT_OK;
	}

    for (ucPduRIndex = 0; ucPduRIndex < DOIP_UPLA_CHANNEL_MAX; ucPduRIndex++) 
	{
        if (s_stDoIP_PduRChannel[ucPduRIndex].ucPduRTxPduId == TxPduId) 
		{
            break;
        }
    }
    if (ucPduRIndex == DOIP_UPLA_CHANNEL_MAX) 
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_INVALID_PDU_SDU_ID);
        return E_NOT_OK;
    }


	/* SWS_DoIP_00164 */
	Chk_Ptr(PduInfoPtr);	
	Chk_Ptr(PduInfoPtr->SduDataPtr);
//	Chk_Ptr(PduInfoPtr->MetaDataPtr);

	/* get Transmit to soad index */
	ucSoAdIndex = s_stDoIP_PduRChannel[ucPduRIndex].ucConnectTyp;
	pucTxBuffer = s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->pucBuff;
	
	/* SWS_DoIP_00279 */
	if (DOIP_IFPDU == s_stDoIP_PduRChannel[ucPduRIndex].DoIPPduType)
	{
		if (DOIP_BUFFER_LENGTH_MAX < PduInfoPtr->SduLength)
		{
			return E_NOT_OK;
		}
		else
		{
			s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->total_Len = PduInfoPtr->SduLength;
			DoIP_Inner_MemCpy(pucTxBuffer, PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);
		}
	}

	/* SWS_DoIP_00230 .1 */
    if (s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status == DOIP_BUFFSTS_TX_IDLE) 
	{
		s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status = DOIP_BUFFSTS_TX_READY_TO_SEND;

#if 0		
		if (1 /* todo... DOIP_MSGTYP_DIAG == s_stDoIPCfg.Connection_Table[ucIndex].stChannel.PduRTxPdu.DoIPPduType */ )
			{}
		else
#endif
		{
			ucTxPduId = s_stDoIPCfg.Connection_Table[ucSoAdIndex].DoIPSoAdPduId;
			stPduInfo.SduLength = DOIP_GH_TOTAL + ulPayLoadLength;;
			stPduInfo.SduDataPtr =  PduInfoPtr->SduDataPtr;
			/* SWS_DoIP_00198 *//* SWS_DoIP_00279 note: */
	        if (E_OK == s_stDoIPCfg.Connection_Table[ucSoAdIndex].pfSoAd_Transmit(ucTxPduId, &stPduInfo))	
			{
	            s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status = DOIP_BUFFSTS_TX_COPY_ALLOW;
	        } 
			else 
			{
				/* SWS_DoIP_00223 */
				DoIP_Inner_MemSet(s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff, 0, sizeof(s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff));
				s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status = DOIP_BUFFSTS_TX_IDLE;
				//DiscardMsg();
	            /* SWS_DoIP_00228 */
				//DiscardMsg();
				if (DOIP_TPPDU == s_stDoIP_PduRChannel[ucPduRIndex].DoIPPduType)
				{
					Dcm_TpTxConfirmation(TxPduId,E_NOT_OK);
				}
	        }
		}
	}
	else	/* SWS_DoIP_00230 .2 */
	{
		if (s_stDoIPPduRWaitQueue.Cnt < 255)
		{
			s_stDoIPPduRWaitQueue.info[s_stDoIPPduRWaitQueue.Cnt].DoIPPduRTxPduId = TxPduId;
			s_stDoIPPduRWaitQueue.info[s_stDoIPPduRWaitQueue.Cnt].DoIPPduType = s_stDoIP_PduRChannel[ucPduRIndex].DoIPPduType;
			DoIP_Inner_MemCpy(&s_stDoIPPduRWaitQueue.info[s_stDoIPPduRWaitQueue.Cnt].DoIPPduRTxPduRef,PduInfoPtr,sizeof(PduInfoType));
			s_stDoIPPduRWaitQueue.Cnt++;			
		}
		else
		{
			aucRet = E_NOT_OK;
		}
	}
	return aucRet;
}

/*===================================================================
 * Function name	: DoIP_IfCancelTransmit
 * Description		: Requests cancellation of an ongoing transmission
 * 					  of a PDU in a lower layer communication module
 * Parameter(in)	: TxPduId 
 * Parameter(inout)	: -
 * Parameter(out)	: -			
 * Return value 	: Std_ReturnType
====================================================================*/
Std_ReturnType DoIP_IfCancelTransmit(PduIdType TxPduId)
{
#if 1
	uint8 SERVICE_ID = DOIP_IFCANCELTRANSMIT;
	uint8 ucPduRIndex;
	uint8 ucSoAdIndex;
	
	/* SWS_DoIP_00166 */
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
		return E_NOT_OK;
	}

	
	PR_DEBUG(DEBUG_SWITCH, "%s  id : %d.\n", __func__,TxPduId);
	

	/* SWS_DoIP_00202 */
	if (DOIP_ACTIVATION_LINE_INACTIVE == s_ucActivationLineStatus)
	{
		return E_NOT_OK;
	}

	/* SWS_DoIP_00257 */
    for (ucPduRIndex = 0; ucPduRIndex < DOIP_UPLA_CHANNEL_MAX; ucPduRIndex++) 
	{
        if (s_stDoIP_PduRChannel[ucPduRIndex].ucPduRTxPduId == TxPduId) 
		{
            break;
        }
    }
	if (ucPduRIndex < DOIP_UPLA_CHANNEL_MAX)
	{
		ucSoAdIndex = s_stDoIP_PduRChannel[ucPduRIndex].ucConnectTyp;
		return SoAd_TpCancelTransmit(s_stDoIPCfg.Connection_Table[ucSoAdIndex].DoIPSoAdPduId);
	}
	else
	{
		return E_NOT_OK;
	}
#endif
}

/*===================================================================
 * Function name	: DoIP_Init
 * Description		: This service initializes all global variables 
 * 					  of the DoIP module. After return of this
 *                    service the DoIP module is operational
 * Parameter(in)	: DoIPConfigPtr 
 * Parameter(inout)	: -
 * Parameter(out)	: -			
 * Return value 	: -
====================================================================*/
void DoIP_Init(const DoIP_ConfigType* DoIPConfigPtr)
{
	uint8 SERVICE_ID = DOIP_INIT;
	uint16 uiLoop;
	
	PR_DEBUG(DEBUG_SWITCH, "DoIP_Init Start !!!\n");
	if (NULL == DoIPConfigPtr)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_PARAM_POINTER);
	}
	else
	{
		for (uiLoop = 0; uiLoop < DOIP_SOAD_CONNECTIONS_MAX; uiLoop++)
		{
			s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState = DOIP_CONCTS_LISTEN;
			PR_DEBUG(DEBUG_SWITCH, "%d-----ucConnectionState:%d\n", __LINE__,s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState);
			s_stDoIPCfg.Connection_Table[uiLoop].ucSoConId = 0xff;
			s_stDoIPCfg.Connection_Table[uiLoop].ucAwaitingAliveCheckResponse = FALSE;
			s_stDoIPCfg.Connection_Table[uiLoop].ulGeneralInactivityTimer = 0;
			s_stDoIPCfg.Connection_Table[uiLoop].ulInitialInactivityTimer = 0;	/* init */
			s_stDoIPCfg.Connection_Table[uiLoop].ulAliveCheckTimer = 0;	
		}
		s_stDoIPCfg.InitStatus = DOIP_STATES_INITED;		
	}
	PR_DEBUG(DEBUG_SWITCH, "DoIP_Init Done !!!\n");
}

/*===================================================================
 * Function name	: DoIP_GetVersionInfo
 * Description		: Returns the version information of this module
 * Parameter(in)	: - 
 * Parameter(inout)	: -
 * Parameter(out)	: versioninfo
 * Return value 	: -
====================================================================*/
void DoIP_GetVersionInfo(Std_VersionInfoType* versioninfo)
{
	uint8 SERVICE_ID = DOIP_GETVERSIONINFO;
	if (versioninfo == NULL_PTR)	/* SWS_DoIP_00172 */
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_PARAM_POINTER);
	}
	else
	{
		versioninfo->vendorID = INVO_ID;
		versioninfo->moduleID = DOIP_ID;
		versioninfo->sw_major_version = DOIP_MAJOR_VERSION;
		versioninfo->sw_minor_version = DOIP_MINOR_VERSION;
		versioninfo->sw_patch_version = DOIP_PATCH_VERSION;			
	}
}

/*===================================================================
 * Function name	: DoIP_SoAdTpCopyTxData
 * Description		: Returns the version information of this module
 * Parameter(in)	: id,  info, retry
 * Parameter(inout)	: -
 * Parameter(out)	: availableDataPtr
 * Return value 	: BufReq_ReturnType
====================================================================*/
BufReq_ReturnType DoIP_SoAdTpCopyTxData(PduIdType id, const PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr)
{
	uint8 SERVICE_ID = DOIP_SOADTPCOPYTXDATA;
	uint8 ucLoop = 0;
	uint8  ucPduRIndex = 0;
	uint8  ucSoAdIndex = 0;
	PduInfoType stInfo_Ext;
	PduLengthType ucOffset_Ext = 0;
	RetryInfoType  retry_Ext;
	PduLengthType availableDataPtr_Ext = 0;
	
	BufReq_ReturnType ucRet = BUFREQ_E_NOT_OK;
	
	PR_DEBUG(DEBUG_SWITCH, "%s  id : %d.\n", __func__,id);
	

	/* SWS_DoIP_00175 */
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
		return BUFREQ_E_NOT_OK;
	}
	/* SWS_DoIP_00202 */
	if (DOIP_ACTIVATION_LINE_INACTIVE == s_ucActivationLineStatus)
	{
		return BUFREQ_E_NOT_OK;
	}
	/* SWS_DoIP_00176 */
	for (ucSoAdIndex = 0;ucSoAdIndex < DOIP_SOAD_CONNECTIONS_MAX;ucSoAdIndex++)
	{
		if (id == s_stDoIPCfg.Connection_Table[ucSoAdIndex].DoIPSoAdPduId)
		{
			break;
		}
	}
	if (ucSoAdIndex == DOIP_SOAD_CONNECTIONS_MAX)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_INVALID_PDU_SDU_ID);
		return BUFREQ_E_NOT_OK;
	}
	
	/* SWS_DoIP_00177 */
	Chk_Ptr_Buf(info);

	/* SWS_DoIP_00178 */
	Chk_Ptr_Buf(retry);

	PR_DEBUG(DEBUG_SWITCH, "%s : %d : %d\n", __func__,__LINE__,s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status);

	
	if ((DOIP_BUFFSTS_TX_COPY_ALLOW == s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status)
		|| (DOIP_BUFFSTS_TX_COPY_ALLOW == s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status))
	{
		s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status = DOIP_BUFFSTS_TX_COPY_ALLOW;

#if 1
		for (ucLoop = 0; ucLoop < DOIP_UPLA_CHANNEL_MAX; ucLoop++)
		{
			if (s_stDoIP_TAChannel[ucLoop].uiTA == s_stDoIPCfg.Connection_Table[ucSoAdIndex].uiTA)
			{
				ucPduRIndex = s_stDoIP_TAChannel[ucLoop].ucPduRRxPduId;
				break;
			}
		}
		
		DoIP_Inner_MemSet(&stInfo_Ext, 0, sizeof(stInfo_Ext));
		DoIP_Inner_MemSet(&retry_Ext, 0, sizeof(retry_Ext));

		(void)stInfo_Ext;
		(void)ucOffset_Ext;
		(void)retry_Ext;
		(void)availableDataPtr_Ext;

		if (info->SduLength == 0)
		{
			*availableDataPtr = s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->total_Len - s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->trans_idx;
		}
		else
		{
			s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->copy_idx = 0;
			s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->total_Len = DOIP_BUFFSTS_TX_COPY_ALLOW;
			s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->trans_idx = DOIP_BUFFSTS_TX_COPY_ALLOW;
			DoIP_Inner_MemSet(s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->pucBuff, 0, DOIP_BUFFER_LENGTH_MAX);
			Chk_Ptr_Buf(info->SduDataPtr);
			ucRet= Dcm_CopyTxData(ucPduRIndex, info, retry, availableDataPtr);
		}

#else
#if 0	/* todo... */
		if (DOIP_MSGTYP_DIAG != s_stDoIPCfg.Connection_Table[ucIndex].stChannel.PduRTxPdu.DoIPPduType)
		{
			if (0 == info->SduLength)	/* SWS_DoIP_00224 */
			{
				*availableDataPtr = info->SduLength;
				ucRet = BUFREQ_OK;		
			}
			else						/* SWS_DoIP_00225 */
			{
				*availableDataPtr = info->SduLength;
				DoIP_Inner_MemCpy(stInfo.SduDataPtr,info->SduDataPtr, info->SduLength);
				ucRet = BUFREQ_OK;
			}	
		}
		else	/*  */
#endif			
		{
			if (0 == info->SduLength) 	/* SWS_DoIP_00231 */
			{
				//the total available data size of the current buffered DoIP message to be transmitted
				/* SWS_DoIP_00231 .note : This means that only the length for the created DoIP header and the diagnostic
										  SourceAddress/TargetAddress is returned and not the total data length */
				*availableDataPtr = DOIP_GH_TOTAL + DOIP_OFFSET_SA + DOIP_OFFSET_TA;
				ucRet = BUFREQ_OK;
			}
			else 						/* SWS_DoIP_00232 */
			{		
				if (info->SduLength > s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->total_Len)
				{
					stInfo_Ext.SduLength =  info->SduLength - s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->total_Len;
					ucOffset_Ext = s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->total_Len;
					stInfo_Ext.SduDataPtr = &s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->pucBuff[ucOffset_Ext];

					for (ucLoop = 0; ucLoop < DOIP_UPLA_CHANNEL_MAX; ucLoop++)
					{
						if (s_stDoIP_TAChannel[ucLoop].uiTA == s_stDoIPCfg.Connection_Table[ucSoAdIndex].uiTA)
						{
							ucPduRIndex = s_stDoIP_TAChannel[ucLoop].ucPduRRxPduId;
							break;
						}
					}

	PR_DEBUG(DEBUG_SWITCH, "%s : %d : %d\n", __func__,__LINE__,ucPduRIndex);

				/* SWS_DoIP_00233 */
				/* todo... */
					retry_Ext.TpDataState = TP_DATACONF;
					retry_Ext.TxTpDataCnt = 0;
					ucRet= Dcm_CopyTxData(ucPduRIndex,&stInfo_Ext,&retry_Ext,&availableDataPtr_Ext);
					if (BUFREQ_OK == ucRet) /* SWS_DoIP_00254 */
					{
						*availableDataPtr = availableDataPtr_Ext - stInfo_Ext.SduLength; /* todo... */
					}
				}
				else if (info->SduLength < s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->total_Len)
				{	
					/* todo... */
					*availableDataPtr = s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->total_Len - (DOIP_GH_TOTAL + DOIP_OFFSET_SA + DOIP_OFFSET_TA);
				}
				else
				{
					/* todo... */
				}
			}			
		}
#endif
	}
	else
	{
		*availableDataPtr = 0;
	}
	return ucRet;
}

/*===================================================================
 * Function name	: DoIP_SoAdTpTxConfirmation
 * Description		: indicates whether the transmission was successful or not
 * Parameter(in)	: id, result
 * Parameter(inout)	: -
 * Parameter(out)	: -
 * Return value 	: -
====================================================================*/
void DoIP_SoAdTpTxConfirmation(PduIdType id, Std_ReturnType result)
{
	uint8 SERVICE_ID = DOIP_SOADTPTXCONFIRMATION;
	uint8 ucSoAdIndex;
	uint8 pdurid = id;

		
	PR_DEBUG(DEBUG_SWITCH, "%s  id : %d.\n", __func__,id);
	
	/* SWS_DoIP_00162 */
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
	}
	else
	{
		/* SWS_DoIP_00229 */
		for (ucSoAdIndex = 0; ucSoAdIndex < DOIP_SOAD_CONNECTIONS_MAX; ++ucSoAdIndex)
		{
			if ((DOIP_BUFFSTS_TX_COPY_ALLOW == s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status)
			|| (DOIP_BUFFSTS_TX_COPY_ALLOW == s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status))
			{
				s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status = DOIP_BUFFSTS_TX_IDLE;
				DoIP_Inner_MemSet(s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff, 0, sizeof(s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff));
				Dcm_TpTxConfirmation(pdurid,result);
				break;
			}
		}
	}
}


/*===================================================================
 * Function name	: DoIP_SoAdTpCopyRxData
 * Description		: provide the received data of an I-PDU segment
 					  (N-PDU) to the upper layer
 * Parameter(in)	: id, info
 * Parameter(inout)	: -
 * Parameter(out)	: bufferSizePtr
 * Return value 	: BufReq_ReturnType
====================================================================*/
BufReq_ReturnType DoIP_SoAdTpCopyRxData(PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr)
{
	uint8 SERVICE_ID = DOIP_SOADTPCOPYRXDATA;
//	uint8* pucRxBuffer;
	PduIdType ucPduRIndex = 0;
	uint8 ucLoop = 0;
	uint16 uiPayLoadType;
	uint32 ulPayLoadLength;
	static uint8 PdurSentSts = BUFREQ_E_NOT_OK;

    PduInfoType stInfo_Ext;
    PduLengthType TpSduLength_Ext;
    PduLengthType bufferSizePtr_Ext = 0;
    PduLengthType availableDataPtr_Ext;

		
	PR_DEBUG(DEBUG_SWITCH, "%s  id : %d.\n", __func__,id);
	

	/* SWS_DoIP_00162 */
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
		return BUFREQ_E_NOT_OK;
	}
	/* SWS_DoIP_00202 */
	if (DOIP_ACTIVATION_LINE_INACTIVE == s_ucActivationLineStatus)
	{
		return BUFREQ_E_NOT_OK;
	}

	/*  */
	if (id >= DOIP_SOAD_CONNECTIONS_MAX)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_INVALID_PDU_SDU_ID);
		return BUFREQ_E_NOT_OK;
	}

	/*  */
	if (info == NULL_PTR)
	{
		DET_REPORT_ERROR(SERVICE_ID, DOIP_E_PARAM_POINTER);
		return BUFREQ_E_NOT_OK;/* or BUFREQ_E_NOT_OK */		
	}
	else if (info->SduDataPtr == NULL_PTR)
	{
		DET_REPORT_ERROR(SERVICE_ID, DOIP_E_PARAM_POINTER);
		return BUFREQ_E_NOT_OK;/* or BUFREQ_E_NOT_OK */			
	}
	else if (bufferSizePtr == NULL_PTR)
	{
		DET_REPORT_ERROR(SERVICE_ID, DOIP_E_PARAM_POINTER);
		return BUFREQ_E_NOT_OK;/* or BUFREQ_E_NOT_OK */			
	}
	else
	{
	
		PR_DEBUG(DEBUG_SWITCH, "%d...%x --	%x . \n", __LINE__,info->SduDataPtr[0],info->SduDataPtr[1]);
		PR_DEBUG(DEBUG_SWITCH, "%d...Status = %d   ---  info->SduLength  = %u.\n", __LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status,info->SduLength);
		if (s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status == DOIP_BUFFSTS_RX_START) 
		{
			s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_COPY;
			PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);

			/* SWS_DoIP_00208 */
			if (0 == info->SduLength)	
			{
				*bufferSizePtr = DOIP_GH_TOTAL;
				//*bufferSizePtr = (DOIP_GH_TOTAL + DOIP_OFFSET_SA + DOIP_OFFSET_TA);
			}
			/* SWS_DoIP_00209 */
 			else if ((DOIP_BUFFER_LENGTH_MAX - s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx) >= info->SduLength)	
			{
				PduLengthType copy_idx = 0;
				copy_idx = s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx;
				DoIP_Inner_MemCpy(&s_stDoIPCfg.Connection_Table[id].pstRxBuff->pucBuff[copy_idx] ,info->SduDataPtr,  info->SduLength);
				s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx += info->SduLength;
				*bufferSizePtr = DOIP_BUFFER_LENGTH_MAX - s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx;
			}
			/* SWS_DoIP_00210 */
			else					
			{
				return BUFREQ_E_NOT_OK;
			}
		}
		else if (s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status == DOIP_BUFFSTS_RX_COPY) 
		{
		   if ((DOIP_BUFFER_LENGTH_MAX - s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx) >= info->SduLength)   
		   {
			   PduLengthType copy_idx = 0;
			   copy_idx = s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx;
			   DoIP_Inner_MemCpy(&s_stDoIPCfg.Connection_Table[id].pstRxBuff->pucBuff[copy_idx] ,info->SduDataPtr,  info->SduLength);
			   s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx += info->SduLength;
			   *bufferSizePtr = DOIP_BUFFER_LENGTH_MAX - s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx; 
		   }
		   else 				   
		   {
			   return BUFREQ_E_NOT_OK;
		   }
		}
		/* SWS_DoIP_00218 */
		else if (s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status == DOIP_BUFFSTS_RX_DIRECT)
		{
			if ((DOIP_BUFFER_LENGTH_MAX - s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx) >= info->SduLength)   
			{
				PduLengthType copy_idx = 0;
				copy_idx = s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx;
				DoIP_Inner_MemCpy(&s_stDoIPCfg.Connection_Table[id].pstRxBuff->pucBuff[copy_idx] ,info->SduDataPtr,  info->SduLength);
				s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx += info->SduLength;
				*bufferSizePtr = DOIP_BUFFER_LENGTH_MAX - s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx; 
			}
			else 				   
			{
				return BUFREQ_E_NOT_OK;
			}
		   
			for (ucLoop = 0; ucLoop < DOIP_UPLA_CHANNEL_MAX; ucLoop++)
			{
				if (s_stDoIP_TAChannel[ucLoop].uiTA == s_stDoIPCfg.Connection_Table[id].uiTA)
				{
					ucPduRIndex = s_stDoIP_TAChannel[ucLoop].ucPduRRxPduId;
					break;
				}
			}

			if (ucPduRIndex < DOIP_UPLA_CHANNEL_MAX)
			{
				stInfo_Ext.SduDataPtr = &s_stDoIPCfg.Connection_Table[id].pstRxBuff->pucBuff[DOIP_GH_TOTAL];
				stInfo_Ext.SduLength = s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx;
				PdurSentSts = Dcm_CopyRxData(ucPduRIndex,&stInfo_Ext,&availableDataPtr_Ext);
				*bufferSizePtr = availableDataPtr_Ext;
			}
			else
			{
				//BUFREQ_E_NOT_OK
			}
		}
		else
		{
			//  ? or DOIP_BUFFSTS_RX_IDLE ?
		}

PR_DEBUG(DEBUG_SWITCH, "%d...copy_idx = %d\n", __LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx);	
PR_DEBUG(DEBUG_SWITCH, "%d...%x --	%x . \n", __LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->pucBuff[0],s_stDoIPCfg.Connection_Table[id].pstRxBuff->pucBuff[1]);
		/* SWS_DoIP_00214 */
	//	if (s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx > (DOIP_GH_TOTAL + DOIP_OFFSET_SA + DOIP_OFFSET_TA))
		if (s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx >= DOIP_GH_TOTAL)
		{
			PR_DEBUG(DEBUG_SWITCH, "%d................................................... \n", __LINE__);		
			uint8* pucRxBuff;
			pucRxBuff = &s_stDoIPCfg.Connection_Table[id].pstRxBuff->pucBuff[0];

			PR_DEBUG(DEBUG_SWITCH, "%d...%x --  %x . \n", __LINE__,pucRxBuff[0],pucRxBuff[1]);
			
			if ((pucRxBuff[0] == DOIP_PROTOCOL_VERSION) && ((uint8)(pucRxBuff[1]) == DOIP_INVERSE_PROTOCOL_VERSION)) 
			{
				uiPayLoadType = pucRxBuff[2] << 8 | pucRxBuff[3];
				ulPayLoadLength = (pucRxBuff[4] << 24) | (pucRxBuff[5] << 16) | (pucRxBuff[6] << 8) | pucRxBuff[7];
				
				PR_DEBUG(DEBUG_SWITCH, "%d...%x %x \n", __LINE__,uiPayLoadType,ulPayLoadLength);

				if (DOIP_SOAD_CONNECTIONS_TCP == id)
				{
					if ((ulPayLoadLength + DOIP_GH_TOTAL) <= DOIP_BUFFER_LENGTH_MAX) 
					{
						switch (uiPayLoadType) 
						{
							case DOIP_PT_ROUTACTVTN_REQ:
								/* SWS_DoIP_00159 */
								s_stDoIPCfg.Connection_Table[id].ulInitialInactivityTimer = 0;/* Stop */
								DoIP_Inner_SendRoutingActivationResponse(id, ulPayLoadLength, pucRxBuff);
								s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_COPY;
								PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);
								break;
				
							case DOIP_PT_ALIVECHK_RESP:
								DoIP_Inner_RcvAliveCheckResponse(id, ulPayLoadLength, pucRxBuff);
								s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_COPY;
								PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);
								break;
				
							case DOIP_PT_DIAGMSG:
							{
								if (ulPayLoadLength >= DOIP_PL_DIAGMSG_MIN) /* SWS_DoIP_00122 */
								{
									uint16 SA = (pucRxBuff[DOIP_GH_TOTAL] << 8) | pucRxBuff[DOIP_GH_TOTAL + 1];
									uint16 TA = (pucRxBuff[DOIP_GH_TOTAL + DOIP_OFFSET_SA] << 8) | pucRxBuff[DOIP_GH_TOTAL + DOIP_OFFSET_SA + 1];
									uint8 lookupResult = CheckSaTa(id,SA, TA, ulPayLoadLength, &ucPduRIndex);
							
									PR_DEBUG(DEBUG_SWITCH, "%d...SA:%x,     TA:%x ,   %x\n", __LINE__,SA,TA,lookupResult);

									if (lookupResult == E_OK) 
									{
										/* 0x8002 */
										DoIP_Inner_SendDiagMsgNack(id, SA, TA, 0,ulPayLoadLength);
										
										PR_DEBUG(DEBUG_SWITCH, "%d...ucPduRIndex: %x. \n", __LINE__,ucPduRIndex);										
										if (ucPduRIndex < DOIP_UPLA_CHANNEL_MAX)
										{
											s_stDoIPCfg.Connection_Table[id].pstRxBuff->trans_idx = DOIP_GH_TOTAL + DOIP_OFFSET_SA + DOIP_OFFSET_SA;
											stInfo_Ext.SduDataPtr = &s_stDoIPCfg.Connection_Table[id].pstRxBuff->pucBuff[s_stDoIPCfg.Connection_Table[id].pstRxBuff->trans_idx];
											stInfo_Ext.SduLength = s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx - s_stDoIPCfg.Connection_Table[id].pstRxBuff->trans_idx;
											TpSduLength_Ext = s_stDoIPCfg.Connection_Table[id].pstRxBuff->total_Len - s_stDoIPCfg.Connection_Table[id].pstRxBuff->trans_idx;
											if (TpSduLength_Ext > DOIP_UDS_BUFF_MAX)
											{
												TpSduLength_Ext = DOIP_UDS_BUFF_MAX;
											}
											PdurSentSts = Dcm_StartOfReception(ucPduRIndex,&stInfo_Ext,TpSduLength_Ext,&bufferSizePtr_Ext);
											
											PR_DEBUG(DEBUG_SWITCH, "%d...PdurSentSts: %x,  bufferSizePtr:%d . \n", __LINE__,PdurSentSts,bufferSizePtr_Ext);	

											/* SWS_DoIP_00253 */
											if (bufferSizePtr_Ext < info->SduLength)
											{
												Dcm_TpRxIndication(ucPduRIndex,E_NOT_OK);
											}
											
											if (PdurSentSts == BUFREQ_OK)
											{
												*bufferSizePtr = bufferSizePtr_Ext;
												s_stDoIPCfg.Connection_Table[id].pstRxBuff->trans_idx += stInfo_Ext.SduLength;
												/* SWS_DoIP_00260 */
												while(s_stDoIPCfg.Connection_Table[id].pstRxBuff->trans_idx < s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx)
												{
													PduLengthType remainsize = 0;
													remainsize = s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx - s_stDoIPCfg.Connection_Table[id].pstRxBuff->trans_idx;
													stInfo_Ext.SduDataPtr = &s_stDoIPCfg.Connection_Table[id].pstRxBuff->pucBuff[s_stDoIPCfg.Connection_Table[id].pstRxBuff->trans_idx];
							
													if (bufferSizePtr_Ext > remainsize)
													{
														stInfo_Ext.SduLength = remainsize;
													}
													else
													{
														stInfo_Ext.SduLength = bufferSizePtr_Ext;
													}
													
													PdurSentSts = Dcm_CopyRxData(ucPduRIndex,&stInfo_Ext,&availableDataPtr_Ext);
													if (PdurSentSts != BUFREQ_OK)
													{
														*bufferSizePtr = 0;
														break;
													}
													else
													{
														s_stDoIPCfg.Connection_Table[id].pstRxBuff->trans_idx += stInfo_Ext.SduLength;
														*bufferSizePtr = availableDataPtr_Ext;
													}
												}
												/* SWS_DoIP_00218 */
												s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_DIRECT; 
												PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);
											}
											/* SWS_DoIP_00216 */
											else if ((BUFREQ_E_OVFL == PdurSentSts) || (BUFREQ_E_NOT_OK == PdurSentSts))
											{
												/* SWS_DoIP_00174 */
												*bufferSizePtr = 0;
												DoIP_TpCancelReceive(id);
												s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_IDLE;
												PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);
												/* SWS_DoIP_00217 */									
												Dcm_TpRxIndication(ucPduRIndex,E_NOT_OK);
											}
											else
											{
												*bufferSizePtr = 0;
												s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_IDLE;
												PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);
											}
										}
									} 
									else if (lookupResult == DOIP_E_DIAG_UNKNOWN_TA)
									{
										// TA not known
										DoIP_Inner_SendDiagMsgNack(id, SA, TA, DOIP_E_DIAG_UNKNOWN_TA,ulPayLoadLength);
										s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_IDLE;
										PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);
									} 
									else 
									{
										// SA not registered on receiving socket
										DoIP_Inner_SendDiagMsgNack(id, SA, TA, DOIP_E_DIAG_INVALID_SA,ulPayLoadLength);
										s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_IDLE;
										PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);
										DoIP_SocketClose(id);
									}
								} 
								else 
								{
									DoIP_Inner_SendHeaderNAck(id, DOIP_E_H_INVALID_PAYLOAD_LENGTH);
									s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_IDLE;
									PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);
									DoIP_SocketClose(id);
								}
								break;
							}
							
							default:
								DoIP_Inner_SendHeaderNAck(id, DOIP_E_H_UNKNOWN_PAYLOAD_TYPE);	/* SWS_DoIP_00016 */
								s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_IDLE;
								PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);
								break;
						}
					} 
					else 
					{
						DoIP_Inner_SendHeaderNAck(id, DOIP_E_H_MESSAGE_TOO_LARGE);	/* SWS_DoIP_00017 */ 
					}
				}
				else		/* UDP */
				{
					uint8  aucEID[DOIP_OFFSET_EID];
					uint8  aucVIN[DOIP_OFFSET_VIN];
				
					if ((ulPayLoadLength + DOIP_GH_TOTAL) <= DOIP_BUFFER_LENGTH_MAX) 
					{
						switch (uiPayLoadType) 
						{
							case DOIP_PT_VHCLIDFIC_REQMSG:	/* Vehicle Identification Request  SWS_DoIP_00061 */
								if (DOIP_PL_VHCLIDFIC_REQMSG == ulPayLoadLength)	/* SWS_DoIP_00059 */
								{
									DoIP_Inner_SendVehicleAnnouncement(id);	/* SWS_DoIP_00060 */
								}
								else
								{
									DoIP_Inner_SendHeaderNAck(id,DOIP_E_H_INVALID_PAYLOAD_LENGTH);
									DoIP_SocketClose(id);
								}
								break;

							case DOIP_PT_VHCLIDFIC_REQEID:	// Vehicle Identification Request with EID
								if (DOIP_PL_VHCLIDFIC_REQEID == ulPayLoadLength)
								{
									if (TRUE == DOIP_USEMACADDRESSFORIDENTIFICATION/*g_stDoIPCfg.DoIPGeneral.DoIPUseMacAddressForIdentification*/)	/* SWS_DoIP_00065 */
									{
										SoAd_GetPhysAddr(id,aucEID);
									}
									else	/* SWS_DoIP_00066 */
									{
										DoIP_Inner_MemCpy(aucEID, DOIPEID /* g_stDoIPCfg.DoIPConfigSet.DoIPEid */, DOIP_OFFSET_EID);
									}
									if (TRUE == DoIP_Inner_MemCmp(aucEID, &pucRxBuff[DOIP_GH_TOTAL], DOIP_OFFSET_EID))
									{
										DoIP_Inner_SendVehicleAnnouncement(id);	
									}
									else
									{
										//...
									}
								}
								else
								{
									DoIP_Inner_SendHeaderNAck(id,DOIP_E_H_INVALID_PAYLOAD_LENGTH);
									DoIP_SocketClose(id);
								}
								break;

							case DOIP_PT_VHCLIDFIC_REQVIN:	// Vehicle Identification Request with VIN
								if (DOIP_PL_VHCLIDFIC_REQVIN == ulPayLoadLength)	/* SWS_DoIP_00068 */
								{
									if (E_OK == Dcm_GetVin(aucVIN))
									{
										if (TRUE == DoIP_Inner_MemCmp(aucVIN, &pucRxBuff[DOIP_GH_TOTAL], DOIP_OFFSET_VIN))
										{
											DoIP_Inner_SendVehicleAnnouncement(id);	/* SWS_DoIP_00070 */											
										}
									}
								}
								else
								{
									DoIP_Inner_SendHeaderNAck(id,DOIP_E_H_INVALID_PAYLOAD_LENGTH);
									DoIP_SocketClose(id);
								}
								break;

							case DOIP_PT_ENTITYSTS_REQ:    /* DoIP entity status request */
								if (DOIP_PL_ENTITYSTS_REQ == ulPayLoadLength)	/* SWS_DoIP_00095 */
								{
									DoIP_Inner_SendEntityStatusResp(id, ulPayLoadLength, pucRxBuff);
								}
								else
								{
									DoIP_Inner_SendHeaderNAck(id,DOIP_E_H_INVALID_PAYLOAD_LENGTH);
									DoIP_SocketClose(id);
								}
								break;

							case DOIP_PT_PWRMODEINFO_REQ:	/* DoIP power mode check request */ 					
								if (DOIP_PL_PWRMODEINFO_REQ == ulPayLoadLength) /* SWS_DoIP_00091 */
								{
									DoIP_Inner_SendPowerModeCheckResp(id, ulPayLoadLength, pucRxBuff);
								}
								else
								{
									DoIP_Inner_SendHeaderNAck(id,DOIP_E_H_INVALID_PAYLOAD_LENGTH);
									DoIP_SocketClose(id);
								}
								break;

							default:
								DoIP_Inner_SendHeaderNAck(id, DOIP_E_H_UNKNOWN_PAYLOAD_TYPE); /* SWS_DoIP_00016 */
								break;
						}
					} 
					else 
					{
						DoIP_Inner_SendHeaderNAck(id, DOIP_E_H_MESSAGE_TOO_LARGE); /* SWS_DoIP_00017 */
					}
				}
			} 
			else
			{
				DoIP_Inner_SendHeaderNAck(id, DOIP_E_H_INCORRECT_PATTERN_FORMAT); /* SWS_DoIP_00014 */
				DoIP_SocketClose(id);
			}
		}
		else
		{
			
		}
	}

	return BUFREQ_OK;
}

/*===================================================================
 * Function name	: DoIP_SoAdTpStartOfReception
 * Description		: provide the currently available maximum buffer 
 					  size when invoked with TpSduLength equal to 0
 * Parameter(in)	: id, result, TpSduLength
 * Parameter(inout)	: -
 * Parameter(out)	: bufferSizePtr
 * Return value 	: BufReq_ReturnType
 * note             : Only when socket connection is opened
====================================================================*/
BufReq_ReturnType DoIP_SoAdTpStartOfReception(PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr)
{
	uint8 SERVICE_ID = DOIP_SOADTPSTARTOFRECEPTION;
//	uint8* pucRxBuffer;
//	uint8 ucPduRIndex = 0;
//	uint8 ucLoop = 0;
//	uint16 uiPayLoadType;
//    PduLengthType CopyLen = 0;;
//    PduInfoType info_Ext;
//    PduLengthType TpSduLength_Ext;
//    PduLengthType bufferSizePtr_Ext;

	PR_DEBUG(DEBUG_SWITCH, "%s  id : %d.\n", __func__,id);

	/* SWS_DoIP_00186 */
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
		return BUFREQ_E_NOT_OK;
	}
	/* SWS_DoIP_00202 */
	if (DOIP_ACTIVATION_LINE_INACTIVE == s_ucActivationLineStatus)
	{
		return BUFREQ_E_NOT_OK;
	}
	/* SWS_DoIP_00187 */
	if (id >= DOIP_SOAD_CONNECTIONS_MAX)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_INVALID_PDU_SDU_ID);
		return BUFREQ_E_NOT_OK;
	}
	/* SWS_DoIP_00188 */
	if(NULL_PTR == bufferSizePtr)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_PARAM_POINTER);
		return BUFREQ_E_NOT_OK;	
	}
	PR_DEBUG(DEBUG_SWITCH, "Conctsts : %d, Buffsts %d,  TpSduLength : %d\n",s_stDoIPCfg.Connection_Table[id].ucConnectionState,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status,TpSduLength);
#if 1	
	if (DOIP_CONCTS_LISTEN == s_stDoIPCfg.Connection_Table[id].ucConnectionState)
	{
		if (0 != TpSduLength)		/* SWS_DoIP_00189 */
		{
			DET_REPORT_ERROR(SERVICE_ID,DOIP_E_INVALID_PARAMETER);
			return BUFREQ_E_NOT_OK;			
		}
		else
		{
			s_ucDoIP_CurrProtocol = id;
			s_stDoIPCfg.Connection_Table[id].ucConnectionState = DOIP_CONCTS_SOCKET_INITIALIZED;
			PR_DEBUG(DEBUG_SWITCH, "%d-----ucConnectionState:%d\n", __LINE__,s_stDoIPCfg.Connection_Table[id].ucConnectionState);
			s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx = 0;
			s_stDoIPCfg.Connection_Table[id].pstRxBuff->total_Len = 0;
			*bufferSizePtr = DOIP_BUFFER_LENGTH_MAX - s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx;
			s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_IDLE;
			PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);
			return BUFREQ_OK;
		}
	}
	/* SWS_DoIP_00207 */
	else	
#endif
	{
	PR_DEBUG(DEBUG_SWITCH, "stus %d,  TpSduLength : %d\n", s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status,TpSduLength);
		if (s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status == DOIP_BUFFSTS_RX_IDLE) 
		{	
			if (0 == TpSduLength)
			{
				DET_REPORT_ERROR(SERVICE_ID,DOIP_E_INVALID_PARAMETER);
				return BUFREQ_E_NOT_OK;			
			}
		/*	else if (TpSduLength > DOIP_BUFFER_LENGTH_MAX)
			{
				DET_REPORT_ERROR(SERVICE_ID,DOIP_E_INVALID_PARAMETER);
				return BUFREQ_E_OVFL;
			}  */
			else
			{
				s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx = 0;
				s_stDoIPCfg.Connection_Table[id].pstRxBuff->total_Len = TpSduLength;
				*bufferSizePtr = DOIP_BUFFER_LENGTH_MAX - s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx;
				s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_START;
				PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);
				return BUFREQ_OK;				
			}


			#if 0
			pucRxBuffer = s_stDoIPCfg.Connection_Table[id].pstRxBuff->pucBuff;

			if (info->SduLength > DOIP_BUFFER_LENGTH_MAX)
			{
				CopyLen = DOIP_BUFFER_LENGTH_MAX;
			}
			else
			{
				CopyLen = info->SduLength;
			}
			DoIP_Inner_MemCpy(pucRxBuffer, info->SduDataPtr, CopyLen);
			s_stDoIPCfg.Connection_Table[id].pstRxBuff->total_Len = CopyLen;
			*bufferSizePtr -= CopyLen;
			PR_DEBUG(DEBUG_SWITCH, "LINE %d: First time---info->SduLength = %d\n", __LINE__, info->SduLength);

			/* SWS_DoIP_00214 */
			if (s_stDoIPCfg.Connection_Table[id].pstRxBuff->total_Len > (DOIP_GH_TOTAL + DOIP_OFFSET_SA + DOIP_OFFSET_TA))
			{
				PR_DEBUG(DEBUG_SWITCH, "%d:---Ver[0],[1] = %d, %d\n", __LINE__, pucRxBuffer[DOIP_GH_PROTOCOL_VERSION], pucRxBuffer[DOIP_GH_INVERSE_PROTOCOL_VERSION]);
				if ( (DOIP_PROTOCOL_VERSION 		== pucRxBuffer[DOIP_GH_PROTOCOL_VERSION]) 
				  && (DOIP_INVERSE_PROTOCOL_VERSION == pucRxBuffer[DOIP_GH_INVERSE_PROTOCOL_VERSION]))
				{
					PR_DEBUG(DEBUG_SWITCH, "%d...\n", __LINE__);
					uiPayLoadType = pucRxBuffer[2] << 8 | pucRxBuffer[3];
					if (uiPayLoadType != DOIP_PT_DIAGMSG)
					{
						PR_DEBUG(DEBUG_SWITCH, "%d...\n", __LINE__);
						/* SWS_DoIP_00219 */
						s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_COPY;
					}
					else					
					{
						/* SWS_DoIP_00212 */
						s_stDoIPCfg.Connection_Table[id].uiTA = (pucRxBuffer[10] << 8) | pucRxBuffer[11];
						for (ucLoop = 0; ucLoop < DOIP_UPLA_CHANNEL_MAX; ucLoop++)
						{
							if (s_stDoIP_TAChannel[ucLoop].uiTA == s_stDoIPCfg.Connection_Table[id].uiTA)
							{
								ucPduRIndex = s_stDoIP_TAChannel[ucLoop].ucPduRRxPduId;
								break;
							}
						}
						PR_DEBUG(DEBUG_SWITCH, "%s : %d : %d\n", __func__,__LINE__,ucPduRIndex);
						if (ucPduRIndex < DOIP_UPLA_CHANNEL_MAX)
						{
							info_Ext.SduLength = (pucRxBuffer[4] << 24) | (pucRxBuffer[5] << 16) | (pucRxBuffer[6] << 8) | pucRxBuffer[7];
							info_Ext.SduDataPtr = info->SduDataPtr;
							TpSduLength_Ext = info->SduLength;
							PR_DEBUG(DEBUG_SWITCH, "%d...\n", __LINE__);
							Dcm_StartOfReception(ucPduRIndex,&info_Ext,TpSduLength_Ext,&bufferSizePtr_Ext);
							*bufferSizePtr = bufferSizePtr_Ext;
							PR_DEBUG(DEBUG_SWITCH, "bufferSizePtr_Ext = %d...\n", bufferSizePtr_Ext);
						}
					}
				}				
			}
			PR_DEBUG(DEBUG_SWITCH, "%d...\n", __LINE__);
			return BUFREQ_OK;	
			#endif
		}
		else
		{
			
		}
	}

	return BUFREQ_OK;
}

/*===================================================================
 * Function name	: DoIP_SoAdTpRxIndication
 * Description		: the result indicates whether the transmission 
 					  was successful or not.
 					  SWS_DoIP_00200 
 * Parameter(in)	: id, result
 * Parameter(inout)	: -
 * Parameter(out)	: -
 * Return value 	: -
 * note             : only called once when the socket is closed
====================================================================*/
void DoIP_SoAdTpRxIndication(PduIdType id, Std_ReturnType result)
{
	uint8 SERVICE_ID = DOIP_SOADTPRXINDICATION;
	uint8 pdurid = 0;
	uint8 ucLoop = 0;
	/* SWS_DoIP_00162 */
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
	}
	else
	{
		PR_DEBUG(DEBUG_SWITCH, "%s  id : %d.\n", __func__,id);
		/* SWS_DoIP_00200 */
		if ((DOIP_BUFFSTS_RX_START == s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status)
		|| (DOIP_BUFFSTS_RX_COPY == s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status)
		|| (DOIP_BUFFSTS_RX_DIRECT == s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status))
		{
			for (ucLoop = 0; ucLoop < DOIP_UPLA_CHANNEL_MAX; ucLoop++)
			{
				if (s_stDoIP_TAChannel[ucLoop].uiTA == s_stDoIPCfg.Connection_Table[id].uiTA)
				{
					pdurid = s_stDoIP_TAChannel[ucLoop].ucPduRRxPduId;
					break;
				}
			}
			s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_IDLE;
			PR_DEBUG(DEBUG_SWITCH, "%d  status : %d\n",__LINE__,s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status);
			DoIP_Inner_MemSet(s_stDoIPCfg.Connection_Table[id].pstRxBuff, 0, sizeof(s_stDoIPCfg.Connection_Table[id].pstRxBuff));
			Dcm_TpRxIndication(pdurid,result);
		}
	
	}
}


/*===================================================================
 * Function name	: DoIP_SoAdIfRxIndication
 * Description		: Indication of a received PDU from a lower layer
 					  communication interface module.
 * Parameter(in)	: RxPduId, PduInfoPtr
 * Parameter(inout)	: -
 * Parameter(out)	: -
 * Return value 	: -
====================================================================*/
void DoIP_SoAdIfRxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr)
{
	uint8  SERVICE_ID = DOIP_SOADIFRXINDICATION;
	uint8  ucIndex;
	uint16 uiPayLoadType;
	uint32 ulPayLoadLength;
	uint8  aucEID[DOIP_OFFSET_EID];
	uint8  aucVIN[DOIP_OFFSET_VIN];
	uint8* UDPRxBuffer;
//	uint8  ucLoop;
	uint8 SoConId;

	PR_DEBUG(DEBUG_SWITCH, "%s  id : %d.\n", __func__,RxPduId);
	
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)	/* SWS_DoIP_00246 */
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
	}
	else
	{
		if (DOIP_ACTIVATION_LINE_INACTIVE == s_ucActivationLineStatus)	/* SWS_DoIP_00202 */
		{
		}
		else
		{
			for (ucIndex = 0; ucIndex < DOIP_SOAD_CONNECTIONS_MAX; ucIndex++) /* SWS_DoIP_00247 */				
			{
				if (s_stDoIPCfg.Connection_Table[ucIndex].DoIPSoAdPduId == RxPduId)
				{
					break;
				}
			}
			if (DOIP_SOAD_CONNECTIONS_MAX == ucIndex)
			{
				DET_REPORT_ERROR(SERVICE_ID,DOIP_E_INVALID_PDU_SDU_ID);
			}
			else
			{	
				if (PduInfoPtr == NULL_PTR)   /* SWS_DoIP_00248 */
				{
					DET_REPORT_ERROR(SERVICE_ID,DOIP_E_PARAM_POINTER);
				}
				else if (PduInfoPtr->SduDataPtr == NULL_PTR)
				{
					DET_REPORT_ERROR(SERVICE_ID,DOIP_E_PARAM_POINTER);
				}
				else
				{
				
				#if 0	/* SWS_DoIP_00276 *///...to do...
					if (ucLoop = 0; ucLoop < g_stDoIPCfg.DoIPGeneral.DoIPMaxUDPRequestPerMessage; ucLoop++)
					{
						if (g_ucUdpRxState[ucLoop] == DOIP_UDPRXSTATE_BUSY)
						{

						}
					}
				#endif
					/* SWS_DoIP_00197 */
					UDPRxBuffer = s_stDoIPCfg.Connection_Table[ucIndex].pstRxBuff->pucBuff;
					SoConId = s_stDoIPCfg.Connection_Table[ucIndex].ucSoConId;
					//UDPRxBuffer = DoIP_GetUdpBuffer(RxPduId);//&g_ucUDPRxBuffer[0];//g_stDoIPCfg.DoIPConfigSet.DoIPConnections[].DoIPUdpConnection.DoIPSoAdRxPdu.DoIPSoAdRxPduRef.SduDataPtr;
					if ( (DOIP_PROTOCOL_VERSION         == UDPRxBuffer[DOIP_GH_PROTOCOL_VERSION]) 
					  && (DOIP_INVERSE_PROTOCOL_VERSION == UDPRxBuffer[DOIP_GH_INVERSE_PROTOCOL_VERSION]) ) 
					{
						uiPayLoadType   = BTYE2INT(UDPRxBuffer[DOIP_GH_PAYLOAD_TYPE],UDPRxBuffer[DOIP_GH_PAYLOAD_TYPE+1]);
						ulPayLoadLength = BTYE2WORD(UDPRxBuffer[DOIP_GH_PAYLOAD_LENGTH],UDPRxBuffer[DOIP_GH_PAYLOAD_LENGTH+1],UDPRxBuffer[DOIP_GH_PAYLOAD_LENGTH+2],UDPRxBuffer[DOIP_GH_PAYLOAD_LENGTH+3]);
						
						if ((ulPayLoadLength + DOIP_GH_TOTAL) <= DOIP_BUFFER_LENGTH_MAX) 
						{
							switch (uiPayLoadType) 
							{
								case DOIP_PT_VHCLIDFIC_REQMSG:	/* Vehicle Identification Request  SWS_DoIP_00061 */
									if (DOIP_PL_VHCLIDFIC_REQMSG == ulPayLoadLength)	/* SWS_DoIP_00059 */
									{
										DoIP_Inner_SendVehicleAnnouncement(SoConId);	/* SWS_DoIP_00060 */
									}
									else
									{
										DoIP_Inner_SendHeaderNAck(SoConId,DOIP_E_H_INVALID_PAYLOAD_LENGTH);
										DoIP_SocketClose(SoConId);
									}
									break;

								case DOIP_PT_VHCLIDFIC_REQEID:	// Vehicle Identification Request with EID
									if (DOIP_PL_VHCLIDFIC_REQEID == ulPayLoadLength)
									{
										if (TRUE == DOIP_USEMACADDRESSFORIDENTIFICATION/*g_stDoIPCfg.DoIPGeneral.DoIPUseMacAddressForIdentification*/)	/* SWS_DoIP_00065 */
										{
											SoAd_GetPhysAddr(SoConId,aucEID);
										}
										else	/* SWS_DoIP_00066 */
										{
											DoIP_Inner_MemCpy(aucEID, DOIPEID /* g_stDoIPCfg.DoIPConfigSet.DoIPEid */, DOIP_OFFSET_EID);
										}
										if (TRUE == DoIP_Inner_MemCmp(aucEID, &UDPRxBuffer[DOIP_GH_TOTAL], DOIP_OFFSET_EID))
										{
											DoIP_Inner_SendVehicleAnnouncement(SoConId);	
										}
										else
										{
											//...
										}
									}
									else
									{
										DoIP_Inner_SendHeaderNAck(SoConId,DOIP_E_H_INVALID_PAYLOAD_LENGTH);
										DoIP_SocketClose(SoConId);
									}
									break;

								case DOIP_PT_VHCLIDFIC_REQVIN:	// Vehicle Identification Request with VIN
									if (DOIP_PL_VHCLIDFIC_REQVIN == ulPayLoadLength)	/* SWS_DoIP_00068 */
									{
									//	if (E_OK == Dcm_GetVin(aucVIN))
										{
											if (TRUE == DoIP_Inner_MemCmp(aucVIN, &UDPRxBuffer[DOIP_GH_TOTAL], DOIP_OFFSET_VIN))
											{
												DoIP_Inner_SendVehicleAnnouncement(SoConId);	/* SWS_DoIP_00070 */											
											}
										}
									}
									else
									{
										DoIP_Inner_SendHeaderNAck(SoConId,DOIP_E_H_INVALID_PAYLOAD_LENGTH);
										DoIP_SocketClose(SoConId);
									}
									break;

								case DOIP_PT_ENTITYSTS_REQ:	   /* DoIP entity status request */
									if (DOIP_PL_ENTITYSTS_REQ == ulPayLoadLength)	/* SWS_DoIP_00095 */
									{
										DoIP_Inner_SendEntityStatusResp(SoConId, ulPayLoadLength, UDPRxBuffer);
									}
									else
									{
										DoIP_Inner_SendHeaderNAck(SoConId,DOIP_E_H_INVALID_PAYLOAD_LENGTH);
										DoIP_SocketClose(SoConId);
									}
									break;

								case DOIP_PT_PWRMODEINFO_REQ:	/* DoIP power mode check request */						
									if (DOIP_PL_PWRMODEINFO_REQ == ulPayLoadLength)	/* SWS_DoIP_00091 */
									{
										DoIP_Inner_SendPowerModeCheckResp(SoConId, ulPayLoadLength, UDPRxBuffer);
									}
									else
									{
										DoIP_Inner_SendHeaderNAck(SoConId,DOIP_E_H_INVALID_PAYLOAD_LENGTH);
										DoIP_SocketClose(SoConId);
									}
									break;

								default:
									DoIP_Inner_SendHeaderNAck(SoConId, DOIP_E_H_UNKNOWN_PAYLOAD_TYPE); /* SWS_DoIP_00016 */
									break;
							}
						} 
						else 
						{
							DoIP_Inner_SendHeaderNAck(SoConId, DOIP_E_H_MESSAGE_TOO_LARGE);	/* SWS_DoIP_00017 */
						}
					} 
					else 
					{
						DoIP_Inner_SendHeaderNAck(SoConId, DOIP_E_H_INCORRECT_PATTERN_FORMAT); /* SWS_DoIP_00014 */
						DoIP_SocketClose(SoConId);
					}											
				}
			}			
		}
	}
}


/*===================================================================
 * Function name	: DoIP_SoAdIfTxConfirmation
 * Description		: The lower layer communication interface module
 					  confirms the transmission of a PDU, or the 
 					  failure to transmit a PDU.
 * Parameter(in)	: RxPduId, result
 * Parameter(inout)	: -
 * Parameter(out)	: -
 * Return value 	: -
====================================================================*/
void DoIP_SoAdIfTxConfirmation(PduIdType TxPduId, Std_ReturnType result)
{
	uint8 SERVICE_ID = DOIP_SOADIFTXCONFIRMATION;
	uint8 ucSoAdIndex = 0;
	uint8 pdurid = TxPduId;
		PR_DEBUG(DEBUG_SWITCH, "%s  id : %d.\n", __func__,TxPduId);
	/* SWS_DoIP_00162 */
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
		return;
	}
	else
	{
		/* SWS_DoIP_00199 */
		for (ucSoAdIndex = 0; ucSoAdIndex < DOIP_SOAD_CONNECTIONS_MAX; ++ucSoAdIndex)
		{
			if (TxPduId == s_stDoIPCfg.Connection_Table[ucSoAdIndex].DoIPSoAdPduId)
			{
				s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff->Status = DOIP_BUFFSTS_TX_IDLE;
				DoIP_Inner_MemSet(s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff, 0, sizeof(s_stDoIPCfg.Connection_Table[ucSoAdIndex].pstTxBuff));
				Dcm_TpTxConfirmation(pdurid,result);
				break;
			}
		}
	}
	if (E_OK == result)
	{
		#if 0
		/* SWS_DoIP_00286 */
		if (AnnouncementTransmitted == LastIfTransmit)
		{
			if (s_AnnouncementCnt <DoIPVehicleAnnouncementRepetition)
			{
				s_AnnouncementCnt++;
			}
		}
		#endif
	}
}

/*===================================================================
 * Function name	: DoIP_SoConModeChg
 * Description		: Notification about a SoAd socket connection state change
 * Parameter(in)	: SoConId, Mode
 * Parameter(inout)	: -
 * Parameter(out)	: -
 * Return value 	: -
====================================================================*/
void DoIP_SoConModeChg(SoAd_SoConIdType SoConId, SoAd_SoConModeType Mode)
{
	uint8 SERVICE_ID = DOIP_SOCONMODECHG;
	uint8 ucIndex;
	PduIdType TxPduId;
	PR_DEBUG(DEBUG_SWITCH, "%s %d  %d ", __func__,SoConId,Mode);

	/* SWS_DoIP_00162 */
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID, DOIP_E_UNINIT);
		return;
	}

	for (ucIndex = 0; ucIndex < DOIP_SOAD_CONNECTIONS_MAX; ++ucIndex)
	{
		if (SoConId == s_stDoIPCfg.Connection_Table[ucIndex].ucSoConId)
		{
			TxPduId = s_stDoIPCfg.Connection_Table[ucIndex].DoIPSoAdPduId;
			break;
		}
	}
	PR_DEBUG(DEBUG_SWITCH, "  %d\n", TxPduId);


	if (SOAD_SOCON_ONLINE /* SOAD_SOCON_ONLINE */ == Mode)	/* SWS_DoIP_00241 */
	{
		s_ucDoIP_CurrProtocol = TxPduId;
		//s_stDoIPCfg.Connection_Table[TxPduId].ucConnectionState = DOIP_CONCTS_SOCKET_INITIALIZED /* SOAD_SOCON_ONLINE */;
		/* SWS_DoIP_00143 */
		s_stDoIPCfg.Connection_Table[TxPduId].ulInactivityTimer = DOIP_INITIALINACTIVITYTIME /* g_stDoIPCfg.DoIPGeneral.DoIPInitialInactivityTime */;	/* Start */
			/* or */
		s_stDoIPCfg.Connection_Table[TxPduId].ulInitialInactivityTimer = DOIP_INITIALINACTIVITYTIME /* g_stDoIPCfg.DoIPGeneral.DoIPInitialInactivityTime */;	/* Start */
		/* SWS_DoIP_00205 */ /* SWS_DoIP_00003 has been performed */
		#if 0
		for ()
		{
			if ()
			{
				TxPduId = g_stDoIPCfg.DoIPConfigSet.DoIPConnections[].DoIPUdpVehicleAnnouncementConnection.DoIPSoAdUdpVehicleAnnouncementTxPdu.DoIPSoAdUdpVehicleAnnouncementTxPduId;	
			}
		}
		#endif
	//	DoIP_Inner_SendVehicleAnnouncement(SoConId);
	}
	else if(SOAD_SOCON_RECONNECT == Mode)
	{
		(void)SoAd_OpenSoCon(SoConId);
	}
	else	/* SWS_DoIP_00243 */
	{
		s_ucDoIP_CurrProtocol = 0xff;
		s_stDoIPCfg.Connection_Table[TxPduId].ucConnectionState = DOIP_CONCTS_LISTEN /* SOAD_SOCON_OFFLINE */;
		PR_DEBUG(DEBUG_SWITCH, "%d-----ucConnectionState:%d\n", __LINE__,s_stDoIPCfg.Connection_Table[TxPduId].ucConnectionState);
		/* SWS_DoIP_00115 */
		
		/*
		*- unregister and release the socket connection to the related Tester,
		*- discard the ongoing diagnostic message processing and
		*- reset the inactivity timer of the given socket connection
		*/
		
		//TesterCnt--;
		/* SWS_DoIP_00235 */
		(void)SoAd_GetSoConId(TxPduId,&SoConId);
		(void)SoAd_ReleaseIpAddrAssignment(SoConId);	
	}
}

/*===================================================================
* Function name		: DoIP_LocalIpAddrAssignmentChg
* Description		: called by the SoAd if an IP address assignment related to a
					  socket connection changes. called after SWS_DoIP_00204
* Parameter(in)		: SoConId, State
* Parameter(inout)	: -
* Parameter(out)	: -
* Return value 	: -
====================================================================*/
void DoIP_LocalIpAddrAssignmentChg(SoAd_SoConIdType SoConId,TcpIp_IpAddrStateType State)
{
//	uint8 SERVICE_ID = DOIP_LOCALIPADDRASSIGNMENTCHG;
	uint8 length;
	uint8 data[255];
	const char* HostName = "DoIP-";
	const char* VINName = "DoIP-VIN";

	PR_DEBUG(DEBUG_SWITCH, "%s !!!\n", __func__);
	switch (State)
	{
		case TCPIP_IPADDR_STATE_ASSIGNED:	
			LinkStatus = DOIP_LINK_UP;
			/* SWS_DoIP_00154 */
		//	SoAd_ReadDhcpHostNameOption(SoConId,&length,data);
			if (TRUE == strncmp(HostName, (const char *)data, 5))
			{
				
			}
			else /* SWS_DoIP_00155 */
			{
				if (FALSE == DOIP_DHCPOPTIONVINUSE /* g_stDoIPCfg.DoIPGeneral.DoIPDhcpOptionVinUse */)
				{
					length = 5;
				//	data = HostName;
				//	SoAd_WriteDhcpHostNameOption(SoConId,length,data);
				}
				else /* SWS_DoIP_00156 */
				{
					DoIP_Inner_MemCpy(data, VINName, sizeof(VINName));
				//	if (E_NOT_OK == Dcm_GetVin(&data[sizeof(VINName)]))
					{
						DoIP_Inner_MemCpy(&data[sizeof(VINName)], DOIP_VININVALIDITYPATTERN /* g_stDoIPCfg.DoIPGeneral.DoIPVinInvalidityPattern */ , DOIP_OFFSET_VIN);
					}
					/* VIN should be in ASCLL */
					SoAd_WriteDhcpHostNameOption(SoConId,length,data);
				}
			}
			
			/* SWS_DoIP_00003 */
			SoAd_OpenSoCon(SoConId);
			break;
		default:	/* SWS_DoIP_00085 */
			LinkStatus = DOIP_LINK_DOWN;
			DoIp_AnnouncementTimer = 0xffff;
			break;
	}

}


/*===================================================================
* Function name		: DoIP_ActivationLineSwitchActive
* Description		: notify the DoIP on a switch of the 
					  DoIPActivationLine to active
* Parameter(in)		: -
* Parameter(inout) 	: -
* Parameter(out)	: -
* Return value 		: -
====================================================================*/
void DoIP_ActivationLineSwitchActive(void)	
{
	uint8 SERVICE_ID = DOIP_ACTIVATIONLINESWITCHACTIVE;
	
	PduIdType TxPduId;
	SoAd_SoConIdType SoConId;
	uint8 Netmask;
	TcpIp_SockAddrType DefaultRouter;
	TcpIp_SockAddrType LocalIpAddrDoIP;
	
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
	}

	/* SWS_DoIP_00204 */	
	/* 如果DoIP激活线路状态从DOIP_ACTIVATION_LINE_INACTIVE改变为DOIP_ACTIVATION_LINE_ACTIVE，
	   DoIP模块应通过调用SoAd_GetSoConId检索第一个已配置的UDPConnection的SoConId，并通过对检
	   索到的SoConId执行SoAd_RequestIpAddrAssignment的两次后续调用来触发IP地址分配，
	   LocalIpAddrPtr设置为NULL_PTR，
	   第一个调用类型设置为TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP，
	   第二个调用类型设置为TCPIP_IPADDR_ASSIGNMENT_DHCP。 */

#if ENABLE_TCP


	/* --------------TCP----------------------- */

	TxPduId = 0;

	Netmask = 0;
	
	(void)SoAd_GetSoConId(TxPduId,&SoConId);
	PR_DEBUG(DEBUG_SWITCH, "=======SoConId=======\n");
	PR_DEBUG(DEBUG_SWITCH, "SoConId = %u\n", SoConId);
	PR_DEBUG(DEBUG_SWITCH, "=======SoConId=======\n");
	(void)SoAd_RequestIpAddrAssignment(SoConId,TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP,&LocalIpAddrDoIP,Netmask,&DefaultRouter);
 	{
 		char String[20] = "";
		TcpIp_SockAddrInetType *IPADDR = (TcpIp_SockAddrInetType *)&LocalIpAddrDoIP;
 		IPN NetIpaddr = (IPN)IPADDR->addr.s_addr;
		NetIpaddr = htonl(NetIpaddr);
		snprintf(String, sizeof(String), "%d.%d.%d.%d",
				(UINT8)((NetIpaddr>>24)&0xFF), (UINT8)((NetIpaddr>>16)&0xFF),
				(UINT8)((NetIpaddr>>8)&0xFF), (UINT8)(NetIpaddr&0xFF));			
		PR_DEBUG(DEBUG_SWITCH, "DoIP request IPADDR:%s !!!\n", String);
 	}
	(void)SoAd_RequestIpAddrAssignment(SoConId,TCPIP_IPADDR_ASSIGNMENT_DHCP,NULL_PTR,Netmask,&DefaultRouter);
	
#else
	/* ------------UDP-------------------- */

	TxPduId = 1;
	Netmask = 0;
	(void)SoAd_GetSoConId(TxPduId,&SoConId);
	PR_DEBUG(DEBUG_SWITCH, "=======SoConId=======\n");
	PR_DEBUG(DEBUG_SWITCH, "SoConId = %u\n", SoConId);
	PR_DEBUG(DEBUG_SWITCH, "=======SoConId=======\n");
	(void)SoAd_RequestIpAddrAssignment(SoConId,TCPIP_IPADDR_ASSIGNMENT_LINKLOCAL_DOIP,&LocalIpAddrDoIP,Netmask,&DefaultRouter);
 	{
 		char String[20] = "";
		TcpIp_SockAddrInetType *IPADDR = (TcpIp_SockAddrInetType *)&LocalIpAddrDoIP;
 		IPN NetIpaddr = (IPN)IPADDR->addr.s_addr;
		NetIpaddr = htonl(NetIpaddr);
		snprintf(String, sizeof(String), "%d.%d.%d.%d",
				(UINT8)((NetIpaddr>>24)&0xFF), (UINT8)((NetIpaddr>>16)&0xFF),
				(UINT8)((NetIpaddr>>8)&0xFF), (UINT8)(NetIpaddr&0xFF));			
		PR_DEBUG(DEBUG_SWITCH, "DoIP request IPADDR:%s !!!\n", String);
 	}
	(void)SoAd_RequestIpAddrAssignment(SoConId,TCPIP_IPADDR_ASSIGNMENT_DHCP,NULL_PTR,Netmask,&DefaultRouter);
#endif

	s_ucActivationLineStatus = DOIP_ACTIVATION_LINE_ACTIVE;
}


/*===================================================================
* Function name		: DoIP_ActivationLineSwitchInactive
* Description		: notify the DoIP on a switch of the 
					  DoIPActivationLine to inactive
* Parameter(in)		: -
* Parameter(inout) 	: -
* Parameter(out)	: -
* Return value 		: -
====================================================================*/
void DoIP_ActivationLineSwitchInactive(void)
{
	uint8 SERVICE_ID = DOIP_ACTIVATIONLINESWITCHINACTIVE;
	SoAd_SoConIdType SoConId = 0;
	PduIdType TxPduId;
	
	/* SWS_DoIP_00162 */
	if (s_stDoIPCfg.InitStatus != DOIP_STATES_INITED)
	{
		DET_REPORT_ERROR(SERVICE_ID,DOIP_E_UNINIT);
	}

#if ENABLE_TCP
		TxPduId = 0;
#else
		TxPduId = 1;
#endif

	
	/* SWS_DoIP_00234 */
	//while()
	{
		(void)SoAd_GetSoConId(TxPduId,&SoConId);
		SoAd_CloseSoCon(SoConId, TRUE);
		//DoIP_SocketClose();
	}

	s_ucActivationLineStatus = DOIP_ACTIVATION_LINE_INACTIVE;
}


/*===================================================================
 * Function name	: DoIP_MainFunction
 * Description		: Schedules the Diagnostic over IP module
 * Parameter(in)	: -
 * Parameter(inout)	: -
 * Parameter(out)	: -
 * Return value 	: -
====================================================================*/
void DoIP_MainFunction(void)
{
	static uint16 numAnnouncements = 3;
	uint16 i = 0;
	uint32 timer = 0;
    
	PR_DEBUG(DEBUG_SWITCH, "LinkStatus: %s\n", LinkStatus == DOIP_LINK_UP ? "DOIP_LINK_UP" : "DOIP_LINK_DOWN");
	if (DOIP_LINK_UP == LinkStatus) 
	{
		DoIp_AnnouncementTimer += DOIP_MAINFUNCTIONPERIOD;
		if ( (DoIp_AnnounceWait <= DoIp_AnnouncementTimer) 
		  &&((DoIp_AnnounceWait + DOIP_VEHICLEANNOUNCEMENTREPETITION * DOIP_VEHICLEANNOUNCEMENTINTERVAL) > DoIp_AnnouncementTimer) )
		{
			/* Within announcement period */

			timer = DoIp_AnnouncementTimer - DoIp_AnnounceWait;

			if (numAnnouncements * DOIP_VEHICLEANNOUNCEMENTINTERVAL <= timer) 
			{
				/* Time to send another announcement */
				DoIP_Inner_SendVehicleAnnouncement(0);
				numAnnouncements++;
			}
		} 
		else 
		{
			numAnnouncements = 0;
		}
	} 
	else 
	{
		numAnnouncements = 0;
	}
	
	/* SWS_DoIP_00076 */
	if ((TRUE == DOIP_VINGIDMASTER /* g_stDoIPCfg.DoIPGeneral.DoIPVinGidMaster */)
		&&(TRUE != s_stDoIPCfg.Connection_Table[i].ucGIDSyncStatus)
		&&(DOIP_CONCTS_LISTEN == s_stDoIPCfg.Connection_Table[i].ucConnectionState)
		&&(DOIP_LINK_UP == LinkStatus))
	{
	//	if (NULL_PTR != g_stDoIPCfg.DoIPGeneral.DoIPTriggerGidSyncDirect)
		{
	//		if (E_OK == g_stDoIPCfg.DoIPGeneral.DoIPTriggerGidSyncDirect())
			{
				s_stDoIPCfg.Connection_Table[i].ucGIDSyncStatus = TRUE;
			}
		}
	}

	
	/* Handle DoIP connection timeouts... */
	for (i = 0; i < DOIP_SOAD_CONNECTIONS_MAX; i++) 
	{
		if ((DOIP_CONCTS_REGISTERED_PEND_AUTHEN == s_stDoIPCfg.Connection_Table[i].ucConnectionState)
		  ||(DOIP_CONCTS_REGISTERED_PEND_CONFIRM == s_stDoIPCfg.Connection_Table[i].ucConnectionState)
		  ||(DOIP_CONCTS_REGISTERED_ROUTING_ALIVE == s_stDoIPCfg.Connection_Table[i].ucConnectionState))
		{
			if (TRUE == s_stDoIPCfg.Connection_Table[i].ucAwaitingAliveCheckResponse) 
			{
				if (s_stDoIPCfg.Connection_Table[i].ulAliveCheckTimer > DOIP_ALIVECHECKRESPONSETIMEOUT) 
				{
					handleTimeout(i);
				} 
				else 
				{
					s_stDoIPCfg.Connection_Table[i].ulAliveCheckTimer += DOIP_MAINFUNCTIONPERIOD;
				}
			}

			if ((s_stDoIPCfg.Connection_Table[i].ulGeneralInactivityTimer <= DOIP_GENERALINACTIVITYTIME)
			 || (s_stDoIPCfg.Connection_Table[i].ulGeneralInactivityTimer <= DOIP_ALIVECHECKRESPONSETIMEOUT))
			{
				s_stDoIPCfg.Connection_Table[i].ulGeneralInactivityTimer += DOIP_MAINFUNCTIONPERIOD;
			} 
			else 
			{
				handleTimeout(i);
			}

			if (s_stDoIPCfg.Connection_Table[i].ulInitialInactivityTimer <= DOIP_ALIVECHECKRESPONSETIMEOUT)
			{
				s_stDoIPCfg.Connection_Table[i].ulInitialInactivityTimer += DOIP_MAINFUNCTIONPERIOD;
			}
			else 
			{
				/* SWS_DoIP_00144 */
				if (0 == s_stDoIPCfg.Connection_Table[i].ucAllRoutingActivationStatus)
				{
					DoIP_SocketClose(s_stDoIPCfg.Connection_Table[i].ucSoConId);
				}
				//handleTimeout(i);
			}

			if (s_stDoIPCfg.Connection_Table[i].ulInitialInactivityTimer <= DOIP_GENERALINACTIVITYTIME)
			{
				s_stDoIPCfg.Connection_Table[i].ulInitialInactivityTimer += DOIP_MAINFUNCTIONPERIOD;
			}
			else 
			{
				/* SWS_DoIP_00146 */
				DoIP_SocketClose(s_stDoIPCfg.Connection_Table[i].ucSoConId);
				//handleTimeout(i);
			}
		}
	}

	
}





/*
** ===================================================================
**	   Static function definition.
** ===================================================================
*/

#if 0
static uint8* GetBuff(uint8 bufferType, uint8 ucConnectTyp)
{
#if 0
	switch (bufferType)
	{
		case DOIP_BUFFTYP_TX:
			if (DOIP_BUFFSTS_IDLE == s_stDoIPTxBuffer[ucConnectTyp].Status)
			{
				s_stDoIPTxBuffer[ucConnectTyp].Status = DOIP_BUFFSTS_TX_READY_TO_SEND;
				return s_stDoIPTxBuffer[ucConnectTyp].pucBuff;
			}					
		break;
		case DOIP_BUFFTYP_RX:
			if (DOIP_BUFFSTS_RX_IDLE == s_stDoIPRxBuffer[ucConnectTyp].Status)
			{
				s_stDoIPRxBuffer[ucConnectTyp].Status = DOIP_BUFFSTS_BUSY;
				return s_stDoIPRxBuffer[ucConnectTyp].pucBuff;
			}
		break;
		default:
			return NULL_PTR;
		break;
	}
#endif
			return NULL_PTR;
}

static void ClrBuff(uint8 bufferType,uint8 ucConnectTyp)
{
#if 0

	*pp = NULL_PTR;
	switch (bufferType)
	{
		case DOIP_BUFFTYP_TX:
		//	s_stDoIPTxBuffer[ucConnectTyp].pucBuff = NULL_PTR;
			DoIP_Inner_MemSet(&s_stDoIPTxBuffer[ucConnectTyp],0,sizeof(s_stDoIPTxBuffer[ucConnectTyp]));			
		break;
		case DOIP_BUFFTYP_RX:
		//	s_stDoIPRxBuffer[ucConnectTyp].pucBuff = NULL_PTR;
			DoIP_Inner_MemSet(&s_stDoIPRxBuffer[ucConnectTyp],0,sizeof(s_stDoIPRxBuffer[ucConnectTyp]));
		break;
		default:
		break;
	}

#endif
}
#endif


static void DiscardMsg(void)
{
	
}

#if 0
static boolean SoAd_BufferGet(uint32 size, uint8** buffPtr)
{
	static uint8 s_ucArr[1024];

	if (size < 1024)
	{
		*buffPtr = s_ucArr;
		return TRUE;
	}
	return FALSE;
}
#endif

static void SoAd_BufferFree(uint8* buffPtr)
{
	buffPtr = NULL_PTR;
	(void)buffPtr;
}


static void DoIP_Inner_MemSet(void* s, uint8 c, uint32 ulSize)  
{  
    uint8 *tmpS;
    if ((NULL_PTR != s) && (ulSize > 0))
    {
	    tmpS = (uint8*)s;  
	    while(ulSize--)  
	        *tmpS++ = c; 		
	}   
}

static void DoIP_Inner_MemCpy( void* vpTgt, const void* vpSrc, const uint32 ulSize )
{
	uint32 ulCounter;
	uint8 *ucpTgt;
	const uint8 *ucpSrc;

	ucpTgt = (uint8 *)(vpTgt);
	ucpSrc = (const uint8 *)(vpSrc);

	for (ulCounter = 0; ulCounter < ulSize; ulCounter++)
	{
		*ucpTgt = *ucpSrc;
		ucpTgt++;
		ucpSrc++;
	}
}

static boolean DoIP_Inner_MemCmp( void* vpTgt, const void* vpSrc, const uint32 ulSize )
{
	uint32 ulCounter;
	uint8 *ucpTgt;
	const uint8 *ucpSrc;

	ucpTgt = (uint8 *)(vpTgt);
	ucpSrc = (const uint8 *)(vpSrc);

	for (ulCounter = 0; ulCounter < ulSize; ulCounter++)
	{
		if (*ucpTgt != *ucpSrc)
			return FALSE;
		
		ucpTgt++;
		ucpSrc++;
	}
	
	return TRUE;
}




static Std_ReturnType DoIP_Inner_FillHeader(uint8* Header,uint16 PayLoadType,uint32 ulPayLoadLength)
{
	Std_ReturnType ucRet = E_OK;
	if (NULL_PTR != Header)
	{
		Header[DOIP_GH_PROTOCOL_VERSION] 			= DOIP_PROTOCOL_VERSION;
		Header[DOIP_GH_INVERSE_PROTOCOL_VERSION] 	= ~DOIP_PROTOCOL_VERSION;
		Header[DOIP_GH_PAYLOAD_TYPE] 				= (uint8)((PayLoadType >> 8) & 0x00FF);
		Header[DOIP_GH_PAYLOAD_TYPE+1] 				= (uint8)(PayLoadType & 0x00FF);
		Header[DOIP_GH_PAYLOAD_LENGTH] 				= (uint8)((ulPayLoadLength >> 24) & 0x000000FF);
		Header[DOIP_GH_PAYLOAD_LENGTH+1] 			= (uint8)((ulPayLoadLength >> 16) & 0x000000FF);
		Header[DOIP_GH_PAYLOAD_LENGTH+2] 			= (uint8)((ulPayLoadLength >> 8) & 0x000000FF);
		Header[DOIP_GH_PAYLOAD_LENGTH+3] 			= (uint8)(ulPayLoadLength & 0x000000FF);
	}
	else
	{
		ucRet = E_NOT_OK;
	}
	return ucRet;
}

static void DoIP_Inner_RegisterSocket(uint8 slotIndex, uint16 SoConId, uint8 ActivationType, uint16 SA) 
{
	PR_DEBUG(DEBUG_SWITCH, "%s  %d\n", __func__,slotIndex);

	if (slotIndex < DOIP_SOAD_CONNECTIONS_MAX)
	{
		s_stDoIPCfg.Connection_Table[slotIndex].ucSoConId = SoConId;
		s_stDoIPCfg.Connection_Table[slotIndex].ucActivationType = ActivationType;
		s_stDoIPCfg.Connection_Table[slotIndex].uiTA = SA;
		s_stDoIPCfg.Connection_Table[slotIndex].ulGeneralInactivityTimer = 0;
		s_stDoIPCfg.Connection_Table[slotIndex].ulInitialInactivityTimer = 0;	/* Stop */
		s_stDoIPCfg.Connection_Table[slotIndex].ucAuthenticationInfo = FALSE;
		s_stDoIPCfg.Connection_Table[slotIndex].ucConfirmationInfo = FALSE;
		s_stDoIPCfg.Connection_Table[slotIndex].ucAwaitingAliveCheckResponse = FALSE;
		s_stDoIPCfg.Connection_Table[slotIndex].ucConnectionState = DOIP_CONCTS_SOCKET_INITIALIZED;	
		PR_DEBUG(DEBUG_SWITCH, "%d-----ucConnectionState:%d\n", __LINE__,s_stDoIPCfg.Connection_Table[slotIndex].ucConnectionState);
	}
}

#if 0
/* According to ISO13400-2 Figure 15 — Alive check on all TCP_DATA sockets */
static void DoIP_Inner_AllSocketsAliveCheck(void)
{
	uint16 uiLoop;
	for (uiLoop = 0; uiLoop < DOIP_SOAD_CONNECTIONS_MAX; uiLoop++)
	{
		if ( (DOIP_CONCTS_REGISTERED_PEND_AUTHEN == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState)
		   ||(DOIP_CONCTS_REGISTERED_PEND_CONFIRM == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState)
		   ||(DOIP_CONCTS_REGISTERED_ROUTING_ALIVE == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState) )
		{
			DoIP_Inner_SendAliveCheckRequest(uiLoop);			
		}
	}
}
#endif

/* According to ISO13400-2 Figure 13 depicts the TCP_DATA socket handler */
static Std_ReturnType DoIP_Inner_TCPSocketHandle(uint16 SoConId, uint8 ActivationType, uint16 SA, uint8* routingActivationResponseCode)
{
	uint16 uiLoop;
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

	/* 	Calculate number of currently registered TCP sockets */
	for (uiLoop = 0; uiLoop < DOIP_SOAD_CONNECTIONS_MAX; uiLoop++) 
	{
		if ( (DOIP_CONCTS_REGISTERED_PEND_AUTHEN == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState)
		   ||(DOIP_CONCTS_REGISTERED_PEND_CONFIRM == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState)
		   ||(DOIP_CONCTS_REGISTERED_ROUTING_ALIVE == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState) ) 
		{
			break;
		}
	}
	if (DOIP_SOAD_CONNECTIONS_MAX == uiLoop)  /* number of registered TCP_DATA sockets == 0 */
	{
		//Assign received SA to the TCP_DATA socket on which the routing activation request was received
		DoIP_Inner_RegisterSocket(0, SoConId, ActivationType, SA);		
		return DOIP_ROUTINGACTIVATION_ACCEPTED;
	} 
	else 	/*	number of registered TCP_DATA sockets > 0 */
	{
		/*	Check if the TCP_DATA socket on which the current message was received is already registered */
		for (uiLoop = 0; uiLoop < DOIP_SOAD_CONNECTIONS_MAX; uiLoop++) 	
		{
			if (s_stDoIPCfg.Connection_Table[uiLoop].ucSoConId == SoConId)
			{				
				if ( (DOIP_CONCTS_REGISTERED_PEND_AUTHEN == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState)
			    || (DOIP_CONCTS_REGISTERED_PEND_CONFIRM == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState) 
			    || (DOIP_CONCTS_REGISTERED_ROUTING_ALIVE == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState) ) 
				{
					// Check if received SA is already assigned to this TCP_DATA socket
					if (s_stDoIPCfg.Connection_Table[uiLoop].uiTA == SA) 
					{
						return DOIP_ROUTINGACTIVATION_ACCEPTED;	/* DoIP-089 */
					} 
					else 
					{
						return DOIP_ROUTINGACTIVATION_REJECTED;	/* DoIP-106 */
					}
				}
			}
		}

		/* Check if SA is already registered to another TCP _DATA socket */
		for (uiLoop = 0; uiLoop < DOIP_SOAD_CONNECTIONS_MAX; uiLoop++) 
		{
			if ( (s_stDoIPCfg.Connection_Table[uiLoop].uiTA == SA) 
			  && ( (DOIP_CONCTS_REGISTERED_PEND_AUTHEN == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState)
			    || (DOIP_CONCTS_REGISTERED_PEND_CONFIRM == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState) 
			    || (DOIP_CONCTS_REGISTERED_ROUTING_ALIVE == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState) ) )
			{
				/* DoIP-091 *//* Figure 14 — Alive check on single TCP_DATA sockets */
			//	if (FALSE == s_stDoIPCfg.Connection_Table[uiLoop].ucAwaitingAliveCheckResponse)
				{	
					DoIP_Inner_SendAliveCheckRequest(uiLoop);
				}
				return DOIP_ROUTINGACTIVATION_PENDING; /* DoIP-093 If alivecheckresponse received within T_TCP_Alive_Check */
			}
		}

		/* Check if TCP_DATA sockets are available */
		for (uiLoop = 0; uiLoop < DOIP_SOAD_CONNECTIONS_MAX; uiLoop++) 
		{
			if (DOIP_CONCTS_LISTEN == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState) 
			{
				DoIP_Inner_RegisterSocket(uiLoop, SoConId, ActivationType, SA); /* DoIp-090 */
				return DOIP_ROUTINGACTIVATION_ACCEPTED;
			}
			else if ( (DOIP_CONCTS_REGISTERED_PEND_AUTHEN == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState)
			   		||(DOIP_CONCTS_REGISTERED_PEND_CONFIRM == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState)
			   		||(DOIP_CONCTS_REGISTERED_ROUTING_ALIVE == s_stDoIPCfg.Connection_Table[uiLoop].ucConnectionState) )
			{
				DoIP_Inner_SendAliveCheckRequest(uiLoop);		/* DoIP-094 */	
			}
			else
			{
				//no action 
			}			
		}
		
		/* DoIP-094 */
		return DOIP_ROUTINGACTIVATION_PENDING;
	}
}



/* 0x0000 */
/* Generic DoIP header negative acknowledge */
static void DoIP_Inner_SendHeaderNAck(uint16 SoConId, uint8 NackCode) 
{
//	uint16 uiRetSent;
	PduIdType ucTxPduId;
	PduInfoType stPduInfo;
	uint8 pucTxBuffer[DOIP_GH_TOTAL+DOIP_PL_GNRHEADERNACK];
	
	PR_DEBUG(DEBUG_SWITCH, "%s_%x\n", __func__,NackCode);
	
	(void)DoIP_Inner_FillHeader(pucTxBuffer,DOIP_PT_GNRHEADERNACK,DOIP_PL_GNRHEADERNACK);
	pucTxBuffer[8] = NackCode;

	ucTxPduId = s_stDoIPCfg.Connection_Table[SoConId].DoIPSoAdPduId;
	stPduInfo.SduLength = DOIP_GH_TOTAL+DOIP_PL_GNRHEADERNACK;
	stPduInfo.SduDataPtr = pucTxBuffer;
	
	if (s_stDoIPCfg.Connection_Table[SoConId].pfSoAd_Transmit(ucTxPduId, &stPduInfo)) 
	{

	}
}



/* 0x0004 UDP */
/* Vehicle Identification response/vehicle announcement */
static void DoIP_Inner_SendVehicleAnnouncement(uint16 SoConId) 
{
	uint8 uiLoop;
    uint8* pucTxBuffer;
	uint32 ulIndex = 0;
	Std_ReturnType ucRetGetVIN = E_OK;
	Std_ReturnType ucRetGID = E_OK;
//	PduIdType ucTxPduId;
	PduInfoType stPduInfo;

	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

    if ((DOIP_GH_TOTAL+DOIP_PL_VHCLIDFIC_RESP) > s_stDoIPCfg.Connection_Table[SoConId].pstRxBuff->total_Len) 
	{
		pucTxBuffer = s_stDoIPCfg.Connection_Table[SoConId].pstRxBuff->pucBuff;
		if (NULL_PTR != pucTxBuffer)
		{
			(void)DoIP_Inner_FillHeader(pucTxBuffer,DOIP_PT_VHCLIDFIC_RESP,DOIP_PL_VHCLIDFIC_RESP);
			
			ulIndex += DOIP_GH_TOTAL;

		//	ucRetGetVIN = Dcm_GetVin(&pucTxBuffer[ulIndex]);
			if (E_NOT_OK == ucRetGetVIN)
			{
				DoIP_Inner_MemCpy(&pucTxBuffer[DOIP_GH_TOTAL], DOIP_VININVALIDITYPATTERN /* g_stDoIPCfg.DoIPGeneral.DoIPVinInvalidityPattern */ , DOIP_OFFSET_VIN);
			}
			
			ulIndex += DOIP_OFFSET_VIN;

			pucTxBuffer[ulIndex] = (DOIP_LOGICAL_ADDRESS /*g_stDoIPCfg.DoIPConfigSet.DoIPLogicalAddress*/ >> 8) & 0xff;
			pucTxBuffer[ulIndex+1] = (DOIP_LOGICAL_ADDRESS /*g_stDoIPCfg.DoIPConfigSet.DoIPLogicalAddress*/ >> 0) & 0xff;
			
			ulIndex += DOIP_OFFSET_LA;

			if (TRUE == DOIP_USEMACADDRESSFORIDENTIFICATION/*g_stDoIPCfg.DoIPGeneral.DoIPUseMacAddressForIdentification*/)
			{
			//	if (E_NOT_OK == SoAd_GetPhysAddr(&pucTxBuffer[ulIndex], DOIP_OFFSET_EID)) 
				{
					DoIP_Inner_MemCpy(&pucTxBuffer[ulIndex], DOIPEID /* g_stDoIPCfg.DoIPConfigSet.DoIPEid */, DOIP_OFFSET_EID);
				}				
			}
			else
			{
				DoIP_Inner_MemCpy(&pucTxBuffer[ulIndex], DOIPEID /* g_stDoIPCfg.DoIPConfigSet.DoIPEid */, DOIP_OFFSET_EID);				
			}

			ulIndex += DOIP_OFFSET_EID;
			
			if (TRUE == DOIP_USEEIDASGID /*g_stDoIPCfg.DoIPGeneral.DoIPUseEIDasGID*/)
			{
				DoIP_Inner_MemCpy(&pucTxBuffer[ulIndex], DOIPEID /* g_stDoIPCfg.DoIPConfigSet.DoIPEid */, DOIP_OFFSET_GID);	
			}
			else
			{
				if	(0 != DOIPGID)
				{
					DoIP_Inner_MemCpy(&pucTxBuffer[ulIndex], DOIPGID /* g_stDoIPCfg.DoIPConfigSet.DoIPGid */, DOIP_OFFSET_GID);
				}	
				else
				{
					if (TRUE == DOIP_VINGIDMASTER /* g_stDoIPCfg.DoIPGeneral.DoIPVinGidMaster */)
					{
					//	ucRetGID = g_stDoIPCfg.DoIPGeneral.DoIPGetGidDirect(&pucTxBuffer[ulIndex]);
					}
					else
					{
					//	ucRetGID = DoIPGetGID(&pucTxBuffer[ulIndex]);
					}
					if (E_NOT_OK == ucRetGID)
					{
						DoIP_Inner_MemSet(&pucTxBuffer[ulIndex],DOIP_GIDINVALIDITYPATTERN /* g_stDoIPCfg.DoIPGeneral.DoIPGIDInvalidityPattern */, DOIP_OFFSET_GID);
					}
				}
			}

			ulIndex += DOIP_OFFSET_GID;

			if (E_NOT_OK == ucRetGetVIN)
			{
				pucTxBuffer[ulIndex] = 0x00; /* SWS_DoIP_00072 */
			}
			else
			{
				for (uiLoop = 0; uiLoop < DOIP_SOAD_CONNECTIONS_MAX; uiLoop++)
				{
					if (DOIP_ACTIVATIONTYPE_CENTRALSECURITYOBD == DOIP_ROUT_ACT_NUM)
					{
						break;
					}
				}
				if (uiLoop == DOIP_SOAD_CONNECTIONS_MAX)
				{
					pucTxBuffer[ulIndex] = 0x00;  /* SWS_DoIP_00083 */
				}
				else
				{
					if (s_stDoIPCfg.Connection_Table[ulIndex].ucAllRoutingActivationStatus)
					{
						pucTxBuffer[ulIndex] = 0x10;  /* SWS_DoIP_00082 */
					}
					else
					{
						pucTxBuffer[ulIndex] = 0x00;  /* SWS_DoIP_00084 */
					}
				}
			}
			
			ulIndex += DOIP_OFFSET_FAR;

			if (TRUE == DOIP_USEVEHICLEIDENTIFICATIONSYNCSTATUS /* g_stDoIPCfg.DoIPGeneral.DoIPUseVehicleIdentificationSyncStatus */)
			{
				if (E_NOT_OK == ucRetGetVIN)/* SWS_DoIP_00072 */
				{
					if (0 == s_stDoIPCfg.Connection_Table[ulIndex].ucGIDSyncStatus)	/* SWS_DoIP_00076 */
					{
						pucTxBuffer[ulIndex] = 0x10;/* SWS_DoIP_00088 */
					}
					else
					{
						pucTxBuffer[ulIndex] = 0x00;/* SWS_DoIP_00089 */
					}
				}
				else
				{
					pucTxBuffer[ulIndex] = 0x00;/* SWS_DoIP_00087 */
				}
				
			}
			
			ulIndex += DOIP_OFFSET_VGSS;
			stPduInfo.SduLength = DOIP_GH_TOTAL+DOIP_PL_VHCLIDFIC_RESP;
			stPduInfo.SduDataPtr =	pucTxBuffer;

			
			if (s_stDoIPCfg.Connection_Table[SoConId].pfSoAd_Transmit(SoConId, &stPduInfo))
			{

			}
			SoAd_BufferFree(pucTxBuffer);
		}
		else
		{
			
		}
    }
	else
	{

    }
}

/* 0x0006 */
/* Routing activation response */
/* According to ISO13400-2 Figure 9 DoIP routing activation handler */
static void DoIP_Inner_SendRoutingActivationResponse(uint16 SoConId, uint32 ulPayLoadLength, uint8 *pucRxBuff)
{
    uint16 SA;
    uint8 ActivationType;
    uint8* pucTxBuffer;
	uint32 ulIndex = 0;
	uint8 ucRoutingActivationResponseCode = 0xFF; // Vehicle manufacturer-specific
	boolean bAuthenticated = FALSE;
	boolean bConfirmed;
	uint8   ucSocketHandleResult;
	PduInfoType stPduInfo;
	PduIdType ucTxPduId;
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

    if ((DOIP_PL_ROUTACTVTN_REQ_7 == ulPayLoadLength) || (DOIP_PL_ROUTACTVTN_REQ_11 == ulPayLoadLength)) 
	{
		if ((DOIP_GH_TOTAL+DOIP_PL_ROUTACTVTN_RESP_13) > s_stDoIPCfg.Connection_Table[SoConId].pstRxBuff->total_Len) 
		{
			pucTxBuffer = s_stDoIPCfg.Connection_Table[SoConId].pstRxBuff->pucBuff;
			SA = (pucRxBuff[DOIP_GH_TOTAL+0] << 8) + pucRxBuff[DOIP_GH_TOTAL+1];
			ActivationType = pucRxBuff[DOIP_GH_TOTAL+DOIP_OFFSET_SA];
			
			(void)DoIP_Inner_FillHeader(pucTxBuffer,DOIP_PT_ROUTACTVTN_RESP,DOIP_PL_ROUTACTVTN_RESP_13);
			ulIndex += DOIP_GH_TOTAL;
			
			//Logical address of external test Equipment
			pucTxBuffer[ulIndex] = SA >> 8;
			pucTxBuffer[ulIndex + 1] = SA;
			ulIndex += DOIP_OFFSET_LA;
			
			//Logical address of external DoIP entity			
			pucTxBuffer[ulIndex] = (DOIP_LOGICAL_ADDRESS /*g_stDoIPCfg.DoIPConfigSet.DoIPLogicalAddress*/ >> 8) & 0xff;
			pucTxBuffer[ulIndex + 1] = (DOIP_LOGICAL_ADDRESS /*g_stDoIPCfg.DoIPConfigSet.DoIPLogicalAddress*/ >> 0) & 0xff;
			ulIndex += DOIP_OFFSET_LA;
			
			if (TRUE == isSourceAddressKnown(SA)) 
			{
				if (TRUE == isRoutingTypeSupported(ActivationType)) 
				{
					ucSocketHandleResult = DoIP_Inner_TCPSocketHandle(SoConId, ActivationType, SA, &ucRoutingActivationResponseCode);
					if (DOIP_ROUTINGACTIVATION_ACCEPTED == ucSocketHandleResult) 
					{
						if (TRUE == isAuthenticationRequired(ActivationType)) 
						{
							if (TRUE != isAuthenticated(SA,SoConId))
							{
								/* DoIP-061 */
								bAuthenticated = FALSE; //Send routing activation negative response code
								(void)bAuthenticated;
								ucRoutingActivationResponseCode = DOIP_RARC_AUTHENTIC_MISS;
								/* DoIP-102 */
								DoIP_SocketClose(SoConId);
										/* Final- routing activation handler */
							}
						} 

						if (TRUE == isConfirmationRequired(ActivationType)) 
						{
							bConfirmed = isConfirmed(SA,SoConId);
							if (TRUE == bConfirmed) 
							{
								/* DoIP-062 */
								ucRoutingActivationResponseCode = DOIP_RARC_SUCCESS;
							} 
							else if (0 == bConfirmed)
							{
								/* Routing will be activated; confirmation required */
								ucRoutingActivationResponseCode = DOIP_RARC_CONFRM_REQUIRED;
							}
							else /* (Rejected == bConfirmed) */
							{
								
							}
						} 	
						else 
						{
							/* Routing activation rejected due to missing authentication */
							ucRoutingActivationResponseCode = DOIP_RARC_AUTHENTIC_MISS;
						}

					} 
					else if (DOIP_ROUTINGACTIVATION_REJECTED == ucSocketHandleResult)
					{ 	/*	DoIP-060
							DoIP-148
							DoIP-149
							DoIP-150 */
						/* Validate response code */
						switch (ucRoutingActivationResponseCode) 
						{
							case 0x01:
							case 0x02:
							case 0x03:
								/* OK! */
								break;
							default:
									/* Unsupported response code at this level */
									ucRoutingActivationResponseCode = 0xFF;
								break;
						}
					} 
					else if (DOIP_ROUTINGACTIVATION_PENDING == ucSocketHandleResult) 
					{ 	/* socketHandle */
						/*
						* Trying to assign a connection slot, but pending
						* alive check responses before continuing.
						* Continuation handled from DoIp_MainFunction (if a
						* connection times out and thus becomes free) or from
						* handleAliveCheckResp (if all connections remain
						* active)
						*/
						if (NULL == pendingRoutingActivationTxBuffer) 
						{
							pendingRoutingActivationTxBuffer = pucTxBuffer;
							pendingRoutingActivationSa = SA;
							pendingRoutingActivationSocket = SoConId;
							pendingRoutingActivationActivationType = ActivationType;
						} 
						else 
						{
							// Socket assignment pending; alive check response already pending
							// This should not happen - the connection attempt should not have been accepted..
						}
						return;
					} 
					else
					{
						/* This cannot happen */
					}
				} 
				else /* Routing activation denied due to unsupported routing activation type */
				{ 	/* DoIP-151 */
					ucRoutingActivationResponseCode = 0x06;
					/* DoIP-102 */
					/* Do not activate routing and close this TCP_DATA socket. */
				}
			}
			else	/* Routing activation rejected due to unknown source address */
			{ 	/* DoIP-059 */
				ucRoutingActivationResponseCode = 0x00;
				/* DoIP-102 */
				/* Do not activate routing and close this TCP_DATA socket. */
			}

			pucTxBuffer[ulIndex] = ucRoutingActivationResponseCode; 
			ulIndex++;
			pucTxBuffer[ulIndex] = 0;// Reserved ISO
			ulIndex++;
			pucTxBuffer[ulIndex] = 0; 
			ulIndex++;
			pucTxBuffer[ulIndex] = 0; 
			ulIndex++;
			pucTxBuffer[ulIndex] = 0; 
			ulIndex++;
			pucTxBuffer[ulIndex] = 0;// OEM specific GID	
			ulIndex++;		
			pucTxBuffer[ulIndex] = 0; 
			ulIndex++;
			pucTxBuffer[ulIndex] = 0; 
			ulIndex++;
			pucTxBuffer[ulIndex] = 0;

			ucTxPduId = s_stDoIPCfg.Connection_Table[SoConId].DoIPSoAdPduId;
			stPduInfo.SduLength = DOIP_GH_TOTAL+DOIP_PL_ROUTACTVTN_RESP_13;
			stPduInfo.SduDataPtr =	pucTxBuffer;

			if (E_NOT_OK == s_stDoIPCfg.Connection_Table[SoConId].pfSoAd_Transmit(ucTxPduId, &stPduInfo)) 
			{
				/* SWS_DoIP_00223 */
				DiscardMsg();
				// Could not send data - report error in development error tracer
			}
			SoAd_BufferFree(pucTxBuffer);

			/* SWS_DoIP_00145 */
			s_stDoIPCfg.Connection_Table[SoConId].ulInitialInactivityTimer = 0;/* Stop */

			/**
			*  ISO13400-2 : DoIP-102
			*
			* Negative response code --> Close socket on which the current message was received
			*/
			switch (ucRoutingActivationResponseCode) 
			{
				case 0x10: // Routing activated.
				case 0x11: // Confirmation pending
				case 0x04: // Missing authentication
					break;
				default:
						DoIP_SocketClose(SoConId);
					break;
			}
		} 
		else
		{ 
			DoIP_Inner_SendHeaderNAck(SoConId, DOIP_E_H_OUT_OF_MEMORY);
		}
	} 
	else
	{
        DoIP_Inner_SendHeaderNAck(SoConId, DOIP_E_H_INVALID_PAYLOAD_LENGTH);
        DoIP_SocketClose(SoConId);
    }
	if (ucRoutingActivationResponseCode == DOIP_RARC_SUCCESS)
	{
		s_stDoIPCfg.Connection_Table[SoConId].uiTA = SA;  //Save TA which has activated.
	}
	else
	{
		s_stDoIPCfg.Connection_Table[SoConId].uiTA = 0;  //Clr TA which activated failed.
	}
}

/* 0x0007 */
/* Alive check request */
static void DoIP_Inner_SendAliveCheckRequest(uint16 uiConnectionIndex)
{
	uint8 ucRetSent;
	PduInfoType stPduInfo;
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

    if ((DOIP_GH_TOTAL+DOIP_PL_ALIVECHK_REQ) > s_stDoIPCfg.Connection_Table[uiConnectionIndex].pstRxBuff->total_Len) 
	{
		stPduInfo.SduDataPtr = s_stDoIPCfg.Connection_Table[uiConnectionIndex].pstRxBuff->pucBuff;

		(void)DoIP_Inner_FillHeader(stPduInfo.SduDataPtr, DOIP_PT_ALIVECHK_REQ, DOIP_PL_ALIVECHK_REQ);
		stPduInfo.SduLength = DOIP_GH_TOTAL + DOIP_PL_ALIVECHK_REQ;
		
		ucRetSent = s_stDoIPCfg.Connection_Table[uiConnectionIndex].pfSoAd_Transmit(s_stDoIPCfg.Connection_Table[uiConnectionIndex].ucSoConId, &stPduInfo);
		
		if (E_OK != ucRetSent)
		{
			/* SWS_DoIP_00223 */
			DiscardMsg();

			s_stDoIPCfg.Connection_Table[uiConnectionIndex].ucConnectionState = DOIP_CONCTS_LISTEN;
			PR_DEBUG(DEBUG_SWITCH, "%d-----ucConnectionState:%d\n", __LINE__,s_stDoIPCfg.Connection_Table[uiConnectionIndex].ucConnectionState);

			/* If there are pending routing activations waiting, activate that one. */
			if (0xff != pendingRoutingActivationSocket) 
			{
				handleTimeout(uiConnectionIndex);
			} 
			else 
			{
				DoIP_SocketClose(s_stDoIPCfg.Connection_Table[uiConnectionIndex].ucSoConId);
			}
		}

		s_stDoIPCfg.Connection_Table[uiConnectionIndex].ucAwaitingAliveCheckResponse = TRUE;
		s_stDoIPCfg.Connection_Table[uiConnectionIndex].ulGeneralInactivityTimer = 0;
		s_stDoIPCfg.Connection_Table[uiConnectionIndex].ulAliveCheckTimer = 0;
		
		SoAd_BufferFree(stPduInfo.SduDataPtr);
	} 
	else 
	{
		// No tx buffer available. Report a Det error.
	}
}

/* 0x0008 */
/* Alive Check response */
static void DoIP_Inner_RcvAliveCheckResponse(uint16 SoConId, uint32 ulPayLoadLength, uint8 *pucRxBuff)
{
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

	/* SWS_DoIP_00141 */
	if (1 /*SourceAddress matches the registered Source Address of the socket connection the response was received on*/)
	{
		/* do nothing */
	}
	else
	{
		/* SWS_DoIP_00058 */
		DoIP_SocketClose(SoConId);
	}
	s_stDoIPCfg.Connection_Table[SoConId].ulInitialInactivityTimer = 0; /* Stop */
	s_stDoIPCfg.Connection_Table[SoConId].ucAwaitingAliveCheckResponse = FALSE;

	//...
}


#if 0
/* 0x8002 / 0x8003 */
/* Diagnostic positive / negative acknowledge */
static void DoIP_Inner_SendDiagMsgAck(uint16 id, uint32 ulPayLoadLength, uint8 *pucRxBuff)
{
	uint8 lookupResult;
	uint16 SA = 0;
	uint16 TA = 0;
	uint8 ucLoop = 0;
	uint8 ucPduRIndex = 0;
	uint16 uiPayLoadType;
	static uint8 PdurSentSts = BUFREQ_E_NOT_OK;
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);
    PduInfoType stInfo_Ext;
    PduLengthType TpSduLength_Ext;
    PduLengthType bufferSizePtr_Ext;
    PduLengthType availableDataPtr_Ext;

	if (ulPayLoadLength >= DOIP_PL_DIAGMSG_MIN) 
	{
		SA = (pucRxBuff[DOIP_GH_TOTAL] << 8) | pucRxBuff[DOIP_GH_TOTAL + 1];
		TA = (pucRxBuff[DOIP_GH_TOTAL + DOIP_OFFSET_SA] << 8) | pucRxBuff[DOIP_GH_TOTAL + DOIP_OFFSET_SA + 1];

		lookupResult = CheckSaTa(id,SA, TA, ulPayLoadLength, &ucPduRIndex );
		if (lookupResult == LOOKUP_SA_TA_OK) 
		{
			/* 0x8002 */
			DoIP_Inner_SendDiagMsgNack(id, SA, TA, 0,ulPayLoadLength);

			for (ucLoop = 0; ucLoop < DOIP_UPLA_CHANNEL_MAX; ucLoop++)
			{
				if (s_stDoIP_TAChannel[ucLoop].uiTA == s_stDoIPCfg.Connection_Table[id].uiTA)
				{
					ucPduRIndex = s_stDoIP_TAChannel[ucLoop].ucPduRRxPduId;
					break;
				}
			}
			
			if (ucPduRIndex < DOIP_UPLA_CHANNEL_MAX)
			{
				stInfo_Ext.SduDataPtr = &s_stDoIPCfg.Connection_Table[id].pstRxBuff->pucBuff[DOIP_GH_TOTAL];
				stInfo_Ext.SduLength = s_stDoIPCfg.Connection_Table[id].pstRxBuff->total_Len;
				TpSduLength_Ext = stInfo_Ext->SduLength;
				PdurSentSts = Dcm_StartOfReception(ucPduRIndex,&stInfo_Ext,TpSduLength_Ext,&bufferSizePtr_Ext);

				/* SWS_DoIP_00253 */
				if (bufferSizePtr_Ext < stInfo_Ext->SduLength)
				{
					Dcm_TpRxIndication(ucPduRIndex,E_NOT_OK);
				}
				
				if (PdurSentSts == BUFREQ_OK)
				{
					availableDataPtr_Ext = bufferSizePtr_Ext;
					//s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx += stInfo_Ext.SduLength;
					/* SWS_DoIP_00260 */
					while(s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx < s_stDoIPCfg.Connection_Table[id].pstRxBuff->total_Len)
					{
						stInfo_Ext.SduDataPtr = &s_stDoIPCfg.Connection_Table[id].pstRxBuff->pucBuff[s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx];

						if (availableDataPtr_Ext > s_stDoIPCfg.Connection_Table[id].pstRxBuff->total_Len)
						{
							stInfo_Ext.SduLength = s_stDoIPCfg.Connection_Table[id].pstRxBuff->total_Len;
						}
						else
						{
							stInfo_Ext.SduLength = availableDataPtr_Ext;
						}
						
						PdurSentSts = Dcm_CopyRxData(ucPduRIndex,&stInfo_Ext,&availableDataPtr_Ext);
						if (PdurSentSts != BUFREQ_OK)
						{
							break;
						}
						else
						{
							s_stDoIPCfg.Connection_Table[id].pstRxBuff->copy_idx += stInfo_Ext.SduLength;
						}
					}
					/* SWS_DoIP_00218 */
					s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_DIRECT; 
				}
				/* SWS_DoIP_00216 */
				else if ((BUFREQ_E_OVFL == PdurSentSts) || (BUFREQ_E_NOT_OK == PdurSentSts))
				{
					/* SWS_DoIP_00174 */
					DoIP_TpCancelReceive(id);
					s_stDoIPCfg.Connection_Table[id].pstRxBuff->Status = DOIP_BUFFSTS_RX_IDLE;
					/* SWS_DoIP_00217 */									
					Dcm_TpRxIndication(ucPduRIndex,E_NOT_OK);
				}
				else
				{
					
				}
			}
		} 
		else if (lookupResult == LOOKUP_SA_TA_TAUNKNOWN)
		{
			// TA not known
			DoIP_Inner_SendDiagMsgNack(id, SA, TA, DOIP_E_DIAG_UNKNOWN_TA,ulPayLoadLength);
		} 
		else 
		{
			// SA not registered on receiving socket
			DoIP_Inner_SendDiagMsgNack(id, SA, TA, DOIP_E_DIAG_INVALID_SA,ulPayLoadLength);
			DoIP_SocketClose(id);
		}
	} 
	else 
	{
		DoIP_Inner_SendHeaderNAck(id, DOIP_E_H_INVALID_PAYLOAD_LENGTH);
		DoIP_SocketClose(id);
	}
}
#endif

/* 0x8003 */
static void DoIP_Inner_SendDiagMsgNack(uint16 SoConId, uint16 sa, uint16 ta, uint8 nackCode,uint32 ulPayLoadLength)
{
//	uint8 ucIndex;
	uint8 pucTxBuffer[8+5];
//	PduIdType TxPduId;
	PduInfoType stPduInfo;
	PR_DEBUG(DEBUG_SWITCH, "%s  %d.\n", __func__,nackCode);

	pucTxBuffer[0] = DOIP_PROTOCOL_VERSION;
	pucTxBuffer[1] = ~DOIP_PROTOCOL_VERSION;
	pucTxBuffer[2] = 0x80;	// 0x8003->Diagnostic Message Negative Acknowledge
	pucTxBuffer[3] = (nackCode == 0)?0x02:0x03;
	pucTxBuffer[4] = 0;	// Length = 0x00000005
	pucTxBuffer[5] = 0;
	pucTxBuffer[6] = 0;
	pucTxBuffer[7] = 5;
	pucTxBuffer[8+0] = ta >> 8;	// TA
	pucTxBuffer[8+1] = ta;
	pucTxBuffer[8+2] = sa >> 8;	// SA
	pucTxBuffer[8+3] = sa;
	pucTxBuffer[8+4] = nackCode;

/*
	for (ucIndex = 0; ucIndex < DOIP_SOAD_CONNECTIONS_MAX; ++ucIndex)
	{
		if (SoConId == s_stDoIPCfg.Connection_Table[ucIndex].ucSoConId)
		{
			TxPduId = s_stDoIPCfg.Connection_Table[ucIndex].DoIPSoAdPduId;
			break;
		}
	}
*/
#if 0
	stPduInfo.SduLength = 8+5+ulPayLoadLength;  //?  to do ... length is bigger than buffer.
#else
	stPduInfo.SduLength = 8+5;
#endif
	stPduInfo.SduDataPtr = pucTxBuffer;

#if 1
	if (E_NOT_OK == s_stDoIPCfg.Connection_Table[SoConId].pfSoAd_Transmit(SoConId,&stPduInfo))
#else
	if (E_NOT_OK == TcpIp_TcpTransmit(2, pucTxBuffer, (uint32)stPduInfo.SduLength, NULL_PTR, FALSE ))
#endif
	{
		/* SWS_DoIP_00223 */
		DiscardMsg();
	}
}



/* 0x4002 UDP */
/* DoIP entity status response */
static void DoIP_Inner_SendEntityStatusResp(uint16 SoConId, uint32 ulPayLoadLength, uint8 *pucRxBuff)
{
	PduInfoType stPduInfo;
	PduIdType ucTxPduId;
	uint8* pucTxBuffer = NULL;
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

	if ((DOIP_GH_TOTAL+DOIP_PL_ENTITYSTS_RESP) > s_stDoIPCfg.Connection_Table[SoConId].pstTxBuff->total_Len) 
	{
		pucTxBuffer = s_stDoIPCfg.Connection_Table[SoConId].pstTxBuff->pucBuff;
		DoIPNodeType_Enum ucNodeType = DOIP_NODE;
		uint8 maxNofSockets = DOIP_SOAD_CONNECTIONS_MAX;
		uint8 curNofOpenSockets = 1 /*CurrentlyUsedTcpSockets()*/;
		uint32 maxDataSize = DOIP_BUFFER_LENGTH_MAX;

		DoIP_Inner_FillHeader(pucTxBuffer, DOIP_PT_ENTITYSTS_RESP, DOIP_PL_ENTITYSTS_RESP);
		pucTxBuffer[8] = ucNodeType;
		pucTxBuffer[9] = maxNofSockets;  // Max TCP sockets
		pucTxBuffer[10] = curNofOpenSockets; // Currently open TCP sockets

		pucTxBuffer[11] = (maxDataSize << 24);
		pucTxBuffer[12] = (maxDataSize << 16) & 0xff;
		pucTxBuffer[13] = (maxDataSize << 8) & 0xff;
		pucTxBuffer[14] = maxDataSize & 0xff;
		
		ucTxPduId = s_stDoIPCfg.Connection_Table[SoConId].DoIPSoAdPduId;
		stPduInfo.SduLength = DOIP_GH_TOTAL+DOIP_PL_ENTITYSTS_RESP;
		stPduInfo.SduDataPtr =	pucTxBuffer;

		if (E_NOT_OK == s_stDoIPCfg.Connection_Table[SoConId].pfSoAd_Transmit(ucTxPduId, &stPduInfo))
		{
			/* SWS_DoIP_00223 */
			DiscardMsg();
		}

		SoAd_BufferFree(pucTxBuffer);

	} 
	else 
	{
		DoIP_Inner_SendHeaderNAck(SoConId, DOIP_E_H_OUT_OF_MEMORY);
	}
}


/* 0x4004 UDP */
/* Diagnostic power mode information response */
static void DoIP_Inner_SendPowerModeCheckResp(uint16 SoConId, uint32 ulPayLoadLength, uint8 *pucRxBuff)
{
	uint8* pucTxBuffer = NULL;
	PduInfoType stPduInfo;
	PduIdType ucTxPduId;
	uint8 /*SoAd_DoIp_PowerMode*/ powerMode;
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

	if ((DOIP_GH_TOTAL+DOIP_PL_PWRMODEINFO_RESP) > s_stDoIPCfg.Connection_Table[SoConId].pstTxBuff->total_Len) 
	{
		pucTxBuffer = s_stDoIPCfg.Connection_Table[SoConId].pstTxBuff->pucBuff;	

	//	if (E_NOT_OK == g_stDoIPCfg.DoIPGeneral.DoIPPowerModeDirect(&powerMode)) 
		{
			// SWS_DoIP_00093
			powerMode = 0;
		}

		DoIP_Inner_FillHeader(pucTxBuffer, DOIP_PT_PWRMODEINFO_RESP, DOIP_PL_PWRMODEINFO_RESP);
		pucTxBuffer[8] = (uint8)powerMode;

		// SWS_DoIP_00092
		ucTxPduId = s_stDoIPCfg.Connection_Table[SoConId].DoIPSoAdPduId;
		stPduInfo.SduLength = DOIP_GH_TOTAL+DOIP_PL_PWRMODEINFO_RESP;
		stPduInfo.SduDataPtr =	pucTxBuffer;
		if (E_NOT_OK == s_stDoIPCfg.Connection_Table[SoConId].pfSoAd_Transmit(ucTxPduId, &stPduInfo)) 
		{
			/* SWS_DoIP_00223 */
			DiscardMsg();		
		}
		SoAd_BufferFree(pucTxBuffer);

	} 
	else 
	{
		DoIP_Inner_SendHeaderNAck(SoConId, DOIP_E_H_OUT_OF_MEMORY);
	}
}




/*  */
#if 0
static void DoIp_Inner_RxTCPMessage(uint8 ucIndex)
{
	PduLengthType Lenth;
	uint8* pucRxBuff;
	uint16 uiPayLoadType;
	uint16 ulPayLoadLength;
	uint8 SoConId;
	
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

	pucRxBuff = s_stDoIPCfg.Connection_Table[ucIndex].pstRxBuff->pucBuff;
	Lenth = s_stDoIPCfg.Connection_Table[ucIndex].pstRxBuff->total_Len;

	PR_DEBUG(DEBUG_SWITCH, "%d,%d\n", pucRxBuff[0],pucRxBuff[1]);
	if ((pucRxBuff[0] == DOIP_PROTOCOL_VERSION) && ((uint8)(~pucRxBuff[1]) == DOIP_PROTOCOL_VERSION)) 
	{
		uiPayLoadType = pucRxBuff[2] << 8 | pucRxBuff[3];
		ulPayLoadLength = (pucRxBuff[4] << 24) | (pucRxBuff[5] << 16) | (pucRxBuff[6] << 8) | pucRxBuff[7];
		PR_DEBUG(DEBUG_SWITCH, "%04x,%d\n", uiPayLoadType,ulPayLoadLength);
		if ((ulPayLoadLength + DOIP_GH_TOTAL) <= DOIP_BUFFER_LENGTH_MAX) 
		{
			switch (uiPayLoadType) 
			{
				case DOIP_PT_ROUTACTVTN_REQ:
					/* SWS_DoIP_00159 */
					s_stDoIPCfg.Connection_Table[ucIndex].ulInitialInactivityTimer = 0;/* Stop */
					SoConId = s_stDoIPCfg.Connection_Table[ucIndex].ucSoConId;
					DoIP_Inner_SendRoutingActivationResponse(SoConId, ulPayLoadLength, pucRxBuff);
					s_stDoIPCfg.Connection_Table[ucIndex].pstRxBuff->Status = DOIP_BUFFSTS_RX_COPY;
					break;

				case DOIP_PT_ALIVECHK_RESP:
					DoIP_Inner_RcvAliveCheckResponse(SoConId, ulPayLoadLength, pucRxBuff);
					s_stDoIPCfg.Connection_Table[ucIndex].pstRxBuff->Status = DOIP_BUFFSTS_RX_COPY;
					break;

				case DOIP_PT_DIAGMSG:
					DoIP_Inner_SendDiagMsgAck(SoConId, ulPayLoadLength, pucRxBuff);
					break;

				default:
					DoIP_Inner_SendHeaderNAck(SoConId, DOIP_E_H_UNKNOWN_PAYLOAD_TYPE);
					s_stDoIPCfg.Connection_Table[ucIndex].pstRxBuff->Status = DOIP_BUFFSTS_RX_IDLE;
					break;
			}
		} 
		else 
		{
			DoIP_Inner_SendHeaderNAck(SoConId, DOIP_E_H_MESSAGE_TOO_LARGE);
		}
	} 
	else
	{
		DoIP_Inner_SendHeaderNAck(SoConId, DOIP_E_H_INCORRECT_PATTERN_FORMAT);
		DoIP_SocketClose(SoConId);
	}
}
#endif
/* SWS_DoIP_00058 */
static Std_ReturnType DoIP_SocketClose(PduIdType TxPduId)
{
	SoAd_SoConIdType /* SoAd_SoConIdType */ SoConId;
	
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

	(void)SoAd_GetSoConId(TxPduId,&SoConId);
	(void)SoAd_CloseSoCon(SoConId,TRUE);
	s_stDoIPCfg.Connection_Table[TxPduId].ulInitialInactivityTimer = 0;/* Stop */
	/* SWS_DoIP_00115 */
	#if 0
	- unregister and release the socket connection to the related Tester,
	- discard the ongoing diagnostic message processing and
	- reset the inactivity timer of the given socket connection
	TesterCnt--;
	#endif
	return E_OK;
}

static boolean isSourceAddressKnown(uint16 SA) 
{
	uint16 i;
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

	for (i = 0; i < DOIP_SOAD_CONNECTIONS_MAX; i++) 
	{
		if (s_stDoIPCfg.Connection_Table[i].uiTA == SA) 
		{
			return TRUE;
		}
	}

	return FALSE;
}

static boolean isRoutingTypeSupported(uint16 ActivationType) 
{
	uint16 i;
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

	for (i = 0; i < DOIP_SOAD_CONNECTIONS_MAX; i++) 
	{
		if (DOIP_ROUT_ACT_NUM == ActivationType) 
		{
			return TRUE;
		}
	}

	return FALSE;
}

static boolean isAuthenticationRequired(uint16 ActivationType) 
{
	uint16 i;
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

	for (i = 0; i < DOIP_SOAD_CONNECTIONS_MAX; i++) 
	{
		if (DOIP_ROUT_ACT_NUM == ActivationType) 
		{
#if 0
			if (NULL == g_stDoIPCfg.DoIPConfigSet.DoIPRoutingActivation[i].DoIPRoutingActivationAuthenticationCallback) 
			{
				return FALSE;
			} 
			else 
			{
				return TRUE;
			}
#endif
		}
	}
	return FALSE;
}

static boolean isAuthenticated(uint16 SA, uint8 SoConId) 
{	
	uint16 i;PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);
	for (i = 0; i < DOIP_SOAD_CONNECTIONS_MAX; i++) 
	{
		if ((SA == s_stDoIPCfg.Connection_Table[i].uiTA) 
		 && (SoConId == s_stDoIPCfg.Connection_Table[i].ucSoConId)
		 && (DOIP_CONCTS_REGISTERED_PEND_AUTHEN == s_stDoIPCfg.Connection_Table[i].ucAuthenticationInfo) ) 
		{
			return TRUE;
		}
	}
	return FALSE;
}

static boolean isConfirmationRequired(uint16 ActivationType) 
{
	uint16 i;
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

	for (i = 0; i < DOIP_SOAD_CONNECTIONS_MAX; i++) 
	{
		if (DOIP_ROUT_ACT_NUM == ActivationType) 
		{
#if 0
			if (NULL == g_stDoIPCfg.DoIPConfigSet.DoIPRoutingActivation[i].DoIPRoutingActivationConfirmationCallback) 
			{
				return FALSE;
			} 
			else 
			{
				return TRUE;
			}
#endif
		}
	}
	return FALSE;
}

static boolean isConfirmed(uint16 SA, uint8 SoConId) 
{
	uint16 i;PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);
	for (i = 0; i < DOIP_SOAD_CONNECTIONS_MAX; i++) 
	{
		if ((SA == s_stDoIPCfg.Connection_Table[i].uiTA) 
		 && (SoConId == s_stDoIPCfg.Connection_Table[i].ucSoConId)
		 && (DOIP_CONCTS_REGISTERED_PEND_CONFIRM == s_stDoIPCfg.Connection_Table[i].ucAuthenticationInfo) ) 
		{
			return TRUE;
		}
	}
	return FALSE;

}
static void handleTimeout(uint16 connectionIndex) 
{
	uint8 routingActivationResponseCode = 0x10;
	PduInfoType stPduInfo;
	PduIdType ucTxPduId;
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);

	DoIP_SocketClose(s_stDoIPCfg.Connection_Table[connectionIndex].ucSoConId);

	if ((pendingRoutingActivationSocket != 0xff) && (NULL != pendingRoutingActivationTxBuffer)) 
	{
		DoIP_Inner_RegisterSocket(connectionIndex, pendingRoutingActivationSocket, pendingRoutingActivationActivationType, pendingRoutingActivationSa);

		DoIP_Inner_FillHeader(pendingRoutingActivationTxBuffer, DOIP_PT_ROUTACTVTN_RESP, DOIP_PL_ROUTACTVTN_RESP_13);
		pendingRoutingActivationTxBuffer[8] = pendingRoutingActivationSa >> 8;
		pendingRoutingActivationTxBuffer[8 + 1] = pendingRoutingActivationSa;
		pendingRoutingActivationTxBuffer[8 + 2] = (DOIP_LOGICAL_ADDRESS /*g_stDoIPCfg.DoIPConfigSet.DoIPLogicalAddress*/ >> 8) & 0xff;
		pendingRoutingActivationTxBuffer[8 + 3] = (DOIP_LOGICAL_ADDRESS /*g_stDoIPCfg.DoIPConfigSet.DoIPLogicalAddress*/ >> 0) & 0xff;
		pendingRoutingActivationTxBuffer[8 + 4] = routingActivationResponseCode;

		ucTxPduId = s_stDoIPCfg.Connection_Table[connectionIndex].DoIPSoAdPduId;
		stPduInfo.SduLength = DOIP_GH_TOTAL+DOIP_PL_ROUTACTVTN_RESP_13;
		stPduInfo.SduDataPtr =	pendingRoutingActivationTxBuffer;

		if (E_NOT_OK == s_stDoIPCfg.Connection_Table[connectionIndex].pfSoAd_Transmit(ucTxPduId, &stPduInfo))
		{
			/* SWS_DoIP_00223 */
			DiscardMsg();
		}

		SoAd_BufferFree(pendingRoutingActivationTxBuffer);
		pendingRoutingActivationTxBuffer = NULL;
		pendingRoutingActivationSocket = 0xff;
	} 
	else
	{
		s_stDoIPCfg.Connection_Table[connectionIndex].ucConnectionState = DOIP_CONCTS_LISTEN;
		PR_DEBUG(DEBUG_SWITCH, "%d-----ucConnectionState:%d\n", __LINE__,s_stDoIPCfg.Connection_Table[connectionIndex].ucConnectionState);
		s_stDoIPCfg.Connection_Table[connectionIndex].ucAwaitingAliveCheckResponse = FALSE;
	}

}


static uint8 CheckSaTa(uint16 id,uint16 SA, uint16 TA, uint32 ulPayLoadLength,uint8* ucPduRIndex)
{
	uint8 Rte = E_OK;
#if 1
	uint8 ucLoop = 0;
	(void)ulPayLoadLength;
	
	if (TA == s_stDoIPCfg.Connection_Table[id].uiSA)
	{
		if (SA == s_stDoIPCfg.Connection_Table[id].uiTA)
		{
			for (ucLoop = 0; ucLoop < DOIP_UPLA_CHANNEL_MAX; ucLoop++)
			{
				if (s_stDoIP_TAChannel[ucLoop].uiTA == s_stDoIPCfg.Connection_Table[id].uiTA)
				{
					*ucPduRIndex = s_stDoIP_TAChannel[ucLoop].ucPduRRxPduId;
					break;
				}
			}
			if (ucLoop < DOIP_UPLA_CHANNEL_MAX)
			{
				Rte = E_OK;
			}
			else
			{
				Rte = DOIP_E_DIAG_INVALID_SA;
			}
		}
		else
		{
			Rte = DOIP_E_DIAG_INVALID_SA;
		}
	}
	else
	{
		Rte = DOIP_E_DIAG_UNKNOWN_TA;
	}

#else
	uint8 ucIndex;
	uint16 routingActivationIndex = 0xffff;
	(void)ucPduRIndex;
	PR_DEBUG(DEBUG_SWITCH, "%s\n", __func__);
	for (ucIndex = 0; ucIndex < DOIP_SOAD_CONNECTIONS_MAX; ucIndex++) 
	{
		if ( (SA == s_stDoIPCfg.Connection_Table[ucIndex].uiSA)  )
		{
			break;
		}
	}
	if (DOIP_SOAD_CONNECTIONS_MAX == ucIndex)	/* SWS_DoIP_00123 */
	{
		DoIP_Inner_SendDiagMsgNack(id, SA, TA, DOIP_E_DIAG_INVALID_SA,ulPayLoadLength);	
		DoIP_SocketClose(id);			
	}
	else
	{
		
	}

	if (0xffff == ucIndex) 
	{
		// Connection not registered!
		return LOOKUP_SA_TA_SAERR;
	}

	for (ucIndex = 0; ucIndex < DOIP_SOAD_CONNECTIONS_MAX; ucIndex++)
	{
		if (s_stDoIPCfg.Connection_Table[ucIndex].ucActivationType == DOIP_ROUT_ACT_NUM) 
		{
			routingActivationIndex = ucIndex;
			break;
		}
	}

	if (0xffff == routingActivationIndex) 
	{
		// No such routing activation!
		return LOOKUP_SA_TA_TAUNKNOWN;
	}

	for (ucIndex = 0; ucIndex < DOIP_SOAD_CONNECTIONS_MAX; ucIndex++) 
	{
//		uint16 itTargetIndex = DoIPTargetAddressValue /*g_stDoIPCfg.DoIPConfigSet.DoIPConnections[ucIndex].DoIPTargetAddress.DoIPTargetAddressValue */;

		if ((TA == DoIPTargetAddressValue /*g_stDoIPCfg.DoIPConfigSet.DoIPConnections[ucIndex].DoIPTargetAddress.DoIPTargetAddressValue */)
		  &&(routingActivationIndex == DOIP_ROUT_ACT_NUM))
		{
			//TA = itTargetIndex;
			return LOOKUP_SA_TA_OK;
		}
	}
#endif
	return Rte;
}




/* EOF */





