//!
//!  \file 		etaldefs.h
//!  \brief 	<i><b> ETAL definitions </b></i>
//!  \details   ETAL system-wide definition
//!
//!				This file lists also various thread-related definitions.
//! 			Note that not all of the threads listed here are actually
//! 			created, the existence of some depends on the build-time options.
//!
//! 			For the list of all the possible threads created in the system
//! 			and the correspondence between symbolical names and thread
//! 			function entry, see under **Thread names**.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!

/*************************************
 * Timeouts
 *************************************/

/*!
 * \def		ETAL_TO_MDR_CMD_TIMEOUT_IN_MSEC
 * 			Time waited for a response after sending a command to
 * 			the DAB DCOP. If exeeded the communication function
 * 			returns a timeout error.
 *
 * 			Time in msec.
 */
#define ETAL_TO_MDR_CMD_TIMEOUT_IN_MSEC          4000
/*!
 * \def		ETAL_TO_MDR_SPECIAL_CMD_TIMEOUT_IN_MSEC
 * 			Same as #ETAL_TO_MDR_CMD_TIMEOUT_IN_MSEC but for
 * 			DAB DCOP special commands. Special commands in this
 * 			context are commands for flashing the DAB DCOP.
 *
 * 			Time in msec.
 */
#define ETAL_TO_MDR_SPECIAL_CMD_TIMEOUT_IN_MSEC  160000
/*!
 * \def		ETAL_TO_MDR_RETRY_TIMEOUT_IN_MSEC
 * 			In some conditions it may happen that ETAL needs to
 * 			re-send a command to the DAB DCOP. This is the amount
 * 			of time to wait before re-sending the command.
 *
 * 			Time in msec.
 * \see		ETAL_cmdGetQuality_MDR
 */
//#define ETAL_TO_MDR_RETRY_TIMEOUT_IN_MSEC        200
#define ETAL_TO_MDR_RETRY_TIMEOUT_IN_MSEC        500

/*!
 * \def		ETAL_TO_CMOST_CMD_TIMEOUT_IN_MSEC
 * 			Max amount of time in milliseconds that the ETAL CMOST
 * 			driver is allowed to wait for the TUNER DRIVER
 * 			to deliver a command to the CMOST and provide an answer.
 */
#define ETAL_TO_CMOST_CMD_TIMEOUT_IN_MSEC         500
/*!
 * \def		ETAL_TO_CMOST_CMD_TIMEOUT_EXT_IN_MSEC
 * 			In case of CONFIG_COMM_DRIVER_EXTERNAL CMOST driver implementation
 * 			this is the max amount of time to wait for the external
 * 			driver to return an answer to ETAL
 */
#define ETAL_TO_CMOST_CMD_TIMEOUT_EXT_IN_MSEC    2500


/*
 * HD tune command waits ETAL_HD_DELAY_BEFORE_HD_SYNC
 * before checking the digital acquisition status for HD flag
 * Then it waits aother ETAL_HD_DELAY_BEFORE_AUDIO_SYNC
 * before starting to poll the digital acquisition status for AUDIO flag
 * and keeps polling it every ETAL_HD_AUDIO_SYNC_POLL_TIME
 * until either AUDIO is aquired or ETAL_HD_TUNE_TIMEOUT
 * from the start of the tune command have passed
 */

/*!
 * \def		ETAL_HD_DELAY_BEFORE_HD_SYNC
 * 			Time to wait in msec after sending the Tune
 * 			command to the HD Radio DCOP before checking
 * 			the Digital acquisition status for the HD Sync flag.
 * 			The HD Sync flag indicates there HD signal on the
 * 			tuned frequency.
 */
#define ETAL_HD_DELAY_BEFORE_HD_SYNC             800

/*!
 * \def		ETAL_HD_DELAY_BEFORE_AUDIO_SYNC
 * 			Time to wait in msec after #ETAL_HD_DELAY_BEFORE_HD_SYNC
 * 			before starting to poll the Digital acquisition status
 * 			of the HD Radio DCOP for the Audio flag.
 * 			This delay is defined to avoid loading the
 * 			communication bus with useless commands (because
 * 			the audio acquisition requires an almost
 * 			fixed amount of time)
 */
