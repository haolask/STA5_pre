//!
//!  \file 		etalapi_read_write_parameter.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Interface functions for the ETAL user application
//!  \author 	Jean-Hugues Perrin
//!

#include "osal.h"
#include "etalinternal.h"
#include "boot_cmost.h"

#if defined (CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS) || defined (CONFIG_ETAL_HAVE_ALL_API) || \
	defined (CONFIG_ETAL_HAVE_XTAL_ALIGNMENT) || defined (CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING)
/*
 * Memory mappings for CMOST devices
 */
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	#include BOOT_FIRMWARE_MEMORY_ADDRESS_STAR_T
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	#include BOOT_FIRMWARE_MEMORY_ADDRESS_STAR_S
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
	#include BOOT_FIRMWARE_MEMORY_ADDRESS_DOT_T
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
	#include BOOT_FIRMWARE_MEMORY_ADDRESS_DOT_S
#endif

#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
extern tU32 etalRegisterAddress_TDA7707[];
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
extern tU32 etalRegisterAddress_TDA7708[];
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
extern tU32 etalRegisterAddress_STA710[];
#endif
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
extern tU32 etalRegisterAddress_STA709[];
#endif

/******************************************************************************
 * Local functions
 *****************************************************************************/
static ETAL_STATUS ETAL_read_parameter_check(ETAL_HANDLE hTuner, etalReadWriteModeTy mode, tU32 *param, tU16 length, tU32 *response, tU16 *responseLength);
static ETAL_STATUS ETAL_write_parameter_check(ETAL_HANDLE hTuner, etalReadWriteModeTy mode, tU32 *paramValueArray, tU16 length);
static tU32 getAddressFromIndex(EtalDeviceType tuner_type, tU32 index);

/***************************
 *
 * getAddressFromIndex
 *
 **************************/
/*
 * Get CMOS Tuner memory address from index
 */
static tU32 getAddressFromIndex(EtalDeviceType tuner_type, tU32 index)
{
	tU32 *lookupTable;

	lookupTable = NULL;
	if (index >= ETAL_IDX_CMT_MAX_INTERNAL)
	{
		return ETAL_UNDEFINED_ADDRESS;
	}

	switch (tuner_type)
	{
		case deviceSTART:
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
			lookupTable = etalRegisterAddress_TDA7707;
#endif
			break;

		case deviceSTARS:
#if defined (CONFIG_ETAL_SUPPORT_CMOST_STAR) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
			lookupTable = etalRegisterAddress_TDA7708;
#endif
			break;

		case deviceDOTT:
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
			lookupTable = etalRegisterAddress_STA710;
			/*
			 * All of the external indexes are defined to access quality
			 * or WSP registers which are not available on the DOT
			 * Instead of inserting ETAL_IDX_CMT_MAX_EXTERNAL ETAL_UNDEFINED_ADDRESS
			 * values in the etalRegisterAddress_STA710, we make a smaller array
			 * containing only the internally addressable registers
			 * and translate the index here
			 */
			index -= ETAL_IDX_CMT_MAX_EXTERNAL;
#endif
			break;

		case deviceDOTS:
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DOT) && defined (CONFIG_ETAL_SUPPORT_CMOST_SINGLE_CHANNEL)
			/*
			 * see deviceDOTT few lines above for explanation on the index translation
			 */
			lookupTable = etalRegisterAddress_STA709;
			index -= ETAL_IDX_CMT_MAX_EXTERNAL;
#endif
			break;

		default:
			break;
	}

	if (lookupTable == NULL)
	{
		return ETAL_UNDEFINED_ADDRESS;
	}
	return lookupTable[index];
}

/***************************
 *
 * ETAL_read_parameter_nolock
 *
 **************************/
ETAL_STATUS ETAL_read_parameter_nolock(ETAL_HANDLE hTuner, etalReadWriteModeTy mode, tU32 *param, tU16 length, tU32 *response, tU16 *responseLength)
{
	ETAL_STATUS ret;
    tU32 address;
    tU8 resp_CMOST[CMOST_MAX_RESPONSE_LEN];
    tU32 resp_len, i;
	EtalDeviceType tuner_type;

	ret = ETAL_RET_SUCCESS;

	tuner_type = ETAL_tunerGetType(hTuner);
	*responseLength = 0;
	for(i = 0; i < length; i++)
	{
		if(mode == fromIndex)
		{
			address = getAddressFromIndex(tuner_type, param[i]);

			if(address == ETAL_UNDEFINED_ADDRESS)
			{
				ret = ETAL_RET_PARAMETER_ERR;
				break;
			}
		}
		else
		{
			address = param[i];
		}
		
		if (ETAL_directCmdRead_CMOST(hTuner, address, 1, resp_CMOST, &resp_len) != OSAL_OK)
		{
			ret = ETAL_RET_ERROR;
		}
		else
		{
			response[*responseLength]  = ((tU32)resp_CMOST[0] << 16) & 0x00FF0000;
			response[*responseLength] |= ((tU32)resp_CMOST[1] <<  8) & 0x0000FF00;
			response[*responseLength] |= ((tU32)resp_CMOST[2] <<  0) & 0x000000FF;

			*responseLength += 1;

			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "--- Read Address: 0x%x, value: 0x%x", address, *response);
		}
	}

	return ret;
}

