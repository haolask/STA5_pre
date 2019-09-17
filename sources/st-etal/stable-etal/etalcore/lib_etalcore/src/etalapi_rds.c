//!
//!  \file 		etalapi_rds.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application, RDS
//!  \author 	Raffaele Belardi
//!

#include "osal.h"
#include "etal_api.h"
#include "etaltml_api.h"
#include "etalinternal.h"

#if defined(CONFIG_ETAL_SUPPORT_CMOST_STAR)
	#include "tunerdriver_internal.h" // for CMOST_MAX_RESPONSE_LEN
#endif

/***************************
 *
 * Variables
 *
 **************************/


/******************************************************************************
 * Local functions
 *****************************************************************************/

#if defined(CONFIG_ETAL_SUPPORT_CMOST_STAR)
/***************************
 *
 * ETAL_RDSRawPeriodicFunc
 *
 **************************/
tVoid ETAL_RDSRawPeriodicFunc(ETAL_HANDLE hGeneric)
{
	ETAL_HANDLE hReceiver;
	ETAL_HANDLE hDatapath;
	tU8 buf[CMOST_MAX_RESPONSE_LEN];
	tU32 len;
	tU32 data_rnr;
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
	ETAL_HANDLE tunerId;
#else
	ETAL_STATUS ret;
	etalRDSAttrInternal *rds_attr_int;
#endif

// In IRQ case we accept a tuner in TKERNEL CASE
#if (!(defined(CONFIG_HOST_OS_TKERNEL) && defined(CONFIG_COMM_ENABLE_RDS_IRQ)))
	if (!ETAL_handleIsReceiver(hGeneric))
	{
		ASSERT_ON_DEBUGGING(0);
		return;
	}
	hReceiver = hGeneric;

	/* get receiver lock */
	if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
	{
		return;
	}

	//check receiver is still valid and has not been destroyed !
	if (false == ETAL_receiverIsValidHandle(hReceiver))
	{
		ETAL_receiverReleaseLock(hReceiver);
		return;
	}

	/* is hReceiver valid RDS handle */
	if (ETAL_receiverIsValidRDSHandle(hReceiver) == FALSE)
	{
		ETAL_receiverReleaseLock(hReceiver);
		return;
	}

	/* read CMOST RDS raw block */
	if (ETAL_cmdGetRDS_CMOST(hReceiver, buf, &len) != OSAL_OK)
	{
		ETAL_receiverReleaseLock(hReceiver);
		return;
	}
#else // TKERNEL & IRQ
	tunerId = hGeneric;

	/* read CMOST RDS raw block */
	if (ETAL_cmdGetRDS_CMOST(hGeneric, buf, &len) != OSAL_OK)
	{
		return;
	}
#endif

	if (len > CMOST_MAX_RESPONSE_LEN)
	{
		ASSERT_ON_DEBUGGING(0);
#if (!(defined(CONFIG_HOST_OS_TKERNEL) && defined(CONFIG_COMM_ENABLE_RDS_IRQ)))
		ETAL_receiverReleaseLock(hReceiver);
#endif
		return;
	}

	if (len >= 6)
	{
		ETAL_getParameter_CMOST(buf, &data_rnr);

//		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL_RDSRawPeriodicFunc: data_rnr 0x%x", data_rnr);

#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
		/* find hReceiver from channel */
		if (data_rnr & STAR_RNR_TUNCH1)
		{
#ifdef CONFIG_HOST_OS_TKERNEL
			if (ETAL_statusGetRDSIrqhReceiver(tunerId, ETAL_CHN_FOREGROUND, &hReceiver) != TRUE)
			{			
				return;
			}
			else
			{
				// lock the receiver

				/* get receiver lock */
				if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
				{
					return;
				}
				
				//check receiver is still valid and has not been destroyed !
				if (false == ETAL_receiverIsValidHandle(hReceiver))
				{
					ETAL_receiverReleaseLock(hReceiver);
					return;
				}
			}
#else
			/* find hReceiver from channel ETAL_CHN_FOREGROUND */
			if ((ETAL_receiverGetTunerId(hReceiver, &tunerId) != OSAL_OK) ||
				(ETAL_statusGetRDSIrqhReceiver(tunerId, ETAL_CHN_FOREGROUND, &hReceiver) != TRUE))

			{
				ETAL_receiverReleaseLock(hReceiver);
				return;
			}
#endif

		}
		else if (data_rnr & STAR_RNR_TUNCH2)
		{
			/* find hReceiver from channel ETAL_CHN_BACKGROUND */
#ifdef CONFIG_HOST_OS_TKERNEL
			if (ETAL_statusGetRDSIrqhReceiver(tunerId, ETAL_CHN_BACKGROUND, &hReceiver) != TRUE)	
			{			
				return;
			}
			else
			{
				// lock the receiver

				/* get receiver lock */
				if (ETAL_receiverGetLock(hReceiver) != ETAL_RET_SUCCESS)
				{
					return;
				}

				//check receiver is still valid and has not been destroyed !
				if (false == ETAL_receiverIsValidHandle(hReceiver))
				{
					ETAL_receiverReleaseLock(hReceiver);
					return;
				}
			}
#else

			if ((ETAL_receiverGetTunerId(hReceiver, &tunerId) != OSAL_OK) ||
				(ETAL_statusGetRDSIrqhReceiver(tunerId, ETAL_CHN_BACKGROUND, &hReceiver) != TRUE))
			{
				ETAL_receiverReleaseLock(hReceiver);
				return;
			}
#endif

		}
		else
		{
#ifdef CONFIG_HOST_OS_TKERNEL
			ETAL_receiverReleaseLock(hReceiver);
#endif
			return;	
		}
#endif

		if (len >= 9)
		{
			/* send RDS raw block to upper layer */
			if ((hDatapath = ETAL_receiverGetDatapathFromDataType(hReceiver, ETAL_DATA_TYPE_FM_RDS_RAW)) != ETAL_INVALID_HANDLE)
			{
				/* len - 3 to remove the 3 bytes CRC  that should not be present in the datapath */
				(void)ETAL_datahandlerInvoke(ETAL_COMM_DATA_CALLBACK_HANDLER, hDatapath, (tVoid *)buf, (len - 3), NULL);
			}
#if defined(CONFIG_ETAL_HAVE_ETALTML) && defined(CONFIG_ETALTML_HAVE_RADIOTEXT)
			ETALTML_RDScheckDecoded(hReceiver, buf, len);
#endif
		}

#ifndef CONFIG_COMM_ENABLE_RDS_IRQ
		if (ETAL_receiverGetRDSAttrInt(hReceiver, &rds_attr_int) == ETAL_RET_SUCCESS)
		{
			/* if fast PI auto mode and DATARDY */
			if ((rds_attr_int->rdsAttr.rdsMode != ETAL_RDS_MODE_PERMANENT_FAST_PI) && (rds_attr_int->rdsAttr.nbRdsBlockforInteruptFastPI != (tU8)0))
			{
				rds_attr_int->numPICnt += (tU8)((len - 6) / 3);
				if (((data_rnr & STAR_RNR_DATARDY) != 0) || (rds_attr_int->numPICnt >= rds_attr_int->rdsAttr.nbRdsBlockforInteruptFastPI))
				{
					rds_attr_int->rdsAttr.nbRdsBlockforInteruptFastPI = (tU8)0;
					rds_attr_int->rdsAttr.rdsMode = ETAL_RDS_MODE_NORMAL;
					/* deregister periodic function if already registered */
					if (ETAL_intCbIsRegisteredPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver) == TRUE)
					{
						ret = ETAL_intCbDeregisterPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver);
						if (ret != ETAL_RET_SUCCESS)
						{
							ETAL_receiverReleaseLock(hReceiver);
							return;
						}
					}

					/* register periodic function */
					/* normal mode */
					ret = ETAL_intCbRegisterPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver, ETAL_RDS_INTCB_DELAY);
					if (ret != ETAL_RET_SUCCESS)
					{
						ETAL_receiverReleaseLock(hReceiver);
						return;
					}
				}
			}
		}
