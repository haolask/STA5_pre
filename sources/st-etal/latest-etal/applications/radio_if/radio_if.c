//!
//!  \file		radio_if.c
//!  \brief		<i><b> Radio interface for Accordo radio tuners</b></i>
//!  \details  Radio interface application start and stop handling
//!  \author	David Pastor
//!

#include "target_config.h"

#include "osal.h"

#include "etal_api.h"
#include "etaltml_api.h"


#include "defines.h"
#include "steci_defines.h"

#include "etaldefs.h"
#include "ipfcomm.h"
#include "TcpIpProtocol.h"
#include "DAB_Protocol.h"

#include "mcp_protocol.h"
#include "connection_modes.h"
#include "steci_protocol.h"
#include "steci_uart_protocol.h"
#include "uart_mngm.h"
#include "rif_msg_queue.h"
#include "rif_protocol_router.h"
#include "rif_rimw_protocol.h"
#include "rif_etalapi_cnv.h"

//#include <assert.h>
//#include <errno.h>
//#include <fcntl.h>
#include <getopt.h>
//#include <poll.h>
#include <signal.h>
//#include <stdbool.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <termios.h>
//#include <unistd.h>

//#include <linux/fb.h>

//#include <sys/mman.h>
//#include <sys/select.h>
//#include <sys/stat.h>
//#include <sys/time.h>
//#include <sys/types.h>

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
enum {
	RIF_OPT_ANALOG = 0x10000,
	RIF_OPT_DIGITAL_FM,
	RIF_OPT_DIGITAL_DAB,
	RIF_OPT_LOAD
};
#define RIF_SEM_WAIT_EXIT       "Sem_rifWaitTermSig"

#define RIF_MAX_ETAL_RECEIVERS 2
#define RIF_MAX_ETAL_DATAPATHS 2	// TODO: to be verified

#define RIF_UART_ID             0
#define RIF_UART_SPEED          B115200

#define RIF_THREAD_MAX                  4
/*!
 * \def		RIF_THREAD_SCHEDULING
 * 			Voluntary sleep time in msec for several actions
 * 			involving the external API user
 */
#define RIF_THREAD_SCHEDULING               10

/*************************************
 * Thread priorities
 *************************************/
#define THD_COMM_MCP_THREAD_PRIORITY                    (OSAL_C_U32_THREAD_PRIORITY_NORMAL - 1)
#define THD_COMM_STECI_UART_THREAD_PRIORITY             (OSAL_C_U32_THREAD_PRIORITY_NORMAL - 1)
#define THD_RIF_PROTOCOL_ROUTER_THREAD_PRIORITY         (OSAL_C_U32_THREAD_PRIORITY_NORMAL - 1)
#define THD_RIF_RIMW_THREAD_PRIORITY                    (OSAL_C_U32_THREAD_PRIORITY_NORMAL - 1)

/*************************************
 * Thread stack size
 *************************************/
#ifdef CONFIG_HOST_OS_TKERNEL
	#define THD_MCP_STACK_SIZE                      (16*1024)
	#define THD_STECI_UART_STACK_SIZE               (16*1024)
	#define THD_RIF_PROTOCOL_ROUTER_STACK_SIZE      (16*1024)
	#define THD_RIF_RIMW_STACK_SIZE                 (16*1024)
#else
	#define THD_MCP_STACK_SIZE                      4096
	#define THD_STECI_UART_STACK_SIZE               4096
	#define THD_RIF_PROTOCOL_ROUTER_STACK_SIZE      4096
	#define THD_RIF_RIMW_STACK_SIZE                 4096
#endif

#define ETAL_DCOP_LOADER_BOOTSTRAP_FILENAME  "xloader_spiloader.bin"
#define ETAL_DCOP_SECTIONS_DESCR_FILENAME    "dcop_fw_sections.cfg"

