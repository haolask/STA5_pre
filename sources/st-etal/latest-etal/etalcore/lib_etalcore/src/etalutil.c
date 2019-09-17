//!
//!  \file 		etalutil.c
//!  \brief 	<i><b> ETAL utilities </b></i>
//!  \details   Various utilities including ETAL_HANDLE and frequency band management
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

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/

/*****************************************************************
| Local types
|----------------------------------------------------------------*/

/*!
 * \enum	etalHandleTypeEnum
 * 			Represents the the ETAL_HANDLE type bits.
 * \remark	Only three bits are available so the enum may range 0 to 7 included.
 */
typedef enum
{
	/*! Invalid handle: **do not modify this value** */
	handleTypeInvalid   = 0x0,
	/*! Receiver handle type */
	handleTypeReceiver  = 0x1,
	/*! Tuner handle type    */
	handleTypeTuner     = 0x2,
	/*! Datapath handle type */
	handleTypeDatapath  = 0x3,
	/*! Monitor handle type  */
	handleTypeMonitor   = 0x4,
	/*! Frontend handle type */
	handleTypeFrontend  = 0x5,
} etalHandleTypeEnum;

/***************************
 *
 * ETAL_utilitySetU8
 *
 **************************/
/*!
 * \brief		Writes an 8 bit value to an array location
 * \details		Mainly used to initialize command array locations.
 * \param[in,out] buf - the array of bytes to write to
 * \param[in]	off   - the offset from the first *buf* byte of the location to write
 * \param[in]	val   - the value to be written
 * \callgraph
 * \callergraph
 */
tVoid ETAL_utilitySetU8(tU8 *buf, tU32 off, tU8 val)
{
	buf[off] = val;
}

/***************************
 *
 * ETAL_utilityGetU8
 *
 **************************/
/*!
 * \brief		Reads an 8 bit value from an array
 * \param[in]	buf - the array of bytes to read from
 * \param[in]	off - the offset from the first *buf* byte of the location to read
 * \return		the byte at position *off*
 * \callgraph
 * \callergraph
 */
tU8  ETAL_utilityGetU8(tU8 *buf, tU32 off)
{
	return buf[off] & 0xFF;
}

/***************************
 *
 * ETAL_utilitySetU16
 *
 **************************/
/*!
 * \brief		Writes a 16 bit value to an array starting at a given location
 * \details		Mainly used to initialize command array locations. Writes in DAB DCOP byte order.
 * \param[in,out] buf - the array of bytes to write to
 * \param[in]	off   - the offset from the first *buf* byte of the first location to write
 * \param[in]	val   - the value to be written
 * \callgraph
 * \callergraph
 */
tVoid ETAL_utilitySetU16(tU8 *buf, tU32 off, tU16 val)
{
	buf[off++] = (tU8)((val >> 8) & 0xFF);
	buf[off] =   (tU8)((val >> 0) & 0xFF);
}

/***************************
 *
 * ETAL_utilityGetU16
 *
 **************************/
/*!
 * \brief		Reads a 16 bit value from an array
 * \details		Reassembles bytes in the DCOP MDR byte order.
 * \param[in]	buf - the array of bytes to read from
 * \param[in]	off - the offset from the first *buf* byte of the locations to read
 * \return		the 16bit value read from position *off*
 * \callgraph
 * \callergraph
 */
tU16 ETAL_utilityGetU16(tU8 *buf, tU32 off)
{
	return (((tU16)buf[off] << 8) & 0xFF00) | ((tU16)buf[off + 1] & 0xFF);
}

/***************************
 *
 * ETAL_utilitySetU24
 *
 **************************/
/*!
 * \brief		Writes a 24 bit value to an array starting at a given location
 * \details		Mainly used to initialize command array locations. Writes in DAB DCOP byte order.
 * \param[in,out] buf - the array of bytes to write to
 * \param[in]	off   - the offset from the first *buf* byte of the first location to write
 * \param[in]	val   - the value to be written
 * \callgraph
 * \callergraph
 */
tVoid ETAL_utilitySetU24(tU8 *buf, tU32 off, tU32 val)
{
	buf[off++] = (tU8)((val >> 16) & 0xFF);
	buf[off++] = (tU8)((val >>  8) & 0xFF);
	buf[off] =   (tU8)((val >>  0) & 0xFF);
}

