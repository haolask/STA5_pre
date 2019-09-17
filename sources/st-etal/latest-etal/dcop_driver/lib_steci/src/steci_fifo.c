//!
//!  \file 		steci_fifo.c
//!  \brief 	<i><b> STECI fifo </b></i>
//!  \details   Implementation of the FIFO used by STECI to forward
//!             the messages received to the ETAL
//!  \author 	Raffaele Belardi
//!
/*
 * Most of the times messages received by STECI will be short (3 bytes) notifications or
 * responses. Sometimes the responses may be longer. To reduce the storage
 * requirements we use a FIFO capable of holding messages of two sizes,
 * small (many messages) and large (one or two messages). Large messages
 * will need more time to be transmitted over the STECI so ETAL should be able
 * to free the large message(s) FIFO locations before a new large message
 * arrives. Thus we can keep the large message number small. Well, in
 * theory, at least...
 *
 * UPDATE 4 Sep 2015: RAW data transfer use packets which are neither small
 * nor large (max 96 bytes) so we add another fifo
 * 
 */
#include "target_config.h"

#if (defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_ETAL_SUPPORT_DCOP_MDR))

#include "osal.h"

#include "steci_defines.h"
#include "steci_helpers.h"
#include "common_fifo.h"
#include "common_trace.h"
#include "steci_trace.h"

/***************************
 *
 * Local macros
 *
 **************************/
/*
 * To add a new FIFO size:
 * 1. #define a new STECI_FIFO_XXX_MESSAGE_SIZE and STECI_FIFO_XXX_MESSAGE_MAX
 * 2. increment STECI_MAX_FIFO_TYPES
 * 3. update STECI_FIFO_MESSAGE_MAX_PER_FIFO if necessary
 * 4. update STECI_FIFO_MESSAGE_MAX
 * 5. define a new array to store the FIFO messages (STECI_fifoXXXMsg)
 * 6. add a new entry to STECI_fifoDescr with the info in 1 and 5, maintaining the sort order
 */

/*
 * STECI_MAX_FIFO_TYPES
 *
 * How many FIFOs we want to support
 */
#define STECI_MAX_FIFO_TYPES             5
/*
 * STECI_FIFO_XXX_MESSAGE_MAX
 *
 * Max size in bytes of the message that will be stored in each FIFO
 */
#define STECI_FIFO_VLARGE_MESSAGE_SIZE   65535
/* Above should be aligned to DCOP side : DCOP message could be sent in max 16 fragments of 4K (including payload).
     And without exceeding u16 type size, due to member maxElemSize of struct STECI_fifoDescrTy */
//#define STECI_FIFO_VLARGE_MESSAGE_SIZE   STECI_MAX_PAYLOAD_LENGTH
#define STECI_FIFO_XLARGE_MESSAGE_SIZE   4096 // Max STECI chunk size
#define STECI_FIFO_LARGE_MESSAGE_SIZE    (1500 + 40) // audio frames
#define STECI_FIFO_MEDIUM_MESSAGE_SIZE   (96 + 8) // MDR Data Protocol frames for RAW (undecoded) format: 96 is the max network packet size, 8 is the overhead added by MDR data protocol
#define STECI_FIFO_SMALL_MESSAGE_SIZE    12 // autonotifications for seek/scan/learn are common and 12 bytes long
/*
 * STECI_FIFO_XXX_MESSAGE_MAX
 *
 * Max number of messages that each FIFO shall support
 * WARNING: remember to update also the next macro
 */
#define STECI_FIFO_VLARGE_MESSAGE_MAX    1
#define STECI_FIFO_XLARGE_MESSAGE_MAX    4
//#define STECI_FIFO_LARGE_MESSAGE_MAX     4
#define STECI_FIFO_LARGE_MESSAGE_MAX     20
//#define STECI_FIFO_MEDIUM_MESSAGE_MAX    6 
#define STECI_FIFO_MEDIUM_MESSAGE_MAX    20 
//#define STECI_FIFO_SMALL_MESSAGE_MAX     4
#define STECI_FIFO_SMALL_MESSAGE_MAX     10