#endif // !CONFIG_COMM_ENABLE_RDS_IRQ
	}
	ETAL_receiverReleaseLock(hReceiver);
}


/***************************
 *
 * ETAL_start_RDS
 *
 **************************/
ETAL_STATUS ETAL_start_RDS(ETAL_HANDLE hReceiver, etalRDSAttr *pRDSAttr)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (pRDSAttr != NULL)
	{
		if (((pRDSAttr->rdsMode == ETAL_RDS_MODE_NORMAL) && (pRDSAttr->nbRdsBlockforInteruptFastPI != (tU8)0)) ||
			((pRDSAttr->rdsMode != ETAL_RDS_MODE_NORMAL) && (pRDSAttr->nbRdsBlockforInteruptFastPI == (tU8)0)))
		{
			//ASSERT_ON_DEBUGGING(0);
			return ETAL_RET_PARAMETER_ERR;
		}

		// TMP processing for Fast PI :
		// the number of block to trigger interupt for fast PI is coded on 4 bits. 
		// so MAX is 0x0F ie 15. for fast PI case.
		if ((pRDSAttr->nbRdsBlockforInteruptFastPI > (tU8)0x0F) || 
			((pRDSAttr->nbRdsBlockforInteruptNormalMode == (tU8)0) || (pRDSAttr->nbRdsBlockforInteruptNormalMode > (tU8)ETAL_RDS_MAX_BLOCK_IN_BUFFER)))
		{
			return ETAL_RET_PARAMETER_ERR;
		}
		if ((pRDSAttr->rdsRbdsMode != ETAL_RDS_MODE) && (pRDSAttr->rdsRbdsMode != ETAL_RBDS_MODE))
		{
			return ETAL_RET_PARAMETER_ERR;
		}
	}

	/* configure RDS mode */
	if (ETAL_RDSreset_CMOST(hReceiver, pRDSAttr) != OSAL_OK)
	{
		return ETAL_RET_ERROR;
	}

	/* start rds periodic call */
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
	if (ETAL_statusStartRDSIrq(hReceiver) != OSAL_OK)
	{
		return ETAL_RET_ERROR;
	}