#define ETAL_HD_DELAY_BEFORE_AUDIO_SYNC         4200
/*!
 * \def		ETAL_HD_AUDIO_SYNC_POLL_TIME
 * 			Time in msec between Digital acquisition status
 * 			polls.
 * 			ETAL continues polling the Digital acquisition
 * 			status until AUDIO is aquired or the #ETAL_HD_TUNE_TIMEOUT
 * 			expires.
 */
#define ETAL_HD_AUDIO_SYNC_POLL_TIME             100
/*!
 * \def		ETAL_HD_TUNE_TIMEOUT_AAA_DISABLED
 * 			Max time in msec that ETAL waits for the AUDIO
 * 			acquisition flag from the Tune command. If this
 * 			amount of time passes without receiving the AUDIO
 * 			flag, the HD Radio digital audio is considered
 * 			not available for the current station.
 *			Value depends if AAA is activated or not
 *			
 */
#if defined CONFIG_COMM_DRIVER_EXTERNAL
	/* EXTERNAL driver has long round trip delays for commands
	 * to DCOP so the timeout needs to be increased */
	#define ETAL_HD_TUNE_TIMEOUT_AAA_DISABLED        7700
#else
	#define ETAL_HD_TUNE_TIMEOUT_AAA_DISABLED        7000
#endif

/*!
 * \def		ETAL_HD_TUNE_TIMEOUT_AAA_ENABLED
 * 			Max time in msec that ETAL waits for the AUDIO
 * 			acquisition flag from the Tune command. If this
 * 			amount of time passes without receiving the AUDIO
 * 			flag, the HD Radio digital audio is considered
 * 			not available for the current station.
 *			Value depends if AAA is activated or not
 *			When AAA is enabled, spec RX_TN_4013 indicates 20s
 *			
 */
#if defined CONFIG_COMM_DRIVER_EXTERNAL
	/* EXTERNAL driver has long round trip delays for commands
	 * to DCOP so the timeout needs to be increased */
	#define ETAL_HD_TUNE_TIMEOUT_AAA_ENABLED        20700
#else
	#define ETAL_HD_TUNE_TIMEOUT_AAA_ENABLED        20000
#endif

/*!
 * \def		ETAL_HD_TUNE_STATUS_MONITORING
 * 			Time in msec between TUNE  status
 * 			polls.
 * 			ETAL is periodically polling for tune status change 
 */
#define ETAL_HD_TUNE_STATUS_MONITORING             100

/*!
 * \def		ETAL_HD_BLEND_STATUS_CMOST_PERIOD
 * 			Time in msecfor sending the HD blend status info to CMOST
 * 			ETAL is periodically sending the HD blend gain to CMOST 
 */
#define ETAL_HD_BLEND_STATUS_CMOST_PERIOD          1000

/*!
 * \def		ETAL_HD_T_AD_GAIN_STEP_MS
 * 			Analogue to digital blending time in miliseconds 
 * 			Value is sent to CMOST 
 */

#define ETAL_HD_T_AD_GAIN_STEP_MS 				500

/*!
 * \def		ETAL_HD_T_AD_GAIN_STEP_MS
 * 			Digital to Analog blending time in miliseconds 
 * 			Value is sent to CMOST 
 */

#define ETAL_HD_T_DA_GAIN_STEP_MS 				100

/*!
 * \def		ETAL_HD_AV_GAIN_DB
 * 			Analogue source level gain in dB
 * 			Value is sent to CMOST 
 */

#define ETAL_HD_AV_GAIN_DB	 				0




/*************************************
 * Data sizes
 *************************************/

/*!
 * \def		ETAL_MAX_RESPONSE_LEN_CONTROL_LUN
 *
 * 			The max size of the data expected on the CONTROL_LUN
 * 			for DAB DCOP.
 * 			This size depends only on the DCOP protocol.
 */
