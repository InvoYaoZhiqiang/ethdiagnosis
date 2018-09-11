/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : CommonError.h
*	Description		: Contains definitions of error types, which can be used in user application.
************************************************************************************************
*	END_FILE_HDR*/

#ifndef COMMON_ERROR_H
#define COMMON_ERROR_H


typedef	unsigned long int       ER_RESULT;			        /* Error result type */

#define	ER_OK					(ER_RESULT)(0x00000000U)	/* OK, no error */
#define	ER_PARA_CONST			(ER_RESULT)(0x00000001U)	/* Parameter loaded from NVM error */
#define	ER_PARA_RANGE			(ER_RESULT)(0x00000002U)	/* Parameter out of range */
#define	ER_PARA_VALUE			(ER_RESULT)(0x00000004U)	/* Parameter of incorrect value */
#define	ER_OVERTIME				(ER_RESULT)(0x00000008U)	/* Wait overtime */
#define	ER_BUSY					(ER_RESULT)(0x00000010U)	/* Device of Module is busy */
#define	ER_NOT_INIT				(ER_RESULT)(0x00000020U)	/* Device of Module has not been init */
#define	ER_NOT_SUPPORT			(ER_RESULT)(0x00000040U)	/* Request not support */
#define	ER_BUFF_EMPTY			(ER_RESULT)(0x00000080U)	/* Buffer is empty */
#define	ER_BUFF_FULL			(ER_RESULT)(0x00000100U)	/* Buffer is full */
#define	ER_HW_PER				(ER_RESULT)(0x00000200U)	/* Internal peripherals error */
#define	ER_HW_IC				(ER_RESULT)(0x00000400U)	/* External IC error */
#define	ER_ACCESS				(ER_RESULT)(0x00000800U)	/* Can not access to the desire area */
#define	ER_CHECK				(ER_RESULT)(0x00001000U)	/* Value Check Error */
#define	ER_BUS_OFF				(ER_RESULT)(0x00002000U)	/* CAN bus-off */
#define	ER_ABORT				(ER_RESULT)(0x00004000U)	/* Process has been aborted */
#define	ER_OVERFLOW				(ER_RESULT)(0x00008000U)	/* Data overflow */
#define	ER_UNKNOW				(ER_RESULT)(0x80000000U)	/* Unknow */

#endif	/* COMMON_ERROR_H */


