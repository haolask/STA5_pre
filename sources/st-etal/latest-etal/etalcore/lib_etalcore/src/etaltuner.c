//!
//!  \file 		etaltuner.c
//!  \brief 	<i><b> ETAL Tuner management </b></i>
//!  \details   Tuner is the ETAL abstraction to describe the hardware devices
//!				embedding one or more channel decoders, capable of complete
//!				demodulation (e.g. CMOST STAR) or just the base band (e.g. CMOST DOT).
//!				This file includes the functions used to access the etalTuner array
//!				which describes the Tuners present in the system.
//!
//!				These functions (apart from the initialization) are all read-only
//!				so do not require lock mechanism.
//!				The lock mechanism is instead used to control access to the hardware
//!				tuner.
//!
//!				The etalTuner array is defined in one of the etalconfig_*.c files.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

#include "osal.h"
#include "etalinternal.h"


/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
/*!
 * \def		ETAL_TUNER_SEM_NAME_MAX
 * 			Max length of the name of semaphores used for Tuner lock
 */
#define ETAL_TUNER_SEM_NAME_MAX  16

/*****************************************************************
| Local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/
/*!
 * \var		etalTunerSem
 * 			Semaphores used to avoid concurrent accesses to the Tuner
 */
static OSAL_tSemHandle etalTunerSem[ETAL_CAPA_MAX_TUNER];

/*****************************************************************
| static functions
|----------------------------------------------------------------*/
#if defined(CONFIG_BOARD_ACCORDO5) && defined(CONFIG_HOST_OS_LINUX)
static tSInt ETAL_getI2cBusName(tChar *aO_i2cDeviceName);
#endif

/***************************
 *
 * ETAL_IRQCallbackFunction_CMOST_TUNER_ID_0
 *
 **************************/
/*!
 * \brief		Callback function when IRQ occurs in CMOST_0
 * \details		This function is called when IRQ occurs.
 * \callgraph
 * \callergraph
 */
tVoid ETAL_IRQCallbackFunction_CMOST_TUNER_ID_0(tVoid)
{
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "ETAL_IRQCallbackFunction_CMOST_TUNER_ID_0 called !");
	
#if defined(CONFIG_HOST_OS_TKERNEL) && defined(CONFIG_COMM_ENABLE_RDS_IRQ)
	ETAL_IRQ_EntryTuner1();
#endif
	return;
}

/***************************
 *
 * ETAL_IRQCallbackFunction_CMOST_TUNER_ID_1
 *
 **************************/
/*!
 * \brief		Callback function when IRQ occurs in CMOST_1
 * \details		This function is called when IRQ occurs.
 * \callgraph
 * \callergraph
 */
tVoid ETAL_IRQCallbackFunction_CMOST_TUNER_ID_1(tVoid)
{
	ETAL_tracePrintComponent(TR_CLASS_APP_ETAL_COMM, "ETAL_IRQCallbackFunction_CMOST_TUNER_ID_1 called !");

	
//	ETAL_IRQ_EntryTuner2();
	return;
}

/***************************
 *
 * ETAL_tunerIsValidHandle
 *
 **************************/
/*!
 * \brief		Checks if the Tuner handle is valid
 * \details		Checks only if the handle is within allowed bounds.
 * \param[in]	hTuner - Tuner handle
 * \return		TRUE  - the handle is valid
 * \return		FALSE - the handle is not valid
 * \callgraph
 * \callergraph
 */
tBool ETAL_tunerIsValidHandle(ETAL_HANDLE hTuner)
{
	ETAL_HINDEX tuner_index;
	tBool ret = TRUE;

	if (!ETAL_handleIsTuner(hTuner))
	{
#if defined(CONFIG_HOST_OS_TKERNEL) && defined (CONFIG_DEBUG_OSAL)
		/* T-Kernel does not behave as Linux with the ASSERT: it does not
		 * stop execution unless a breakpoint is set;
		 * add the printf for debugging but avoid including it in the final build */
		ETAL_tracePrintError(TR_CLASS_APP_ETAL, "ETAL_tunerIsValidHandle : invalid handle hTuner = %d", hTuner);
#endif
		ASSERT_ON_DEBUGGING(0);
		ret = FALSE;
	}
	else
	{
		tuner_index = ETAL_handleTunerGetIndex(hTuner);
		if ((tuner_index == ETAL_INVALID_HINDEX) ||
			((tU32)tuner_index >= ETAL_CAPA_MAX_TUNER))
		{
			ASSERT_ON_DEBUGGING(0);
			ret = FALSE;
		}
	}
	return ret;
}

/***************************
 *
 * ETAL_tunerInitLock
 *
 **************************/