#define ETAL_MAX_RESPONSE_LEN_CONTROL_LUN       1024
/*!
 * \def		ETAL_MAX_RESPONSE_LEN_DATA_LUN
 *
 * 			The max size of the data expected on the DATA_LUN.
 * 			This size depends on the DAB DCOP configuration (in particular
 * 			which data services are activated) and on the broadcaster;
 * 			for example, a Slideshow (SLS) image may be several
 * 			10th of kbytes.
 */
#define ETAL_MAX_RESPONSE_LEN_DATA_LUN          32768
/*!
 * \def		ETAL_MAX_RESPONSE_LEN_AUTONOTIF_LUN
 *
 * 			The max size of the data expected on the AUTONOTIF_LUN.
 * 			This size depends only on the DCOP protocol.
 *
 * \todo	This value is currently unused.
 */
#define ETAL_MAX_RESPONSE_LEN_AUTONOTIF_LUN     1024
/*!
 * \def		ETAL_MAX_RESPONSE_LEN
 *
 * 			The max of the previous three defines 
 * 			(#ETAL_MAX_RESPONSE_LEN_CONTROL_LUN, #ETAL_MAX_RESPONSE_LEN_DATA_LUN
 * 			and #ETAL_MAX_RESPONSE_LEN_AUTONOTIF_LUN).
 * 			Used to allocate generic buffers where it cannot
 * 			be defined aforehand the type of data being received.
 */
#define ETAL_MAX_RESPONSE_LEN                  (ETAL_MAX_RESPONSE_LEN_DATA_LUN)
/*!
 * \def		ETAL_HD_MAX_PROGRAM_NUM
 * 			An HD Radio program is composed of one Main Program Service
 * 			and up to 7 Supplemental Program Services.
 * 			This limit comes from the HD Radio specifications to 
 * 			it should not be changed.
 */
#define ETAL_HD_MAX_PROGRAM_NUM                    8
/*!
 * \def		ETAL_HD_MAX_RESPONSE_LEN
 *
 * 			This define the maximum size of PSD data received in answer to PSD Decode command.
 * 			Worst-case scenario is all PSD fields/sub-fields at their maximum length
 *			Each field/sub-field will be given in a "frame" of (field length + 7 bytes overhead)
 * 			There are 12 fields and sub-fields possible
 *                  UFID and XHDR frames are always repeated four times
 *			So that is 18 max frames, with 12 of field length 0x7F and 6 of field length 0xFF
 */
#define ETAL_HD_MAX_RESPONSE_LEN                   3180
/* 
 * Size of the FIFO between IPForward thread and ETAL COMM thread
 * TCP/IP may concatenate several SSI32 messages together and they will all be pushed
 * onto the FIFO before the ETAL can process them
 */
#define PROTOCOL_LAYER_FIFO_SIZE           64

/*************************************
 * Thread stack size
 *************************************/

#define ETAL_IPFORWARD_STACK_SIZE               (16*1024)
#define ETAL_COMM_MDR_STACK_SIZE                (16*1024)
#define ETAL_COMM_HDRADIO_STACK_SIZE            (16*1024)
#define ETAL_CALLBACK_STACK_SIZE                (16*1024)
#define ETAL_DATA_STACK_SIZE                    (16*1024)
#define ETAL_CONTROL_STACK_SIZE                 (16*1024)
#define ETAL_IRQ_STACK_SIZE                     (16*1024)
#define ETAL_TRACE_STACK_SIZE                   (16*1024)

/*************************************
 * Thread priorities
 *************************************/