#define ETAL_DCOP_BOOT_FILENAME_MAX_SIZE      256

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
static tChar ETALDcop_bootstrap_filename[ETAL_DCOP_BOOT_FILENAME_MAX_SIZE] = ETAL_DCOP_LOADER_BOOTSTRAP_FILENAME;
static tChar ETALDcop_sections_filename[ETAL_DCOP_BOOT_FILENAME_MAX_SIZE] = ETAL_DCOP_SECTIONS_DESCR_FILENAME;
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
static tChar *ETALDcop_dcopLoading_filename[1]={ETALDcop_bootstrap_filename};
#endif
#endif

/*************************************
 * Thread names
 *************************************/

/*
 * Names containing _BASE_ will be run-time extended with a numerical value
 * Limit to 7 characters + zero ending character
 */

/*!
 * \def		THD_COMM_MCP_THREAD_NAME
 * 			Thread name for the
 * 			#MCP_ProtocolHandle
 */
#define THD_COMM_MCP_THREAD_NAME                    "tMCP"
/*!
 * \def		THD_COMM_STECI_UART_THREAD_NAME
 * 			Thread name for the
 * 			#STECI_UART_ProtocolHandle
 */
#define THD_COMM_STECI_UART_THREAD_NAME             "tSTECIU"
/*!
 * \def		THD_RIF_PROTOCOL_ROUTER_THREAD_NAME
 * 			Thread name for the
 * 			#rif_protocol_router_handle
 */
#define THD_RIF_PROTOCOL_ROUTER_THREAD_NAME         "tRIF_PR"

/*!
 * \def		THD_RIF_RIMW_THREAD_NAME
 * 			Thread name for the
 * 			#rif_rimw_handle
 */
#define THD_RIF_RIMW_THREAD_NAME                    "tRIF_RIW"

/*!
 * \def		THD_BASE_NAME_MAX_LEN
 * 			The max size of thread name, used for names created run-time
 * 			from a _BASE_NAME
 * \remark	must be equal or greter than the longest _BASE_NAME number of 
 * 			characters plus one chatacter (a digit) plus one null byte.
 */
#define THD_BASE_NAME_MAX_LEN           16

/*****************************************************************
| Local types
|----------------------------------------------------------------*/

struct rif_etalStatus {
	ETAL_HANDLE recv_handle[RIF_MAX_ETAL_RECEIVERS];
	EtalReceiverAttr recv_attr[RIF_MAX_ETAL_RECEIVERS];

	ETAL_HANDLE dpth_handle[RIF_MAX_ETAL_DATAPATHS];
	EtalReceiverAttr dpth_attr[RIF_MAX_ETAL_DATAPATHS];
};

struct rif_serial_port
{
	int fd;
	struct termios oldtio;
	struct termios newtio;
};

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/

/*
 * UGLY HACK: we need a global context here to store some info because
 *            hmi_etal_initialize() does not allow us to register a pointer
 *            to a local context whose value is provided back by
 *            hmi_etal_notification_handler()
 */
static struct rif_global_context {
	struct rif_serial_port *serial_port;
	struct rif_etalStatus *etal_status;
	FILE *log_file;
	tU8 is_mdr3_cmd;
	tBool is_audio_analog;
	tBool is_audio_digital_fm;
	tBool is_audio_digital_dab;
	tBool is_DcopNvmless;
} rif_global_context;

static tBool rif_running = true;
OSAL_tSemHandle rif_SemWaitExit = 0;

struct rif_etalStatus rif_etal_status;
void *rif_serial = NULL;

OSAL_tThreadID rif_threadId_List[RIF_THREAD_MAX];

STECI_UART_paramsTy rif_SteciUartParams;

/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/
#if defined(CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE) && !defined(CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE)
static int rif_GetDCOPImageCb(void *pvContext, tU32 requestedByteNum, tU8* block, tU32* returnedByteNum, tU32 *remainingByteNum, tBool isBootstrap);
#endif

/*****************************************************************
| functions
|----------------------------------------------------------------*/
#if defined(CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE) && !defined(CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE)

/***************************
 *
 * rif_GetDCOPImageCb
 *
 **************************/