/*!
 * \brief		Initializes the semaphores used to lock the Tuners
 * \return		OSAL_OK    - no error
 * \return		OSAL_ERROR - some semaphores could not be created. This is a FATAL error.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_tunerInitLock(tVoid)
{
	tU32 i;
	tChar sem_name[ETAL_TUNER_SEM_NAME_MAX];
	tSInt ret = OSAL_OK;
	
	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		if (OSAL_s32NPrintFormat(sem_name, ETAL_TUNER_SEM_NAME_MAX, "%s%.2u", ETAL_SEM_TUNER_BASE, i) < 0)
		{
			ret = OSAL_ERROR;
			goto exit;
		}
		if (OSAL_s32SemaphoreCreate(sem_name, &etalTunerSem[i], 1) == OSAL_ERROR)
		{
			ret = OSAL_ERROR;
			goto exit;
		}
	}

exit:
	return ret;
}

/***************************
 *
 * ETAL_tunerDeinitLock
 *
 **************************/
/*!
 * \brief		Destroys the semaphores used to lock the Tuners
 * \remark		It is not normally required to invoke this function.
 * \return		OSAL_OK    - no error
 * \return		OSAL_ERROR - some semaphores could not be destroyed.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_tunerDeinitLock(tVoid)
{
	tU32 i;
	tChar sem_name[ETAL_TUNER_SEM_NAME_MAX];
	tSInt ret = OSAL_OK;

	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		if (OSAL_s32NPrintFormat(sem_name, ETAL_TUNER_SEM_NAME_MAX, "%s%.2u", ETAL_SEM_TUNER_BASE, i) < 0)
		{
			ret = OSAL_ERROR;
			goto exit;
		}
		if (OSAL_s32SemaphoreClose(etalTunerSem[i]) == OSAL_ERROR)
		{
			ret = OSAL_ERROR;
		}
		else if (OSAL_s32SemaphoreDelete(sem_name) == OSAL_ERROR)
		{
			ret = OSAL_ERROR;
		}
		else
		{
			/* Nothing to do */
		}
	}
exit:
	return ret;
}

/***************************
 *
 * ETAL_tunerGetLock
 *
 **************************/
/*!
 * \brief		Locks a Tuner
 * \param[in]	hTuner - Tuner handle
 * \return		ETAL_RET_SUCCESS - the Tuner is locked
 * \return		ETAL_RET_ERROR   - error accessing the semaphore. The Tuner is not locked. This is a FATAL error.
 * \return		ETAL_RET_INVALID_HANDLE - invalid Tuner handle
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_tunerGetLock(ETAL_HANDLE hTuner)
{
	ETAL_HINDEX tuner_index;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	if (!ETAL_tunerIsValidHandle(hTuner))
	{
		ASSERT_ON_DEBUGGING(0);
		ret = ETAL_RET_INVALID_HANDLE;
	}
	else
	{
		tuner_index = ETAL_handleTunerGetIndex(hTuner);
		if (OSAL_s32SemaphoreWait(etalTunerSem[tuner_index], OSAL_C_TIMEOUT_FOREVER) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
	}
	return ret;
}

/***************************
 *
 * ETAL_tunerReleaseLock
 *
 **************************/
/*!
 * \brief		Unlocks a Tuner
 * \param[in]	hTuner - Tuner handle
 * \callgraph
 * \callergraph
 */
tVoid ETAL_tunerReleaseLock(ETAL_HANDLE hTuner)
{
	ETAL_HINDEX tuner_index;

	tuner_index = ETAL_handleTunerGetIndex(hTuner);
	(void)OSAL_s32SemaphorePost(etalTunerSem[tuner_index]);
}

/***************************
 *
 * ETAL_tunerInitRDSSlot
 *
 **************************/
/*!
 * \brief		Initializes a unique identifier for each Tuner
 * \details		The RDS Slot is needed by the RDS code inherited from MDR3 code base. From the ETAL point
 * 				of view it just needs to be a unique identifier across all Tuners and Tuner channels capable
 * 				of RDS decoding.
 * \callgraph
 * \callergraph
 */
tVoid ETAL_tunerInitRDSSlot(tVoid)
{
	etalFrontendDescTy *fe_list;
	tU32 i, fe_index, slot;

	slot = 0;
	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		fe_list = etalTuner[i].frontEndList;
		for (fe_index = 0; fe_index < ETAL_CAPA_MAX_FRONTEND_PER_TUNER; fe_index++)
		{
			if ((fe_list[fe_index].dataTypes & ETAL_DATA_TYPE_FM_RDS) == ETAL_DATA_TYPE_FM_RDS)
			{
				fe_list[fe_index].RDSSlotIndex = (tU8)slot++; /* the cast is validated by the ASSERT below */
			}
		}
	}
	ASSERT_ON_DEBUGGING(slot <= ETAL_CAPA_MAX_FRONTEND);
}

/***************************
 *
 * ETAL_tunerGetNumberOfTuners
 *
 **************************/