#define ETAL_IPFORWARD_THREAD_PRIORITY         (OSAL_C_U32_THREAD_PRIORITY_NORMAL)
#define ETAL_COMM_MDR_THREAD_PRIORITY          (OSAL_C_U32_THREAD_PRIORITY_NORMAL - 1)
#define ETAL_COMM_HDRADIO_THREAD_PRIORITY      (OSAL_C_U32_THREAD_PRIORITY_NORMAL)
#define ETAL_CALLBACK_THREAD_PRIORITY          (OSAL_C_U32_THREAD_PRIORITY_NORMAL)
#define ETAL_DATA_THREAD_PRIORITY              (OSAL_C_U32_THREAD_PRIORITY_NORMAL - 1)
#define ETAL_CONTROL_THREAD_PRIORITY           (OSAL_C_U32_THREAD_PRIORITY_NORMAL)
#define ETAL_IRQ_THREAD_PRIORITY               (OSAL_C_U32_THREAD_PRIORITY_NORMAL)
#define ETAL_TRACE_THREAD_PRIORITY             (OSAL_C_U32_THREAD_PRIORITY_LOWEST)

/*************************************
 * Thread names
 *************************************/

/*
 * Names containing _BASE_ will be run-time extended with a numerical value
 */

/*!
 * \def		ETAL_IPFORWARD_THREAD_BASE_NAME
 * 			Thread name for the
 * 			#TcpIpProtocolHandle
 */
#define ETAL_IPFORWARD_THREAD_BASE_NAME        "ETAL_IPFORW"
/*!
 * \def		ETAL_COMM_MDR_THREAD_NAME
 * 			Thread name for the
 * 			#ETAL_CommunicationLayer_ThreadEntry_MDR
 */
#define ETAL_COMM_MDR_THREAD_NAME              "ETAL_COMM_MDR"

/*!
 * \def		ETAL_COMM_HDRADIO_THREAD_NAME
 * 			Thread name for the
 * 			#ETAL_CommunicationLayer_ThreadEntry_HDRADIO
 */
#define ETAL_COMM_HDRADIO_THREAD_NAME          "ETAL_COMM_HD"
/*!
 * \def		ETAL_CALLBACK_THREAD_BASE_NAME
 * 			Thread name for the
 * 			#ETAL_CallbackHandler_ThreadEntry
 */
#define ETAL_CALLBACK_THREAD_BASE_NAME         "ETAL_CALLBACK"
/*!
 * \def		ETAL_DATA_THREAD_BASE_NAME
 * 			Thread name for the
 * 			#ETAL_DataHandler_ThreadEntry
 */
#define ETAL_DATA_THREAD_BASE_NAME             "ETAL_DATA"
/*!
 * \def		ETAL_CONTROL_THREAD_NAME
 * 			Thread name for the
 * 			#ETAL_Control_ThreadEntry
 */
#define ETAL_CONTROL_THREAD_NAME               "ETAL_CONTROL"
/*!
 * \def		ETAL_IRQ_THREAD_NAME
 * 			Thread name for the
 * 			#ETAL_IRQ_ThreadEntry
 */
#define ETAL_IRQ_THREAD_NAME                   "ETAL_IRQ"
/*!
 * \def		ETAL_TRACE_THREAD_NAME
 * 			Thread name for the
 * 			#ETAL_tracePrint_ThreadEntry
 */
#define ETAL_TRACE_THREAD_NAME                 "ETAL_TRACE"

/*!
 * \def		ETALTML_LEARN_THREAD_NAME
 * 			Thread name for the
 * 			#ETAL_learn_WaitingTask
 */
#define ETALTML_LEARN_THREAD_NAME  "ETALTML_learn"
/*!
 * \def     ETALTML_SCAN_THREAD_NAME
 *          Thread name for the
 *          #ETAL_scan_WaitingTask
 */
#define ETALTML_SCAN_THREAD_NAME  "ETALTML_scan"
/*!
 * \def		ETALTML_SERVICE_FOLLOWING_THREAD_NAME
 * 			Thread name for the
 * 			#ETALTML_ServiceFollowing_WaitingTask
 */
#define ETALTML_SERVICE_FOLLOWING_THREAD_NAME  "ETALTML_SF"

/*!
 * \def		ETALTML_RDS_SEEK_THREAD_NAME
 * 			Thread name for the
 * 			#ETAL_RdsSeek_WaitingTask
 */
 #define ETALTML_RDS_SEEK_THREAD_NAME  "ETALTML_RdsSeek"

