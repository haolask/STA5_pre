//!
//!  \file 		etalinit.c
//!  \brief 	<i><b> ETAL initialization </b></i>
//!  \details   The ETAL system initialization functions
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#ifdef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
#include "osalmain.h"
#include "streamdecadapt.h"
#endif
#include "etalinternal.h"

#ifdef CONFIG_ETAL_SUPPORT_CMOST
	#include "tunerdriver.h"
	#include "cmost_helpers.h"
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_RESET_LIGHT_FREERTOS
	#include "bsp_sta1095evb.h"
#endif

/***********************************
 *
 * Defines and macros
 *
 **********************************/
/* CMOST silicon version register;
 * see https://codex.cro.st.com/tracker/?func=detail&aid=349087&atid=4573&group_id=1277 */
#define ETAL_CMOST_VERSION_REG_ADDRESS    0x01401E
#define ETAL_CMOST_VERSION_REG_LEN        0x02

/***********************************
 *
 * Local types
 *
 **********************************/

/***********************************
 *
 * Local variables
 *
 **********************************/
/*!
 * \var		ETAL_callbackThreadId
 * 			Callback thread ID, needed only for deinitialization
 */
static OSAL_tThreadID ETAL_callbackThreadId[ETAL_CALLBACK_HANDLERS_NUM];
/*!
 * \var		ETAL_datahandlerThreadId
 * 			Datahandler thread ID, needed only for deinitialization
 */
static OSAL_tThreadID ETAL_datahandlerThreadId[ETAL_DATA_HANDLERS_NUM];
/*!
 * \var		ETAL_controlThreadId
 * 			Control thread ID, needed only for deinitialization
 */
static OSAL_tThreadID ETAL_controlThreadId;
#if defined(CONFIG_COMM_ENABLE_RDS_IRQ)
/*!
 * \var		ETAL_IRQThreadId
 * 			IRQ thread ID, needed only for deinitialization
 */
static OSAL_tThreadID ETAL_IRQThreadId;
#endif

/***************************
 *
 * ETAL_restart
 *
 **************************/
/*!
 * \brief		Resets ETAL to the status it has just after startup
 * \details		
 * \remark		This function is provided only for the test environment:
 * \remark		**ITS USE IN ANY OTHER SITUATION IS HIGHLY DEPRECATED**
 * \callgraph
 * \callergraph
 */
tVoid ETAL_restart(tVoid)
{
	EtalHardwareAttr hardware_attr;
	tU32 i;
	ETAL_HANDLE vl_hReceiver;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	tBool pad_active;
#endif

	ETAL_statusGetInternalLock();

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	pad_active = ETAL_statusIsPADActive();
#endif

	/* Destroy any valid receiver */
	for (i = 0; i < ETAL_MAX_RECEIVERS; i++)
	{
		vl_hReceiver = ETAL_handleMakeReceiver((ETAL_HINDEX)i);

		if (ETAL_receiverIsValidHandle(vl_hReceiver))
		{
			if (ETAL_receiverGetLock(vl_hReceiver) == ETAL_RET_SUCCESS)
			{
				(LINT_IGNORE_RET)ETAL_destroyReceiverInternal(vl_hReceiver);
				ETAL_receiverReleaseLock(vl_hReceiver);
			}
		}
	}
	
	ETAL_statusHardwareAttrBackup(&hardware_attr);
	ETAL_statusInternalInit(&hardware_attr);
	ETAL_receiverInit();

	/*
	 * ETAL_restart does not reload the DCOP thus we have to avoid
	 * sending the GetPADDLS in auto mode or the DCOP will respond
	 * "Error Duplicated auto command" and further tests will fail
	 */
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	if (pad_active)
	{
		ETAL_statusSetPADActive();
	}
#endif
	ETAL_statusSetInitialized(TRUE);

	ETAL_statusReleaseInternalLock();
}

/***********************************
 *
 * ETAL_initThreadSpawn
 *
 **********************************/
/*!
 * \brief		Starts the main ETAL Operating System threads
 * \details		The function starts the Datahandler threads, the Callback threads,
 * 				the Control thread and optionally (depending on the build configuration)
 * 				the IRQ thread.
 * \return		OSAL_OK
 * \return		OSAL_ERROR_DEVICE_INIT - error during the thread creation
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_initThreadSpawn(tVoid)
{
	OSAL_trThreadAttribute thread1_attr;
	OSAL_trThreadAttribute thread2_attr;
	OSAL_trThreadAttribute thread3_attr;
#if defined(CONFIG_COMM_ENABLE_RDS_IRQ)
	OSAL_trThreadAttribute thread4_attr;
#endif
	tChar name[ETAL_THREAD_BASE_NAME_MAX_LEN];
	tU32 i;

	tSInt ret = OSAL_OK;

	thread1_attr.szName = name;
	thread1_attr.u32Priority =  ETAL_DATA_THREAD_PRIORITY;
	thread1_attr.s32StackSize = ETAL_DATA_STACK_SIZE;
	thread1_attr.pfEntry = &ETAL_DataHandler_ThreadEntry;
	thread1_attr.pvArg = NULL;

	for (i = 0; i < ETAL_DATA_HANDLERS_NUM; i++)
	{
		if (OSAL_s32NPrintFormat(name, ETAL_THREAD_BASE_NAME_MAX_LEN, "%s%u", ETAL_DATA_THREAD_BASE_NAME, i) < 0)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
			goto exit;
		}
		thread1_attr.pvArg = (tPVoid)((tULong)i);
		ETAL_datahandlerThreadId[i] = OSAL_ThreadSpawn(&thread1_attr);
		if (ETAL_datahandlerThreadId[i] == OSAL_ERROR)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
			goto exit;
		}
	}

	thread2_attr.szName = name;
	thread2_attr.u32Priority =  ETAL_CALLBACK_THREAD_PRIORITY;
	thread2_attr.s32StackSize = ETAL_CALLBACK_STACK_SIZE;
	thread2_attr.pfEntry = &ETAL_CallbackHandler_ThreadEntry;

	for (i = 0; i < ETAL_CALLBACK_HANDLERS_NUM; i++)
	{
		if (OSAL_s32NPrintFormat(name, ETAL_THREAD_BASE_NAME_MAX_LEN, "%s%u", ETAL_CALLBACK_THREAD_BASE_NAME, i) < 0)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
			goto exit;
		}
		thread2_attr.pvArg = (tPVoid)((tULong)i);
		ETAL_callbackThreadId[i] = OSAL_ThreadSpawn(&thread2_attr);
		if (ETAL_callbackThreadId[i] == OSAL_ERROR)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
			goto exit;
		}
	}

	thread3_attr.szName =       ETAL_CONTROL_THREAD_NAME;
	thread3_attr.u32Priority =  ETAL_CONTROL_THREAD_PRIORITY;
	thread3_attr.s32StackSize = ETAL_CONTROL_STACK_SIZE;
	thread3_attr.pfEntry = &ETAL_Control_ThreadEntry;
	thread3_attr.pvArg = NULL;

	ETAL_controlThreadId = OSAL_ThreadSpawn(&thread3_attr);
	if (ETAL_controlThreadId == OSAL_ERROR)
	{
		ret = OSAL_ERROR_DEVICE_INIT;
		goto exit;
	}


#if defined(CONFIG_COMM_ENABLE_RDS_IRQ)
	thread4_attr.szName =       ETAL_IRQ_THREAD_NAME;
	thread4_attr.u32Priority =  ETAL_IRQ_THREAD_PRIORITY;
	thread4_attr.s32StackSize = ETAL_IRQ_STACK_SIZE;
	thread4_attr.pfEntry = &ETAL_IRQ_ThreadEntry;
	thread4_attr.pvArg = NULL;

	ETAL_IRQThreadId = OSAL_ThreadSpawn(&thread4_attr);
	if (ETAL_IRQThreadId == OSAL_ERROR) 
	{
		ret = OSAL_ERROR_DEVICE_INIT;
		goto exit;
	}
#endif

exit:
	return ret;	
}

/***********************************
 *
 * ETAL_initThreadDelete
 *
 **********************************/
/*!
 * \brief		Destroys the Operating System threads created by #ETAL_initThreadSpawn
 * \return		OSAL_OK
 * \return		OSAL_ERROR - error while destroying a thread
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_initThreadDelete(tVoid)
{
	tU32 i;
	tSInt retosal = OSAL_OK;

	if ((ETAL_controlThreadId != OSAL_ERROR) &&
		(OSAL_s32ThreadDelete(ETAL_controlThreadId) == OSAL_OK))
	{
		ETAL_controlThreadId = OSAL_ERROR; /* invalidate the ID */
	}
	else
	{
		retosal = OSAL_ERROR;
	}

#if defined(CONFIG_COMM_ENABLE_RDS_IRQ)
	if ((ETAL_IRQThreadId != OSAL_ERROR) &&
		(OSAL_s32ThreadDelete(ETAL_IRQThreadId) == OSAL_OK))
	{
		ETAL_IRQThreadId = OSAL_ERROR; /* invalidate the ID */
	}
	else
	{
		retosal = OSAL_ERROR;
	}
#endif

	for (i = 0; i < ETAL_DATA_HANDLERS_NUM; i++)
	{
		if ((ETAL_datahandlerThreadId[i] != OSAL_ERROR) &&
			(OSAL_s32ThreadDelete(ETAL_datahandlerThreadId[i]) == OSAL_OK))
		{
			ETAL_datahandlerThreadId[i] = OSAL_ERROR; /* invalidate the ID */
		}
		else
		{
			retosal = OSAL_ERROR;
		}
	}

	for (i = 0; i < ETAL_CALLBACK_HANDLERS_NUM; i++)
	{
		if ((ETAL_callbackThreadId[i] != OSAL_ERROR) && 
			(OSAL_s32ThreadDelete(ETAL_callbackThreadId[i]) == OSAL_OK))
		{
			ETAL_callbackThreadId[i] = OSAL_ERROR; /* invalidate the ID */
		}
		else
		{
			retosal = OSAL_ERROR;
		}
	}

	/* give the killed thread the opportunity to do cleanup if required */
	(void)OSAL_s32ThreadWait(ETAL_API_THREAD_SCHEDULING);
	return retosal;
}

