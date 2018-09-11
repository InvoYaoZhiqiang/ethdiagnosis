/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : DoIP_Cfg.h
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

#ifndef DOIP_CFG_H
#define DOIP_CFG_H

#define DOIP_PROTOCOL_VERSION	(uint8)1u
#define DOIP_INVERSE_PROTOCOL_VERSION	(uint8)(~DOIP_PROTOCOL_VERSION)

/*************************************/
#define DOIP_PDUID_RX_MAX	 (uint8)4u				
#define DOIP_PDUID_TX_MAX	 (uint8)2u	

/*************************************/




#define DOIPEID					0 //{ 1, 2, 3, 4, 5, 6}

#define DOIPGID					0  //	{ , , , , , }

#define DOIP_LOGICAL_ADDRESS	(uint16)0xFFFFu

#define DOIP_TARGET_COUNT		(uint16)0xFFFFu


#define	DOIP_ALIVECHECKRESPONSETIMEOUT	500u	//uint32	DoIPAliveCheckResponseTimeout;
#define	DOIP_DEVELOPMENTERRORDETECT		TRUE	//boolean 	DoIPDevelopmentErrorDetect;
#define	DOIP_DHCPOPTIONVINUSE			1	//boolean	DoIPDhcpOptionVinUse;
#define	DOIP_ENTITYSTATUSMAXBYTEFIELDUSE TRUE	//boolean	DoIPEntityStatusMaxByteFieldUse;
#define	DOIP_GENERALINACTIVITYTIME		200	//uint32	DoIPGeneralInactivityTime;
#define	DOIP_GIDINVALIDITYPATTERN 		0	//uint8 	DoIPGIDInvalidityPattern;
#define	DOIP_HEADERFILEINCLUSION 		0		//uint8 *	DoIPHeaderFileInclusion;
#define	DOIP_HOSTNAMESIZEMAX 			5		//uint8 	DoIPHostNameSizeMax;
#define	DOIP_INITIALINACTIVITYTIME 		50		//uint32	DoIPInitialInactivityTime;
#define	DOIP_INITIALVEHICLEANNOUNCEMENTTIME 20	//uint32	DoIPInitialVehicleAnnouncementTime;
#define	DOIP_MAINFUNCTIONPERIOD			10u		//uint32	DoIPMainFunctionPeriod;
#define	DOIP_MAXREQUESTBYTES 			255ul	//uint32	DoIPMaxRequestBytes;
#define	DOIP_MAXTESTERCONNECTIONS		255u	//uint8 	DoIPMaxTesterConnections;
#define	DOIP_MAXUDPREQUESTPERMESSAGE 	2u		//uint8 	DoIPMaxUDPRequestPerMessage;
#define	DOIP_NODETYPE 			   DOIP_NODE 	//DoIPNodeType_Enum DoIPNodeType;
#define	DOIP_USEEIDASGID 				TRUE		//boolean	DoIPUseEIDasGID;
#define	DOIP_USEMACADDRESSFORIDENTIFICATION	TRUE	//boolean	DoIPUseMacAddressForIdentification;
#define	DOIP_USEVEHICLEIDENTIFICATIONSYNCSTATUS TRUE //boolean	DoIPUseVehicleIdentificationSyncStatus;
#define	DOIP_VEHICLEANNOUNCEMENTINTERVAL 3		//uint32	DoIPVehicleAnnouncementInterval;
#define	DOIP_VEHICLEANNOUNCEMENTREPETITION 3u	//uint8 	DoIPVehicleAnnouncementRepetition;
#define	DOIP_VERSIONINFOAPI 			TRUE	//boolean	DoIPVersionInfoApi;
#define	DOIP_VINGIDMASTER 				TRUE	//boolean	DoIPVinGidMaster;
#define	DOIP_VININVALIDITYPATTERN 		0u		//uint8 	DoIPVinInvalidityPattern;


#if 0
extern DoIPGetGidCallback UDS_DoIPGetGidDirect_Callback(void);
extern DoIPPowerModeCallback UDS_DoIPPowerModeDirect_Callback(void);
extern DoIPTriggerGidSyncCallback UDS_DoIPTriggerGidSyncDirect_Callback(void);