static int rif_GetDCOPImageCb(void *pvContext, tU32 requestedByteNum, tU8* block, tU32* returnedByteNum, tU32 *remainingByteNum, tBool isBootstrap)
{
	static FILE *fhbs = NULL, *fhpg = NULL;
	static long file_bs_size = 0, file_pg_size = 0;
	FILE **fhgi;
	long file_position = 0, *file_size = NULL;
	size_t retfread;
	int do_get_file_size = 0;
	
	/* open file bootstrap or program firmware if not already open */
	if (isBootstrap != 0)
	{
		fhgi = &fhbs;
		file_size = &file_bs_size;
		if (*fhgi == NULL)
		{
			*fhgi = fopen(ETALDcop_bootstrap_filename, "r");
			do_get_file_size = 1;
		}
		if (*fhgi == NULL)
		{
			printf("Error %d opening file %s\n", errno, ETALDcop_bootstrap_filename);
			return errno;
		}
	}
	else
	{
		fhgi = &fhpg;
		file_size = &file_pg_size;
		if (*fhgi == NULL)
		{
			*fhgi = fopen(ETALDcop_sections_filename, "r");
			do_get_file_size = 1;
		}
		if (*fhgi == NULL)
		{
			printf("Error %d opening file %s\n", errno, ETALDcop_sections_filename);
			return errno;
		}
	}

	if (do_get_file_size == 1)
	{
		/* read size of file */
		if (fseek(*fhgi, 0, SEEK_END) != 0)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error fseek(0, SEEK_END) %d\n", errno);
			return errno;
		}
		if ((*file_size = ftell(*fhgi)) == -1)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error ftell end of file %d\n", errno);
			return errno;
		}
		if (fseek(*fhgi, 0, SEEK_SET) != 0)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error fseek(0, SEEK_SET) %d\n", errno);
			return errno;
		}
	}

	/* set remaining bytes number */
	if (remainingByteNum != NULL)
	{
		if ((file_position = ftell(*fhgi)) == -1)
		{
			fclose(*fhgi);
			*fhgi = NULL;
			printf("Error ftell %d\n", errno);
			return errno;
		}
		*remainingByteNum = (*file_size - file_position);
	}

	/* read requestedByteNum bytes in file */
	retfread = fread(block, 1, requestedByteNum, *fhgi);
	*returnedByteNum = (tU32) retfread;
	if (*returnedByteNum != requestedByteNum)
	{
		if (ferror(*fhgi) != 0)
		{
			/* error reading file */
			if (isBootstrap != 0)
			{
				printf("Error %d reading file %s\n", errno, ETALDcop_bootstrap_filename);
			}
			else
			{
				printf("Error %d reading file %s\n", errno, ETALDcop_sections_filename);
			}
			clearerr(*fhgi);
			fclose(*fhgi);
			*fhgi = NULL;
			return -1;
		}
	}

	/* Close file if EOF */
	if (feof(*fhgi) != 0)
	{
		/* DCOP bootstrap or flash program successful */
		clearerr(*fhgi);
		fclose(*fhgi);
		*fhgi = NULL;
	}
	return 0;
}
	
#endif // CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE

static void rif_etal_notification_handler(void * pvContext, ETAL_EVENTS dwEvent, void* pvParams)
{
	if (rif_global_context.is_mdr3_cmd == FALSE)
	{
		rif_etalapi_cnv_CbNotify(pvContext, dwEvent, pvParams);
	}
	else
	{
		switch (dwEvent) {
			case ETAL_INFO_TUNE:
				break;
			case ETAL_INFO_LEARN:
				break;
			case ETAL_INFO_SCAN:
				break;
			case ETAL_ERROR_COMM_FAILED:
				break;
#if 0
			case ETAL_INFO_PAD:
				break;
#endif
			case ETAL_INFO_SEAMLESS_ESTIMATION_END:
				break;
			case ETAL_INFO_SEAMLESS_SWITCHING_END:
				break;
			default:
				fprintf(stderr, "Unexpected event from ETAL notification handler (%d)", dwEvent);
				break;
		}
	}
}