/***************************
 *
 * ETAL_utilityGetU24
 *
 **************************/
/*!
 * \brief		Reads a 24 bit value from an array
 * \details		Reassembles bytes in the DCOP MDR byte order.
 * \param[in]	buf - the array of bytes to read from
 * \param[in]	off - the offset from the first *buf* byte of the locations to read
 * \return		the 32bit value read from position *off*
 * \callgraph
 * \callergraph
 */
tU32 ETAL_utilityGetU24(tU8 *buf, tU32 off)
{
	return (((tU32)buf[off + 0] << 16) & 0x00FF0000) | (((tU32)buf[off + 1] << 8) & 0x0000FF00) | ((tU32)buf[off + 2] & 0x000000FF);
}

/***************************
 *
 * ETAL_utilitySetU32
 *
 **************************/
/*!
 * \brief		Writes a 32 bit value to an array starting at a given location
 * \details		Mainly used to initialize command array locations. Writes in DAB DCOP byte order.
 * \param[in,out] buf - the array of bytes to write to
 * \param[in]	off   - the offset from the first *buf* byte of the first location to write
 * \param[in]	val   - the value to be written
 * \callgraph
 * \callergraph
 */
tVoid ETAL_utilitySetU32(tU8 *buf, tU32 off, tU32 val)
{
	buf[off++] = (tU8)((val >> 24) & 0xFF);
	buf[off++] = (tU8)((val >> 16) & 0xFF);
	buf[off++] = (tU8)((val >>  8) & 0xFF);
	buf[off] =   (tU8)((val >>  0) & 0xFF);
}

/***************************
 *
 * ETAL_utilityGetU32
 *
 **************************/
/*!
 * \brief		Reads a 32 bit value from an array
 * \details		Reassembles bytes in the DCOP MDR byte order.
 * \param[in]	buf - the array of bytes to read from
 * \param[in]	off - the offset from the first *buf* byte of the locations to read
 * \return		the 32bit value read from position *off*
 * \callgraph
 * \callergraph
 */
tU32 ETAL_utilityGetU32(tU8 *buf, tU32 off)
{
	return (((tU32)buf[off + 0] << 24) & 0xFF000000) | (((tU32)buf[off + 1] << 16) & 0x00FF0000) | (((tU32)buf[off + 2] << 8) & 0x0000FF00) | ((tU32)buf[off + 3] & 0x000000FF);
}

/***************************
 *
 * ETAL_utilityGetDefaultBandLimits
 *
 **************************/
/*!
 * \brief		Returns the default upper and lower band limits and seek step for a frequency band
 * \details		The band can be any of the values defined in #EtalFrequencyBand except
 * 				the 'custom' bands ETAL_BAND_USERFM and ETAL_BAND_USERAM and the undefined value.
 * \remark		The function does not validate the output parameters.
 * \param[in]	band    - the requested band
 * \param[out]	bandMin - pointer to an location where the function stores the min frequency for the
 * 				          requested band, in Khz
 * \param[out]	bandMax - pointer to an location where the function stores the max frequency for the
 * 				          requested band, in Khz
 * \param[out]	step    - pointer to a location where the function stores the step used by the CMOST
 * 				          firmware (or internal) seek algorithm
 * \return		ETAL_RET_SUCCESS
 * \return		ETAL_RET_ERROR   - undefined or illegal frequency band
 * \callgraph
 * \callergraph
 */