const DoIPGeneral_Type DoIPGeneral_Cfg = 
{
	DOIP_ALIVECHECKRESPONSETIMEOUT,		//uint32	DoIPAliveCheckResponseTimeout;
	DOIP_DEVELOPMENTERRORDETECT,		//boolean 	DoIPDevelopmentErrorDetect;
	DOIP_DHCPOPTIONVINUSE,				//boolean 	DoIPDhcpOptionVinUse;
	DOIP_ENTITYSTATUSMAXBYTEFIELDUSE,	//boolean 	DoIPEntityStatusMaxByteFieldUse;
	DOIP_GENERALINACTIVITYTIME,			//uint32	DoIPGeneralInactivityTime;
	DOIP_GIDINVALIDITYPATTERN,			//uint8		DoIPGIDInvalidityPattern;
	DOIP_HEADERFILEINCLUSION,			//uint8 * 	DoIPHeaderFileInclusion;
	DOIP_HOSTNAMESIZEMAX,				//uint8		DoIPHostNameSizeMax;
	DOIP_INITIALINACTIVITYTIME,			//uint32	DoIPInitialInactivityTime;
	DOIP_INITIALVEHICLEANNOUNCEMENTTIME,//uint32	DoIPInitialVehicleAnnouncementTime;
	DOIP_MAINFUNCTIONPERIOD,			//uint32	DoIPMainFunctionPeriod;
	DOIP_MAXREQUESTBYTES,				//uint32	DoIPMaxRequestBytes;
	DOIP_MAXTESTERCONNECTIONS,			//uint8		DoIPMaxTesterConnections;
	DOIP_MAXUDPREQUESTPERMESSAGE,		//uint8		DoIPMaxUDPRequestPerMessage;
	DOIP_NODETYPE,						//DoIPNodeType_Enum DoIPNodeType;
	DOIP_USEEIDASGID,					//boolean 	DoIPUseEIDasGID;
	DOIP_USEMACADDRESSFORIDENTIFICATION,//boolean 	DoIPUseMacAddressForIdentification;
	DOIP_USEVEHICLEIDENTIFICATIONSYNCSTATUS,//boolean	 DoIPUseVehicleIdentificationSyncStatus;
	DOIP_VEHICLEANNOUNCEMENTINTERVAL,	//uint32	DoIPVehicleAnnouncementInterval;
	DOIP_VEHICLEANNOUNCEMENTREPETITION,	//uint8		DoIPVehicleAnnouncementRepetition;
	DOIP_VERSIONINFOAPI,				//boolean 	DoIPVersionInfoApi;
	DOIP_VINGIDMASTER,					//boolean 	DoIPVinGidMaster;
	DOIP_VININVALIDITYPATTERN,			//uint8		DoIPVinInvalidityPattern;
	UDS_DoIPGetGidDirect_Callback,			//DoIPGetGidCallback			DoIPGetGidDirect;
	UDS_DoIPPowerModeDirect_Callback,		//DoIPPowerModeCallback			DoIPPowerModeDirect;
	UDS_DoIPTriggerGidSyncDirect_Callback,	//DoIPTriggerGidSyncCallback	DoIPTriggerGidSyncDirect;
};
#endif


#if 0
const DoIPChannel_Type DoIPChannel_Cfg[] = 
{
	.DoIPChannelSARef = ;
	.DoIPChannelTARef = ;
	.DoIPPduRRxPdu = ;
	.DoIPPduRTxPdu = ;
};

const DoIPRoutingActivation_Type DoIPRoutingActivation_Cfg = 
{
	.DoIPRoutingActivationNumber = ;
	.DoIPTargetAddressRef = ; 
	.DoIPRoutingActivationAuthenticationCallback = ;
	.DoIPRoutingActivationConfirmationCallback = ;
};

const DoIPTester_Type DoIPTester_Cfg[] = 
{
	.DoIPNumByteDiagAckNack = ;
	.DoIPTesterSA = ;
	.DoIPRoutingActivationRef = ;
};




const DoIPTargetAddress_Type DoIPTargetAddress_Cfg = 
{
	.DoIPTargetAddressValue = TA;
};

const DoIPSoAdRxPdu_Type DoIPSoAdRxPdu_Cfg[2] = 
{
	{
		.DoIPSoAdRxPduId = 0;
		.DoIPSoAdRxPduRef = g_stTcpSoAdRxPdu;
	},
	{
		.DoIPSoAdRxPduId = 1;
		.DoIPSoAdRxPduRef = g_stUdpSoAdRxPdu;
	}
};


