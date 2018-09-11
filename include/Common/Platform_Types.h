/*  BEGIN_FILE_HDR
********************************************************************************
*   NOTICE                              
*   This software is the property of HiRain Technologies. Any information 
*   contained in this doc should not be reproduced, or used, or disclosed 
*   without the written authorization from HiRain Technologies.
********************************************************************************
*   File Name       : Platform_Types.h
********************************************************************************
*   Project/Product : Common
*   Title           : Platform_Types.h 
*   Author          : ning.chen
********************************************************************************
*   Description     : platform related types                         
*
********************************************************************************
*   Limitations     : only used for Freescale S12X 
*
********************************************************************************
*
********************************************************************************
*   Revision History:
* 
*   Version     Date          Initials      CR#          Descriptions
*   ---------   ----------    ------------  ----------   ---------------
*   01.00.00    13/06/2013    ning.chen     N/A          Original
*   01.00.01    17/07/2013    ning.chen     N/A          DPN_COMMON_130717_01
*   01.00.02    15/08/2013    ning.chen     N/A          D10_COMMON51_130815_01
*   01.00.03    08/01/2014    ning.chen     N/A          D10_COMMON51_140108_01
********************************************************************************
* END_FILE_HDR*/

#ifndef _PLATFORM_TYPES_H_
#define _PLATFORM_TYPES_H_

/*******************************************************************************
*   Includes 
*******************************************************************************/

/*******************************************************************************
*   Macro 
*******************************************************************************/
#define CPU_TYPE_8       (8)
#define CPU_TYPE_16      (16)
#define CPU_TYPE_32      (32)

#define MSB_FIRST        (0)    /* big endian bit ordering */
#define LSB_FIRST        (1)    /* little endian bit ordering */

#define HIGH_BYTE_FIRST  (0)    /* big endian byte ordering */
#define LOW_BYTE_FIRST   (1)    /* little endian byte ordering */

#ifndef TRUE
   #define TRUE   (1)
#endif
    
#ifndef FALSE
   #define FALSE  (0)
#endif


#define CPU_TYPE            CPU_TYPE_16 
#define CPU_BIT_ORDER       LSB_FIRST 
#define CPU_BYTE_ORDER      HIGH_BYTE_FIRST                      

#ifndef TRUE
#define TRUE             1U
#endif

#ifndef FALSE
#define FALSE            0U
#endif

#ifndef NULL
#define NULL         	 ((void*)0)
#endif

/*******************************************************************************
*   Typedef 
*******************************************************************************/
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

#endif  /* _PLATFORM_TYPES_H_ */

