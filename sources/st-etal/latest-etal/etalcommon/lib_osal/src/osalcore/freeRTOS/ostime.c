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

/* fine control over COMPONENT TracePrintf */
/* 1 to enable traces for this module */
#define STRACE_CONTROL 0

/************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|-----------------------------------------------------------------------*/
#include "osal.h"
#include "task.h"

/************************************************************************
|defines and macros (scope: module-local)
|-----------------------------------------------------------------------*/
#define USEC_MAX   1000000

/************************************************************************
|typedefs (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
| variable definition (scope: module-local)
|-----------------------------------------------------------------------*/
TickType_t startup_tv;

/************************************************************************
| variable definition (scope: global)
|-----------------------------------------------------------------------*/

/************************************************************************
|function prototype (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
|function implementation (scope: module-local)
|-----------------------------------------------------------------------*/

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
	TickType_t tv;

	tv = xTaskGetTickCount();
	
    return ((tv - startup_tv)*1000/configTICK_RATE_HZ) ;
}

void OSAL_ClockResetTime(void)
{
	startup_tv = xTaskGetTickCount();
}

#ifdef __cplusplus
}
#endif

/** @} */

/* End of File */
