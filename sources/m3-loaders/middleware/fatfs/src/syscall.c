/*------------------------------------------------------------------------*/
/* Sample code of OS dependent controls for FatFs R0.08                   */
/* (C)ChaN, 2010                                                          */
/*------------------------------------------------------------------------*/
/*  **********************
    ***** DISCLAIMER *****
    **********************
    "Long File Names" is an intelluectual property of 
    "Microsoft Corporation". In this module LFN can be used exclusively when 
    propedeutic for flashing WinCE6.0 artifacts.
    LFN is used  for copying Storage Based Image auxialiary files.
 */

#include <stdlib.h>		/* ANSI memory controls */
#include "ff.h"


#if _FS_REENTRANT


/*------------------------------------------------------------------------*/
/* Create a Synchronization Object										  */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount function to create a new
/  synchronization object, such as semaphore and mutex. When a FALSE is
/  returned, the f_mount function fails with FR_INT_ERR.
*/

int ff_cre_syncobj (	/* TRUE:Function succeeded, FALSE:Could not create due to any error */
	BYTE vol,			/* Corresponding logical drive being processed */
	_SYNC_t **sobj		/* Pointer to return the created sync object */
)
{ 	  	
  	// important add a mutex for each volume
  	*sobj = mutex_create_fifo ();
    	
	return TRUE;
}



/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount function to delete a synchronization
/  object that created with ff_cre_syncobj function. When a FALSE is
/  returned, the f_mount function fails with FR_INT_ERR.
*/

int ff_del_syncobj (	/* TRUE:Function succeeded, FALSE:Could not delete due to any error */
	_SYNC_t **sobj		/* Sync object tied to the logical drive to be deleted */
)
{
	mutex_delete(*sobj);
	return TRUE;
}



/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a FALSE is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant (	/* TRUE:Got a grant to access the volume, FALSE:Could not get a grant */
	_SYNC_t **sobj	/* Sync object to wait */
)
{
	mutex_lock (*sobj);
//	mutex_wait (sobj);
	return TRUE;
}



/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant (
	_SYNC_t **sobj	/* Sync object to be signaled */
)
{
	mutex_release (*sobj);
//	mutex_signal(&MUTEX);
}

#endif


#if _USE_LFN == 3	/* LFN with a working buffer on the heap */
/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/
/* If a NULL is returned, the file function fails with FR_NOT_ENOUGH_CORE.
*/

void* ff_memalloc (	/* Returns pointer to the allocated memory block */
	tU32 size		/* Number of bytes to allocate */
)
{
	return pvPortMalloc(size);
}


/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree(
	void* mblock	/* Pointer to the memory block to free */
)
{
	vPortFree(mblock);
}

#endif
