//!  \file 		tunerdriver_internal.h
//!  \brief 	<i><b> Internal interface for CMOST driver. </b></i>
//!  \details   Internal interface for CMOST driver.
//!  $Author$
//!  \author 	(original version) Jean-Hugues Perrin
//!  $Revision$
//!  $Date$
//!
#ifndef TUNERDRIVER_INTERNAL_H
#define TUNERDRIVER_INTERNAL_H

#include "tunerdriver_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************
 * Macros
 *************************************/
 /*!
 * \def		TUNER_DRIVER_CMOST_THREAD_SCHEDULING
 * 			Voluntary sleep time in msec for various CMOST
 * 			related actions.
 *
 * 			Note that the CMOST communication is not really managed by a thread.
 */
#define TUNER_DRIVER_CMOST_THREAD_SCHEDULING            1

/*!
 * \def		TUNER_DRIVER_HEADER_RESERVED_CMOST
 * 			Some TUNER DRIVER interfaces for efficiency reasons
 * 			require that the data passed to the interface is
 * 			preceded by this number of unused bytes.
 * \see		TUNERDRIVER_writeRawBlock_CMOST
 */
#define TUNER_DRIVER_HEADER_RESERVED_CMOST  3
/*!
 * \def		CMOST_HEADER_LEN
 * 			Size in bytes of the CMOST command header part.
 * 			CMOST command is composed of a header of CMOST_HEADER_LEN bytes,
 * 			followed by at most #CMOST_PARAMETER_LEN bytes of parameters,
 * 			followed by #CMOST_CRC_LEN bytes of checksum.
 */
#define CMOST_HEADER_LEN                    3

/*!
 * \def		CMOST_PHY_HEADER_LEN_SPI
 * 			Size in bytes of the CMOST PHY HEADER when SPI access.
 *			the protocol to send data to CMOST is : 
 *			for CMD
 *			<PHY HEADER> <CMOST data : (CMOST HEADER + CMOST param + CHECKSUM)>
 * 			for write 
 *			<PHY HEADER> <data>
 */
#define CMOST_PHY_HEADER_LEN_SPI                 4

/*!
 * \def		CMOST_PHY_HEADER_LEN_I2C
 * 			Size in bytes of the CMOST PHY HEADER when I2C access.
 *			the protocol to send data to CMOST is : 
 *			for CMD
 *			<PHY HEADER> <CMOST data : (CMOST HEADER + CMOST param + CHECKSUM)>
 * 			for write 
 *			<PHY HEADER> <data>

 */
#define CMOST_PHY_HEADER_LEN_I2C                 3

/*!
 * \def		CMOST_PHY_HEADER_LEN_MAX
 * 			Maximum size in bytes of the CMOST PHY HEADER header part (I2C or SPI).
 */
#define CMOST_PHY_HEADER_LEN_MAX     (CMOST_PHY_HEADER_LEN_SPI)


/*!
 * \def		CMOST_CRC_LEN
 * 			Size in bytes of the CMOST command CRC part.
 * \see		CMOST_HEADER_LEN
 */
#define CMOST_CRC_LEN                       3
/*!
 * \def		CMOST_PARAMETER_LEN
 * 			Size in bytes of the CMOST command parameter part.
 * 			CMOST command language supports max 30 parameters, 3 bytes each
 * \see		CMOST_HEADER_LEN
 */
#define CMOST_PARAMETER_LEN                90
/*!
 * \def		CMOST_MAX_COMMAND_LEN
 * 			Max size in bytes of the complete CMOST command including
 * 			the command header, the parameters and the checksum
 */
#define CMOST_MAX_COMMAND_LEN             (CMOST_HEADER_LEN + CMOST_PARAMETER_LEN + CMOST_CRC_LEN)
/*!
 * \def		CMOST_MAX_RESPONSE_LEN
 * 			Max size in bytes of the complete CMOST response including
 * 			the command header, the parameters and the checksum
 */
#define CMOST_MAX_RESPONSE_LEN            (CMOST_HEADER_LEN + CMOST_PARAMETER_LEN + CMOST_CRC_LEN)

/*!
 * \def		CMOST_CHECKSUM_ERROR_FLAG
 * 			The CMOST response to every command contains a status indication
 * 			in the MSB of the header. This flag is set in case
 * 			the command was rejected due to checksum error.
 * \see		CMOST_CHECKSUM_ERROR
 */
#define CMOST_CHECKSUM_ERROR_FLAG        ((tU8)0x80)
/*!
 * \def		CMOST_WRONG_CID_ERROR_FLAG
 * 			The CMOST response to every command contains a status indication
 * 			in the MSB of the header. This flag is set in case
 * 			the command was rejected because unrecognized.
 * \see		CMOST_ILLEGAL_CID_ERROR
 */
#define CMOST_WRONG_CID_ERROR_FLAG       ((tU8)0x40)
/*!
 * \def		CMOST_CHECKSUM_ERROR
 * 			Macro that returns TRUE if its argument indicates the presence
 * 			of a checksum error. It must be used on the MSB of
 * 			the response header received from the CMOST.
 */
#define CMOST_CHECKSUM_ERROR(_buf)     (((_buf) & CMOST_CHECKSUM_ERROR_FLAG) == CMOST_CHECKSUM_ERROR_FLAG)
/*!
 * \def		CMOST_ILLEGAL_CID_ERROR
 * 			Macro that returns TRUE if its argument indicates the presence
 * 			of unknown command. It must be used on the MSB of
 * 			the response header received from the CMOST.
 */
#define CMOST_ILLEGAL_CID_ERROR(_buf)  (((_buf) & CMOST_WRONG_CID_ERROR_FLAG) == CMOST_WRONG_CID_ERROR_FLAG)

#ifdef CONFIG_COMM_DRIVER_EXTERNAL
/*!
 * \def		CMOST_HELPER_SEM_NAME
 * 			Semaphore name
 */
#define CMOST_HELPER_SEM_NAME         "Sem_CMOSTHelper"

/*!
 * \var		cmostHelperSendMsg_sem
 * 			Local semaphore used to build the command to be sent
 * 			to the external driver.
 * \todo	The buffer is unique across the system, it should be made
 * 			tuner-dependent to avoid race conditions in multiple-tuner
 * 			environment, in this case the semaphore can be removed.
 */
extern OSAL_tSemHandle cmostHelperSendMsg_sem;
#endif //#ifdef CONFIG_COMM_DRIVER_EXTERNAL

typedef struct
{
	tBool					   active;
	tyCMOSTDeviceConfiguration CMOSTDeviceConfiguration;
}
tyTunerDriverCMOSTDeviceConfiguration;

tBool TUNERDRIVER_IsDeviceActive(tU32 deviceID);
tyCommunicationBusType TUNERDRIVER_GetBusType(tU32 deviceID);
tU32 TUNERDRIVER_GetGPIOReset(tU32 deviceID);
tU32 TUNERDRIVER_GetGPIOIrq(tU32 deviceID);
tVoid *TUNERDRIVER_GetCommunicationBusConfig(tU32 deviceID);
tVoid *TUNERDRIVER_GetIRQCallbackFunction(tU32 deviceID);

#ifdef __cplusplus
}
#endif


#endif // TUNERDRIVER_INTERNAL_H
