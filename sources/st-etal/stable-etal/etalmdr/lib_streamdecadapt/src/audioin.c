//!
//!  \file audioin.c
//!  \brief <i><b>This file contains /dev/AudioIn device driver and uses the LLD_AudioIn routines</b></i>
//!  \author David Pastor
//!  \version 1.0
//!  \date 2017.10.02
//!  \bug Unknown
//!  \warning   -The execution with semihosting could be slowed down so to test the features
//!              disable the semihosting
//!             - Audio In not developed
//!
//!
#ifdef __cplusplus
extern "C"
{
#endif

//----------------------------------------------------------------------
// includes
//----------------------------------------------------------------------
#include "osalIO_common.h"
#include "audioin.h"

/* fine control over COMPONENT TracePrintf */
/* 1 to enable traces for this module */
#define STRACE_CONTROL 0

#if defined(CONFIG_APP_STREAMDEC_AUDIO_OUT) || defined (CONFIG_APP_STREAMDEC_AUDIO_TO_ADR3A)
        #include <alsa/asoundlib.h>
#endif

//----------------------------------------------------------------------
// defines
//----------------------------------------------------------------------
#define AUDIOIO_C_S32_IO_VERSION         (tS32)(0x00000100)

//----------------------------------------------------------------------
// typedefs
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// variable inclusion  (scope: global)
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// variable definition (scope: module-local)
//----------------------------------------------------------------------
#if defined(CONFIG_APP_STREAMDEC_AUDIO_OUT) || defined (CONFIG_APP_STREAMDEC_AUDIO_TO_ADR3A)
static snd_pcm_t *handle = OSAL_NULL;
static char *device = "hw:5,0";                     /* capture device */
static snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE; /* sample format */
static tU32 rate = 48000;                       /* stream rate */
static tU32 channels = 4;                       /* count of channels */
static tU32 buffer_time = 500000;               /* ring buffer length in us */
static tU32 period_time = 100000;               /* period time in us */
static tS32 resample = 0;
static snd_pcm_sframes_t buffer_size;
static snd_pcm_sframes_t period_size;
#endif

//----------------------------------------------------
//  Global structures:
//----------------------------------------------------

static const OSAL_trDevFunctionTable AudioIOFunctionTable =
{
NULL, NULL, AudioIOIN_s32IOOpen, AudioIOIN_s32IOClose, AudioIOIN_s32IOControl, NULL, NULL,
};

static const OSAL_trOpenTable AudioIOOpenTable =
{
OSAL_EN_NOT_MULTIOPEN, (tU32) 0,
};

//----------------------------------------------------
//  Global structures:
//----------------------------------------------------

#ifdef DPAL_CODEC
static tU8 DPAL_version;
#endif

//----------------------------------------------------------------------
// function prototype (scope: module-local)
//----------------------------------------------------------------------
#if defined (CONFIG_APP_STREAMDEC_AUDIO_OUT) || defined (CONFIG_APP_STREAMDEC_AUDIO_TO_ADR3A)
static tVoid AudioIO_Configure_codec(void);
static tS32 set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access);
static tS32 set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);
static tS32 read_loop(snd_pcm_t *handle, tS16 *samples, tU32 size);
#endif

//----------------------------------------------------------------------
// function inclusion  (scope: global)
//----------------------------------------------------------------------
extern tVoid Linux_vSetErrorCode( OSAL_tThreadID, tU32);

//----------------------------------------------------------------------
// function implementation (scope: local)
//----------------------------------------------------------------------
#if defined (CONFIG_APP_STREAMDEC_AUDIO_OUT) || defined (CONFIG_APP_STREAMDEC_AUDIO_TO_ADR3A)

