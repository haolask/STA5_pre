//!
//!  \file 		etaltest_special.c
//!  \brief 	<i><b> Special tests </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltml_api.h"

#include "etaltest.h"

#ifdef CONFIG_APP_TEST_SIGNALLING_GPIO
	#include "bsp_sta1095evb.h"
#endif
 
   
/*****************************************************************
| local macros
|----------------------------------------------------------------*/

/*****************************************************************
| local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable defintion (scope: module-local)
|----------------------------------------------------------------*/

/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/
#ifdef CONFIG_APP_TEST_STACK
static tVoid etalTestStack_ThreadEntry(void *dummy);
#endif

#ifdef CONFIG_APP_TEST_STACK
static tU8 another_big_array[ETAL_TEST_ARRAY_SIZE];
/***************************
 *
 * etalTestStack_ThreadEntry
 *
 **************************/
#ifdef CONFIG_HOST_OS_TKERNEL
static tVoid etalTestStack_ThreadEntry(tSInt stacd, tPVoid dummy)
#else
static tVoid etalTestStack_ThreadEntry(void *dummy)
#endif
{
	tU8 big_array[ETAL_TEST_ARRAY_SIZE];
	tU32 i;

	for (i = 0; i < ETAL_TEST_ARRAY_SIZE; i++)
	{
		big_array[i] = i & 0xFF;
	}
	for (i = 0; i < ETAL_TEST_ARRAY_SIZE; i++)
	{
		/*
		 * copy to a global to ensure the code is not optimized away
		 */
		another_big_array[i] = big_array[i];
	}
#if defined (CONFIG_TRACE_CLASS_ETAL)
	printf("Thread complete\n");
#endif
}

/***************************
 *
 * etalTestStack
 *
 **************************/
tVoid etalTestStack(void)
{
	OSAL_trThreadAttribute thread_attr;

	thread_attr.szName = (tChar *)"STACK_TEST";
	thread_attr.u32Priority = OSAL_C_U32_THREAD_PRIORITY_NORMAL;
	thread_attr.s32StackSize = etalTestOption.oStackSize;
	thread_attr.pfEntry = etalTestStack_ThreadEntry;
	thread_attr.pvArg = NULL;

#if defined (CONFIG_TRACE_CLASS_ETAL)
	printf("Stack test, using thread stack of %d\n", etalTestOption.oStackSize);
#endif
	if (OSAL_ThreadSpawn(&thread_attr) == OSAL_ERROR)
	{
#if defined (CONFIG_TRACE_CLASS_ETAL)
		etalTestPrintError("etalTestStack thread creation");
#endif
		return;
	}
	OSAL_s32ThreadWait(2 * ETAL_TEST_ONE_SECOND); // force reschedule
}
#endif // CONFIG_APP_TEST_STACK

#ifdef CONFIG_APP_TEST_STECI
/***************************
 *
 * etalTestSTECI
 *
 **************************/
tU32 etalTestSTECI(void)
{
	ETAL_STATUS ret;
	EtalLearnFrequencyTy freqList[10];

	etalTestPrintNormal("<---STECI stress test start");

	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return 1;
	}

	while (1)
	{
		// parameters not used but will not start otherwise
		if ((ret = etaltml_learn_start(handledab, ETAL_BAND_DAB3, 250 /*ETAL_SEEK_MAX_FM_STEP/2*/, 10, normalMode, freqList)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etaltml_learn_start (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return 1;
		}
		if ((ret = etaltml_learn_stop(handledab, initialFrequency)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etaltml_learn_stop (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return 1;
		}
	}
}
#endif // CONFIG_APP_TEST_STECI

#ifdef CONFIG_APP_TEST_SIGNALLING_GPIO
/***************************
 *
 * etalTestSignallingGPIO
 *
 **************************/
/*
 * - Set the signalling GPIO to a 0 value
 * - sleep 5sec
 * - set to 1
 * - sleep 5 sec
 * - toggle the GPIO on/off, 50% duty cycle, one cycle per 500ms, for 30s
 */
tVoid etalTestSignallingGPIO(void)
{
	tU32 cycle;

	etalTestPrintNormal("<---Signalling GPIO test start");
	BSP_setSignallingGPIO(0);
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	BSP_setSignallingGPIO(1);
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	for (cycle = 0; cycle < 60; cycle++)
	{
		BSP_setSignallingGPIO(0);
		OSAL_s32ThreadWait(250);
		BSP_setSignallingGPIO(1);
		OSAL_s32ThreadWait(250);
	}
}
#endif

#ifdef CONFIG_APP_TEST_TUNE_TIME_MEASURE
/***************************
 *
 * etalTestTuneTiming
 *
 **************************/
tVoid etalTestTuneTiming(void)
{
	ETAL_STATUS ret;

	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return;
	}
	etalTestPrintNormal("* Pass1: Tune to DAB empty freq %d (with band change, no tune to 0)", ETAL_EMPTY_DAB_FREQ);
	if ((ret = etal_tune_receiver(handledab, ETAL_EMPTY_DAB_FREQ)) != ETAL_RET_NO_DATA)
	{
		etalTestPrintError("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return;
	}
	etalTestPrintNormal("* Pass2: Tune again to empty DAB freq %d (no band change, tune to 0)", ETAL_EMPTY_DAB_FREQ);
	if ((ret = etal_tune_receiver(handledab, ETAL_EMPTY_DAB_FREQ)) != ETAL_RET_NO_DATA)
	{
		etalTestPrintError("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return;
	}
	etalTestPrintNormal("* Pass3: Tune to valid DAB freq %d (no band change, tune to 0)", ETAL_VALID_DAB_FREQ);
	if ((ret = etal_tune_receiver(handledab, ETAL_VALID_DAB_FREQ)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return;
	}
	etalTestPrintNormal("* Pass4: Tune to empty DAB freq %d (no band change, tune to 0)", ETAL_EMPTY_DAB_FREQ);
	if ((ret = etal_tune_receiver(handledab, ETAL_EMPTY_DAB_FREQ)) != ETAL_RET_NO_DATA)
	{
		etalTestPrintError("etal_tune_receiver DAB (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return;
	}
}
#endif // CONFIG_APP_TEST_TUNE_TIME_MEASURE
#endif // CONFIG_APP_ETAL_TEST

