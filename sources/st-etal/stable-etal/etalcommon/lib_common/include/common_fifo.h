//!
//!  \file 		common_fifo.h
//!  \brief 	<i><b> Generic FIFO management </b></i>
//!  \details   Functions to manage a generic FIFO meta-data, that is the read and write
//!				pointer management.
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!
#ifndef COMMON_FIFO_H_
#define COMMON_FIFO_H_

/***********************************
 *
 * Defines
 *
 **********************************/
/*!
 * \def		COMMON_FIFO_POINTER_INVALID
 * 			Value used to identify invalid read or write pointer
 * 			or invalid *index* parameter in case of error or
 * 			empty FIFO condition
 */
#define COMMON_FIFO_POINTER_INVALID  ((tS16)(-1))

/***********************************
 *
 * Types
 *
 **********************************/
/*!
 * \struct	COMMON_fifoStatusTy
 * 			Defines the type of the variable containing the
 * 			private status of the COMMON FIFO.
 * 			
 */
typedef struct
{
	/*! The FIFO name, only for debug purposes */
	tChar *name;
	/*! Max number of elements in the fifo */
	tU16   fifoSize;
	/*! Index of the last location read */
	tS16   readPtr;
	/*! Index of the last location written to */
	tS16   writePtr;
	/*! Semaphore needed by #COMMON_fifoGetLock/#COMMON_fifoReleaseLock */
	OSAL_tSemHandle lock;
} COMMON_fifoStatusTy;

/***********************************
 *
 * Function prototypes
 *
 **********************************/

tSInt COMMON_fifoInit(COMMON_fifoStatusTy *pfifo, tChar *name, tU16 fifoSize);
tVoid COMMON_fifoDeinit(COMMON_fifoStatusTy *pfifo, tChar *name);
tVoid COMMON_fifoReset(COMMON_fifoStatusTy *pfifo);
tVoid COMMON_fifoGetLock(COMMON_fifoStatusTy *pfifo);
tVoid COMMON_fifoReleaseLock(COMMON_fifoStatusTy *pfifo);
tSInt COMMON_fifoPush(COMMON_fifoStatusTy *pfifo, tS16 *index);
tSInt COMMON_fifoPop(COMMON_fifoStatusTy *pfifo, tS16 *index);
#endif // COMMON_FIFO_H_
