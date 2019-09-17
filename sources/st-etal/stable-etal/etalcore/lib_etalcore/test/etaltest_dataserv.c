//!
//!  \file 		etaltest_dataserv.c
//!  \brief 	<i><b> ETAL test, data services functions </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

#ifndef CONFIG_HOST_OS_WIN32
	// for WIFEXITED, WEXITSTATUS
	#include <sys/types.h>
	#include <sys/wait.h>
#endif //CONFIG_HOST_OS_WIN32

/***************************
 * Local Macros
 **************************/

#if defined (CONFIG_APP_TEST_DAB)
#define ETAL_TEST_DATASERVICES_IMG0_SIZE  9587
#define ETAL_TEST_DATASERVICES_IMG1_SIZE 18956
#define ETAL_TEST_DATASERVICES_IMG2_SIZE 23307
#endif //defined (CONFIG_APP_TEST_DAB)

#define PSD_HDFM_TITLE_1 "Bach's Lunch"
#define PSD_HDFM_TITLE_2 "Hungarian Rhapsody No. 2"
#define PSD_HDFM_TITLE_3 "Without a Song"
#define PSD_HDFM_TITLE_4 "Subway"

/* PSD TITLE valid with the IB_AMr208_e1awfa05.wv (ARB file loaded in the SFE) */
#define PSD_HDAM_TITLE_1 "AM Radio - Crystal clear"

#if defined (CONFIG_APP_TEST_HDRADIO_FM) ||defined (CONFIG_APP_TEST_HDRADIO_AM)
#ifdef CONFIG_HOST_OS_TKERNEL
#define ROOT_FS "/sda/"
#else //CONFIG_HOST_OS_TKERNEL
#define ROOT_FS ""
#endif //CONFIG_HOST_OS_TKERNEL
#endif //defined (CONFIG_APP_TEST_HDRADIO_FM) ||defined (CONFIG_APP_TEST_HDRADIO_AM)

/***************************
 * Local types
 **************************/

#if defined (CONFIG_APP_TEST_HDRADIO_FM) ||defined (CONFIG_APP_TEST_HDRADIO_AM)
typedef enum {
	TITLE	   = 0x0001,
	ARTIST	   = 0x0002,
	ALBUM	   = 0x0004,
	GENRE	   = 0x0008,
	COMMENT    = 0x0010,
	UFID	   = 0x0020,
	COMMERCIAL = 0x0040,
	XHDR	   = 0x0080,
} tyHDRADIOPSDFieldBitMask;
#endif //defined (CONFIG_APP_TEST_HDRADIO_FM) ||defined (CONFIG_APP_TEST_HDRADIO_AM)

/***************************
 * Local variables
 **************************/

#if defined (CONFIG_ETAL_HAVE_DATASERVICES) || defined (CONFIG_ETAL_HAVE_ALL_API)
tU32 etalTestDataServ;

#if defined (CONFIG_APP_TEST_DAB)
#if defined(CONFIG_APP_TEST_DATASERVICES_SLS) || defined(CONFIG_APP_TEST_DATASERVICES_EPG) || defined(CONFIG_APP_TEST_DATASERVICES_JOURNALINE) || defined(CONFIG_APP_TEST_DATASERVICES_TPEG) ||	defined(CONFIG_APP_TEST_DATASERVICES_DLS)
static tU32 DLSCbInvocations;
#endif
#if  defined(CONFIG_APP_TEST_DATASERVICES_DLPLUS)
static tU32 DLPLUSCbInvocations;
#endif
tU32 count_EPG_BIN, count_EPG_SRV, count_EPG_PRG, count_EPG_LOGO;
#endif

#endif // CONFIG_ETAL_HAVE_DATASERVICES || CONFIG_ETAL_HAVE_ALL_API

/***************************
 * function prototypes
 **************************/
#ifdef CONFIG_APP_TEST_DATASERVICES
#if defined (CONFIG_ETAL_HAVE_DATASERVICES) || defined (CONFIG_ETAL_HAVE_ALL_API)

#ifdef CONFIG_APP_TEST_DATASERVICES_PSD
static tSInt etalTestDataServices_PSD(etalTestBroadcastTy test_mode, tBool *pass);
#endif //CONFIG_APP_TEST_DATASERVICES_PSD

#if defined(CONFIG_APP_TEST_DATASERVICES_SLS) || defined(CONFIG_APP_TEST_DATASERVICES_EPG) || defined(CONFIG_APP_TEST_DATASERVICES_JOURNALINE) || defined(CONFIG_APP_TEST_DATASERVICES_TPEG) ||	defined(CONFIG_APP_TEST_DATASERVICES_DLS) || defined(CONFIG_APP_TEST_DATASERVICES_DLPLUS)
static tSInt etalTestDataServices_internal(etalTestBroadcastTy test_mode, EtalDataServiceType serviceType, tBool *pass);
#endif

#if defined (CONFIG_APP_TEST_DAB)
static tSInt etalTestDoServiceSelectData(ETAL_HANDLE hReceiver, ETAL_HANDLE *phDatapath, tU32 ueid, tU32 sid, tBool Data, tSInt sc, tSInt subch);
static tSInt etalTestUndoServiceSelectData(ETAL_HANDLE hDatapath, tU32 ueid, tU32 sid);
tSInt etalTestDataServices_SLS(void);
tSInt etalTestDataServices_EPG(void);
tSInt etalTestDataServices_TPEG(void);
tSInt etalTestDataServices_JML(void);
tSInt etalTestDataServices_DLS(void);
tSInt etalTestDataServices_DLPLUS(void);
#endif //defined (CONFIG_APP_TEST_DAB)

#if (defined(CONFIG_APP_TEST_DAB) && (defined (CONFIG_ETAL_HAVE_DATASERVICES) || defined (CONFIG_ETAL_HAVE_ALL_API))) || \
	((defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)) && CONFIG_APP_TEST_DATASERVICES_PSD)
static tVoid etalTestDataServCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext);
#endif

#endif // CONFIG_ETAL_HAVE_DATASERVICES || CONFIG_ETAL_HAVE_ALL_API

#if defined (CONFIG_ETAL_HAVE_DATASERVICES) || defined (CONFIG_ETAL_HAVE_ALL_API)

#if defined (CONFIG_APP_TEST_DAB)

#if defined(CONFIG_APP_TEST_DATASERVICES_DLS)
static tVoid etalTestPADDLSCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalPADDLSTy *ppad;
	ppad = (etalPADDLSTy *)pBuffer;
	etalTestPrintNormal("PAD DLS : CharSet: %d\n", ppad->m_charset);
	etalTestPrintNormal("PAD DLS : Content: %s\n", ppad->m_PAD_DLS);
	DLSCbInvocations++;
	return;
}
#endif

#if defined(CONFIG_APP_TEST_DATASERVICES_DLPLUS)
static tVoid etalTestPADDLPLUSCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalPADDLPLUSTy *ppad;
	ppad = (etalPADDLPLUSTy *)pBuffer;
	tU32 i;

	etalTestPrintNormal("PAD DL PLUS : nbOfItems: %d\n", ppad->m_nbOfItems);
	for(i=0;i<ppad->m_nbOfItems;i++)
	{
		etalTestPrintNormal("PAD DL PLUS : Item Nb %d :\n", i);
		etalTestPrintNormal("PAD DL PLUS : Content type: 0x%x\n", ppad->m_item[i].m_contentType);
		etalTestPrintNormal("PAD DL PLUS : running status: 0x%x\n", ppad->m_item[i].m_runningStatus);
		etalTestPrintNormal("PAD DL PLUS : Charset: %d\n", ppad->m_item[i].m_charset);
		etalTestPrintNormal("PAD DL PLUS : Label length: %d\n", ppad->m_item[i].m_labelLength);
		etalTestPrintNormal("PAD DL PLUS : Label: %s\n", ppad->m_item[i].m_label);
	}

	DLPLUSCbInvocations++;
	return;
}
#endif
#endif // CONFIG_APP_TEST_DAB

#if (defined(CONFIG_APP_TEST_DAB) && (defined (CONFIG_ETAL_HAVE_DATASERVICES) || defined (CONFIG_ETAL_HAVE_ALL_API))) || \
	((defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)) && CONFIG_APP_TEST_DATASERVICES_PSD)
	

/***************************
 *
 * etalTestDataServCallback
 *
 **************************/
static tVoid etalTestDataServCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
#define ETAL_TEST_DATASERV_NAMELEN 32
	tChar name[ETAL_TEST_DATASERV_NAMELEN];

#if defined (CONFIG_APP_TEST_DAB)

#define ETAL_TEST_DATASERV_CMDLEN  64
	tChar ref_name[ETAL_TEST_DATASERV_NAMELEN];

#ifndef CONFIG_HOST_OS_WIN32
	char cmd[ETAL_TEST_DATASERV_CMDLEN];
	int ret;
#endif //CONFIG_HOST_OS_WIN32

#endif //defined (CONFIG_APP_TEST_DAB)
	FILE *fp;
#if defined (CONFIG_APP_TEST_DAB)
	static tU32 count = 0;
	static tU32 count_JML = 0;
	static tU32 count_TPEG_RAW = 0;
	static tU32 count_TPEG_SNI = 0;
	static tU32 count_EPG_RAW = 0;
	
