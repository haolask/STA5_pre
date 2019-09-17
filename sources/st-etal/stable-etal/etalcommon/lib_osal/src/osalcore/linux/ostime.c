//!
//!  \file 		ostime.c
//!  \brief 	<i><b>OSAL Timers Handling Functions</b></i>
//!  \details	This is the implementation file for the OSAL
//!             (Operating System Abstraction Layer) timer Functions.
//!  \author 	Raffaele Belardi
//!  \author 	(original version) Luca Pesenti
//!  \version 	1.0
//!  \date 		07 Sept 2010
//!  \bug 		Unknown
//!  \warning	None
//!
#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|-----------------------------------------------------------------------*/
#include "target_config.h"

#include "osal.h"
#include <sys/time.h> /* for gettimeofday */

/************************************************************************
|defines and macros (scope: module-local)
|-----------------------------------------------------------------------*/
#define USEC_MAX   1000000

/************************************************************************
| variable definition (scope: module-local)
|-----------------------------------------------------------------------*/
static struct timeval startup_tv;

/************************************************************************
|function implementation (scope: global)
|-----------------------------------------------------------------------*/

/**
 *
 * @brief    OSAL_ClockGetElapsedTime;
 *
 * @details  This Function returns the elapsed time since the start of the
 *              system through OSAL_Boot() in milliseconds.
 *
 * @return   Time in milliseconds
 *
 */
OSAL_tMSecond OSAL_ClockGetElapsedTime(void)
{
	struct timeval tv;
	int sec, usec;

	gettimeofday(&tv, NULL);
	if (startup_tv.tv_usec > tv.tv_usec)
	{
		tv.tv_sec--;
		tv.tv_usec += USEC_MAX;
	}
	sec = tv.tv_sec - startup_tv.tv_sec;
	usec = tv.tv_usec - startup_tv.tv_usec;
    return (OSAL_tMSecond)((sec * 1000) + (usec / 1000));
}

void OSAL_ClockResetTime(void)
{
	gettimeofday(&startup_tv, NULL);
}

#ifdef __cplusplus
}
#endif

/** @} */

/* End of File */
