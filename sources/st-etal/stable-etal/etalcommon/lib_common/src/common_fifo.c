//!
//!  \file 		common_fifo.c
//!  \brief 	<i><b> Generic FIFO management </b></i>
//!  \details   Functions to manage a generic FIFO meta-data, that is the read and write
//!				pointer management.
//!				The following function only manage the FIFO metadata (read and write pointers)
//! 			The memory necessary to actually store the data is not managed here,
//! 			it must be allocated by the caller. Also it is up to the caller to copy the
//! 			object to/from the FIFO element identified by the *index* parameter in
//! 			some of the function interfaces.
//! 			The functions only provide an index to uniquely identify the fifo element.
//!				
//!				Usage
//! 			-----
//! 			Allocate or define a variable of type #COMMON_fifoStatusTy and initialize
//! 			it calling #COMMON_fifoInit. The variable will be used by
//! 			the COMMON FIFO functions as private state, it should not
//! 			be modified by the caller.
//! 			
//! 			Allocate or define an array of *fifoSize* elements, each element of the
//! 			size of the data to be managed by the FIFO.
//! 			
//! 			To push new data on the FIFO, follow the sequence of operations
//! 			described in #COMMON_fifoGetLock. The *index* returned by the #COMMON_fifoPush
//! 			is used as array index to copy the data (i.e. push it on the COMMON FIFO).
//! 
//! 			To pop data from the FIFO, follow a similar sequence of operations
//! 			but call #COMMON_fifoPop instead.
//! 			
//! 			When the COMMON FIFO is no longer needed, call #COMMON_fifoDeinit
//! 			to release the COMMON FIFO resources and finally free the memory
//! 			possibly allocated for the array of data.
//! 			
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!


#include "osal.h"

#include "types.h"
#include "common_fifo.h"
#include "common_trace.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
/*!
 * \def		MAX_FIFO_SEM_NAME
 * 			Max length of the FIFO semaphore name
 */
#define MAX_FIFO_SEM_NAME  32

/*****************************************************************
| local types
|----------------------------------------------------------------*/

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/

/***************************
 *
 * COMMON_fifoInit
 *
 **************************/
/*!
 * \brief		Initializes a COMMON FIFO meta-data information
 * \details		The function does not allocate the space for the FIFO meta-data,
 * 				nor the space for the FIFO data. It only initializes the space
 * 				allocated by the caller.
 * \param[in,out] pfifo - pointer to a COMMON FIFO allocated by the caller.
 * 				          The function initializes the FIFO status.
 * \param[in]	name - pointer to a string that is copied into the FIFO status,
 * 				       only for debug purposes.
 * \param[in]	fifoSize - The number of elements in the FIFO. This is needed
 * 				           to calculate the read and write pointers wrap-around,
 * 				           not to allocate the space.
 * \return		OSAL_OK
 * \return		OSAL_ERROR - parameter error, semaphore creation error or
 * 				             name creation error. In all cases the FIFO
 * 				             is not available.
 * \callgraph
 * \callergraph
 */
tSInt COMMON_fifoInit(COMMON_fifoStatusTy *pfifo, tChar *name, tU16 fifoSize)
{
	tChar sem_name[MAX_FIFO_SEM_NAME];
	tChar prefix[5] = "Sem_";
	tSInt ret = OSAL_OK;
	
	if ((pfifo == NULL) ||
		(name == NULL) ||
		(fifoSize == 0))
	{
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		ret = OSAL_ERROR;
		goto exit;
	}

	(void)OSAL_pvMemorySet((tVoid *)pfifo, 0x00, sizeof(COMMON_fifoStatusTy));
	COMMON_fifoReset(pfifo);

	pfifo->name = name;
	pfifo->fifoSize = fifoSize;

	if (OSAL_s32NPrintFormat(sem_name, MAX_FIFO_SEM_NAME, "%s%s", prefix, name) < 0)
	{
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		ret = OSAL_ERROR;
		goto exit;
	}
	if (OSAL_s32SemaphoreCreate(sem_name, &pfifo->lock, 1) == OSAL_ERROR)
	{
		ret = OSAL_ERROR;
		goto exit;
	}

exit:
	return ret;
}

/***************************
 *
 * COMMON_fifoDeinit
 *
 **************************/