ETAL_STATUS ETAL_utilityGetDefaultBandLimits(EtalFrequencyBand band, tU32 *bandMin, tU32 *bandMax, tU32 *step)
{
	ETAL_STATUS retval = ETAL_RET_SUCCESS;

	switch (band)
	{
		case ETAL_BAND_FM:
			*bandMin = ETAL_BAND_FM_MIN;
			*bandMax = ETAL_BAND_FM_MAX;
			*step = ETAL_BAND_FM_STEP;
			break;

			
		case ETAL_BAND_FMEU:
			*bandMin = ETAL_BAND_FMEU_MIN;
			*bandMax = ETAL_BAND_FMEU_MAX;
			*step = ETAL_BAND_FMEU_STEP;
			break;

		case ETAL_BAND_FMUS:
			*bandMin =  ETAL_BAND_FMUS_MIN;
			*bandMax = ETAL_BAND_FMUS_MAX;
			*step = ETAL_BAND_FMUS_STEP;
			break;

		case ETAL_BAND_FMJP:
			*bandMin = ETAL_BAND_FMJP_MIN;
			*bandMax = ETAL_BAND_FMJP_MAX;
			*step = ETAL_BAND_FMJP_STEP;
			break;

		case ETAL_BAND_FMEEU:
			*bandMin = ETAL_BAND_FMEEU_MIN;
			*bandMax = ETAL_BAND_FMEEU_MAX;
			*step = ETAL_BAND_FMEEU_STEP;
			break;

		case ETAL_BAND_WB:
			*bandMin = ETAL_BAND_WB_MIN;
			*bandMax = ETAL_BAND_WB_MAX;
			*step = 25;
			break;

		case ETAL_BAND_DAB3:
			*bandMin = ETAL_BAND_DAB3_MIN;
			*bandMax = ETAL_BAND_DAB3_MAX;
			*step = 0;
			break;

		case ETAL_BAND_DABL:
			*bandMin = ETAL_BAND_DABL_MIN;
			*bandMax = ETAL_BAND_DABL_MAX;
			*step = 1712;
			break;

		case ETAL_BAND_LW:
			*bandMin = 144;
			*bandMax = 288;
			*step = 3;
			break;

		case ETAL_BAND_AM:
		case ETAL_BAND_MWEU:
			*bandMin = 531;
			*bandMax = 1629;
			*step = 1;
			break;

		case ETAL_BAND_MWUS:
		// case ETAL_BAND_HD: completely identycal to ETAL_BAND_MWUS and DEPRECATED
			*bandMin = 530;
			*bandMax = 1710;
			*step = 10;
			break;

		case ETAL_BAND_SW:
			*bandMin = 5900;
			*bandMax = 7350;
			*step = 5;
			break;

		case ETAL_BAND_CUSTAM:
			*bandMin = 21450;
			*bandMax = 26100;
			*step = 5;
			break;

		case ETAL_BAND_DRM30:
			*bandMin = ETAL_BAND_DRM3_MIN;
			*bandMax = ETAL_BAND_DRM3_MAX;
			*step = 1;
			break;
			
		case ETAL_BAND_DRMP:
			*bandMin = ETAL_BAND_DRMP_MIN;
			*bandMax = ETAL_BAND_DRMP_MAX;
			*step = 1;
			break;

		case ETAL_BAND_UNDEF:
			ASSERT_ON_DEBUGGING(0);
			/* fall through */
		case ETAL_BAND_USERFM:
			/* fall through */
		case ETAL_BAND_USERAM:
			*bandMin = 0;
			*bandMax = 0;
			*step = 0;
			retval = ETAL_RET_ERROR;
			break;
		/* no default label to let the compiler warn in case of missing band */
	}
	return retval;
}

/***************************
 *
 * ETAL_handleGetType
 *
 **************************/
/*!
 * \brief		Returns the 'handle type' bits of an ETAL_HANDLE
 * \details		The 'handle type' bits are bits 13-15 of the *hGeneric*.
 * \remark		The function does not check if the returned value is one of the
 * 				defined values of the #etalHandleTypeEnum enum.
 * \param[in]	hGeneric - the ETAL_HANDLE
 * \return		the handle type
 * \see			ETAL_handleMake
 * \callgraph
 * \callergraph
 */
static etalHandleTypeEnum ETAL_handleGetType(ETAL_HANDLE hGeneric)
{
	return ((etalHandleTypeEnum)(((tU32)hGeneric >> 13) & 0x07));
}

/***************************
 *
 * ETAL_handleGetReceiverIndex
 *
 **************************/
/*!
 * \brief		Returns the 'receiver index' of an ETAL_HANDLE
 * \details		The 'receiver index' bits are bits 8-12 of the *hGeneric*. This index
 * 				represents the index of a Receiver and is defined only for ETAL_HANDLE
 * 				of type handleTypeDatapath because Datapaths are allocated inside a Receiver.
 * \remark		The function does not validate the returned index, not if the
 * 				handle is of handleTypeDatapath type.
 * \param[in]	hGeneric - the ETAL_HANDLE
 * \return		The Receiver index
 * \see			ETAL_handleMake
 * \callgraph
 * \callergraph
 */