static ETAL_STATUS rif_etal_initialize(struct rif_etalStatus *etal_status)
{
	ETAL_STATUS rc;
	EtalHardwareAttr init_params;
	EtalInitStatus status;
	int i;

	OSAL_pvMemorySet(&init_params, 0, sizeof(EtalHardwareAttr));
	init_params.m_cbNotify = rif_etal_notification_handler;
	init_params.m_context = &rif_global_context;

#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE
	if (true == rif_global_context.is_DcopNvmless)
	{
    	init_params.m_DCOPAttr.m_doDownload = TRUE;
#ifdef CONFIG_COMM_DCOP_MDR_FIRMWARE_FILE_MODE_FILE
		init_params.m_DCOPAttr.m_pvGetImageContext = ETALDcop_dcopLoading_filename;
#else
		init_params.m_DCOPAttr.m_cbGetImage = rif_GetDCOPImageCb;
#endif
        init_params.m_DCOPAttr.m_sectDescrFilename = (tU8 *) ETALDcop_sections_filename;	
	}
#endif
	
	if ((rc = etal_initialize(&init_params)) != ETAL_RET_SUCCESS)
	{
		fprintf(stderr, "etal_initialize() failed (%d)\n", rc);
	}

	/* if HW not present disable it */
	if (ETAL_RET_NO_HW_MODULE == rc)
	{
		if (etal_get_init_status(&status) == ETAL_RET_SUCCESS)
		{
			printf("m_lastInitState=%d\nT1.m_deviceStatus=%d\nT2.m_deviceStatus=%d\nDCOP.m_deviceStatus=%d\n", status.m_lastInitState, status.m_tunerStatus[0].m_deviceStatus, status.m_tunerStatus[1].m_deviceStatus, status.m_DCOPStatus.m_deviceStatus);
		}
	}

	for (i = 0; i < RIF_MAX_ETAL_RECEIVERS; i++) {
		etal_status->recv_handle[i] = ETAL_INVALID_HANDLE;
		OSAL_pvMemorySet(&etal_status->recv_attr[i], 0, sizeof(EtalReceiverAttr));
	}

	for (i = 0; i < RIF_MAX_ETAL_DATAPATHS; i++) {
		etal_status->dpth_handle[i] = ETAL_INVALID_HANDLE;
		OSAL_pvMemorySet(&etal_status->dpth_attr[i], 0, sizeof(EtalDataPathAttr));
	}

	return rc;
}

static void rif_etal_deinitialize(struct rif_etalStatus *etal_status)
{
	int i;

	for (i = 0; i < RIF_MAX_ETAL_RECEIVERS; i++) {
		if (etal_status->recv_handle[i] != ETAL_INVALID_HANDLE) {
			etal_destroy_receiver(&etal_status->recv_handle[i]);
		}
	}

	for (i = 0; i < RIF_MAX_ETAL_DATAPATHS; i++) {
		if (etal_status->dpth_handle[i] != ETAL_INVALID_HANDLE) {
			etal_destroy_datapath(&etal_status->dpth_handle[i]);
		}
	}

	printf("rif etal deinitialize begin\n");
	if (etal_deinitialize() != ETAL_RET_SUCCESS)
	{
		printf("etal_deinitialize() return error\n");
	}
	printf("rif etal deinitialize end\n");
}

/***********************************
 *
 * rif_initThreadSpawn
 *
 **********************************/
/*!
 * \brief		Starts the main Radio Interface threads
 * \details		The function starts the MCP protocol threads, the STECI UART threads,
 * \return		OSAL_OK
 * \return		OSAL_ERROR_DEVICE_INIT - error during the thread creation
 * \callgraph
 * \callergraph
 */