/*
 * STECI_FIFO_MESSAGE_MAX_PER_FIFO
 *
 * Max of all STECI_FIFO_XXX_MESSAGE_MAX
 */
#define STECI_FIFO_MESSAGE_MAX_PER_FIFO 20
/*
 * STECI_FIFO_MESSAGE_MAX
 *
 * sum of all STECI_FIFO_XXX_MESSAGE_MAX
 */
#define STECI_FIFO_MESSAGE_MAX          (STECI_FIFO_VLARGE_MESSAGE_MAX + STECI_FIFO_XLARGE_MESSAGE_MAX + STECI_FIFO_LARGE_MESSAGE_MAX + STECI_FIFO_MEDIUM_MESSAGE_MAX + STECI_FIFO_SMALL_MESSAGE_MAX)

/*
 * STECI_PROFILE_FIFO_USAGE
 *
 * Include a counter to check how many times each FIFO size is used
 * Default should be #undef
 */
//#define STECI_PROFILE_FIFO_USAGE
#undef STECI_PROFILE_FIFO_USAGE

#ifndef CONFIG_DEBUG_SYMBOLS
	#undef STECI_PROFILE_FIFO_USAGE
#endif

/*
 * sanity checks
 */
#if (STECI_FIFO_MESSAGE_MAX_PER_FIFO < STECI_FIFO_VLARGE_MESSAGE_MAX) || \
	(STECI_FIFO_MESSAGE_MAX_PER_FIFO < STECI_FIFO_XLARGE_MESSAGE_MAX) || \
	(STECI_FIFO_MESSAGE_MAX_PER_FIFO < STECI_FIFO_LARGE_MESSAGE_MAX) || \
	(STECI_FIFO_MESSAGE_MAX_PER_FIFO < STECI_FIFO_MEDIUM_MESSAGE_MAX) || \
	(STECI_FIFO_MESSAGE_MAX_PER_FIFO < STECI_FIFO_SMALL_MESSAGE_MAX)
	#error "Adjust STECI_FIFO_MESSAGE_MAX_PER_FIFO"
#endif

#define STECI_FIFO_NAME               "STECI"

/***************************
 *
 * Local types
 *
 **************************/
typedef struct
{
	tU16 len;
	tU8  lun;
	tU8 *buf;
} STECI_fifoMsgElemTy;

typedef struct
{
	COMMON_fifoStatusTy steciFifo;
	STECI_fifoMsgElemTy steciFifoStorage[STECI_FIFO_MESSAGE_MAX]; 
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
	SteciPushNotifyCallback p_steciFifoPushNotify;
#endif
} STECI_fifoTy;

typedef struct
{
	tU16   maxElemSize;
	tU8    maxElemCount;
	tU8    elemStatusFlag[STECI_FIFO_MESSAGE_MAX_PER_FIFO];
	tU8   *elemArray;
} STECI_fifoDescrTy;

#ifdef STECI_PROFILE_FIFO_USAGE
tS32   STECI_maxDepth[STECI_MAX_FIFO_TYPES] = {-1, -1, -1, -1, -1};
#endif

/***************************
 *
 * Local variables
 *
 **************************/
static tU8 STECI_fifoSmallMsg [STECI_FIFO_SMALL_MESSAGE_MAX] [STECI_FIFO_SMALL_MESSAGE_SIZE];
static tU8 STECI_fifoMediumMsg[STECI_FIFO_MEDIUM_MESSAGE_MAX][STECI_FIFO_MEDIUM_MESSAGE_SIZE];
static tU8 STECI_fifoLargeMsg [STECI_FIFO_LARGE_MESSAGE_MAX] [STECI_FIFO_LARGE_MESSAGE_SIZE];
static tU8 STECI_fifoXLargeMsg[STECI_FIFO_XLARGE_MESSAGE_MAX] [STECI_FIFO_XLARGE_MESSAGE_SIZE];
static tU8 STECI_fifoVLargeMsg[STECI_FIFO_VLARGE_MESSAGE_MAX] [STECI_FIFO_VLARGE_MESSAGE_SIZE];

