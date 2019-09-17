//!
//!  \file 		 steci_defines.h
//!  \brief 	 <i><b> STECI global defines </b></i>
//!  \details This interface define all compilation specific defines that allows for target
//!           specific customization. Defines global to project are also defined here.
//!  \author  Alberto Saviotti
//!
#ifndef STECI_DEFINES_H
#define STECI_DEFINES_H

///
// If defined will introduce a timeout in the conversation
///
#define STECI_COMM_TIMEOUT_ENABLED
//#undef STECI_COMM_TIMEOUT_ENABLED

// Define error codes
#define STECI_STATUS_SUCCESS                ((tS32)0)   // Operation correctly executed
#define STECI_STATUS_ERROR                  ((tS32)-1)  // Generic error
#define STECI_STATUS_CMD_COMMAND_FAILURE    ((tS32)-2)
#define STECI_STATUS_DEV_NOT_RESPONDING     ((tS32)-3)
#define STECI_STATUS_PORT_NOT_OPENED        ((tS32)-4)
#define STECI_STATUS_INTERMEDIATE_FRAME     ((tS32)-5) // Used to indicate that a frame that is not the last has been received
#define STECI_STATUS_INVALID_LUN            ((tS32)-6)
#define STECI_STATUS_CMD_ONGOING            ((tS32)-7)
#define STECI_STATUS_DEV_NOT_READY          ((tS32)-8)
#define STECI_STATUS_CMD_PARAMETER_WRONG    ((tS32)-9)
#define STECI_STATUS_CMD_NOT_SUPPORTED      ((tS32)-10)
#define STECI_STATUS_INVALID_HANDLER        ((tS32)-11)
#define STECI_STATUS_NO_RESOURCES           ((tS32)-12)
#define STECI_STATUS_DATA_NOT_PRESENT       ((tS32)-14) // USed to indicate that a zero content frame has been received
#define STECI_STATUS_HEADER_RE_TX_FLAG      ((tS32)-15) // Used to indicate re-tx flag raised
#define STECI_STATUS_HEADER_PARITY_ERROR    ((tS32)-16) // Used to indicate that received header is corrupted
#define STECI_STATUS_DATA_CRC_ERROR         ((tS32)-17) // Used to indicate that received data is corrupted
#define STECI_STATUS_HOST_DEVICE_CLASH      ((tS32)-19) // Used to indicate both thinking to rx

// Invalid codes
#define STECI_INVALID_HANDLER               (NULL)

// Special lunes
#define STECI_INVALID_LUN                   ((tU8)0x00)

// Bit status
#define STECI_BIT_SET                       1
#define STECI_BIT_CLEAR                     0

#ifdef CONFIG_HOST_OS_TKERNEL
	#define ETAL_COMM_STECI_STACK_SIZE              (16*1024)
#else
	#define ETAL_COMM_STECI_STACK_SIZE              4096
#endif

#define ETAL_COMM_STECI_THREAD_PRIORITY        (OSAL_C_U32_THREAD_PRIORITY_NORMAL - 1)

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT_STECI

#ifdef CONFIG_HOST_OS_TKERNEL
	#define ETAL_COMM_STECI_IRQ_STACK_SIZE              (1024)
#else
	#define ETAL_COMM_STECI_IRQ_STACK_SIZE              1024
#endif
#define ETAL_COMM_STECI_IRQ_THREAD_PRIORITY        (ETAL_COMM_STECI_THREAD_PRIORITY - 1)

#endif


/*!
 * \def		ETAL_COMM_STECI_THREAD_NAME
 * 			Thread name for the
 * 			#STECI_ProtocolHandle
 */
#define ETAL_COMM_STECI_THREAD_NAME            "ETAL_COMM_STECI"
/*!
 * \def		ETAL_COMM_STECI_THREAD_NAME
 * 			Thread name for the
 * 			#STECI_ProtocolHandle
 */
#define ETAL_COMM_STECI_EVENT_REQ_THREAD_NAME            "ETAL_STECI_REQ"

#endif // STECI_DEFINES_H

// End of file