static tS32 rif_initThreadSpawn(tVoid)
{
	OSAL_trThreadAttribute thread1_attr;
	OSAL_trThreadAttribute thread2_attr;
	OSAL_trThreadAttribute thread3_attr;
	tU8 i, thread_idx = 0;
	tS32 ret = OSAL_OK;

	/* initialize Thread Id list */
	for (i = 0; i < RIF_THREAD_MAX; i++)
	{
		rif_threadId_List[i] = OSAL_ERROR; /* invalidate the ID */
	}

	/* initialize STECI UART before thread ceration */
	ret = STECI_UART_init();
	/* start STECI UART thread */
	if (ret == OSAL_OK)
	{
		thread1_attr.szName = THD_COMM_STECI_UART_THREAD_NAME;
		thread1_attr.u32Priority =  THD_COMM_STECI_UART_THREAD_PRIORITY;
		thread1_attr.s32StackSize = THD_STECI_UART_STACK_SIZE;
		thread1_attr.pfEntry = STECI_UART_ProtocolHandle;
		OSAL_pvMemorySet(&rif_SteciUartParams, 0, sizeof(STECI_UART_paramsTy));
		rif_SteciUartParams.id = RIF_UART_ID;
		rif_SteciUartParams.speed = RIF_UART_SPEED;
		rif_SteciUartParams.devHandle = rif_serial;
		thread1_attr.pvArg = (tVoid *)&rif_SteciUartParams;
		if (thread_idx >= RIF_THREAD_MAX)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
		}
	}
	if (ret == OSAL_OK)
	{
		rif_threadId_List[thread_idx] = OSAL_ThreadSpawn(&thread1_attr);
		if (rif_threadId_List[thread_idx] == OSAL_ERROR)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
		}
		thread_idx++;
	}

	/* start MCP thread */
	if (ret == OSAL_OK)
	{
		thread2_attr.szName = THD_COMM_MCP_THREAD_NAME;
		thread2_attr.u32Priority =  THD_COMM_MCP_THREAD_PRIORITY;
		thread2_attr.s32StackSize = THD_MCP_STACK_SIZE;
		thread2_attr.pfEntry = MCP_ProtocolHandle;
		thread2_attr.pvArg = (tVoid *)&rif_SteciUartParams;
		if (thread_idx >= RIF_THREAD_MAX)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
		}
	}
	if (ret == OSAL_OK)
	{
		rif_threadId_List[thread_idx] = OSAL_ThreadSpawn(&thread2_attr);
		if (rif_threadId_List[thread_idx] == OSAL_ERROR)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
		}
		thread_idx++;
	}

	/* start rif protocol router thread */
	if (ret == OSAL_OK)
	{
		thread3_attr.szName = THD_RIF_PROTOCOL_ROUTER_THREAD_NAME;
		thread3_attr.u32Priority =  THD_RIF_PROTOCOL_ROUTER_THREAD_PRIORITY;
		thread3_attr.s32StackSize = THD_RIF_PROTOCOL_ROUTER_STACK_SIZE;
		thread3_attr.pfEntry = rif_protocol_router_handle;
		thread3_attr.pvArg = (tVoid *)NULL;
		if (thread_idx >= RIF_THREAD_MAX)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
		}
	}
	if (ret == OSAL_OK)
	{
		rif_threadId_List[thread_idx] = OSAL_ThreadSpawn(&thread3_attr);
		if (rif_threadId_List[thread_idx] == OSAL_ERROR)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
		}
		thread_idx++;
	}

	/* start rif rimw thread */
	if (ret == OSAL_OK)
	{
		thread3_attr.szName = THD_RIF_RIMW_THREAD_NAME;
		thread3_attr.u32Priority =  THD_RIF_RIMW_THREAD_PRIORITY;
		thread3_attr.s32StackSize = THD_RIF_RIMW_STACK_SIZE;
		thread3_attr.pfEntry = rif_rimw_protocol_handle;
		thread3_attr.pvArg = (tVoid *)NULL;
		if (thread_idx >= RIF_THREAD_MAX)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
		}
	}
	if (ret == OSAL_OK)
	{
		rif_threadId_List[thread_idx] = OSAL_ThreadSpawn(&thread3_attr);
		if (rif_threadId_List[thread_idx] == OSAL_ERROR)
		{
			ret = OSAL_ERROR_DEVICE_INIT;
		}
		thread_idx++;
	}
	return ret;
}

/***********************************
 *
 * rif_initThreadDelete
 *
 **********************************/
/*!
 * \brief		Destroys the threads created by #rif_initThreadSpawn
 * \return		OSAL_OK
 * \return		OSAL_ERROR - error while destroying a thread
 * \callgraph
 * \callergraph
 */