static ETAL_HINDEX ETAL_handleGetReceiverIndex(ETAL_HANDLE hGeneric)
{
	return (ETAL_HINDEX)(((tU32)hGeneric >> 8) & 0x1F);
}

/***************************
 *
 * ETAL_handleGetIndex
 *
 **************************/
/*!
 * \brief		Returns the 'index' bits of an ETAL_HANDLE
 * \details		The 'index' bits are bits 0-7 of the *hGeneric*.
 * \param[in]	hGeneric - the ETAL_HANDLE
 * \return		The index of the object referred by the *hGeneric*.
 * \see			ETAL_handleMake
 * \callgraph
 * \callergraph
 */
static ETAL_HINDEX ETAL_handleGetIndex(ETAL_HANDLE hGeneric)
{
	return (ETAL_HINDEX)(((tU32)hGeneric >> 0) & 0xFF);
}

/***************************
 *
 * ETAL_handleIsValid
 *
 **************************/
/*!
 * \brief		Checks if an ETAL_HANDLE is valid
 * \details		The function checks if the handle is #ETAL_INVALID_HANDLE and also
 * 				if the 'handle type' bits have one of the defined #etalHandleTypeEnum
 * 				values.
 * \remark		The function does not validate the 'index bits': it only checks if the
 * 				handle numerical value is formally correct.
 * \param[in]	hGeneric - the ETAL_HANDLE
 * \return		TRUE  - the handle is valid
 * \return		FALSE - the handle is **NOT** valid
 * \callgraph
 * \callergraph
 */
tBool ETAL_handleIsValid(ETAL_HANDLE hGeneric)
{
	etalHandleTypeEnum type;
	tBool retval;

	if (hGeneric == ETAL_INVALID_HANDLE)
	{
		retval = FALSE;
	}
	else
	{
		type = ETAL_handleGetType(hGeneric);
		switch (type)
		{
			case handleTypeInvalid:
				retval = FALSE;
				break;
			case handleTypeReceiver:
			case handleTypeTuner:
			case handleTypeDatapath:
			case handleTypeMonitor:
			case handleTypeFrontend:
				retval = TRUE;
				break;
			default:
				ASSERT_ON_DEBUGGING(0);
				retval = FALSE;
				break;
		}
	}
	return retval;
}

/***************************
 *
 * ETAL_handleMake
 *
 **************************/
/*!
 * \brief		Generate a **system-wide unique** ETAL_HANDLE given the basic fields of the handle
 * \details		An ETAL_HANDLE is composed of three fields:
 * 				- type: defines the type of object described by the handle
 * 				- receiver_index: some objects (namely Datapaths) are described by structures
 * 				                 allocated inside a Receiver status; for these objects the
 * 				                 *receiver_index* field defines the index of the Receiver;
 * 				                 in other cases it is 0.
 *
 * 				                 For Frontend handles this field is used to indicate the Tuner
 * 				                 index instead of the Receiver index.
 * 				- index: uniquely identifies the object among objects of the same type.
 * 				         Implementation note: ETAL objects described by an ETAL_HANDLE are typically
 * 				         implemented as part of arrays: the *index* field is the index in the array.
 * \param[in]	type  - the handle type
 * \param[in]	receiver_index - the index of the receiver to which this object is attached, or 0
 * 				                 if not used.
 * \param[in]	index - the index of the object in the array
 * \return		The ETAL_HANDLE, or ETAL_INVALID_HANDLE if *type* or *receiver_index* are out of bounds.
 * \callgraph
 * \callergraph
 */
static ETAL_HANDLE ETAL_handleMake(etalHandleTypeEnum type, ETAL_HINDEX receiver_index, ETAL_HINDEX index)
{
	ETAL_HANDLE retval;
	
	if (((tU32)type > 0x7) ||
		((tU32)receiver_index > 0x1F))
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_INVALID_HANDLE;
	}
	else
	{
		retval = (ETAL_HANDLE)(((tU32)type & 0x07) << 13) | (((tU32)receiver_index & 0x1F) << 8) | ((tU32)index & 0xFF);
	}
	return retval;
}

/***************************
 *
 * ETAL_handleMakeTuner
 *
 **************************/
