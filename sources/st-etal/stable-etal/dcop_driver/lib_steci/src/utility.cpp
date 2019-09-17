//!
//!  \file 		 utility.h
//!  \brief 	 <i><b> Utility functions interface</b></i>
//!  \details Utility functions interface.
//!  \author 	Alessandro Vaghi
//!
#include "target_config.h"

#if defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_ETAL_SUPPORT_DCOP_MDR)

#include <math.h>

#include "osal.h"

#include "utility.h"

#define BYTE        tU8


// Tick-time is machine dependent 
#if (defined CONFIG_HOST_OS_WIN64_VISUALSTUDIO)
	#define UTILITY_GetCurrentTime						GetTickCount64
#elif defined (CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined (CONFIG_HOST_OS_FREERTOS)
	#define UTILITY_GetCurrentTime                      (tS64)OSAL_ClockGetElapsedTime
#else
	#define UTILITY_GetCurrentTime						GetTickCount
#endif

#if 0 // CONFIG_HOST_OS_WIN32
static tChar UTILITY_logFileName[LOG_MAX_FILE_NAME];
static tChar UTILITY_logBuffer[32768];
static FILE *UTILITY_logFilePtr;
static const tChar *UTILITY_dataChannelFileName = "DataChannel.bin";
static FILE *UTILITY_dataChannelFilePtr;
static tU32 UTILITY_logMask;
static tBool UTILITY_logFlag;
static CRITICAL_SECTION UTILITY_criticalSection;
static tBool UTILITY_initDone = false;

static FILE *UTILITY_saveDataFilePtr;
#endif

// clock leads to error in T-Kernel because not standard lib
// it should be replaces by OS_getTime or something like that
// for now, this is not used function, so put under #if 0
#if 0
tU32 UTILITY_GetProcessTimeMs (tVoid)
{
    return (tU32)clock ();
} 
#endif

tS64 UTILITY_GetTime (tVoid)
{
	return UTILITY_GetCurrentTime ();
}

tBool UTILITY_CheckTimeout (tS64 startTime, tS64 thrVal)
{
	tBool timeoutReached = false;
	tS64 endTime, elapsedTime;

	// Read system time
	endTime = UTILITY_GetCurrentTime ();
	elapsedTime = endTime - startTime;

	if (elapsedTime > thrVal)
	{
		timeoutReached = true;
	}

	return timeoutReached;
}

/* ETAL uses its own logging mechanisms so all of this is useless */
#if 0 // CONFIG_HOST_OS_WIN32
tVoid StringToUpper (tChar *StringToConvert)
{
    while (0 != *StringToConvert) 
    {
        if (*StringToConvert >= 'a' && *StringToConvert <= 'z') 
        {
            *StringToConvert = *StringToConvert + ('A' - 'a');
        } 

        StringToConvert++;
    } 
} 

tVoid StringToUpperLen (tChar *StringToConvert, tU16 BytesNum)
{
    while (0 != BytesNum--) 
    {
        if (*StringToConvert >= 'a' && *StringToConvert <= 'z') 
        {
            *StringToConvert = *StringToConvert - 'a' + 'A';
        } 

        StringToConvert++;
    } 
} 

tU32 GetBELongFromBuffer (tU8 *Buffer)
{
    tU32 LongToGet;

    LongToGet = 0;

    LongToGet = *Buffer++;
    LongToGet = (LongToGet << 8) + *Buffer++;
    LongToGet = (LongToGet << 8) + *Buffer++;
    LongToGet = (LongToGet << 8) + *Buffer;

    return LongToGet;
} 

tU16 GetBEWordFromBuffer (tU8 *Buffer)
{
    tU16 WordToGet;

    WordToGet = 0;

    WordToGet = *Buffer++;
    WordToGet = (WordToGet << 8) + *Buffer;

    return WordToGet;
}

tVoid PutBELongOnBuffer (tU8 *Buffer, tU32 LongToPut)
{
    *Buffer++ = (tU8) (LongToPut >> 24);
    *Buffer++ = (tU8) (LongToPut >> 16);
    *Buffer++ = (tU8) (LongToPut >> 8);
    *Buffer   = (tU8)LongToPut;
}

tVoid PutBEWordOnBuffer (tU8 *Buffer, tU16 WordToPut)
{
    *Buffer++ = (tU8) (WordToPut >> 8);
    *Buffer   = (tU8)WordToPut;
}

tVoid OpenSaveData (tVoid)
{
	tChar dataFileName[LOG_MAX_FILE_NAME] = "devices.txt";	

	UTILITY_saveDataFilePtr = fopen (dataFileName, "w");

	if (NULL == UTILITY_saveDataFilePtr)
	{
		printf ("\nSave data file[%s] open error", dataFileName);
	}
}

tVoid SaveData (tChar *strToLog, int length)
{
	if (NULL != UTILITY_saveDataFilePtr)
	{
		fprintf (UTILITY_saveDataFilePtr, "%s\n", strToLog);
	}
}

tVoid CloseSaveData (tVoid)
{
	fclose (UTILITY_saveDataFilePtr);
}