static tS32 rif_initThreadDelete(tVoid)
{
	tU32 i;
	tSInt ret = OSAL_OK;

	for (i = 0; i < RIF_THREAD_MAX; i++)
	{
		if (rif_threadId_List[i] != OSAL_ERROR)
		{
			if ((ret = OSAL_s32ThreadDelete(rif_threadId_List[i])) == OSAL_OK)
			{
				rif_threadId_List[i] = OSAL_ERROR; /* invalidate the ID */
			}
			else
			{
				printf("OSAL_s32ThreadDelete(%d) error %d\n", rif_threadId_List[i], ret);
				ret = OSAL_ERROR;
			}
		}
	}
	/* give the killed thread the opportunity to do cleanup if required */
	OSAL_s32ThreadWait(RIF_THREAD_SCHEDULING);

	return ret;
}

/***************************
 *
 * rif_initialize
 *
 **************************/
/*!
 * \brief		Initialize radio interface devices and threads
 * \callgraph
 * \callergraph
 */
static tS32 rif_initialize(void)
{
	tS32 ret;
	EtalAudioInterfTy audioIf;

	/* initialize etal and osal */
	if ((rif_running == true) && (rif_etal_initialize(&rif_etal_status) != ETAL_RET_SUCCESS))
	{
		return OSAL_ERROR;
	}

	/* create semaphore rif_SemWaitExit */
	if ((ret = OSAL_s32SemaphoreCreate(RIF_SEM_WAIT_EXIT, &rif_SemWaitExit, 0)) != OSAL_OK)
	{
		fprintf(stderr, "semaphore rif_SemWaitTermSig creation failed %d\n", ret);
		return ret;
	}

	if ((rif_running == true) && ((rif_serial = CommParameterSetup(RIF_UART_ID, RIF_UART_SPEED)) == NULL))
	{
		return OSAL_ERROR;
	}

	/* start threads */
	if ((rif_running == true) && (rif_initThreadSpawn() != OSAL_OK))
	{
		return OSAL_ERROR;
	}


	/* Configure audio path on CMOST */
	memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
	audioIf.m_sai_out = audioIf.m_sai_in = 1;
	if (rif_global_context.is_audio_analog == true)
	{
		audioIf.m_dac = 1;
	}
#if ((defined(CONFIG_MODULE_DCOP_HDRADIO)) || (!defined(CONFIG_BOARD_ACCORDO5)) || (!defined(CONFIG_DIGITAL_AUDIO)))
	/* for HD Radio or Accordo2 use only analog */
	audioIf.m_dac = 1;
#endif
	// Slave mode adaptation depending on HD
	// HD mod: in this case, DCOP680 is the I2S master, so CMOST has to be configured as SAI slave
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = TRUE;
#endif
	if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
	{
		printf("etal_config_audio_path (%d)", ret);
		return OSAL_ERROR;
	}

	/* configure Accordo audio path */
#ifdef CONFIG_BOARD_ACCORDO5
	if (audioIf.m_dac == 1)
	{
		system("amixer -c 3 sset Source adcauxdac > /dev/null" );

		// select the audio channel
		system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
	}
	else if (rif_global_context.is_audio_digital_fm == true)
	{
		system("amixer -c 3 sset Source sai4rx1fm > /dev/null");
	}
	else if (rif_global_context.is_audio_digital_dab == true)
	{
		system("amixer -c 3 sset Source sai4rx2dab > /dev/null");
	}
	system("amixer -c 3 sset \"Scaler Primary Media Volume Master\" 1200 > /dev/null");
#endif

#ifdef CONFIG_BOARD_ACCORDO2
	// select the audio source
	system("amixer -c 3 sset Source adcauxdac > /dev/null" );

	// select the audio channel
	system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
#endif
#if (defined(CONFIG_BOARD_ACCORDO2) || defined(CONFIG_BOARD_ACCORDO5))
	// Set the output volume 
	system("amixer -c 3 sset \"Volume Master\" 1200 > /dev/null");
#endif // CONFIG_BOARD_ACCORDO2 || CONFIG_BOARD_ACCORDO5

	return ret;
}