static STECI_fifoTy      STECI_fifo;
/*
 * Fifo descriptions: MUST BE ORDERED FROM SMALLER MESSAGE TO LARGER MESSAGE
 */
static STECI_fifoDescrTy STECI_fifoDescr[STECI_MAX_FIFO_TYPES] =
{
 {STECI_FIFO_SMALL_MESSAGE_SIZE,  STECI_FIFO_SMALL_MESSAGE_MAX,  {0,0,0,0,0,0}, (tU8 *)STECI_fifoSmallMsg},
 {STECI_FIFO_MEDIUM_MESSAGE_SIZE, STECI_FIFO_MEDIUM_MESSAGE_MAX, {0,0,0,0,0,0}, (tU8 *)STECI_fifoMediumMsg},
 {STECI_FIFO_LARGE_MESSAGE_SIZE,  STECI_FIFO_LARGE_MESSAGE_MAX,  {0,0,0,0,0,0}, (tU8 *)STECI_fifoLargeMsg},
 {STECI_FIFO_XLARGE_MESSAGE_SIZE, STECI_FIFO_XLARGE_MESSAGE_MAX, {0,0,0,0,0,0}, (tU8 *)STECI_fifoXLargeMsg},
 {STECI_FIFO_VLARGE_MESSAGE_SIZE, STECI_FIFO_VLARGE_MESSAGE_MAX, {0,0,0,0,0,0}, (tU8 *)STECI_fifoVLargeMsg}
};

/***************************
 *
 * Local functions
 *
 **************************/
static tSInt STECI_fifoReset(STECI_fifoTy *f);
static tU8 *STECI_fifoAllocateAndCopy(tU8 *buf, tU32 len);
static tVoid STECI_fifoFree(tU8 *buf, tU32 len);



/***************************
 *
 * STECI_fifoReset
 *
 **************************/
