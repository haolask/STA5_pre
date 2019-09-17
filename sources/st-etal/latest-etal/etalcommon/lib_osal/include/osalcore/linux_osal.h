//!
//!  \file 		linux_osal.h
//!  \brief 	<i><b> OSAL Linux specific definitions</b></i>
//!  \details	This is the header file for the OSAL Abstraction Layer for
//!             Linux-specific definition
//!

#ifndef LINUX_OSAL_H
#define LINUX_OSAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fcntl.h>   /* for O_* constants and mode_t */

#ifdef CONFIG_DEBUG_OSAL
#include <assert.h>
#define ASSERT_ON_DEBUGGING(a) (assert(a))
#else
#define ASSERT_ON_DEBUGGING(a) ((void)(a))
#endif

#define LINUX_OSAL_THREAD_MAX  40
#define LINUX_OSAL_EVENT_MAX  20
#define LINUX_OSAL_MAX_THREAD_PER_EVENT_HANDLER  5
#ifdef OSAL_USE_MQUEUE_FREE_FUNCTION
  #define LINUX_OSAL_MQUEUE_MAX  20
#endif
# ifdef OSAL_USE_SHM_FREE_FUNCTION
  #define LINUX_OSAL_SHM_MAX  20
#endif

/* used by OSALIO.c */
#define PROTECT(_a)   (pthread_mutex_lock(_a))
#define UNPROTECT(_a) (pthread_mutex_unlock(_a))

#ifdef __cplusplus
}
#endif


#else  /* !LINUX_OSAL_H */
#error linux_osal.h included several times
#endif  /* LINUX_OSAL_H */

/* End of File */