/*!
 * \brief		Returns from the board description array the number of tuner devices
 * \details		The etalTuner array is defined at system build time
 * 				with the description of each Tuner capabilities.
 * 				This function returns the number of tuners present
 * 				in the etalTuner array.
 * \return		The number of tuners
 * \callgraph
 * \callergraph
 */
tU32 ETAL_tunerGetNumberOfTuners(tVoid)
{
	tU32 i, count;

	count = 0;
	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		if (etalTuner[i].deviceDescr.m_deviceType != deviceUnknown)
		{
			count++;
		}
	}

	return count;
}

/***************************
 *
 * ETAL_tunerGetFEsupportedStandards
 *
 **************************/
/*!
 * \brief		Return the list of Broadcast Standards supported by a Frontend
 * \details		Broadcast Standards for a Tuner are configured at build-time
 * 				By specifying them as a bitmap in the etalTuner field. The
 * 				bitmap contains values of type #EtalBcastStandard. This function
 * 				returns that bitmap.
 * \param[in]	hFrontend - Frontend handle
 * \return		A bitmap of #EtalBcastStandard values describing the supported Standards
 * \return		0 if the Frontend is invalid
 * \see			The etalconfig_*.c files for the Tuner description.
 * \callgraph
 * \callergraph
 */
tU32 ETAL_tunerGetFEsupportedStandards(ETAL_HANDLE hFrontend)
{
	ETAL_HINDEX tuner_index;
	ETAL_HINDEX channel_index;
	tU32 ret;

	if (ETAL_tunerSearchFrontend(hFrontend) != OSAL_OK)
	{
		ret = 0;
	}
	else
	{
		tuner_index = ETAL_handleFrontendGetTunerIndex(hFrontend);
		channel_index = ETAL_handleFrontendGetChannel(hFrontend);
		ret = etalTuner[tuner_index].frontEndList[channel_index].standards;
	}
	return ret;
}

/***************************
 *
 * ETAL_tunerGetFEsupportedDataTypes
 *
 **************************/
/*!
 * \brief		Returns the list of supported Data Types
 * \details		ETAL uses the Data Type abstraction to describe the data processing
 * 				capabilities of a Tuner. The list of supported Data Types is
 * 				specified at build-time as a bitmap of #EtalBcastDataType values.
 * 				This function returns that bitmap.
 * \param[in]	hFrontend - Frontend handle
 * \return		A bitmap of #EtalBcastDataType values describing the supported Data Types
 * \return		0 if the Tuner or Frontend is invalid
 * \see			The etalconfig_*.c files for the Tuner description.
 * \callgraph
 * \callergraph
 */
tU32 ETAL_tunerGetFEsupportedDataTypes(ETAL_HANDLE hFrontend)
{
	ETAL_HINDEX tuner_index;
	ETAL_HINDEX channel_index;
	tU32 ret;

	if (ETAL_tunerSearchFrontend(hFrontend) != OSAL_OK)
	{
		ret = 0;
	}
	else
	{
		tuner_index = ETAL_handleFrontendGetTunerIndex(hFrontend);
		channel_index = ETAL_handleFrontendGetChannel(hFrontend);
		ret = etalTuner[tuner_index].frontEndList[channel_index].dataTypes;
	}
	return ret;
}

/***************************
 *
 * ETAL_tunerGetChannelSupportedDataTypes
 *
 **************************/
/*!
 * \brief		Returns the list of supported Data Types
 * \details		ETAL uses the Data Type abstraction to describe the data processing
 * 				capabilities of a Tuner. The list of supported Data Types is
 * 				specified at build-time as a bitmap of #EtalBcastDataType values.
 * 				This function returns that bitmap.
 * 				If channel is 
 * \param[in]	hTuner  - Tuner handle
 * \param[in]	channel - channel identifier
 * \return		A bitmap of #EtalBcastDataType values describing the supported Data Types
 * \return		0 if the Tuner or Frontend is invalid
 * \see			The etalconfig_*.c files for the Tuner description.
 * \callgraph
 * \callergraph
 */
tU32 ETAL_tunerGetChannelSupportedDataTypes(ETAL_HANDLE hTuner, etalChannelTy channel)
{
	ETAL_HINDEX tuner_index;
	tU32 fe_index = 0;
	tBool valid = TRUE;
	tU32 ret;

	if (!ETAL_tunerIsValidHandle(hTuner))
	{
		ret = 0;
		goto exit;
	}

	tuner_index = ETAL_handleTunerGetIndex(hTuner);
	switch (channel)
	{
		case ETAL_CHN_UNDEF:
			valid = FALSE;
			break;

		case ETAL_CHN_FOREGROUND:
			fe_index = 0;
			break;

		case ETAL_CHN_BACKGROUND:
		case ETAL_CHN_BOTH:
			if (etalTuner[tuner_index].deviceDescr.m_channels < (tU8)2)
			{
				valid = FALSE;
			}
			else if (channel == ETAL_CHN_BACKGROUND)
			{
				fe_index = 1;
			}
			else
			{
				/* if ETAL_CHN_BOTH is selected, return the foreground channel info */
				fe_index = 0;
			}
			break;
		
	}
	if (valid)
	{
		ret = etalTuner[tuner_index].frontEndList[fe_index].dataTypes;
	}
	else
	{
		ret = 0;
	}

exit:
	return ret;
}

