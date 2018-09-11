/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : Platform_Types.h
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

#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

typedef unsigned char boolean;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;

typedef signed char sint8;
typedef signed short sint16;
typedef signed int sint32;
typedef signed long long sint64;

typedef signed char sint8_least;
typedef unsigned char uint8_least;
typedef signed short sint16_least;
typedef unsigned short uint16_least;
typedef signed long sint32_least;
typedef unsigned long uint32_least;

typedef float  float32;
typedef double float64;

#ifndef BOOL
typedef unsigned char	 BOOL;
#endif

#ifndef TRUE
#define TRUE             1U
#endif

#ifndef FALSE
#define FALSE            0U
#endif

#ifndef NULL
#define NULL         	 ((void*)0)
#endif

#define CPU_TYPE_8       8u
#define CPU_TYPE_16      16u
#define CPU_TYPE_32      32u
#define CPU_TYPE_64      64u

#define MSB_FIRST        0u 
#define LSB_FIRST        1u 

#define HIGH_BYTE_FIRST  0u
#define LOW_BYTE_FIRST   1u

#define CPU_TYPE         CPU_TYPE_16
#define CPU_BIT_ORDER    LSB_FIRST
#define CPU_BYTE_ORDER   HIGH_BYTE_FIRST

#endif /*PLATFORM_TYPES_H*/