/*!
 * \brief		Creates a **system-wide unique** ETAL_HANDLE of type Tuner
 * \param[in]	index - the Tuner index
 * \return		The Tuner handle
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_handleMakeTuner(ETAL_HINDEX index)
{
	return ETAL_handleMake(handleTypeTuner, (ETAL_HINDEX)0, index);
}

/***************************
 *
 * ETAL_handleMakeReceiver
 *
 **************************/
/*!
 * \brief		Creates a **system-wide unique** ETAL_HANDLE of type Receiver
 * \param[in]	index - the Receiver index
 * \return		The Receiver handle
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_handleMakeReceiver(ETAL_HINDEX index)
{
	return ETAL_handleMake(handleTypeReceiver, (ETAL_HINDEX)0, index);
}

/***************************
 *
 * ETAL_handleMakeDatapath
 *
 **************************/
/*!
 * \brief		Creates a **system-wide unique** ETAL_HANDLE of type Datapath
 * \param[in]	receiverIndex - the index of the Receiver to which this Datapath is attached
 * \param[in]	index - the Datapath index
 * \return		The Datapath handle, or ETAL_INVALID_HANDLE if *receiverIndex* is out of bounds
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_handleMakeDatapath(ETAL_HINDEX receiverIndex, ETAL_HINDEX index)
{
	return ETAL_handleMake(handleTypeDatapath, receiverIndex, index);
}

/***************************
 *
 * ETAL_handleMakeMonitor
 *
 **************************/
/*!
 * \brief		Creates a **system-wide unique** ETAL_HANDLE of type Monitor
 * \param[in]	index - the Monitor index
 * \return		The Monitor handle
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_handleMakeMonitor(ETAL_HINDEX index)
{
	return ETAL_handleMake(handleTypeMonitor, (ETAL_HINDEX)0, index);
}

/***************************
 *
 * ETAL_handleMakeFrontend
 *
 **************************/
/*!
 * \brief		Creates a **system-wide unique** ETAL_HANDLE of type Frontend
 * \param[in]	tunerIndex - the index of the Tuner in which this Frontend is located
 * \param[in]	channel - the Frontend channel:
 *				        - ETAL_FE_FOREGROUND for the foreground channel
 *				        - ETAL_FE_BACKGROUND for the background channel
 * \return		The Frontend handle, or ETAL_INVALID_HANDLE if *tunerIndex* is out of bounds
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_handleMakeFrontend(ETAL_HINDEX tunerIndex, ETAL_HINDEX channel)
{
	return ETAL_handleMake(handleTypeFrontend, tunerIndex, channel);
}

/***************************
 *
 * ETAL_handleIsTuner
 *
 **************************/
/*!
 * \brief		Checks if an ETAL_HANDLE describes a Tuner
 * \param[in]	hGeneric - the ETAL_HANDLE
 * \return		TRUE  - *hGeneric* describes a Tuner
 * \return		FALSE - *hGeneric* does **not** describe a Tuner
 * \callgraph
 * \callergraph
 */
tBool ETAL_handleIsTuner(ETAL_HANDLE hGeneric)
{
	return ETAL_handleGetType(hGeneric) == handleTypeTuner;
}

/***************************
 *
 * ETAL_handleTunerGetIndex
 *
 **************************/
/*!
 * \brief		Returns the 'index' bits of a Tuner handle
 * \details		The 'index' bits uniquely identify the Tuner **among other Tuners**.
 * \param[in]	hTuner - the ETAL_HANDLE
 * \return		The Tuner-unique index, or ETAL_INVALID_HINDEX if the *hTuner* does not
 * 				describe a Tuner.
 * \callgraph
 * \callergraph
 */
ETAL_HINDEX ETAL_handleTunerGetIndex(ETAL_HANDLE hTuner)
{
	ETAL_HINDEX retval;
	
	if (ETAL_handleGetType(hTuner) != handleTypeTuner)
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_INVALID_HINDEX;
	}
	else
	{
		retval = ETAL_handleGetIndex(hTuner);
	}
	return retval;
}

/***************************
 *
 * ETAL_handleIsReceiver
 *
 **************************/
/*!
 * \brief		Checks if an ETAL_HANDLE describes a Receiver
 * \param[in]	hGeneric - the ETAL_HANDLE
 * \return		TRUE  - *hGeneric* describes a Receiver
 * \return		FALSE - *hGeneric* does **not** describe a Receiver
 * \callgraph
 * \callergraph
 */
