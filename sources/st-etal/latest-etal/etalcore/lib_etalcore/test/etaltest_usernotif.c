//!
//!  \file 		etaltest_usernotif.c
//!  \brief 	<i><b> ETAL test, user notification handler </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST

#include "etal_api.h"
#include "etaltml_api.h"

#include "etaltest.h"

/***************************
 *
 * userNotificationHandler
 *
 **************************/
tVoid userNotificationHandler(void *context, ETAL_EVENTS dwEvent, void *pvParam)
{
#if defined (CONFIG_APP_TEST_SEEK) || defined (CONFIG_APP_TEST_SCAN) || defined (CONFIG_APP_TEST_LEARN)
	etalTestEventCountTy *count;
#endif

	if (dwEvent == ETAL_ERROR_COMM_FAILED)
	{
		if (!etalTestServeCallbacks(ETAL_CALLBACK_REASON_IS_COMM_ERROR, pvParam))
		{
			EtalCommErrStatus *comm_err;

			/* provide a default handler if none is registered */
			comm_err = (EtalCommErrStatus *)pvParam;
			etalTestPrintError("Communication error code %d, receiver %d", comm_err->m_commErr, comm_err->m_commErrReceiver);
		}
	}
#ifdef CONFIG_APP_TEST_RECEIVER_ALIVE
	else if (dwEvent == ETAL_RECEIVER_ALIVE_ERROR)
	{
		ETAL_HANDLE hReceiver = *((ETAL_HANDLE *)pvParam);

		if (hReceiver == etalTestReceiverAliveErrorHandleExpected)
		{
			etalTestReceiverAliveError = TRUE;
			/* the semaphore is created only during the Receiver Alive test
			 * so if the ping fails during a test sequence not including
			 * that test we would hit an ASSERT here */
			if (etalTestAliveErrSem != 0)
			{
				OSAL_s32SemaphorePost(etalTestAliveErrSem);
			}
		}
	}
#endif
	else if (pvParam != NULL)
	{
		if (dwEvent == ETAL_INFO_TUNE)
		{
#if defined(CONFIG_APP_TEST_HDRADIO_FM) || defined(CONFIG_APP_TEST_HDRADIO_AM)
			EtalTuneStatus *status = (EtalTuneStatus *)pvParam;

			if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_SYNC)
			{
				if (status->m_receiverHandle == handlehd)
				{
#if defined (CONFIG_APP_TEST_HDRADIO_FM)
					etalTestHDTuneEventCount++;
#endif // defined (CONFIG_APP_TEST_HDRADIO_FM)
				}
				else if (status->m_receiverHandle == handlehdam)
				{
#if defined (CONFIG_APP_TEST_HDRADIO_AM)
					etalTestHDAMTuneEventCount++;
#endif // defined (CONFIG_APP_TEST_HDRADIO_AM)
				}
			}
			if ((status->m_sync & ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC) == ETAL_TUNESTATUS_SYNCMASK_HD_AUDIO_SYNC)
			{
				if (status->m_receiverHandle == handlehd)
				{
					etalTestPrintNormal("HD FM digital audio acquired");
				}
				else if (status->m_receiverHandle == handlehdam)
				{
					etalTestPrintNormal("HD AM digital audio acquired");
				}
			}
#endif //defined(CONFIG_APP_TEST_HDRADIO_FM) || defined(CONFIG_APP_TEST_HDRADIO_AM)
		}
#if defined (CONFIG_APP_TEST_SEEK) || defined (CONFIG_APP_TEST_GETRDS_DATAPATH) ||  defined (CONFIG_APP_TEST_RDS_SEEK)
		else if (dwEvent == ETAL_INFO_SEEK)
		{
			EtalSeekStatus *status = (EtalSeekStatus *)pvParam;

#ifdef CONFIG_APP_TEST_SEEK
			if (status->m_receiverHandle == handledab)
			{
				count = &etalTestSeekEventCountDAB;
			}
			else if (status->m_receiverHandle == handledab1_5)
			{
				count = &etalTestSeekEventCountDAB1_5;
			}
			else if (status->m_receiverHandle == handlefm)
			{
				count = &etalTestSeekEventCountFM;
			}
			else if (status->m_receiverHandle == handleam)
			{
				count = &etalTestSeekEventCountAM;
			}
			else if (status->m_receiverHandle == handlehd)
			{
				count = &etalTestSeekEventCountHD;
			}

			switch(status->m_status)
			{
				case ETAL_SEEK_STARTED:
					count->STARTED++;
					break;
				case ETAL_SEEK_RESULT:
					count->RESULT++;
					break;
				case ETAL_SEEK_FINISHED:
					count->FINISHED++;
					break;
				case ETAL_SEEK_ERROR:
					count->NB_ERROR++;
					break;
				default:
					break;
			}

			if (status->m_frequencyFound == TRUE)
			{
				// freq found is a bool, cannot be a counter
				// else if Result / Finished followed without time to process
				// Freq_Found will be > 1
				count->FREQ_FOUND = TRUE;
			}
			
			if (status->m_fullCycleReached == TRUE)
			{
				// FULL_CYCLE_REACHED is a bool, cannot be a counter
				// else if Result / Finished followed without time to process
				// Freq_Found will be > 1
				
				count->FULL_CYCLE_REACHED = TRUE;
			}

#endif // CONFIG_APP_TEST_SEEK
#if defined (CONFIG_APP_TEST_GETRDS_DATAPATH) || defined (CONFIG_APP_TEST_RDS_SEEK)
			if(status->m_status == ETAL_SEEK_FINISHED)
			{
#ifdef CONFIG_APP_TEST_GETRDS_DATAPATH
				if (status->m_frequencyFound == TRUE)
				{
					etalTestRDSSeekFrequencyFound = status->m_frequency;
				}
				/*
				 * This semaphore is normally created in etalTestGetRDSDatapathChangeFreq
				 * but if that function aborts for some error during the test execution
				 * (e.g. "AFList not received") the semaphore is not created and
				 * the OSAL_s32SemaphorePost hits an ASSERT_ON_DEBUGGING.
				 * So we have to check if the semaphore was created before posting it.
				 */
				if (etalTestRDSSeekFinishSem)
				{
					OSAL_s32SemaphorePost(etalTestRDSSeekFinishSem);
				}
#endif

#ifdef  CONFIG_APP_TEST_RDS_SEEK
				etalTestRDSSeek_SetSeekResult(status);
				if (etalTestRDSSeekSem)
				{
					OSAL_s32SemaphorePost(etalTestRDSSeekSem);
				}
#endif
			}
#ifdef  CONFIG_APP_TEST_RDS_SEEK
			else if(status->m_status == ETAL_SEEK_RESULT)
			{
				if (status->m_frequencyFound)
				{
					etalTestRDSSeek_SetSeekResult(status);
					if (etalTestRDSSeekSem)
					{
						OSAL_s32SemaphorePost(etalTestRDSSeekSem);
					}
				}
			}
#endif

#endif // CONFIG_APP_TEST_GETRDS_DATAPATH || CONFIG_APP_TEST_RDS_SEEK
		}
#endif // CONFIG_APP_TEST_SEEK || CONFIG_APP_TEST_GETRDS_DATAPATH
#ifdef CONFIG_APP_TEST_SCAN
		else if (dwEvent == ETAL_INFO_SCAN)
		{
			EtalScanStatusTy *status = (EtalScanStatusTy *)pvParam;

			if (status->m_receiverHandle == handlehd)
			{
				count = &etalTestScanCountHD;
			}
			else if (status->m_receiverHandle == handlefm)
			{
				count = &etalTestScanCountFM;
			}
			else if (status->m_receiverHandle == handleam)
			{
				count = &etalTestScanCountAM;
			}

			switch(status->m_status)
			{
				case ETAL_SCAN_STARTED:
					count->STARTED++;
					break;
				case ETAL_SCAN_RESULT:
					count->RESULT++;
					break;
				case ETAL_SCAN_FINISHED:
					count->FINISHED++;
					break;
				case ETAL_SCAN_ERROR:
					count->NB_ERROR++;
					break;
				default:
					break;
			}

			if (status->m_status == ETAL_SCAN_FINISHED)
			{
                if (status->m_receiverHandle != handledab)
                {
                    // Post the information of scan completed
                    // But, only if the semaphore is awaited : i.e. if the scan has not been interrupted due to a timeout
                    //
                    if (true == vl_etalTestScan_ScanOnGoing)
                    {
                        OSAL_s32SemaphorePost(etalTestScanSem);
                    }
                }
			}
		}
#endif // CONFIG_APP_TEST_SCAN
#ifdef CONFIG_APP_TEST_LEARN
		else if (dwEvent == ETAL_INFO_LEARN)
		{
			EtalLearnStatusTy *status = (EtalLearnStatusTy *)pvParam;

			if (status->m_receiverHandle == handledab)
			{
				count = &etalTestLearnCountDAB;
			}
			else if (status->m_receiverHandle == handlefm)
			{
				count = &etalTestLearnCountFM;
			}
			else if (status->m_receiverHandle == handleam)
			{
				count = &etalTestLearnCountAM;
			}
			else if (status->m_receiverHandle == handlehd)
			{
				count = &etalTestLearnCountHD;
			}

			switch(status->m_status)
			{
				case ETAL_LEARN_STARTED:
					count->STARTED++;
					break;
				case ETAL_LEARN_RESULT:
					count->RESULT++;
					break;
				case ETAL_LEARN_FINISHED:
					count->FINISHED++;
					break;
				case ETAL_LEARN_ERROR:
					count->NB_ERROR++;
					break;
				default:
					break;
			}

			if (status->m_status == ETAL_LEARN_FINISHED)
			{
				count->aux = status->m_nbOfFrequency;
				if (status->m_receiverHandle != handledab)
				{
					// Post the information of learned completed
					// But, only if the semaphore is awaited : i.e. if the learn has not been interrupted due to a timeout
					//
					if (true == vl_etalTestLearn_LearnOnGoing)
					{
						OSAL_s32SemaphorePost(etalTestLearnSem);
					}
				}
			}
		}
#endif
#ifdef CONFIG_APP_TEST_SEAMLESS
		else if (dwEvent == ETAL_INFO_SEAMLESS_ESTIMATION_END)
		{
			EtalSeamlessEstimationStatus *seamless_estimation_status = (EtalSeamlessEstimationStatus *)pvParam;

			/* gives seamless estimation response to etaltest_seamless  */
			etalTestSeamlessEstimation_Resp(seamless_estimation_status);

			etalTestSeamlessEstimationCount++;
		}
		else if (dwEvent == ETAL_INFO_SEAMLESS_SWITCHING_END)
		{
			EtalSeamlessSwitchingStatus *seamless_switching_status = (EtalSeamlessSwitchingStatus *)pvParam;

			/* gives seamless switching response to etaltest_seamless  */
			etalTestSeamlessSwitching_Resp(seamless_switching_status);

			etalTestSeamlessSwitchingCount++;
		}
#endif // CONFIG_APP_TEST_SEAMLESS
#ifdef CONFIG_APP_TEST_AUDIO_FM_STEREO
		else if (dwEvent == ETAL_INFO_FM_STEREO)
		{
			EtalStereoStatus *stereoStatusp = (EtalStereoStatus *)pvParam;

			if ((etalTestInfoFmStereoReceiverExpected == stereoStatusp->m_hReceiver) && (etalTestInfoFmStereoExpected == stereoStatusp->m_isStereo))
			{
				etalTestInfoFmStereoCount++;
			}
		}
#endif // CONFIG_APP_TEST_AUDIO_FM_STEREO
	}
}
#endif // CONFIG_APP_ETAL_TEST