/*!
 * \brief		Releases the resources used by a COMMON FIFO
 * \details		The function destroys the semaphore connected to the FIFO.
 * \param[in]	pfifo - pointer of the COMMON FIFO to release
 * \param[in]	name - pointer to a string used in the #COMMON_fifoInit
 * \callgraph
 * \callergraph
 */
tVoid COMMON_fifoDeinit(COMMON_fifoStatusTy *pfifo, tChar *name)
{
	tChar sem_name[MAX_FIFO_SEM_NAME];
	tChar prefix[5] = "Sem_";

	if (OSAL_s32NPrintFormat(sem_name, MAX_FIFO_SEM_NAME, "%s%s", prefix, name) < 0)
	{
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		goto exit;
	}
	if (OSAL_s32SemaphoreClose(pfifo->lock) != OSAL_OK)
	{
		goto exit;
	}
	// normally done at system shutdown, we can ignore errors
	(LINT_IGNORE_RET) OSAL_s32SemaphoreDelete(sem_name);

exit:
	return;
}

/***************************
 *
 * COMMON_fifoReset
 *
 **************************/
/*!
 * \brief		Initializes the read and write pointers of a COMMON FIFO to the empty state
 * \details		The function uses a special value #COMMON_FIFO_POINTER_INVALID
 * 				to set the read and write pointer to empty state.
 * 				This function is provided mainly for exceptional cases (e.g.
 * 				to restart the FIFO in case of errors).
 * \param[in]	pfifo - pointer of the COMMON FIFO to reset
 * \callgraph
 * \callergraph
 */
tVoid COMMON_fifoReset(COMMON_fifoStatusTy *pfifo)
{
	if (pfifo == NULL)
	{
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		goto exit;
	}

	pfifo->readPtr = COMMON_FIFO_POINTER_INVALID;
	pfifo->writePtr = COMMON_FIFO_POINTER_INVALID;

exit:
	return;
}

/***************************
 *
 * COMMON_fifoGetLock
 *
 **************************/
/*!
 * \brief		Lock a COMMON FIFO to avoid race conditions
 * \details		COMMON fifo only manages the FIFO metadata thus the complete
 * 				access to the FIFO element cannot be atomic; to avoid
 * 				race conditions the caller should call this function to take
 * 				a lock before calling #COMMON_fifoPush/#COMMON_fifoPop and
 * 				call #COMMON_fifoReleaseLock to release it after the data 
 * 				write is complete.
 * \details		To avoid possible race conditions, the correct sequence of operations is:
 * 				1. #COMMON_fifoGetLock
 * 				2. #COMMON_fifoPush
 * 				3. update the fifo element
 * 				4. #COMMON_fifoReleaseLock
 *
 * 				(similar sequence for #COMMON_fifoPop)
 * \details		Note: without taking this lock it could happen that:
 * 				1. fifo empty, read pointer equal to write pointer
 * 				2. #COMMON_fifoPush is called and it updates the write pointer
 * 				3. THREAD SWITCH
 * 				4. #COMMON_fifoPop finds the updated write pointer and assumes
 * 				the fifo is not empty, fetches the fifo element pointed by
 * 				read pointer which has not yet been initialized by #COMMON_fifoPush
 * 				due to the thread switch so	it is 0-filled
 * 				5. THREAD SWITCH
 * 				6. #COMMON_fifoPush completes the write operation
 *
 * 				As a consequence, the #COMMON_fifoPop returns a 0-filled element,
 * 				but the	fifo contains a non-null element which will never be freed.
 * \param[in]	pfifo - pointer of the COMMON FIFO to lock
 * \callgraph
 * \callergraph
 */
tVoid COMMON_fifoGetLock(COMMON_fifoStatusTy *pfifo)
{
	if (OSAL_s32SemaphoreWait(pfifo->lock, OSAL_C_TIMEOUT_FOREVER) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
	}
}

/***************************
 *
 * COMMON_fifoReleaseLock
 *
 **************************/
/*!
 * \brief		Unlock a COMMON FIFO
 * \param[in]	pfifo - pointer of the COMMON FIFO to lock
 * \see			COMMON_fifoGetLock
 * \callgraph
 * \callergraph
 */
tVoid COMMON_fifoReleaseLock(COMMON_fifoStatusTy *pfifo)
{
	if (OSAL_s32SemaphorePost(pfifo->lock) != OSAL_OK)
	{
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
	}
}