#if defined(CONFIG_APP_TEST_DATASERVICES_DLS)
	static tU32 count_DLS = 0;
	etalPADDLSTy *pad_DLS;

#endif
#if defined(CONFIG_APP_TEST_DATASERVICES_DLPLUS)
	static tU32 count_DLPLUS = 0;
	etalPADDLPLUSTy *pad_DLPLUS;
	tU32 i;

#endif

#endif //defined (CONFIG_APP_TEST_DAB)

#if defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
	static tU32 count_PSD = 0;
#endif //defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)

	EtalGenericDataServiceRaw *DataServiceRawBuffer;

	/*
	 * First determine which data service type this is
	 */

	DataServiceRawBuffer = (EtalGenericDataServiceRaw*)pBuffer;

	switch((*DataServiceRawBuffer).m_dataType)
	{
#if defined (CONFIG_APP_TEST_DAB)
		case ETAL_DATASERV_TYPE_SLS_XPAD:
			/*
			 * Captures the images to files on the
			 * filesystem and compares them with reference files.
			 * The service contains three jpeg images, of three different sizes;
			 * the test is considered passed if at least two good images are
			 * received.
			 * Only 2 images are needed because depending on which position the
			 * ETI is when the test starts not all images may be captured
			 */
			etalTestPrintNormal("Data Service SLS callback received packet size %d", dwActualBufferSize);
			OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_DATASERV_NAMELEN, "image-%d.jpg", count++);
			fp = fopen(name, "w");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
			}
			else
			{
				if (fwrite((tPVoid)(&(DataServiceRawBuffer->m_data)), (size_t)1, (size_t)(dwActualBufferSize-4), fp) != (size_t)(dwActualBufferSize-4))
				{
					etalTestPrintError("writing to file %s", name);
				}
				fclose(fp);

				/*
				 * compare results with reference using the command 'cmp' part of the 'diffutils' commands
				 */
				if ((dwActualBufferSize-4) == ETAL_TEST_DATASERVICES_IMG0_SIZE)
				{
					OSAL_s32NPrintFormat(ref_name, ETAL_TEST_DATASERV_NAMELEN, "image-small-ref.jpg");
				}
				else if ((dwActualBufferSize-4) == ETAL_TEST_DATASERVICES_IMG1_SIZE)
				{
					OSAL_s32NPrintFormat(ref_name, ETAL_TEST_DATASERV_NAMELEN, "image-big-ref.jpg");
				}
				else if ((dwActualBufferSize-4) == ETAL_TEST_DATASERVICES_IMG2_SIZE)
				{
					OSAL_s32NPrintFormat(ref_name, ETAL_TEST_DATASERV_NAMELEN, "image-huge-ref.jpg");
				}
				else
				{
					etalTestPrintNormal("Unknown image size (%d)", dwActualBufferSize-4);
					return;
				}
#ifdef CONFIG_HOST_OS_WIN32
				/* I don't know an easy way to do file compare on MinGW so only
				 * consider image size for test to pass */
				etalTestDataServ++;
#else
				OSAL_s32NPrintFormat(cmd, ETAL_TEST_DATASERV_CMDLEN, "cmp %s %s", name, ref_name);
				ret = system(cmd);
				if (WIFEXITED(ret))
				{
					if (WEXITSTATUS(ret) == 0)
					{
						etalTestDataServ++;
					}
				}
				else
				{
					etalTestPrintError("Spawning system command");
				}
#endif
			}
			break;
		case ETAL_DATASERV_TYPE_TPEG_RAW:
			/*
			 * Writes the RAW content to files on the filesystem
			 */
			etalTestPrintNormal("Data Service TPEG RAW callback received packet size %d", dwActualBufferSize);
			OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_DATASERV_NAMELEN, "TPEG_RAW-%d.txt", count_TPEG_RAW++);
			fp = fopen(name, "w");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
			}
			else
			{
				if (fwrite((tPVoid)(&(DataServiceRawBuffer->m_data)), (size_t)1, (size_t)(dwActualBufferSize-4), fp) != (size_t)(dwActualBufferSize-4))
				{
					etalTestPrintError("writing to file %s", name);
				}
				fclose(fp);
				etalTestDataServ++;
			}
			break;
		case ETAL_DATASERV_TYPE_TPEG_SNI:
			/*
			 * Writes the SNI content to files on the filesystem
			 */
			etalTestPrintNormal("Data Service TPEG SNI callback received packet size %d", dwActualBufferSize);
			OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_DATASERV_NAMELEN, "TPEG_SNI-%d.txt", count_TPEG_SNI++);
			fp = fopen(name, "w");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
			}
			else
			{
				if (fwrite((tPVoid)(&(DataServiceRawBuffer->m_data)), (size_t)1, (size_t)(dwActualBufferSize-4), fp) != (size_t)(dwActualBufferSize-4))
				{
					etalTestPrintError("writing to file %s", name);
				}
				fclose(fp);
				etalTestDataServ++;
			}
			break;
		case ETAL_DATASERV_TYPE_EPG_RAW:
			/*
			 * Writes the EPG RAW content to files on the filesystem
			 */
			etalTestPrintNormal("Data Service EPG RAW callback received packet size %d", dwActualBufferSize);
			OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_DATASERV_NAMELEN, "EPG_RAW-%d.txt", count_EPG_RAW++);
			fp = fopen(name, "w");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
			}
			else
			{
				if (fwrite((tPVoid)(&(DataServiceRawBuffer->m_data)), (size_t)1, (size_t)(dwActualBufferSize-4), fp) != (size_t)(dwActualBufferSize-4))
				{
					etalTestPrintError("writing to file %s", name);
				}
				fclose(fp);
				etalTestDataServ++;
			}
			break;
		case ETAL_DATASERV_TYPE_EPG_BIN:
			/*
			 * Writes the EPG BIN content to files on the filesystem
			 */
			etalTestPrintNormal("Data Service EPG BIN callback received packet size %d", dwActualBufferSize);
			OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_DATASERV_NAMELEN, "EPG_BIN-%d.txt", count_EPG_BIN++);
			fp = fopen(name, "w");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
			}
			else
			{
				if (fwrite((tPVoid)(&(DataServiceRawBuffer->m_data)), (size_t)1, (size_t)(dwActualBufferSize-4), fp) != (size_t)(dwActualBufferSize-4))
				{
					etalTestPrintError("writing to file %s", name);
				}
				fclose(fp);
			}
			break;
		case ETAL_DATASERV_TYPE_EPG_SRV:
			/*
			 * Writes the EPG SRV content to files on the filesystem
			 */
			etalTestPrintNormal("Data Service EPG SRV callback received packet size %d", dwActualBufferSize);
			OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_DATASERV_NAMELEN, "EPG_SRV-%d.txt", count_EPG_SRV++);
			fp = fopen(name, "w");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
			}
			else
			{
				if (fwrite((tPVoid)(&(DataServiceRawBuffer->m_data)), (size_t)1, (size_t)(dwActualBufferSize-4), fp) != (size_t)(dwActualBufferSize-4))
				{
					etalTestPrintError("writing to file %s", name);
				}
				fclose(fp);
			}
			break;
		case ETAL_DATASERV_TYPE_EPG_PRG:
			/*
			 * Writes the EPG PRG content to files on the filesystem
			 */
			etalTestPrintNormal("Data Service EPG PRG callback received packet size %d", dwActualBufferSize);
			OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_DATASERV_NAMELEN, "EPG_PRG-%d.txt", count_EPG_PRG++);
			fp = fopen(name, "w");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
			}
			else
			{
				if (fwrite((tPVoid)(&(DataServiceRawBuffer->m_data)), (size_t)1, (size_t)(dwActualBufferSize-4), fp) != (size_t)(dwActualBufferSize-4))
				{
					etalTestPrintError("writing to file %s", name);
				}
				fclose(fp);
			}
			break;
		case ETAL_DATASERV_TYPE_EPG_LOGO:
			/*
			 * Writes the EPG LOGO content to files on the filesystem
			 */
			etalTestPrintNormal("Data Service EPG LOGO callback received packet size %d", dwActualBufferSize);
			OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_DATASERV_NAMELEN, "EPG_LOGO-%d.txt", count_EPG_LOGO++);
			fp = fopen(name, "w");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
			}
			else
			{
				if (fwrite((tPVoid)(&(DataServiceRawBuffer->m_data)), (size_t)1, (size_t)(dwActualBufferSize-4), fp) != (size_t)(dwActualBufferSize-4))
				{
					etalTestPrintError("writing to file %s", name);
				}
				fclose(fp);
			}
			break;
		case ETAL_DATASERV_TYPE_JML_OBJ:
			/*
			 * Writes the JML content to files on the
			 * filesystem
			 */
			etalTestPrintNormal("Data Service Journaline callback received packet size %d", dwActualBufferSize);
			OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_DATASERV_NAMELEN, "JML-%d.txt", count_JML++);
			fp = fopen(name, "w");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
			}
			else
			{
				if (fwrite((tPVoid)(&(DataServiceRawBuffer->m_data)), (size_t)1, (size_t)(dwActualBufferSize-4), fp) != (size_t)(dwActualBufferSize-4))
				{
					etalTestPrintError("writing to file %s", name);
				}
				fclose(fp);
				etalTestDataServ++;
			}
			break;
