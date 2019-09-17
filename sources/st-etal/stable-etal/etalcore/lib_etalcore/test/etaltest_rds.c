//!
//!  \file 		etaltest_rds.c
//!  \brief 	<i><b> ETAL test, RDS readout </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#if defined (CONFIG_ETAL_HAVE_ETALTML)
#include "etaltml_api.h"
#endif
#include "etaltest.h"

#if defined (CONFIG_APP_TEST_GETRDS) || defined (CONFIG_APP_TEST_GETRAWRDS_DATAPATH) || defined(CONFIG_APP_TEST_FM_VPA)

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
#define ETAL_TEST_GETRDS_HD_DURATION    (6 * ETAL_TEST_ONE_SECOND)
#define ETAL_TEST_SEEK_FM_STEP_FREQ          100

/*****************************************************************
| variable defintion (scope: module-local)
|----------------------------------------------------------------*/
#if defined (CONFIG_APP_TEST_GETRDS_DATAPATH)
tU32 etalTestRDSSeekFrequencyFound;
OSAL_tSemHandle etalTestRDSSeekFinishSem = 0;
#endif
#if defined (CONFIG_APP_TEST_GETRAWRDS_DATAPATH) || defined(CONFIG_APP_TEST_FM_VPA)
 tU32 RdsRawCbInvocations;
#if defined(CONFIG_APP_TEST_FM_VPA)
 tU32 RdsRawCbInvocations2;
#endif // defined(CONFIG_APP_TEST_FM_VPA)
#endif

#endif // defined (CONFIG_APP_TEST_GETRDS) || defined (CONFIG_APP_TEST_GETRAWRDS_DATAPATH)

/* variables for RDS tests */
tU32 RdsDecodedCbInvocations, RdsDecodedCbInvocations2;

#if defined (CONFIG_ETAL_HAVE_ETALTML)
static tBool etalTestGetRDSIsValidCharacters(tChar *text, tU32 size);

EtalRDSData rdsDecodedDatapathData, rdsDecodedDatapathData2;

/*
 * RDS data captured from the air, used to validate
 * data received during CONFIG_APP_TEST_GETRDS.
 * The first time the test is run in some geographical location
 * the array must be hand-filled.
 */
EtalRDSData etalTestRDSReference =
{
#ifdef CONFIG_APP_TEST_IN_LE_MANS
#if 0
	0x00,   // m_validityBitmap
	" RIRE & ",  // m_PS
	0xF2,   // m_DI
	0xf226, // m_PI
	0x00,   // m_PTY
	0x01,   // m_TP
	0x00,   // m_TA
	0x01,   // m_MS
	0x00,   // m_timeHour
	0x00,   // m_timeMinutes
	0x00,   // m_offset
	0x00,   // m_MJD
	"",     // m_RT
	5,      // m_AFListLen
	0xf226, // m_AFListPI
	{0x4f, 0x03, 0x57, 0x50, 0x7c}, // m_AFList

	0x00,   // m_validityBitmap
	" VIRGIN ",  // m_PS
	0xF1,   // m_DI
	0xf219, // m_PI
	0x00,   // m_PTY
	0x01,   // m_TP
	0x00,   // m_TA
	0x01,   // m_MS
	0x00,   // m_timeHour
	0x00,   // m_timeMinutes
	0x00,   // m_offset
	0x00,   // m_MJD
	"",     // m_RT
	17,      // m_AFListLen
	0xf226, // m_AFListPI
	{0xA0, 0x0B, 0x0A, 0x17, 0x0F, 0x38, 0x25, 0x56, 0x49, 0x61, 0x5D, 0x86, 0x79, 0x91, 0x8E, 0xA2, 0x9E}, // m_AFList

	0x00,   // m_validityBitmap
	"EUROPE 1",  // m_PS
	0xF0,   // m_DI
	0xf213, // m_PI
	0x00,   // m_PTY
	0x01,   // m_TP
	0x00,   // m_TA
	0x00,   // m_MS
	0x00,   // m_timeHour
	0x00,   // m_timeMinutes
	0x00,   // m_offset
	0x00,   // m_MJD
	"",     // m_RT
	12,      // m_AFListLen
	0xf213, // m_AFListPI
	{0xAC, 0x1C, 0x14, 0x71, 0x3A, 0x96, 0x73, 0xAB, 0xAA, 0xBF, 0xBE, 0xC3}, // m_AFList
#endif  // 0
	0x00,   // m_validityBitmap
	"   FUN",  // m_PS
	0xF2,   // m_DI
	0xf217, // m_PI
	0x00,   // m_PTY
	0x01,   // m_TP
	0x00,   // m_TA
	0x01,   // m_MS
	0x00,   // m_timeHour
	0x00,   // m_timeMinutes
	0x00,   // m_offset
	0x00,   // m_MJD
	"",     // m_RT
	17,      // m_AFListLen
	0xf217, // m_AFListPI
	{0x80, 0x30, 0x01, 0x46, 0x3C, 0x64, 0x58, 0x90, 0x83, 0x99, 0x98, 0xAE, 0x9F, 0xBC, 0xB9, 0xC7, 0xC3} // m_AFList
#else   // Italy
	0x00,   // m_validityBitmap
	"MARCONI", // m_PS
	0xF0,   // m_DI TODO update for "MARCONI"
	0x5275, // m_PI
	0x09,   // m_PTY
	0x01,   // m_TP TODO update for "MARCONI"
	0x00,   // m_TA
	0x01,   // m_MS TODO update for "MARCONI"
	0x00,   // m_timeHour
	0x00,   // m_timeMinutes
	0x00,   // m_offset
	0x00,   // m_MJD
	"",     // m_RT
	25,     // m_AFListLen TODO update for "MARCONI"
	0x5245, // m_AFListPI TODO update for "MARCONI"
	{0xad, 0xac, 0xae, 0xaa, 0xab, 0xc1, 0xaf, 0x69, 0xc3, 0x1f, 0x4e, 0x9b, 0x8a, 0x20, 0x8f, 0x34, 0x86, 0x4f, 0x2c, 0xa3, 0x21, 0x3b, 0xa2, 0xbf, 0x80} // m_AFList
#endif
};

EtalRDSData etalTestRDSReference2 =
{
#ifdef CONFIG_APP_TEST_IN_LE_MANS
#if 0
#endif  // 0
	0x00,   // m_validityBitmap
	" RIRE & ",  // m_PS
	0xF2,   // m_DI
	0xf226, // m_PI
	0x00,   // m_PTY
	0x01,   // m_TP
	0x00,   // m_TA
	0x01,   // m_MS
	0x00,   // m_timeHour
	0x00,   // m_timeMinutes
	0x00,   // m_offset
	0x00,   // m_MJD
	"",     // m_RT
	5,      // m_AFListLen
	0xf226, // m_AFListPI
	{0x4f, 0x03, 0x57, 0x50, 0x7c} // m_AFList
#if 0
	0x00,   // m_validityBitmap
	" VIRGIN ",  // m_PS
	0xF1,   // m_DI
	0xf219, // m_PI
	0x00,   // m_PTY
	0x01,   // m_TP
	0x00,   // m_TA
	0x01,   // m_MS
	0x00,   // m_timeHour
	0x00,   // m_timeMinutes
	0x00,   // m_offset
	0x00,   // m_MJD
	"",     // m_RT
	17,      // m_AFListLen
	0xf226, // m_AFListPI
	{0xA0, 0x0B, 0x0A, 0x17, 0x0F, 0x38, 0x25, 0x56, 0x49, 0x61, 0x5D, 0x86, 0x79, 0x91, 0x8E, 0xA2, 0x9E} // m_AFList

	0x00,   // m_validityBitmap
	"EUROPE 1",  // m_PS
	0xF0,   // m_DI
	0xf213, // m_PI
	0x00,   // m_PTY
	0x01,   // m_TP
	0x00,   // m_TA
	0x00,   // m_MS
	0x00,   // m_timeHour
	0x00,   // m_timeMinutes
	0x00,   // m_offset
	0x00,   // m_MJD
	"",     // m_RT
	12,      // m_AFListLen
	0xf213, // m_AFListPI
	{0xAC, 0x1C, 0x14, 0x71, 0x3A, 0x96, 0x73, 0xAB, 0xAA, 0xBF, 0xBE, 0xC3} // m_AFList

	0x00,   // m_validityBitmap
	"   FUN",  // m_PS
	0xF2,   // m_DI
	0xf217, // m_PI
	0x00,   // m_PTY
	0x01,   // m_TP
	0x00,   // m_TA
	0x01,   // m_MS
	0x00,   // m_timeHour
	0x00,   // m_timeMinutes
	0x00,   // m_offset
	0x00,   // m_MJD
	"",     // m_RT
	17,      // m_AFListLen
	0xf217, // m_AFListPI
	{0x80, 0x30, 0x01, 0x46, 0x3C, 0x64, 0x58, 0x90, 0x83, 0x99, 0x98, 0xAE, 0x9F, 0xBC, 0xB9, 0xC7, 0xC3} // m_AFList
#endif // 0
#else   // Italy
	0x00,   // m_validityBitmap
	"* RDS *", // m_PS
	0xF1,   // m_DI
	0x5264, // m_PI
	0x09,   // m_PTY
	0x01,   // m_TP
	0x00,   // m_TA
	0x01,   // m_MS
	0x00,   // m_timeHour
	0x00,   // m_timeMinutes
	0x00,   // m_offset
	0x00,   // m_MJD
	"",     // m_RT
	25,     // m_AFListLen
	0x5264, // m_AFListPI
	{0xc6, 0x26, 0x01, 0x3b, 0x38, 0x41, 0x3d, 0x46, 0x45, 0x50, 0x4f, 0x57, 0x54, 0x5a, 0x59, 0x6a, 0x61, 0x72, 0x70, 0xa8, 0x7f, 0xc5, 0xa9, 0xc8, 0xc7} // m_AFList
#endif
};
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML)

#if defined (CONFIG_APP_TEST_GETRDS) || defined (CONFIG_APP_TEST_GETRAWRDS_DATAPATH) || defined(CONFIG_APP_TEST_FM_VPA)

EtalRDSRaw etalTestRDSRawReference = 
{
    0x00,
    {
        {((STAR_RNR_DATARDY | STAR_RNR_SYNC | STAR_RNR_BNE) >> 16), 0x00, 0x01},
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00 }
    }
};
EtalRDSRaw rdsRawDatapathData;
#if defined(CONFIG_APP_TEST_FM_VPA)
EtalRDSRaw rdsRawDatapathData2;
#endif // defined(CONFIG_APP_TEST_FM_VPA)
#endif // defined (CONFIG_APP_TEST_GETRDS) || defined (CONFIG_APP_TEST_GETRAWRDS_DATAPATH) || defined(CONFIG_APP_TEST_FM_VPA)

#ifdef CONFIG_APP_TEST_GETRAWRDS_DATAPATH
typedef enum {
	GET_RAW_RDS_START_DATAPATH = 1,
	GET_RAW_RDS_STOP_DATAPATH,
	GET_RAW_RDS_VERIFY_CB_INVOC,
	GET_RAW_RDS_VERIFY_DATA,
	GET_RAW_RDS_DESTROY_DATAPATH,
	GET_RAW_RDS_START_RBDS_DATAPATH,
	GET_RAW_RDS_STOP_RBDS_DATAPATH,
	GET_RAW_RDS_VERIFY_RBDS_CB_INVOC,
	GET_RAW_RDS_VERIFY_RBDS_DATA,
	GET_RAW_RDS_DESTROY_RBDS_DATAPATH,
	GET_RAW_RDS_CONFIGURE_RAW_DATAPATH_FAST_PI,
	GET_RAW_RDS_START_RAW_FAST_PI,
	GET_RAW_RDS_VERIFY_RAW_INVOC_FAST_PI,
	GET_RAW_RDS_CHECK_RDS_VAL_FAST_PI,
	GET_RAW_RDS_CHECK_RDS_VAL_NORMAL,
	GET_RAW_RDS_FORCE_FAST_PI,
	GET_RAW_RDS_START_NORMAL
} GetRAWRDSTestTy;
#endif

#if defined (CONFIG_ETAL_HAVE_ETALTML)
/***************************
 *
 * etalTestGetRDSAccumulate
 *
 **************************/