/*!
 * \def		ETAL_THREAD_BASE_NAME_MAX_LEN
 * 			The max size of thread name, used for names created run-time
 * 			from a _BASE_NAME
 * \remark	must be equal or greter than the longest _BASE_NAME number of 
 * 			characters plus one chatacter (a digit) plus one null byte.
 */
#define ETAL_THREAD_BASE_NAME_MAX_LEN           (OSAL_C_U32_MAX_NAMELENGTH - 1)

/*!
 * \def		ETAL_EVENT_BASE_NAME_MAX_LEN
 * 			The max size of event name, used for names created run-time
 * 			from a _BASE_NAME
 * \remark	must be equal or greter than the longest _BASE_NAME number of 
 * 			characters plus one chatacter (a digit) plus one null byte.
 */
#define ETAL_EVENT_BASE_NAME_MAX_LEN           (OSAL_C_U32_MAX_NAMELENGTH - 1)


/*************************************
 * Thread sleep
 *************************************/

/*
 * The following define how much each thread voluntarily sleeps between
 * successive executions and other similar conditions.
 */

#define ETAL_COMM_MDR_THREAD_SCHEDULING         1
#define ETAL_COMM_HDRADIO_THREAD_SCHEDULING    10
/*!
 * \def		ETAL_API_THREAD_SCHEDULING
 * 			Voluntary sleep time in msec for several actions
 * 			involving the external API user
 */
#define ETAL_API_THREAD_SCHEDULING             10
/*!
 * \def		ETAL_DATAHANDLER_THREAD_SCHEDULING
 * 			Voluntary sleep time in msec for
 * 			#ETAL_DataHandler_ThreadEntry
 * 			
 * \remark	This time should be kept low because if ETAL is
 * 			required to process many  RAW audio streams
 * 			the FIFO should be emptied quickly.
 */
#define ETAL_DATAHANDLER_THREAD_SCHEDULING      1
#define ETAL_CALLBACK_THREAD_SCHEDULING        10
#define ETAL_CONTROL_THREAD_SCHEDULING         10
/*!
 * \def		ETAL_POLL_MONITOR_SCHEDULING
 * 			Voluntary sleep time in msec for ETAL monitors
 */
#define ETAL_POLL_MONITOR_SCHEDULING            (ETAL_CONTROL_THREAD_SCHEDULING)
#define ETAL_IRQ_THREAD_SCHEDULING            300

#define ETAL_TRACE_THREAD_SCHEDULING            1

/*************************************
 * Internal Periodic Callback period
 *************************************/

#define ETAL_RDS_INTCB_DELAY               87
#define ETAL_RDS_FAST_PI_INTCB_DELAY       50
#ifdef CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
	// need to have higher seek polling info :
	// for perf & for algo seek/timing SF for measurements.
	#define ETAL_SEEK_INTCB_DELAY          50
#else
	#define ETAL_SEEK_INTCB_DELAY         300
#endif // CONFIG_ETALTML_HAVE_SERVICE_FOLLOWING
#define ETAL_PADRADIOTEXT_INTCB_DELAY     500
#if defined (CONFIG_COMM_DRIVER_EXTERNAL)
	/* EXTERNAL driver has long round trip delays for commands
	 * to DCOP so the timeout needs to be increased */
	#define ETAL_HDRADIOPSD_INTCB_DELAY   800
#else
	#define ETAL_HDRADIOPSD_INTCB_DELAY   500
#endif

/*************************************
 * Callback thread configuration
 *************************************/

/*!
 * \def		ETAL_CALLBACK_FIFO_ELEMENTS
 * 			The ETAL API user callbacks are invoked by one (or more) dedicated
 * 			threads to avoid locking all of ETAL if one callback takes too long to
 * 			execute. The thread(s) communicate with ETAL through FIFOs.
 * 			This defines the number of elements in each fifo used by
 * 			the callback handler threads.
 */