tVoid OpenLogSession (tVoid)
{
    char LocalLogBuffer[100];
    time_t DateAndTime;

	//Initilize the critical section
	InitializeCriticalSection (&UTILITY_criticalSection);

	// Open the log file
	if (NULL == UTILITY_logFilePtr && 1 == UTILITY_logFlag)
    {
		// Open log file in write text mode
		UTILITY_logFilePtr = fopen (UTILITY_logFileName, "w");

        if (NULL != UTILITY_logFilePtr) 
        {
            DateAndTime = time (NULL);
            sprintf (LocalLogBuffer, "MDR PROTOCOL APPLICATION - %s", ctime (&DateAndTime));
            LogString (LocalLogBuffer, LOG_MASK_INTERLAYER);
        }
        else 
        {
			printf ("\nLog file[%s] open error", UTILITY_logFileName);
        } 
    }

	// Open the data channel file
	if (NULL == UTILITY_dataChannelFilePtr && 1 == UTILITY_logFlag)
	{
		// Open data channel file in write binary mode
		UTILITY_dataChannelFilePtr = fopen (UTILITY_dataChannelFileName, "wb");
	}	
	// Set init to done
	UTILITY_initDone = true;
} 

tVoid CloseLogSession (tVoid)
{
    if (NULL != UTILITY_logFilePtr) 
    {
        fclose (UTILITY_logFilePtr);
        UTILITY_logFilePtr = NULL;
    } 

    printf ("\nLog file closed");
}

tVoid LogStringMaxSize (tChar *StringToLog, tU32 LogId, tU32 maxSize)
{
	tU32 Time;

	if (true == UTILITY_initDone)
	{
		// Lock the Critical section
		EnterCriticalSection (&UTILITY_criticalSection);

		if (0 != (LogId & UTILITY_logMask) && 0 != UTILITY_logFlag)
		{
			Time = (tU32)clock();

			printf ("T %.8u - %.4x: %*.*s\n", Time, LogId, 0, maxSize, StringToLog);

			if (NULL != UTILITY_logFilePtr) 
			{
				fprintf (UTILITY_logFilePtr, "\nT: %.8u - %.4x - %s", Time, LogId, StringToLog);
			} 
		} 

		//Release the Critical section
		LeaveCriticalSection (&UTILITY_criticalSection);
	}
}

tVoid LogString (tChar *StringToLog, tU32 LogId)
{
	LogStringMaxSize (StringToLog, LogId, INT_MAX);
}

tVoid LogStringAndFlush (tChar *StringToLog, tU32 LogId)
{
	LogStringMaxSize (StringToLog, LogId, INT_MAX);

	FixLogData ();
}

tVoid LogStringDataChannelFile (tU8 *DataToSend, tS32 BytesNum)
{
	if (true == UTILITY_initDone)
	{
		// Lock the Critical section
		EnterCriticalSection (&UTILITY_criticalSection);

		if (NULL != UTILITY_dataChannelFilePtr)
		{
			for (int cnt = 0; cnt < BytesNum; cnt++)
			{
				fprintf (UTILITY_dataChannelFilePtr, "%c", *(DataToSend + cnt));
			}
		}

		//Release the Critical section
		LeaveCriticalSection (&UTILITY_criticalSection);
	}
}

tVoid FixLogData (tVoid)
{
	if (NULL != UTILITY_logFilePtr && 0 != UTILITY_logFlag)
	{
		fflush (UTILITY_logFilePtr);
	} 
}

tChar *LogBufferPointer (tVoid)
{
	return &UTILITY_logBuffer[0];
}

tVoid SetLogFile (tChar *fileNamePtr)
{
#if (defined CONFIG_HOST_OS_WIN_VISUALSTUDIO)
	memcpy_s (UTILITY_logFileName, LOG_MAX_FILE_NAME, fileNamePtr, LOG_MAX_FILE_NAME);
#elif (defined CONFIG_HOST_OS_WIN32_MINGW) || (defined CONFIG_HOST_OS_LINUX)
	memcpy (UTILITY_logFileName, fileNamePtr, LOG_MAX_FILE_NAME);
#endif
}

tVoid SetLogMask (tU32 newMask)
{
	UTILITY_logMask = newMask;
}

tVoid SetLogFlag (tBool newLogFlag)
{
	UTILITY_logFlag = newLogFlag;
}
#endif // 0

tU32 ConvertBaseBin2UInt (tU8 *baseBin, size_t baseLen, tBool LittleEndian)
{
    tSInt i;
    tU32 baseInt = 0;

    for (i = 0; i < (tSInt)baseLen; i++)
    {
        if (LittleEndian)
        {
            baseInt += (tU32)(baseBin[baseLen - i - 1]) * ((tU32)pow ((tSDouble)256, (tSDouble)i));
        }
        else
        {
            baseInt += (tU32)(baseBin[i]) * ((tU32)pow ((tSDouble)256, (tSDouble)i));
        }
    }

    return baseInt;
}

/* converted parameter 3 type to tS32 (from size_t) because a size_t may be unsigned
 * and thus the first comparison below causes a warning */
tU32 ConvertBaseUInt2Bin (tU32 baseUInt, tU8 *baseBin, tS32 baseLen, tBool LittleEndian)
{
    tU32 i;
	tU32 baseLen_unsign;

	if (baseLen < 0)
	{
		ASSERT_ON_DEBUGGING(0);
		baseLen_unsign = 0;
	}
	else
	{
		baseLen_unsign = (tU32)baseLen;
	}
    for (i = 0; i<baseLen_unsign; i++)
    {
        if (LittleEndian)
        {
            baseBin[baseLen_unsign - i - 1] = (BYTE)(baseUInt >> (8 * i));
        }
        else
        {
            baseBin[i] = (BYTE)(baseUInt >> (8 * i));
        }
    }

    return (baseLen_unsign - i); 
}

#endif // CONFIG_COMM_DRIVER_EMBEDDED && CONFIG_ETAL_SUPPORT_DCOP_MDR

// End of file
