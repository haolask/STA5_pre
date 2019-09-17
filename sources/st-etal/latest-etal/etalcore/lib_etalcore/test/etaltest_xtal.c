//!
//!  \file 		etaltest_xtal.c
//!  \brief 	<i><b> ETAL test, XTAL alignment </b></i>
//!  \details   Simulates the ETAL User application
//!  \author 	Raffaele Belardi
//!

#include "osal.h"

#ifdef CONFIG_APP_ETAL_TEST
#include "etal_api.h"
#include "etaltest.h"

#ifdef CONFIG_APP_TEST_XTAL_ALIGNMENT
typedef enum {
	XTAL_ALIGNMENT_MEASURE = 1,
	XTAL_ALIGNMENT_INVALID_PARAM,
	XTAL_ALIGNEMENT_INVALID_RCV,
	XTAL_ALIGNMENT_NON_FM_RCV
} XtalAlignmentTestTy;
#endif

/***************************
 * function prototypes
 **************************/

#ifdef CONFIG_APP_TEST_XTAL_ALIGNMENT
/***************************
 *
 * etalTestXTALalignmentRead
 *
 **************************/
tSInt etalTestReadXTALalignmentRead(tU32 *calculatedAlignment)
{
	tSInt ret = OSAL_OK;
	FILE *fp;

	if ((fp = fopen(ETAL_TEST_XTAL_ALIGN_VALUE_FILENAME, "r")) == NULL)
	{
		etalTestPrintError("Error opening ETAL alignment file \"%s\" for reading (%s)", ETAL_TEST_XTAL_ALIGN_VALUE_FILENAME, strerror(errno));
		return OSAL_ERROR;
	}
	if (fscanf(fp, "0x%x", calculatedAlignment) != 1)
	{
		etalTestPrintError("Error reading ETAL alignment value (%s)", strerror(errno));
		ret = OSAL_ERROR;
	}
	else
	{
		etalTestPrintNormal("Read ETAL alignment value (0x%x) from %s", *calculatedAlignment, ETAL_TEST_XTAL_ALIGN_VALUE_FILENAME);
	}
	fclose(fp);
	return ret;
}


#ifdef CONFIG_APP_TEST_FM
/***************************
 *
 * etalTestXTALalignmentWrite
 *
 **************************/
static tSInt etalTestReadXTALalignmentWrite(tU32 calculatedAlignment)
{
	tSInt ret = OSAL_OK;
	FILE *fp;
	tChar str[10];

	if ((fp = fopen(ETAL_TEST_XTAL_ALIGN_VALUE_FILENAME, "w")) == NULL)
	{
		etalTestPrintNormal("Error opening ETAL alignment file for writing (%s)", ETAL_TEST_XTAL_ALIGN_VALUE_FILENAME);
		return OSAL_ERROR;
	}
	sprintf(str, "0x%x", calculatedAlignment);
	if (fputs(str, fp) == EOF)
	{
		etalTestPrintNormal("Error writing ETAL alignment value (%s)", strerror(errno));
		ret = OSAL_ERROR;
	}
	fclose(fp);
	return ret;
}
#endif // CONFIG_APP_TEST_FM
#endif // CONFIG_APP_TEST_XTAL_ALIGNMENT

/***************************
 *
 * etalTestXTALalignment
 *
 **************************/
tSInt etalTestXTALalignment(void)
{
#ifdef CONFIG_APP_TEST_XTAL_ALIGNMENT
#if defined (CONFIG_APP_TEST_FM) || defined (CONFIG_APP_TEST_DAB)
	tSInt ret;
	tU32 calculatedAlignment;
#endif
	tBool pass;

	etalTestStartup();
	pass = TRUE;

#if defined (CONFIG_APP_TEST_FM)
	/* pass1, invoke the measure algo with correct paramters */

	/*
	 * The initialization already performed by etalTestInitialize
	 * used default EtalHardwareAttr with no XTAL configuration enabled.
	 * Now we can measure the correction and re-initialize ETAL
	 * with the measured value
	 */

	/*
	 * create an FM receiver and start the measure
	 */
	if (etalTestDoConfigSingle(ETAL_CONFIG_FM1, &handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	etalTestPrintReportPassStart(XTAL_ALIGNMENT_MEASURE, ETAL_TEST_MODE_FM, "XTAL alignment measure");
	if ((ret = etal_xtal_alignment(handlefm, &calculatedAlignment)) != ETAL_RET_SUCCESS)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("etal_xtal_alignment (%s, %d)", ETAL_STATUS_toString(ret), ret);
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	
	etalTestPrintNormal("XTAL alignment measure = 0x%x", calculatedAlignment);

	etalTestPrintNormal("Save alignment to file %s", ETAL_TEST_XTAL_ALIGN_VALUE_FILENAME);
	if (etalTestReadXTALalignmentWrite(calculatedAlignment) != OSAL_OK)
	{
		return OSAL_ERROR;
	}

	/*
	 * TODO: is there a way to evaluate the returned value for correctness?
	 *       A valid range?
	 */

	/* pass2, call with invalid parameters */

	etalTestPrintReportPassStart(XTAL_ALIGNMENT_INVALID_PARAM, ETAL_TEST_MODE_FM, "XTAL alignment with invalid parameter");

	if ((ret = etal_xtal_alignment(handlefm, NULL)) != ETAL_RET_PARAMETER_ERR)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2a FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	etalTestPrintReportPassStart(XTAL_ALIGNEMENT_INVALID_RCV, ETAL_TEST_MODE_FM, "XTAL alignment with invalid receiver");
	if ((ret = etal_xtal_alignment(ETAL_INVALID_HANDLE, &calculatedAlignment)) != ETAL_RET_INVALID_RECEIVER)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2b FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}

	/* free the front-end in case of test with single channel STAR */
	if (etalTestUndoConfigSingle(&handlefm) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_FM

#if defined (CONFIG_APP_TEST_DAB)
	if (etalTestDoConfigSingle(ETAL_CONFIG_DAB, &handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	etalTestPrintReportPassStart(XTAL_ALIGNMENT_NON_FM_RCV, ETAL_TEST_MODE_DAB, "XTAL alignment with non-FM receiver");
	if ((ret = etal_xtal_alignment(handledab, &calculatedAlignment)) != ETAL_RET_INVALID_RECEIVER)
	{
		pass = FALSE;
		etalTestPrintReportPassEnd(testFailed);
		etalTestPrintNormal("pass2c FAILED");
	}
	else
	{
		etalTestPrintReportPassEnd(testPassed);
	}
	if (etalTestUndoConfigSingle(&handledab) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
#endif

#if defined (CONFIG_APP_TEST_AM)
	etalTestPrintNormal("AM support not yet available in the test");
#endif

#if defined (CONFIG_APP_TEST_HDRADIO_FM)
	etalTestPrintNormal("HDRadio support not yet available in the test");
#endif

	if (!pass)
	{
		return OSAL_ERROR;
	}
#endif // CONFIG_APP_TEST_XTAL_ALIGNMENT
	return OSAL_OK;
}
#endif // CONFIG_APP_ETAL_TEST