/**
* @brief    set_hwparams
*
* @details  
*
* @param    
*
* @return
*/
static tS32 set_hwparams(snd_pcm_t *handle,
			snd_pcm_hw_params_t *params,
			snd_pcm_access_t access)
{
	tU32 rrate;
	snd_pcm_uframes_t size;
	tS32 err, dir;

	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0)
	{
		return err;
	}
	err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
	if (err < 0)
	{
		return err;
	}
	err = snd_pcm_hw_params_set_access(handle, params, access);
	if (err < 0)
	{
		return err;
	}
	err = snd_pcm_hw_params_set_format(handle, params, format);
	if (err < 0)
	{
		return err;
	}
	err = snd_pcm_hw_params_set_channels(handle, params, channels);
	if (err < 0)
	{
		return err;
	}
	rrate = rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
	if (err < 0)
	{
		return err;
	}
	if (rrate != rate)
	{
		return -1;
	}
	err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
   	if (err < 0)
   	{
		return err;
   	}
   	err = snd_pcm_hw_params_get_buffer_size(params, &size);
   	if (err < 0)
   	{
		return err;
   	}
   	buffer_size = size;
   	err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time, 0);
   	if (err < 0)
   	{
		return err;
   	}
   	err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
   	if (err < 0)
   	{
		return err;
	}
   	period_size = size;
   	err = snd_pcm_hw_params(handle, params);
   	if (err < 0)
   	{
		return err;
   	}
   	return 0;
}

/**
* @brief    set_swparams
*
* @details  
*
* @param    
*
* @return
*/
static tS32 set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
	tS32 err;

	err = snd_pcm_sw_params_current(handle, swparams);
	if (err < 0)
	{
		return err;
	}
	/* start the transfer when the buffer is almost full: */
	/* (buffer_size / avail_min) * avail_min */
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
	if (err < 0)
   	{
		return err;
   	}
	/* allow the transfer when at least period_size samples can be processed */
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size);
	if (err < 0)
	{
		return err;
	}
	err = snd_pcm_sw_params(handle, swparams);
	if (err < 0)
	{
		return err;
	}
   	return 0;
}

/**
* @brief    read_xrun_recovery
*
* @details  
*
* @param    
*
* @return
*/
static tS32 read_xrun_recovery(snd_pcm_t *handle, tS32 err)
{
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_COMPONENT)
    OSALUTIL_s32TracePrintf(0, TR_LEVEL_COMPONENT, TR_CLASS_OSALIO, "audioin: stream recovery from %s", snd_strerror(err));
#endif
	if (err == -EPIPE)
	{
		/* under-run */
		err = snd_pcm_prepare(handle);
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_ERRORS)
		if (err < 0)
		{
			OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALIO, "audioin: Can't recover from underrun, prepare failed");
		}
#endif
		return 0;
	}
	else if (err == -ESTRPIPE)
	{
		while ((err = snd_pcm_resume(handle)) == -EAGAIN)
		{
			sleep(1);       /* wait until the suspend flag is released */
		}
		if (err < 0)
		{
			err = snd_pcm_prepare(handle);
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_ERRORS)
			if (err < 0)
			{
				OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALIO, "audioin: Can't recover from suspend, prepare failed");
			}
#endif
		}
		return 0;
	}
	return err;
}

/**
* @brief    read_loop
*
* @details  
*
* @param    
*
* @return
*/
static tS32 read_loop(snd_pcm_t *handle, tS16 *samples, tU32 size_in_bytes)
{
	tS16 *ptr;
	tS32 err, cptr;

	ptr = samples;
	cptr = size_in_bytes / (2 * channels);
	err = -EAGAIN;
	while (err == -EAGAIN)//(cptr > 0) do not wait alsa buffer filling
	{
		err = snd_pcm_readi(handle, ptr, cptr);
		if (err == -EAGAIN)
		{
			continue;
		}
		if (err < 0)
		{
			if (read_xrun_recovery(handle, err) < 0)
			{
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_ERRORS)
				OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALIO, "audioin: Read error %s", snd_strerror(err));
#endif
				return -1;
			}
			break;  /* skip one period */
		}
		ptr += err * channels;
		cptr -= err;
	}
	return (size_in_bytes - (cptr * (2 * channels)));
}