#if  defined(CONFIG_APP_TEST_DATASERVICES_DLS)

		case ETAL_DATASERV_TYPE_DLS:
			/*
			 * Writes the DLS content to files on the
			 * filesystem
			 */
			etalTestPrintNormal("PAD DLS callback received packet size %d", dwActualBufferSize);
			pad_DLS = (etalPADDLSTy *)&(DataServiceRawBuffer->m_data);
			etalTestPrintNormal("PAD DLS: Charset: %d, String: %s", pad_DLS->m_charset, pad_DLS->m_PAD_DLS);

			DLSCbInvocations++;
			OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_DATASERV_NAMELEN, "DLS-%d.txt", count_DLS++);
			fp = fopen(name, "w");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
			}
			else
			{
				if (fwrite((tPVoid)(&(DataServiceRawBuffer->m_data)), (size_t)1, (size_t)(dwActualBufferSize-4), fp) != (size_t)(dwActualBufferSize-4))
				{
					etalTestPrintError("writing to file %s", name);
				}
				fclose(fp);
				etalTestDataServ++;
			}
			break;
#endif
#if defined(CONFIG_APP_TEST_DATASERVICES_DLPLUS)

		case ETAL_DATASERV_TYPE_DLPLUS:
			/*
			 * Writes the PAD DL PLUS content to files on the
			 * filesystem
			 */
			etalTestPrintNormal("PAD DL PLUS callback received packet size %d", dwActualBufferSize);
			pad_DLPLUS = (etalPADDLPLUSTy *)&(DataServiceRawBuffer->m_data);
			etalTestPrintNormal("PAD DL PLUS: nbOfItems: %d\n", pad_DLPLUS->m_nbOfItems);

			for(i = 0; i < pad_DLPLUS->m_nbOfItems; i++)
			{
				etalTestPrintNormal("PAD DL PLUS : Item Nb %d:\n", i);
				etalTestPrintNormal("PAD DL PLUS : Content type: 0x%x\n", pad_DLPLUS->m_item[i].m_contentType);
				etalTestPrintNormal("PAD DL PLUS : running status: 0x%x\n", pad_DLPLUS->m_item[i].m_runningStatus);
				etalTestPrintNormal("PAD DL PLUS : Charset: %d\n", pad_DLPLUS->m_item[i].m_charset);
				etalTestPrintNormal("PAD DL PLUS : Label length: %d\n", pad_DLPLUS->m_item[i].m_labelLength);
				etalTestPrintNormal("PAD DL PLUS : Label: %s\n", pad_DLPLUS->m_item[i].m_label);
			}

			DLPLUSCbInvocations++;
			OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_DATASERV_NAMELEN, "DLPLUS-%d.txt", count_DLPLUS++);
			fp = fopen(name, "w");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
			}
			else
			{
				if (fwrite((tPVoid)(&(DataServiceRawBuffer->m_data)), (size_t)1, (size_t)(dwActualBufferSize-4), fp) != (size_t)(dwActualBufferSize-4))
				{
					etalTestPrintError("writing to file %s", name);
				}
				fclose(fp);
				etalTestDataServ++;
			}
			break;
#endif // CONFIG_APP_TEST_DATASERVICES_DLPLUS
#endif // CONFIG_APP_TEST_DAB

#if defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
		case ETAL_DATASERV_TYPE_PSD:
			/*
			 * Writes the PSD content to files on the filesystem
			 */
			etalTestPrintNormal("Data Service PSD callback received packet size %d", dwActualBufferSize);
			OSAL_s32NPrintFormat(name, (size_t)ETAL_TEST_DATASERV_NAMELEN, "%sPSD-%d.txt", ROOT_FS, count_PSD++);
			fp = fopen(name, "w");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
			}
			else
			{
				if (fwrite((tPVoid)(&(DataServiceRawBuffer->m_data)), (size_t)1, (size_t)(dwActualBufferSize-4), fp) != (size_t)(dwActualBufferSize-4))
				{
					etalTestPrintError("writing to file %s", name);
				}
			fclose(fp);
			}
			etalTestDataServ++;
			break;
#endif //defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)

		default:
			etalTestPrintNormal("DataServCallback: Unexpected Data Service 0x%x || packet size %d", (*DataServiceRawBuffer).m_dataType, dwActualBufferSize);
			break;
	}
}
#endif // #if defined (CONFIG_APP_TEST_DAB) || defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)

#if defined (CONFIG_APP_TEST_DAB) && (defined (CONFIG_ETAL_HAVE_DATASERVICES) || defined (CONFIG_ETAL_HAVE_ALL_API))

/***************************
 *
 * etalTestDoServiceSelectData
 *
 **************************/
