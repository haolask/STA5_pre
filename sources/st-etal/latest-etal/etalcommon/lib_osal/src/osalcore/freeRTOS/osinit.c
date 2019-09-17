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
/************************************************************************
|function implementation (scope: module-local)
|-----------------------------------------------------------------------*/

/**
 * @brief     Process_0
 *
 * @details   This is the first thread of the first process. It calls
 *                OSAL_s32Boot function.
 *
 * @param     pvArg pointer to arguments
 *
 * @return    NULL
 *
 */
 /* Not needed
 */
 #if 0
tVoid Process_0(void)
{
#if 0
    if (OSAL_s32Boot(1,NULL)==OSAL_OK)
    {
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_COMPONENT, TR_CLASS_OSALCORE, "OSAL_s32Boot ended properly");
#endif
    }
    else
    {
#if defined(CONFIG_TRACE_CLASS_OSALCORE) && (CONFIG_TRACE_CLASS_OSALCORE >= TR_LEVEL_ERRORS)
        OSALUTIL_s32TracePrintf((OSAL_tIODescriptor)0, TR_LEVEL_ERRORS, TR_CLASS_OSALCORE, "OSAL_s32Boot returned an error");
#endif
    }
    return;
#endif
}
#endif
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
  // nothing to do for T-Kernel 
  // this is simplifyed

	OSAL_ClockResetTime();

#if defined (CONFIG_TRACE_ENABLE)
	OSALUTIL_vTraceInit();
#endif

    return OSAL_OK; /* lint */
}

tSInt OSAL_DEINIT(void)
{
#if defined (CONFIG_TRACE_ENABLE)
	OSALUTIL_vTraceDeInit();
#endif

#if defined (CONFIG_ETAL_HAVE_ETALTML)
	// deinit Event : not needed ?
	// 
#endif

	// deinit others ? 
	// a priori nothing needed.

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