//----------------------------------------------------------------------
// FUNCTION    :AudioIOIN_Configure_codec
//
// DESCRIPTION : Configure the codec for the AudioIO
//----------------------------------------------------------------------
//!
//! \brief       <i><b> Configure the codec for the AudioIO </b></i>
//! \details
//! \param       tVoid
//! \return      tVoid
//! \sa          n.a.
//! \callgraph
//! \cal
static tVoid AudioIOIN_Configure_codec(void)
{
	tS32 err;

	snd_pcm_hw_params_t *hwparams = NULL;
	snd_pcm_sw_params_t *swparams = NULL;

	if (handle != OSAL_NULL)
	{
		if ((err = snd_pcm_close(handle)) < 0)
		{
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_ERRORS)
			OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALIO, "audioin: initialization close error (%s)", snd_strerror(err));
#endif
			goto init_fail;
		}
		handle = OSAL_NULL;
	}

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);

	if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK)) < 0)
	{
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_ERRORS)
		OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALIO, "audioin: initialization error (%s)", snd_strerror(err));
#endif
		goto init_fail;
	}
	if ((err = set_hwparams(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_ERRORS)
		OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALIO, "audioin: setting of hwparams failed (%s)", snd_strerror(err));
#endif
		goto init_fail;
	}

	if ((err = set_swparams(handle, swparams)) < 0)
	{
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_ERRORS)
		OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALIO, "audioin: setting of swparams failed (%s)", snd_strerror(err));
#endif
		goto init_fail;
	}

	goto init_pass;

init_fail:
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_ERRORS)
	OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALIO, "audioin: Sound will not be available");
#endif
init_pass:
	if (hwparams != NULL)
	{
//		snd_pcm_hw_params_free(hwparams);
	}
	if (swparams != NULL)
	{
//		snd_pcm_sw_params_free(swparams);
	}
	return;
}
#endif

//----------------------------------------------------------------------
// function implementation (scope: global)
//----------------------------------------------------------------------
tVoid AUDIO_IOIN_vIOInit(void)
{
    (void)OSALIO_s32AddDevice(((OSAL_tenDevID)(OSAL_EN_DEVID_AUDIOIO_2)), AudioIOFunctionTable, AudioIOOpenTable);
//    (void)OSALIO_s32AddDevice(((OSAL_tenDevID)(OSAL_EN_DEVID_AUDIOIO_3)), AudioIOFunctionTable, AudioIOOpenTable);

#if defined(CONFIG_APP_STREAMDEC_AUDIO_OUT) || defined (CONFIG_APP_STREAMDEC_AUDIO_TO_ADR3A)
    AudioIOIN_Configure_codec();
#endif
}

tVoid AUDIO_IOIN_vIODeInit(void)
{
    OSALIO_s32RemoveDevice(((OSAL_tenDevID)(OSAL_EN_DEVID_AUDIOIO_2)));
//    OSALIO_s32RemoveDevice(((OSAL_tenDevID)(OSAL_EN_DEVID_AUDIOIO_3)));
}

//----------------------------------------------------------------------
// FUNCTION    :AudioIOIN_s32IOOpen
//
// DESCRIPTION : AudioIO_s32IOOpen required device
//----------------------------------------------------------------------
//!
//! \brief      <i><b>AudioIO_s32IOOpen required device</b></i>
//! \details
//! \param[in]  tS32 s32DevId       : The path of theAudioIO device
//! \return     OSAL_OK on succes, OSAL_ERROR otherwise
//! \sa          n.a.
//! \callgraph
//! \callergraph
//!
tS32 AudioIOIN_s32IOOpen(OSAL_tenDevID tDevID, tCString coszName, OSAL_tenAccess enAccess, OSAL_tIODescriptor * pfd)
{
	*pfd = ((OSAL_tIODescriptor) OSAL_IODESCRIPTOR(tDevID, ++OSAL_arIODevTable[tDevID].pDevOpenTable->OpenCounter));

	return (OSAL_OK);
}

