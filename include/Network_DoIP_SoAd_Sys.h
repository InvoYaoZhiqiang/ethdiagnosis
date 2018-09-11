/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : Network_DoIP_SoAd.h
*   Module Name     : 
*   Project         : Autosar
*   Processor       : 
*   Description		: AUTOSAR NDK legacy basic stack structure support
*   Component		: 
************************************************************************************************
*   Revision History:
*
*   Version     Date          Initials      CR#          Descriptions
*   ---------   ----------    -----------   ----------   ---------------
*   1.0         2018/07/19    YaoZhiqiang   N/A          Original
************************************************************************************************
*	END_FILE_HDR*/

#ifndef _C_SYS_H_
#define _C_SYS_H_

/* C++ / C Function Declarations */
#ifdef __cplusplus
#define _extern extern "C"
#define _externfar extern "C" far
#else
#define _extern extern
#define _externfar extern far
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(_BIG_ENDIAN) || defined(_big_endian__))
#define BIGENDIAN
#endif

#if (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN))
#if (__BYTE_ORDER == __BIG_ENDIAN)
#define BIGENDIAN
#endif
#endif

#ifdef __cplusplus
}
#endif /* extern "C" */


/* Standard C includes */
#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h> 
#include <string.h>
#include <strings.h> 

/* NDK common headers. */
#include <ti/ndk/inc/usertype.h>
#include <ti/ndk/inc/socket.h> /* use original NDK style sockets prototypes */
#include <ti/ndk/inc/serrno.h>

/* Operating System */
#include <ti/ndk/inc/os/osif.h>

/* Hardware Driver Support */
#include <ti/ndk/inc/hal/hal.h>

/* NDK interfaces. */
#include <ti/ndk/inc/stack/stack.h>

#include "Common/CommonError.h" /* AUTOSAR COMMON ERRORS */
#include "Common/ComStack_Types.h" /* AUTOSAR COMMON STACK TYPES */

#include "TcpIp_SoAd/TcpIp_SoAd.h" /* AUTOSAR NDK TCPIP SOCKET SUPPORT */
#include "SoAd/SoAd.h" /* AUTOSAR NDK SOCKET ADAPTER SUPPORT */

/*AUTOSAR DoIP MODULE DEFINATION. */
#include "DoIP/DoIP_Cfg.h"
#include "DoIP/DoIP_Cbk.h"
#include "DoIP/DoIP.h"

/* DEBUG DEFINATION. */
#define _DEBUG_

/* Debug global. */
#include "Debug/Debug.h"

#endif
