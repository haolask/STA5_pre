//!
//!  \file 		etaltml_init.c
//!  \brief 	<i><b> ETALTML API layer </b></i>
//!  \details   Interface functions to initialize ETALTML
//!  \author 	Jean-Hugues Perrin
//!
#include "osal.h"
#include "etalinternal.h"
#include "service_following_task.h"


#if defined (CONFIG_ETAL_HAVE_ETALTML)


#if defined (CONFIG_ETALTML_HAVE_LEARN)
OSAL_tSemHandle etalLearnSem;
#endif

#if defined (CONFIG_ETALTML_HAVE_SCAN)
OSAL_tSemHandle etalScanSem;
#endif

/***************************
 *
 * ETALTML_init
 *
 **************************/
/*
 * Returns:
 *
 * OSAL_ERROR
 * Semaphore creation error
 *
 * OSAL_ERROR_DEVICE_INIT
 *  Memory allocation/thread allocation error
 *
 * OSAL_ERROR_DEVICE_NOT_OPEN
 *  Timeout while trying to communicate with ProtocolLayer
 *
 * OSAL_OK
 */
tSInt ETALTML_init(const EtalHardwareAttr *init_params, tBool power_up)
{
    tSInt retval = OSAL_OK;
#ifdef	CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
	retval = ETALTML_ServiceFollowing_task_init();
#endif 	


#ifdef CONFIG_ETALTML_HAVE_LEARN
    if (OSAL_s32SemaphoreCreate(ETAL_SEM_LEARN, &etalLearnSem, 1) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    retval |= ETALTML_learnTaskInit();
#endif

#ifdef CONFIG_ETALTML_HAVE_SCAN
    if (OSAL_s32SemaphoreCreate(ETAL_SEM_SCAN, &etalScanSem, 1) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    retval |= ETALTML_scanTaskInit();
#endif

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
	retval |= ETALTML_RDS_Strategy_InitLock();

	// init the task
	retval |= ETALTML_RdsSeekTaskInit();
#endif
	// init variables 
	ETALTML_PathAllocationInit();

    return retval;
}

/***************************
 *
 * ETALTML_deinit
 *
 **************************/
tSInt ETALTML_deinit(tBool power_up)
{
    tSInt retosal = OSAL_OK;
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
	tSInt retosal2 = OSAL_OK;
#endif

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
	tSInt retosal3 = OSAL_OK;
	tSInt retosal4 = OSAL_OK;

#endif


#ifdef CONFIG_ETALTML_HAVE_LEARN
	if (ETALTML_learnTaskDeinit() == OSAL_OK)
	{
		if (OSAL_s32SemaphoreClose(etalLearnSem) != OSAL_OK)
		{
			retosal = OSAL_ERROR;
		}
		else if (OSAL_s32SemaphoreDelete(ETAL_SEM_LEARN) != OSAL_OK)
		{
			retosal = OSAL_ERROR;
		}
	}
	else
	{
		/* don't destroy the semaphore if the task is not stopped */
		retosal = OSAL_ERROR;
	}
#endif

#ifdef CONFIG_ETALTML_HAVE_SCAN
    if (ETALTML_scanTaskDeinit() == OSAL_OK)
    {
        if (OSAL_s32SemaphoreClose(etalScanSem) != OSAL_OK)
        {
            retosal = OSAL_ERROR;
        }
        else if (OSAL_s32SemaphoreDelete(ETAL_SEM_SCAN) != OSAL_OK)
        {
            retosal = OSAL_ERROR;
        }
    }
    else
    {
        /* don't destroy the semaphore if the task is not stopped */
        retosal = OSAL_ERROR;
    }
#endif

#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
	retosal2 = ETALTML_ServiceFollowing_task_deinit();
	if (retosal2 != OSAL_OK)
	{
		/* don't overwrite ETALTML_externalSeekTaskDeinit errors */
		retosal = retosal2;
	}

#endif 	

#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
	retosal3 = ETALTML_RdsSeekTaskDeinit();
	if (retosal3 != OSAL_OK)
	{
		/* don't overwrite ETALTML_externalSeekTaskDeinit errors */
		retosal = retosal3;
	}

	retosal4 = ETALTML_RDS_Strategy_DeinitLock();

	if (retosal4 != OSAL_OK)
	{
		/* don't overwrite ETALTML_externalSeekTaskDeinit errors */
		retosal = retosal4;
	}

#endif

    return retosal;
}

#endif // CONFIG_ETAL_HAVE_ETALTML