//----------------------------------------------------------------------
// FUNCTION    :AudioIOIN_s32IOClose
//
// DESCRIPTION : CloseAudioIO required device
//----------------------------------------------------------------------
//!
//! \brief      <i><b>CloseAudioIO required device</b></i>
//! \details
//! \param[in]  tS32 s32DevId       : The path of theAudioIO device
//! \return     OSAL_OK on succes, OSAL_ERROR otherwise
//! \sa          n.a.
//! \callgraph
//! \callergraph
//!
tS32 AudioIOIN_s32IOClose(OSAL_tIODescriptor fd)
{
    return OSAL_OK;
}

//----------------------------------------------------------------------
// FUNCTION    :AudioIO_s32IOControl
// DESCRIPTION : Call aAudioIO control function
//----------------------------------------------------------------------
//!
//! \brief      <i><b>Call aAudioIO control function</b></i>
//! \details
//! \param[in]  tS32 s32DevId       : The path of theAudioIO device
//! \param[in]  tS32 fun:                   specific action
//!             -   OSAL_C_S32_IOCTRL_VERSION
//!                     This function gets version of this device.
//!             -   OSAL_C_S32_IOCTRL_AUDIO_IO_INIT
//!                     Configure the AudioIo device
//!             -   OSAL_C_S32_IOCTRL_AUDIO_IO_ENQUEUE_BUFFER
//!                     Add a new frame buffer to the queue
//!                 -   OSAL_C_S32_IOCTRL_AUDIO_IO_START_TRANSMISSION
//!                     Start the Tx/Rx
//!             -   OSAL_C_S32_IOCTRL_AUDIO_IO_STOP_TRANSMISSION
//!                     Stop the Tx/Rx
//!             -   OSAL_C_S32_IOCTRL_AUDIO_IO_GET_QUEUE_LENGTH
//!                     Return the Frame queue length
//! \param[in]   tS32 arg:  a generic input parameter
//! \return      OSAL_E_NOERROR on success, an OSAL_ERROR otherwise.
//! \sa          n.a.
//! \callgraph
//! \callergraph
//!
tS32 AudioIOIN_s32IOControl(OSAL_tIODescriptor fd, tS32 s32fun, tS32 s32arg)
{
    tS32 s32ReturnValue = OSAL_OK;
    tU32 u32ErrorCode = OSAL_E_NOERROR;
#if defined(CONFIG_APP_STREAMDEC_AUDIO_OUT) || defined (CONFIG_APP_STREAMDEC_AUDIO_TO_ADR3A)
	tS32 ret;
#endif

#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_COMPONENT)
    char *fun;
    if (STRACE_CONTROL) fun = (s32fun == OSAL_C_S32_IOCTRL_AUDIO_IO_ENQUEUE_BUFFER) ? "ENQUEUE function" : "other function";
    if (STRACE_CONTROL) OSALUTIL_s32TracePrintf(0, TR_LEVEL_COMPONENT, TR_CLASS_OSALIO, "AudioIOIN_s32IOControl (%s, 0x%x)", fun, s32fun);
#endif
  
	switch (s32fun)
	{
		case OSAL_C_S32_IOCTRL_VERSION:
			*(tPS32) s32arg = AUDIOIO_C_S32_IO_VERSION;
		break;

		case OSAL_C_S32_IOCTRL_AUDIO_IO_INIT:
		break;

		case OSAL_C_S32_IOCTRL_AUDIO_IO_ENQUEUE_BUFFER:
		{
#if defined(CONFIG_APP_STREAMDEC_AUDIO_OUT) || defined (CONFIG_APP_STREAMDEC_AUDIO_TO_ADR3A)
			tS32 err;
			OSAL_trAudioIOBufferExchangeInfo * BufferExInfo = (OSAL_trAudioIOBufferExchangeInfo *) s32arg;
			OSAL_trAudioIOBufferInfo * BufferInfo = BufferExInfo->ptBufferInfo;
			tU32* pvDataBuffer = BufferInfo->pvDataBuffer;
			tU32 SamplesSize = BufferInfo->u32BufferSize;
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_COMPONENT)
    		if (STRACE_CONTROL) OSALUTIL_s32TracePrintf(0, TR_LEVEL_COMPONENT, TR_CLASS_OSALIO, "AudioIOIN_s32IOControl ENQUEUE function %d bytes", SamplesSize);
#endif

			err = snd_pcm_avail(handle);
			if (err < 0)
			{
				err = read_xrun_recovery(handle, err);
				if (err == 0)
				{
					err = snd_pcm_avail(handle);
					if (err < 0)
					{
						SamplesSize = 0;
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_ERRORS)
						OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALIO, "audioin pcm_avail error: %s", snd_strerror(err));
#endif
					}
				}
			}
			else if (err == 0)
			{
				SamplesSize = 0;
			}
			if (err > 0)
			{
				pvDataBuffer += (BufferExInfo->u32CurrentQueueLength / sizeof(tU32));
				if ((BufferExInfo->u32CurrentQueueLength + (err * (2 * channels))) > SamplesSize)
				{
					SamplesSize -= BufferExInfo->u32CurrentQueueLength;
				}
				else
				{
					SamplesSize = (err * (2 * channels));
				}
			}
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_COMPONENT)
			OSALUTIL_s32TracePrintf(0, TR_LEVEL_COMPONENT, TR_CLASS_OSALIO, "audioin Frame buffer size %d", ((buffer_size * (2 * channels)) - BufferExInfo->u32CurrentQueueLength));