#else
	if ((pRDSAttr != NULL) && ((ret = ETAL_receiverSetRDSAttr(hReceiver, pRDSAttr)) != ETAL_RET_SUCCESS))
	{
		return ret;
	}

	/* deregister periodic function if already registered */
	if (ETAL_intCbIsRegisteredPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver) == TRUE)
	{
		ret = ETAL_intCbDeregisterPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver);
		if (ret != ETAL_RET_SUCCESS)
		{
			return ret;
		}
	}

	/* register periodic function */
	if ((pRDSAttr != NULL) && (pRDSAttr->rdsMode != ETAL_RDS_MODE_NORMAL) && (pRDSAttr->nbRdsBlockforInteruptFastPI != (tU8)0))
	{
		/* fast PI mode */
		ret = ETAL_intCbRegisterPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver, ETAL_RDS_FAST_PI_INTCB_DELAY);
	}
	else
	{
		/* normal mode */
		ret = ETAL_intCbRegisterPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver, ETAL_RDS_INTCB_DELAY);
	}
	
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
	if (ret == ETAL_RET_SUCCESS)		//inform RDS strategy RDS decoding is on
	{
		ETALTML_RDS_Strategy_RDSEnable(hReceiver);
	}
#endif	
#endif

	ETAL_receiverSetSpecial(hReceiver, cmdSpecialRDS, cmdActionStart);

	return ret;
}

/***************************
 *
 * ETAL_stop_RDS
 *
 **************************/
ETAL_STATUS ETAL_stop_RDS(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

    /* stop rds periodic call */
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
    ETAL_statusStopRDSIrq(hReceiver);
#else
    if (ETAL_intCbIsRegisteredPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver) == TRUE)
    {
        ret = ETAL_intCbDeregisterPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver);
    }
#endif
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY
    if (ret == ETAL_RET_SUCCESS)		//inform RDS strategy RDS decoding is on
    {
	  ETALTML_RDS_Strategy_RDSDisable(hReceiver);
    }
#endif	

    /* configure RDS mode */
    if (ETAL_RDSreset_CMOST(hReceiver, (etalRDSAttr *)NULL) != OSAL_OK)
    {
        ret = ETAL_RET_ERROR;
    }

	ETAL_receiverSetSpecial(hReceiver, cmdSpecialRDS, cmdActionStop);

    return ret;
}

#if defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
/***************************
 *
 * ETAL_suspend_RDS
 *
 **************************/
ETAL_STATUS ETAL_suspend_RDS(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;


	// If nothing on-going on this receiver, just return
	//
	
	if (false == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialRDS))
	{
		return ETAL_RET_ERROR;
	}
 
    /* start rds periodic call */
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
    ETAL_statusStopRDSIrq(hReceiver);
#else
  
    /* deregister periodic function if already registered */
    if (ETAL_intCbIsRegisteredPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver) == TRUE)
    {
        ret = ETAL_intCbDeregisterPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver);
    }
 
#endif


    return ret;
}

/***************************
 *
 * ETAL_resume_RDS
 *
 **************************/
ETAL_STATUS ETAL_resume_RDS(ETAL_HANDLE hReceiver)
{
	/* start rds periodic call */
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
    ETAL_STATUS ret = ETAL_RET_SUCCESS;

    // If nothing on-going on this receiver, just return

    if (false == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialRDS))
    {
        return ETAL_RET_ERROR;
    }
    if (ETAL_statusStartRDSIrq(hReceiver) != OSAL_OK)
	{
		ret = ETAL_RET_ERROR;
	}