/***********************************
 *
 * ETAL_initRollback
 *
 **********************************/
/*!
 * \brief		destroys the resources created during ETAL_init
 * \details		If an error occurs during ETAL_init some resources (semaphores, threads)
 * 				may be created, others not. It is not possible to call
 * 				#etal_deinitialize since the call will be rejected with ETAL_RET_NOT_INITIALIZED
 * 				so this function tries to roll back the operations performed up to the
 * 				moment the function is called. To do this it uses a global state variable,
 * 				updated every time an operation creating some operating system resource succeds.
 * 				After etal_initialize succeeds this function is completely useless.
 * \param[in]	power_up - TRUE if the operation is a power up
 * \callgraph
 * \callergraph
 */
tVoid ETAL_initRollback(tBool power_up)
{
	EtalInitState state;
	
	state = ETAL_initStatusGetState();

	do
	{
		switch (state)
		{
			case state_initNotInitialized:
				break;

			case state_initStart:
				break;

			case state_OSALINIT:
				state = state_initStart;
				break;

			case state_tracePrintInit:
				if (power_up)
				{
					(LINT_IGNORE_RET) TUNERDRIVER_system_deinit();
				}
				state = state_OSALINIT;
				break;

			case state_statusInitLock:
				ETAL_tracePrintDeInit();
				state = state_tracePrintInit;
				break;

			case state_callbackInit:
				(LINT_IGNORE_RET) ETAL_statusDeinitLock(power_up);
				state = state_statusInitLock;
				break;

			case state_datahandlerInit:
				(LINT_IGNORE_RET) ETAL_callbackDeinit();
				state = state_callbackInit;
				break;

			case state_controlInit:
				(LINT_IGNORE_RET) ETAL_controlDeinit();
				state = state_callbackInit;
				break;

			case state_initCommunication_CMOST:
				(LINT_IGNORE_RET) ETAL_datahandlerDeinit();
				state = state_callbackInit;
				break;

			case state_initCheckSiliconVersion_CMOST:
#ifdef CONFIG_ETAL_SUPPORT_CMOST
				(LINT_IGNORE_RET) ETAL_deinitCommunication_CMOST();
#endif
				state = state_initCommunication_CMOST;
				break;

			case state_downloadFirmware_CMOST:
				state = state_initCheckSiliconVersion_CMOST;
				break;

			case state_initPingCmost:
				state = state_downloadFirmware_CMOST;
				break;

			case state_boardInit:
				state = state_downloadFirmware_CMOST;
				break;

			case state_initCommunication_MDR:
				state = state_boardInit;
				break;

			case state_initCommunication_HDRADIO:
				state = state_initCommunication_MDR;
				break;

			case state_initReadSiliconVersion_DCOP:
				if (ETAL_statusHardwareAttrIsDCOPActive())
				{
#if defined (CONFIG_ETAL_SUPPORT_DCOP_HDRADIO)
					(LINT_IGNORE_RET) ETAL_deinitCommunication_HDRADIO();
#endif
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
					(LINT_IGNORE_RET) ETAL_deinitCommunication_MDR();
#endif
				}
				state = state_initCommunication_HDRADIO;
				break;

			case state_initThreadSpawn:
				state = state_initReadSiliconVersion_DCOP;
				break;

			case state_initCustomParameter_CMOST:
				state = state_initThreadSpawn;
				break;

			case state_initPingDcop:
				(LINT_IGNORE_RET) ETAL_statusReleaseLock();
				(LINT_IGNORE_RET) ETAL_initThreadDelete();
				state = state_initCustomParameter_CMOST;
				break;

			case state_statusExternalInit:
#if defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)
				// If DCOP active, then request a power down command for saving landscape.
				//
				if (ETAL_statusHardwareAttrIsDCOPActive())
				{
					(LINT_IGNORE_RET) ETAL_cmdPowerDown_MDR(ETAL_DAB_NVM_SAVE_DAB_LANDSCAPE);
				}
#endif
				state = state_initPingDcop;
				break;

			case state_ETALTML_init:
				state = state_statusExternalInit;
				break;

			case state_AudioInterfaceConfigure:
#if defined (CONFIG_ETAL_HAVE_ETALTML)
				(LINT_IGNORE_RET) ETALTML_deinit(power_up);
#endif
				state = state_ETALTML_init;
				break;

			case state_ETALMDR_init:
#ifdef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
				(LINT_IGNORE_RET) streamdecadapt_destructor();
#endif
				state = state_AudioInterfaceConfigure;
				break;

			case state_initComplete:
#ifndef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
				state = state_AudioInterfaceConfigure;
#else
				state = state_ETALMDR_init;
#endif
				break;

			/* no default label intentional to have a compiler
			 * warning in case of missing labels */
		}
	} while (state != state_initStart);
}

#ifdef CONFIG_ETAL_SUPPORT_CMOST
/***************************
 *
 * ETAL_initCheckSiliconVersion_CMOST
 *
 **************************/
/*!
 * \brief		Reads the silicon version from the CMOST and verifies if compatible
 * \details		The silicon version is stored in registers accessible also
 * 				before the CMOST firmware is started. The registers contain
 * 				the silicon type and the silicon cut, in the following format:
 * 				register 0x1401E = 0xyy00jjjj where 0xyy encode the chip address
 * 				                              and 0xjjjj encode the silicon version
 * 				                              in ASCII chars (e.g. 'BG' for cut BG)
 * 				register 0x1401F = 0xwwwwwwww the silicon name in ASCII (e.g. 'V766' for STAR-T)
 *
 * 				The function also checks if the firmware or patches embedded in ETAL
 * 				are compatible with the silcon version. The latter check can be controlled
 * 				with CONFIG_ETAL_INIT_CHECK_SILICON_VERSION.
 * \param[in]	tuner_index - the index of the tuner to access (index into the #etalTuner array)
 * \return		OSAL_ERROR_INVALID_PARAM - invalid *tuner_index*
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - device communication error
 * \return		OSAL_ERROR - silicon version mismatch
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_initCheckSiliconVersion_CMOST(tU32 tuner_index)
{
	ETAL_HANDLE hTuner;
	tU8 resp1[4];
	tU8 resp2[4];
	tChar vers[8];
	EtalDeviceDesc deviceDescription;
	tSInt ret = OSAL_OK;

	hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)tuner_index);
	if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
	{
		if (ETAL_tunerGetAddress(hTuner, &deviceDescription) != OSAL_OK)
		{
			ret = OSAL_ERROR_INVALID_PARAM;
			goto exit;
		}

		if ((TUNERDRIVER_readRaw32_CMOST(tuner_index, ETAL_CMOST_VERSION_REG_ADDRESS, resp1) != OSAL_OK) ||
			(TUNERDRIVER_readRaw32_CMOST(tuner_index, ETAL_CMOST_VERSION_REG_ADDRESS+1, resp2) != OSAL_OK))
		{
			ret = OSAL_ERROR_DEVICE_NOT_OPEN;
			goto exit;
		}
		else
		{
			vers[0] = (tChar)resp1[2];
			vers[1] = (tChar)resp1[3];
			vers[2] = ' ';
			vers[3] = (tChar)resp2[0];
			vers[4] = (tChar)resp2[1];
			vers[5] = (tChar)resp2[2];
			vers[6] = (tChar)resp2[3];
			vers[7] = (tChar)0x00;
			ETAL_initStatusSetTunerVersion(tuner_index, vers);
#if defined (CONFIG_ETAL_INIT_CHECK_SILICON_VERSION)
			if (!ETAL_initStatusIsCompatibleTunerVersion(tuner_index))
			{
				ret = OSAL_ERROR;
				goto exit;
			}
#endif
		}
	}

	// add a hw version print update
	ETAL_tracePrintHwVersion();

exit:
	return ret;
}

/***************************
 *
 * ETAL_initProgramCustomParameter_CMOST
 *
 **************************/
/*!
 * \brief		Sends the custom parameters provided in etal_initialize to the TUner
 * \details		Checks if the custom parameters are provided in the hardware attrs
 * 				passed to etal_initialize and if so sends them to the tuner
 * \remark		Does not check if the Tuner is enabled
 * \param[in]	tuner_index - the index of the tuner to access (index into the #etalTuner array)
 * \return		OSAL_ERROR - invalid *tuner_index*
 * \return		OSAL_ERROR_INVALID_PARAM - invalid parameter array size
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - device communication error
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_initProgramCustomParameter_CMOST(tU32 tuner_index)
{
	ETAL_HANDLE hTuner;
	tU32 *params;
	tU32 params_size;
	tSInt ret = OSAL_OK;

	hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)tuner_index);
	if (!ETAL_tunerIsValidHandle(hTuner))
	{
		ret = OSAL_ERROR;
		goto exit;
	}
	if (ETAL_statusHardwareAttrUseCustomParams(hTuner))
	{
		ETAL_statusHardwareAttrGetCustomParams(hTuner, &params, &params_size);
		if ((params == NULL) || (params_size == 0))
		{
			ret = OSAL_ERROR_INVALID_PARAM;
			goto exit;
		}
		else if (params_size > ETAL_DEF_MAX_READWRITE_SIZE)
		{
			ret = OSAL_ERROR_INVALID_PARAM;
			goto exit;
		}
		else
		{
			/* Nothing to do */
		}
		/* ETAL_write_parameter_internal might invoke the callback handler
		 * in case of errors; at this stage of the initialization
		 * the thread is not started yet (it is started in state_initThreadSpawn)
		 * but the FIFO is initialized (in state_callbackInit) */
		if (ETAL_write_parameter_internal(hTuner, fromAddress, params, (tU16)params_size) != ETAL_RET_SUCCESS)
		{
			ret = OSAL_ERROR_DEVICE_NOT_OPEN;
			goto exit;
		}
	}

exit:
	return ret;
}
#endif // CONFIG_ETAL_SUPPORT_CMOST