/***************************
 *
 * ETAL_tunerGetAddress
 *
 **************************/
/*!
 * \brief		Returns the hardware address of a Tuner
 * \details		CMOST Tuners support communication to the Host controller via I2C or
 * 				SPI buses and may respond to different bus addresses. Both configurations depend
 * 				on the hardware set up and are specified at ETAL build-time.
 * \param[in]	hTuner  - Tuner handle
 * \param[out]	address - pointer to a location where the function stores the hardware address of the device.
 * 				          Note that this is the hardware bus address, not a memory address.
 * \param[out]	useI2C  - pointer to a location where the function stores a Boolean set to TRUE if the
 * 				          device is connected to a I2C bus, FALSE if it is an SPI bus.
 * \return		OSAL_OK    - no errors
 * \return		OSAL_ERROR - invalid Tuner handle, *address* and *useI2C* are unchanged
 * \see			The etalconfig_*.c files for the Tuner description.
 * \callgraph
 * \callergraph
 */
tSInt ETAL_tunerGetAddress(ETAL_HANDLE hTuner, EtalDeviceDesc *deviceDescription)
{
	ETAL_HINDEX tuner_index;
	tSInt ret;

	if (!ETAL_tunerIsValidHandle(hTuner))
	{
		ret = OSAL_ERROR;
	}
	else
	{
		tuner_index = ETAL_handleTunerGetIndex(hTuner);
		*deviceDescription = etalTuner[tuner_index].deviceDescr;
		ret = OSAL_OK;
	}
	return ret;
}


#if defined(CONFIG_BOARD_ACCORDO5) && defined(CONFIG_HOST_OS_LINUX)
/***************************
 *
 * ETAL_getI2cBusName
 *
 **************************/
/*!
 * \brief		Returns the i2c device name from its adress configured
 * \details		CMOST Tuners support communication to the Host Controller via I2C or
 * \param[in]	 none
 * \param[out]	deviceName
 * \return		OSAL_OK    - no errors
 * \return		OSAL_ERROR - invalid Tuner handle, *address* and *useI2C* are unchanged
 * \callgraph
 * \callergraph
 */
tSInt ETAL_getI2cBusName(tChar *aO_i2cDeviceName)
{
  FILE *fp;
  tSInt ret = OSAL_OK;
  tSInt vl_count = 0;
#define BSP_BUS_CONFIG_TIME_TO_WAIT_I2C_DEVICE_IN_MS			5000
#define BSP_BUS_CONFIG_WAITING_TIME_POLLING_I2C_DEVICE_IN_MS	100
#define BSP_BUS_CONFIG_WAITING_COUNT_POLLING					(BSP_BUS_CONFIG_TIME_TO_WAIT_I2C_DEVICE_IN_MS/BSP_BUS_CONFIG_WAITING_TIME_POLLING_I2C_DEVICE_IN_MS)
#define I2C_BUS_NAME_LEN		6
  tChar v_systemCmd[100];

	sprintf(v_systemCmd, "i2cdetect -l | grep 0x%x", CMOST_ACCORDO5_I2C_DEVICE_BASE_ADDRESS);
	
	do 
  	{
		/* Open the command for reading. */
	  	fp = popen(v_systemCmd, "r");
		
	  	if (fp == NULL)
	  	{
			printf("Failed to run command\n" );
			ret = OSAL_ERROR;
	   		break;
	  	}

	 	/* Read the output a line at a time - output it. */
	 	if (fgets(aO_i2cDeviceName, I2C_BUS_NAME_LEN, fp) != NULL)
	  	{
		  	// line read
		    ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "ETAL_getI2cBusName = name %s\n", aO_i2cDeviceName);
			ret = OSAL_OK;;
		}
		else
		{
		  	// no device
		 	ret = OSAL_ERROR;
		}
		/* close */
		pclose(fp);

		vl_count++;
		OSAL_s32ThreadWait(BSP_BUS_CONFIG_WAITING_TIME_POLLING_I2C_DEVICE_IN_MS);
  	}while ((ret != OSAL_OK) && (vl_count < BSP_BUS_CONFIG_WAITING_COUNT_POLLING));

	
	
  return ret;
}
#endif // ACCORDO_5 && LINUX

/***************************
 *
 * ETAL_getDeviceConfig_CMOST
 *
 **************************/