tBool ETAL_handleIsReceiver(ETAL_HANDLE hGeneric)
{
	return ETAL_handleGetType(hGeneric) == handleTypeReceiver;
}

/***************************
 *
 * ETAL_handleReceiverGetIndex
 *
 **************************/
/*!
 * \brief		Returns the 'index' bits of a Receiver handle
 * \details		The 'index' bits uniquely identify the Receiver **among other Receivers**.
 * \param[in]	hReceiver - the ETAL_HANDLE
 * \return		The Receiver-unique index, or ETAL_INVALID_HINDEX if the *hReceiver* does not
 * 				describe a Receiver.
 * \callgraph
 * \callergraph
 */
ETAL_HINDEX ETAL_handleReceiverGetIndex(ETAL_HANDLE hReceiver)
{
	ETAL_HINDEX retval;
	
	if (ETAL_handleGetType(hReceiver) != handleTypeReceiver)
	{
		retval = ETAL_INVALID_HINDEX;
	}
	else
	{
		retval = ETAL_handleGetIndex(hReceiver);
	}
	
	return retval;
}
 
/***************************
 *
 * ETAL_handleIsDatapath
 *
 **************************/
/*!
 * \brief		Checks if an ETAL_HANDLE describes a Datapath
 * \param[in]	hGeneric - the ETAL_HANDLE
 * \return		TRUE  - *hGeneric* describes a Datapath
 * \return		FALSE - *hGeneric* does **not** describe a Datapath
 * \callgraph
 * \callergraph
 */
tBool ETAL_handleIsDatapath(ETAL_HANDLE hGeneric)
{
	return ETAL_handleGetType(hGeneric) == handleTypeDatapath;
}

/***************************
 *
 * ETAL_handleDatapathGetReceiver
 *
 **************************/
/*!
 * \brief		Returns the handle of the Receiver to which a Datapath is attached
 * \details		A Datapath is attached to a Receiver at creation time. The Receiver
 * 				handle is encoded in the Datapath handle. This function extracts
 * 				the Receiver handle given a Datapath handle.
 * \param[in]	hDatapath - the ETAL_HANDLE
 * \return		The ETAL_HANDLE of the Receiver, or ETAL_INVALID_HANDLE if the *hDatapath* is not
 * 				a Datapath type.
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_handleDatapathGetReceiver(ETAL_HANDLE hDatapath)
{
	ETAL_HINDEX receiver_index;
	ETAL_HANDLE retval = ETAL_INVALID_HANDLE;

	if (ETAL_handleGetType(hDatapath) != handleTypeDatapath)
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_INVALID_HANDLE;
	}
	else
	{
		receiver_index = ETAL_handleDatapathGetReceiverIndex(hDatapath);
		if (receiver_index != ETAL_INVALID_HINDEX)
		{
			retval = ETAL_handleMakeReceiver(receiver_index);
		}	
	}
	return retval;
}

/***************************
 *
 * ETAL_handleDatapathGetReceiverIndex
 *
 **************************/
/*!
 * \brief		Returns the Receiver index of the Receiver to which a Datapath is attached
 * \details		A Datapath is attached to a Receiver at creation time. The Receiver
 * 				handle is encoded in the Datapath handle. This function extracts
 * 				the Receiver index given a Datapath handle.
 * \param[in]	hDatapath - the ETAL_HANDLE
 * \return		The 'index' of the Receiver, or ETAL_INVALID_HINDEX if the *hDatapath* is not
 * 				a Datapath type.
 * \callgraph
 * \callergraph
 */
ETAL_HINDEX ETAL_handleDatapathGetReceiverIndex(ETAL_HANDLE hDatapath)
{
	ETAL_HINDEX retval;

	if (ETAL_handleGetType(hDatapath) != handleTypeDatapath)
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_INVALID_HINDEX;
	}
	else
	{
		retval = ETAL_handleGetReceiverIndex(hDatapath);
	}	
	return retval;
}

/***************************
 *
 * ETAL_handleDatapathGetIndex
 *
 **************************/
