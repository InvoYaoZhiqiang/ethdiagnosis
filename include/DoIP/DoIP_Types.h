/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : DoIP_Types.h
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
*   1.0         2018/03/29    PangXD        N/A          Original
*
************************************************************************************************
*	END_FILE_HDR*/

#ifndef DOIP_TYPES_H
#define DOIP_TYPES_H

/*
** ===================================================================
**     Macro definition.
** ===================================================================
*/

typedef enum 
{
	DOIP_GATEWAY = 0,
	DOIP_NODE
}DoIPNodeType_Enum;

typedef enum
{
	DOIP_TPPDU = 0,
	DOIP_IFPDU	
}DoIPPduType_Enum;



/*
** ===================================================================
**     Type definition.
** ===================================================================
*/
typedef void(*vpFunc)(void);
typedef Std_ReturnType (*DoIPGetGidCallback)(uint8* GroupId);
typedef Std_ReturnType (*DoIPPowerModeCallback)(uint8* PowerStateReady);
typedef Std_ReturnType (*DoIPTriggerGidSyncCallback)(void);



/*--------------------------------------------------------------*/
typedef struct
{
	uint32	DoIPAliveCheckResponseTimeout;
	boolean DoIPDevelopmentErrorDetect;
	boolean DoIPDhcpOptionVinUse;
	boolean DoIPEntityStatusMaxByteFieldUse;
	uint32	DoIPGeneralInactivityTime;
	uint8	DoIPGIDInvalidityPattern;
	uint8 * DoIPHeaderFileInclusion;
	uint8	DoIPHostNameSizeMax;
	uint32	DoIPInitialInactivityTime;
	uint32	DoIPInitialVehicleAnnouncementTime;
	uint32	DoIPMainFunctionPeriod;
	uint32	DoIPMaxRequestBytes;
	uint8	DoIPMaxTesterConnections;
	uint8	DoIPMaxUDPRequestPerMessage;
	DoIPNodeType_Enum DoIPNodeType;
	boolean DoIPUseEIDasGID;
	boolean DoIPUseMacAddressForIdentification;
	boolean DoIPUseVehicleIdentificationSyncStatus;
	uint32	DoIPVehicleAnnouncementInterval;
	uint8	DoIPVehicleAnnouncementRepetition;
	boolean DoIPVersionInfoApi;
	boolean DoIPVinGidMaster;
	uint8	DoIPVinInvalidityPattern;
	DoIPGetGidCallback			DoIPGetGidDirect;
	DoIPPowerModeCallback		DoIPPowerModeDirect;
	DoIPTriggerGidSyncCallback	DoIPTriggerGidSyncDirect;
}DoIPGeneral_Type;

typedef struct
{
	PduIdType DoIPPduRRxPduId;
	PduInfoType DoIPPduRRxPduRef;
}DoIPPduRRxPdu_Type;

typedef struct
{
	PduIdType DoIPPduRTxPduId;
	DoIPPduType_Enum DoIPPduType;
	PduInfoType DoIPPduRTxPduRef;
}DoIPPduRTxPdu_Type;

typedef struct
{
	uint16 DoIPTargetAddressValue;
}DoIPTargetAddress_Type;

typedef struct
{
	vpFunc DoIPRoutingActivationAuthenticationFunc;
	uint8 DoIPRoutingActivationAuthenticationReqLength; 
	uint8 DoIPRoutingActivationAuthenticationResLength;
}DoIPRoutingActivationAuthenticationCallback_Type;

typedef struct
{
	vpFunc DoIPRoutingActivationConfirmationFunc;
	uint8 DoIPRoutingActivationConfirmationReqLength; 
	uint8 DoIPRoutingActivationConfirmationResLength;
}DoIPRoutingActivationConfirmationCallback_Type;

typedef struct
{
	uint8 DoIPRoutingActivationNumber;
	DoIPTargetAddress_Type DoIPTargetAddressRef; 
	DoIPRoutingActivationAuthenticationCallback_Type DoIPRoutingActivationAuthenticationCallback;
	DoIPRoutingActivationConfirmationCallback_Type DoIPRoutingActivationConfirmationCallback;
}DoIPRoutingActivation_Type;

typedef struct
{
	uint64 DoIPNumByteDiagAckNack;
	uint16 DoIPTesterSA;
	DoIPRoutingActivation_Type DoIPRoutingActivationRef;
}DoIPTester_Type;

typedef DoIPTester_Type DoIPChannelSARef_Type;
typedef DoIPTester_Type DoIPChannelTARef_Type;

typedef struct
{
	DoIPChannelSARef_Type DoIPChannelSARef;
	DoIPChannelTARef_Type DoIPChannelTARef;
	DoIPPduRRxPdu_Type DoIPPduRRxPdu;
	DoIPPduRTxPdu_Type DoIPPduRTxPdu;
}DoIPChannel_Type;

typedef struct
{
	uint16 DoIPSoAdRxPduId;
	PduInfoType DoIPSoAdRxPduRef;
}DoIPSoAdRxPdu_Type;

typedef struct
{
	uint16 DoIPSoAdTxPduId;
	PduInfoType DoIPSoAdTxPduRef;
}DoIPSoAdTxPdu_Type;

typedef struct
{
	DoIPSoAdRxPdu_Type DoIPSoAdRxPdu;
	DoIPSoAdTxPdu_Type DoIPSoAdTxPdu;
}DoIPTcpConnection_Type;

typedef struct
{
	DoIPSoAdRxPdu_Type DoIPSoAdRxPdu;
	DoIPSoAdTxPdu_Type DoIPSoAdTxPdu;
}DoIPUdpConnection_Type;

typedef struct
{
	uint64 DoIPSoAdUdpVehicleAnnouncementTxPduId;
	PduInfoType DoIPSoAdUdpVehicleAnnouncementTxPduRef;
}DoIPSoAdUdpVehicleAnnouncementTxPdu_Type;

typedef struct
{
	DoIPSoAdUdpVehicleAnnouncementTxPdu_Type DoIPSoAdUdpVehicleAnnouncementTxPdu;
}DoIPUdpVehicleAnnouncementConnection_Type;

typedef struct
{
	DoIPTargetAddress_Type DoIPTargetAddress;
	DoIPTcpConnection_Type DoIPTcpConnection;
	DoIPUdpConnection_Type DoIPUdpConnection;
	DoIPUdpVehicleAnnouncementConnection_Type DoIPUdpVehicleAnnouncementConnection;
}DoIPConnections_Type;

typedef struct
{
	uint8 DoIPEid[6];
	uint8 DoIPGid[6];	
	uint16 DoIPLogicalAddress;
	DoIPChannel_Type DoIPChannel[1];
	DoIPRoutingActivation_Type DoIPRoutingActivation[1];
	DoIPTester_Type DoIPTester[DOIP_MAXTESTERCONNECTIONS];
	DoIPConnections_Type DoIPConnections[1];
}DoIPConfigSet_Type;

typedef struct
{
	DoIPGeneral_Type DoIPGeneral;
	DoIPConfigSet_Type DoIPConfigSet[1];
}DoIP_Type;

typedef DoIP_Type DoIP_ConfigType;



#endif /* DOIP_TYPES_H */