/*!
 * \brief		Returns the device configuration of a Tuner
 * \details		CMOST Tuners support communication to the Host Controller via I2C or
 * 				SPI buses. Configurations depend on how CMOST device have been configured during initialisation phase.
 * \param[in]	deviceID  - device identifier
 * \param[out]	deviceConfiguration - pointer to the device configuration.
 * \return		OSAL_OK    - no errors
 * \return		OSAL_ERROR - invalid Tuner handle, *address* and *useI2C* are unchanged
 * \callgraph
 * \callergraph
 */
tSInt ETAL_getDeviceConfig_CMOST(ETAL_HANDLE hTuner, tyCMOSTDeviceConfiguration *deviceConfiguration)
{
	ETAL_HINDEX tuner_index;
	tSInt vl_res = OSAL_OK;
#if defined(CONFIG_BOARD_ACCORDO5) && defined(CONFIG_HOST_OS_LINUX)
	// dynamic detection of device name from address in I2C
	tChar al_i2cDeviceName[MAX_SIZE_BUS_NAME];
#endif
	
	if (!ETAL_tunerIsValidHandle(hTuner))
	{
		vl_res = OSAL_ERROR;
		goto exit;
	}
	tuner_index = ETAL_handleTunerGetIndex(hTuner);

	if(tuner_index == ETAL_CMOST_TUNER_1_ID)
	{
		deviceConfiguration->GPIO_RESET = CMOST_T1_ACCORDO2_RESET_GPIO;
		deviceConfiguration->GPIO_IRQ = CMOST_T1_ACCORDO2_IRQ_GPIO;
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
		deviceConfiguration->IRQCallbackFunction = &ETAL_IRQCallbackFunction_CMOST_TUNER_ID_0;
#else
		deviceConfiguration->IRQCallbackFunction = NULL;
#endif

		deviceConfiguration->communicationBusType = (etalTuner[tuner_index].deviceDescr.m_busType == ETAL_BusI2C) ? BusI2C : BusSPI;
		if (deviceConfiguration->communicationBusType == BusI2C)
		{
#if defined(CONFIG_BOARD_ACCORDO5) && defined(CONFIG_HOST_OS_LINUX)
			// in I2C from the tuner get the bus name
			//
			if (ETAL_getI2cBusName(al_i2cDeviceName) != OSAL_OK)
			{
				// no device detected.
				// try default one
				strcpy(al_i2cDeviceName,CMOST_ACCORDO5_I2C_DEVICE);
			}	


			strncpy(deviceConfiguration->communicationBus.i2c.busName, (tChar *)al_i2cDeviceName, MAX_SIZE_BUS_NAME);
#else
			strncpy(deviceConfiguration->communicationBus.i2c.busName, (tChar *)CMOST_ACCORDO2_I2C_DEVICE, MAX_SIZE_BUS_NAME);
#endif // CONFIG_BOARD_ACCORDO5
			deviceConfiguration->communicationBus.i2c.deviceAddress = etalTuner[tuner_index].deviceDescr.m_busAddress;
		}
		else
		{
			strncpy(deviceConfiguration->communicationBus.spi.busName, (tChar *)CMOST_T1_ACCORDO2_SPI_DEVICE, MAX_SIZE_BUS_NAME);
			deviceConfiguration->communicationBus.spi.GPIO_CS = CMOST_T1_ACCORDO2_SPI_CS_GPIO;
			deviceConfiguration->communicationBus.spi.mode = CMOST_ACCORDO2_SPI_MODE;
			deviceConfiguration->communicationBus.spi.speed = CMOST_ACCORDO2_SPI_SPEED;
		}
	}
	else if(tuner_index == ETAL_CMOST_TUNER_2_ID)
	{
		deviceConfiguration->GPIO_RESET = CMOST_T2_ACCORDO2_RESET_GPIO;
		deviceConfiguration->GPIO_IRQ = CMOST_T2_ACCORDO2_IRQ_GPIO;
#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
		deviceConfiguration->IRQCallbackFunction = &ETAL_IRQCallbackFunction_CMOST_TUNER_ID_1;
#else
		deviceConfiguration->IRQCallbackFunction = NULL;
#endif

		deviceConfiguration->communicationBusType = (etalTuner[tuner_index].deviceDescr.m_busType == ETAL_BusI2C) ? BusI2C : BusSPI;
		if (deviceConfiguration->communicationBusType == BusI2C)
		{
#if defined (CONFIG_BOARD_ACCORDO5) && defined(CONFIG_HOST_OS_LINUX)
			// in I2C from the tuner get the bus name
			//
			if (ETAL_getI2cBusName(al_i2cDeviceName) != OSAL_OK)
			{
				// no device detected.
				// try default one
				strcpy(al_i2cDeviceName,CMOST_ACCORDO5_I2C_DEVICE);
			}	

			strncpy(deviceConfiguration->communicationBus.i2c.busName, (tChar *)al_i2cDeviceName, MAX_SIZE_BUS_NAME);
#else
			strncpy(deviceConfiguration->communicationBus.i2c.busName, (tChar *)CMOST_ACCORDO2_I2C_DEVICE, MAX_SIZE_BUS_NAME);
#endif // CONFIG_BOARD_ACCORDO5
			deviceConfiguration->communicationBus.i2c.deviceAddress = etalTuner[tuner_index].deviceDescr.m_busAddress;
		}
		else
		{
			strncpy(deviceConfiguration->communicationBus.spi.busName, (tChar *)CMOST_T2_ACCORDO2_SPI_DEVICE, MAX_SIZE_BUS_NAME);
			deviceConfiguration->communicationBus.spi.GPIO_CS = CMOST_T2_ACCORDO2_SPI_CS_GPIO;
			deviceConfiguration->communicationBus.spi.mode = CMOST_ACCORDO2_SPI_MODE;
			deviceConfiguration->communicationBus.spi.speed = CMOST_ACCORDO2_SPI_SPEED;
		}
	}
	else
	{
		// not supported configuration 
		// 
		
		vl_res = OSAL_ERROR;
	}

exit:	
	return vl_res;
}