/*!
 * \brief		Returns the 'index' bits of a Datapath handle
 * \details		The 'index' bits uniquely identify the Datapath **inside a Receiver**.
 * \param[in]	hDatapath - the ETAL_HANDLE
 * \return		The Receiver-unique index, or ETAL_INVALID_HINDEX if the *hDatapath* does not
 * 				describe a Datapath.
 * \callgraph
 * \callergraph
 */
ETAL_HINDEX ETAL_handleDatapathGetIndex(ETAL_HANDLE hDatapath)
{
	ETAL_HINDEX retval;

	if (ETAL_handleGetType(hDatapath) != handleTypeDatapath)
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_INVALID_HINDEX;
	}
	else
	{
		retval = ETAL_handleGetIndex(hDatapath);
	}
	return retval;
}

/***************************
 *
 * ETAL_handleIsMonitor
 *
 **************************/
/*!
 * \brief		Checks if an ETAL_HANDLE describes a Monitor
 * \param[in]	hGeneric - the ETAL_HANDLE
 * \return		TRUE  - *hGeneric* describes a Monitor
 * \return		FALSE - *hGeneric* does **not** describe a Monitor
 * \callgraph
 * \callergraph
 */
tBool ETAL_handleIsMonitor(ETAL_HANDLE hGeneric)
{
	return ETAL_handleGetType(hGeneric) == handleTypeMonitor;
}

/***************************
 *
 * ETAL_handleMonitorGetIndex
 *
 **************************/
/*!
 * \brief		Returns the 'index' bits of a Monitor handle
 * \details		The 'index' bits uniquely identify the Monitor **among other Monitors**.
 * \param[in]	hMonitor - the ETAL_HANDLE
 * \return		The Monitor-unique index, or ETAL_INVALID_HINDEX if the *hMonitor* does not
 * 				describe a Monitor.
 * \callgraph
 * \callergraph
 */
ETAL_HINDEX ETAL_handleMonitorGetIndex(ETAL_HANDLE hMonitor)
{
	ETAL_HINDEX retval;
	if (ETAL_handleGetType(hMonitor) != handleTypeMonitor)
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_INVALID_HINDEX;
	}
	else
	{
		retval = ETAL_handleGetIndex(hMonitor);
	}
	return retval;
}

/***************************
 *
 * ETAL_handleIsFrontend
 *
 **************************/
/*!
 * \brief		Checks if an ETAL_HANDLE describes a Frontend
 * \param[in]	hGeneric - the ETAL_HANDLE
 * \return		TRUE  - *hGeneric* describes a Frontend
 * \return		FALSE - *hGeneric* does **not** describe a Frontend
 * \callgraph
 * \callergraph
 */
tBool ETAL_handleIsFrontend(ETAL_HANDLE hGeneric)
{
	return ETAL_handleGetType(hGeneric) == handleTypeFrontend;
}

/***************************
 *
 * ETAL_handleFrontendGetTuner
 *
 **************************/
/*!
 * \brief		Returns the handle of the Tuner which contains the Frontend
 * \details		Frontend is part of only one Tuner. The Tuner
 * 				handle is encoded in the Frontend handle. This function extracts
 * 				the Tuner handle given a Frontend handle.
 * \param[in]	hFrontend - the ETAL_HANDLE
 * \return		The ETAL_HANDLE of the Receiver, or ETAL_INVALID_HANDLE if the *hDatapath* is not
 * 				a Datapath type.
 * \callgraph
 * \callergraph
 */
ETAL_HANDLE ETAL_handleFrontendGetTuner(ETAL_HANDLE hFrontend)
{
	ETAL_HANDLE retval = ETAL_INVALID_HANDLE;
	ETAL_HINDEX tuner_index;

	if (ETAL_handleGetType(hFrontend) != handleTypeFrontend)
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_INVALID_HANDLE;
	}
	else
	{
		tuner_index = ETAL_handleFrontendGetTunerIndex(hFrontend);
		if (tuner_index != ETAL_INVALID_HINDEX)
		{
			retval = ETAL_handleMakeTuner(tuner_index);
		}
	}
	return retval;
}

/***************************
 *
 * ETAL_handleFrontendGetTunerIndex
 *
 **************************/
