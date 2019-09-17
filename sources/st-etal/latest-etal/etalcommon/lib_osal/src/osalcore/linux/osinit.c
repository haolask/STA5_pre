//!
//!  \file 		osinit.c
//!  \brief 	<i><b>OSAL initialization Functions</b></i>
//!  \details	This is the implementation file for the OSAL
//!            (Operating System Abstraction Layer) System Initilazition Functions.
//!  \author 	Raffaele Belardi
//!  \author 	(original version) Luca Pesenti
//!  \version 	1.0
//!  \date 	13.09.2010	
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

/************************************************************************
|defines and macros (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
|typedefs (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
| variable definition (scope: module-local)
|-----------------------------------------------------------------------*/

/************************************************************************
|function prototype (scope: module-local)
|-----------------------------------------------------------------------*/
extern tS32 OSAL_s32Boot(tS32 cPar, tString aPar[]);

/************************************************************************
|function implementation (scope: module-local)
|-----------------------------------------------------------------------*/

/**
 * @brief     OSAL_INIT
 *
 * @details   This function starts the OSAL environment. This is
 *                used to create the OSAL environment.
 *
 *
 * @return    NULL
 *
 */
tSInt OSAL_INIT (void)
{
    if (Linux_initSemaphores() != OSAL_OK)
	{
		return OSAL_ERROR;
	}
    if (Linux_initProcesses() != OSAL_OK)
	{
		return OSAL_ERROR;
	}

    Linux_initEvent();

	OSAL_ClockResetTime();
#if defined (CONFIG_TRACE_ENABLE)
	OSALUTIL_vTraceInit();
#endif

    return OSAL_OK;
}

tSInt OSAL_DEINIT(void)
{
#if defined (CONFIG_TRACE_ENABLE)
	OSALUTIL_vTraceDeInit();
#endif

	Linux_deinitEvent();

    if (Linux_deinitProcesses() != OSAL_OK)
	{
		return OSAL_ERROR;
	}
    if (Linux_deinitSemaphores() != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/************************************************************************
|function implementation (scope: global)
|-----------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/** @} */

/* End of File */
