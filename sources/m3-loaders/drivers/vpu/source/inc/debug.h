
#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "trace.h"

#define ASSERT(expr) TRACE_ASSERT(expr)
#define ERROR_PRINT(expr) TRACE_ERR("g1: " expr)
#define EPRINT(expr) TRACE_ERR("g1: " expr)
#define DEBUG_PRINT(expr) TRACE_INFO( expr)
#define DPRINT(expr) TRACE_INFO (expr)
#define TRACE_PP_CTRL(...) TRACE_INFO(__VA_ARGS__)
#define PPDEBUG_PRINT(...) TRACE_INFO(__VA_ARGS__)

#endif