#ifdef CONFIG_ETAL_SUPPORT_DCOP
/***************************
 *
 * ETAL_initReadSiliconVersion_DCOP
 *
 **************************/
/*!
 * \brief		Reads the DCOP silicon version
 * \remark		Not implemented
 * \param[in]	init_params - the parameter used to initialize ETAL through #etal_initialize,
 * 				              used by the function to check if the DCOP is enabled.
 * 				              The function ignores DCOP not enabled.
 * \return		OSAL_OK
 * \callgraph
 * \callergraph
 * \todo		Not implemented
 */
static tSInt ETAL_initReadSiliconVersion_DCOP(const EtalDCOPAttr *Dcop_init_params)
{
	tSInt retosal = OSAL_OK;

	if ((Dcop_init_params == NULL) || (false == Dcop_init_params->m_isDisabled))
	{
	}
	return retosal;
}
#endif // CONFIG_ETAL_SUPPORT_DCOP

#if 0
/***************************
 *
 * ETAL_AudioInterfaceConfigure
 *
 **************************/
static ETAL_STATUS ETAL_AudioInterfaceConfigure(tVoid)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR)
	etalAudioIntfStatusTy vl_audioConfig;
	ETAL_HANDLE hTuner;
	tU8 tuner_index;

	/* TODO: use a configured initialization;
	 * TODO: the initialization is already done in the BSP
	 * should it be repeated here? */
	vl_audioConfig.all = (tU8)0;
#ifdef CONFIG_DIGITAL_AUDIO
	vl_audioConfig.bitfield.m_dac	  = (tU8)0; /* disable audio DAC */
#else
	vl_audioConfig.bitfield.m_dac     = (tU8)1; /* Enable audio DAC */
#endif
	vl_audioConfig.bitfield.m_sai_out = (tU8)1; /* Enable audio SAI output */
	vl_audioConfig.bitfield.m_sai_in  = (tU8)1; /* Enable audio SAI input */

#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
		// HD RADIO is SAI master, so CMOST SAI is slave
		vl_audioConfig.bitfield.m_sai_slave_mode = (tU8)1; /* 0x1: SAI is configured in Slave Mode*/
#else
		vl_audioConfig.bitfield.m_sai_slave_mode = (tU8)0; /* 0x0: SAI is configured in Master Mode*/
#endif

	ETAL_configGetDCOPAudioTunerIndex(&tuner_index);
	if (tuner_index != ETAL_INVALID_HINDEX)
	{
		hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)tuner_index);
		if (ETAL_DEVICE_IS_STAR(ETAL_tunerGetType(hTuner)))
		{
			if (ETAL_cmdSelectAudioInterface_CMOST(hTuner, vl_audioConfig) != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
			}
			else
			{
				ETAL_statusSetTunerAudioStatus(hTuner, vl_audioConfig);
			}
		}
	}
#endif // CONFIG_ETAL_SUPPORT_CMOST_STAR
	return ret;
}
#endif 

/***************************
 *
 * ETAL_initUpdateRetval
 *
 **************************/
/*!
 * \brief		processes the ETAL_init return value
 * \details		Avoids overwriting the previous return value if it contained
 * 				an error
 * \param[in]	retold - the previous return value
 * \param[in]	retnew - the new return value
 * \return		depends on inputs
 * \callgraph
 * \callergraph
 */
static tSInt ETAL_initUpdateRetval(tSInt retold, tSInt retnew)
{
	tSInt ret;
	
	if (retold == OSAL_OK)
	{
		ret = retnew;
	}
	else
	{
		ret = retold;
	}
	
	return ret;
}

/***********************************
 *
 * ETAL_init
 *
 **********************************/
/*!
 * \brief		Main ETAL initialization function
 * \details		The function performs the following actions:
 * 				- initializes OSAL (only if *power_up* is TRUE)
 * 				- sets the ETAL internal variables to a known state
 * 				- creates the ETAL locks (typically Operating System semaphores)
 * 				- initializes the CMOST devices (through the #ETAL_initCommunication_CMOST)
 * 				- resets and loads the DAB DCOP firmware if present
 * 				- resets and initializes the HDRadio DCOP device
 * 				- starts the communication with the devices
 * 				- starts all the ETAL threads (through the #ETAL_initThreadSpawn)
 *
 * \param[in]	hardware_attr - describes some basic system attributes
 * \param[in]	power_up - if TRUE the function performs some initialization
 * 				           normally required only at power up; if FALSE only
 * 				           empties the ETAL data structures, creates the
 * 				           semaphores and reloads the devices.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - generic error (parameter error or flash download error)
 * \return		OSAL_ERROR_DEVICE_INIT - Memory allocation/thread allocation/semaphore creation error
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - Device communication error
 * \callgraph
 * \callergraph
 */
tSInt ETAL_init(const EtalHardwareAttr *hardware_attr, tBool power_up)
{
	tU32 i;
	tSInt retosal;
	tU32 tuner_index;
#ifdef CONFIG_ETAL_SUPPORT_DCOP
	tSInt ret_dcop;
	EtalDcopBootType isBootMode = ETAL_DCOP_BOOT_REGULAR;
#endif
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	tSInt ret_cmost;
	ETAL_HANDLE hTuner;
	EtalDeviceStatus status;
#endif
	tBool	vl_AtLeastOnetunerIsActive = FALSE;

#if defined (CONFIG_DEBUG_ETAL_CHECKS)
	ETAL_runtimecheck();
#endif

	/*************************
	 * state_OSALINIT
	 *
	 * initialize OSAL
	 ************************/
	ETAL_initStatusSetState(state_OSALINIT);

	if (power_up == TRUE)
	{
		if (TUNERDRIVER_system_init() != OSAL_OK)
		{
			retosal = OSAL_ERROR_DEVICE_INIT;
			goto exit;
		}
   }

	/* initialize internal structures (only MemorySet) */

	ETAL_statusInternalInit(hardware_attr);
	ETAL_receiverInit();
	ETAL_intCbInit();

	/*************************
	 * state_tracePrintInit
	 *
	 * initialize trace and log system
	 ************************/
	ETAL_initStatusSetState(state_tracePrintInit);

	retosal = ETAL_tracePrintInit();
	if (retosal == OSAL_ERROR_INVALID_PARAM)
	{
		ETAL_initStatusSetNonFatal(warningTraceDefault);
	}
	else if (retosal != OSAL_OK)
	{
		ETAL_initStatusSetNonFatal(warningNoTrace);
	}
	else
	{
		/* Nothing to do */
	}
	/* tracePrintf errors are non fatal, ignore them
	 * and overwrite retosal
	 */
	retosal = OSAL_OK;

	/***********************************************************************/
	/* if using CONFIG_TRACE_ASYNC printfs not available before this point */
	/***********************************************************************/

	/*************************
	 * state_statusInitLock
	 *
	 * initialize global locks
	 ************************/
	ETAL_initStatusSetState(state_statusInitLock);

	if (ETAL_statusInitLock(power_up) != OSAL_OK)
	{
		ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Status semaphore creation");
		retosal = OSAL_ERROR_DEVICE_INIT;
		goto exit;
	}

	/*************************
	 * state_callbackInit
	 *
	 * initialize callback handler
	 ************************/
	ETAL_initStatusSetState(state_callbackInit);

if (ETAL_callbackInit() != OSAL_OK)
	{
		ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Callback handler initialization");
		retosal = OSAL_ERROR_DEVICE_INIT;
		goto exit;
	}

	/*************************
	 * state_datahandlerInit
	 *
	 * initialize data handler
	 ************************/
	ETAL_initStatusSetState(state_datahandlerInit);

if (ETAL_datahandlerInit() != OSAL_OK)
	{
		ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Data handler initialization");
		retosal = OSAL_ERROR_DEVICE_INIT;
		goto exit;
	}

	if (ETAL_controlInit() != OSAL_OK)
		{
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Control handler initialization");
			retosal = OSAL_ERROR_DEVICE_INIT;
			goto exit;
		}

	/*
	 * In some HW configurations the CMOST provides the clock to the
	 * DCOP so it must be started before the DCOP
	 */

#ifdef CONFIG_ETAL_SUPPORT_CMOST
	/*************************
	 * state_initCommunication_CMOST
	 *
	 * initialize OS resources, reset CMOST
	 ************************/
	ETAL_initStatusSetState(state_initCommunication_CMOST);

	ret_cmost = ETAL_initCommunication_CMOST();
	if (ret_cmost != OSAL_OK)
	{
		/* set state for all tuners */
		ETAL_initStatusSetTunerStatus(ETAL_CAPA_MAX_TUNER, deviceCommunication);
		ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Unable to reset CMOST");
	}
	retosal = ret_cmost;
#endif

	if (retosal == OSAL_OK)
	{
		for (i = 0; i < ETAL_tunerGetNumberOfTuners(); i++)
		{
			tuner_index = i;

#ifdef CONFIG_ETAL_SUPPORT_CMOST

			/*************************
			 * state_initCheckSiliconVersion_CMOST
			 *
			 * CMOST silicon version check
			 *************************/
			ETAL_initStatusSetState(state_initCheckSiliconVersion_CMOST);

			ret_cmost = ETAL_initCheckSiliconVersion_CMOST(tuner_index);
			
			if (ret_cmost == OSAL_ERROR_DEVICE_NOT_OPEN)
			{
				ETAL_initStatusSetTunerStatus(tuner_index, deviceCommunication);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "CMOST communication error");
			}
			else if (ret_cmost != OSAL_OK)
			{
				ETAL_initStatusSetTunerStatus(tuner_index, deviceSiliconVersionError);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "CMOST silicon version mismatch");
				ret_cmost = OSAL_ERROR_DEVICE_NOT_OPEN;
			}
			else
			{
				/*************************
				 * state_downloadFirmware_CMOST
				 *
				 * download CMOST firmware
				 *************************/
				ETAL_initStatusSetState(state_downloadFirmware_CMOST);

				hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)tuner_index);
				if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
				{
					vl_AtLeastOnetunerIsActive = TRUE;
					
					ret_cmost = ETAL_load_CMOST(hTuner);
					if (ret_cmost == OSAL_OK)
					{
						/*************************
						 * state_initCustomParameter_CMOST
						 *
						 * download custom parameters to the Tuners
						 *************************/
						ETAL_initStatusSetState(state_initCustomParameter_CMOST);

						ret_cmost = ETAL_initProgramCustomParameter_CMOST(tuner_index);
						if (ret_cmost != OSAL_OK)
						{
							/* an error means communication failure with the Tuner
							 * or illegal parameters */
							ETAL_initStatusSetTunerStatus(tuner_index, deviceParameters);
							ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Programming custom parameters");
						}
					}
					else
					{
						ETAL_initStatusSetTunerStatus(tuner_index, deviceDownload);
						ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "CMOST download");
					}
				}
				else
				{
					ETAL_initStatusSetTunerStatus(tuner_index, deviceDisabled);
				}
			}
			retosal = ETAL_initUpdateRetval(retosal, ret_cmost);