/***************************
 *
 * ETAL_tunerGetType
 *
 **************************/
/*!
 * \brief		Returns the Tuner device type
 * \details		ETAL supports CMOST tuners single and dual channel, STAR and DOT flavours.
 * \param[in]	hTuner  - Tuner handle
 * \return		The tuner device type, or deviceUnknown if the specified Tuner is invalid
 * \callgraph
 * \callergraph
 */
EtalDeviceType ETAL_tunerGetType(ETAL_HANDLE hTuner)
{
	ETAL_HINDEX tunerIndex;
	EtalDeviceType deviceType;

	if (!ETAL_tunerIsValidHandle(hTuner))
	{
		deviceType = deviceUnknown;
	}
	else
	{
		tunerIndex = ETAL_handleTunerGetIndex(hTuner);
		deviceType = etalTuner[tunerIndex].deviceDescr.m_deviceType;
	}
	return deviceType;
}

/***************************
 *
 * ETAL_tunerGetFrontend
 *
 **************************/
/*!
 * \brief		Returns the description of a Frontend (a.k.a channel)
 * \details		A Frontend description contains the Frontend's internal address (that is
 * 				the address used by the device's command protocol to address the
 * 				Frontend) and the supported Broadcast Standards and Data Types. This
 * 				information is specified at build time.
 * \remark		Returns a pointer to an ETAL internal structure that should not be changed by the caller.
 * \param[in]	hFrontend  - Frontend handle
 * \return		A pointer to the Frontend description, or NULL if the hFrontend is invalid.
 * \see			The etalconfig_*.c files for the Tuner description.
 * \callgraph
 * \callergraph
 */
etalFrontendDescTy *ETAL_tunerGetFrontend(ETAL_HANDLE hFrontend)
{
	etalFrontendDescTy *fedescp;
	ETAL_HINDEX tunerIndex;
	ETAL_HINDEX channelIndex;

	if (ETAL_tunerSearchFrontend(hFrontend) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);
		fedescp = NULL;
	}
	else
	{
		tunerIndex = ETAL_handleFrontendGetTunerIndex(hFrontend);
		channelIndex = ETAL_handleFrontendGetChannel(hFrontend);
	
		fedescp = &etalTuner[tunerIndex].frontEndList[channelIndex];
	}
	return fedescp;
}

/***************************
 *
 * ETAL_tunerSearchFrontend
 *
 **************************/
/*!
 * \brief		Checks if a Frontend is valid
 * \details		ETAL uses the Frontend handle to uniquely identify a Frontend
 * 				in the whole system. This function validates the Frontend
 * 				handle by checking if it referes to a channel present on the
 * 				Tuner and the Tuner is enabled.
 * \param[in]	hFrontend  - Frontend handle
 * \return		OSAL_OK    - hFrontend is valid
 * \returns		OSAL_ERROR - hFrontend is not valid
 * \callgraph
 * \callergraph
 */
tSInt ETAL_tunerSearchFrontend(ETAL_HANDLE hFrontend)
{
	ETAL_HANDLE hTuner;
	ETAL_HINDEX channelIndex;
	tSInt ret = OSAL_ERROR;

	if (!ETAL_handleIsFrontend(hFrontend))
	{
		ret = OSAL_ERROR;
	}
	else
	{
		hTuner = ETAL_handleFrontendGetTuner(hFrontend);
		if (ETAL_tunerIsValidHandle(hTuner))
		{
			channelIndex = ETAL_handleFrontendGetChannel(hFrontend);
			if ((channelIndex == ETAL_FE_FOREGROUND) ||
				((channelIndex == ETAL_FE_BACKGROUND) && ((ETAL_tunerGetType(hTuner) & deviceTwinFE) == deviceTwinFE)))
			{
				if (ETAL_statusHardwareAttrIsTunerActive(hTuner))
				{
					ret = OSAL_OK;
				}
			}
		}
	}

	return ret;
}