/***************************
 *
 * ETAL_read_parameter_check
 *
 **************************/
static ETAL_STATUS ETAL_read_parameter_check(ETAL_HANDLE hTuner, etalReadWriteModeTy mode, tU32 *param, tU16 length, tU32 *response, tU16 *responseLength)
{
	tU32 i;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	/* 
	 * external indexes are defined only for STAR
	 * so reject requests for other devices
	 */
	if (!ETAL_DEVICE_IS_STAR(ETAL_tunerGetType(hTuner)))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}

	if((param == NULL) || (response == NULL) || (responseLength == NULL) ||
	   (length > ETAL_DEF_MAX_READWRITE_SIZE))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}

	if((mode != fromAddress) && (mode != fromIndex))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}

	if ((mode == fromIndex) && (param != NULL))
	{
		/* ensure all the indexes are legal, that is allowed to the external API user */
		for (i = 0; i < length; i++)
		{
			if (param[i] >= ETAL_IDX_CMT_MAX_EXTERNAL)
			{
				ret = ETAL_RET_PARAMETER_ERR;
			}
		}
	}
	return ret;
}

/***************************
 *
 * ETAL_read_parameter_internal
 *
 **************************/
ETAL_STATUS ETAL_read_parameter_internal(ETAL_HANDLE hTuner, etalReadWriteModeTy mode, tU32 *param, tU16 length, tU32 *response, tU16 *responseLength)
{
	ETAL_STATUS ret;

	if ((ret = ETAL_read_parameter_check(hTuner, mode, param, length, response, responseLength)) == ETAL_RET_SUCCESS)
	{
		ret = ETAL_read_parameter_nolock(hTuner, mode, param, length, response, responseLength);
	}
	return ret;
}

/***************************
 *
 * ETAL_write_parameter_internal
 *
 **************************/
ETAL_STATUS ETAL_write_parameter_internal(ETAL_HANDLE hTuner, etalReadWriteModeTy mode, tU32 *paramValueArray, tU16 length)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	tU16 i;
    tU32 address, index, value;
	EtalDeviceType tuner_type;
	tSInt vl_res;

	if((ret = ETAL_write_parameter_check(hTuner, mode, paramValueArray, length)) == ETAL_RET_SUCCESS)
	{
		tuner_type = ETAL_tunerGetType(hTuner);
		for (i = 0; i < length * ETAL_WRITE_PARAM_ENTRY_SIZE; i += ETAL_WRITE_PARAM_ENTRY_SIZE)
		{
			if (mode == fromIndex)
			{
				index = ETAL_WRITE_PARAM_GET_INDEX(paramValueArray + i);
				address = getAddressFromIndex(tuner_type, index);

				if (address == ETAL_UNDEFINED_ADDRESS)
				{
					ret = ETAL_RET_PARAMETER_ERR;
					break;
				}
			}
			else
			{
				address = ETAL_WRITE_PARAM_GET_ADDRESS(paramValueArray + i);
			}

			value = ETAL_WRITE_PARAM_GET_VALUE(paramValueArray + i);
			vl_res = ETAL_directCmdWrite_CMOST(hTuner, address, value);
			ETAL_tracePrintComponent(TR_CLASS_APP_ETAL, "--- Write Address: 0x%x, value: 0x%x", address, value);

			if (vl_res != OSAL_OK)
			{
				ret = ETAL_RET_ERROR;
				break;
			}
		}
	}
	return ret;
}

/***************************
 *
 * ETAL_write_parameter_check
 *
 **************************/
static ETAL_STATUS ETAL_write_parameter_check(ETAL_HANDLE hTuner, etalReadWriteModeTy mode, tU32 *paramValueArray, tU16 length)
{
	tU32 i;
	tU32 index;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;

	/* 
	 * external indexes are defined only for STAR
	 * so reject requests for other devices
	 */
	if (!ETAL_DEVICE_IS_STAR(ETAL_tunerGetType(hTuner)))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	if ((paramValueArray == NULL) ||
	   (length > ETAL_DEF_MAX_READWRITE_SIZE))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}
	if ((mode != fromAddress) && (mode != fromIndex))
	{
		ret = ETAL_RET_PARAMETER_ERR;
	}


	if (mode == fromIndex)
	{
		/* ensure all the indexes are legal, that is allowed to the external API user */
		for (i = 0; i < length * ETAL_WRITE_PARAM_ENTRY_SIZE; i += ETAL_WRITE_PARAM_ENTRY_SIZE)
		{
			index = ETAL_WRITE_PARAM_GET_INDEX(paramValueArray + i);
			if (index >= ETAL_IDX_CMT_MAX_INTERNAL)
			{
				ret = ETAL_RET_PARAMETER_ERR;
			}
		}
	}
	return ret;
}