/***************************
 *
 * rif_deinitialize
 *
 **************************/
/*!
 * \brief		Deinitialize radio interface devices and threads
 * \callgraph
 * \callergraph
 */
static tS32 rif_deinitialize(void)
{
	tS32 ret = OSAL_OK;

	/* delete app semaphore */
	if (rif_SemWaitExit != 0)
	{
		if ((ret = OSAL_s32SemaphoreClose(rif_SemWaitExit)) != OSAL_OK)
		{
			fprintf(stderr, "semaphore Sem_rifWaitTermSig close failed %d\n", ret);
		}
		else if ((ret = OSAL_s32SemaphoreDelete(RIF_SEM_WAIT_EXIT)) != OSAL_OK)
		{
			fprintf(stderr, "semaphore Sem_rifWaitTermSig delete failed %d\n", ret);
		}
		rif_SemWaitExit = 0;
	}

printf("rif close Com Port\n");
	if (rif_serial != NULL)
	{
		CommPortClose(rif_serial);
	}

	/* Delete app threads */
printf("rif thread Delete\n");
	rif_initThreadDelete();

	/* etal deinitialize */
	rif_etal_deinitialize(&rif_etal_status);

printf("rif deinitialize end\n");
	if (rif_global_context.log_file)
	{
		fclose(rif_global_context.log_file);
	}

	return ret;
}

/******************************************************************************
 *
 * rif_atexit
 *
 *****************************************************************************/
static tVoid rif_atexit(tVoid)
{
	if (rif_running == true)
	{
		rif_running = false;
		if (rif_deinitialize() != OSAL_OK)
		{
			fprintf(stderr, "rif_deinitialize return error\n");
		}
		else
		{
			printf("rif stopped\n");
		}
	}
}

/******************************************************************************
 *
 * rif_stop_rif
 *
 *****************************************************************************/
tVoid rif_stop_rif(tVoid)
{
	printf("rif stop rif received\n");
	if (rif_running == true)
	{
		rif_running = false;
		if ((rif_SemWaitExit == 0) || (OSAL_s32SemaphorePost(rif_SemWaitExit) != OSAL_OK))
		{
			fprintf(stderr, "rif_signal_handler OSAL_s32SemaphorePost(rif_SemWaitExit) return error\n");
		}
		else
		{
			printf("rif sem post done\n");
		}
	}
}


/******************************************************************************
 *
 * Signal handler
 *
 *****************************************************************************/
static tVoid rif_signal_handler(int sig)
{
	switch (sig) {
	case SIGINT:
		printf("rif SIGINT received\n");
		if (rif_running == true)
		{
			rif_running = false;
			if ((rif_SemWaitExit == 0) || (OSAL_s32SemaphorePost(rif_SemWaitExit) != OSAL_OK))
			{
				fprintf(stderr, "rif_signal_handler OSAL_s32SemaphorePost(rif_SemWaitExit) return error\n");
			}
			else
			{
				printf("rif sem post done\n");
			}
		}
		break;
	default:
		//do nothing
		break;
	}
}

/******************************************************************************
 *
 * Help
 *
 *****************************************************************************/
static void rif_help(char *program_name)
{
	printf("%s [OPTIONS...]\n\n"
	       "Radio Interface application. (C) STMicroelectronics\n\n"
	       "  -h --help                 Print this message\n"
	       "  -m --mdr3                 Use MDR3 API conversion instead of ETAL API\n"
	       "  -l --log[=filename]       Log operations to filename\n"
	       "     --analog               select Accordo analog tuner audio\n"
	       "     --digital_fm           select Accordo digital AM/FM tuner audio\n"
	       "     --digital_dab          select Accordo digital DAB tuner audio\n"
	       "     --load                 start the DCOP in load mode\n"
	       "\n\n",
	       program_name);
}

/******************************************************************************
 *
 * Application
 *
 *****************************************************************************/