#else
			ETAL_initStatusSetTunerStatus(tuner_index, deviceNotSupported);
#endif // CONFIG_ETAL_SUPPORT_CMOST
		}
	}

	/*************************
	 * state_initPingCmost
	 *
	 * Ping CMOST to check it is functionnal
	 *************************/
	ETAL_initStatusSetState(state_initPingCmost);

	for (i = 0; i < ETAL_tunerGetNumberOfTuners(); i++)
	{
#ifdef CONFIG_ETAL_SUPPORT_CMOST
		hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)i);
		if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
		{
			/* parameter correct by construction */
			(LINT_IGNORE_RET)ETAL_initStatusGetTunerStatus(i, &status);
			/* only ping the devices that are in good state i.e. there was no
			 * error during the firmware download */
			if (status == deviceUninitializedEntry)
			{
				if (ETAL_directCmdPing_CMOST(hTuner) != OSAL_OK)
				{
					ETAL_initStatusSetTunerStatus(i, devicePingError);
					retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
				}
				else
				{
					ETAL_initStatusSetTunerStatus(i, deviceAvailable);
				}
			}
		}
#endif
	}
		

	/* Execute board init even if the CMOST initialization
	 * failed because in multi-tuner configuration it may
	 * be that the failed CMOST does not affect the DCOP
	 */

	/*************************
	 * state_boardInit
	 *
	 * initialize board specific functions
	 *************************/

	if (TRUE == vl_AtLeastOnetunerIsActive)
	{
		/* performed here because it affects the DCOP startup */
		if (*etalBoardInitFunctionPtr != NULL)
		{
			if ((*etalBoardInitFunctionPtr)() != OSAL_OK)
			{
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Board initialization");
				/* failure here means communication error */
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
			}
		}
	}
	
	/* even if the CMOST initialization failed try to initialize the DCOP
	 * to give more detailed status */

#if defined (CONFIG_ETAL_SUPPORT_DCOP)
	if (ETAL_statusHardwareAttrIsDCOPActive())
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		/*************************
		 * state_initCommunication_MDR
		 *
		 * initialize DAB DCOP (load/flash firmware)
		 *************************/
		ETAL_initStatusSetState(state_initCommunication_MDR);

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
		/* check if a DCOP firmware is available for download */
		isBootMode = ETAL_isDoFlashOrDownloadOrDumpMDR();
#endif
		if (ETAL_initCommunication_MDR(isBootMode, TRUE) != OSAL_OK)
		{
			ETAL_initStatusSetDCOPStatus(deviceDownload);
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "DAB DCOP communication");
			retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
		}
		else if (ETAL_statusHardwareAttrIsValidDCOPAttr() == FALSE)
		{
				ETAL_initStatusSetDCOPStatus(deviceDownload);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "DAB DCOP communication invalid parameters");
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_INVALID_PARAM);
		}
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
		/* download MDR firmware and switch to normal mode */
		else if (isBootMode && (ETAL_doFlashOrDownloadOrDumpMDR() != OSAL_OK))
		{
			ETAL_initStatusSetDCOPStatus(deviceDownload);
			retosal = OSAL_ERROR;
			goto exit;
		}
#endif
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		/*************************
		 * state_initCommunication_HDRADIO
		 *
		 * initialize HDRADIO DCOP (load/flash firmware)
		 *************************/
		ETAL_initStatusSetState(state_initCommunication_HDRADIO);

#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD
		isBootMode = ETAL_isDoFlashOrDownloadOrDumpHDR();
#endif
		if (ETAL_initCommunication_HDRADIO(isBootMode, TRUE) != OSAL_OK)
		{
			ETAL_initStatusSetDCOPStatus(deviceDownload);
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication");
			retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
		}
		else if (ETAL_statusHardwareAttrIsValidDCOPAttr() == FALSE)
		{
				ETAL_initStatusSetDCOPStatus(deviceDownload);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication invalid parameters");
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_INVALID_PARAM);
		}
#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD
		else if (isBootMode == TRUE)
		{
			if (ETAL_doFlashOrDownloadOrDumpHDR() != OSAL_OK)
			{
				ETAL_initStatusSetDCOPStatus(deviceDownload);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication flash or dump fail");
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
			}
			else if (ETAL_isDoDownloadHDR() == FALSE)
			{
				if (ETAL_deinitCommunication_HDRADIO() != OSAL_OK)
				{
					ETAL_initStatusSetDCOPStatus(deviceDownload);
					ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication deinit");
					retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
				}
				else if (ETAL_initCommunication_HDRADIO(FALSE, TRUE) != OSAL_OK)
				{
					ETAL_initStatusSetDCOPStatus(deviceDownload);
					ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication init");
					retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
				}
				else
				{
					/* Nothing to do */
				}
			}
			else
			{
				/* Nothing to do */
			}
		}
#endif // #ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD
		else
		{
			/* Nothing to do */
		}
#endif
		/*************************
		 * state_initReadSiliconVersion_DCOP
		 *
		 * DCOP silicon version get
		 *************************/
		ETAL_initStatusSetState(state_initReadSiliconVersion_DCOP);

		ret_dcop = ETAL_initReadSiliconVersion_DCOP(&(hardware_attr->m_DCOPAttr));
		if (ret_dcop == OSAL_ERROR_DEVICE_NOT_OPEN)
		{
			ETAL_initStatusSetDCOPStatus(deviceCommunication);
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "DCOP communication error");
			retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
		}
		else if (ret_dcop != OSAL_OK)
		{
			ETAL_initStatusSetDCOPStatus(deviceSiliconVersionError);
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "DCOP silicon version mismatch");
			retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
		}
		else
		{
			/* Nothing to do */
		}
	}
	else
	{
		ETAL_initStatusSetDCOPStatus(deviceDisabled);
	}
#else
	ETAL_initStatusSetDCOPStatus(deviceNotSupported);
#endif // CONFIG_ETAL_SUPPORT_DCOP

	/*************************
	 * state_initThreadSpawn
	 *
	 * initialize remanining threads
	 *************************/
	ETAL_initStatusSetState(state_initThreadSpawn);

	if (ETAL_initThreadSpawn() != OSAL_OK)
	{
		ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Starting Operating System threads");
		retosal = OSAL_ERROR_DEVICE_INIT;
		goto exit;
	}

	/*************************
	 * state_initPingDcop
	 *
	 * check DCOP device communication
	 *************************/
	ETAL_initStatusSetState(state_initPingDcop);

#ifdef CONFIG_ETAL_SUPPORT_DCOP
	if (ETAL_statusHardwareAttrIsDCOPActive())
	{
		ETAL_initStatusGetDCOPStatus(&status);
		/* only ping the devices that are in good state i.e. there was no
		 * error during the firmware download */
		if (status == deviceUninitializedEntry)
		{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
			ret_dcop = ETAL_cmdPing_MDR();
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
			ret_dcop = ETAL_cmdPing_HDRADIO();
#endif
			if (ret_dcop != OSAL_OK)
			{
				ETAL_initStatusSetDCOPStatus(devicePingError);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Error pinging the DCOP");
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
			}
			else
			{
				ETAL_initStatusSetDCOPStatus(deviceAvailable);
			}
		}
	}
#endif

	if (retosal == OSAL_OK)
	{
		/*************************
		 * state_statusExternalInit
		 *
		 * external initialization (send power up commands to devices)
		 *************************/
		ETAL_initStatusSetState(state_statusExternalInit);

		if (ETAL_statusExternalInit() != OSAL_OK)
		{
			retosal = ETAL_RET_ERROR;
		}
	}

	if (retosal == OSAL_OK)
	{
		/*************************
		 * state_ETALTML_init
		 *
		 * TML initialization
		 *************************/
		ETAL_initStatusSetState(state_ETALTML_init);

#ifdef CONFIG_ETAL_HAVE_ETALTML
		if (ETALTML_init(hardware_attr, power_up) != OSAL_OK)
		{
			retosal = ETAL_RET_ERROR;
		}
#endif
	}

	// Audio interface are already correctly set in init
#if 0
	if (retosal == OSAL_OK)
	{
		/*************************
		 * state_AudioInterfaceConfigure
		 *
		 * default audio configuration
		 *************************/
		ETAL_initStatusSetState(state_AudioInterfaceConfigure);

		retosal = ETAL_AudioInterfaceConfigure();
	}
#endif

	if (retosal == OSAL_OK)
	{
		/*************************
		 * state_ETALMDR_init
		 *
		 * streamdec initialization
		 *************************/
		ETAL_initStatusSetState(state_ETALMDR_init);

#ifdef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
		if (OSAL_s32Boot(1,NULL) != OSAL_OK)
		{
			retosal = ETAL_RET_ERROR;
		}
#endif
	}

	/*************************
	 * state_initComplete
	 *
	 * complete
	 *************************/

	if (retosal == OSAL_OK)
	{
		ETAL_initStatusSetState(state_initComplete);
	}

exit:
    return retosal;
}

/***********************************
 *
 * ETAL_tuner_init
 *
 **********************************/
