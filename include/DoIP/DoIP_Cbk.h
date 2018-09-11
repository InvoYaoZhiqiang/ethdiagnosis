/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : DoIP_Cbk.h
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

#ifndef DOIP_CBK_H
#define DOIP_CBK_H

extern BufReq_ReturnType DoIP_SoAdTpCopyTxData(PduIdType id, const PduInfoType* info, RetryInfoType* retry, PduLengthType* availableDataPtr);
extern BufReq_ReturnType DoIP_SoAdTpCopyRxData(PduIdType id, const PduInfoType* info, PduLengthType* bufferSizePtr);
extern void DoIP_SoAdTpTxConfirmation(PduIdType id, Std_ReturnType result);
extern BufReq_ReturnType DoIP_SoAdTpStartOfReception(PduIdType id, const PduInfoType* info, PduLengthType TpSduLength, PduLengthType* bufferSizePtr);
extern void DoIP_SoAdTpRxIndication(PduIdType id, Std_ReturnType result);
extern void DoIP_SoAdIfRxIndication(PduIdType RxPduId, const PduInfoType* PduInfoPtr);
extern void DoIP_SoAdIfTxConfirmation(PduIdType TxPduId, Std_ReturnType result);
extern void DoIP_SoConModeChg(SoAd_SoConIdType SoConId, SoAd_SoConModeType Mode);
extern void DoIP_LocalIpAddrAssignmentChg(SoAd_SoConIdType SoConId,TcpIp_IpAddrStateType State);
extern void DoIP_ActivationLineSwitchActive(void);	
extern void DoIP_ActivationLineSwitchInactive(void);


#endif /* DOIP_CBK_H */