/***************************
 *
 * ETAL_tunerSearchAllFrontend
 *
 **************************/
/*!
 * \brief		Checks if all the specified Frontends are part of some Tuner
 * \details		The function checks if the list of Frontends is contained is one Tuner
 * 				and returns the handle of the Tuner.
 * \param[in]	list   - the array of Frontend handles to search for
 * \param[in]   size   - the size of the *list* array; the max supported size is #ETAL_CAPA_MAX_FRONTEND_PER_TUNER
 * \param[out]	hTuner - pointer to the location where the function stores the handle
 * 				         of the Tuner containing the specified list of Frontends,
 * 				         or ETAL_INVALID_HANDLE if none was found.
 * \return		OSAL_OK    - The Frontend list was found
 * \return		OSAL_ERROR - The Frontend list was not found, in this case hTuner is set
 * 				             to ETAL_INVALID_HANDLE
 * \callgraph
 * \callergraph
 */
tSInt ETAL_tunerSearchAllFrontend(const ETAL_HANDLE *list, tU32 size, ETAL_HANDLE *hTuner)
{
	tU32 i;
	ETAL_HANDLE hFrontend;
	ETAL_HANDLE hTuner_local;
	tSInt ret = OSAL_OK;

	*hTuner = ETAL_INVALID_HANDLE;
	for (i = 0; i < size; i++)
	{
		hFrontend = list[i];
		if (hFrontend == ETAL_INVALID_HANDLE)
		{
			break;
		}
		
		if (ETAL_tunerSearchFrontend(hFrontend) == OSAL_OK)
		{
			hTuner_local = ETAL_handleFrontendGetTuner(hFrontend);
			if (*hTuner == ETAL_INVALID_HANDLE)
			{
				*hTuner = hTuner_local;
			}
			else if (*hTuner == hTuner_local)
			{
				continue;
			}
			else
			{
				*hTuner = ETAL_INVALID_HANDLE;
				break;
			}
		}
		else
		{
			*hTuner = ETAL_INVALID_HANDLE;
			break;
		}

	}
	if (*hTuner == ETAL_INVALID_HANDLE)
	{
		ret = OSAL_ERROR;
	}
	return ret;
}

#ifdef CONFIG_ETAL_SUPPORT_CMOST
/***************************
 *
 * ETAL_tunerGetFirstSTAR_CMOST
 *
 **************************/
/*!
 * \brief		Returns the handle of the first STAR device present in the system
 * \details		If a system includes both STAR and DOT devices, this function can be used
 * 				to find the handle of a STAR device. The function scans the etalTuner array
 * 				and returns the handle of the first STAR device it finds. This information
 * 				is used during ETAL system startup to initialize the audio connections
 * 				(only the STAR device supports audio).
 * \remark		For more complex system topology a dedicated function must be provided.
 * \return		The handle of the Tuner if there is at least one STAR device in the system
 * 				or ETAL_INVALID_HANDLE
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_tunerGetFirstSTAR_CMOST(tVoid)
{
	ETAL_HINDEX tunerIndex;
	ETAL_HANDLE ret = ETAL_INVALID_HANDLE;
	tU32 i;

	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		if (ETAL_DEVICE_IS_STAR(etalTuner[i].deviceDescr.m_deviceType))
		{
			tunerIndex = (ETAL_HINDEX)i;
			ret = ETAL_handleMakeTuner(tunerIndex);
			goto exit;
		}
	}

exit:
	return ret;
}

/***************************
 *
 * ETAL_tunerGetFirstDOT_CMOST
 *
 **************************/
/*!
 * \brief		Returns the handle of the first DOT device present in the system
 * \remark		For more complex system topology a dedicated function must be provided.
 * \return		The handle of the Tuner if there is at least one DOT device in the system
 * 				or ETAL_INVALID_HANDLE
 * \see			ETAL_tunerGetFirstSTAR_CMOST for a similar function, but for STAR device
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_tunerGetFirstDOT_CMOST(tVoid)
{
	ETAL_HINDEX tunerIndex;
	ETAL_HANDLE ret = ETAL_INVALID_HANDLE;
	tU32 i;

	for (i = 0; i < ETAL_CAPA_MAX_TUNER; i++)
	{
		if (ETAL_DEVICE_IS_DOT(etalTuner[i].deviceDescr.m_deviceType))
		{
			tunerIndex = (ETAL_HINDEX)i;
			ret = ETAL_handleMakeTuner(tunerIndex);
			goto exit;
		}
	}

exit:
	return ret;
}

/***************************
 *
 * ETAL_tunerGetFirstTuner_CMOST
 *
 **************************/