/*!
 * \brief		Tuner ETAL initialization function
 * \details		The function performs the following actions:
  * 				- initializes the CMOST devices (through the #ETAL_initCommunication_CMOST)
 *
 * \param[in]	deviceID - the device ID of the CMOST 
 * \param[in]	tuner_hardware_attr - describes CMOST system attributes
 * \param[in]	tunerIsAlreadyStarted - indicated if the tuner is already started : in early audio use case
 *			the tuner may have be started in advance. Setting this parameter avoids to reset/reload the tuner
 *		 	if tuner is not already started, it will be reset/fw reload... as for a normal startup
 *
 * \return		OSAL_OK
 * \return		OSAL_ERROR - generic error (parameter error or flash download error)
 * \return		OSAL_ERROR_DEVICE_INIT - Memory allocation/thread allocation/semaphore creation error
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - Device communication error
 * \callgraph
 * \callergraph
 */
tSInt ETAL_tuner_init(tU32 deviceID, const EtalTunerAttr *tuner_hardware_attr, tBool tunerIsAlreadyStarted)
{
	tSInt retosal = OSAL_OK;
	tU32 tuner_index;
	
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	tSInt ret_cmost;
	ETAL_HANDLE hTuner;
	EtalDeviceStatus status;
#ifdef CONFIG_MODULE_INTEGRATED
	ETAL_HANDLE	hFirstTuner;
#endif
#endif

	if (deviceID >= ETAL_CAPA_MAX_TUNER)
	{
		retosal = OSAL_ERROR;
		goto exit;
	}

	/*
	 * In some HW configurations the CMOST provides the clock to the
	 * DCOP so it must be started before the DCOP
	 */
	ETAL_statusHardwareAttrTunerInit(deviceID, tuner_hardware_attr);
	ETAL_initStatusSetTunerStatus(deviceID, deviceUninitializedEntry);

#ifdef CONFIG_ETAL_SUPPORT_CMOST

	// Pre check : 
	// the minimum requirement is that Tuner 1 which is consider as the master should be init prior to others
	// and ETAL should be init but this is checked in extern function
	//
	hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)deviceID);
#ifdef CONFIG_MODULE_INTEGRATED
	hFirstTuner = ETAL_tunerGetFirstTuner_CMOST();

	if (hTuner != hFirstTuner)
	{
		// we try to init a second or third tuner
		// check Tuner 1st is already activce
		if (false == ETAL_statusHardwareAttrIsTunerActive(hFirstTuner))
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Main - 1st tuner not active");
			retosal = OSAL_ERROR_DEVICE_NOT_OPEN;
		}
		else
		{
			if (false == ETAL_initStatusIsTunerStatusReadyToUse(0))
			{
				ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Main - 1st tuner not ready to use");
				retosal = OSAL_ERROR_DEVICE_NOT_OPEN;
			}
		}

		if (retosal != OSAL_OK)
		{
			ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Main - 1st tuner not active");
			goto exit;
		}
	}
#endif



	/*************************
	 * state_initCommunication_CMOST
	 *
	 * initialize OS resources, reset CMOST
	 ************************/
	ETAL_initStatusSetState(state_initCommunication_CMOST);

	// Init the CMOST information without reset : 
	// this is to enable communication, and check if already alive
	ret_cmost = ETAL_initCommunication_SingleCMOST(deviceID, FALSE);

#if 0	
	// Start by Pinging the CMOST to check if already alive
	//
	if (ETAL_directCmdPing_CMOST(hTuner) != OSAL_OK)
//	if (1)
	{
		vl_CmostAlreadyAlive = FALSE;
#endif 

	if (FALSE == tunerIsAlreadyStarted)
	{
		// Reset the CMOST if needed

		/*
		 * On the MTD board there are two CMOST connected to the
		 * same reset line, the reset_issued
		 * parameter ensures that the reset is generated only once
		 * TODO insert a configuration item
		 */
#ifdef CONFIG_MODULE_INTEGRATED
#ifndef CONFIG_COMM_CMOST_HAVE_DEDICATED_RESET_LINE

		// the reset line is shared, reset only if 1st tuner
		if (hTuner == hFirstTuner)

#endif
#endif
		{
			ret_cmost = TUNERDRIVER_reset_CMOST(deviceID);

			retosal = ret_cmost;
		}
	
	}
	else
	{
//		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL, "ETAL_tuner_init : device %d already active", deviceID);
	}	

#endif

	if (retosal == OSAL_OK)
	{
			tuner_index = deviceID;

#ifdef CONFIG_ETAL_SUPPORT_CMOST

			/*************************
			 * state_initCheckSiliconVersion_CMOST
			 *
			 * CMOST silicon version check
			 *************************/
			ETAL_initStatusSetState(state_initCheckSiliconVersion_CMOST);

			ret_cmost = ETAL_initCheckSiliconVersion_CMOST(tuner_index);
			if (ret_cmost == OSAL_ERROR_DEVICE_NOT_OPEN)
			{
				ETAL_initStatusSetTunerStatus(tuner_index, deviceCommunication);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "CMOST communication error");
			}
			else if (ret_cmost != OSAL_OK)
			{
				ETAL_initStatusSetTunerStatus(tuner_index, deviceSiliconVersionError);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "CMOST silicon version mismatch");
				ret_cmost = OSAL_ERROR_DEVICE_NOT_OPEN;
			}
			else
			{

				// check if CMOST already alive.
				// if yes, no need to reload sw
				//

				if (FALSE == tunerIsAlreadyStarted)
				{
					/*************************
					 * state_downloadFirmware_CMOST
					 *
					 * download CMOST firmware
					 *************************/
					ETAL_initStatusSetState(state_downloadFirmware_CMOST);

					hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)tuner_index);
					if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
					{
						ret_cmost = ETAL_load_CMOST(hTuner);
						if (ret_cmost == OSAL_OK)
						{
							/*************************
							 * state_initCustomParameter_CMOST
							 *
							 * download custom parameters to the Tuners
							 *************************/
							ETAL_initStatusSetState(state_initCustomParameter_CMOST);

							ret_cmost = ETAL_initProgramCustomParameter_CMOST(tuner_index);
							if (ret_cmost != OSAL_OK)
							{
								/* an error means communication failure with the Tuner
								 * or illegal parameters */
								ETAL_initStatusSetTunerStatus(tuner_index, deviceParameters);
								ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Programming custom parameters");
							}
						}
						else
						{
							ETAL_initStatusSetTunerStatus(tuner_index, deviceDownload);
							ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "CMOST download");
						}
					}
					else
					{
						ETAL_initStatusSetTunerStatus(tuner_index, deviceDisabled);
					}
				
				retosal = ETAL_initUpdateRetval(retosal, ret_cmost);
				}
#else
			ETAL_initStatusSetTunerStatus(tuner_index, deviceNotSupported);
#endif // CONFIG_ETAL_SUPPORT_CMOST
		}
	}

	/*************************
	 * state_initPingCmost
	 *
	 * Ping CMOST to check it is functionnal
	 *************************/
	ETAL_initStatusSetState(state_initPingCmost);


#ifdef CONFIG_ETAL_SUPPORT_CMOST
		hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)deviceID);

		if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
		{
			/* parameter correct by construction */
			(LINT_IGNORE_RET)ETAL_initStatusGetTunerStatus(deviceID, &status);
			/* only ping the devices that are in good state i.e. there was no
			 * error during the firmware download */
			if (status == deviceUninitializedEntry)
			{
				if (ETAL_directCmdPing_CMOST(hTuner) != OSAL_OK)
				{
					ETAL_initStatusSetTunerStatus(deviceID, devicePingError);
					retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
				}
				else
				{
					ETAL_initStatusSetTunerStatus(deviceID, deviceAvailable);
				}
			}
		}
#endif


	/*************************
	 * state_boardInit
	 *
	 * initialize board specific functions
	 *************************/
	ETAL_initStatusSetState(state_boardInit);

	// perform init
	// 
	if (FALSE == tunerIsAlreadyStarted)
	{
		/* performed here because it affects the DCOP startup */
		if (*etalBoardInitFunctionPtr != NULL)
		{
			if ((*etalBoardInitFunctionPtr)() != OSAL_OK)
			{
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Board initialization");
				/* failure here means communication error */
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
			}
		}
	}

	/*************************
	 * state_initComplete
	 *
	 * complete
	 *************************/

	if (retosal == OSAL_OK)
	{
		ETAL_initStatusSetState(state_initComplete);
	}

exit:
    return retosal;
}

/***********************************
 *
 * ETAL_DCOP_init
 *
 **********************************/
/*!
 * \brief		 ETAL initialization function for DCOP module
 * \details		The function performs the following actions:
 * 				- resets and loads the DAB DCOP firmware if present
 * 				- resets and initializes the HDRadio DCOP device
 * 				- starts the communication with the devices
 *
 * \param[in]	dcop_hardware_attr - describes some basic system attributes
 * \param[in]	InitType - indicated if the tuner is already started : in early audio use case
 *			the tuner may have be started in advance. Setting this parameter avoids to reset/reload the tuner
 *		 	if tuner is not already started, it will be reset/fw reload... as for a normal startup
 *
 * \return		OSAL_OK
 * \return		OSAL_ERROR - generic error (parameter error or flash download error)
 * \return		OSAL_ERROR_DEVICE_INIT - Memory allocation/thread allocation/semaphore creation error
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - Device communication error
 * \callgraph
 * \callergraph
 */
