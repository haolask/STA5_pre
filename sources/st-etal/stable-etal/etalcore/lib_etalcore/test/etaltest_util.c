//!
//!  \file 		etaltest_util.c
//!  \brief 	<i><b> ETAL test, receiver configuration </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST

#include "etal_api.h"
#include "etaltest.h"

/***************************
 *
 * etalTestDoConfigSingle
 *
 **************************/
tSInt etalTestDoConfigSingle(etalTestConfigTy conf, ETAL_HANDLE *handle)
{
	EtalReceiverAttr attr;
	ETAL_STATUS ret;
	tBool do_config = FALSE;
	tSInt retval = OSAL_OK;
	EtalAudioInterfTy audioIf;

	if (handle != NULL)
	{
		*handle = ETAL_INVALID_HANDLE;
		OSAL_pvMemorySet(&attr, 0x00, sizeof(EtalReceiverAttr));
		switch (conf)
		{
			case ETAL_CONFIG_NONE:
				break;
			case ETAL_CONFIG_DAB:
#ifdef CONFIG_APP_TEST_DAB
				attr.m_Standard = ETAL_BCAST_STD_DAB;
				attr.m_FrontEndsSize = 1;
				attr.m_FrontEnds[0] = ETAL_FE_FOR_DAB_TEST;
				do_config = TRUE;
#endif //CONFIG_APP_TEST_DAB
				break;
			case ETAL_CONFIG_DAB1_5:
#ifdef CONFIG_APP_TEST_DAB
				attr.m_Standard = ETAL_BCAST_STD_DAB;
				attr.m_FrontEndsSize = 1;
				attr.m_FrontEnds[0] = ETAL_FE_FOR_DAB_TEST1_5;
				do_config = TRUE;
#endif //CONFIG_APP_TEST_DAB
				break;
			case ETAL_CONFIG_FM1:
			case ETAL_CONFIG_FM2:
#ifdef CONFIG_APP_TEST_FM
				attr.m_Standard = ETAL_BCAST_STD_FM;
				attr.m_FrontEndsSize = 1;
				if (conf == ETAL_CONFIG_FM1)
				{
					attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
				}
				else
				{
					attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST2;
				}
				do_config = TRUE;
#endif //CONFIG_APP_TEST_FM
				break;
			case ETAL_CONFIG_FM1_FM2_VPA:
#ifdef CONFIG_APP_TEST_FM
				attr.m_Standard = ETAL_BCAST_STD_FM;
				attr.m_FrontEndsSize = 2;
				attr.m_FrontEnds[0] = ETAL_FE_FOR_FM_TEST;
				attr.m_FrontEnds[1] = ETAL_FE_FOR_FM_TEST2;
				attr.processingFeatures.u.m_processing_features = ETAL_PROCESSING_FEATURE_FM_VPA;
				do_config = TRUE;
#endif //CONFIG_APP_TEST_FM
				break;
			case ETAL_CONFIG_AM:
#ifdef CONFIG_APP_TEST_AM
				attr.m_Standard = ETAL_BCAST_STD_AM;
				attr.m_FrontEndsSize = 1;
				attr.m_FrontEnds[0] = ETAL_FE_FOR_AM_TEST;
				do_config = TRUE;
#endif //ETAL_CONFIG_AM
				break;
			case ETAL_CONFIG_HDRADIO_FM:
#ifdef CONFIG_APP_TEST_HDRADIO_FM
				attr.m_Standard = ETAL_BCAST_STD_HD_FM;
				attr.m_FrontEndsSize = 1;
				attr.m_FrontEnds[0] = ETAL_FE_FOR_HD_TEST;
				do_config = TRUE;
#endif //CONFIG_APP_TEST_HDRADIO_FM
				break;
			case ETAL_CONFIG_HDRADIO_AM:
#ifdef CONFIG_APP_TEST_HDRADIO_AM
				attr.m_Standard = ETAL_BCAST_STD_HD_AM;
				attr.m_FrontEndsSize = 1;
				attr.m_FrontEnds[0] = ETAL_FE_FOR_HD_TEST;
				do_config = TRUE;
#endif //CONFIG_APP_TEST_HDRADIO_AM
				break;
			default:
				ASSERT_ON_DEBUGGING(0);
				break;
		}
		if (do_config)
		{
			// set the correct audio path 
			/* Configure audio path */
			memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
			audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
#ifdef CONFIG_DIGITAL_AUDIO
			if ((conf == ETAL_CONFIG_DAB) || (conf == ETAL_CONFIG_DAB1_5) || (conf == ETAL_CONFIG_FM1) ||
				(conf == ETAL_CONFIG_FM2) || (conf == ETAL_CONFIG_FM1_FM2_VPA) || (conf == ETAL_CONFIG_AM))
			{
				audioIf.m_dac = 0;
			}
			else
			{
				system("amixer -c 3 sset Source adcauxdac > /dev/null" );

				// select the audio channel
				system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
			}
#endif
			// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
			audioIf.m_sai_slave_mode = TRUE;
#endif //CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
			if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
				return OSAL_ERROR;
			}

			etalTestPrintNormal("* Create %s receiver", etalTestStandard2Ascii(attr.m_Standard));
			if ((ret = etal_config_receiver(handle, &attr)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
				retval = OSAL_ERROR;
			}
			else
			{
				etalTestPrintNormal("* Created %s receiver, handle %d", etalTestStandard2Ascii(attr.m_Standard), *handle);
			}
		}
	}
	return retval;
}

/***************************
 *
 * etalTestUndoConfigSingle
 *
 **************************/
tSInt etalTestUndoConfigSingle(ETAL_HANDLE *handle)
{
	ETAL_STATUS ret;

	if ((handle != NULL) && (*handle != ETAL_INVALID_HANDLE))
	{
		etalTestPrintNormal("* Destroy the receiver");
		if ((ret = etal_destroy_receiver(handle)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_destroy_receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}
	return OSAL_OK;
}

#endif // CONFIG_APP_ETAL_TEST