/*!
 * \brief		Returns the Tuner index of the Tuner which includes the Frontend
 * \details		Frontend is part of only one Tuner. The Tuner
 * 				handle is encoded in the Frontend handle. This function extracts
 * 				the Tuner index given a Frontend handle.
 * \param[in]	hFrontend - the ETAL_HANDLE
 * \return		The 'index' of the Tuner, or ETAL_INVALID_HINDEX if the *hFrontend* is not
 * 				a Frontend type.
 * \callgraph
 * \callergraph
 */
ETAL_HINDEX ETAL_handleFrontendGetTunerIndex(ETAL_HANDLE hFrontend)
{
	ETAL_HINDEX retval;

	if (ETAL_handleGetType(hFrontend) != handleTypeFrontend)
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_INVALID_HINDEX;
	}
	else
	{
		/* ETAL_handleGetReceiverIndex is not a typo, the Tuner index in
		 * the Frontend handle uses the same bits as the Receiver index
		 * in the Datapath handle */
		 retval = ETAL_handleGetReceiverIndex(hFrontend);
	}
	return retval;
}

/***************************
 *
 * ETAL_handleFrontendGetChannel
 *
 **************************/
/*!
 * \brief		Returns the 'index' bits of a Frontend handle
 * \details		The 'index' bits uniquely identify the Frontend **inside a Tuner**.
 * 				The index is:
 *				        - ETAL_FE_FOREGROUND for the foreground channel
 *				        - ETAL_FE_BACKGROUND for the background channel
 * \param[in]	hFrontend - the ETAL_HANDLE
 * \return		The Tuner-unique index, or ETAL_INVALID_HINDEX if the *hFrontend* does not
 * 				describe a Frontend.
 * \callgraph
 * \callergraph
 */
ETAL_HINDEX ETAL_handleFrontendGetChannel(ETAL_HANDLE hFrontend)
{
	ETAL_HINDEX retval;
	
	if (ETAL_handleGetType(hFrontend) != handleTypeFrontend)
	{
		ASSERT_ON_DEBUGGING(0);
		retval = ETAL_INVALID_HINDEX;
	}
	else
	{
		retval = ETAL_handleGetIndex(hFrontend);
	}
	return retval;
}

#if 0
//!
//! \brief      <i><b> tVoid ETAL_TaskClearEvent (tU32 event) </b></i>
//! \details    Functions to clear possible pending event in the middlewar task on event.
//!           
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
tVoid ETAL_TaskClearEvent (OSAL_tEventHandle vI_eventHandler, tU32 event)
{

	// Clear old event if any (this can happen after a stop)
	 OSAL_s32EventPost (vI_eventHandler, 
                           (~((tU32)0x01 << event)), OSAL_EN_EVENTMASK_AND);
}

//!
//! \brief      <i><b> ETAL_TaskWakeUpOnEventFlag </b></i>
//! \details    Functions to wake-up the middlewar task on event.
//!             The main loop for the task is waked-up here.
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
tVoid ETAL_TaskWakeUpOnEventFlag (OSAL_tEventHandle vI_eventHandler, tU32 eventFlag)
{
	OSAL_s32EventPost (vI_eventHandler, 
                           eventFlag, OSAL_EN_EVENTMASK_OR);
}

#endif

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT

//!
//! \brief      <i><b> tVoid ETAL_CommMdr_TaskClearEventFlag (tU32 event) </b></i>
//! \details    Functions to clear possible pending event in the middlewar task on event.
//!           
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
tVoid ETAL_TaskClearEventFlag(OSAL_tEventHandle vI_eventHandler, tU32 eventFlag)
{

		 // Clear old event if any (this can happen after a stop)
	 OSAL_s32EventPost (vI_eventHandler, 
                           ~eventFlag, OSAL_EN_EVENTMASK_AND);
}

//!
//! \brief      <i><b> ETAL_CommMdr_TaskWakeUpOnEvent </b></i>
//! \details    Functions to wake-up the middlewar task on event.
//!             The main loop for the task is waked-up here.
//! \param[in]  tU32            Event 
//! \return     tVoid           None
//! \sa         n.a.
//! \callgraph
//! \callergraph
//!
tVoid ETAL_TaskWakeUpOnEvent (OSAL_tEventHandle vI_eventHandler, tU32 event)
{
	OSAL_s32EventPost (vI_eventHandler, 
                           ((tU32)0x01 << event), OSAL_EN_EVENTMASK_OR);
}
#endif