const DoIPSoAdTxPdu_Type DoIPSoAdTxPdu_Cfg[2] = 
{
	{
		.DoIPSoAdRxPduId = 0;
		.DoIPSoAdRxPduRef = g_stTcpSoAdRxPdu;
	},
	{
		.DoIPSoAdRxPduId = 1;
		.DoIPSoAdRxPduRef = g_stUdpSoAdRxPdu;
	}
};

const DoIPTcpConnection_Type DoIPTcpConnection_Cfg = 
{
	.DoIPSoAdRxPdu = &DoIPSoAdRxPdu_Cfg[0];
	.DoIPSoAdTxPdu = &DoIPSoAdTxPdu_Cfg[0];
};

const DoIPUdpConnection_Type DoIPUdpConnection_Cfg = 
{
	.DoIPSoAdRxPdu = &DoIPSoAdRxPdu_Cfg[1];
	.DoIPSoAdTxPdu = &DoIPSoAdTxPdu_Cfg[1];
};


const DoIPSoAdUdpVehicleAnnouncementTxPdu_Type DoIPSoAdUdpVehicleAnnouncementTxPdu_Cfg = 
{
	.DoIPSoAdUdpVehicleAnnouncementTxPduId = 0x430;
	.DoIPSoAdUdpVehicleAnnouncementTxPduRef = PduInfoType AnnouncementTxPdu;
};

const DoIPUdpVehicleAnnouncementConnection_Type DoIPUdpVehicleAnnouncementConnection_Cfg = 
{
	.DoIPSoAdUdpVehicleAnnouncementTxPdu = DoIPSoAdUdpVehicleAnnouncementTxPdu_Cfg;
};


const DoIPConnections_Type DoIPConnections_Cfg[] = 
{
	{
		.DoIPTargetAddress = &DoIPTargetAddress_Cfg[0];
		.DoIPTcpConnection = &DoIPTcpConnection_Cfg[0];
		.DoIPUdpConnection = &DoIPUdpConnection_Cfg[0];
		.DoIPUdpVehicleAnnouncementConnection = &DoIPUdpVehicleAnnouncementConnection_Cfg[0];
	},
	{
		.DoIPTargetAddress = &DoIPTargetAddress_Cfg[0];
		.DoIPTcpConnection = &DoIPTcpConnection_Cfg[];
		.DoIPUdpConnection = &DoIPUdpConnection_Cfg[];
		.DoIPUdpVehicleAnnouncementConnection = &DoIPUdpVehicleAnnouncementConnection_Cfg[0];
	},
	{
		.DoIPTargetAddress = &DoIPTargetAddress_Cfg[0];
		.DoIPTcpConnection = &DoIPTcpConnection_Cfg[];
		.DoIPUdpConnection = &DoIPUdpConnection_Cfg[];
		.DoIPUdpVehicleAnnouncementConnection = &DoIPUdpVehicleAnnouncementConnection_Cfg[0];
	}
};

const DoIPConfigSet_Type DoIPConfigSet_Cfg = 
{
	{
		.DoIPEid = DOIPEID,							//	uint8 DoIPEid[6];
		.DoIPGid = DOIPGID,							//	uint8 DoIPGid[6];	
		.DoIPLogicalAddress = DOIP_LOGICAL_ADDRESS,	//	uint16 DoIPLogicalAddress;
		.DoIPChannel = DoIPChannel_Cfg,				//	DoIPChannel_Type DoIPChannel[];
		.DoIPRoutingActivation = DoIPRoutingActivation_Cfg,//	DoIPRoutingActivation_Type DoIPRoutingActivation;
		.DoIPTester = DoIPTester_Cfg,				//	DoIPTester_Type DoIPTester;
		.DoIPConnections = DoIPConnections_Cfg,		//	DoIPConnections_Type DoIPConnections;
	}
};

const DoIP_ConfigType g_stDoIPCfg = 
{
	.DoIPGeneral = DoIPGeneral_Cfg,
	.DoIPConfigSet = DoIPConfigSet_Cfg,
};
#endif


#endif /* DOIP_CFG_H */