int main(int argc, char* argv[])
{
	tS32 ret;
	int c;
	static const struct option long_options[] = {
			{ "analog",         no_argument,       NULL, RIF_OPT_ANALOG },
			{ "digital_fm",     no_argument,       NULL, RIF_OPT_DIGITAL_FM },
			{ "digital_dab",	no_argument,       NULL, RIF_OPT_DIGITAL_DAB },
			{ "log",            optional_argument, NULL, 'l' },
			{ "mdr3",           no_argument,       NULL, 'm' },
			{ "help",           no_argument,       NULL, 'h' },
			{ "load",           no_argument,       NULL, RIF_OPT_LOAD },
			{ 0, 0, 0, 0 }
	};

	/* init global struct */
	OSAL_pvMemorySet(&rif_global_context, 0, sizeof(struct rif_global_context));

	/*
	 * parse command line options
	 */
	while ((c = getopt_long(argc, argv, "l:mh", long_options, NULL)) >= 0)
	{
		switch (c)
		{
			case 'l':
				rif_global_context.log_file = fopen(optarg ? optarg : "radio_hmi_log.txt", "a");
				break;
			case 'm':
				rif_global_context.is_mdr3_cmd = true;
				break;
			case RIF_OPT_ANALOG:
				if ((rif_global_context.is_audio_digital_fm == true) || (rif_global_context.is_audio_digital_dab == true))
				{
					printf("option --analog cannot be used with --digital_fm or --digital_dab\n");
					return EXIT_FAILURE;
				}
				rif_global_context.is_audio_analog = true;
				break;
			case RIF_OPT_DIGITAL_FM:
				if (rif_global_context.is_audio_digital_dab == true)
				{
					printf("option --digital_fm cannot be used with --digital_dab\n");
					return EXIT_FAILURE;
				}
				else if (rif_global_context.is_audio_analog == true)
				{
					printf("option --digital_fm cannot be used with --analog\n");
					return EXIT_FAILURE;
				}
				rif_global_context.is_audio_digital_fm = true;
				break;
			case RIF_OPT_DIGITAL_DAB:
				if (rif_global_context.is_audio_digital_fm == true)
				{
					printf("option --digital_dab cannot be used with --digital_fm\n");
					return EXIT_FAILURE;
				}
				else if (rif_global_context.is_audio_analog == true)
				{
					printf("option --digital_dab cannot be used with --analog\n");
					return EXIT_FAILURE;
				}
				rif_global_context.is_audio_digital_dab = true;
				break;
			case RIF_OPT_LOAD:
				/// start dcop in load mode ie nvmless
				//
				rif_global_context.is_DcopNvmless = true;
				break;
			case 'h':
				rif_help(argv[0]);
				return EXIT_SUCCESS;
			default:
				printf("Unknown option: %c\n", c);
				return EXIT_FAILURE;
		}
	}

	/*
	 * install signal handler to exit gracefully
	 */
	signal(SIGINT, rif_signal_handler);
	if (atexit(rif_atexit) != 0)
	{
		fprintf(stderr, "atexit return error\n");
		return EXIT_FAILURE;
	}

	if ((ret = rif_initialize()) != OSAL_OK)
	{
		fprintf(stderr, "semaphore rif_initialize failed return %d\n", ret);
		return EXIT_FAILURE;
	}

	/*
	 * UGLY HACK: we need a global context here to store some info because
	 *            hmi_etal_initialize() does not allow us to register a pointer to
	 *            a local context whose value is provided back by
	 *            hmi_etal_notification_handler()
	 */
	rif_global_context.serial_port = rif_serial;
	rif_global_context.etal_status = &rif_etal_status;

	printf("rif started\n");

	/*
	 * main application loop: wait for user exit
	 */
	while ((rif_running == true) && (ret == OSAL_OK))
	{
		/*
		 * wait for semaphore rif_SemWaitExit
		 */
		ret = OSAL_s32SemaphoreWait(rif_SemWaitExit, OSAL_C_TIMEOUT_FOREVER);
	}

	printf("rif deinitialize start\n");

	ret = rif_deinitialize();

	printf("rif stopped\n");

	if (ret == OSAL_OK)
	{
		return EXIT_SUCCESS;
	}
	else
	{
		return EXIT_FAILURE;
	}
}