#else
    ETAL_STATUS ret = ETAL_RET_SUCCESS;
    etalRDSAttr *RDSAttrInt;

    // If nothing on-going on this receiver, just return

    if (false == ETAL_receiverIsSpecialInProgress(hReceiver, cmdSpecialRDS))
    {
        return ETAL_RET_ERROR;
    }

	if (ETAL_RET_SUCCESS != ETAL_receiverGetRDSAttr(hReceiver, &RDSAttrInt))
	{
		return(ETAL_RET_ERROR);
	}

	/* deregister periodic function if already registered */
	if (ETAL_intCbIsRegisteredPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver) == TRUE)
	{
		ret = ETAL_intCbDeregisterPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver);
		if (ret != ETAL_RET_SUCCESS)
		{
			return ret;
		}
	}

	// force a decoding directly now to save time to read RDS buffers
	// We cannot directly decode the RDS : 
	// the periodic functiion locks the receiver !!
	// it may lead to double lock !!
	// because the receiver may be locked by the caller
	//
	//ETAL_RDSRawPeriodicFunc(hReceiver);
	
	/* register periodic function */
	if ((RDSAttrInt != NULL) && (RDSAttrInt->rdsMode != ETAL_RDS_MODE_NORMAL))
	{
		/* fast PI mode */
		ret = ETAL_intCbRegisterPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver, ETAL_RDS_FAST_PI_INTCB_DELAY);
	}
	else
	{
		/* normal mode */
		ret = ETAL_intCbRegisterPeriodic(ETAL_RDSRawPeriodicFunc, hReceiver, ETAL_RDS_INTCB_DELAY);
	}
#endif
	
	return ret;

}
#endif // CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR

/******************************************************************************
 * Exported functions
 *****************************************************************************/

/***************************
 *
 * etal_start_RDS
 *
 **************************/
ETAL_STATUS etal_start_RDS(ETAL_HANDLE hReceiver, tBool forceFastPi, tU8 numPi, EtalRDSRBDSModeTy mode)
{
	ETAL_STATUS ret;
#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
	etalRDSAttr RDSAttr;
#endif

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_start_RDS(rec: %d, forFasPi: %d, numPi: %d, mode: %d)", hReceiver, forceFastPi, numPi, mode);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		if (!ETAL_handleIsValid(hReceiver))
		{
			ret = ETAL_RET_INVALID_HANDLE;
		}
		else if (!ETAL_receiverIsValidHandle(hReceiver) ||
			!ETAL_receiverIsValidRDSHandle(hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else
		{
			ret = ETAL_receiverGetLock(hReceiver);
			if (ret == ETAL_RET_SUCCESS)
			{
				/* start RDS */
				(void)OSAL_pvMemorySet((tVoid *)&RDSAttr, 0, sizeof(etalRDSAttr));
				if (forceFastPi ==  TRUE)
				{
					RDSAttr.rdsMode = ETAL_RDS_MODE_PERMANENT_FAST_PI;
				}
				else if (numPi != (tU8)0)
				{
					RDSAttr.rdsMode = ETAL_RDS_MODE_TEMPORARY_FAST_PI;
				}
				else
				{
					RDSAttr.rdsMode = ETAL_RDS_MODE_NORMAL;
				}
				RDSAttr.nbRdsBlockforInteruptFastPI = numPi;
				RDSAttr.nbRdsBlockforInteruptNormalMode = (tU8)16;
				RDSAttr.rdsRbdsMode = mode;
				ret = ETAL_start_RDS(hReceiver, &RDSAttr);
				ETAL_receiverReleaseLock(hReceiver);
			}
		}

		ETAL_statusReleaseLock();
	}
#else
	ret =  ETAL_RET_NOT_IMPLEMENTED;
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR
	ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_start_RDS() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_stop_RDS
 *
 **************************/
ETAL_STATUS etal_stop_RDS(ETAL_HANDLE hReceiver)
{
	ETAL_STATUS ret;

#ifdef CONFIG_ETAL_SUPPORT_CMOST_STAR
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_stop_RDS(rec: %d)", hReceiver);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		if (!ETAL_handleIsValid(hReceiver))
		{
			ret = ETAL_RET_INVALID_HANDLE;
		}
		else if (!ETAL_receiverIsValidHandle(hReceiver) ||
			!ETAL_receiverIsValidRDSHandle(hReceiver))
		{
			ret = ETAL_RET_INVALID_RECEIVER;
		}
		else
		{
			ret = ETAL_receiverGetLock(hReceiver); 
			if (ret == ETAL_RET_SUCCESS)
			{
				ret = ETAL_stop_RDS(hReceiver);
				ETAL_receiverReleaseLock(hReceiver);
			}
		}

		ETAL_statusReleaseLock();
	}
#else
	ret = ETAL_RET_NOT_IMPLEMENTED;
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_stop_RDS() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