tSInt ETAL_Dcop_init(const EtalDCOPAttr *dcop_hardware_attr, EtalDcopInitTypeEnum InitType)
{

	tSInt retosal = OSAL_OK;
#ifdef CONFIG_ETAL_SUPPORT_DCOP
	EtalDcopBootType isBootMode = ETAL_DCOP_BOOT_REGULAR;
	EtalDeviceStatus status;
	tSInt ret_dcop;
	tBool vl_needToReset = TRUE;

	/*
	 * In some HW configurations the CMOST provides the clock to the
	 * DCOP so it must be started before the DCOP
	 */
	ETAL_statusHardwareAttrDcopInit(dcop_hardware_attr);

	vl_needToReset = (ETAL_DCOP_INIT_FULL == InitType);

	if (ETAL_statusHardwareAttrIsDCOPActive())
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

		if (ETAL_DCOP_INIT_RESET_ONLY == InitType)
		{
			ETAL_Reset_MDR();
			// all is done, end the processing
			goto exit;
		}
		else
		{
			/*************************
			 * state_initCommunication_MDR
			 *
			 * initialize DAB DCOP (load/flash firmware)
			 *************************/
			ETAL_initStatusSetState(state_initCommunication_MDR);

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
			/* check if a DCOP firmware is available for download */
			isBootMode = ETAL_isDoFlashOrDownloadOrDumpMDR();
#endif
			// Init communication MDR with Reset 
			// excpetion of Boot Mode : 
			
			if (ETAL_initCommunication_MDR(isBootMode, vl_needToReset) != OSAL_OK)
	//		if (ETAL_initCommunication_MDR(isBootMode, TRUE) != OSAL_OK)
			{
				ETAL_initStatusSetDCOPStatus(deviceDownload);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "DAB DCOP communication");
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
			}
			else if (ETAL_statusHardwareAttrIsValidDCOPAttr() == FALSE)
			{
					ETAL_initStatusSetDCOPStatus(deviceDownload);
					ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "DAB DCOP communication invalid parameters");
					retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_INVALID_PARAM);
			}
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
			/* download MDR firmware and switch to normal mode */
			else if (isBootMode && (ETAL_doFlashOrDownloadOrDumpMDR() != OSAL_OK))
			{
				ETAL_initStatusSetDCOPStatus(deviceDownload);
				retosal = OSAL_ERROR;
				goto exit;
			}

#endif //CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
		}

#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR
	
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		if (ETAL_DCOP_INIT_RESET_ONLY == InitType)
		{
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP Reset only not supported");
			retosal = OSAL_ERROR;
			// all is done, end the processing
			goto exit;
		}

	
		/*************************
		 * state_initCommunication_HDRADIO
		 *
		 * initialize HDRADIO DCOP (load/flash firmware)
			 *************************/
		ETAL_initStatusSetState(state_initCommunication_HDRADIO);
#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD
		isBootMode = ETAL_isDoFlashOrDownloadOrDumpHDR();
#endif
		if (ETAL_initCommunication_HDRADIO(isBootMode, vl_needToReset) != OSAL_OK)
		{
			ETAL_initStatusSetDCOPStatus(deviceDownload);
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication");
			retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
		}
		else if (ETAL_statusHardwareAttrIsValidDCOPAttr() == FALSE)
		{
				ETAL_initStatusSetDCOPStatus(deviceDownload);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication invalid parameters");
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_INVALID_PARAM);
		}
#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD
		else if (isBootMode == TRUE)
		{
			if (ETAL_doFlashOrDownloadOrDumpHDR() != OSAL_OK)
			{
				ETAL_initStatusSetDCOPStatus(deviceDownload);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication flash or dump fail");
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
			}
			else if (ETAL_isDoDownloadHDR() == FALSE)
			{
				if (ETAL_deinitCommunication_HDRADIO() != OSAL_OK)
				{
					ETAL_initStatusSetDCOPStatus(deviceDownload);
					ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication deinit");
					retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
				}
				else if (ETAL_initCommunication_HDRADIO(FALSE, TRUE) != OSAL_OK)
				{
					ETAL_initStatusSetDCOPStatus(deviceDownload);
					ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication init");
					retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
				}
				else
				{
					/* Nothing to do */
				}
			}
			else
			{
				/* Nothing to do */
			}
		}
#endif // #ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD
		else
		{	
			// nothing to do
		}
#endif // CONFIG_ETAL_SUPPORT_DCOP_HDRADIO


		/*************************
		 * state_initReadSiliconVersion_DCOP
		 *
		 * DCOP silicon version get
		 *************************/
		ETAL_initStatusSetState(state_initReadSiliconVersion_DCOP);

		ETAL_tracePrintSystem(TR_CLASS_APP_ETAL, "ETAL_Dcop_init ETAL_initReadSiliconVersion_DCOP");

		ret_dcop = ETAL_initReadSiliconVersion_DCOP(dcop_hardware_attr);
		if (ret_dcop == OSAL_ERROR_DEVICE_NOT_OPEN)
		{
			ETAL_initStatusSetDCOPStatus(deviceCommunication);
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "DCOP communication error");
			retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
		}
		else if (ret_dcop != OSAL_OK)
		{
			ETAL_initStatusSetDCOPStatus(deviceSiliconVersionError);
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "DCOP silicon version mismatch");
			retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
		}
		else
		{
			/* Nothing to do */
		}
	}
	else
	{
		ETAL_initStatusSetDCOPStatus(deviceDisabled);
	}

	/*************************
	 * state_initPingDcop
	 *
	 * Ping CMOST to check it is functionnal
	 *************************/
	ETAL_initStatusSetState(state_initPingDcop);

	if (ETAL_statusHardwareAttrIsDCOPActive())
	{
		ETAL_initStatusGetDCOPStatus(&status);
		/* only ping the devices that are in good state i.e. there was no
		 * error during the firmware download */
		if (status == deviceUninitializedEntry)
		{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
			ret_dcop = ETAL_cmdPing_MDR();
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
			ret_dcop = ETAL_cmdPing_HDRADIO();
#endif
			if (ret_dcop != OSAL_OK)
			{
				ETAL_initStatusSetDCOPStatus(devicePingError);
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
			}
			else
			{
				ETAL_initStatusSetDCOPStatus(deviceAvailable);
			}
		}
	}


	/*************************
	 * state_initComplete
	 *
	 * complete
	 *************************/

exit:


	if (retosal == OSAL_OK)
	{
		ETAL_initStatusSetState(state_initComplete);
	}

#else // CONFIG_ETAL_SUPPORT_DCOP
	retosal = ETAL_RET_NO_HW_MODULE;
	ETAL_initStatusSetDCOPStatus(deviceNotSupported);

#endif // CONFIG_ETAL_SUPPORT_DCOP

    return retosal;
}

#if defined(CONFIG_HOST_OS_FREERTOS)
/***********************************
 *
 * ETAL_DCOP_init_Light
 *
 **********************************/
/*!
 * \brief		 ETAL initialization function for DCOP module
 * \details		The function performs the following actions:
 * 				- resets and loads the DAB DCOP firmware if present
 * 				- resets and initializes the HDRadio DCOP device
 * 				- starts the communication with the devices
 *
 * \param[in]	dcop_hardware_attr - describes some basic system attributes
 * \param[in]	InitType - indicated if the tuner is already started : in early audio use case
 *			the tuner may have be started in advance. Setting this parameter avoids to reset/reload the tuner
 *		 	if tuner is not already started, it will be reset/fw reload... as for a normal startup
 *
 * \return		OSAL_OK
 * \return		OSAL_ERROR - generic error (parameter error or flash download error)
 * \return		OSAL_ERROR_DEVICE_INIT - Memory allocation/thread allocation/semaphore creation error
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - Device communication error
 * \callgraph
 * \callergraph
 */
tSInt ETAL_Dcop_init_Light(const EtalDCOPAttr *dcop_hardware_attr, EtalDcopInitTypeEnum InitType)
{

	tSInt retosal = OSAL_ERROR_NOT_SUPPORTED;

	/*
	 * In some HW configurations the CMOST provides the clock to the
	 * DCOP so it must be started before the DCOP
	 */
	ETAL_statusHardwareAttrDcopInit(dcop_hardware_attr);

	if (ETAL_statusHardwareAttrIsDCOPActive())
	{
		if (ETAL_DCOP_INIT_RESET_ONLY == InitType)
		{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_RESET_LIGHT_FREERTOS
			BSP_SteciSetBOOT_MDR(DAB_ACCORDO2_BOOT_GPIO, FALSE);

			BSP_DeviceReset_MDR(DAB_ACCORDO2_RESET_GPIO);
			retosal = OSAL_OK;
			// all is done, end the processing
#endif /* CONFIG_ETAL_SUPPORT_DCOP_RESET_LIGHT_FREERTOS */
		}
	}

	return retosal;

}
#endif /* CONFIG_HOST_OS_FREERTOS */

/***************************
 *
 * ETAL_resetHDQualityContainer_HDRADIO
 *
 **************************/
/*!
 * \brief		Initializes an #EtalHdQualityEntries quality container
 * \details		All fields are initialized to default value
 * \param[out]	d - pointer to the quality container
 * \callgraph
 * \callergraph
 */
tVoid ETAL_resetHDQualityContainer_HDRADIO(EtalHdQualityEntries *d)
{
	d->m_isValidDigital     = FALSE;
	d->m_QI                 = ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_CdToNo             = ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_DSQM               = ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_AudioAlignment     = FALSE;
}

/***************************
 *
 * ETAL_resetAmFmQualityContainer_CMOST
 *
 **************************/
/*!
 * \brief		Initializes an #EtalFmQualityEntries quality container
 * \details		All fields are initialized to default value
 * \param[out]	d - pointer to the quality container
 * \callgraph
 * \callergraph
 */
tVoid ETAL_resetAmFmQualityContainer_CMOST(EtalFmQualityEntries *d)
{
	d->m_RFFieldStrength 		=   -127;
	d->m_BBFieldStrength 		=   ETAL_VALUE_NOT_AVAILABLE(tS32);
	d->m_FrequencyOffset 		=   ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_Multipath 				=   ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_UltrasonicNoise		=  	ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_AdjacentChannel 		=   ETAL_VALUE_NOT_AVAILABLE(tS32);
	d->m_ModulationDetector		=   ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_SNR					=   ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_coChannel				=   ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_StereoMonoReception	=   ETAL_VALUE_NOT_AVAILABLE(tU32);
}

/***************************
 *
 * ETAL_resetDABQualityContainer_MDR
 *
 **************************/