#endif // CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS || CONFIG_ETAL_HAVE_ALL_API || CONFIG_ETAL_HAVE_XTAL_ALIGNMENT || CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING

/***************************
 *
 * External interfaces
 *
 **************************/

#if defined (CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS) || defined (CONFIG_ETAL_HAVE_ALL_API)
/***************************
 *
 * etal_read_parameter
 *
 **************************/
ETAL_STATUS etal_read_parameter(tU32 tunerIndex, etalReadWriteModeTy mode, tU32 *param, tU16 length, tU32 *response, tU16 *responseLength)
{
	ETAL_STATUS ret;
	ETAL_HANDLE hTuner;
	
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_read_parameter(tun: %d, mod: %d, len: %d)", tunerIndex, mode, length);

	hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)tunerIndex);
	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		if (!ETAL_tunerIsValidHandle(hTuner))
		{
			ret = ETAL_RET_INVALID_HANDLE;
		}
		else
		{
			ret = ETAL_read_parameter_internal(hTuner, mode, param, length, response, responseLength);
		}
		ETAL_statusReleaseLock();
	}
	
    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_read_parameter() = %s", ETAL_STATUS_toString(ret));
	return (ret);
}

/***************************
 *
 * etal_write_parameter
 *
 **************************/
ETAL_STATUS etal_write_parameter(tU32 tunerIndex, etalReadWriteModeTy mode, tU32 *paramValueArray, tU16 length)
{
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
	ETAL_HANDLE hTuner;
    tU32 index, i;

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "APP->ETAL: etal_write_parameter(tun: %d, mod: %d, len: %d)", tunerIndex, mode, length);

	hTuner = ETAL_handleMakeTuner((ETAL_HINDEX)tunerIndex);

	if (ETAL_statusGetLock() != OSAL_OK)
	{
		ret = ETAL_RET_NOT_INITIALIZED;
	}
	else
	{
		if (!ETAL_tunerIsValidHandle(hTuner))
		{
			ret = ETAL_RET_INVALID_HANDLE;
		}
		else
		{
			if (mode == fromIndex)
			{
				/* ensure all the indexes are legal, that is allowed to the external API user */
				for (i = 0; i < length * ETAL_WRITE_PARAM_ENTRY_SIZE; i += ETAL_WRITE_PARAM_ENTRY_SIZE)
				{
					index = ETAL_WRITE_PARAM_GET_INDEX(paramValueArray + i);
					if (index >= ETAL_IDX_CMT_MAX_EXTERNAL)
					{
						ret = ETAL_RET_PARAMETER_ERR;
					}
				}
			}

			if (ret == ETAL_RET_SUCCESS)
			{
				ret = ETAL_write_parameter_internal(hTuner, mode, paramValueArray, length);
			}
		}
		ETAL_statusReleaseLock();
	}

    ETAL_tracePrintSystem(TR_CLASS_APP_ETAL_API, "ETAL->APP: etal_write_parameter() = %s", ETAL_STATUS_toString(ret));
	return ret;
}

/***************************
 *
 * etal_write_parameter_convert
 *
 **************************/
/*!
 * \brief		Convert old-style address-value parameter to current API
 * \details		Function #etal_write_parameter used to have two separate parameters (pointer
 * 				to tU32) to pass the addresses (or indexes) of the locations
 * 				to write and the list of values to write. This is now changed to
 * 				use the same format provided by some STM utilities for parameters,
 * 				that is a unique array containing:
 * 				bytes N+0 to N+3 = address or index
 * 				bytes N+4 to N+6 = value
 *
 * 				This function converts the old format to the new format.
 * \remark		This function is provided only to aid porting from old
 * 				application code and mat be removed in later releases of ETAL
 * \param[in]	buf - pointer to the buffer where the function will
 * 				      store the converted parameters. It must be at least
 * 				      (*length* x 2) bytes long.
 * \param[in]	address - list of addresses or indexes to use, of size *length*
 * \param[in]	value - list of values to write, of size *length*
 * \param[in]	length - number of entries in the *address* and *value* arrays
 * \return		ETAL_RET_PARAMETER_ERR - *length* is larger than the max supported
 * 				                         by the #etal_write_parameter function
 * 				                         (i.e. #ETAL_DEF_MAX_READWRITE_SIZE)
 * \return		ETAL_RET_SUCCESS
 * \callgraph
 * \callergraph
 */
ETAL_STATUS etal_write_parameter_convert(tU32 *buf, tU32 *address, tU32 *value, tU16 length)
{
	tU32 i;
	tU32 index;

	if (length > ETAL_DEF_MAX_READWRITE_SIZE)
	{
		return ETAL_RET_PARAMETER_ERR;
	}
	index = 0;
	for (i = 0; i < length; i++)
	{
		buf[index + 0] = address[i];
		buf[index + 1] = value[i];
		index += 2;
	}

	return ETAL_RET_SUCCESS;
}
#endif // CONFIG_ETAL_HAVE_READ_WRITE_PARAMETERS || CONFIG_ETAL_HAVE_ALL_API