static tSInt etalTestDoServiceSelectData(ETAL_HANDLE hReceiver, ETAL_HANDLE *phDatapath, tU32 ueid, tU32 sid, tBool Data, tSInt sc, tSInt subch)
{
	EtalDataPathAttr dataPathAttr;
	ETAL_STATUS ret;

    /* Configure datapath */

	etalTestPrintNormal("* Config datapath for Service Select, %s service", Data ? "DATA":"AUDIO");

	*phDatapath = ETAL_INVALID_HANDLE;
	OSAL_pvMemorySet((tPVoid)&dataPathAttr, 0x00, sizeof(EtalDataPathAttr));
	dataPathAttr.m_receiverHandle = hReceiver;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_DATA_SERVICE;
	dataPathAttr.m_sink.m_BufferSize = 0x00;
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestDataServCallback;
	if ((ret = etal_config_datapath(phDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for Service Select (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	if (!Data)
	{
		/* Service Select by SID */
		etalTestPrintNormal("* Service Select by Sid");
		if ((ret = etal_service_select_audio(hReceiver, ETAL_SERVSEL_MODE_SERVICE, ueid, sid, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}
	else
	{
		if (subch != ETAL_INVALID)
		{
			/* Service Select by Subchannel */
			etalTestPrintNormal("* Service Select by Subchannel");
			if ((ret = etal_service_select_data(*phDatapath, ETAL_SERVSEL_MODE_DAB_SUBCH, ETAL_SERVSEL_SUBF_APPEND, ueid, sid, sc, subch)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
				return OSAL_ERROR;
			}
		}
		else if (sc != ETAL_INVALID)
		{
			/* Service Select by Service Component */
			etalTestPrintNormal("* Service Select by Service Component");
			if ((ret = etal_service_select_data(*phDatapath, ETAL_SERVSEL_MODE_DAB_SC, ETAL_SERVSEL_SUBF_APPEND, ueid, sid, sc, subch)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
				return OSAL_ERROR;
			}
		}
		else
		{
			/* Service Select by SID */
			etalTestPrintNormal("* Service Select by Sid");
			if ((ret = etal_service_select_data(*phDatapath, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_APPEND, ueid, sid, sc, subch)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
				return OSAL_ERROR;
			}
		}
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestUndoServiceSelectData
 *
 **************************/
static tSInt etalTestUndoServiceSelectData(ETAL_HANDLE hDatapath, tU32 ueid, tU32 sid)
{
	ETAL_STATUS ret;

	/* Remove service */

	etalTestPrintNormal("* Remove DATA service");

	if ((ret = etal_service_select_data(hDatapath, ETAL_SERVSEL_MODE_SERVICE, ETAL_SERVSEL_SUBF_REMOVE, ueid, sid, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_data (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/* Destroy datapath */

	etalTestPrintNormal("* Destroy datapath, DATA service");

	if ((ret = etal_destroy_datapath(&hDatapath)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_datapath (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestDataServices_SLS
 *
 **************************/
tSInt etalTestDataServices_SLS(void)
{
	ETAL_STATUS ret;
	tSInt retval = OSAL_OK;

	ETAL_HANDLE hDatapath;

	tU32 enabledServices = 0;
	EtalDataServiceParam param;

	/*
	 * pass1,
	 */
	
	etalTestPrintNormal("<---SLS Test start");
	
	/*
	 * Assumes the DE-Bayern ETI
	 */
	
	etalTestPrintNormal("Wait some time to allow DAB data capture");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);
	
	/*
	 * Selects audio service 'Rock Antenne'
	 */
	
	if (etalTestDoServiceSelectData(handledab, &hDatapath, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV1_SID, 0, ETAL_INVALID, ETAL_INVALID) != OSAL_OK)
	{
		etalTestPrintError("This test requires DE-Bayern ETI file");
		etalTestPrintError("SLS_XPAD test FAILED");
		return OSAL_ERROR;
	}
	
	/*
	 * Enables the service containing
	 * SLS (Slideshow) images. Captures the images to files on the
	 * filesystem and compares them with reference files.
	 * The service contains three jpeg images, of three different sizes;
	 * the test is considered passed if at least two good images are
	 * received.
	 * Only 2 images are needed because depending on which position the
	 * ETI is when the test starts not all images may be captured
	 */
	
	etalTestDataServ = 0;
	OSAL_pvMemorySet(&param, 0, sizeof(param));
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_SLS_XPAD, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("SLS_XPAD test FAILED");
		return OSAL_ERROR;
	}
	
	/* the ETI is 120s long; use a sligthly different test length so that,
	 * in repeated runs of the test, there is a chance to capture all
	 * three images
	 */
	etalTestPrintNormal("Wait 125 seconds to allow for SLS data delivery");
	OSAL_s32ThreadWait(125 * ETAL_TEST_ONE_SECOND);
	etalTestPrintNormal("Received %d good images", etalTestDataServ);
	if (etalTestDataServ < 2)
	{
		retval = OSAL_ERROR;
		etalTestPrintError("SLS_XPAD test FAILED");
	}
	
	/*
	 * disable SLS over XPAD data service
	 */
	if ((ret = etal_disable_data_service(handledab, ETAL_DATASERV_TYPE_SLS_XPAD)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_disable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("SLS_XPAD test FAILED");
		return OSAL_ERROR;
	}
	
	if (etalTestUndoServiceSelectData(hDatapath, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV1_SID) != OSAL_OK)
	{
		etalTestPrintError("SLS_XPAD test FAILED");
		return OSAL_ERROR;
	}

	if(retval == OSAL_OK)
	{
		etalTestPrintNormal("SLS_XPAD test SUCCESS");
	}
	return retval;
}

/***************************
 *
 * etalTestDataServices_EPG
 *
 **************************/
tSInt etalTestDataServices_EPG(void)
{
	ETAL_STATUS ret;
	tSInt retval = OSAL_OK;

	ETAL_HANDLE hDatapath;

	tU32 enabledServices = 0;
	EtalDataServiceParam param;
	
	etalTestPrintNormal("<---EPG Test start");
	
	/*
	 * Selects data service 'EPG Bayern'
	 */
	
	if (etalTestDoServiceSelectData(handledab, &hDatapath, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD5_SID, 1, ETAL_INVALID, ETAL_INVALID) != OSAL_OK)
	{
		etalTestPrintError("This test requires DE-Bayern file");
		etalTestPrintError("EPG tests FAILED");
		return OSAL_ERROR;
	}
	
	/*
	 * Enables the EPG data services.
	 */
	
	etalTestPrintNormal("<---EPG RAW");
	
	enabledServices = 0;
	etalTestDataServ = 0;
	OSAL_pvMemorySet(&param, 0, sizeof(param));

	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_EPG_RAW, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(10 * ETAL_TEST_ONE_SECOND);
	
	/*
	 * disable EPG RAW data service
	 */
	if ((ret = etal_disable_data_service(handledab, ETAL_DATASERV_TYPE_EPG_RAW)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_disable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("EPG_RAW test FAILED");
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	
	if (etalTestDataServ == 0)
	{
		retval = OSAL_ERROR;
		etalTestPrintError("EPG_RAW test FAILED");
	}
	
	etalTestPrintNormal("<---EPG BIN");

	enabledServices = 0;
	count_EPG_BIN = 0;
	
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_EPG_BIN, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("EPG_BIN test FAILED");
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(30 * ETAL_TEST_ONE_SECOND);

	/*
	 * disable EPG BIN data service
	 */
	if ((ret = etal_disable_data_service(handledab, ETAL_DATASERV_TYPE_EPG_BIN)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_disable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("EPG_BIN test FAILED");
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);

	etalTestPrintNormal("<---EPG SRV");

	enabledServices = 0;
	count_EPG_SRV = 0;
	param.m_ecc = 0xE0;
	param.m_eid = 0x1008;
	
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_EPG_SRV, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("EPG_SRV test FAILED");
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(30 * ETAL_TEST_ONE_SECOND);

	/*
	 * disable EPG SRV data service
	 */
	if ((ret = etal_disable_data_service(handledab, ETAL_DATASERV_TYPE_EPG_SRV)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_disable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("EPG_SRV test FAILED");
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	
	etalTestPrintNormal("<---EPG PRG");
	
	enabledServices = 0;
	count_EPG_PRG = 0;
	param.m_ecc = 0xE0;
	param.m_eid = 0x1008;
	param.m_sid = 0xD312;
	
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_EPG_PRG, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("EPG_PRG test FAILED");
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(60 * ETAL_TEST_ONE_SECOND);
	
	/*
	 * disable EPG PRG data service
	 */
	if ((ret = etal_disable_data_service(handledab, ETAL_DATASERV_TYPE_EPG_PRG)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_disable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("EPG_PRG test FAILED");
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	
	etalTestPrintNormal("<---EPG LOGO");
	
	enabledServices = 0;
	count_EPG_LOGO = 0;
	param.m_ecc = 0xE0;
	param.m_eid = 0x1008;
	param.m_sid = 0xD312;
	param.m_logoType = 1; //unrestricted
	
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_EPG_LOGO, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("EPG_LOGO test FAILED");
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(30 * ETAL_TEST_ONE_SECOND);
	
	/*
	 * disable EPG LOGO data service
	 */
	if ((ret = etal_disable_data_service(handledab, ETAL_DATASERV_TYPE_EPG_LOGO)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_disable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("EPG_LOGO test FAILED");
		return OSAL_ERROR;
	}
	
	if (count_EPG_BIN == 0)
	{
		//Always failing ; do not hamper test results
		retval = OSAL_ERROR;
		etalTestPrintError("EPG_BIN test FAILED");
	}
	
	if (count_EPG_SRV == 0)
	{
		retval = OSAL_ERROR;
		etalTestPrintError("EPG_SRV test FAILED");
	}
	
	if (count_EPG_PRG == 0)
	{
		retval = OSAL_ERROR;
		etalTestPrintError("EPG_PRG test FAILED");
	}
	
	if (count_EPG_LOGO == 0)
	{
		retval = OSAL_ERROR;
		etalTestPrintError("EPG_LOGO test FAILED");
	}
	
	/*
	 * cleanup
	 */
	if (etalTestUndoServiceSelectData(hDatapath, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERVD5_SID) != OSAL_OK)
	{
		etalTestPrintError("EPG tests FAILED");
		return OSAL_ERROR;
	}
	if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		etalTestPrintError("EPG tests FAILED");
		return OSAL_ERROR;
	}

	if(retval == OSAL_OK)
	{
		etalTestPrintNormal("EPG tests SUCCESS");
	}
	return retval;
}

/***************************
 *
 * etalTestDataServices_JML
 *
 **************************/
tSInt etalTestDataServices_JML(void)
{
// For now, no Journaline content is available
	tSInt retval = OSAL_OK;
#if 0
	ETAL_STATUS ret;

	tU32 enabledServices = 0;
	EtalDataServiceParam param;

	etalTestPrintNormal("<---Journaline Test start");
	
// TODO : handle two or more DAB frequencies
	
	/*
	 * Assumes the FraunhoferIIS-JournalineVariety ETI
	 */
	
	etalTestPrintNormal("Wait some time to allow DAB data capture");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);
	
	/*
	 * Selects data service 'Journaline'
	 */
	
	if (etalTestDoServiceSelectData(handledab, &hDatapath, 0xE0D123, 0xD125, 0, ETAL_INVALID, ETAL_INVALID) != OSAL_OK)
	{
		etalTestPrintError("This test requires FraunhoferIIS-JournalineVariety file");
		etalTestPrintError("JML_OBJ test FAILED");
		return OSAL_ERROR;
	}
	
	/*
	 * Enables the service containing
	 * Journaline. First ask for root menu (Object ID 0) then selects other JML objects
	 */
	
	enabledServices = 0;
	etalTestDataServ = 0;
	param.m_JMLObjectId = 0;
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_JML_OBJ, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("JML_OBJ test FAILED");
		return OSAL_ERROR;
	}
	
	do
	{
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	}
	while (!etalTestDataServ);
	
	etalTestDataServ = 0;
	param.m_JMLObjectId = 60001; /* News ticker */
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_JML_OBJ, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("JML_OBJ test FAILED");
		return OSAL_ERROR;
	}
	
	do
	{
		OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	}
	while (etalTestDataServ<3);
	
	/* TODO need the possibility to break the loop */
	
	if (!pass3)
	{
		etalTestPrintError("JML_OBJ test FAILED");
	}
	
	/*
	 * cleanup
	 */
	if (etalTestUndoServiceSelectData(hDatapath, ETAL_DAB_ETI4_UEID, ETAL_DAB_ETI4_SERVD1_SID) != OSAL_OK)
	{
		etalTestPrintError("JML_OBJ test FAILED");
		return OSAL_ERROR;
	}

	if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		etalTestPrintError("JML_OBJ test FAILED");
		return OSAL_ERROR;
	}

	if(retval == OSAL_OK)
	{
		etalTestPrintNormal("JML_OBJ tests SUCCESS");
	}
#else
	if(retval == OSAL_OK)
	{
		etalTestPrintNormal("JML_OBJ tests not started");
	}
#endif
	return retval;
}

/***************************
 *
 * etalTestDataServices_TPEG
 *
 **************************/
tSInt etalTestDataServices_TPEG(void)
{
	ETAL_STATUS ret;
	tSInt retval = OSAL_OK;

	ETAL_HANDLE hDatapath;

	tU32 enabledServices = 0;
	EtalDataServiceParam param;

	
	etalTestPrintNormal("<---TPEG Test start");
	/*
	 * Assumes the TPEG-Fraunhofer-2010-10-25-1 ETI
	 */
	
	etalTestPrintNormal("Wait some time to allow DAB data capture");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);
	
	/*
	 * Selects data service 'RTM'
	 */
	
	if (etalTestDoServiceSelectData(handledab, &hDatapath, ETAL_DAB_ETI2_UEID, ETAL_DAB_ETI2_SERVD1_SID, 1, ETAL_INVALID, ETAL_INVALID) != OSAL_OK)
	{
		etalTestPrintError("This test requires TPEG-Fraunhofer-2010-10-25-1 file");
		etalTestPrintError("TPEG tests FAILED");
		return OSAL_ERROR;
	}
	
	/*
	 * Enables the TPEG data services.
	 */
	
	etalTestPrintNormal("<---TPEG RAW");
	
	enabledServices = 0;
	etalTestDataServ = 0;
	OSAL_pvMemorySet(&param, 0, sizeof(param));
	
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_TPEG_RAW, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("TPEG_RAW test FAILED");
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	
	/*
	 * disable TPEG RAW data service
	 */
	if ((ret = etal_disable_data_service(handledab, ETAL_DATASERV_TYPE_TPEG_RAW)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_disable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("TPEG_RAW test FAILED");
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(1 * ETAL_TEST_ONE_SECOND);
	
	if (etalTestDataServ == 0)
	{
		retval = OSAL_ERROR;
		etalTestPrintError("TPEG_RAW test FAILED");
	}
	
	etalTestPrintNormal("<---TPEG SNI");
	
	enabledServices = 0;
	etalTestDataServ = 0;
	
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_TPEG_SNI, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("TPEG_SNI test FAILED");
		return OSAL_ERROR;
	}
	
	OSAL_s32ThreadWait(5 * ETAL_TEST_ONE_SECOND);
	
	if (etalTestDataServ == 0)
	{
		retval = OSAL_ERROR;
		etalTestPrintError("TPEG_SNI test FAILED");
	}
	
	/*
	 * cleanup
	 */
	if (etalTestUndoServiceSelectData(hDatapath, ETAL_DAB_ETI2_UEID, ETAL_DAB_ETI2_SERVD1_SID) != OSAL_OK)
	{
		etalTestPrintError("TPEG tests FAILED");
		return OSAL_ERROR;
	}
	if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
	{
		etalTestPrintError("TPEG tests FAILED");
		return OSAL_ERROR;
	}

	if(retval == OSAL_OK)
	{
		etalTestPrintNormal("TPEG tests SUCCESS");
	}
	return retval;
}

#if defined(CONFIG_APP_TEST_DATASERVICES_DLS)

/***************************
 *
 * etalTestDataServices_DLS
 *
 **************************/
tSInt etalTestDataServices_DLS(void)
{
	ETAL_STATUS ret;
	tSInt retval = OSAL_OK;

	EtalDataPathAttr dataPathAttr;
	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE;
	tU32 enabledServices = 0;
	EtalDataServiceParam param;

	etalTestPrintNormal("<---DLS Test start (ETAL_DATA_TYPE_DAB_DLS)");

	etalTestPrintNormal("Wait some time to allow DAB data capture");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

	etalTestPrintNormal("* Service Select by Sid");
	if ((ret = etal_service_select_audio(handledab, ETAL_SERVSEL_MODE_SERVICE, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DLS test FAILED");
		return OSAL_ERROR;
	}

	/*
	 * Configure datapath
	 */
	OSAL_pvMemorySet((tPVoid)&dataPathAttr, 0x00, sizeof(EtalDataPathAttr));
	dataPathAttr.m_receiverHandle = handledab;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_DAB_DLS;
	dataPathAttr.m_sink.m_context = NULL;
	dataPathAttr.m_sink.m_BufferSize = sizeof(etalPADDLSTy);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestPADDLSCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for Service Select (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DLS test FAILED");
		return OSAL_ERROR;
	}

	/*
	 * Enables the DLS data services.
	 */
	DLSCbInvocations = 0;
	enabledServices = 0;
	OSAL_pvMemorySet(&param, 0, sizeof(param));
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_DLS, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DLS test FAILED");
		return OSAL_ERROR;
	}

	OSAL_s32ThreadWait(60 * ETAL_TEST_ONE_SECOND);

	etalTestPrintNormal("= complete, %d DLS callback invocations", DLSCbInvocations);
	etalTestPrintNormal("= (expected at least 1)");

	/* check number of callback invocations */
	if(DLSCbInvocations == 0)
	{
		etalTestPrintError("DLS test FAILED");
		retval = OSAL_ERROR;
	}

	/*
	 * disable DLS service
	 */
	if ((ret = etal_disable_data_service(handledab, ETAL_DATASERV_TYPE_DLS)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_disable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DLS test FAILED");
		return OSAL_ERROR;
	}

	/*
	 * cleanup
	 */
	if ((ret = etal_destroy_datapath(&hDatapath)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("DLS tests FAILED");
		return OSAL_ERROR;
	}

	etalTestPrintNormal("<---DLS Test start (ETAL_DATA_TYPE_DATA_SERVICE)");

	if (etalTestDoServiceSelectData(handledab, &hDatapath, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV1_SID, 0, ETAL_INVALID, ETAL_INVALID) != OSAL_OK)
	{
		etalTestPrintError("This test requires DE-Bayern ETI file");
		etalTestPrintError("DLS tests FAILED");
		return OSAL_ERROR;
	}

	/*
	 * Enables the DLS data services.
	 */
	DLSCbInvocations = 0;
	enabledServices = 0;
	OSAL_pvMemorySet(&param, 0, sizeof(param));
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_DLS, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DLS test FAILED");
		return OSAL_ERROR;
	}

	OSAL_s32ThreadWait(60 * ETAL_TEST_ONE_SECOND);

	etalTestPrintNormal("= complete, %d DLS callback invocations", DLSCbInvocations);
	etalTestPrintNormal("= (expected at least 1)");

	/* check number of callback invocations */
	if(DLSCbInvocations == 0)
	{
		etalTestPrintError("DLS test FAILED");
		retval = OSAL_ERROR;
	}

	/*
	 * disable DLS service
	 */
	if ((ret = etal_disable_data_service(handledab, ETAL_DATASERV_TYPE_DLS)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_disable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DLS test FAILED");
		return OSAL_ERROR;
	}

	/*
	 * cleanup
	 */
	if (etalTestUndoServiceSelectData(hDatapath, ETAL_DAB_ETI3_UEID, ETAL_DAB_ETI3_SERV1_SID) != OSAL_OK)
	{
		etalTestPrintError("DLS tests FAILED");
		return OSAL_ERROR;
	}

	if ((ret = etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab)) != OSAL_OK)
	{
		etalTestPrintError("DLS tests FAILED");
		return OSAL_ERROR;
	}

	if(retval == OSAL_OK)
	{
		etalTestPrintNormal("DLS tests SUCCESS");
	}
	return retval;
}

#endif // #if defined(CONFIG_APP_TEST_DATASERVICES_DLS)

#if defined(CONFIG_APP_TEST_DATASERVICES_DLPLUS)
/***************************
 *
 * etalTestDataServices_DLPLUS
 *
 **************************/
tSInt etalTestDataServices_DLPLUS(void)
{
	ETAL_STATUS ret;
	tSInt retval = OSAL_OK;
	EtalDataPathAttr dataPathAttr;
	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE;
	tU32 enabledServices = 0;
	EtalDataServiceParam param;

	etalTestPrintNormal("<---DL PLUS Test start (ETAL_DATA_TYPE_DAB_DLPLUS)");

	etalTestPrintNormal("Wait some time to allow DAB data capture");
	OSAL_s32ThreadWait(ETAL_TEST_DAB_TUNE_SETTLE_TIME);

	etalTestPrintNormal("* Service Select by Sid");
	if ((ret = etal_service_select_audio(handledab, ETAL_SERVSEL_MODE_SERVICE, ETAL_DAB_ETI6_UEID, ETAL_DAB_ETI6_SERV1_SID, ETAL_INVALID, ETAL_INVALID)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_service_select_audio (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DL PLUS test FAILED");
		return OSAL_ERROR;
	}

	/*
	 * Configure datapath
	 */
	OSAL_pvMemorySet((tPVoid)&dataPathAttr, 0x00, sizeof(EtalDataPathAttr));
	dataPathAttr.m_receiverHandle = handledab;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_DAB_DLPLUS;
	dataPathAttr.m_sink.m_context = NULL;
	dataPathAttr.m_sink.m_BufferSize = sizeof(etalPADDLPLUSTy);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestPADDLPLUSCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for Service Select (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DL PLUS test FAILED");
		return OSAL_ERROR;
	}

	/*
	 * Enables the DL PLUS data services.
	 */
	DLPLUSCbInvocations = 0;
	enabledServices = 0;
	OSAL_pvMemorySet(&param, 0, sizeof(param));
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_DLPLUS, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DL PLUS test FAILED");
		return OSAL_ERROR;
	}

	OSAL_s32ThreadWait(130 * ETAL_TEST_ONE_SECOND);

	etalTestPrintNormal("= complete, %d DL PLUS callback invocations", DLPLUSCbInvocations);
	etalTestPrintNormal("= (expected at least 1)");

	/* check number of callback invocations */
	if(DLPLUSCbInvocations == 0)
	{
		etalTestPrintError("DL PLUS test FAILED");
		retval = OSAL_ERROR;
	}

	/*
	 * disable DL PLUS service
	 */
	if ((ret = etal_disable_data_service(handledab, ETAL_DATASERV_TYPE_DLPLUS)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_disable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DL PLUS test FAILED");
		return OSAL_ERROR;
	}

	/*
	 * cleanup
	 */
	if ((ret = etal_destroy_datapath(&hDatapath)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("DL PLUS tests FAILED");
		return OSAL_ERROR;
	}

	etalTestPrintNormal("<---DL PLUS Test start (ETAL_DATA_TYPE_DATA_SERVICE)");

	if (etalTestDoServiceSelectData(handledab, &hDatapath, ETAL_DAB_ETI6_UEID, ETAL_DAB_ETI6_SERV1_SID, 0, ETAL_INVALID, ETAL_INVALID) != OSAL_OK)
	{
		etalTestPrintError("This test requires FraunhoferIIS-International_Fig2_DLPlus-2010-01-28 ETI file");
		etalTestPrintError("DL PLUS tests FAILED");
		return OSAL_ERROR;
	}

	/*
	 * Enables the DLS data services.
	 */
	DLPLUSCbInvocations = 0;
	enabledServices = 0;
	OSAL_pvMemorySet(&param, 0, sizeof(param));
	if ((ret = etal_enable_data_service(handledab, ETAL_DATASERV_TYPE_DLPLUS, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DL PLUS test FAILED");
		return OSAL_ERROR;
	}

	OSAL_s32ThreadWait(130 * ETAL_TEST_ONE_SECOND);

	etalTestPrintNormal("= complete, %d DL PLUS callback invocations", DLPLUSCbInvocations);
	etalTestPrintNormal("= (expected at least 1)");

	/* check number of callback invocations */
	if(DLPLUSCbInvocations == 0)
	{
		etalTestPrintError("DL PLUS test FAILED");
		retval = OSAL_ERROR;
	}

	/*
	 * disable DLS service
	 */
	if ((ret = etal_disable_data_service(handledab, ETAL_DATASERV_TYPE_DLPLUS)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_disable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		etalTestPrintError("DL PLUS test FAILED");
		return OSAL_ERROR;
	}

	/*
	 * cleanup
	 */
	if (etalTestUndoServiceSelectData(hDatapath, ETAL_DAB_ETI6_UEID, ETAL_DAB_ETI6_SERV1_SID) != OSAL_OK)
	{
		etalTestPrintError("DL PLUS tests FAILED");
		return OSAL_ERROR;
	}

	if ((ret = etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab)) != OSAL_OK)
	{
		etalTestPrintError("DL PLUS tests FAILED");
		return OSAL_ERROR;
	}

	if(retval == OSAL_OK)
	{
		etalTestPrintNormal("DLS tests SUCCESS");
	}

	return retval;
}
#endif // #if defined(CONFIG_APP_TEST_DATASERVICES_DLPLUS)
#endif // CONFIG_APP_TEST_DAB

#if defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
#ifdef CONFIG_APP_TEST_DATASERVICES_PSD
/***************************
 *
 * etalTestDataServicesHDPass1
 *
 **************************/
static tSInt etalTestDataServicesHDPass1(etalTestBroadcastTy test_mode, ETAL_HANDLE hReceiver, tBool *pass)
{
	ETAL_STATUS ret;
	tSInt retval = OSAL_OK;
	
	ETAL_HANDLE hDatapath;
	EtalDataPathAttr dataPathAttr;

	tU32 enabledServices = 0;
	EtalDataServiceParam param;
	tU16 PSDServiceEnableBitmap;

#define ETAL_TEST_DATASERV_NAMELEN 32
	tChar name[ETAL_TEST_DATASERV_NAMELEN];
	FILE *fp;

	tU32 file_size;

	tU8 PSD_file_nb;
	
	tU32 offset = 0;
	tU8 content[256];
	tU8 length;

	tyHDRADIOPSDFieldBitMask field;

	tBool titleFound = FALSE;

	/*
	 * pass1,
	 */
	
	PSDServiceEnableBitmap = 0x00FF; /* Title, Artist, Album, Genre, Comment, UFID, Commercial, XHDR */
	
	if ((ret = etal_setup_PSD(hReceiver, PSDServiceEnableBitmap, NULL, NULL)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_setup_PSD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	hDatapath = ETAL_INVALID_HANDLE;
	OSAL_pvMemorySet((tPVoid)&dataPathAttr, 0x00, sizeof(EtalDataPathAttr));
	dataPathAttr.m_receiverHandle = hReceiver;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_DATA_SERVICE;
	dataPathAttr.m_sink.m_BufferSize = 0x00;
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestDataServCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for PSD Test (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestDataServ = 0;
	OSAL_pvMemorySet(&param, 0, sizeof(param));
	if ((ret = etal_enable_data_service(hReceiver, ETAL_DATASERV_TYPE_PSD, &enabledServices, param)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_enable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	
	etalTestPrintNormal("Wait 40 seconds to allow for PSD delivery");
	OSAL_s32ThreadWait(40 * ETAL_TEST_ONE_SECOND);

	/*
	 * stop PSD
	 */
	if ((ret = etal_disable_data_service(hReceiver, ETAL_DATASERV_TYPE_PSD)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_disable_data_service (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(test_mode, "Read PSD");

	/*
	 * ensure all the callbacks have been processed
	 */
	OSAL_s32ThreadWait(200);

	etalTestPrintNormal("Received %d PSD data", etalTestDataServ);
	if (etalTestDataServ == 0)
	{
		*pass = FALSE;
		etalTestPrintNormal("pass1 FAILED");
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);

		etalTestPrintReportPassStart(test_mode, "Validate PSD");
		/*
		 * We need some magic here
		 * to verify PSD content and confirm that test has been passed
		 * Title will be checked against known content from HD generator
		 */
		for (PSD_file_nb = etalTestDataServ; PSD_file_nb != 0; PSD_file_nb--)
		{
			//Open PSD file and get size
			OSAL_s32PrintFormat(name, "%sPSD-%d.txt", ROOT_FS, PSD_file_nb-1);
			fp = fopen(name, "r");
			if (fp == NULL)
			{
				etalTestPrintError("opening file %s", name);
				retval = OSAL_ERROR;
			}
			else
			{
				//Get size
				fseek(fp, 0, SEEK_END);
				file_size = ftell(fp);
				rewind(fp);
				//Check first byte
				if (fread((tPVoid)(&content[0]), (size_t)1, (size_t)1, fp) != (size_t)1)
				{
					etalTestPrintError("First byte wrong, reading file %s", name);
					retval = OSAL_ERROR;
				}
				offset++;
				if(content[0] != 0x02)
				{
					retval = OSAL_ERROR;
				}
				else
				{
					offset = 1;
					//Frame parse loop (til offset reach size)
					do
					{
						//Offset 0 : check that Program Number is 0
						if (fread((tPVoid)(&content[0]), (size_t)1, (size_t)7, fp) != (size_t)7)
						{
							etalTestPrintError("Header bytes wrong, reading file %s", name);
							retval = OSAL_ERROR;
							break;
						}
						if(content[0] != 0x00)
						{
							retval = OSAL_ERROR;
							break;
						}
						//Offset 1 : check the field
						field = content[1];
						//for all cases, length is at Offset 6 ; data is at Offset 7
						length = content[6];
						OSAL_pvMemorySet(content, 0, sizeof(tU8)*256);
						if (fread((tPVoid)(&content[0]), (size_t)1, (size_t)length, fp) != (size_t)length)
						{
							etalTestPrintError("Data bytes reading error | file %s", name);
							retval = OSAL_ERROR;
						}
						//Switch field
						switch (field)
						{
							//case Title: display string
							case TITLE:
								etalTestPrintNormal("Title: %s", content);
								if (test_mode == ETAL_TEST_MODE_HD_FM)
								{
									if (strcmp((char *)content,PSD_HDFM_TITLE_1) == 0)
									{
										titleFound = true;
									}
									else if	(strcmp((char *)content,PSD_HDFM_TITLE_2) == 0)
									{
										titleFound = true;
									}
									else if	(strcmp((char *)content,PSD_HDFM_TITLE_3) == 0)
									{
										titleFound = true;
									}
									else if	(strcmp((char *)content,PSD_HDFM_TITLE_4) == 0)
									{
										titleFound = true;
									}
								}
								else // ETAL_TEST_MODE_HD_AM
								{
									if (strcmp((char *)content,PSD_HDAM_TITLE_1) == 0)
									{
										titleFound = true;
									}
								}
								break;
							//case Artist: display string
							case ARTIST:
								etalTestPrintNormal("Artist: %s", content);
								break;
							//case Album: display string
							case ALBUM:
								etalTestPrintNormal("Album: %s", content);
								break;
							//case Genre: display string
							case GENRE:
								etalTestPrintNormal("Genre: %s", content);
								break;
							//other cases: support later
							default:
								break;
						}
						offset += 7 + length;
						//Next loop iteration
					}while (offset < file_size);
				}
			}
			//Only display for now
			fclose(fp);
		}
		if (FALSE == titleFound)
		{
			*pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}

	/*
	 * cleanup
	 */
	if (etal_destroy_datapath(&hDatapath) != ETAL_RET_SUCCESS)
	{
		return OSAL_ERROR;
	}

	if(retval == OSAL_OK)
	{
		etalTestPrintNormal("PSD test1 SUCCESS");
	}
	return retval;
}

/***************************
 *
 * etalTestDataServicesHDPass2
 *
 **************************/
static tSInt etalTestDataServicesHDPass2(etalTestBroadcastTy test_mode, ETAL_HANDLE hReceiver, tBool *pass)
{
	ETAL_STATUS ret;
	tBool local_pass = TRUE;

	tU16 PSDServiceEnableBitmap;
	EtalPSDLength lenSettings;

	/*
	 * pass2,
	 */
	
	etalTestPrintReportPassStart(test_mode, "Parameter checks");

	PSDServiceEnableBitmap = 0x00FF; /* Title, Artist, Album, Genre, Comment, UFID, Commercial, XHDR */
	
	if ((ret = etal_setup_PSD(ETAL_INVALID_HANDLE,PSDServiceEnableBitmap, NULL, NULL)) != ETAL_RET_INVALID_HANDLE)
	{
		etalTestPrintNormal("etal_setup_PSD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		local_pass = FALSE;
		etalTestPrintNormal("pass2a FAILED");
	}

	/* too short */
	lenSettings.m_PSDAlbumLength = 5;

	if ((ret = etal_setup_PSD(hReceiver,PSDServiceEnableBitmap, &lenSettings, NULL)) != ETAL_RET_ERROR)
	{
		etalTestPrintNormal("etal_setup_PSD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		local_pass = FALSE;
		etalTestPrintNormal("pass2b FAILED");
	}

	/* too big */
	lenSettings.m_PSDAlbumLength = 250;

	if ((ret = etal_setup_PSD(hReceiver,PSDServiceEnableBitmap, &lenSettings, NULL)) != ETAL_RET_ERROR)
	{
		etalTestPrintNormal("etal_setup_PSD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		local_pass = FALSE;
		etalTestPrintNormal("pass2c FAILED");
	}
	
	if (!local_pass)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintNormal("PSD test2 SUCCESS");
		etalTestPrintReportPassEnd(testPassed);
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestDataServicesHDPass3
 *
 **************************/
static tSInt etalTestDataServicesHDPass3(etalTestBroadcastTy test_mode, ETAL_HANDLE hReceiver, tBool *pass)
{
	ETAL_STATUS ret;
	tBool local_pass = TRUE;

	tU16 PSDServiceEnableBitmap;
	EtalPSDLength lenSettings;
	EtalPSDLength lenRead;

	/*
	 * pass3,
	 */
	
	etalTestPrintReportPassStart(test_mode, "Setup PSD Length");

	PSDServiceEnableBitmap = 0x00FF; /* Title, Artist, Album, Genre, Comment, UFID, Commercial, XHDR */
	/* Arbitrary values */
	lenSettings.m_PSDAlbumLength = 48;
	lenSettings.m_PSDArtistLength = 48;
	lenSettings.m_PSDCommentLength = 64;
	lenSettings.m_PSDCommentShortLength = 32;
	lenSettings.m_PSDCommercialContactLength = 48;
	lenSettings.m_PSDCommercialDescriptionLength = 48;
	lenSettings.m_PSDCommercialPriceLength = 48;
	lenSettings.m_PSDCommercialSellerLength = 48;
	lenSettings.m_PSDGenreLength = 48;
	lenSettings.m_PSDTitleLength = 48;
	lenSettings.m_PSDUFIDLength = 64;
	lenSettings.m_PSDXHDRLength = 64;
	
	if ((ret = etal_setup_PSD(hReceiver,PSDServiceEnableBitmap, &lenSettings, NULL)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintNormal("etal_setup_PSD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		local_pass = FALSE;
		etalTestPrintNormal("pass3a FAILED");
	}

	if ((ret = etal_setup_PSD(hReceiver,PSDServiceEnableBitmap, NULL, &lenRead)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintNormal("etal_setup_PSD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		local_pass = FALSE;
		etalTestPrintNormal("pass3b FAILED");
	}
	if (lenRead.m_PSDAlbumLength != lenSettings.m_PSDAlbumLength)
	{
		local_pass = FALSE;
		etalTestPrintNormal("pass3c1 FAILED");
	}
	if (lenRead.m_PSDAlbumLength != lenSettings.m_PSDAlbumLength)
	{
		local_pass = FALSE;
		etalTestPrintNormal("pass3c2 FAILED");
	}
	if (lenRead.m_PSDAlbumLength != lenSettings.m_PSDAlbumLength)
	{
		local_pass = FALSE;
		etalTestPrintNormal("pass3c3 FAILED");
	}
	if (lenRead.m_PSDAlbumLength != lenSettings.m_PSDAlbumLength)
	{
		local_pass = FALSE;
		etalTestPrintNormal("pass3c4 FAILED");
	}
	if (lenRead.m_PSDAlbumLength != lenSettings.m_PSDAlbumLength)
	{
		local_pass = FALSE;
		etalTestPrintNormal("pass3c5 FAILED");
	}
	if (lenRead.m_PSDAlbumLength != lenSettings.m_PSDAlbumLength)
	{
		local_pass = FALSE;
		etalTestPrintNormal("pass3c6 FAILED");
	}
	if (lenRead.m_PSDAlbumLength != lenSettings.m_PSDAlbumLength)
	{
		local_pass = FALSE;
		etalTestPrintNormal("pass3c7 FAILED");
	}
	if (lenRead.m_PSDAlbumLength != lenSettings.m_PSDAlbumLength)
	{
		local_pass = FALSE;
		etalTestPrintNormal("pass3c8 FAILED");
	}
	if (lenRead.m_PSDAlbumLength != lenSettings.m_PSDAlbumLength)
	{
		local_pass = FALSE;
		etalTestPrintNormal("pass3c9 FAILED");
	}
	if (lenRead.m_PSDAlbumLength != lenSettings.m_PSDAlbumLength)
	{
		local_pass = FALSE;
		etalTestPrintNormal("pass3c10 FAILED");
	}
	if (lenRead.m_PSDAlbumLength != lenSettings.m_PSDAlbumLength)
	{
		local_pass = FALSE;
		etalTestPrintNormal("pass3c11 FAILED");
	}
	if (lenRead.m_PSDAlbumLength != lenSettings.m_PSDAlbumLength)
	{
		local_pass = FALSE;
		etalTestPrintNormal("pass3c12 FAILED");
	}

	if (!local_pass)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintNormal("PSD test3 SUCCESS");
		etalTestPrintReportPassEnd(testPassed);
	}
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_DATASERVICES_PSD
#endif // CONFIG_APP_TEST_HDRADIO_FM || CONFIG_APP_TEST_HDRADIO_AM

#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_AM)
#ifdef CONFIG_APP_TEST_DATASERVICES_PSD
/***************************
 *
 * etalTestDataServicesNonHD
 *
 **************************/
static tSInt etalTestDataServicesNonHD(etalTestBroadcastTy test_mode, ETAL_HANDLE hReceiver, tBool *pass)
{
	ETAL_STATUS ret;
	tBool local_pass = TRUE;
	tU16 PSDServiceEnableBitmap;

	etalTestPrintReportPassStart(test_mode, "Check parameter");

	PSDServiceEnableBitmap = 0x00FF; /* Title, Artist, Album, Genre, Comment, UFID, Commercial, XHDR */

	if ((ret = etal_setup_PSD(hReceiver, PSDServiceEnableBitmap, NULL, NULL)) != ETAL_RET_PARAMETER_ERR)
	{
		etalTestPrintNormal("etal_setup_PSD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		local_pass = FALSE;
		etalTestPrintNormal("pass4 FAILED");
	}

	if (!local_pass)
	{
		*pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintNormal("PSD test SUCCESS");
		etalTestPrintReportPassEnd(testPassed);
	}

	return OSAL_OK;
}
#endif
#endif

#if defined(CONFIG_APP_TEST_DATASERVICES_SLS) || defined(CONFIG_APP_TEST_DATASERVICES_EPG) || defined(CONFIG_APP_TEST_DATASERVICES_JOURNALINE) || defined(CONFIG_APP_TEST_DATASERVICES_TPEG) || defined(CONFIG_APP_TEST_DATASERVICES_DLS) || defined(CONFIG_APP_TEST_DATASERVICES_DLPLUS)
/***************************
 *
 * etalTestDataServices_internal
 *
 **************************/
static tSInt etalTestDataServices_internal(etalTestBroadcastTy test_mode, EtalDataServiceType serviceType, tBool *pass)
{
	etalTestConfigTy config_mode;
	etalTestTuneTy tune_mode;
	ETAL_HANDLE *phReceiver;

	switch (test_mode)
	{
		case ETAL_TEST_MODE_DAB:
			config_mode = ETAL_CONFIG_DAB;
			tune_mode = ETAL_TUNE_DAB; // not used
			phReceiver = &handledab; // not used
			break;

		case ETAL_TEST_MODE_FM:
			config_mode = ETAL_CONFIG_FM1;
			tune_mode = ETAL_TUNE_FM;
			phReceiver = &handlefm;
			break;

		case ETAL_TEST_MODE_AM:
			config_mode = ETAL_CONFIG_AM;
			tune_mode = ETAL_TUNE_AM;
			phReceiver = &handleam;
			break;

		case ETAL_TEST_MODE_HD_FM:
			config_mode = ETAL_CONFIG_HDRADIO_FM;
			tune_mode = ETAL_TUNE_HDRADIO_FM;
			phReceiver = &handlehd;
			break;

		case ETAL_TEST_MODE_HD_AM:
			config_mode = ETAL_CONFIG_HDRADIO_AM;
			tune_mode = ETAL_TUNE_HDRADIO_AM;
			phReceiver = &handlehdam;
			break;

		default:
			return OSAL_ERROR;
	}

	if (etalTestDoConfigSingle(config_mode, phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(tune_mode, *phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	
	if (test_mode == ETAL_TEST_MODE_DAB)
	{
#if defined (CONFIG_APP_TEST_DAB)

#if defined(CONFIG_APP_TEST_DATASERVICES_SLS)
		if(serviceType == ETAL_DATASERV_TYPE_SLS_XPAD)
		{
			if (OSAL_OK != etalTestDataServices_SLS())
			{
				*pass = FALSE;
			}
		}
#endif

#if defined(CONFIG_APP_TEST_DATASERVICES_EPG)
		if((serviceType == ETAL_DATASERV_TYPE_EPG_RAW) ||
		   (serviceType == ETAL_DATASERV_TYPE_EPG_BIN) ||
		   (serviceType == ETAL_DATASERV_TYPE_EPG_SRV) ||
		   (serviceType == ETAL_DATASERV_TYPE_EPG_PRG) ||
		   (serviceType == ETAL_DATASERV_TYPE_EPG_LOGO))
		{
			if (OSAL_OK != etalTestDataServices_EPG())
			{
				*pass = FALSE;
			}
		}
#endif

#if defined(CONFIG_APP_TEST_DATASERVICES_TPEG)

		if((serviceType == ETAL_DATASERV_TYPE_TPEG_RAW) ||
		   (serviceType == ETAL_DATASERV_TYPE_TPEG_SNI))
		{
			if (OSAL_OK != etalTestDataServices_TPEG())
			{
				*pass = FALSE;
			}
		}
#endif

#if defined(CONFIG_APP_TEST_DATASERVICES_JOURNALINE)
		if(serviceType == ETAL_DATASERV_TYPE_JML_OBJ)
		{
			if (OSAL_OK != etalTestDataServices_JML())
			{
				*pass = FALSE;
			}
		}
#endif

#if defined(CONFIG_APP_TEST_DATASERVICES_DLS)
		if(serviceType == ETAL_DATASERV_TYPE_DLS)
		{
			if (OSAL_OK != etalTestDataServices_DLS())
			{
				*pass = FALSE;
			}
		}
#endif

#if defined(CONFIG_APP_TEST_DATASERVICES_DLPLUS)
		if(serviceType == ETAL_DATASERV_TYPE_DLPLUS)
		{
			if (OSAL_OK != etalTestDataServices_DLPLUS())
			{
				*pass = FALSE;
			}
		}
#endif
#endif // CONFIG_APP_TEST_DAB
	}

	if (etalTestUndoTuneSingle(tune_mode, *phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}
#endif

#ifdef CONFIG_APP_TEST_DATASERVICES_PSD
/***************************
 *
 * etalTestDataServices_PSD
 *
 **************************/
static tSInt etalTestDataServices_PSD(etalTestBroadcastTy test_mode, tBool *pass)
{
	etalTestConfigTy config_mode;
	etalTestTuneTy tune_mode;
	ETAL_HANDLE *phReceiver;

	switch (test_mode)
	{
		case ETAL_TEST_MODE_DAB:
			config_mode = ETAL_CONFIG_DAB;
			tune_mode = ETAL_TUNE_DAB; // not used
			phReceiver = &handledab; // not used
			break;

		case ETAL_TEST_MODE_FM:
			config_mode = ETAL_CONFIG_FM1;
			tune_mode = ETAL_TUNE_FM;
			phReceiver = &handlefm;
			break;

		case ETAL_TEST_MODE_AM:
			config_mode = ETAL_CONFIG_AM;
			tune_mode = ETAL_TUNE_AM;
			phReceiver = &handleam;
			break;

		case ETAL_TEST_MODE_HD_FM:
			config_mode = ETAL_CONFIG_HDRADIO_FM;
			tune_mode = ETAL_TUNE_HDRADIO_FM;
			phReceiver = &handlehd;
			break;

		case ETAL_TEST_MODE_HD_AM:
			config_mode = ETAL_CONFIG_HDRADIO_AM;
			tune_mode = ETAL_TUNE_HDRADIO_AM;
			phReceiver = &handlehdam;
			break;

		default:
			return OSAL_ERROR;
	}

	if (etalTestDoConfigSingle(config_mode, phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(tune_mode, *phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

 if ((test_mode == ETAL_TEST_MODE_HD_FM) || (test_mode == ETAL_TEST_MODE_HD_AM))
	{
#if defined (CONFIG_APP_TEST_HDRADIO_FM) || defined (CONFIG_APP_TEST_HDRADIO_AM)
		if (etalTestDataServicesHDPass1(test_mode, *phReceiver, pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}

		if (etalTestDataServicesHDPass2(test_mode, *phReceiver, pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}

		if (etalTestDataServicesHDPass3(test_mode, *phReceiver, pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
#endif // CONFIG_APP_TEST_HDRADIO_FM || CONFIG_APP_TEST_HDRADIO_AM
	}
	else
	{
#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_AM)
		/* for non-HDRadio just check if the request is rejected */
		if (etalTestDataServicesNonHD(test_mode, *phReceiver, pass) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
#endif // CONFIG_APP_TEST_FM || CONFIG_APP_TEST_AM
	}

	if (etalTestUndoTuneSingle(tune_mode, *phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoConfigSingle(phReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_DATASERVICES_PSD
#endif // CONFIG_ETAL_HAVE_DATASERVICES || CONFIG_ETAL_HAVE_ALL_API
#endif // CONFIG_APP_TEST_DATASERVICES

/***************************
 *
 * etalTestDataServices
 *
 **************************/
tSInt etalTestDataServices(void)
{
#ifdef CONFIG_APP_TEST_DATASERVICES
	tBool pass = TRUE;

	etalTestStartup();

#ifdef CONFIG_APP_TEST_DAB
#ifdef CONFIG_APP_TEST_DATASERVICES_SLS
	if (OSAL_OK != etalTestDataServices_internal(ETAL_TEST_MODE_DAB, ETAL_DATASERV_TYPE_SLS_XPAD, &pass))
	{
		return OSAL_ERROR;
	}
#endif
#ifdef CONFIG_APP_TEST_DATASERVICES_EPG
	if (OSAL_OK != etalTestDataServices_internal(ETAL_TEST_MODE_DAB, ETAL_DATASERV_TYPE_EPG_RAW, &pass))
	{
		return OSAL_ERROR;
	}
#endif
#ifdef CONFIG_APP_TEST_DATASERVICES_JOURNALINE
	if (OSAL_OK != etalTestDataServices_internal(ETAL_TEST_MODE_DAB, ETAL_DATASERV_TYPE_JML_OBJ, &pass))
	{
		return OSAL_ERROR;
	}
#endif
#ifdef CONFIG_APP_TEST_DATASERVICES_TPEG
	if (OSAL_OK != etalTestDataServices_internal(ETAL_TEST_MODE_DAB, ETAL_DATASERV_TYPE_TPEG_RAW, &pass))
	{
		return OSAL_ERROR;
	}
#endif
#ifdef CONFIG_APP_TEST_DATASERVICES_DLS
	if (OSAL_OK != etalTestDataServices_internal(ETAL_TEST_MODE_DAB, ETAL_DATASERV_TYPE_DLS, &pass))
	{
		return OSAL_ERROR;
	}
#endif
#ifdef CONFIG_APP_TEST_DATASERVICES_DLPLUS
	if (OSAL_OK != etalTestDataServices_internal(ETAL_TEST_MODE_DAB, ETAL_DATASERV_TYPE_DLPLUS, &pass))
	{
		return OSAL_ERROR;
	}
#endif
#endif

#ifdef CONFIG_APP_TEST_FM
#ifdef CONFIG_APP_TEST_DATASERVICES_PSD
	if (OSAL_OK != etalTestDataServices_PSD(ETAL_TEST_MODE_FM, &pass))
	{
		return OSAL_ERROR;
	}
#endif
#endif

#ifdef CONFIG_APP_TEST_AM
#ifdef CONFIG_APP_TEST_DATASERVICES_PSD
	if (OSAL_OK != etalTestDataServices_PSD(ETAL_TEST_MODE_AM, &pass))
	{
		return OSAL_ERROR;
	}
#endif
#endif

#ifdef CONFIG_APP_TEST_HDRADIO_FM
#ifdef CONFIG_APP_TEST_DATASERVICES_PSD
	if (OSAL_OK != etalTestDataServices_PSD(ETAL_TEST_MODE_HD_FM, &pass))
	{
		return OSAL_ERROR;
	}
#endif
#endif

#ifdef CONFIG_APP_TEST_HDRADIO_AM
#ifdef CONFIG_APP_TEST_DATASERVICES_PSD
	if (OSAL_OK != etalTestDataServices_PSD(ETAL_TEST_MODE_HD_AM, &pass))
	{
		return OSAL_ERROR;
	}
#endif
#endif //#ifdef CONFIG_APP_TEST_DATASERVICES_PSD

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_ADVANCED_TUNING
	return OSAL_OK;
}

#endif // CONFIG_APP_ETAL_TEST

