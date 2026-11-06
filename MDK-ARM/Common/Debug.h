#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "usart.h"
#include "stdio.h"
#include "stdarg.h"

#define DEBUG_LOG_ENABLE 1
#ifdef DEBUG_LOG_ENABLE
#define Debug_Printf(format,...) printf("[%s:%d]"   format, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define Debug_Printf(format,...)
#endif 

#endif 
