/*  BEGIN_FILE_HDR
************************************************************************************************
*   NOTICE
*   This software is the property of IntelliDrive Co.,Ltd.. Any information contained in this
*   doc should not be reproduced, or used, or disclosed without the written authorization from
*   IntelliDrive Co.,Ltd..
************************************************************************************************
*   File Name       : Compiler.h
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

#ifndef COMPILER_H
#define COMPILER_H

#include "Compiler_Cfg.h"

#define AUTOMATIC
#define TYPEDEF
#define NULL_PTR  ((void*)0)

#define FUNC(rettype, memclass) memclass rettype

#define FUNC_P2CONST(rettype, ptrclass, memclass)\
        const ptrclass rettype * memclass
        
#define FUNC_P2VAR(rettype, ptrclass, memclass)\
        ptrclass rettype * memclass        

#define P2VAR(ptrtype, memclass, ptrclass) \
        ptrclass ptrtype * memclass
        
#define P2CONST(ptrtype, memclass, ptrclass) \
        const ptrtype memclass * ptrclass
        
#define CONSTP2VAR(ptrtype, memclass, ptrclass) \
        ptrclass ptrtype * const memclass

#define CONSTP2CONST(ptrtype, memclass, ptrclass) \
        const memclass ptrtype * const ptrclass

#define P2FUNC(rettype, ptrclass, fctname) \
        rettype (*ptrclass fctname)
        
#define CONSTP2FUNC(rettype, ptrclass, fctname) \
        rettype (* const ptrclass fctname)
        
#define CONST(type, memclass) memclass const type

#define VAR(type, memclass) memclass type


#endif /* COMPILER_H */


