/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : Std_Types.h
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

#ifndef STD_TYPES_H
#define STD_TYPES_H

#include "Compiler.h"
#include "Platform_Types.h"

typedef uint32 EcuM_WakeupSourceType;

typedef uint8 Std_ReturnType;

typedef struct                                          
{
    uint16 vendorID;                           
    uint16 moduleID;
    uint8  sw_major_version;                         
    uint8  sw_minor_version;
    uint8  sw_patch_version;
}Std_VersionInfoType;


#ifndef STATUSTYPEDEFINED 
#define STATUSTYPEDEFINED

#define E_OK 0x00u 
typedef unsigned char StatusType; /* OSEK compliance */ 

#endif /*STATUSTYPEDEFINED*/

#define E_NOT_OK 0x01u

#define STD_HIGH 0x01u /* Physical state 5V or 3.3V */ 
#define STD_LOW 0x00u  /* Physical state 0V */

#define STD_ACTIVE 0x01u /* Logical state active */ 
#define STD_IDLE 0x00u   /* Logical state idle */

#define STD_ON 0x01u 
#define STD_OFF 0x00u


#endif /*STD_TYPES_H*/