#endif
			if (SamplesSize != 0)
			{
				if ((err = read_loop(handle, (tU16*)pvDataBuffer, SamplesSize)) < 0)
				{
					/* TODO: define the error type! */
				}
				else
				{
					BufferExInfo->u32CurrentQueueLength += err;
				}
			}
#endif
		}
		break;

		case OSAL_C_S32_IOCTRL_AUDIO_IO_START_TRANSMISSION:
		{
#if defined(CONFIG_APP_STREAMDEC_AUDIO_OUT) || defined (CONFIG_APP_STREAMDEC_AUDIO_TO_ADR3A)
			if ((ret = snd_pcm_start(handle)) < 0)
			{
				u32ErrorCode = OSAL_E_NOACCESS;
				s32ReturnValue = OSAL_ERROR;
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_ERRORS)
				OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALIO, "AudioIOIN_s32IOControl snd_pcm_start (0x%x, %s)", ret, snd_strerror(ret));
#endif
			}
#endif
		}
		break;

		case OSAL_C_S32_IOCTRL_AUDIO_IO_STOP_TRANSMISSION:
		{
#if defined(CONFIG_APP_STREAMDEC_AUDIO_OUT) || defined (CONFIG_APP_STREAMDEC_AUDIO_TO_ADR3A)
			snd_pcm_drain(handle);
#endif
		}
		break;

		case OSAL_C_S32_IOCTRL_AUDIO_IO_ALLOCATE_BUFFERS:
		case OSAL_C_S32_IOCTRL_AUDIO_IO_DEALLOCATE_BUFFERS:
		case OSAL_C_S32_IOCTRL_AUDIO_IO_GET_QUEUE_LENGTH: 
		case OSAL_C_S32_IOCTRL_AUDIO_IO_RESET_PLL_FRAC:
		case OSAL_C_S32_IOCTRL_AUDIO_IO_MODIFY_PLL_FREQ:
		default:
			u32ErrorCode = OSAL_E_NOTSUPPORTED;
			s32ReturnValue = OSAL_ERROR;
		break;
	}

    if (u32ErrorCode != OSAL_E_NOERROR)
    {
#if defined(CONFIG_ENABLE_CLASS_OSALIO) && (CONFIG_ENABLE_CLASS_OSALIO >= TR_LEVEL_ERRORS)
	OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_OSALIO, "AudioIOIN_s32IOControl (%s, 0x%x)", OSAL_coszErrorText(u32ErrorCode), u32ErrorCode);
#endif
        Linux_vSetErrorCode(OSAL_C_THREAD_ID_SELF, u32ErrorCode);
    }

    return (s32ReturnValue);
}

#ifdef __cplusplus
}
#endif

// END OF FILE