static tSInt STECI_fifoReset(STECI_fifoTy *f)
{
	OSAL_pvMemorySet((tVoid *)f->steciFifoStorage, 0x00, sizeof(STECI_fifoMsgElemTy) * STECI_FIFO_MESSAGE_MAX);
	if (COMMON_fifoInit(&f->steciFifo, STECI_FIFO_NAME, STECI_FIFO_MESSAGE_MAX) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	return OSAL_OK;
}

/***************************
 *
 * STECI_fifoInit
 *
 **************************/
tSInt STECI_fifoInit(SteciPushNotifyCallback pI_Callback)
{
	STECI_fifoTy *f = &STECI_fifo;
	tU32 i;

	if (STECI_fifoReset(f) != OSAL_OK)
	{
		return OSAL_ERROR;
	}
	for (i = 0; i < STECI_MAX_FIFO_TYPES; i++)
	{
		OSAL_pvMemorySet((tVoid *)STECI_fifoDescr[i].elemStatusFlag, 0x00, sizeof(STECI_fifoDescr[i].elemStatusFlag));
	}
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
	f->p_steciFifoPushNotify = pI_Callback;
#else
	(tVoid) pI_Callback;
#endif

	return OSAL_OK;
}

/***************************
 *
 * STECI_fifoDeinit
 *
 **************************/
tVoid STECI_fifoDeinit(tVoid)
{
	STECI_fifoTy *f = &STECI_fifo;

	COMMON_fifoDeinit(&f->steciFifo, STECI_FIFO_NAME);
}

 
/***************************
 *
 * STECI_fifoAllocateAndCopy
 *
 **************************/
/*
 * Walk through the configured fifo sizes and check
 * if there is free space in the fifo capable of holding
 * <len> bytes; if so copy the buf and return a pointer to it
 */
static tU8 *STECI_fifoAllocateAndCopy(tU8 *buf, tU32 len)
{
	STECI_fifoDescrTy *ds;
	tU8 *fifo_buf;
	tU32 i, j;

	for (i = 0; i <	STECI_MAX_FIFO_TYPES; i++)
	{
		ds = &STECI_fifoDescr[i];
		if ((tU32)ds->maxElemSize >= len)
		{
			for (j = 0; j < (tU32)ds->maxElemCount; j++)
			{
				if (ds->elemStatusFlag[j] == (tU8)0)
				{
					ds->elemStatusFlag[j] = (tU8)1;
#ifdef STECI_PROFILE_FIFO_USAGE
					if (STECI_maxDepth[i] < (tS32)j)
					{
						STECI_maxDepth[i] = (tS32)j;
					}
#endif
					fifo_buf = ds->elemArray + (j * sizeof(tU8) * ds->maxElemSize);
					OSAL_pvMemoryCopy((tVoid *)fifo_buf, (tPCVoid)buf, len);
					STECI_tracePrintComponent(TR_CLASS_STECI, "STECI_fifoAllocateAndCopy : allocated in fifo %d, len %d, element %d\n", i, len, j);
					return fifo_buf;
				}
			}
			/*
			 * This means we did not find an available FIFO able to hold required len
			 * So probably need to increase the number of FIFO of that size 
			 * For the record, print that current FIFO is FULL.
			 * there is no need to assert or block here
			 * it will try to find available room in bigger sized FIFOs (if any)
			 */
			STECI_tracePrintError(TR_CLASS_STECI, "STECI_fifoAllocateAndCopy, fifo full, req len =%d, fifo index %d, fifo size %d",
					len, i, ds->maxElemCount); 
			//dump the context
			for (j = 0; j < (tU32)ds->maxElemCount; j++)
			{
				fifo_buf = ds->elemArray + (j * sizeof(tU8) * ds->maxElemSize);
				STECI_tracePrintError(TR_CLASS_STECI,"STECI_fifoAllocateAndCopy, Buf [%d] content ----->", j);
				STECI_tracePrintBufError(TR_CLASS_STECI, fifo_buf, ds->maxElemSize, NULL);
			}
		}
	}
	/*
	 * message larger than max available
	 */
	STECI_tracePrintError(TR_CLASS_STECI, "STECI_fifoAllocateAndCopy, message larger than max available, req len =  %d", len); 
	ASSERT_ON_DEBUGGING(0); // TODO remove
	return NULL;
}

/***************************
 *
 * STECI_fifoFree
 *
 **************************/
/*
 * Search the buf in one of the fifo types (using len to understand
 * which fifo is the right one) and set its status flag to free.
 */
static tVoid STECI_fifoFree(tU8 *buf, tU32 len)
{
	STECI_fifoDescrTy *ds;
	tU8 *fifo_buf;
	tU32 i, j;

	if (buf == NULL)
	{
		/* might happen if STECI_fifoAllocateAndCopy was not able to allocate */
		return;
	}

	for (i = 0; i <	STECI_MAX_FIFO_TYPES; i++)
	{
		ds = &STECI_fifoDescr[i];
		if ((tU32)ds->maxElemSize >= len)
		{
			for (j = 0; j < (tU32)ds->maxElemCount; j++)
			{
				fifo_buf = ds->elemArray + (j * sizeof(tU8) * ds->maxElemSize);
				if (fifo_buf == buf)
				{
					if (ds->elemStatusFlag[j] == (tU8)1)
					{
						STECI_tracePrintComponent(TR_CLASS_STECI, "STECI_fifoFree : allocated in fifo %d, len %d, element %d\n", i, len, j);

						ds->elemStatusFlag[j] = (tU8)0;
						return;
					}
					else
					{
						/*
						 * we found the fifo buffer but it is already free,
						 * it's what we wanted but it should not happen
						 */
						ASSERT_ON_DEBUGGING(0);
						break;
					}
				}
			}
			/*
			 * buffer not found ; let's search in bigger sized FIFOs
			 */
		}
	}
	/*
	 * probably corrupt len field
	 */
	ASSERT_ON_DEBUGGING(0);
}

/***************************
 *
 * STECI_fifoPush
 *
 **************************/
tVoid STECI_fifoPush(tU8 *buf, tU32 len, tU8 lun)
{
	STECI_fifoTy *f = &STECI_fifo;
	STECI_fifoMsgElemTy *el;
	tU8 *fifo_buf;
	tS16 new_write_ptr;

	COMMON_fifoGetLock(&f->steciFifo);

	if (COMMON_fifoPush(&f->steciFifo, &new_write_ptr) != OSAL_OK)
	{
		// FIFO overflow
	    STECI_tracePrintSystem(TR_CLASS_STECI, "FIFO overflow");
		COMMON_fifoReleaseLock(&f->steciFifo);
		return;
	}

	if ((fifo_buf = STECI_fifoAllocateAndCopy(buf, len)) != NULL)
	{
		el = &f->steciFifoStorage[new_write_ptr];
		el->len = (tU16)len;
		el->lun = lun;
		el->buf = fifo_buf;

// TODO don't commit to trunk!
#undef DATASERVICE_BYPASS
#ifdef DATASERVICE_BYPASS
		if (len > 100)
		{
			FILE *fp;
			static tChar name[] = "0";

			fp = fopen(name, "w");
			if (fp)
			{
				if (fwrite(fifo_buf, 1, len, fp) != len)
				{
					STECI_tracePrintError(TR_CLASS_STECI, "truncated write");
				}
				fclose(fp);
				name[0]++;
			}
			else
			{
				STECI_tracePrintError(TR_CLASS_STECI, "file open");
			}
		}
#endif
	}
	else
	{
		// run out of buffers, should not happen;
		// do nothing since there are asserts in STECI_fifoAllocateAndCopy to catch this problem
	}
	COMMON_fifoReleaseLock(&f->steciFifo);

	// notify data are available
	//
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
	
	STECI_tracePrintComponent(TR_CLASS_STECI, "STECI_fifoPush Msg Available");

	f->p_steciFifoPushNotify();
#endif 

}

/***************************
 *
 * STECI_fifoPop
 *
 **************************/
tS32 STECI_fifoPop(tU8 *buf, tU32 max_buf, tU32 *len, tU8 *lun)
{
    STECI_fifoTy *f = &STECI_fifo;
    STECI_fifoMsgElemTy *el;
    tSInt ret = OSAL_OK;
    tS16 new_read_ptr;

    COMMON_fifoGetLock(&f->steciFifo);

    if ((ret = COMMON_fifoPop(&f->steciFifo, &new_read_ptr)) != OSAL_OK)
    {
        if((ret == OSAL_ERROR) && (f->steciFifo.readPtr == f->steciFifo.writePtr))
        {
            COMMON_fifoReleaseLock(&f->steciFifo);
            //FIFO queue is empty
            *len = 0;
            *lun = f->steciFifoStorage[new_read_ptr].lun;
            return OSAL_OK;
        }
        else
        {
            COMMON_fifoReleaseLock(&f->steciFifo);
            return OSAL_ERROR;
        }
    }

    el = &f->steciFifoStorage[new_read_ptr];
    if ((tU32)el->len > max_buf)
    {
        // received a message that would overflow the caller's buffer, abort.
        // solution: increase max_buf
        ASSERT_ON_DEBUGGING(0);
        ret = OSAL_ERROR;
    }
    else
    {
        *len = (tU32)el->len;
        *lun = el->lun;
        OSAL_pvMemoryCopy((tVoid *)buf, (tPCVoid)el->buf, el->len);
    }
    STECI_fifoFree(el->buf, el->len);
    OSAL_pvMemorySet((tVoid *)el, 0x00, sizeof(STECI_fifoMsgElemTy));

    COMMON_fifoReleaseLock(&f->steciFifo);
    return ret;
}
#endif // #if (defined (CONFIG_COMM_DRIVER_EMBEDDED) && defined (CONFIG_ETAL_SUPPORT_DCOP_MDR))
// End of file