/***************************
 *
 * COMMON_fifoPush
 *
 **************************/
/*!
 * \brief		Calculates the FIFO index for a COMMON FIFO push operation
 * \details		The function does not actually perform the data copy but rather
 * 				returns the index of the fifo location that can be written. It
 * 				is up to the caller	to actually write the data.
 * \param[in]	pfifo - pointer of the COMMON FIFO
 * \param[out]	index - pointer to a location where the function stores the index
 * 				        of the FIFO location to write to, or #COMMON_FIFO_POINTER_INVALID
 * 				        in case of error.
 * \return		OSAL_OK - *index* contains the index of the location to be filled
 * \return		OSAL_ERROR the FIFO is full; *index* set to #COMMON_FIFO_POINTER_INVALID
 * \see			COMMON_fifoGetLock for the correct sequence of operations.
 * \callgraph
 * \callergraph
 */
tSInt COMMON_fifoPush(COMMON_fifoStatusTy *pfifo, tS16 *index)
{
	tS16 new_write_ptr;
	tSInt ret = OSAL_OK; 

	if ((pfifo == NULL) || (index == NULL))
	{
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		ret = OSAL_ERROR;
		goto exit;
	}

	*index = COMMON_FIFO_POINTER_INVALID;

	new_write_ptr = (pfifo->writePtr + 1) % pfifo->fifoSize;

	if (pfifo->writePtr == COMMON_FIFO_POINTER_INVALID)
	{
		// startup condition, FIFO empty, no need to check the overflow condition
	}
	else if ((pfifo->writePtr == (tS16)pfifo->fifoSize - 1) &&
			(pfifo->readPtr == COMMON_FIFO_POINTER_INVALID))
	{
		// write ptr reached the last FIFO location but read has not started yet:
		// the FIFO is full but the comparison in the 'else' would not detect the situation
		ret = OSAL_ERROR;
		goto exit;
	}
	else if (new_write_ptr == pfifo->readPtr)
	{
		// overflow condition: accepting the data would result in overwriting a yet unread FIFO location
		ret = OSAL_ERROR;
		goto exit;
	}
	else
	{
		/* Nothing to do */
	}
	
	pfifo->writePtr = new_write_ptr;
	*index = new_write_ptr;

exit:
	return ret;
}

/***************************
 *
 * COMMON_fifoPop
 *
 **************************/
/*!
 * \brief		Calculates the FIFO index for a COMMON FIFO pop operation
 * \details		The function does not actually perform the data copy but rather
 * 				returns the index of the fifo location that can be written. It
 * 				is up to the caller	to actually write the data.
 * \param[in]	pfifo - pointer of the COMMON FIFO
 * \param[out]	index - pointer to a location where the function stores the index
 * 				        of the FIFO location to read from, or #COMMON_FIFO_POINTER_INVALID
 * 				        in case of error.
 * \return		OSAL_OK - the FIFO was not empty, the index of the head of
 * 				          the FIFO is returned in *index.
 * \return		OSAL_ERROR - the FIFO is empty; in this case *index*
 * 				             is set to #COMMON_FIFO_POINTER_INVALID
 * \see			COMMON_fifoGetLock for the correct sequence of operations.
 * \callgraph
 * \callergraph
 */
tSInt COMMON_fifoPop(COMMON_fifoStatusTy *pfifo, tS16 *index)
{
	tS16 new_read_ptr;
	tSInt ret = OSAL_OK;

	if ((pfifo == NULL) || (index == NULL))
	{
		ASSERT_ON_DEBUGGING(0);			//lint !e960	- MISRA 12.10 - use of assert code
		ret = OSAL_ERROR_INVALID_PARAM;
		goto exit;
	}

	if (pfifo->readPtr == pfifo->writePtr)
	{
		// FIFO empty condition
	    *index = pfifo->readPtr;

		ret = OSAL_ERROR;
		goto exit;
	}
	else
	{
        if (pfifo->readPtr == COMMON_FIFO_POINTER_INVALID)
        {
            // startup condition: read pointer is still uninitialized but write pointer
            // was modified: initialize the read pointer to the first FIFO location
            new_read_ptr = 0;
        }
        else
        {
            new_read_ptr = (pfifo->readPtr + 1) % pfifo->fifoSize;
        }
        pfifo->readPtr = new_read_ptr;
        *index = new_read_ptr;
	}

exit:
	return ret;
}

