/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : DoIP.h
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

#ifndef DOIP_H
#define DOIP_H

/*
** ===================================================================
**     Include files.
** ===================================================================
*/

#include "DoIP_Types.h"

/*
** ===================================================================
**     Macro definition.
** ===================================================================
*/


/*
** ===================================================================
**     func definition.
** ===================================================================
*/

extern Std_ReturnType DoIP_TpTransmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr);
extern Std_ReturnType DoIP_TpCancelTransmit(PduIdType TxPduId);
extern Std_ReturnType DoIP_TpCancelReceive(PduIdType RxPduId);
extern Std_ReturnType DoIP_IfTransmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr);
extern Std_ReturnType DoIP_IfCancelTransmit(PduIdType TxPduId);
extern void DoIP_Init(const DoIP_ConfigType* DoIPConfigPtr);
extern void DoIP_MainFunction(void);
extern void DoIP_GetVersionInfo(Std_VersionInfoType* versioninfo);

#endif /* DOIP_H */