/*!
 * \brief		Initializes an #EtalDabQualityEntries quality container
 * \details		All fields are initialized to default value
 * \param[out]	d - pointer to the quality container
 * \callgraph
 * \callergraph
 */
tVoid ETAL_resetDABQualityContainer_MDR(EtalDabQualityEntries *d)
{
	d->m_RFFieldStrength = -127;
	d->m_BBFieldStrength = ETAL_VALUE_NOT_AVAILABLE(tS32);
	d->m_FicBitErrorRatio = ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_isValidFicBitErrorRatio = false;
	d->m_MscBitErrorRatio = ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_isValidMscBitErrorRatio = false;
	d->m_audioSubChBitErrorRatio = ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_isValidAudioSubChBitErrorRatio = false;
	d->m_dataSubChBitErrorRatio = ETAL_VALUE_NOT_AVAILABLE(tU32);
	d->m_isValidDataSubChBitErrorRatio = false;
	d->m_audioBitErrorRatioLevel = (tU8)0xFF;
	d->m_reedSolomonInformation = (tU8)0x0F;
	d->m_muteFlag = (tBool)0xFF;
	d->m_syncStatus = (tU8)0xFF;
}

/***************************
 *
 * ETAL_resetQualityContainer
 *
 **************************/
/*!
 * \brief		Initializes the #EtalBcastQualityContainer quality container corresponding to the receiver
 * \details		All fields are initialized to default value
 * \param[out]	d - pointer to the broadcast quality container
 * \callgraph
 * \callergraph
 */
tVoid ETAL_resetQualityContainer(EtalBcastStandard standard, EtalBcastQualityContainer *d)
{
	if(d != NULL)
	{
		switch(standard)
		{
			case ETAL_BCAST_STD_AM:
			case ETAL_BCAST_STD_FM:
				ETAL_resetAmFmQualityContainer_CMOST(&(d->EtalQualityEntries.amfm));
				break;

			case ETAL_BCAST_STD_DAB:
				ETAL_resetDABQualityContainer_MDR(&(d->EtalQualityEntries.dab));
				break;

			case ETAL_BCAST_STD_HD_AM:
			case ETAL_BCAST_STD_HD_FM:
				ETAL_resetHDQualityContainer_HDRADIO(&(d->EtalQualityEntries.hd));
				ETAL_resetAmFmQualityContainer_CMOST(&(d->EtalQualityEntries.hd.m_analogQualityEntries));
				break;

			default:
				break;
		}
	}
	else
	{
		ASSERT_ON_DEBUGGING(0);
		ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Error in ETAL_resetQualityContainer");
	}
}


/***********************************
 *
 * ETAL_init_Light
 *
 **********************************/
/*!
 * \brief		Main ETAL initialization function
 * \details		The function performs the following actions:
 * 				- initializes OSAL (only if *power_up* is TRUE)
 * 				- sets the ETAL internal variables to a known state
 * 				- creates the ETAL locks (typically Operating System semaphores)
 * 				- initializes the CMOST devices (through the #ETAL_initCommunication_CMOST)
 * 				- resets and loads the DAB DCOP firmware if present
 * 				- resets and initializes the HDRadio DCOP device
 * 				- starts the communication with the devices
 * 				- starts all the ETAL threads (through the #ETAL_initThreadSpawn)
 *
 * \param[in]	hardware_attr - describes some basic system attributes
 * \param[in]	power_up - if TRUE the function performs some initialization
 * 				           normally required only at power up; if FALSE only
 * 				           empties the ETAL data structures, creates the
 * 				           semaphores and reloads the devices.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - generic error (parameter error or flash download error)
 * \return		OSAL_ERROR_DEVICE_INIT - Memory allocation/thread allocation/semaphore creation error
 * \return		OSAL_ERROR_DEVICE_NOT_OPEN - Device communication error
 * \callgraph
 * \callergraph
 */
tSInt ETAL_init_Light(const EtalHardwareAttr *hardware_attr, tBool power_up)
{
	tU32 i;
	tSInt retosal;
	tU32 tuner_index;
#ifdef CONFIG_ETAL_SUPPORT_DCOP
	tSInt ret_dcop;
	EtalDcopBootType isBootMode = ETAL_DCOP_BOOT_REGULAR;
#endif
#ifdef CONFIG_ETAL_SUPPORT_CMOST
	tSInt ret_cmost;
	ETAL_HANDLE hTuner;
	EtalDeviceStatus status;
#endif

	EtalHardwareAttr hardware_attr_Tmp;

#if defined (CONFIG_DEBUG_ETAL_CHECKS)
	ETAL_runtimecheck();
#endif

	/*************************************************************
	* CLE modif : 
	*  Change hardware_attr to take only first tuner into account
	*
	*
	**************************************************************/
	memcpy(&hardware_attr_Tmp,hardware_attr, sizeof(EtalHardwareAttr));

	hardware_attr_Tmp.m_tunerAttr[1].m_isDisabled = TRUE;	/* Disable 2nd Tuner if existing */
	hardware_attr_Tmp.m_DCOPAttr.m_isDisabled = TRUE;		/* Deactivate DCOP */


	/*************************
	 * state_OSALINIT
	 *
	 * initialize OSAL
	 ************************/
	ETAL_initStatusSetState(state_OSALINIT);

	if (power_up == TRUE)
	{
		if (TUNERDRIVER_system_init() != OSAL_OK)
		{
			retosal = OSAL_ERROR_DEVICE_INIT;
			goto exit;
		}
   }

	/* initialize internal structures (only MemorySet) */

	ETAL_statusInternalInit(&hardware_attr_Tmp);
	ETAL_receiverInit();
	ETAL_intCbInit();

	/*************************
	 * state_tracePrintInit
	 *
	 * initialize trace and log system
	 ************************/
	ETAL_initStatusSetState(state_tracePrintInit);

	retosal = ETAL_tracePrintInit();
	if (retosal == OSAL_ERROR_INVALID_PARAM)
	{
		ETAL_initStatusSetNonFatal(warningTraceDefault);
	}
	else if (retosal != OSAL_OK)
	{
		ETAL_initStatusSetNonFatal(warningNoTrace);
	}
	else
	{
		/* Nothing to do */
	}
	/* tracePrintf errors are non fatal, ignore them
	 * and overwrite retosal
	 */
	retosal = OSAL_OK;

	/***********************************************************************/
	/* if using CONFIG_TRACE_ASYNC printfs not available before this point */
	/***********************************************************************/

	/*************************
	 * state_statusInitLock
	 *
	 * initialize global locks
	 ************************/
	ETAL_initStatusSetState(state_statusInitLock);

	if (ETAL_statusInitLock(power_up) != OSAL_OK)
	{
		ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Status semaphore creation");
		retosal = OSAL_ERROR_DEVICE_INIT;
		goto exit;
	}

	/*************************
	 * state_callbackInit
	 *
	 * initialize callback handler
	 ************************/
	ETAL_initStatusSetState(state_callbackInit);

	/*************************
	 * state_datahandlerInit
	 *
	 * initialize data handler
	 ************************/
	ETAL_initStatusSetState(state_datahandlerInit);

	/*
	 * In some HW configurations the CMOST provides the clock to the
	 * DCOP so it must be started before the DCOP
	 */

#ifdef CONFIG_ETAL_SUPPORT_CMOST
	/*************************
	 * state_initCommunication_CMOST
	 *
	 * initialize OS resources, reset CMOST
	 ************************/
	ETAL_initStatusSetState(state_initCommunication_CMOST);

	ret_cmost = ETAL_initCommunication_CMOST();
	if (ret_cmost != OSAL_OK)
	{
		/* set state for all tuners */
		ETAL_initStatusSetTunerStatus(ETAL_CAPA_MAX_TUNER, deviceCommunication);
		ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Unable to reset CMOST");
	}
	retosal = ret_cmost;
#endif

	if (retosal == OSAL_OK)
	{
		for (i = 0; i < ETAL_tunerGetNumberOfTuners(); i++)
		{
			tuner_index = i;

#ifdef CONFIG_ETAL_SUPPORT_CMOST

			/*************************
			 * state_initCheckSiliconVersion_CMOST
			 *
			 * CMOST silicon version check
			 *************************/
			ETAL_initStatusSetState(state_initCheckSiliconVersion_CMOST);

			ret_cmost = ETAL_initCheckSiliconVersion_CMOST(tuner_index);
			if (ret_cmost == OSAL_ERROR_DEVICE_NOT_OPEN)
			{
				ETAL_initStatusSetTunerStatus(tuner_index, deviceCommunication);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "CMOST communication error");
			}
			else if (ret_cmost != OSAL_OK)
			{
				ETAL_initStatusSetTunerStatus(tuner_index, deviceSiliconVersionError);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "CMOST silicon version mismatch");
				ret_cmost = OSAL_ERROR_DEVICE_NOT_OPEN;
			}
			else
			{
				/*************************
				 * state_downloadFirmware_CMOST
				 *
				 * download CMOST firmware
				 *************************/
				ETAL_initStatusSetState(state_downloadFirmware_CMOST);

				hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)tuner_index);
				if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
				{
					ret_cmost = ETAL_load_CMOST(hTuner);
					if (ret_cmost == OSAL_OK)
					{
						/*************************
						 * state_initCustomParameter_CMOST
						 *
						 * download custom parameters to the Tuners
						 *************************/
						ETAL_initStatusSetState(state_initCustomParameter_CMOST);

						ret_cmost = ETAL_initProgramCustomParameter_CMOST(tuner_index);
						if (ret_cmost != OSAL_OK)
						{
							/* an error means communication failure with the Tuner
							 * or illegal parameters */
							ETAL_initStatusSetTunerStatus(tuner_index, deviceParameters);
							ETAL_tracePrintError(TR_CLASS_APP_ETAL, "Programming custom parameters");
						}
					}
					else
					{
						ETAL_initStatusSetTunerStatus(tuner_index, deviceDownload);
						ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "CMOST download");
					}
				}
				else
				{
					ETAL_initStatusSetTunerStatus(tuner_index, deviceDisabled);
				}
			}
			retosal = ETAL_initUpdateRetval(retosal, ret_cmost);
