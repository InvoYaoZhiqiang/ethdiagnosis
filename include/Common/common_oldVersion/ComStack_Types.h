/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : ComStack_Types.h
*   Module Name     : 
*   Project         : Autosar
*   Processor       : MC9S12G128MLH
*   Description		: 
*   Component		: 
************************************************************************************************
*   Revision History:
*
*   Version     Date          Initials      CR#          Descriptions
*   ---------   ----------    -----------   ----------   ---------------
*   1.0         2017/02/09    ZhuLiya       N/A          Original
************************************************************************************************
*	END_FILE_HDR*/

#ifndef COMSTACK_TYPES_H
#define COMSTACK_TYPES_H

#include "Std_Types.h"
#include "ComStack_Cfg.h"

typedef struct
{
	uint8* SduDataPtr;
	uint8* MetaDataPtr;
	PduLengthType SduLength;
}PduInfoType;

typedef enum
{
	TP_STMIN = 0x00u,
	TP_BS    = 0x01u,
	TP_BC    = 0x02u
}TPParameterType;

typedef enum
{
	BUFREQ_OK       = 0x00u,
	BUFREQ_E_NOT_OK = 0x01u,	
	BUFREQ_E_BUSY   = 0x02u,
	BUFREQ_E_OVFL   = 0x03u
}BufReq_ReturnType;

typedef enum
{
	TP_DATACONF    = 0x00u,
	TP_DATARETRY   = 0x01u,
	TP_CONFPENDING = 0x02u
}TpDataStateType;

typedef struct
{
	TpDataStateType TpDataState;
	PduLengthType TxTpDataCnt;
}RetryInfoType;

typedef uint8 NetworkHandleType;
typedef uint8 IcomConfigIdType;

typedef enum
{
	ICOM_SWITCH_E_OK     = 0x00u,
	ICOM_SWITCH_E_FAILED = 0x01u	
}IcomSwitch_ErrorType;


#endif /* COMSTACK_TYPES_H */




