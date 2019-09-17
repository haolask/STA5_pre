#include <stdarg.h>

#ifdef RVDEC_TRACE

/* ## added to take into account the case 'no parameter' */
#define RV_API_TRC(str, ...) pr_debug(str, ## __VA_ARGS__)

/* no usage of __VA_ARGS__ because caller is using two brackets */
#define RVDEC_API_DEBUG(str, ...) pr_debug str
#define RVDEC_DEBUG(str, ...) pr_debug str

#else
#define RV_API_TRC(str, ...)
#define RVDEC_API_DEBUG(str, ...)
#define RVDEC_DEBUG(str, ...)
#endif
#define RVFLUSH