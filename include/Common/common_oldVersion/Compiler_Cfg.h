/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : Compiler_Cfg.h
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

#ifndef COMPILER_CFG_H
#define COMPILER_CFG_H

/*Syntax of memory class (memclass) and pointer class (ptrclass) macro parameter*/
#define CAN_CODE
#define CAN_CONST
#define CAN_APPL_DATA
#define CAN_APPL_CONST
#define CAN_VAR

#define CANIF_CODE
#define CANIF_CONST
#define CANIF_APPL_DATA
#define CANIF_APPL_CONST
#define CANIF_VAR

#define CANNM_CODE
#define CANNM_CONST
#define CANNM_APPL_DATA
#define CANNM_APPL_CONST
#define CANNM_VAR


#define DET_CODE
#define DET_CONST
#define DET_APPL_DATA
#define DET_APPL_CONST
#define DET_VAR


#endif /*COMPILER_CFG_H*/