#else
			ETAL_initStatusSetTunerStatus(tuner_index, deviceNotSupported);
#endif // CONFIG_ETAL_SUPPORT_CMOST
		}
	}

	/*************************
	 * state_initPingCmost
	 *
	 * Ping CMOST to check it is functionnal
	 *************************/
	ETAL_initStatusSetState(state_initPingCmost);

	for (i = 0; i < ETAL_tunerGetNumberOfTuners(); i++)
	{
#ifdef CONFIG_ETAL_SUPPORT_CMOST
		hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)i);
		if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
		{
			/* parameter correct by construction */
			(LINT_IGNORE_RET)ETAL_initStatusGetTunerStatus(i, &status);
			/* only ping the devices that are in good state i.e. there was no
			 * error during the firmware download */
			if (status == deviceUninitializedEntry)
			{
				if (ETAL_directCmdPing_CMOST(hTuner) != OSAL_OK)
				{
					ETAL_initStatusSetTunerStatus(i, devicePingError);
					retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
				}
				else
				{
					ETAL_initStatusSetTunerStatus(i, deviceAvailable);
				}
			}
		}
#endif
	}

	/* Execute board init even if the CMOST initialization
	 * failed because in multi-tuner configuration it may
	 * be that the failed CMOST does not affect the DCOP
	 */

	/*************************
	 * state_boardInit
	 *
	 * initialize board specific functions
	 *************************/
	ETAL_initStatusSetState(state_boardInit);

	/* performed here because it affects the DCOP startup */
	if (*etalBoardInitFunctionPtr != NULL)
	{
		if ((*etalBoardInitFunctionPtr)() != OSAL_OK)
		{
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "Board initialization");
			/* failure here means communication error */
			retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
		}
	}

	/* even if the CMOST initialization failed try to initialize the DCOP
	 * to give more detailed status */

#if defined (CONFIG_ETAL_SUPPORT_DCOP)

	if (ETAL_statusHardwareAttrIsDCOPActive())
	{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
		/*************************
		 * state_initCommunication_MDR
		 *
		 * initialize DAB DCOP (load/flash firmware)
		 *************************/
		ETAL_initStatusSetState(state_initCommunication_MDR);

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
		/* check if a DCOP firmware is available for download */
		isBootMode = ETAL_isDoFlashOrDownloadOrDumpMDR();
#endif
		if (ETAL_initCommunication_MDR(isBootMode, TRUE) != OSAL_OK)
		{
			ETAL_initStatusSetDCOPStatus(deviceDownload);
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "DAB DCOP communication");
			retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
		}
		else if (ETAL_statusHardwareAttrIsValidDCOPAttr() == FALSE)
		{
				ETAL_initStatusSetDCOPStatus(deviceDownload);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "DAB DCOP communication invalid parameters");
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_INVALID_PARAM);
		}
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
		/* download MDR firmware and switch to normal mode */
		else if (isBootMode && (ETAL_doFlashOrDownloadOrDumpMDR() != OSAL_OK))
		{
			ETAL_initStatusSetDCOPStatus(deviceDownload);
			retosal = OSAL_ERROR;
			goto exit;
		}
#endif
#endif // CONFIG_ETAL_SUPPORT_DCOP_MDR

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
		/*************************
		 * state_initCommunication_HDRADIO
		 *
		 * initialize HDRADIO DCOP (load/flash firmware)
		 *************************/
		ETAL_initStatusSetState(state_initCommunication_HDRADIO);

#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD
		isBootMode = ETAL_isDoFlashOrDownloadOrDumpHDR();
#endif
		if (ETAL_initCommunication_HDRADIO(isBootMode, TRUE) != OSAL_OK)
		{
			ETAL_initStatusSetDCOPStatus(deviceDownload);
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication");
			retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
		}
		else if (ETAL_statusHardwareAttrIsValidDCOPAttr() == FALSE)
		{
				ETAL_initStatusSetDCOPStatus(deviceDownload);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication invalid parameters");
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_INVALID_PARAM);
		}
#ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD
		else if (isBootMode == TRUE)
		{
			if (ETAL_doFlashOrDownloadOrDumpHDR() != OSAL_OK)
			{
				ETAL_initStatusSetDCOPStatus(deviceDownload);
				ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication flash or dump fail");
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
			}
			else if (ETAL_isDoDownloadHDR() == FALSE)
			{
				if (ETAL_deinitCommunication_HDRADIO() != OSAL_OK)
				{
					ETAL_initStatusSetDCOPStatus(deviceDownload);
					ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication deinit");
					retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
				}
				else if (ETAL_initCommunication_HDRADIO(FALSE, TRUE) != OSAL_OK)
				{
					ETAL_initStatusSetDCOPStatus(deviceDownload);
					ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "HDRADIO DCOP communication init");
					retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
				}
				else
				{
					/* Nothing to do */
				}
			}
			else
			{
				/* Nothing to do */
			}
		}
#endif // #ifdef CONFIG_COMM_DCOP_HDRADIO_FIRMWARE_DOWNLOAD
		else
		{
			/* Nothing to do */
		}
#endif
		/*************************
		 * state_initReadSiliconVersion_DCOP
		 *
		 * DCOP silicon version get
		 *************************/
		ETAL_initStatusSetState(state_initReadSiliconVersion_DCOP);

		ret_dcop = ETAL_initReadSiliconVersion_DCOP(&(hardware_attr->m_DCOPAttr));
		if (ret_dcop == OSAL_ERROR_DEVICE_NOT_OPEN)
		{
			ETAL_initStatusSetDCOPStatus(deviceCommunication);
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "DCOP communication error");
			retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
		}
		else if (ret_dcop != OSAL_OK)
		{
			ETAL_initStatusSetDCOPStatus(deviceSiliconVersionError);
			ETAL_tracePrintFatal(TR_CLASS_APP_ETAL, "DCOP silicon version mismatch");
			retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
		}
		else
		{
			/* Nothing to do */
		}
	}
	else
	{
		ETAL_initStatusSetDCOPStatus(deviceDisabled);
	}
#else
	ETAL_initStatusSetDCOPStatus(deviceNotSupported);
#endif // CONFIG_ETAL_SUPPORT_DCOP

	/*************************
	 * state_initThreadSpawn
	 *
	 * initialize remanining threads
	 *************************/
	ETAL_initStatusSetState(state_initThreadSpawn);

	/*************************
	 * state_initPingDcop
	 *
	 * check DCOP device communication
	 *************************/
	ETAL_initStatusSetState(state_initPingDcop);

#ifdef CONFIG_ETAL_SUPPORT_DCOP
	if (ETAL_statusHardwareAttrIsDCOPActive())
	{
		ETAL_tracePrintUser1(TR_CLASS_APP_ETAL, "CLE - Ping DCOP");

		ETAL_initStatusGetDCOPStatus(&status);
		/* only ping the devices that are in good state i.e. there was no
		 * error during the firmware download */
		if (status == deviceUninitializedEntry)
		{
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
			ret_dcop = ETAL_cmdPing_MDR();
#endif
#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
			ret_dcop = ETAL_cmdPing_HDRADIO();
#endif
			if (ret_dcop != OSAL_OK)
			{
				ETAL_initStatusSetDCOPStatus(devicePingError);
				retosal = ETAL_initUpdateRetval(retosal, OSAL_ERROR_DEVICE_NOT_OPEN);
			}
			else
			{
				ETAL_initStatusSetDCOPStatus(deviceAvailable);
			}
		}
	}
#endif

	if (retosal == OSAL_OK)
	{
		/*************************
		 * state_statusExternalInit
		 *
		 * external initialization (send power up commands to devices)
		 *************************/
		ETAL_initStatusSetState(state_statusExternalInit);

		ETAL_tracePrintUser1(TR_CLASS_APP_ETAL, "CLE - send power up commands to devices %d");

		if (ETAL_statusExternalInit() != OSAL_OK)
		{
			retosal = ETAL_RET_ERROR;
		}
	}

	if (retosal == OSAL_OK)
	{
		/*************************
		 * state_ETALTML_init
		 *
		 * TML initialization
		 *************************/
		ETAL_initStatusSetState(state_ETALTML_init);

	}
#if 0
	if (retosal == OSAL_OK)
	{
		/*************************
		 * state_AudioInterfaceConfigure
		 *
		 * default audio configuration
		 *************************/
		ETAL_initStatusSetState(state_AudioInterfaceConfigure);

		ETAL_tracePrintUser1(TR_CLASS_APP_ETAL, "CLE - Audio ITF configuration");
		retosal = ETAL_AudioInterfaceConfigure();
	}
#endif

	if (retosal == OSAL_OK)
	{
		/*************************
		 * state_ETALMDR_init
		 *
		 * streamdec initialization
		 *************************/
		ETAL_initStatusSetState(state_ETALMDR_init);

#ifdef CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
		if (OSAL_s32Boot(1,NULL) != OSAL_OK)
		{
			retosal = ETAL_RET_ERROR;
		}
#endif
	}

	/*************************
	 * state_initComplete
	 *
	 * complete
	 *************************/

	if (retosal == OSAL_OK)
	{
		ETAL_initStatusSetState(state_initComplete);
	}

	ETAL_tracePrintUser1(TR_CLASS_APP_ETAL, "CLE - INITIALIZATION FINISHED");
	
exit:
    return retosal;
}
/***********************************
 *
 * ETAL_deinit_Light
 *
 **********************************/
/*!
 * \brief		Deinit function for early tuner use case
 * \details		The function performs the following actions:
 * 				- deinit tunerdriver
 * \callgraph
 * \callergraph
 */
tVoid ETAL_deinit_Light(tVoid)
{
	tU32 tuner_index = 0;
	ETAL_HANDLE hTuner;

	for (tuner_index = 0; tuner_index < ETAL_tunerGetNumberOfTuners(); tuner_index++)
	{
		hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)tuner_index);
		if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
		{
			TUNERDRIVER_deinit(tuner_index);
		}
	}
}