#define ETAL_CALLBACK_FIFO_ELEMENTS         10
/*!
 * \def		ETAL_CALLBACK_HANDLERS_NUM
 * 			Number of threads handling the callbacks.
 * 			Only 1 thread was tested.
 */
#define ETAL_CALLBACK_HANDLERS_NUM          1
/*!
 * \def		ETAL_COMM_EVENT_CALLBACK_HANDLER
 * 			Index of the thread handling the user event callback.
 *
 * 			Must range from 0 to #ETAL_CALLBACK_HANDLERS_NUM excluded.
 *
 * 			Currenlty only one thread is defined so all indexes are the same.
 * \see		ETAL_CALLBACK_FIFO_ELEMENTS
 */
#define ETAL_COMM_EVENT_CALLBACK_HANDLER    0
/*!
 * \def		ETAL_COMM_QUALITY_CALLBACK_HANDLER
 * 			Index of the thread handling the user quality callback.
 *
 * 			Must range from 0 to #ETAL_CALLBACK_HANDLERS_NUM excluded.
 *
 * 			Currenlty only one thread is defined so all indexes are the same.
 * \see		ETAL_CALLBACK_FIFO_ELEMENTS
 */
#define ETAL_COMM_QUALITY_CALLBACK_HANDLER  0

/*************************************
 * Datahandler thread configuration
 *************************************/

/*!
 * \def		ETAL_DATAHANDLER_FIFO_ELEMENTS
 * 			The ETAL API user data handler callbacks are invoked by one (or more) dedicated
 * 			threads to avoid locking all of ETAL if one callback takes too long to
 * 			execute. The thread(s) communicate with ETAL through FIFOs.
 * 			This defines the number of elements in each fifo used by
 * 			the callback handler threads.
 */
#define ETAL_DATAHANDLER_FIFO_ELEMENTS    30
/*!
 * \def		ETAL_DATA_HANDLERS_NUM
 * 			Number of threads handling the Data handlers.
 * 			Only 1 thread was tested.
 * \see		ETAL_DATAHANDLER_FIFO_ELEMENTS
 */
#define ETAL_DATA_HANDLERS_NUM            1
/*!
 * \def		ETAL_COMM_DATA_CALLBACK_HANDLER
 * 			Index of the thread handling the data
 * 			Must range from 0 to #ETAL_DATA_HANDLERS_NUM excluded.
 * \see		ETAL_DATAHANDLER_FIFO_ELEMENTS
 */
#define ETAL_COMM_DATA_CALLBACK_HANDLER    0

/*************************************
 * Semaphore names
 *************************************/

/*
 * Names containing _BASE_ will be run-time extended with a numerical value
 */

/*!
 * \def		ETAL_SEM_GLOBAL_EXTERNAL
 * 			Name of the semaphore used to block access to the ETAL API.
 * 			Used to avoid multiple accesses from the ETAL API user and thus
 * 			obtain system-exclusiveness.
 */
#define ETAL_SEM_GLOBAL_EXTERNAL           "Sem_etalApiStatus"
/*!
 * \def		ETAL_SEM_GLOBAL_INTERNAL
 * 			Name of the semaphore used to lock the access to the #etalStatus.
 * 			This lock is a subset of the lock obtained with #ETAL_SEM_GLOBAL_EXTERNAL,
 * 			which also locks the external API.
 */
#define ETAL_SEM_GLOBAL_INTERNAL           "Sem_etalInternalStatus"
/*!
 * \def		ETAL_SEM_RECEIVER_BASE
 * 			Name of the semaphore used to lock a Receiver's internal state.
 * 			There is one such semaphore per each Receiver supported by the system.
 */
#define ETAL_SEM_RECEIVER_BASE             "Sem_etalRecv"
/*
 * \def		ETAL_SEM_TUNER_BASE
 * 			Name of the semaphore used to lock access to a Tuner's hardware
 * 			bus.
 * 			There is one such semaphore per each Tuner supported by the system.
 */
