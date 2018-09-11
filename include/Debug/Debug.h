/*************************************************************************
	> File Name: Debug.h
	> Author:Yaozhiqiang 
	> Mail:yao19911220@163.com 
	> Created Time: Fri 13 Jul 2018 08:28:35 PM CST
 ************************************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H

#include <include/link_api/system_trace.h>

#define DEBUG_SWITCH 	0
#define ENABLE_DEBUG    0

//first
#if (defined(_DEBUG_))
#define PR_DEBUG(param, ...)  \
	do { param ? Vps_printf(__VA_ARGS__) : (void)0; } while(0)
#else
#define PR_DEBUG(...)
#endif

#if 0
//second
#define PR_DEBUG(param, fmt...) \
	do { param ? Vps_printf(fmt) : (void)0; } while(0)

//third
#define PR_DEBUG(param, fmt, ...) \
	do { param ? Vps_printf(fmt, ##__VA_ARGS__) : (void)0; } while(0)
#endif

#endif