/*!
 * \brief		Returns the handle of the first Tuner device present in the system (Star or Dot)
 * \details		If a system includes several Tuner (STAR or/and DOT) devices, this function can be used
 * 				to find the handle of a 1st device. The function returns the handle of the first device.
 * 				This information is used during ETAL system startup to initialize the configuration of
 *				the tuners for audio, and SDM connections to DCOP
 * \remark		For more complex system topology a dedicated function must be provided.
 * \return		The handle of the first Tuner if there is at least one STAR device in the system
 * 				or ETAL_INVALID_HANDLE
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_tunerGetFirstTuner_CMOST(tVoid)
{
	ETAL_HINDEX tunerIndex;

	tunerIndex = (ETAL_HINDEX)0;
	return ETAL_handleMakeTuner(tunerIndex);
}

#ifdef CONFIG_MODULE_INTEGRATED
/***************************
 *
 * ETAL_tunerGetSecondTuner_CMOST
 *
 **************************/
/*!
 * \brief		Returns the handle of the 2nd Tuner device present in the system (Star or Dot)
 * \details		If a system includes several Tuner (STAR or/and DOT) devices, this function can be used
 * 				to find the handle of a 2nd device. The function returns the handle of the first device.
 * 				This information is used during ETAL system startup to initialize the configuration of
 *				the tuners for audio, and SDM connections to DCOP
 * \remark		For more complex system topology a dedicated function must be provided.
 * \return		The handle of the second Tuner if there is one
 * 				or ETAL_INVALID_HANDLE
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_tunerGetSecondTuner_CMOST(tVoid)
{
	ETAL_HINDEX tunerIndex;

	tunerIndex = (ETAL_HINDEX)1;
	if (tunerIndex < (ETAL_HINDEX)ETAL_CAPA_MAX_TUNER)
	{
		return ETAL_handleMakeTuner(tunerIndex);
	}
	
	return ETAL_INVALID_HANDLE;
}
#endif
#endif // #ifdef CONFIG_ETAL_SUPPORT_CMOST

#if defined (CONFIG_ETAL_HAVE_ETALTML)
/***************************
 *
 * ETALTML_pathGetNextFEList
 *
 **************************/
#if 0 // commented out to get a Doxygen warning, add comments and remove the #if 0
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \param[out]	
 * \param[in,out] 
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		ADDITION for PATH mgt : to be located somewhere else later on
 */
#endif
/*
 * return the FE list and list size of the 
 */
tSInt ETALTML_pathGetNextFEList(etalDiversTy **pO_Felist, EtalPathName vI_path, tU8 vI_Index)
{
	tSInt vl_res = OSAL_OK;
	tU32 vl_indexPath;
	tBool vl_pathFound = false;
	
	// basic checks 
	for (vl_indexPath=0; vl_indexPath < (tU32)etalPathPreferences.m_maxPath; vl_indexPath++)
	{
		if (etalPathPreferences.m_Path[vl_indexPath].m_PathName == vI_path)
		{
			vl_pathFound = true;
			break;
		}
	}

	if (false == vl_pathFound)
	{
		// path not found
		*pO_Felist = NULL;
		vl_res = OSAL_ERROR;
		goto exit;
	}

	// check now if the index is ok : it should be below the nb tuner comibnaison
	if (vI_Index >= etalPathPreferences.m_Path[vl_indexPath].m_NbTunerCombinaison)
	{
		// no more tuner index available 
		// path not found
		*pO_Felist = NULL;
		vl_res = OSAL_ERROR;
		goto exit;
	}

	// return the FE list
	*pO_Felist = &etalPathPreferences.m_Path[vl_indexPath].m_Diversity[vI_Index];

	ETALTML_PRINTF(TR_LEVEL_USER_1, "ETALTML_pathGetNextFEList, found, path index %d, diversity index = %d\n", vl_indexPath, vI_Index);

exit:
	return vl_res;


}

/***************************
 *
 * ETALTML_pathGetStandard
 *
 **************************/
#if 0 // commented out to get a Doxygen warning, add comments and remove the #if 0
/*!
 * \brief		
 * \details		
 * \remark		
 * \param[in]	
 * \return		
 * \see			
 * \callgraph
 * \callergraph
 * \todo		ADDITION for PATH mgt : to be located somewhere else later on
 */
#endif
/*
 * return the FE list and list size of the 
 */
EtalBcastStandard ETALTML_pathGetStandard(EtalPathName vI_path)
{
	EtalBcastStandard vl_res = ETAL_BCAST_STD_UNDEF;
	tU32 vl_indexPath;
	tBool vl_pathFound = false;
	
	
	// basic checks 
	for (vl_indexPath=0; vl_indexPath < (tU32)etalPathPreferences.m_maxPath; vl_indexPath++)
	{
		if (etalPathPreferences.m_Path[vl_indexPath].m_PathName == vI_path)
		{
			vl_pathFound = true;
			break;
		}
	}

	if (true == vl_pathFound)
	{
		vl_res = etalPathPreferences.m_Path[vl_indexPath].m_PathStandard;
	}


	return vl_res;


}


#endif // CONFIG_ETAL_HAVE_ETALTML