#define ETAL_SEM_TUNER_BASE                "Sem_etalTuner"
/*!
 * \def		ETAL_SEM_TUNE_STATUS
 * 			Name of the semaphore used to lock access to a particular sub-section
 * 			of the #etalStatus.
 */
#define ETAL_SEM_TUNE_STATUS               "Sem_etalTuneStatus"
/*!
 * \def		ETAL_SEM_LEARN
 * 			Name of the semaphore used for Learn state machine synchronization.
 */
#define ETAL_SEM_LEARN                     "Sem_etalLearn"
/*!
 * \def     ETAL_SEM_SCAN
 *          Name of the semaphore used for Scan state machine synchronization.
 */
#define ETAL_SEM_SCAN                     "Sem_etalScan"
/*!
 * \def		ETAL_SEM_CMD_QUEUE_MDR
 * 			Name of the semaphore used to lock access to the DAB DCOP
 * 			command FIFO. ETAL supports only one command at a time to the
 * 			DAB DCOP (but several responses).
 */
#define ETAL_SEM_CMD_QUEUE_MDR             "Sem_etalToMDRCmd"

/*!
 * \def		ETAL_SEM_COMM_MDR_MSG_TX
 * 			Name of the semaphore used to sync thread for DAB messages
 *			Inidcate the message has been transmitted to DCOP, 
 *			Answer is awaited
 * 
 */
#define ETAL_SEM_COMM_MDR_MSG_TX             "Sem_comMDR_Tx"

/*!
 * \def		ETAL_SEM_COMM_MDR_MSG_COMPLETE
 * 			Name of the semaphore used to sync thread for DAB messages
 *			Inidcate the message has been transmitted to DCOP, 
 *			and notification and response received 
 */
#define ETAL_SEM_COMM_MDR_MSG_COMPLETE       "Sem_comMDR_Complete"


/*!
 * \def		ETAL_SEM_DEVICE_HDRADIO
 * 			Name of the semaphore used to lock the access to the HD Radio DCOP
 * 			hardware bus.
 */
#define ETAL_SEM_DEVICE_HDRADIO            "Sem_etalStatusHDRADIODev"
/*!
 * \def		ETAL_SEM_TUNEFSM_HDRADIO_BASE
 * 			Name of the semaphore used for HD Radio Tune state machine synchronization.
 */
#define ETAL_SEM_TUNEFSM_HDRADIO_BASE      "Sem_etalTune_HDRADIO"

/*!
 * \def		ETAL_SEM_RECLIST_HDRADIO_BASE
 * 			Name of the semaphore used for HD Radio Receiver Monitoring array protection.
 */
#define ETAL_SEM_RECLIST_HDRADIO_BASE      "Sem_RecList_HDRADIO"


/*!
 * \def		ETAL_SEM_NAME_LEN_HDRADIO
 * 			Semaphore name length
 */
#define ETAL_SEM_NAME_LEN_HDRADIO           24

/*************************************
 * FIFO names
 *************************************/

#define ETAL_FIFO_CALLBACK_NAME            "Callback"
#define ETAL_FIFO_DATAHANDLER_BASE_NAME    "Datahandler"
#define ETAL_FIFO_HDRADIO_NAME             "HDRadioFifo"

/*************************************
 * EVENT names
 *************************************/
#define ETAL_EVENT_HANDLER_COMM_MDR					"ETAL_COM_MDR_EventHandler"
#define ETAL_EVENT_HANDLER_CONTROL					"ETAL_control_EventHandler"
#define ETAL_EVENT_HANDLER_CONTROL_RDS_IRQ			"ETAL_RdsIrq_EventHandler"
#define ETAL_EVENT_HANDLER_DATAHANDLER_BASE_NAME	"ETAL_DH_Event_"
#define ETAL_EVENT_HANDLER_CALLBACK_BASE_NAME		"ETAL_callback_Event_"
#define ETAL_EVENT_HANDLER_STECI					"ETAL_STECI_EventHandler"