tVoid etalTestGetRDSAccumulate(EtalRDSData *dst, EtalRDSData *src)
{
	if (src->m_validityBitmap == 0)
	{
		return;
	}

	if ((src->m_validityBitmap & ETAL_DECODED_RDS_VALID_PI) == ETAL_DECODED_RDS_VALID_PI)
	{
		dst->m_validityBitmap |= ETAL_DECODED_RDS_VALID_PI;
		dst->m_PI = src->m_PI;
	}

	if ((src->m_validityBitmap & ETAL_DECODED_RDS_VALID_DI) == ETAL_DECODED_RDS_VALID_DI)
	{
		dst->m_validityBitmap |= ETAL_DECODED_RDS_VALID_DI;
		dst->m_DI = src->m_DI;
	}

	if ((src->m_validityBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
	{
		dst->m_validityBitmap |= ETAL_DECODED_RDS_VALID_PS;
		OSAL_pvMemoryCopy(dst->m_PS, src->m_PS, sizeof(tChar) * ETAL_DEF_MAX_PS_LEN);
	}

	if ((src->m_validityBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
	{
		dst->m_validityBitmap |= ETAL_DECODED_RDS_VALID_RT;
		OSAL_pvMemoryCopy(dst->m_RT, src->m_RT, sizeof(tChar) * ETAL_DEF_MAX_RT_LEN);
	}

	if ((src->m_validityBitmap & ETAL_DECODED_RDS_VALID_TOM) == ETAL_DECODED_RDS_VALID_TOM)
	{
		dst->m_validityBitmap |= ETAL_DECODED_RDS_VALID_TOM;
		dst->m_timeHour = src->m_timeHour;
		dst->m_timeMinutes = src->m_timeMinutes;
		dst->m_offset = src->m_offset;
		dst->m_MJD = src->m_MJD;
	}

	if ((src->m_validityBitmap & ETAL_DECODED_RDS_VALID_AF) == ETAL_DECODED_RDS_VALID_AF)
	{
		dst->m_validityBitmap |= ETAL_DECODED_RDS_VALID_AF;
		dst->m_AFListPI = src->m_AFListPI;
		dst->m_AFListLen = src->m_AFListLen;
		OSAL_pvMemoryCopy(dst->m_AFList, src->m_AFList, sizeof(tU8) * src->m_AFListLen);
	}
	
	if ((src->m_validityBitmap & ETAL_DECODED_RDS_VALID_PTY) == ETAL_DECODED_RDS_VALID_PTY)
	{
		dst->m_validityBitmap |= ETAL_DECODED_RDS_VALID_PTY;
		dst->m_PTY = src->m_PTY;
	}

	if ((src->m_validityBitmap & ETAL_DECODED_RDS_VALID_TP) == ETAL_DECODED_RDS_VALID_TP)
	{
		dst->m_validityBitmap |= ETAL_DECODED_RDS_VALID_TP;
		dst->m_TP = src->m_TP;
	}

	if ((src->m_validityBitmap & ETAL_DECODED_RDS_VALID_TA) == ETAL_DECODED_RDS_VALID_TA)
	{
		dst->m_validityBitmap |= ETAL_DECODED_RDS_VALID_TA;
		dst->m_TA = src->m_TA;
	}

	if ((src->m_validityBitmap & ETAL_DECODED_RDS_VALID_MS) == ETAL_DECODED_RDS_VALID_MS)
	{
		dst->m_validityBitmap |= ETAL_DECODED_RDS_VALID_MS;
		dst->m_MS = src->m_MS;
	}
}

/***************************
 *
 * etalTestGetRDSCompare
 *
 **************************/
/*
 * Compare two RDS structs.
 * Since the data actually captured depends on the broadcaster and there isn't an
 * easy way to decide when the data is complete, the function only compares
 * the fields that have been received (using the bitmap fields)
 */
tBool etalTestGetRDSCompare(EtalRDSData *data, EtalRDSData *reference)
{
	tU32 i, j;
	tBool af_found;

	if (data->m_validityBitmap == 0)
	{
		etalTestPrintNormal("No RDS data to compare");
		return FALSE;
	}

	if ((data->m_validityBitmap & ETAL_DECODED_RDS_VALID_PS) == ETAL_DECODED_RDS_VALID_PS)
	{
#ifndef CONFIG_APP_TEST_IN_LE_MANS
		if (OSAL_s32MemoryCompare(data->m_PS, reference->m_PS, sizeof(data->m_PS) != 0))
		{
			etalTestPrintNormal("PS RDS mismatch");
			return FALSE;
		}
#endif
		if (etalTestGetRDSIsValidCharacters(data->m_PS, sizeof(data->m_PS)) == FALSE)
		{
			etalTestPrintNormal("PS RDS invalid characters");
			return FALSE;
		}
	}
	if ((data->m_validityBitmap & ETAL_DECODED_RDS_VALID_DI) == ETAL_DECODED_RDS_VALID_DI)
	{
		if (data->m_DI != reference->m_DI)
		{
			etalTestPrintNormal("DI RDS mismatch");
			return FALSE;
		}
	}
	if ((data->m_validityBitmap & ETAL_DECODED_RDS_VALID_PI) == ETAL_DECODED_RDS_VALID_PI)
	{
		if (data->m_PI != reference->m_PI)
		{
			etalTestPrintNormal("PI RDS mismatch");
			return FALSE;
		}
	}
	if ((data->m_validityBitmap & ETAL_DECODED_RDS_VALID_RT) == ETAL_DECODED_RDS_VALID_RT)
	{
		if ((OSAL_s32MemoryCompare("", reference->m_RT, 1) != 0) && (OSAL_s32MemoryCompare(data->m_RT, reference->m_RT, ETAL_DEF_MAX_RT_LEN) != 0))
		{
			etalTestPrintNormal("RT RDS mismatch");
			return FALSE;
		}
		if (etalTestGetRDSIsValidCharacters(data->m_RT, sizeof(data->m_RT)) == FALSE)
		{
			etalTestPrintNormal("RT RDS invalid characters");
			return FALSE;
		}
	}
#if 0 // not comparable, if present always changes
	if ((data->m_validityBitmap & ETAL_DECODED_RDS_VALID_TOM) == ETAL_DECODED_RDS_VALID_TOM)
	{
		if (data->m_timeHour != reference->m_timeHour)
		{
			etalTestPrintNormal("timeHour RDS mismatch");
			return FALSE;
		}
		if (data->m_timeMinutes != reference->m_timeMinutes)
		{
			etalTestPrintNormal("timeMinute RDS mismatch");
			return FALSE;
		}
		if (data->m_offset != reference->m_offset)
		{
			etalTestPrintNormal("offset RDS mismatch");
			return FALSE;
		}
		if (data->m_MJD != reference->m_MJD)
		{
			etalTestPrintNormal("MJD RDS mismatch");
			return FALSE;
		}
	}
#endif
	if ((data->m_validityBitmap & ETAL_DECODED_RDS_VALID_AF) == ETAL_DECODED_RDS_VALID_AF)
	{
		if (data->m_AFListLen != reference->m_AFListLen)
		{
			etalTestPrintNormal("AFListLen RDS mismatch");
			return FALSE;
		}
		if (data->m_AFListPI != reference->m_AFListPI)
		{
			etalTestPrintNormal("AFListPI RDS mismatch");
			return FALSE;
		}
		/* Compare AF List */
		for(i = 0; i < data->m_AFListLen; i++)
		{
			af_found = FALSE;
			for(j = 0; j < reference->m_AFListLen; j++)
			{
				if (data->m_AFList[i] == reference->m_AFList[j])
				{
					af_found = TRUE;
					break;
				}
			}
			if (af_found == FALSE)
			{
				etalTestPrintNormal("AFList RDS mismatch, missing %02x = %6d MHz", data->m_AFList[i], 87500 + 100 * data->m_AFList[i]);
				return FALSE;
			}
		}
	}
	else
	{
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
		if (reference->m_AFListLen != 0)
		{
			etalTestPrintNormal("AFList not received");
			return FALSE;
		}
#endif // CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
	}

	if ((data->m_validityBitmap & ETAL_DECODED_RDS_VALID_PTY) == ETAL_DECODED_RDS_VALID_PTY)
	{
		if (data->m_PTY != reference->m_PTY)
		{
			etalTestPrintNormal("PTY RDS mismatch");
			return FALSE;
		}
	}
	if ((data->m_validityBitmap & ETAL_DECODED_RDS_VALID_TP) == ETAL_DECODED_RDS_VALID_TP)
	{
		if (data->m_TP != reference->m_TP)
		{
			etalTestPrintNormal("TP RDS mismatch");
			return FALSE;
		}
	}
	if ((data->m_validityBitmap & ETAL_DECODED_RDS_VALID_TA) == ETAL_DECODED_RDS_VALID_TA)
	{
		if (data->m_TA != reference->m_TA)
		{
			etalTestPrintNormal("TA RDS mismatch");
			return FALSE;
		}
	}
	if ((data->m_validityBitmap & ETAL_DECODED_RDS_VALID_MS) == ETAL_DECODED_RDS_VALID_MS)
	{
		if (data->m_MS != reference->m_MS)
		{
			etalTestPrintNormal("MS RDS mismatch");
			return FALSE;
		}
	}
	return TRUE;
}

/***************************
 *
 * etalTestGetRDSIsValidCharacters
 *
 **************************/
/*
 * Chech RDS characters are valid from IEC 62106:1999 Annex E definition.
 * return TRUE if characters are valid and FALSE if characters are invalid
 */
static tBool etalTestGetRDSIsValidCharacters(tChar *text, tU32 size)
{
	tU32 i;
	tBool ret = TRUE;

	/* check empty text */
	if (text[0] == 0)
	{
		ret = FALSE;
		printf("RDS empty\n");
	}
	for(i = 0; i < size; i++)
	{
		/* check valid control characters 0/15 0/14, 1/11, 6/14 or valid code-table characters E.1, E.2 and E.3 */
		if ((text[i] != 0) && (text[i] != 0x0F) && (text[i] != 0x0E) &&
			(text[i] != 0x1B) && (text[i] != 0x6E) &&
			((text[i] < 0x20) || ((tUChar)text[i] > 0xFE)))
		{
			ret = FALSE;
			printf("RDS invalid 1 (0x%x, position %d)\n", text[i], i);
			printf("RDS string: \"%s\"\n", text);
		}
		/* check valid sequence of control characters 0/15 0/15, 0/14 0/14, 1/11 6/14 */
		if (((text[i] == 0x0F) && (text[i-1] != 0x0F) && (text[i+1] != 0x0F)) ||
			((text[i] == 0x0E) && (text[i-1] != 0x0E) && (text[i+1] != 0x0E)) ||
			((text[i] == 0x1B) && (text[i] != 0x6E)))
		{
			ret = FALSE;
			printf("RDS invalid 2 (0x%x, position %d)\n", text[i], i);
			printf("RDS string: \"%s\"\n", text);
		}
	}
	return ret;
}
#endif //#if defined (CONFIG_ETAL_HAVE_ETALTML)

/***************************
 *
 * etalTestGetParameterCMOST
 *
 **************************/
/*
 * Copy of the ETAL_getParameter_CMOST to avoid inclusion of etalinternal.h
 */
tVoid etalTestGetParameterCMOST(tU8 *cmd, tU32 *p)
{
	tU32 i = 0;

	*p  = ((tU32)cmd[i++] << 16) & 0x00FF0000;
	*p |= ((tU32)cmd[i++] <<  8) & 0x0000FF00;
	*p |= ((tU32)cmd[i  ] <<  0) & 0x000000FF;
}

#if defined (CONFIG_APP_TEST_GETRDS) || defined (CONFIG_APP_TEST_GETRAWRDS_DATAPATH) || defined(CONFIG_APP_TEST_FM_VPA)
#if defined (CONFIG_APP_TEST_GETRDS_DATAPATH)
/***************************
 *
 * etalTestRDSCallback
 *
 **************************/
static void etalTestRDSCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	EtalRDSData *prdsfm;
	tChar ts[2][4] = {"FG ", "BG "};

	prdsfm = (EtalRDSData *) pBuffer;
	if (pvContext == NULL)
	{
		etalTestPrintRDS(prdsfm, ts[0]);
#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_HDRADIO_FM)
		etalTestGetRDSAccumulate(&rdsDecodedDatapathData, prdsfm);
#endif
		RdsDecodedCbInvocations++;
	}
	else
	{
		etalTestPrintRDS(prdsfm, ts[1]);
#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_HDRADIO_FM)
		etalTestGetRDSAccumulate(&rdsDecodedDatapathData2, prdsfm);
#endif
		RdsDecodedCbInvocations2++;
	}
}
#endif // CONFIG_APP_TEST_GETRDS_DATAPATH

#if defined(CONFIG_APP_TEST_GETRAWRDS_DATAPATH) || defined(CONFIG_APP_TEST_FM_VPA)
/***************************
 *
 * etalTestGetRDSRawAccumulate
 *
 **************************/
tVoid etalTestGetRDSRawAccumulate(EtalRDSRaw *dst, EtalRDSRawData *src, tU32 len)
{
    dst->len = len;
    OSAL_pvMemoryCopy(dst->raw_data.m_RNR, src, len);
}

/***************************
 *
 * etalTestGetRDSRawCompare
 *
 **************************/
tSInt etalTestGetRDSRawCompare(EtalRDSRaw *data, EtalRDSRaw *reference, tBool fastPiMode)
{
    tU32 data_rnr, ref_rnr, rds_data, i;

    if (data->len >= 6)
    {
        etalTestGetParameterCMOST(data->raw_data.m_RNR, &data_rnr);
        etalTestGetParameterCMOST(reference->raw_data.m_RNR, &ref_rnr);
        if (fastPiMode == FALSE)
        {
            /* normal mode */
            if ((data_rnr & (STAR_RNR_SYNC | STAR_RNR_BOFL | STAR_RNR_BNE)) != 
                (ref_rnr & (STAR_RNR_SYNC | STAR_RNR_BOFL | STAR_RNR_BNE)))
            {
                etalTestPrintNormal("rnr different: %06x expected %06x", data_rnr & (STAR_RNR_DATARDY | STAR_RNR_SYNC | STAR_RNR_BOFL | STAR_RNR_BNE), ref_rnr & (STAR_RNR_DATARDY | STAR_RNR_SYNC | STAR_RNR_BOFL | STAR_RNR_BNE));
                return OSAL_ERROR;
            }
        }
        else
        {
            /* fast PI mode */
            if ((data_rnr & (STAR_RNR_BOFL | STAR_RNR_BNE)) != 
                (ref_rnr & (STAR_RNR_BOFL | STAR_RNR_BNE)))
            {
                etalTestPrintNormal("rnr different: %06x expected %06x", data_rnr & (STAR_RNR_BOFL | STAR_RNR_BNE), ref_rnr & (STAR_RNR_BOFL | STAR_RNR_BNE));
                return OSAL_ERROR;
            }
            /* check A or C' */
            for(i = 0; i < (data->len - 3); i += 3)
            {
                etalTestGetParameterCMOST(&(reference->raw_data.m_RDS_Data[i]), &rds_data);
                if (((rds_data & (START_RDS_DATA_CTYPE_MASK | START_RDS_DATA_BLOCKID_MASK)) != 0) &&
                    ((rds_data & (START_RDS_DATA_CTYPE_MASK | START_RDS_DATA_BLOCKID_MASK)) != 0x060000))
                {
                    etalTestPrintNormal("rds data different of A C': %06x expected %06x", rds_data & (START_RDS_DATA_CTYPE_MASK | START_RDS_DATA_BLOCKID_MASK), rds_data & (START_RDS_DATA_CTYPE_MASK | START_RDS_DATA_BLOCKID_MASK));
                    return OSAL_ERROR;
                }
            }
        }
    }
    else
    {
        return OSAL_ERROR;
    }
	return OSAL_OK;
}

#if defined(CONFIG_APP_TEST_GETRAWRDS_DATAPATH)
/***************************
 *
 * etalTestRDSRawCallback
 *
 **************************/
static void etalTestRDSRawCallback(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	etalTestGetRDSRawAccumulate(&rdsRawDatapathData, (EtalRDSRawData *)pBuffer, dwActualBufferSize);
	if (pvContext != NULL)
	{
		etalTestPrintRDSRaw(&rdsRawDatapathData, *((ETAL_HANDLE *)pvContext));
	}
	else
	{
		etalTestPrintRDSRaw(&rdsRawDatapathData, 0);
	}
	RdsRawCbInvocations++;
}
#endif // defined(CONFIG_APP_TEST_GETRAWRDS_DATAPATH)
#endif // CONFIG_APP_TEST_GETRAWRDS_DATAPATH || CONFIG_APP_TEST_FM_VPA

#if (defined (CONFIG_APP_TEST_GETRDS) || defined(CONFIG_APP_TEST_GETRDS_DATAPATH) || defined (CONFIG_APP_TEST_GETRAWRDS_DATAPATH)) && (defined(CONFIG_APP_TEST_FM) || defined(CONFIG_APP_TEST_HDRADIO_FM))
/***************************
 *
 * etalTestDoRDSConfig
 *
 **************************/
/*
 * Creates a valid RDS configuration 
 *
 * Returns:
 *  OSAL_OK if no error and valid handle is stored in *hReceiver
 *  OSAL_ERROR otherwise
 */
static tSInt etalTestDoRDSConfig(ETAL_HANDLE *hReceiver, tBool reconfigure, etalTestBroadcastTy mode, ETAL_HANDLE hFrontend)
{
	EtalReceiverAttr attr;
	tChar standard[5] = "\0";
	ETAL_HANDLE hFrontend_local;
	EtalBcastStandard std;
	ETAL_STATUS ret;
	EtalAudioInterfTy audioIf;

	if (mode == ETAL_TEST_MODE_FM)
	{
		std = ETAL_BCAST_STD_FM;
		if (hFrontend == ETAL_INVALID_HANDLE)
		{
			hFrontend_local = ETAL_FE_FOR_FM_TEST;
		}
		else
		{
			hFrontend_local = hFrontend;
		}
		OSAL_szStringCopy(standard, "FM");
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		std = ETAL_BCAST_STD_HD_FM;
		if (hFrontend == ETAL_INVALID_HANDLE)
		{
			hFrontend_local = ETAL_FE_FOR_HD_TEST;
		}
		else
		{
			hFrontend_local = hFrontend;
		}
		OSAL_szStringCopy(standard, "HD");
	}
	else 
	{
		return OSAL_ERROR;
	}

	/* Create RDS receiver */

	if (hReceiver != NULL)
	{
		if ((!reconfigure) || (*hReceiver == ETAL_INVALID_HANDLE))
		{
			*hReceiver = ETAL_INVALID_HANDLE;
			etalTestPrintNormal("* Create RDS receiver for %s", standard);
		}
		else
		{
			etalTestPrintNormal("* Reconfiguring RDS receiver for %s", standard);
			if ((ret = etal_destroy_receiver(hReceiver)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_destroy_receiver RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
				return OSAL_ERROR;
			}

		}
		OSAL_pvMemorySet(&attr, 0x00, sizeof(EtalReceiverAttr));
		attr.m_Standard = std;
		attr.m_FrontEnds[0] = hFrontend_local;
		attr.m_FrontEndsSize = 1;
		if ((ret = etal_config_receiver(hReceiver, &attr)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_config_receiver RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}

			// set the correct audio path 
			/* Configure audio path */
			memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
			audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
#ifdef CONFIG_DIGITAL_AUDIO
			if ((mode == ETAL_TEST_MODE_FM) || (mode == ETAL_TEST_MODE_AM) || (mode == ETAL_TEST_MODE_DAB))
			{
				audioIf.m_dac = 0;
			}
			else
			{
				system("amixer -c 3 sset Source adcauxdac > /dev/null" );

				// select the audio channel
				system("amixer -c 3 sset \"ADCAUX CHSEL\" 0 > /dev/null");
			}
#endif
			// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
			audioIf.m_sai_slave_mode = TRUE;
#endif //CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER

			if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
			{
				etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
				return OSAL_ERROR;
			}
			
	}

	return OSAL_OK;
}

/***************************
 *
 * etalTestUndoRDSConfig
 *
 **************************/
static tSInt etalTestUndoRDSConfig(ETAL_HANDLE *hReceiver)
{
	ETAL_STATUS ret;

	if ((hReceiver != NULL) && (*hReceiver != ETAL_INVALID_HANDLE))
	{
		etalTestPrintNormal("* Destroy RDS receiver");
		if ((ret = etal_destroy_receiver(hReceiver)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_destroy_receiver RDS (%s, %d)", ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}

	return OSAL_OK;
}
#endif // defined (CONFIG_APP_TEST_GETRDS) || defined (CONFIG_APP_TEST_GETRAWRDS_DATAPATH)

#if (defined(CONFIG_APP_TEST_GETRDS_DATAPATH)) && (defined(CONFIG_APP_TEST_FM) || defined(CONFIG_APP_TEST_HDRADIO_FM))
/***************************
 *
 * etalTestGetRDSDatapath
 *
 **************************/
static tSInt etalTestGetRDSDatapath(ETAL_HANDLE hReceiver, etalTestBroadcastTy mode, tBool *pass)
{
	EtalDataPathAttr dataPathAttr;
	tChar standard[16] = "\0";
	tU32 duration;
	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret;
    EtalAudioInterfTy audioIf;

	if (mode == ETAL_TEST_MODE_FM)
	{
		duration = ETAL_TEST_GETRDS_FM_DURATION;
		OSAL_szStringCopy(standard, "FM");
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		duration = ETAL_TEST_GETRDS_HD_DURATION;
		OSAL_szStringCopy(standard, "HD");
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		duration = ETAL_TEST_GETRDS_FM_DURATION;
		OSAL_szStringCopy(standard, "HD from FM freq");
	}
	else
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("");
	etalTestPrintNormal("GetRDS from datapath for %s", standard);
	/*
	 * reconfigure the receiver so to force ETAL to reset RDS
	 * data, otherwise after the pass1 all data is considered
	 * already sent by the RDS decoder and not sent again
	 */
	if (etalTestDoRDSConfig(&hReceiver, TRUE, mode, ETAL_INVALID_HANDLE) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	 * After a reconfiguration the tuned frequency is reset to 0
	 * and this causes problems with the RDS landscape, so a new
	 * tune is required
	 */
	if (mode == ETAL_TEST_MODE_FM)
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_FM, hReceiver) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		etalTestPrintNormal("* Tune to FM freq %d", ETAL_VALID_FM_FREQ);
		if ((ret = etal_tune_receiver(hReceiver, ETAL_VALID_FM_FREQ)) != ETAL_RET_NO_DATA)
		{
			etalTestPrintError("etal_tune_receiver %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
        /* Configure audio path */
        memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
        audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
		// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = true;
#endif

				// Audio path should be correctly set before
#if 0

        if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
        {
            etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
            return OSAL_ERROR;
        }
#endif

        /* Select audio source */
		if ((ret = etal_audio_select(hReceiver, ETAL_AUDIO_SOURCE_AUTO_HD)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_audio_select %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}
	else
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, hReceiver) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

	etalTestPrintNormal("* Config RDS datapath for %s", standard);
	dataPathAttr.m_receiverHandle = hReceiver;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS;
	dataPathAttr.m_sink.m_context = NULL;
	dataPathAttr.m_sink.m_BufferSize = sizeof(EtalRDSData);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestRDSCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * reset callback counter/data
	 */
	RdsDecodedCbInvocations = 0;
	OSAL_pvMemorySet(&rdsDecodedDatapathData, 0x00, sizeof(EtalRDSData));
	/*
	 * start RDS reception
	 */
	etalTestPrintNormal("* Start RDS reception for %s", standard);
	if ((ret = etal_start_RDS(hReceiver, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_start_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * start RDS decoding
	 */
	etalTestPrintNormal("* Start RDS decoding for %s", standard);
	if ((ret = etaltml_start_decoded_RDS(hReceiver, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_start_decoded_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	OSAL_s32ThreadWait(duration);

	/*
	 * stop RDS decoding
	 */
	etalTestPrintNormal("* Stop RDS decoding for %s", standard);
	if ((ret = etaltml_stop_decoded_RDS(hReceiver, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_stop_decoded_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * stop RDS reception and compare
	 */
	if ((ret = etal_stop_RDS(hReceiver)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_stop_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("= complete, %d RDS callback invocations", RdsDecodedCbInvocations);
	etalTestPrintNormal("= (expected more than %d)", ETAL_TEST_GET_RDS_MIN);

	if ((mode == ETAL_TEST_MODE_FM) ||
		(mode == ETAL_TEST_MODE_HD_FM))
	{
		if (RdsDecodedCbInvocations >= ETAL_TEST_GET_RDS_MIN)
		{
			*pass = etalTestGetRDSCompare(&rdsDecodedDatapathData, &etalTestRDSReference);
		}
		else
		{
			*pass = FALSE;
		}
	}
	if ((ret = etal_destroy_datapath(&hDatapath)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

#ifdef CONFIG_APP_TEST_FM
static tVoid etalTestSetRdsThreshold(tBool vI_goodThreshold, EtalSeekThreshold *pO_seekThreshold)
{
	if (TRUE == vI_goodThreshold)
	{
		pO_seekThreshold->SeekThresholdBBFieldStrength = 0x0F;
        pO_seekThreshold->SeekThresholdDetune = 0x1B;
        pO_seekThreshold->SeekThresholdAdjacentChannel = 0x60;
        pO_seekThreshold->SeekThresholdMultipath = 0xC1;
		pO_seekThreshold->SeekThresholdSignalNoiseRatio = 0x3F;
		pO_seekThreshold->SeekThresholdMpxNoise = 0x64;
		pO_seekThreshold->SeekThresholdCoChannel = 0xFF;
	}
	else
	{
		pO_seekThreshold->SeekThresholdBBFieldStrength = 0x80;
        pO_seekThreshold->SeekThresholdDetune = 0xFF;
        pO_seekThreshold->SeekThresholdAdjacentChannel = 0x7F;
        pO_seekThreshold->SeekThresholdMultipath = 0xFF;
		pO_seekThreshold->SeekThresholdSignalNoiseRatio = 0x00;
		pO_seekThreshold->SeekThresholdMpxNoise = 0xFF;
		pO_seekThreshold->SeekThresholdCoChannel = 0xFF;		
	}
	
}

/***************************
 *
 * etalTestGetRDSDatapathChangeFreq
 *
 * Test RDS datapath with frequency change by tune, manual seek, auto seek, af switch
 *
 **************************/
static tSInt etalTestGetRDSDatapathChangeFreq(ETAL_HANDLE *hReceiver, etalTestBroadcastTy mode, tBool *pass)
{
	EtalDataPathAttr dataPathAttr;
	tChar standard[16] = "\0";
	tU32 duration, frequency, step;
	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret;
    EtalAudioInterfTy audioIf;
    EtalSeekThreshold seekThreshold;

	if (mode == ETAL_TEST_MODE_FM)
	{
		duration = ETAL_TEST_GETRDS_FM_DURATION;
		OSAL_szStringCopy(standard, "FM");
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		duration = ETAL_TEST_GETRDS_HD_DURATION;
		OSAL_szStringCopy(standard, "HD");
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		duration = ETAL_TEST_GETRDS_FM_DURATION;
		OSAL_szStringCopy(standard, "HD from FM");
	}
	else
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("");
	etalTestPrintNormal("GetRDS from datapath with change frequency for %s", standard);
	/*
	 * reconfigure the receiver so to force ETAL to reset RDS
	 * data, otherwise after the pass1 all data is considered
	 * already sent by the RDS decoder and not sent again
	 */
	if (etalTestDoRDSConfig(hReceiver, TRUE, mode, ETAL_INVALID_HANDLE) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	 * After a reconfiguration the tuned frequency is reset to 0
	 * and this causes problems with the RDS landscape, so a new
	 * tune is required
	 */
	if (mode == ETAL_TEST_MODE_FM)
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_FM, *hReceiver) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
        frequency = ETAL_VALID_FM_FREQ;
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		etalTestPrintNormal("* Tune to FM freq %d", ETAL_VALID_FM_FREQ);
		if ((ret = etal_tune_receiver(*hReceiver, ETAL_VALID_FM_FREQ)) != ETAL_RET_NO_DATA)
		{
			etalTestPrintError("etal_tune_receiver %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
        frequency = ETAL_VALID_FM_FREQ;
        /* Configure audio path */
        memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
        audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
		// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = true;
#endif

				// Audio path should be correctly set before
#if 0

        if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
        {
            etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
            return OSAL_ERROR;
        }
#endif

        /* Select audio source */
		if ((ret = etal_audio_select(*hReceiver, ETAL_AUDIO_SOURCE_AUTO_HD)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_audio_select %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}
	else
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, *hReceiver) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
        frequency = ETAL_VALID_HD_FREQ;
	}

	etalTestPrintNormal("* Config RDS datapath for %s", standard);
	dataPathAttr.m_receiverHandle = *hReceiver;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS;
	dataPathAttr.m_sink.m_context = NULL;
	dataPathAttr.m_sink.m_BufferSize = sizeof(EtalRDSData);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestRDSCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

    /* loop for tune, af switch, manual seek and auto seek change of frequency */
    for(step = 0; step < 8; step++)
    {
        /*
         * reset callback counter/data
         */
        RdsDecodedCbInvocations = 0;
        OSAL_pvMemorySet(&rdsDecodedDatapathData, 0x00, sizeof(EtalRDSData));
        if (*hReceiver != ETAL_INVALID_HANDLE)
        {
            switch (step)
            {
                case 0:
                /*
                 * start RDS reception
                 */
                etalTestPrintNormal("* Start RDS reception for %s", standard);
                if ((ret = etal_start_RDS(*hReceiver, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
                {
                    etalTestPrintError("etal_start_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                    return OSAL_ERROR;
                }
                /*
                 * start RDS decoding
                 */
                etalTestPrintNormal("* Start RDS decoding for %s", standard);
                if ((ret = etaltml_start_decoded_RDS(*hReceiver, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
                {
                    etalTestPrintError("etaltml_start_decoded_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                    return OSAL_ERROR;
                }
                    break;
                case 1:
                    /*
                     * tune foreground to other frequency
                     */
                    etalTestPrintNormal("* Tune to %s freq %d", standard, frequency);
                    if ((ret = etal_tune_receiver(*hReceiver, frequency)) != ((mode == ETAL_TEST_MODE_HD_FM)?ETAL_RET_NO_DATA:ETAL_RET_SUCCESS))
                    {
                        etalTestPrintError("etal_tune_receiver %s freq (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                case 2:
                case 3:
                    /*
                     * af_switch foreground to other frequency
                     */
                    etalTestPrintNormal("* AF switch to %s freq %d", standard, frequency);
                    if ((ret = etal_AF_switch(*hReceiver, frequency)) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_AF_switch %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                case 4:
                case 5:
                    /*
                     * manual seek foreground to other frequency
                     */
                    etalTestPrintNormal("* Tune to %s freq %d", standard, frequency - ETAL_TEST_SEEK_FM_STEP_FREQ);
                    if ((ret = etal_tune_receiver(*hReceiver, frequency - ETAL_TEST_SEEK_FM_STEP_FREQ)) != ((mode == ETAL_TEST_MODE_HD_FM)?ETAL_RET_NO_DATA:ETAL_RET_SUCCESS))
                    {
                        etalTestPrintError("etal_tune_receiver %s freq (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    etalTestPrintNormal("* manual seek start to %s freq %d", standard, frequency);
                    if (((ret = etal_seek_start_manual(*hReceiver, cmdDirectionUp, ETAL_TEST_SEEK_FM_STEP_FREQ, &etalTestRDSSeekFrequencyFound)) != ETAL_RET_SUCCESS) || (etalTestRDSSeekFrequencyFound != frequency))
                    {
                        etalTestPrintError("etal_seek_start_manual %s freq %d (%s, %d)", standard, etalTestRDSSeekFrequencyFound, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    etalTestPrintNormal("* manual seek stop to %s freq %d", standard, etalTestRDSSeekFrequencyFound);
                    if (((ret = etal_seek_stop_manual(*hReceiver, cmdAudioUnmuted, &etalTestRDSSeekFrequencyFound)) != ETAL_RET_SUCCESS) || (etalTestRDSSeekFrequencyFound != frequency))
                    {
                        etalTestPrintError("etal_seek_stop_manual %s freq %d (%s, %d)", standard, etalTestRDSSeekFrequencyFound, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                case 6:
                case 7:
                    /*
                     * auto seek foreground to other frequency
                     */
                    etalTestRDSSeekFrequencyFound = ETAL_INVALID_FREQUENCY;
                    if ((etalTestRDSSeekFinishSem == 0) && 
                        (OSAL_s32SemaphoreCreate("Sem_etalTestRDSSeekFinish", &etalTestRDSSeekFinishSem, 0) == OSAL_ERROR))
                    {
                        return OSAL_ERROR;
                    }
                    etalTestPrintNormal("* Tune to %s freq %d", standard, frequency - ETAL_TEST_SEEK_FM_STEP_FREQ);
                    if ((ret = etal_tune_receiver(*hReceiver, frequency - ETAL_TEST_SEEK_FM_STEP_FREQ)) != ((mode == ETAL_TEST_MODE_HD_FM)?ETAL_RET_NO_DATA:ETAL_RET_SUCCESS))
                    {
                        etalTestPrintError("etal_tune_receiver %s freq (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }

					etalTestSetRdsThreshold(TRUE, &seekThreshold);

                    if (etal_set_autoseek_thresholds_value(*hReceiver, &seekThreshold) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_set_autoseek_thresholds_value %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }

                    etalTestPrintNormal("* auto seek start to %s freq %d", standard, frequency);
                    if ((ret = etal_autoseek_start(*hReceiver, cmdDirectionUp, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_autoseek_start %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    /* wait max 2 seconds for ETAL_SEEK_FINISHED event */
                    OSAL_s32SemaphoreWait(etalTestRDSSeekFinishSem, 2000);
                    if (etalTestRDSSeekFrequencyFound != frequency)
                    {
                        etalTestPrintError("etal_autoseek_start %s freq not found %d (%s, %d)", standard, etalTestRDSSeekFrequencyFound, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    etalTestPrintNormal("* auto seek stop to %s freq %d", standard, frequency);
                    if ((ret = etal_autoseek_stop(*hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_autoseek_stop %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                default:
                    break;
            }
        }

        OSAL_s32ThreadWait(duration);

        /*
         * check RDS callback invocations and compare
         */
        etalTestPrintNormal("= complete, %d RDS callback invocations", RdsDecodedCbInvocations);
        etalTestPrintNormal("= (expected more than %d)", ETAL_TEST_GET_RDS_MIN);

        if ((mode == ETAL_TEST_MODE_FM) || (mode == ETAL_TEST_MODE_HD_FM))
        {
            if (RdsDecodedCbInvocations >= ETAL_TEST_GET_RDS_MIN)
            {
                if (frequency == ETAL_VALID_FM_FREQ)
                {
                    *pass = etalTestGetRDSCompare(&rdsDecodedDatapathData, &etalTestRDSReference);
                }
                else
                {
                    *pass = etalTestGetRDSCompare(&rdsDecodedDatapathData, &etalTestRDSReference2);
                }
            }
            else
            {
                *pass = FALSE;
            }
            if (*pass == FALSE)
            {
                break;
            }
        }

        /*
         * change foreground to other frequency
         */
        if ((mode == ETAL_TEST_MODE_FM) || (mode == ETAL_TEST_MODE_HD_FM)) 
        {
            if (frequency == ETAL_VALID_FM_FREQ)
            {
                frequency = ETAL_VALID_FM_FREQ2;
            }
            else
            {
                frequency = ETAL_VALID_FM_FREQ;
            }
        }
        else
        {
            if (frequency == ETAL_VALID_HD_FREQ)
            {
                frequency = ETAL_VALID_HD_FREQ2;
            }
            else
            {
                frequency = ETAL_VALID_HD_FREQ;
            }
        }
    }

	/*
	 * stop RDS decoding
	 */
	etalTestPrintNormal("* Stop RDS decoding for %s", standard);
	if ((ret = etaltml_stop_decoded_RDS(*hReceiver, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_stop_decoded_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * stop RDS reception and datapath
	 */
	if ((ret = etal_stop_RDS(*hReceiver)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_stop_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	if ((ret = etal_destroy_datapath(&hDatapath)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/*
	 * Undo tune
	 */
    if ((mode == ETAL_TEST_MODE_FM) || (mode == ETAL_TEST_MODE_HD_FM))
    {
        if (etalTestUndoTuneSingle(ETAL_TUNE_FM, *hReceiver) != OSAL_OK)
        {
            return OSAL_ERROR;
        }
    }
    else
    {
        if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_FM, *hReceiver) != OSAL_OK)
        {
            return OSAL_ERROR;
        }
    }

    if (etalTestUndoRDSConfig(hReceiver) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

	return OSAL_OK;
}

/***************************
 *
 * etalTestGetRDSDatapathChangeFreqNoRDS
 *
 * Test RDS datapath with frequency change by tune, manual seek, auto seek, af switch
 * on a frequency with no RDS then change back to a frequency with RDS, user should be notified of RDS.
 *
 **************************/
static tSInt etalTestGetRDSDatapathChangeFreqNoRDS(ETAL_HANDLE *hReceiver, etalTestBroadcastTy mode, tBool *pass)
{
	EtalDataPathAttr dataPathAttr;
	tChar standard[16] = "\0";
	tU32 duration, frequency, step;
	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret;
    EtalAudioInterfTy audioIf;
    EtalSeekThreshold seekThreshold;

	if (mode == ETAL_TEST_MODE_FM)
	{
		duration = ETAL_TEST_GETRDS_FM_DURATION;
		OSAL_szStringCopy(standard, "FM");
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		duration = ETAL_TEST_GETRDS_HD_DURATION;
		OSAL_szStringCopy(standard, "HD");
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		duration = ETAL_TEST_GETRDS_FM_DURATION;
		OSAL_szStringCopy(standard, "HD from FM");
	}
	else
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("");
	etalTestPrintNormal("GetRDS from datapath with change frequency to no RDS for %s", standard);
	/*
	 * reconfigure the receiver so to force ETAL to reset RDS
	 * data, otherwise after the pass1 all data is considered
	 * already sent by the RDS decoder and not sent again
	 */
	if (etalTestDoRDSConfig(hReceiver, TRUE, mode, ETAL_INVALID_HANDLE) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	 * After a reconfiguration the tuned frequency is reset to 0
	 * and this causes problems with the RDS landscape, so a new
	 * tune is required
	 */
	if (mode == ETAL_TEST_MODE_FM)
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_FM, *hReceiver) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
        frequency = ETAL_VALID_FM_FREQ;
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		etalTestPrintNormal("* Tune to FM freq %d", ETAL_VALID_FM_FREQ);
		if ((ret = etal_tune_receiver(*hReceiver, ETAL_VALID_FM_FREQ)) != ETAL_RET_NO_DATA)
		{
			etalTestPrintError("etal_tune_receiver %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
        frequency = ETAL_VALID_FM_FREQ;
        /* Configure audio path */
        memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
        audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
		// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = true;
#endif

				// Audio path should be correctly set before
#if 0

        if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
        {
            etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
            return OSAL_ERROR;
        }
#endif

        /* Select audio source */
		if ((ret = etal_audio_select(*hReceiver, ETAL_AUDIO_SOURCE_AUTO_HD)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_audio_select %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}
	else
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, *hReceiver) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
        frequency = ETAL_VALID_HD_FREQ;
	}

	etalTestPrintNormal("* Config RDS datapath for %s", standard);
	dataPathAttr.m_receiverHandle = *hReceiver;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS;
	dataPathAttr.m_sink.m_context = NULL;
	dataPathAttr.m_sink.m_BufferSize = sizeof(EtalRDSData);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestRDSCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

    /* loop for tune, af switch, manual seek and auto seek change of frequency between RDS and no RDS */
#ifdef CONFIG_APP_TEST_IN_LE_MANS
    for(step = 0; step < 11; step++)
#else
    for(step = 0; step < 9; step++)
#endif // CONFIG_APP_TEST_IN_LE_MANS
    {
        /*
         * reset callback counter/data
         */
        RdsDecodedCbInvocations = 0;
        OSAL_pvMemorySet(&rdsDecodedDatapathData, 0x00, sizeof(EtalRDSData));

        if (*hReceiver != ETAL_INVALID_HANDLE)
        {
            switch (step)
            {
                case 0:
                    /*
                     * start RDS reception
                     */
                    etalTestPrintNormal("* Start RDS reception for %s", standard);
                    if ((ret = etal_start_RDS(*hReceiver, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_start_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    /*
                     * start RDS decoding
                     */
                    etalTestPrintNormal("* Start RDS decoding for %s", standard);
                    if ((ret = etaltml_start_decoded_RDS(*hReceiver, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etaltml_start_decoded_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                case 1:
                    /*
                     * tune foreground to other frequency
                     */
                    etalTestPrintNormal("* Tune to %s freq %d", standard, frequency);
                    if ((ret = etal_tune_receiver(*hReceiver, frequency)) != ((mode == ETAL_TEST_MODE_HD_FM)?ETAL_RET_NO_DATA:ETAL_RET_SUCCESS))
                    {
                        etalTestPrintError("etal_tune_receiver %s freq (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                case 2:
                case 3:
                case 4:
                    /*
                     * af_switch foreground to other frequency
                     */
                    etalTestPrintNormal("* AF switch to %s freq %d", standard, frequency);
                    if ((ret = etal_AF_switch(*hReceiver, frequency)) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_AF_switch %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                case 5:
                    frequency = ETAL_VALID_FM_FREQ;
                    etalTestPrintNormal("* Tune to %s freq %d", standard, frequency - ETAL_TEST_SEEK_FM_STEP_FREQ);
                    if ((ret = etal_tune_receiver(*hReceiver, frequency - ETAL_TEST_SEEK_FM_STEP_FREQ)) != ((mode == ETAL_TEST_MODE_HD_FM)?ETAL_RET_NO_DATA:ETAL_RET_SUCCESS))
                    {
                        etalTestPrintError("etal_tune_receiver %s freq %d (%s, %d)", standard, frequency - ETAL_TEST_SEEK_FM_STEP_FREQ, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    /*
                     * manual seek foreground to other frequency
                     */
                    etalTestPrintNormal("* manual seek start to %s freq %d", standard, frequency - ETAL_TEST_SEEK_FM_STEP_FREQ);
                    if (((ret = etal_seek_start_manual(*hReceiver, cmdDirectionUp, ETAL_TEST_SEEK_FM_STEP_FREQ, &etalTestRDSSeekFrequencyFound)) != ETAL_RET_SUCCESS) || (etalTestRDSSeekFrequencyFound != frequency))
                    {
                        etalTestPrintError("etal_seek_start_manual %s freq %d (%s, %d)", standard, etalTestRDSSeekFrequencyFound, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    etalTestPrintNormal("* manual seek stop to %s freq %d", standard, etalTestRDSSeekFrequencyFound);
                    if (((ret = etal_seek_stop_manual(*hReceiver, cmdAudioUnmuted, &etalTestRDSSeekFrequencyFound)) != ETAL_RET_SUCCESS) || (etalTestRDSSeekFrequencyFound != frequency))
                    {
                        etalTestPrintError("etal_seek_stop_manual %s freq %d (%s, %d)", standard, etalTestRDSSeekFrequencyFound, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                case 6:
                    frequency = ETAL_VALID_FM_FREQ - ETAL_TEST_SEEK_FM_STEP_FREQ;
                    /*
                     * manual seek foreground to other frequency
                     */
                    etalTestPrintNormal("* manual seek start to %s freq %d", standard, frequency + ETAL_TEST_SEEK_FM_STEP_FREQ);
                    if (((ret = etal_seek_start_manual(*hReceiver, cmdDirectionDown, ETAL_TEST_SEEK_FM_STEP_FREQ, &etalTestRDSSeekFrequencyFound)) != ETAL_RET_SUCCESS) || (etalTestRDSSeekFrequencyFound != frequency))
                    {
                        etalTestPrintError("etal_seek_start_manual %s freq %d (%s, %d)", standard, etalTestRDSSeekFrequencyFound, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    etalTestPrintNormal("* manual seek stop to %s freq %d", standard, etalTestRDSSeekFrequencyFound);
                    if (((ret = etal_seek_stop_manual(*hReceiver, cmdAudioUnmuted, &etalTestRDSSeekFrequencyFound)) != ETAL_RET_SUCCESS) || (etalTestRDSSeekFrequencyFound != frequency))
                    {
                        etalTestPrintError("etal_seek_stop_manual %s freq %d (%s, %d)", standard, etalTestRDSSeekFrequencyFound, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                case 7:
                    frequency = ETAL_VALID_FM_FREQ;
                    /*
                     * manual seek foreground to other frequency
                     */
                    etalTestPrintNormal("* manual seek start to %s freq %d", standard, frequency - ETAL_TEST_SEEK_FM_STEP_FREQ);
                    if (((ret = etal_seek_start_manual(*hReceiver, cmdDirectionUp, ETAL_TEST_SEEK_FM_STEP_FREQ, &etalTestRDSSeekFrequencyFound)) != ETAL_RET_SUCCESS) || (etalTestRDSSeekFrequencyFound != frequency))
                    {
                        etalTestPrintError("etal_seek_start_manual %s freq %d (%s, %d)", standard, etalTestRDSSeekFrequencyFound, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    etalTestPrintNormal("* manual seek stop to %s freq %d", standard, etalTestRDSSeekFrequencyFound);
                    if (((ret = etal_seek_stop_manual(*hReceiver, cmdAudioUnmuted, &etalTestRDSSeekFrequencyFound)) != ETAL_RET_SUCCESS) || (etalTestRDSSeekFrequencyFound != frequency))
                    {
                        etalTestPrintError("etal_seek_stop_manual %s freq %d (%s, %d)", standard, etalTestRDSSeekFrequencyFound, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                case 8:
                    frequency = ETAL_VALID_FM_FREQ;
                    etalTestRDSSeekFrequencyFound = ETAL_INVALID_FREQUENCY;
                    if ((etalTestRDSSeekFinishSem == 0) && 
                        (OSAL_s32SemaphoreCreate("Sem_etalTestRDSSeekFinish", &etalTestRDSSeekFinishSem, 0) == OSAL_ERROR))
                    {
                        return OSAL_ERROR;
                    }
                    etalTestPrintNormal("* Tune to %s freq %d", standard, frequency - ETAL_TEST_SEEK_FM_STEP_FREQ);
                    if ((ret = etal_tune_receiver(*hReceiver, frequency - ETAL_TEST_SEEK_FM_STEP_FREQ)) != ((mode == ETAL_TEST_MODE_HD_FM)?ETAL_RET_NO_DATA:ETAL_RET_SUCCESS))
                    {
                        etalTestPrintError("etal_tune_receiver %s freq (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }

					etalTestSetRdsThreshold(TRUE, &seekThreshold);

                    if (etal_set_autoseek_thresholds_value(*hReceiver, &seekThreshold) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_set_autoseek_thresholds_value %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    /*
                     * auto seek foreground to other frequency
                     */

                    etalTestPrintNormal("* auto seek start to %s freq %d", standard, frequency);
                    if ((ret = etal_autoseek_start(*hReceiver, cmdDirectionUp, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_autoseek_start %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    /* wait max 2 seconds for ETAL_SEEK_FINISHED event */
                    OSAL_s32SemaphoreWait(etalTestRDSSeekFinishSem, 2000);
                    if (etalTestRDSSeekFrequencyFound != frequency)
                    {
                        etalTestPrintError("etal_autoseek_start %s freq not found %d (%s, %d)", standard, etalTestRDSSeekFrequencyFound, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
					etalTestPrintNormal("* auto seek stop to %s freq %d", standard, frequency);
                    if ((ret = etal_autoseek_stop(*hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_autoseek_stop %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                case 9:
                    frequency = ETAL_VALID_FM_FREQ + ETAL_TEST_SEEK_FM_STEP_FREQ;
					
    				etalTestSetRdsThreshold(FALSE, &seekThreshold);


                    if (etal_set_autoseek_thresholds_value(*hReceiver, &seekThreshold) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_set_autoseek_thresholds_value %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    /*
                     * auto seek foreground to other frequency
                     */

                    etalTestPrintNormal("* auto seek start to %s freq %d", standard, frequency);
                    if ((ret = etal_autoseek_start(*hReceiver, cmdDirectionUp, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_autoseek_start %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    /* wait max 2 seconds for ETAL_SEEK_FINISHED event */
                    OSAL_s32SemaphoreWait(etalTestRDSSeekFinishSem, 2000);
                    if ((ret = etal_autoseek_stop(*hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_autoseek_stop %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                case 10:
                    frequency = ETAL_VALID_FM_FREQ;

					etalTestSetRdsThreshold(TRUE, &seekThreshold);


                    if (etal_set_autoseek_thresholds_value(*hReceiver, &seekThreshold) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_set_autoseek_thresholds_value %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    /*
                     * auto seek foreground to other frequency
                     */

                    etalTestPrintNormal("* auto seek start to %s freq %d", standard, frequency);
                    if ((ret = etal_autoseek_start(*hReceiver, cmdDirectionDown, ETAL_TEST_SEEK_FM_STEP_FREQ, cmdAudioUnmuted, seekInSPS, TRUE)) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_autoseek_start %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    /* wait max 2 seconds for ETAL_SEEK_FINISHED event */
                    OSAL_s32SemaphoreWait(etalTestRDSSeekFinishSem, 2000);
                    if (etalTestRDSSeekFrequencyFound != frequency)
                    {
                        etalTestPrintError("etal_autoseek_start %s freq not found %d (%s, %d)", standard, etalTestRDSSeekFrequencyFound, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
					etalTestPrintNormal("* auto seek stop to %s freq %d", standard, frequency);
                    if ((ret = etal_autoseek_stop(*hReceiver, lastFrequency)) != ETAL_RET_SUCCESS)
                    {
                        etalTestPrintError("etal_autoseek_stop %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
                        return OSAL_ERROR;
                    }
                    break;
                default:
                    break;
            }
        }

        OSAL_s32ThreadWait(duration);

        /*
         * check RDS callback invocations and compare
         */
        etalTestPrintNormal("= complete, %d RDS callback invocations", RdsDecodedCbInvocations);
        if (frequency == ETAL_VALID_FM_FREQ)
        {
            etalTestPrintNormal("= (expected more than %d)", ETAL_TEST_GET_RDS_MIN);
        }
        else
        {
            etalTestPrintNormal("= (expected 0)");
        }

        if ((mode == ETAL_TEST_MODE_FM) || (mode == ETAL_TEST_MODE_HD_FM))
        {
            if (RdsDecodedCbInvocations >= ETAL_TEST_GET_RDS_MIN)
            {
                if (frequency == ETAL_VALID_FM_FREQ)
                {
                    *pass = etalTestGetRDSCompare(&rdsDecodedDatapathData, &etalTestRDSReference);
                }
                else
                {
                    *pass = FALSE;
                }
            }
            else if ((RdsDecodedCbInvocations != 0) && (RdsDecodedCbInvocations < ETAL_TEST_GET_RDS_MIN))
            {
                *pass = FALSE;
            }
            else
            {   /* RdsDecodedCbInvocations == 0 */
                if (frequency == ETAL_VALID_FM_FREQ)
                {
                    *pass = FALSE;
                }
                else
                {
                    *pass = TRUE;
                }
            }
            if (*pass == FALSE)
            {
                break;
            }
        }

        /*
         * change foreground to other frequency
         */
        if ((mode == ETAL_TEST_MODE_FM) || (mode == ETAL_TEST_MODE_HD_FM)) 
        {
            if (frequency == ETAL_VALID_FM_FREQ)
            {
                frequency = ETAL_EMPTY_FM_FREQ;
            }
            else
            {
                frequency = ETAL_VALID_FM_FREQ;
            }
        }
        else
        {
            if (frequency == ETAL_VALID_HD_FREQ)
            {
                frequency = ETAL_EMPTY_FM_FREQ;
            }
            else
            {
                frequency = ETAL_VALID_HD_FREQ;
            }
        }
    }

	/*
	 * stop RDS decoding
	 */
	etalTestPrintNormal("* Stop RDS decoding for %s", standard);
	if ((ret = etaltml_stop_decoded_RDS(*hReceiver, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_stop_decoded_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * stop RDS reception and datapath
	 */
	if ((ret = etal_stop_RDS(*hReceiver)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_stop_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	if ((ret = etal_destroy_datapath(&hDatapath)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/*
	 * Undo tune
	 */
    if ((mode == ETAL_TEST_MODE_FM) || (mode == ETAL_TEST_MODE_HD_FM))
    {
        if (etalTestUndoTuneSingle(ETAL_TUNE_FM, *hReceiver) != OSAL_OK)
        {
            return OSAL_ERROR;
        }
    }
    else
    {
        if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_FM, *hReceiver) != OSAL_OK)
        {
            return OSAL_ERROR;
        }
    }

    if (etalTestUndoRDSConfig(hReceiver) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_FM

#if defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_APP_TEST_DAB) // otherwise FE1 is dedicated to DAB and not available for BG
#if defined (CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL)
/***************************
 *
 * etalTestGetRDSDatapath2
 *
 **************************/
static tSInt etalTestGetRDSDatapath2(ETAL_HANDLE *hReceiver, etalTestBroadcastTy mode, ETAL_HANDLE *hReceiver2, tU8 mode2, tBool *pass)
{
	EtalDataPathAttr dataPathAttr;
	tChar standard[16] = "\0", standard2[21] = "\0";
	tU32 duration;
	ETAL_HANDLE hDatapath = ETAL_INVALID_HANDLE, hDatapath2 = ETAL_INVALID_HANDLE;
	ETAL_STATUS ret;
    EtalAudioInterfTy audioIf;

	if (mode == ETAL_TEST_MODE_FM)
	{
		duration = ETAL_TEST_GETRDS_FM_DURATION;
		OSAL_szStringCopy(standard, "FM");
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		duration = ETAL_TEST_GETRDS_HD_DURATION;
		OSAL_szStringCopy(standard, "HD");
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		duration = ETAL_TEST_GETRDS_FM_DURATION;
		OSAL_szStringCopy(standard, "HD from FM freq");
	}
	else
	{
		return OSAL_ERROR;
	}

	if (mode2 == ETAL_TEST_MODE_FM)
	{
		duration = (duration < ETAL_TEST_GETRDS_FM_DURATION) ? ETAL_TEST_GETRDS_FM_DURATION : duration;
		OSAL_szStringCopy(standard2, "FM2");
	}
	else if (mode2 == ETAL_TEST_MODE_HD_FM)
	{
		duration = (duration < ETAL_TEST_GETRDS_HD_DURATION) ? ETAL_TEST_GETRDS_HD_DURATION : duration;
		OSAL_szStringCopy(standard2, "HD2");
	}
	else if (mode2 == ETAL_TEST_MODE_HD_FM)
	{
		duration = (duration < ETAL_TEST_GETRDS_FM_DURATION) ? ETAL_TEST_GETRDS_FM_DURATION : duration;
		OSAL_szStringCopy(standard2, " & HD2 from FM freq2");
	}
	else
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("");
	etalTestPrintNormal("GetRDS from datapath for %s & %s", standard, standard2);
	/*
	 * reconfigure the receiver so to force ETAL to reset RDS
	 * data, otherwise after the pass1 all data is considered
	 * already sent by the RDS decoder and not sent again
	 */
    if (etalTestDoRDSConfig(hReceiver, TRUE, mode, ETAL_INVALID_HANDLE) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (etalTestDoRDSConfig(hReceiver2, TRUE, mode, ETAL_FE_FOR_FM_TEST2) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
	/*
	 * After a reconfiguration the tuned frequency is reset to 0
	 * and this causes problems with the RDS landscape, so a new
	 * tune is required
	 */
	if (mode == ETAL_TEST_MODE_FM)
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_FM, *hReceiver) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}
	else if (mode == ETAL_TEST_MODE_HD_FM)
	{
		etalTestPrintNormal("* Tune to FM freq %d", ETAL_VALID_FM_FREQ);
		if ((ret = etal_tune_receiver(*hReceiver, ETAL_VALID_FM_FREQ)) != ETAL_RET_NO_DATA)
		{
			etalTestPrintError("etal_tune_receiver %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
        /* Configure audio path */
        memset(&audioIf, 0, sizeof(EtalAudioInterfTy));
        audioIf.m_dac = audioIf.m_sai_out = audioIf.m_sai_in = 1;
		// Slave mode adaptation depending on HD
#ifdef CONFIG_MODULE_DCOP_HDRADIO_SAI_IS_MASTER
	audioIf.m_sai_slave_mode = true;
#endif

				// Audio path should be correctly set before
#if 0

        if ((ret = etal_config_audio_path(0, audioIf)) != ETAL_RET_SUCCESS)
        {
            etalTestPrintError("etal_config_audio_path (%s, %d)", ETAL_STATUS_toString(ret), ret);
            return OSAL_ERROR;
        }
#endif
        /* Select audio source */
		if ((ret = etal_audio_select(*hReceiver, ETAL_AUDIO_SOURCE_AUTO_HD)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_audio_select %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
			return OSAL_ERROR;
		}
	}
	else
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, *hReceiver) != OSAL_OK)
		{
			return OSAL_ERROR;
		}
	}

    if (mode2 == ETAL_TEST_MODE_FM)
    {
        /*
         * tune FM background
         */
        if (*hReceiver2 != ETAL_INVALID_HANDLE)
        {
            etalTestPrintNormal("* Tune to FM2 freq2 %d", ETAL_VALID_FM_FREQ2);
            if ((ret = etal_tune_receiver(*hReceiver2, ETAL_VALID_FM_FREQ2)) != ETAL_RET_SUCCESS)
            {
                etalTestPrintError("etal_tune_receiver FM2 (%s, %d)", ETAL_STATUS_toString(ret), ret);
                return OSAL_ERROR;
            }
        }
    }
    else if (mode2 == ETAL_TEST_MODE_HD_FM)
    {
        etalTestPrintNormal("* Tune to FM2 freq2 %d", ETAL_VALID_FM_FREQ2);
        if ((ret = etal_tune_receiver(*hReceiver2, ETAL_VALID_FM_FREQ2)) != ETAL_RET_NO_DATA)
        {
            etalTestPrintError("etal_tune_receiver %s (%s, %d)", standard2, ETAL_STATUS_toString(ret), ret);
            return OSAL_ERROR;
        }
    }
    else
    {
    	if (*hReceiver2 != ETAL_INVALID_HANDLE)
    	{
    		etalTestPrintNormal("* Tune to HD freq %d", ETAL_VALID_HD_FREQ);
    		if ((ret = etal_tune_receiver(*hReceiver2, ETAL_VALID_HD_FREQ)) != ETAL_RET_SUCCESS)
    		{
    			etalTestPrintError("etal_tune_receiver HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
    			return OSAL_ERROR;
            }
        }
    }

	etalTestPrintNormal("* Config RDS datapath for %s", standard);
	dataPathAttr.m_receiverHandle = *hReceiver;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS;
	dataPathAttr.m_sink.m_context = NULL;
	dataPathAttr.m_sink.m_BufferSize = sizeof(EtalRDSData);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestRDSCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	etalTestPrintNormal("* Config RDS datapath for %s", standard2);
	dataPathAttr.m_receiverHandle = *hReceiver2;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS;
	dataPathAttr.m_sink.m_context = (tVoid *)2; // distinguish call from FM from FM2 in callback
	dataPathAttr.m_sink.m_BufferSize = sizeof(EtalRDSData);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestRDSCallback;
	if ((ret = etal_config_datapath(&hDatapath2, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for %s (%s, %d)", standard2, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * reset callback counter/data
	 */
    RdsDecodedCbInvocations = 0;
	RdsDecodedCbInvocations2 = 0;
	OSAL_pvMemorySet(&rdsDecodedDatapathData, 0x00, sizeof(EtalRDSData));
	OSAL_pvMemorySet(&rdsDecodedDatapathData2, 0x00, sizeof(EtalRDSData));
	/*
	 * start RDS recpetion foreground
	 */
	etalTestPrintNormal("* Start RDS reception for %s", standard);
	if ((ret = etal_start_RDS(*hReceiver, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_start_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * start RDS decoding foreground
	 */
	etalTestPrintNormal("* Start RDS decoding for %s", standard);
	if ((ret = etaltml_start_decoded_RDS(*hReceiver, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_start_decoded_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * start RDS reception background
	 */
	etalTestPrintNormal("* Start RDS reception for %s", standard2);
	if ((ret = etal_start_RDS(*hReceiver2, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_start_RDS for %s (%s, %d)", standard2, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * start RDS decoding background
	 */
	etalTestPrintNormal("* Start RDS decoding for %s", standard2);
	if ((ret = etaltml_start_decoded_RDS(*hReceiver2, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_start_decoded_RDS for %s (%s, %d)", standard2, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	OSAL_s32ThreadWait(duration);

	/*
	 * stop RDS decoding
	 */
	etalTestPrintNormal("* Stop RDS decoding for %s and %s", standard, standard2);
	if ((ret = etaltml_stop_decoded_RDS(*hReceiver, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_stop_decoded_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	if ((ret = etaltml_stop_decoded_RDS(*hReceiver2, ETAL_DECODED_RDS_VALID_ALL)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_stop_decoded_RDS for %s (%s, %d)", standard2, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * stop RDS reception and compare
	 */
	if ((ret = etal_stop_RDS(*hReceiver)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_stop_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	if ((ret = etal_stop_RDS(*hReceiver2)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_stop_RDS for %s (%s, %d)", standard2, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("= complete, %d FM and %d FM2 RDS callback invocations", RdsDecodedCbInvocations, RdsDecodedCbInvocations2);
	etalTestPrintNormal("= (expected more than %d)", ETAL_TEST_GET_RDS_MIN);

	if ((mode == ETAL_TEST_MODE_FM) ||
		(mode == ETAL_TEST_MODE_HD_FM))
	{
		if (RdsDecodedCbInvocations >= ETAL_TEST_GET_RDS_MIN)
		{
			*pass = etalTestGetRDSCompare(&rdsDecodedDatapathData, &etalTestRDSReference);
		}
		else
		{
			*pass = FALSE;
		}
	}
	if ((mode2 == ETAL_TEST_MODE_FM) ||
		(mode2 == ETAL_TEST_MODE_HD_FM))
	{
		if (RdsDecodedCbInvocations2 >= ETAL_TEST_GET_RDS_MIN)
		{
			*pass &= etalTestGetRDSCompare(&rdsDecodedDatapathData2, &etalTestRDSReference2);
		}
		else
		{
			*pass = FALSE;
		}
	}
	if ((ret = etal_destroy_datapath(&hDatapath)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	if ((ret = etal_destroy_datapath(&hDatapath2)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_datapath for %s (%s, %d)", standard2, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

    if (etalTestUndoTuneSingle(ETAL_TUNE_FM, *hReceiver) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (etalTestUndoTuneSingle(ETAL_TUNE_FM, *hReceiver2) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (etalTestUndoRDSConfig(hReceiver) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (etalTestUndoRDSConfig(hReceiver2) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
	return OSAL_OK;
}
#endif // CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL
#endif // CONFIG_APP_TEST_FM && !CONFIG_APP_TEST_DAB
#endif // CONFIG_APP_TEST_GETRDS_DATAPATH

#ifdef CONFIG_APP_TEST_GETRDS
/***************************
 *
 * etalTestRDSInvalidParam
 *
 **************************/
static tSInt etalTestRDSInvalidParam(tBool *pass)
{
	tBool pass1 = TRUE, pass2 = TRUE, pass3 = TRUE, pass4 = TRUE, pass5 = TRUE;
	ETAL_STATUS ret = ETAL_RET_SUCCESS;
#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_HDRADIO_FM)
	tU32 i;
#endif // CONFIG_APP_TEST_FM || CONFIG_APP_TEST_HDRADIO_FM
#if defined(CONFIG_APP_TEST_HDRADIO_AM)
	EtalReceiverAttr attr;
#endif // CONFIG_APP_TEST_HDRADIO_AM

#if defined(CONFIG_APP_TEST_FM)
	/*
	* Do FM receiver configuration
	*/
	if (pass1 == TRUE)
	{
		handlefm = ETAL_INVALID_HANDLE;
		if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
		{
			pass1 = FALSE;
		}
	}

	/*
	* Do tune FM
	*/
	if (pass1 == TRUE)
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
		{
			pass1 = FALSE;
		}
	}

	if (pass1 == TRUE)
	{
		/*
		 * start RDS
		 */
		etalTestPrintNormal("* Start RDS with invalid receiver");
		if ((ret = etal_start_RDS(ETAL_INVALID_HANDLE, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_INVALID_HANDLE)
		{
			etalTestPrintError("etal_start_RDS FM not rejected for invalid receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
			pass1 = FALSE;
		}

		etalTestPrintNormal("* Start RDS with invalid forceFastPi");
		if ((ret = etal_start_RDS(handlefm, TRUE, 0, ETAL_RDS_MODE)) != ETAL_RET_PARAMETER_ERR)
		{
			etalTestPrintError("etal_start_RDS FM not rejected for invalid forceFastPi (%s, %d)", ETAL_STATUS_toString(ret), ret);
			pass1 = FALSE;
		}

		etalTestPrintNormal("* Start RDS with invalid numPi");
		for (i = 16; i < 256; i++)
		{
			if ((ret = etal_start_RDS(handlefm, FALSE, (tU8)i, ETAL_RDS_MODE)) != ETAL_RET_PARAMETER_ERR)
			{
				etalTestPrintError("etal_start_RDS FM not rejected for numPi %d (%s, %d)", i, ETAL_STATUS_toString(ret), ret);
				pass1 = FALSE;
			}
		}

		etalTestPrintNormal("* Start RDS with invalid mode");
		if ((ret = etal_start_RDS(handlefm, FALSE, 0, (ETAL_RBDS_MODE + 1))) != ETAL_RET_PARAMETER_ERR)
		{
			etalTestPrintError("etal_start_RDS FM not rejected for invalid mode (%s, %d)", ETAL_STATUS_toString(ret), ret);
			pass1 = FALSE;
		}

		/*
		 * stop RDS
		 */
		etalTestPrintNormal("* Stop RDS with invalid receiver");
		if ((ret = etal_stop_RDS(ETAL_INVALID_HANDLE)) != ETAL_RET_INVALID_HANDLE)
		{
			etalTestPrintError("etal_stop_RDS FM not rejected for invalid receiver (%s, %d)", ETAL_STATUS_toString(ret), ret);
			pass1 = FALSE;
		}
	}

	if (handlefm != ETAL_INVALID_HANDLE)
	{
		/*
		* Undo tune FM
		*/
		if (etalTestUndoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
		{
			pass1 = FALSE;
		}

		/*
		* Undo FM receiver configuration
		*/
		if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
		{
			pass1 = FALSE;
		}
	}
#endif // CONFIG_APP_TEST_FM

#if defined(CONFIG_APP_TEST_HDRADIO_FM)
	/*
	* Do HD FM receiver configuration
	*/
	if (pass2 == TRUE)
	{
		handlehd = ETAL_INVALID_HANDLE;
		if (etalTestDoConfigSingle(ETAL_CONFIG_HDRADIO_FM, &handlehd) != OSAL_OK)
		{
			pass2 = FALSE;
		}
	}

	/*
	* Do tune HD FM
	*/
	if (pass2 == TRUE)
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
		{
			pass2 = FALSE;
		}
	}

	if (pass2 == TRUE)
	{
		/*
		 * start RDS
		 */
		etalTestPrintNormal("* Start RDS with invalid forceFastPi");
		if ((ret = etal_start_RDS(handlehd, TRUE, 0, ETAL_RDS_MODE)) != ETAL_RET_PARAMETER_ERR)
		{
			etalTestPrintError("etal_start_RDS HD FM not rejected for invalid forceFastPi (%s, %d)", ETAL_STATUS_toString(ret), ret);
			pass1 = FALSE;
		}

		etalTestPrintNormal("* Start RDS with invalid numPi");
		for (i = 16; i < 256; i++)
		{
			if ((ret = etal_start_RDS(handlehd, FALSE, (tU8)i, ETAL_RDS_MODE)) != ETAL_RET_PARAMETER_ERR)
			{
				etalTestPrintError("etal_start_RDS HD FM not rejected for numPi %d (%s, %d)", i, ETAL_STATUS_toString(ret), ret);
				pass2 = FALSE;
			}
		}

		etalTestPrintNormal("* Start RDS with invalid mode");
		if ((ret = etal_start_RDS(handlehd, FALSE, 0, (ETAL_RBDS_MODE + 1))) != ETAL_RET_PARAMETER_ERR)
		{
			etalTestPrintError("etal_start_RDS HD FM not rejected for invalid mode (%s, %d)", ETAL_STATUS_toString(ret), ret);
			pass2 = FALSE;
		}
	}

	if (handlehd != ETAL_INVALID_HANDLE)
	{
		/*
		* Undo tune HD FM
		*/
		if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
		{
			pass2 = FALSE;
		}

		/*
		* Undo HD FM receiver configuration
		*/
		if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
		{
			pass2 = FALSE;
		}
	}
#endif // CONFIG_APP_TEST_HDRADIO_FM

#if defined(CONFIG_APP_TEST_AM)
	if (pass3 == TRUE)
	{
		handleam = ETAL_INVALID_HANDLE;
		if (etalTestDoConfigSingle(ETAL_CONFIG_AM, &handleam) != OSAL_OK)
		{
			pass3 = FALSE;
		}
	}

	if (pass3 == TRUE)
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_AM, handleam) != OSAL_OK)
		{
			pass3 = FALSE;
		}
	}

	/*
	 * start RDS
	 */
	if (pass3 == TRUE)
	{
		etalTestPrintNormal("* Start RDS with invalid AM receiver");
		if ((ret = etal_start_RDS(handleam, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_INVALID_RECEIVER)
		{
			etalTestPrintError("etal_start_RDS for AM should be rejected (%s, %d)", ETAL_STATUS_toString(ret), ret);
			pass3 = FALSE;
		}
	}

	if (handleam != ETAL_INVALID_HANDLE)
	{
		/*
		* Undo tune AM background
		*/
		if (etalTestUndoTuneSingle(ETAL_TUNE_AM, handleam) != OSAL_OK)
		{
			pass3 = FALSE;
		}

		/*
		* Undo AM receiver configuration
		*/
		if (etalTestUndoConfigSingle(&handleam) != OSAL_OK)
		{
			pass3 = FALSE;
		}
	}
#endif // CONFIG_APP_TEST_AM

#if defined(CONFIG_APP_TEST_DAB)
	if (pass4 == TRUE)
	{
		handledab = ETAL_INVALID_HANDLE;
		if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
		{
			pass4 = FALSE;
		}
	}

	if (pass4 == TRUE)
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
		{
			pass4 = FALSE;
		}
	}

	/*
	 * start RDS
	 */
	if (pass4 == TRUE)
	{
		etalTestPrintNormal("* Start RDS with invalid DAB receiver");
		if ((ret = etal_start_RDS(handledab, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_INVALID_RECEIVER)
		{
			etalTestPrintError("etal_start_RDS for DAB should be rejected (%s, %d)", ETAL_STATUS_toString(ret), ret);
			pass4 = FALSE;
		}
	}

	if (handledab != ETAL_INVALID_HANDLE)
	{
		/*
		* Undo tune DAB background
		*/
		if (etalTestUndoTuneSingle(ETAL_TUNE_DAB, handledab) != OSAL_OK)
		{
			pass4 = FALSE;
		}

		/*
		* Undo DAB receiver configuration
		*/
		if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
		{
			pass4 = FALSE;
		}
	}
#endif // CONFIG_APP_TEST_DAB

#if defined(CONFIG_APP_TEST_HDRADIO_AM)
	if (pass5 == TRUE)
	{
		handlehd = ETAL_INVALID_HANDLE;
		OSAL_pvMemorySet(&attr, 0x00, sizeof(EtalReceiverAttr));
		attr.m_Standard = ETAL_BCAST_STD_HD_AM;
		attr.m_FrontEndsSize = 1;
		attr.m_FrontEnds[0] = ETAL_FE_HANDLE_1 /*ETAL_FE_FOR_HD_TEST*/;
		etalTestPrintNormal("* Create %s receiver", etalTestStandard2Ascii(attr.m_Standard));
		if ((ret = etal_config_receiver(&handlehd, &attr)) != ETAL_RET_SUCCESS)
		{
			etalTestPrintError("etal_config_receiver %s (%s, %d)", etalTestStandard2Ascii(attr.m_Standard), ETAL_STATUS_toString(ret), ret);
			pass5 = FALSE;
		}
		else
		{
			etalTestPrintNormal("* Created %s receiver, handle %d", etalTestStandard2Ascii(attr.m_Standard), handlehd);
		}
	}

	if (pass5 == TRUE)
	{
		if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_AM, handlehd) != OSAL_OK)
		{
			pass5 = FALSE;
		}
	}

	/*
	 * start RDS
	 */
	if (pass5 == TRUE)
	{
		etalTestPrintNormal("* Start RDS with invalid HD AM receiver");
		if ((ret = etal_start_RDS(handlehd, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_INVALID_RECEIVER)
		{
			etalTestPrintError("etal_start_RDS for HD AM should be rejected (%s, %d)", ETAL_STATUS_toString(ret), ret);
			pass5 = FALSE;
		}
	}

	if (handlehd != ETAL_INVALID_HANDLE)
	{
		/*
		* Undo tune HD AM background
		*/
		if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_AM, handlehd) != OSAL_OK)
		{
			pass5 = FALSE;
		}

		/*
		* Undo HD AM receiver configuration
		*/
		if (etalTestUndoConfigSingle(&handlehd) != OSAL_OK)
		{
			pass5 = FALSE;
		}
	}
#endif // CONFIG_APP_TEST_HDRADIO_FM

	*pass = pass1 && pass2 && pass3 && pass4 && pass5;
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_GETRDS


#ifdef CONFIG_APP_TEST_GETRAWRDS_DATAPATH
/***************************
 *
 * etalTestGetRDSRawDatapath
 *
 **************************/
static tSInt etalTestGetRDSRawDatapath(ETAL_HANDLE *hReceiver, etalTestBroadcastTy mode, tBool *pass_out)
{
	EtalDataPathAttr dataPathAttr;
	tChar standard[16] = "\0";
	tU32 duration;
	ETAL_HANDLE hDatapath;
	ETAL_STATUS ret;
	tBool pass;

	if (mode == ETAL_TEST_MODE_FM)
	{
		duration = ETAL_TEST_GETRDS_FM_DURATION;
		OSAL_szStringCopy(standard, "FM");
	}
	else
	{
		return OSAL_ERROR;
	}

	/*
	 * reconfigure the receiver so to force ETAL to reset RDS
	 * data, otherwise after the pass1 all data is considered
	 * already sent by the RDS decoder and not sent again
	 */
	if (etalTestDoRDSConfig(hReceiver, TRUE, mode, ETAL_INVALID_HANDLE) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	 * After a reconfiguration the tuned frequency is reset to 0
	 * and this causes problems with the RDS landscape, so a new
	 * tune is required
	 */
	if (etalTestDoTuneSingle(ETAL_TUNE_FM, *hReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintNormal("* Config RDS Raw datapath for %s", standard);
	hDatapath = ETAL_INVALID_HANDLE;
	dataPathAttr.m_receiverHandle = *hReceiver;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS_RAW;
	dataPathAttr.m_sink.m_context = hReceiver;
	dataPathAttr.m_sink.m_BufferSize = sizeof(EtalRDSRawData);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestRDSRawCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * reset callback counter/data
	 */

	RdsRawCbInvocations = 0;
	OSAL_pvMemorySet(&rdsRawDatapathData, 0x00, sizeof(EtalRDSRaw));
	/*
	 * start datapath
	 */
	etalTestPrintReportPassStart(GET_RAW_RDS_START_DATAPATH, mode, "Start RDS datapath");
	pass = TRUE;
	if ((ret = etal_start_RDS(*hReceiver, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_start_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);

		OSAL_s32ThreadWait(duration);

		/*
		 * stop datapath and compare
		 */
		etalTestPrintReportPassStart(GET_RAW_RDS_STOP_DATAPATH, mode, "Stop RDS datapath");
		if (etal_stop_RDS(*hReceiver) != ETAL_RET_SUCCESS)
		{
			pass = FALSE;
			etalTestPrintReportPassEnd(testFailed);
			etalTestPrintNormal("etal_stop_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		}
		else
		{
			etalTestPrintReportPassEnd(testPassed);
		}
	}
	etalTestPrintNormal("= complete, %d RDS Raw callback invocations", RdsRawCbInvocations);
	etalTestPrintNormal("= (expected more than %d)", ETAL_TEST_GET_RDS_MIN);

	etalTestPrintReportPassStart(GET_RAW_RDS_VERIFY_CB_INVOC, mode, "Verify RDS callback invocations");
	if (RdsRawCbInvocations < ETAL_TEST_GET_RDS_MIN)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(GET_RAW_RDS_VERIFY_DATA, mode, "Verify RDS data");
	if (etalTestGetRDSRawCompare(&rdsRawDatapathData, &etalTestRDSRawReference, FALSE) != OSAL_OK)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(GET_RAW_RDS_DESTROY_DATAPATH, mode, "Destroy RDS datapath");
	if ((ret = etal_destroy_datapath(&hDatapath)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_destroy_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintNormal("* Config RBDS Raw datapath for %s", standard);
	dataPathAttr.m_receiverHandle = *hReceiver;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS_RAW;
	dataPathAttr.m_sink.m_context = hReceiver;
	dataPathAttr.m_sink.m_BufferSize = sizeof(EtalRDSRawData);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestRDSRawCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_config_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	/*
	 * reset callback counter/data
	 */
	RdsRawCbInvocations = 0;
	OSAL_pvMemorySet(&rdsRawDatapathData, 0x00, sizeof(EtalRDSRaw));
	/*
	 * start datapath
	 */
	etalTestPrintReportPassStart(GET_RAW_RDS_START_RBDS_DATAPATH, mode, "Start RBDS Raw datapath");
	if ((ret = etal_start_RDS(*hReceiver, FALSE, 0, ETAL_RBDS_MODE)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_start_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	OSAL_s32ThreadWait(duration);

	/*
	 * stop datapath and compare
	 */
	etalTestPrintReportPassStart(GET_RAW_RDS_STOP_RBDS_DATAPATH, mode, "Stop RBDS Raw datapath");
	if ((ret = etal_stop_RDS(*hReceiver)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_stop_RBDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintNormal("= complete, %d RBDS Raw callback invocations", RdsRawCbInvocations);
	etalTestPrintNormal("= (expected more than %d)", ETAL_TEST_GET_RDS_MIN);

	etalTestPrintReportPassStart(GET_RAW_RDS_VERIFY_RBDS_CB_INVOC, mode, "Verify RBDS callback invocations");
	if (RdsRawCbInvocations < ETAL_TEST_GET_RDS_MIN)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(GET_RAW_RDS_VERIFY_RBDS_DATA, mode, "Verify RBDS data");
	if (etalTestGetRDSRawCompare(&rdsRawDatapathData, &etalTestRDSRawReference, FALSE) != OSAL_OK)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(GET_RAW_RDS_DESTROY_RBDS_DATAPATH, mode, "Destroy RBDS Raw datapath");
	if ((ret = etal_destroy_datapath(&hDatapath)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_destroy_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (!pass)
	{
		*pass_out = FALSE;
	}
	return OSAL_OK;
}

/***************************
 *
 * etalTestGetRDSRawFastPIDatapath
 *
 **************************/
static tSInt etalTestGetRDSRawFastPIDatapath(ETAL_HANDLE *hReceiver, etalTestBroadcastTy mode, tBool *pass_out)
{
	EtalDataPathAttr dataPathAttr;
	tChar standard[16] = "\0";
	tU32 duration;
	ETAL_HANDLE hDatapath;
	ETAL_STATUS ret;
	tBool pass;

	if (mode == ETAL_TEST_MODE_FM)
	{
		duration = ETAL_TEST_GETRDS_FM_DURATION;
		OSAL_szStringCopy(standard, "FM");
	}
	else
	{
		return OSAL_ERROR;
	}

	/*
	 * reconfigure the receiver so to force ETAL to reset RDS
	 * data, otherwise after the pass1 all data is considered
	 * already sent by the RDS decoder and not sent again
	 */
	if (etalTestDoRDSConfig(hReceiver, TRUE, mode, ETAL_INVALID_HANDLE) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	/*
	 * After a reconfiguration the tuned frequency is reset to 0
	 * and this causes problems with the RDS landscape, so a new
	 * tune is required
	 */
	if (etalTestDoTuneSingle(ETAL_TUNE_FM, *hReceiver) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(GET_RAW_RDS_CONFIGURE_RAW_DATAPATH_FAST_PI, mode, "Configure RAW RDS datapath for Fast PI");
	hDatapath = ETAL_INVALID_HANDLE;
	dataPathAttr.m_receiverHandle = *hReceiver;
	dataPathAttr.m_dataType = ETAL_DATA_TYPE_FM_RDS_RAW;
	dataPathAttr.m_sink.m_context = hReceiver;
	dataPathAttr.m_sink.m_BufferSize = sizeof(EtalRDSRawData);
	dataPathAttr.m_sink.m_CbProcessBlock = etalTestRDSRawCallback;
	if ((ret = etal_config_datapath(&hDatapath, &dataPathAttr)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_config_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/*
	 * reset callback counter/data
	 */
	RdsRawCbInvocations = 0;
	OSAL_pvMemorySet(&rdsRawDatapathData, 0x00, sizeof(EtalRDSRaw));
	/*
	 * start fast PI mode datapath
	 */
	etalTestPrintReportPassStart(GET_RAW_RDS_START_RAW_FAST_PI, mode, "Start RDS Raw fast PI mode");
	if ((ret = etal_start_RDS(*hReceiver, FALSE, 15, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_start_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	OSAL_s32ThreadWait(650);

	/*
	 * compare RDS data
	 */
	etalTestPrintReportPassStart(GET_RAW_RDS_VERIFY_RAW_INVOC_FAST_PI, mode, "Verify RAW RDS invocations for Fast PI datapath");
	if (RdsRawCbInvocations < ETAL_TEST_GET_RDS_MIN)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(GET_RAW_RDS_CHECK_RDS_VAL_FAST_PI, mode, "Check RAW RDS values for Fast PI datapath");
	if (etalTestGetRDSRawCompare(&rdsRawDatapathData, &etalTestRDSRawReference, TRUE) != OSAL_OK)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (!pass)
	{
		*pass_out = FALSE;
		return OSAL_OK;
	}

	OSAL_s32ThreadWait(duration);

	/*
	 * start normal mode datapath
	 */
	etalTestPrintNormal("* Start RDS Raw normal mode datapath for %s", standard);
	if ((ret = etal_start_RDS(*hReceiver, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_start_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	OSAL_s32ThreadWait(duration);

	/*
	 * compare RDS data
	 */
	etalTestPrintReportPassStart(GET_RAW_RDS_CHECK_RDS_VAL_NORMAL, mode, "Check RAW RDS values for normal mode datapath");
	if (RdsRawCbInvocations < ETAL_TEST_GET_RDS_MIN)
	{
		pass = FALSE;
	}

	if (etalTestGetRDSRawCompare(&rdsRawDatapathData, &etalTestRDSRawReference, FALSE) != OSAL_OK)
	{
		pass = FALSE;
	}

	if (!pass)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (!pass)
	{
		*pass_out = FALSE;
		return OSAL_OK;
	}

	/*
	 * start force fast PI mode datapath
	 */
	etalTestPrintReportPassStart(GET_RAW_RDS_FORCE_FAST_PI, mode, "RDS Raw force fast PI mode datapath");
	if ((ret = etal_start_RDS(*hReceiver, TRUE, 1, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintNormal("etal_start_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
	}

	OSAL_s32ThreadWait(duration);

	/*
	 * compare RDS data
	 */
	if (RdsRawCbInvocations < ETAL_TEST_GET_RDS_MIN)
	{
		pass = FALSE;
	}
	if (etalTestGetRDSRawCompare(&rdsRawDatapathData, &etalTestRDSRawReference, TRUE) != OSAL_OK)
	{
		pass = FALSE;
	}
	if (!pass)
	{
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (!pass)
	{
		*pass_out = FALSE;
		return OSAL_OK;
	}

	/*
	 * start normal mode datapath
	 */
	etalTestPrintReportPassStart(GET_RAW_RDS_START_NORMAL, mode, "Start RDS Raw normal mode datapath");
	if ((ret = etal_start_RDS(*hReceiver, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintNormal("etal_start_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
	}

	OSAL_s32ThreadWait(duration);

	/*
	 * compare RDS data
	 */
	if (RdsRawCbInvocations < ETAL_TEST_GET_RDS_MIN)
	{
		pass = FALSE;
	}

	if (etalTestGetRDSRawCompare(&rdsRawDatapathData, &etalTestRDSRawReference, FALSE) != OSAL_OK)
	{
		pass = FALSE;
	}

	if (!pass)
	{
		etalTestPrintReportPassEnd(testFailed);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	if (!pass)
	{
		*pass_out = FALSE;
		return OSAL_OK;
	}

	/*
	 * stop datapath
	 */
	if ((ret = etal_stop_RDS(*hReceiver)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_stop_RDS for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintNormal("= complete, %d RDS Raw callback invocations", RdsRawCbInvocations);
	etalTestPrintNormal("= (expected more than %d)", ETAL_TEST_GET_RDS_MIN);

	if ((ret = etal_destroy_datapath(&hDatapath)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_destroy_datapath for %s (%s, %d)", standard, ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_GETRAWRDS_DATAPATH
#endif // CONFIG_APP_TEST_GETRDS || CONFIG_APP_TEST_GETRAWRDS_DATAPATH

/***************************
 *
 * etalTestForegroundBackgroundFM
 *
 **************************/
#if defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_APP_TEST_DAB) // otherwise FE1 is dedicated to DAB and not available for BG
#if defined CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL
#ifdef CONFIG_APP_TEST_GETRDS_DATAPATH
static tSInt etalTestForegroundBackgroundFM(tBool *pass)
{
	EtalRDSData rdsfm, rdsfm2;
	ETAL_STATUS ret;
	tBool pass9 = TRUE;
	tBool pass10 = TRUE;

    etalTestPrintNormal("");
    etalTestPrintNormal("GetRDS for FM & FM2");

	/*
	 * test RDS foreground and background without datapath
	 */
    OSAL_pvMemorySet(&rdsfm, 0x00, sizeof(EtalRDSData));
    OSAL_pvMemorySet(&rdsfm2, 0x00, sizeof(EtalRDSData));
    
	/*
	 * tune FM foreground
	 */
    if (etalTestDoRDSConfig(&handlefm, FALSE, ETAL_TEST_MODE_FM, ETAL_INVALID_HANDLE) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (etalTestDoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
	/*
	 * tune FM background
	 */
    if (etalTestDoRDSConfig(&handlefm2, FALSE, ETAL_TEST_MODE_FM, ETAL_FE_FOR_FM_TEST2) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (handlefm2 != ETAL_INVALID_HANDLE)
    {
        etalTestPrintNormal("* Tune to FM2 freq %d", ETAL_VALID_FM_FREQ2);
        if ((ret = etal_tune_receiver(handlefm2, ETAL_VALID_FM_FREQ2)) != ETAL_RET_SUCCESS)
        {
            etalTestPrintError("etal_tune_receiver FM2 (%s, %d)", ETAL_STATUS_toString(ret), ret);
            return OSAL_ERROR;
        }
    }
	/*
	 * start RDS foreground
	 */
    etalTestPrintNormal("* Start RDS for FM");
    if ((ret = etal_start_RDS(handlefm, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etal_start_RDS for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
	/*
	 * start RDS background
	 */
    etalTestPrintNormal("* Start RDS for FM2");
    if ((ret = etal_start_RDS(handlefm2, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etal_start_RDS for FM2 (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

	/*
	 * wait for RDS reception
	 */
    etalTestPrintNormal("Wait %ds to aquire RDS", ETAL_TEST_GETRDS_FM_DURATION / ETAL_TEST_ONE_SECOND);
    OSAL_s32ThreadWait(ETAL_TEST_GETRDS_FM_DURATION);

	/*
	 * get decoded RDS foreground
	 */
    if ((ret = etaltml_get_decoded_RDS(handlefm, &rdsfm)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_get_decoded_RDS for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    etalTestPrintNormal("");
    etalTestPrintNormal("RDS for FM foreground");
    etalTestPrintRDS(&rdsfm, "FG ");
    pass9 = etalTestGetRDSCompare(&rdsfm, &etalTestRDSReference);

	/*
	 * get decoded RDS background
	 */
    if ((ret = etaltml_get_decoded_RDS(handlefm2, &rdsfm2)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_get_decoded_RDS for FM2 (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }
    etalTestPrintNormal("");
    etalTestPrintNormal("RDS for FM2 background");
    etalTestPrintRDS(&rdsfm2, "BG ");
    pass9 &= etalTestGetRDSCompare(&rdsfm2, &etalTestRDSReference2);

    if (!pass9)
    {
        etalTestPrintNormal("pass9 FAILED");
    }

	/*
	 * stop RDS foreground
	 */
    etalTestPrintNormal("* Stop RDS for FM");
    if ((ret = etal_stop_RDS(handlefm)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_stop_RDS for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

	/*
	 * stop RDS background
	 */
    etalTestPrintNormal("* Stop RDS for FM2");
    if ((ret = etal_stop_RDS(handlefm2)) != ETAL_RET_SUCCESS)
    {
        etalTestPrintError("etaltml_stop_RDS for FM2 (%s, %d)", ETAL_STATUS_toString(ret), ret);
        return OSAL_ERROR;
    }

    if (etalTestUndoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (etalTestUndoTuneSingle(ETAL_TUNE_FM, handlefm2) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (etalTestUndoRDSConfig(&handlefm) != OSAL_OK)
    {
        return OSAL_ERROR;
    }
    if (etalTestUndoRDSConfig(&handlefm2) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

	/*
	 * test RDS foreground and background with datapath
	 */
    if (etalTestGetRDSDatapath2(&handlefm, ETAL_TEST_MODE_FM, &handlefm2, ETAL_TEST_MODE_FM, &pass10)!= OSAL_OK)
    {
        return OSAL_ERROR;
	}

    if (!pass10)
    {
        etalTestPrintNormal("pass10 FAILED");
    }
	*pass = pass9 && pass10;
	return OSAL_OK;
}
#endif // CONFIG_APP_TEST_GETRDS_DATAPATH
#endif // CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL
#endif // CONFIG_APP_TEST_FM

/***************************
 *
 * etalTestGetRDS
 *
 **************************/
tSInt etalTestGetRDS(void)
{
#ifdef CONFIG_APP_TEST_GETRDS
#ifdef CONFIG_APP_TEST_FM
	EtalRDSData rdsfm;
#endif
#ifdef CONFIG_APP_TEST_HDRADIO_FM
	EtalRDSData rdshd;
#endif
#if defined(CONFIG_APP_TEST_FM) || defined(CONFIG_APP_TEST_HDRADIO_FM)
	ETAL_STATUS ret;
#endif // CONFIG_APP_TEST_FM || CONFIG_APP_TEST_HDRADIO_FM
	tBool pass1 = TRUE;
	tBool pass2 = TRUE;
	tBool pass3 = TRUE;
	tBool pass4 = TRUE;
	tBool pass5 = TRUE;
	tBool pass6 = TRUE;

	tBool pass7 = TRUE;
	tBool pass8 = TRUE;

	tBool pass_ex = TRUE;	// 9 & 10
	tBool pass11 = TRUE;

	etalTestStartup();

#ifdef CONFIG_APP_TEST_FM

	OSAL_pvMemorySet(&rdsfm, 0x00, sizeof(EtalRDSData));

	if (etalTestDoRDSConfig(&handlefm, FALSE, ETAL_TEST_MODE_FM, ETAL_INVALID_HANDLE) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/*
	 * start RDS
	 */
	etalTestPrintNormal("* Start RDS for FM");
	if ((ret = etal_start_RDS(handlefm, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_start_RDS for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/*
	 * wait for RDS reception
	 */
	etalTestPrintNormal("");
	etalTestPrintNormal("GetRDS for FM");
	etalTestPrintNormal("Testing for %dms", ETAL_TEST_GETRDS_FM_DURATION);
    OSAL_s32ThreadWait(ETAL_TEST_GETRDS_FM_DURATION);

	/*
	 * get decoded RDS
	 */
	if ((ret = etaltml_get_decoded_RDS(handlefm, &rdsfm)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_get_decoded_RDS for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintRDS(&rdsfm, "FG ");
	pass1 = etalTestGetRDSCompare(&rdsfm, &etalTestRDSReference);
	if (!pass1)
	{
		etalTestPrintNormal("pass1 FAILED");
	}
	/*
	 * stop RDS
	 */
	etalTestPrintNormal("* Stop RDS for FM");
	if ((ret = etal_stop_RDS(handlefm)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_stop_RDS for FM (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}


#ifdef CONFIG_APP_TEST_GETRDS_DATAPATH
	if (etalTestGetRDSDatapath(handlefm, ETAL_TEST_MODE_FM, &pass2) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass2)
	{
		etalTestPrintNormal("pass2 FAILED");
	}
#endif // CONFIG_APP_TEST_GETRDS_DATAPATH

	etalTestPrintNormal("");
	if (etalTestUndoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoRDSConfig(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_FM

#ifdef CONFIG_APP_TEST_HDRADIO_FM
	if (ETAL_FE_FOR_HD_TEST == ETAL_FE_HANDLE_1)
	{
	if (etalTestDoRDSConfig(&handlehd, FALSE, ETAL_TEST_MODE_HD_FM, ETAL_INVALID_HANDLE) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/*
	 * start RDS
	 */
	etalTestPrintNormal("* Start RDS for HD");
	if ((ret = etal_start_RDS(handlehd, FALSE, 0, ETAL_RDS_MODE)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_start_RDS for HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}

	/*
	 * wait for RDS reception
	 */
	etalTestPrintNormal("");
	etalTestPrintNormal("GetRDS for HD");
	etalTestPrintNormal("Testing for %dms", ETAL_TEST_GETRDS_HD_DURATION);
    OSAL_s32ThreadWait(ETAL_TEST_GETRDS_HD_DURATION);
	/*
	 * get decoded RDS
	 */
	if ((ret = etaltml_get_decoded_RDS(handlehd, &rdshd)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etaltml_get_decoded_RDS for HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
	etalTestPrintRDS(&rdshd, "FG ");
    // TODO: check HD RDS with a reference
    //pass5 = etalTestGetRDSCompare(&rdshd, &etalTestRDSHDReference);
	//if (!pass5)
	//{
	//	etalTestPrintNormal("pass5 FAILED");
	//}
	/*
	 * stop RDS
	 */
	etalTestPrintNormal("* Stop RDS for HD");
	if ((ret = etal_stop_RDS(handlehd)) != ETAL_RET_SUCCESS)
	{
		etalTestPrintError("etal_stop_RDS for HD (%s, %d)", ETAL_STATUS_toString(ret), ret);
		return OSAL_ERROR;
	}
#ifdef CONFIG_APP_TEST_GETRDS_DATAPATH
	if (etalTestGetRDSDatapath(handlehd, ETAL_TEST_MODE_HD_FM, &pass5) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass5)
	{
		etalTestPrintNormal("pass5 FAILED");
	}
	/*
	 * repeat the datapath test but tuning the HD receiver to an FM station
	 */
	if (etalTestGetRDSDatapath(handlehd, ETAL_TEST_MODE_HD_FM, &pass6) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass6)
	{
		etalTestPrintNormal("pass6 FAILED");
	}

#endif // CONFIG_APP_TEST_GETRDS_DATAPATH
	if (etalTestUndoTuneSingle(ETAL_TUNE_HDRADIO_FM, handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoRDSConfig(&handlehd) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	}
	else
	{
		etalTestPrintNormal("Skipping HD RDS test");
	}
#endif // CONFIG_APP_TEST_HDRADIO_FM

#ifdef CONFIG_APP_TEST_GETRDS_DATAPATH
#ifdef CONFIG_APP_TEST_FM
	if (etalTestGetRDSDatapathChangeFreq(&handlefm, ETAL_TEST_MODE_FM, &pass7) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass7)
	{
		etalTestPrintNormal("pass7 FAILED");
	}
    if (etalTestGetRDSDatapathChangeFreqNoRDS(&handlefm, ETAL_TEST_MODE_FM, &pass8) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (!pass8)
	{
		etalTestPrintNormal("pass8 FAILED");
	}
    /* delete semaphore no more used fo tests */
    if (etalTestRDSSeekFinishSem != 0)
    {
        if (OSAL_s32SemaphoreClose(etalTestRDSSeekFinishSem) != OSAL_OK)
        {
            etalTestRDSSeekFinishSem = 0;
            ret = OSAL_ERROR;
        }
        if (OSAL_s32SemaphoreDelete("Sem_etalTestRDSSeekFinish") != OSAL_OK)
        {
            etalTestRDSSeekFinishSem = 0;
            return OSAL_ERROR;
        }
        etalTestRDSSeekFinishSem = 0;
    }
#endif // CONFIG_APP_TEST_FM
#endif // CONFIG_APP_TEST_GETRDS_DATAPATH

#if defined (CONFIG_APP_TEST_FM) && !defined (CONFIG_APP_TEST_DAB) // otherwise FE1 is dedicated to DAB and not available for BG
#if defined CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL
#ifdef CONFIG_APP_TEST_GETRDS_DATAPATH
    // tests 9 and 10
	if (etalTestForegroundBackgroundFM(&pass_ex) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_GETRDS_DATAPATH
#endif // CONFIG_ETAL_SUPPORT_CMOST_DUAL_CHANNEL
#endif // CONFIG_APP_TEST_FM

	if (etalTestRDSInvalidParam(&pass11) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (!pass1 || !pass2 || !pass3 || !pass4 || !pass5 || !pass6 || !pass7 || !pass8 || !pass_ex || !pass11)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_GETRDS

	return OSAL_OK;
}

/***************************
 *
 * etalTestGetRAWRDS
 *
 **************************/
tSInt etalTestGetRAWRDS(void)
{
#ifdef CONFIG_APP_TEST_GETRAWRDS_DATAPATH
	tBool pass;

	etalTestStartup();
	pass = TRUE;

	if (etalTestDoRDSConfig(&handlefm, FALSE, ETAL_TEST_MODE_FM, ETAL_INVALID_HANDLE) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestDoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/* start the RAW RDS test */

    if (etalTestGetRDSRawDatapath(&handlefm, ETAL_TEST_MODE_FM, &pass) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

	/* start the RAW RDS fast PI test */

    if (etalTestGetRDSRawFastPIDatapath(&handlefm, ETAL_TEST_MODE_FM, &pass) != OSAL_OK)
    {
        return OSAL_ERROR;
    }

	/* end the RAW RDS test */

	if (etalTestUndoTuneSingle(ETAL_TUNE_FM, handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	if (etalTestUndoRDSConfig(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_GETRAWRDS_DATAPATH
	return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

