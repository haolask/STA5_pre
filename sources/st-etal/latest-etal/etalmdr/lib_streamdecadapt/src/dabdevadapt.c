//!
//!  \file       dabdevadapt.c
//!  \brief      <i><b>This file contains functions for dabdev audio raw input handling</b></i>
//!  \author     David Pastor
//!  \version    1.0
//!  \date       2017.10.11
//!  \bug        Unknown
//!  \warning    None
//!

#ifdef __cplusplus
extern "C"
{
#endif

//----------------------------------------------------------------------
// Includes and externally used resources inclusion
//----------------------------------------------------------------------
#include "osalIO.h"
#include "libDABDevice.h"
#include "sd_dcsr.h"
//#include "Memory.h"
#include "Signals.h"
#include "MMObject.h"
#include "DABSubchDataProvider.h"
#include "etalinternal.h"
#include "dabdevadapt.h"
#include "dabdevadapt4sd.h"

//----------------------------------------------------------------------
// Local variables definition
//----------------------------------------------------------------------
static struct {
  char                                used;
  /* 16 bit representing the max number of parallel open per dabdev*/
  tU16                                opened_field;

//  tPU8                                pu8cmd_buffer;
//  OSAL_trAsyncControl*                ntf_buffer[CONFIG_MAX_OPEN_PER_DAB_DEV][NTF_BUFFER_NUMB_PER_DABDEV+1];           /*+1 make LINT happy*/
//  OSAL_trAsyncControl*                fic_buffer[CONFIG_MAX_OPEN_PER_DAB_DEV][FIC_BUFFER_NUMB_PER_DABDEV+1];           /*+1 make LINT happy*/

  /*if several instances request the same data (e.g. subchannel) only one ptr will be given to the ll dabdev*/
  /*data for the other instance will be duplicated in the dabdev_data_transport_ready function*/
  OSAL_trAsyncControl*                msc_buffer[CONFIG_MAX_OPEN_PER_DAB_DEV][MSC_BUFFER_NUMB_PER_DABDEV+1];           /*+1 make LINT happy*/
  tU8                                 msc_buff_subch_id[CONFIG_MAX_OPEN_PER_DAB_DEV][MSC_BUFFER_NUMB_PER_DABDEV+1];    /*+1 make LINT happy*/
//  tBool                               msc_buff_at_lldabdev[CONFIG_MAX_OPEN_PER_DAB_DEV][MSC_BUFFER_NUMB_PER_DABDEV+1]; /*+1 make LINT happy*/
  OSAL_tSemHandle                     msc_buffer_lock_sem;

#ifdef CONFIG_SOURCE_OF_PAD_STREAMDECODER
  OSAL_trAsyncControl*                pad_buffer[CONFIG_MAX_OPEN_PER_DAB_DEV][PAD_BUFFER_NUMB_PER_DABDEV+1];           /*+1 make LINT happy*/
  tU8                                 pad_buff_subch_id[CONFIG_MAX_OPEN_PER_DAB_DEV][PAD_BUFFER_NUMB_PER_DABDEV+1];    /*+1 make LINT happy*/
#endif

  OSAL_trAsyncControl*                ms_notify_channel_selection_buffer[CONFIG_MAX_OPEN_PER_DAB_DEV];
  tU8                                 ms_notify_channel_selection_used[CONFIG_MAX_OPEN_PER_DAB_DEV];

#ifdef CONFIG_TARGET_DEV_DABDEVICE_TEST__ENABLE

  tU8                                 u8NotificationBuffer[NOTIFY_BUFFER_SIZE];
  tU32                                u32NotificationBufferLen;
#endif

//  OSAL_tSemHandle                     wait_until_cmd_ready_sem;
//  OSAL_tSemHandle                     one_request_allowed_sem;

//  struct OSAL_trAsyncDABDevSetupSubchBuffer_struct  s32SetupSubchBufferCallback; /*make LINT happy*/
  OSAL_tSemHandle                     sem;

  OSAL_tSemHandle                     read_cb_sem;

  OSAL_tSemHandle                     ms_notify_channel_selection_sem;

#ifdef ENABLE_REALTIME_TIMEOUT_DIAG
  struct
  {
	  OSAL_tMSecond				      starttime;
	  tU16                            pad_callback_timeout_counter;
	  tU16                            fic_callback_timeout_counter;
	  tU16                            msc_callback_timeout_counter;
	  tU16                            ntf_callback_timeout_counter;
	  tU16                            msntfchsel_callback_timeout_counter;
  } callback_timeout_check;
#endif /* ENABLE_REALTIME_TIMEOUT_DIAG */

} dabdevice_members_data [CONFIG_MAX_NUM_DAB_DEVICES];

typedef struct
{
    MMDataProvider *object;
    OSAL_trAsyncControl async;
    tPVoid buffer;
#ifdef ENABLE_ADDITIONAL_DEBUG_CODE
    int index;
    int requested;
#endif
} MSNtfChannelAsyncBufferType;

//----------------------------------------------------------------------
// Local functions declaration
//----------------------------------------------------------------------
tS32 DABDEV_u32IOOpen(OSAL_tenDevID tDevID, tCString coszName,
			          OSAL_tenAccess enAccess, OSAL_tIODescriptor * pfd);
tS32 DABDEV_u32IOClose(OSAL_tIODescriptor fd);
tS32 DABDEV_u32IOControl(OSAL_tIODescriptor fd, tS32 s32FunPar, tS32 s32ArgPar);
tS32 DABDEV_u32IOWrite(OSAL_tIODescriptor fd, tPCS8 ps8BufferPar,
			           tU32 u32MaxLengthPar, tPU32 u32WrChar);
tS32 DABDEV_u32IORead(OSAL_tIODescriptor fd, tPCS8 ps8BufferPar,
			          tU32 u32MaxLengthPar, tPU32 u32RdChar);

static const OSAL_trDevFunctionTable DABDEV_FunctionTable = {
			OSAL_NULL,
			OSAL_NULL,
			(OSAL_tpfIOOpen) DABDEV_u32IOOpen,
			(OSAL_tpfIOClose) DABDEV_u32IOClose,
			(OSAL_tpfIOControl) DABDEV_u32IOControl,
			(OSAL_tpfIOWrite) DABDEV_u32IOWrite,
			(OSAL_tpfIORead) DABDEV_u32IORead, };

static const OSAL_trOpenTable DABDEV_OpenTable = { OSAL_EN_NOT_MULTIOPEN,
			(tU32) 0, };


//----------------------------------------------------------------------
// Local functions declaration
//----------------------------------------------------------------------

tVoid DABDEV_vIOInit( tVoid)
{
	/* do some initializiation */
	int tmp_id;
//	int fic_id;
	int msc_id;
#ifdef CONFIG_SOURCE_OF_PAD_STREAMDECODER
    int pad_id;
#endif
	int open_id;
	tChar tmp_str[32];

    OSAL_pvMemorySet(&dabdevice_members_data,0,sizeof(dabdevice_members_data));

	for (tmp_id = 0; tmp_id < CONFIG_MAX_NUM_DAB_DEVICES; tmp_id++)
	{
		dabdevice_members_data[tmp_id].used         = FALSE;
		dabdevice_members_data[tmp_id].opened_field = 0;

		/*create the semas for the dabdev*/
		OSAL_s32NPrintFormat(tmp_str, 32, "dabdev_sem%d", tmp_id);
		OSAL_s32SemaphoreCreate(tmp_str,
				&dabdevice_members_data[tmp_id].sem, 1);

		OSAL_s32NPrintFormat(tmp_str, 32, "dabdev_read_cb_sem%d", tmp_id);
		OSAL_s32SemaphoreCreate(tmp_str,
				&dabdevice_members_data[tmp_id].read_cb_sem, 0);

		OSAL_s32NPrintFormat(tmp_str, 32, "dabdev_msNotChSel_sem%d", tmp_id);
		OSAL_s32SemaphoreCreate(tmp_str,
				&dabdevice_members_data[tmp_id].ms_notify_channel_selection_sem, 1);
		
		for (msc_id=0;msc_id<MSC_BUFFER_NUMB_PER_DABDEV;msc_id++)
		{
		 for (open_id=0;open_id<CONFIG_MAX_OPEN_PER_DAB_DEV;open_id++)
		 {
		  dabdevice_members_data[tmp_id].msc_buffer[open_id][msc_id]           = NULL;
		  dabdevice_members_data[tmp_id].msc_buff_subch_id[open_id][msc_id]    = 0xff;
#if 0
		  dabdevice_members_data[tmp_id].msc_buff_at_lldabdev[open_id][msc_id] = FALSE;
#endif
		 }
		}

#ifdef CONFIG_SOURCE_OF_PAD_STREAMDECODER
		for (pad_id=0;pad_id<PAD_BUFFER_NUMB_PER_DABDEV;pad_id++)
		{
		 for (open_id=0;open_id<CONFIG_MAX_OPEN_PER_DAB_DEV;open_id++)
		 {
		  dabdevice_members_data[tmp_id].pad_buffer[open_id][pad_id]           = NULL;
		  dabdevice_members_data[tmp_id].pad_buff_subch_id[open_id][pad_id]    = 0xff;
		 }
		}
#endif
#if 0
	    for (fic_id=0;fic_id<FIC_BUFFER_NUMB_PER_DABDEV;fic_id++)
	      {
			 for (open_id=0;open_id<CONFIG_MAX_OPEN_PER_DAB_DEV;open_id++)
			 {
				 dabdevice_members_data[tmp_id].fic_buffer[open_id][fic_id]    = NULL;
			 }
	      }
#endif
	    /*create the sema for the msc_buffer lock*/
		OSAL_s32NPrintFormat(tmp_str, 32, "dabdevice_m_b_l_s%d", tmp_id);
	    OSAL_s32SemaphoreCreate(tmp_str,
		 	                    &dabdevice_members_data[tmp_id].msc_buffer_lock_sem,
							    1);
#if 0
	    /*create the semas for the dcsr response*/
	    OSAL_s32SemaphoreCreate("dabdevice_wu_resp",
		 	                    &dabdevice_members_data[tmp_id].wait_until_cmd_ready_sem,
							    1);
	    /*create the sema for the push_request_to_task*/
	    OSAL_s32SemaphoreCreate("dabdevice_o_r_a_s",
		 	                    &dabdevice_members_data[tmp_id].one_request_allowed_sem,
							    1);
#endif
	}
#if 0
	/*create the dabdevice event*/
	if (OSAL_OK != OSAL_s32EventCreate("DABDEV_EVENT",
			&dabdevice_class_data.phDABDEV_Event))
	{

	}
#endif
#if (CONFIG_MAX_NUM_DAB_DEVICES > 0)
	OSALIO_s32AddDevice(((OSAL_tenDevID)(OSAL_EN_DEVID_DABDEV0)),
			DABDEV_FunctionTable, DABDEV_OpenTable);
#if (CONFIG_MAX_NUM_DAB_DEVICES > 1 )
	OSALIO_s32AddDevice(((OSAL_tenDevID)(OSAL_EN_DEVID_DABDEV1)),
			DABDEV_FunctionTable, DABDEV_OpenTable);
#if (CONFIG_MAX_NUM_DAB_DEVICES > 2)
	OSALIO_s32AddDevice(((OSAL_tenDevID)(OSAL_EN_DEVID_DABDEV2)),
			DABDEV_FunctionTable, DABDEV_OpenTable);
#endif
#endif
#endif
	/*init and start the dabdev task*/
	//DABDEV_task_init();
}

/******************************************************************************

 FUNCTION:      DABDEV_IOOpen

 DESCRIPTION:   Function used to "open" DABDEV device.

 PARAMETERS:    none

 RETURN TYPE:   tU32 error codes

 COMMENTS:      It has to be called before any calls to Read/Write fx
 and AFTER the Init function.

 HISTORY:
 Date         Modification               Author

 ---------------------------------------------------------------------------- */

tS32 DABDEV_u32IOOpen(OSAL_tenDevID tDevID, tCString coszName,
		OSAL_tenAccess enAccess, OSAL_tIODescriptor * pfd)
{
#if 1
    tU8  device_id        = tDevID - OSAL_EN_DEVID_DABDEV0;
    tS32 s32ReturnValue   = OSAL_OK;
    int  i;
	tU16 tmp_opened_field = dabdevice_members_data[device_id].opened_field;

	/*it shall be possible to open one instance of the dab device CONFIG_MAX_OPEN_PER_DAB_DEV times in parallel*/
	/*search for a free place in the opened bit mask*/
	for (i=0;i<CONFIG_MAX_OPEN_PER_DAB_DEV;i++)
	{
	  if ((tmp_opened_field&0x0001)==0)
	  {
	    tU8 init_d = (device_id&0x0f)|(i<<4);
	    dabdevice_members_data[device_id].used = TRUE;
        /*determine the file descriptor*/
        *pfd = ((OSAL_tIODescriptor) OSAL_IODESCRIPTOR(tDevID,init_d));
	    /*set the bit to used*/
        dabdevice_members_data[device_id].opened_field |= (0x01<<i);
        return (s32ReturnValue);
      }
      else
      {
	    /*try the next bit*/
	    tmp_opened_field>>=1;
      }
    }

	/*no free entry found*/
    *pfd = OSAL_NULL;

    (void) enAccess; /*make LINT happy*/
	(void) coszName; /*make LINT happy*/

    return OSAL_ERROR;
#else
	return OSAL_OK;
#endif // 0
}

/******************************************************************************

 FUNCTION:      DABDEV_u32IOClose

 DESCRIPTION:   Function to close DABDEV device.

 PARAMETERS:    none

 RETURN TYPE:   uU32 error codes

 COMMENTS:

 HISTORY:
 Date         Modification               Author

 ---------------------------------------------------------------------------- */

tS32 DABDEV_u32IOClose(OSAL_tIODescriptor fd)
{
	tS32 s32ReturnValue = OSAL_ERROR;
	/*TO DO */
	s32ReturnValue = OSAL_OK;
	(void) fd; /*make LINT happy*/
	return (s32ReturnValue);
}

/******************************************************************************

 FUNCTION:      DABDEV_u32IOWrite

 DESCRIPTION:   DABDEV device block erase information control function
 ---> NOT USED FUNCTION  <-------

 PARAMETERS:    tPS8 ps8BufferPar (->O)
 1st parameter: pointer to OSAL_trRegionInfo Structure
 tU32 u32MaxLengthPar (I)
 2nd parameter: number of the erase block region

 RETURN TYPE:   uS32 error codes

 HISTORY:

 Date         Modification               Author

 ---------------------------------------------------------------------------- */

tS32 DABDEV_u32IOWrite(OSAL_tIODescriptor fd, tPCS8 ps8BufferPar,
		tU32 u32MaxLengthPar, tPU32 u32WrChar)
{
#if 0
	tU8 init_d    = OSAL_IODESCRIPTOR_2_INTID(fd);
	volatile tU8 device_id = init_d & 0x0f;
	/*TO DO */
	OSAL_s32ThreadWait(1000);

	(void) u32WrChar;       /*make LINT happy*/
	(void) u32MaxLengthPar; /*make LINT happy*/
	(void) device_id;       /*make LINT happy*/
	(void) ps8BufferPar   ; /*make LINT happy*/

#endif // 0
	return OSAL_ERROR;
}

tS32 DABDEV_u32IORead(OSAL_tIODescriptor fd, tPCS8 ps8BufferPar,
		tU32 u32MaxLengthPar, tPU32 u32RdChar)
{
#if 0
  tS32 device_id = OSAL_IODESCRIPTOR_2_INTID(fd)&0xf;
  {
	DABDEV_u32IORead_Callback_Type cb;
	/* fill aio structure */
	cb.aio.id = fd;
	cb.aio.s32Offset = 0;
	cb.aio.pvBuffer = (tPVoid)ps8BufferPar;
	cb.aio.u32Length = u32MaxLengthPar;
	cb.aio.pCallBack = DABDEV_u32IORead_callback;
	cb.aio.pvArg = &cb;
	cb.aio.u32ErrorCode = 0;
	cb.aio.s32Status = 0;

	cb.fd = fd;

	OSAL_s32IOControl(fd,
			OSAL_C_S32_IOCTRL_DAB_READ_ASYNC_NTF, (tS32)
					&cb.aio);

	  OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].read_cb_sem,
			  OSAL_C_TIMEOUT_FOREVER);

	  /*set the number of read chars*/
	  *u32RdChar = cb.aio.s32Status;
  }
 #endif // 0
  return OSAL_OK;
}

/******************************************************************************

 FUNCTION:      DABDEV_u32IOControl

 DESCRIPTION:   DABDEV device control function


 PARAMETERS:    uS32 s32FunPar (I)
 1st parameter: Function identificator
 tS32 s32ArgPar (IO)
 2nd parameter: to be passed/returned  to/by function

 RETURN TYPE:   tS32 error codes

 HISTORY:

 Date         Modification               Author

 ---------------------------------------------------------------------------- */
tS32 DABDEV_u32IOControl(OSAL_tIODescriptor fd, tS32 s32FunPar, tS32 s32ArgPar)
{
	tU8  init_d    = OSAL_IODESCRIPTOR_2_INTID(fd);
	tU8  device_id = init_d & 0x0f;
	tU8  open_idx  = (init_d & 0xf0)>>4;
	tU16 tmp_idx   = 0;
    tS32 ret_val = OSAL_ERROR;

#if 0
	tU32           u32oldCurrentPriority;
	OSAL_tThreadID tid;

	tid = OSAL_ThreadWhoAmI();

	/*get the prio of this task*/
	if (OSAL_ERROR == OSAL_s32GetPriority(tid, &u32oldCurrentPriority))
	{
	  /*well, we have a problem*/
	}

	/*increase the prio to assure quick return of the response*/
	OSAL_s32ThreadPriority(tid, CONST_TARGET_DABDEV_TASKPRIORITY);
#endif

	/*wait for other tasks to finish*/
	OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].sem,OSAL_C_TIMEOUT_FOREVER);

#ifdef CONFIG_SOURCE_OF_PAD_STREAMDECODER
	/*check for read async PAD*/
	if ((s32FunPar>=OSAL_C_S32_IOCTRL_DAB_READ_ASYNC_PAD)&&
		(s32FunPar<=(OSAL_C_S32_IOCTRL_DAB_READ_ASYNC_PAD + 63)))
	{
		for (tmp_idx=0;tmp_idx<PAD_BUFFER_NUMB_PER_DABDEV;tmp_idx++)
		{
		  /*store the read async request in either buffer*/
	  	  if(dabdevice_members_data[device_id].pad_buffer[open_idx][tmp_idx]==NULL)
	      {
	  		dabdevice_members_data[device_id].pad_buffer[open_idx][tmp_idx]  	   = (OSAL_trAsyncControl*) s32ArgPar;
	  		dabdevice_members_data[device_id].pad_buff_subch_id[open_idx][tmp_idx] = s32FunPar-OSAL_C_S32_IOCTRL_DAB_READ_ASYNC_PAD;
#if 0
	  	    /*reset the prio to the initial value*/
	  	    OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif
	  	    /*release the sem before returning*/
			OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
			return OSAL_OK;
		  }
		}
#if 0
		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif
        OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
  	    return OSAL_ERROR;
	}

	/*check for cancellation of read async PAD*/
	if ((s32FunPar >= OSAL_C_S32_IOCTRL_DAB_CANCEL_READ_ASYNC_PAD) &&
		(s32FunPar <= (OSAL_C_S32_IOCTRL_DAB_CANCEL_READ_ASYNC_PAD + 63)))
	{
		for (tmp_idx=0;tmp_idx<PAD_BUFFER_NUMB_PER_DABDEV;tmp_idx++)
		{
		   if (dabdevice_members_data[device_id].pad_buffer[open_idx][tmp_idx] == (OSAL_trAsyncControl*) s32ArgPar)
		   {
			   dabdevice_members_data[device_id].pad_buffer[open_idx][tmp_idx]        = NULL;
			   dabdevice_members_data[device_id].pad_buff_subch_id[open_idx][tmp_idx] = 0xff;
#if 0
				/*reset the prio to the initial value*/
				OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif
			   /*release the sem before returning*/
			   OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
			   return OSAL_OK;
		   }
		}
#if 0
		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif
        OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
  	    return OSAL_ERROR;
	}
#endif

	/*check for read async subchannel*/
	if ((s32FunPar>=OSAL_C_S32_IOCTRL_DAB_READ_ASYNC)&&
		(s32FunPar<=(OSAL_C_S32_IOCTRL_DAB_READ_ASYNC + 63)))
	{
	   for (tmp_idx=0;tmp_idx<MSC_BUFFER_NUMB_PER_DABDEV;tmp_idx++)
	   {
	    /*store the read async request in either buffer*/
   	    if(dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx]==OSAL_NULL)
		{
			dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx]  	   = (OSAL_trAsyncControl*) s32ArgPar;
			dabdevice_members_data[device_id].msc_buff_subch_id[open_idx][tmp_idx] = s32FunPar-OSAL_C_S32_IOCTRL_DAB_READ_ASYNC;
#if 0
			/*reset the prio to the initial value*/
			OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif
			/*release the sem before returning*/
			OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
		    return OSAL_OK;
		}
	   }
#if 0
		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif
		/*release the sem before returning*/
		OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
	   // print an arror
	   OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_DEV_DAB, "DABDEV_u32IOControl OSAL_C_S32_IOCTRL_DAB_READ_ASYNC ERROR open_idx %d", open_idx);

	   return OSAL_ERROR;
	}

	/*check for cancellation of read async subchannel*/
	if ((s32FunPar >= OSAL_C_S32_IOCTRL_DAB_CANCEL_READ_ASYNC) && (s32FunPar
			<= (OSAL_C_S32_IOCTRL_DAB_CANCEL_READ_ASYNC + 63)))
	{
		for (tmp_idx=0;tmp_idx<MSC_BUFFER_NUMB_PER_DABDEV;tmp_idx++)
		{
#if 0
 		 OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
		if (dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx] != NULL)
		{
			 dabdev_lock_mem_blk_ptr(init_d, dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx]->pvBuffer);
		}
		 OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].sem,OSAL_C_TIMEOUT_FOREVER);
#endif

		 if (dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx] == (OSAL_trAsyncControl*) s32ArgPar)
		 {
#if 0
			/*check for buffers that are at the ll dab dev*/
			if (dabdevice_members_data[device_id].msc_buff_at_lldabdev[open_idx][tmp_idx] == TRUE)
			{
			  /* this should not happen due to removal of flag in dabdev_dont_need_buffers_anymore*/
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if    (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_USER_1)
	          OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_DEV_DAB,
	        		  "OSAL_dabdev->OSAL_C_S32_IOCTRL_DAB_CANCEL_READ_ASYNC -> buffer is at the ll dabdev\n");
#endif
#endif
			}
			else
#endif // 0
			{
			  /*if the buffer is at the ll dabdev it will be removed later by dont_need_buffers_anymore*/
			  dabdevice_members_data[device_id].msc_buff_subch_id[open_idx][tmp_idx] = 0xff;
			}

			dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx] = NULL;

			OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
#if 0
			dabdev_unlock_mem_blk_ptr(init_d, NULL);
#endif
			OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].sem,OSAL_C_TIMEOUT_FOREVER);
#if 0
			/*reset the prio to the initial value*/
			OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif
			/*release the sem before returning*/
			OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
			return OSAL_OK;
		 }
		 else
		 {
		   OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
#if 0
		   dabdev_unlock_mem_blk_ptr(init_d, NULL);
#endif
		   OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].sem,OSAL_C_TIMEOUT_FOREVER);
		 }
		}
#if 0
		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif
		/*release the sem before returning*/
		OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
		return OSAL_ERROR;
	}

	switch (s32FunPar)
	{
#if 0
#ifdef CONFIG_COMPONENT_API_DDM_PMD_ENABLE_RS_FEC
    case OSAL_C_S32_IOCTRL_DAB_DECODE_PM_RS_FEC:
        /*try the RS of the FEC frame table*/

        PM_RS_FEC_decode((tU8*) s32ArgPar);

		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
        OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
  	    return OSAL_OK;
#endif
#ifdef CONFIG_SOURCE_OF_PAD_STREAMDECODER
    case OSAL_C_S32_IOCTRL_DAB_READ_OPEN_CHANNEL_PAD:
        /*nothing to do, maybe later*/

		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

        OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
  	    return OSAL_OK;

    case OSAL_C_S32_IOCTRL_DAB_READ_CLOSE_CHANNEL_PAD:
    	/*give back all buffers for this channel !!!*/
		for (tmp_idx=0;tmp_idx<PAD_BUFFER_NUMB_PER_DABDEV;tmp_idx++)
		{
		 if (dabdevice_members_data[device_id].pad_buffer[open_idx][tmp_idx] != NULL)
		 {
		   /*store this buffer temporarily to send it out*/
		   OSAL_trAsyncControl* aio = dabdevice_members_data[device_id].pad_buffer[open_idx][tmp_idx];

		   /*clean up the members data*/
		   dabdevice_members_data[device_id].pad_buffer[open_idx][tmp_idx]        = NULL;
		   dabdevice_members_data[device_id].pad_buff_subch_id[open_idx][tmp_idx] = 0xFF;

		   /*call the callback with size set to zero to indicate empty buffer*/
		   aio->s32Status    = 0;
		   aio->u32ErrorCode = OSAL_OK;

		   /*give the buffer back*/
		   if(aio->pCallBack != OSAL_NULL)
		   {
#ifdef ENABLE_REALTIME_TIMEOUT_DIAG
			   dabdevice_members_data[device_id].callback_timeout_check.starttime =
					   OSAL_ClockGetElapsedTime();
#endif /* ENABLE_REALTIME_TIMEOUT_DIAG */

		    /*release the sem because it may be used in  the callback*/
			OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
			/*send out the buffer*/
			aio->pCallBack(aio->pvArg);
			/*and again lock it*/
			OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].sem,OSAL_C_TIMEOUT_FOREVER);

#ifdef ENABLE_REALTIME_TIMEOUT_DIAG
			if( OSAL_ThreadWhoAmI() == DABDEV_ThreadID )
			if( (OSAL_ClockGetElapsedTime()-dabdevice_members_data[device_id].callback_timeout_check.starttime) > DABDEV_CALLBACK_TIMEOUT )
			{
				   dabdevice_members_data[device_id].callback_timeout_check.pad_callback_timeout_counter++;
			}
#endif /* ENABLE_REALTIME_TIMEOUT_DIAG */
		   }
		   /*buffer is now given back and can be used again*/
		 }
		}
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_USER_1)
	    OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_DEV_DAB,
				"OSAL_debdev-> OSAL_C_S32_IOCTRL_DAB_READ_CLOSE_CHANNEL_PAD done\n");
#endif
#endif

		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

        OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
  	    return OSAL_OK;
#endif
#endif //  0

    case OSAL_C_S32_IOCTRL_DAB_READ_OPEN_CHANNEL:
    	/*up to now these functions are only used by dabdevproxy to configure the SSI*/
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_USER_1)
		OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_DEV_DAB,
				"dabdev-> OSAL_C_S32_IOCTRL_DAB_READ_OPEN_CHANNEL (subch_id %d) done\n", s32ArgPar);
#endif
#endif
#if 0
		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif
		/*release the sem before returning*/
		OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
		return OSAL_OK;

    case OSAL_C_S32_IOCTRL_DAB_READ_CLOSE_CHANNEL:
    	/*give back all buffers for this channel !!!*/
    	/*these functionality is done by the SSI in case of dabdevproxy usage*/
		for (tmp_idx=0;tmp_idx<MSC_BUFFER_NUMB_PER_DABDEV;tmp_idx++)
		{
			if (dabdevice_members_data[device_id].msc_buff_subch_id[open_idx][tmp_idx] == (tU8)(s32ArgPar&0xff))
			{
				OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
				if (dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx] != NULL)
				{
#if 0
					dabdev_lock_mem_blk_ptr(init_d, dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx]->pvBuffer);
#endif
				}
				OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].sem,OSAL_C_TIMEOUT_FOREVER);

				if (dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx] != NULL)
				{
					/*store this buffer temporarily to send it out*/
					OSAL_trAsyncControl* aio = dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx];
#if 0
					/*check for buffers that are at the ll dab dev*/
					if (dabdevice_members_data[device_id].msc_buff_at_lldabdev[open_idx][tmp_idx] == TRUE)
					{
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if	   (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_USER_1)
						OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_DEV_DAB,
								"OSAL_dabdev->OSAL_C_S32_IOCTRL_DAB_READ_CLOSE_CHANNEL -> buffer is at the ll dabdev\n");
#endif
#endif
					}
					else
#endif // 0
					{
						/*if the buffer is at the ll dabdev it will be removed later by dont_need_buffers_anymore*/
						dabdevice_members_data[device_id].msc_buff_subch_id[open_idx][tmp_idx] = 0xFF;
					}

					/*clean up the members data*/
					dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx]        = OSAL_NULL;

					/*call the callback with size set to zero to indicate empty buffer*/
					aio->s32Status    = 0;
					aio->u32ErrorCode = OSAL_OK;

					OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
#if 0
					dabdev_unlock_mem_blk_ptr(init_d, NULL);
#endif
					OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].sem,OSAL_C_TIMEOUT_FOREVER);

					/*give the buffer back*/
					if(aio->pCallBack != OSAL_NULL)
					{
#ifdef ENABLE_REALTIME_TIMEOUT_DIAG
						dabdevice_members_data[device_id].callback_timeout_check.starttime =
								OSAL_ClockGetElapsedTime();
#endif /* ENABLE_REALTIME_TIMEOUT_DIAG */
						/*release the sem because it may be used in  the callback*/
						OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
						/*send out the buffer*/
						aio->pCallBack(aio->pvArg);
						/*and again lock it*/
						OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].sem,OSAL_C_TIMEOUT_FOREVER);
#ifdef ENABLE_REALTIME_TIMEOUT_DIAG
						if( OSAL_ThreadWhoAmI() == DABDEV_ThreadID )
							if( (OSAL_ClockGetElapsedTime()-dabdevice_members_data[device_id].callback_timeout_check.starttime) > DABDEV_CALLBACK_TIMEOUT )
							{
								dabdevice_members_data[device_id].callback_timeout_check.msc_callback_timeout_counter++;
							}
#endif /* ENABLE_REALTIME_TIMEOUT_DIAG */
					}
					/*buffer is now given back and can be used again*/
				}
				else
				{
					OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
#if 0
					dabdev_unlock_mem_blk_ptr(init_d, NULL);
#endif
					OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].sem,OSAL_C_TIMEOUT_FOREVER);
				}
			}
		}
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_USER_1)
		OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_DEV_DAB,
				"dabdev-> OSAL_C_S32_IOCTRL_DAB_READ_CLOSE_CHANNEL done\n");
#endif
#endif
#if 0
		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif
		/*release the sem before returning*/
		OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
		return OSAL_OK;

#if 0
#if 0
    case OSAL_C_S32_IOCTRL_DAB_IOREAD_CALLBACK:
      /*currently not used*/
      dabdevice_members_data[device_id].s32ReadCallbackFunction = s32ArgPar;

	 /*reset the prio to the initial value*/
	 OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

 	 /*release the sem before returning*/
 	 OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
     return OSAL_OK;
#endif

   case OSAL_C_S32_IOCTRL_DAB_SETUP_SUBCH_BUFFER_CALLBACK:
      /*up to now these function is only used by api handler for setting up subchannel buffer*/
      dabdevice_members_data[device_id].s32SetupSubchBufferCallback.pCallBack = ((OSAL_trAsyncDABDevSetupSubchBuffer*)s32ArgPar)->pCallBack;
      dabdevice_members_data[device_id].s32SetupSubchBufferCallback.pvArg     = ((OSAL_trAsyncDABDevSetupSubchBuffer*)s32ArgPar)->pvArg;

	  /*reset the prio to the initial value*/
	  OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

      /*release the sem before returning*/
  	  OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
      return OSAL_OK;

   case OSAL_C_S32_IOCTRL_DAB_READ_ASYNC_FIC:
      /*store the AIO buffer for using it when ll dabdev has fic ready*/
      for (tmp_idx=0;tmp_idx<FIC_BUFFER_NUMB_PER_DABDEV;tmp_idx++)
      {
        if (dabdevice_members_data[device_id].fic_buffer[open_idx][tmp_idx] == OSAL_NULL)
    	 {
        	dabdevice_members_data[device_id].fic_buffer[open_idx][tmp_idx]    = (OSAL_trAsyncControl*)s32ArgPar;
        	break;
    	 }
      }

      if( tmp_idx == FIC_BUFFER_NUMB_PER_DABDEV )
  	  {
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_ERRORS)
	   OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_DEV_DAB, "dabdev->OSAL_C_S32_IOCTRL_DAB_READ_ASYNC_FIC -> OSAL_ERROR\n");
#endif
#endif
		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

		/*release the sem before returning*/
		OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
       return OSAL_ERROR;
	  }
      else
	  {
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_USER_1)
	   OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_DEV_DAB, "dabdev->OSAL_C_S32_IOCTRL_DAB_READ_ASYNC_FIC done\n");
#endif
#endif
		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

		/*release the sem before returning*/
		OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
	   return OSAL_OK;
	  }
#endif // 0

	case OSAL_C_S32_IOCTRL_DAB_DEVICE_DCSR:
	   if (s32ArgPar==0)
	   {
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if    (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_ERRORS)
	    OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_DEV_DAB,
	      "OSAL_C_S32_IOCTRL_DAB_DEVICE_DCSR => ptr to DCSR message=0\n");
#endif
#endif
#if 0
		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif // 0
	    OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
	    return OSAL_ERROR;
	   }
	   else
	   /*send dcsr msg to ADR3a*/
	   {
		volatile tU32 buffer_len = ((*(char*) s32ArgPar) << 8)
				| (*((char*) s32ArgPar + 1));

#if defined(CONFIG_ENABLE_CLASS_DEV_DAB_DCSR) && (CONFIG_ENABLE_CLASS_DEV_DAB_DCSR >= TR_LEVEL_COMPONENT)
		if ((*((char*) s32ArgPar+2)==(char)DABDEV_DCSR_MANUFACTURER_SPECIFIC_NOTIFICATION)&&
			(*((char*) s32ArgPar+3)==(char)DABDEV_DCSR_MS_NOTIFY_CHANNEL)&&
			(DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_TYPE((char*) s32ArgPar)==
					DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_TYPE__PAD_FRAME))
		{
			OSALUTIL_s32TraceWrite(0, TR_LEVEL_COMPONENT, (TR_CLASS_DEV_DAB_DCSR_MSNTFCH&0xffffff00)|device_id, (tPCS8)s32ArgPar, buffer_len+2);
		}
		else
		{
			OSALUTIL_s32TraceWrite(0, TR_LEVEL_COMPONENT, (TR_CLASS_DEV_DAB_DCSR_CMD&0xffffff00)|device_id, (tPCS8)s32ArgPar, buffer_len+2);
		}
#endif

        /*check if we have the PAD for TigerWare*/
		if ((*((char*) s32ArgPar+2)==(char)DABDEV_DCSR_MANUFACTURER_SPECIFIC_NOTIFICATION)&&
			(*((char*) s32ArgPar+3)==(char)DABDEV_DCSR_MS_NOTIFY_CHANNEL)&&
			(DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_TYPE((char*) s32ArgPar)==
					DABDEV_DCSR_MS_NOTIFY_CHANNEL__CHANNEL_TYPE__PAD_FRAME))
		{
#ifdef CONFIG_SOURCE_OF_PAD_STREAMDECODER
			uint tmp_idx2,open_idx2;
			uint subchid = DABDEV_DCSR_MS_NOTIFY_CHANNEL__SUBCH_ID (s32ArgPar);

#if defined(CONFIG_ENABLE_CLASS_DEV_DAB) && (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_COMPONENT)
		    /*we have got some PAD, put it into the buffer and give it to TigerWare*/
            OSALUTIL_s32TracePrintf(0, TR_LEVEL_COMPONENT, TR_CLASS_DEV_DAB,
            		"OSAL_C_S32_IOCTRL_DAB_DEVICE_DCSR => got the PAD\n");
#endif
				if (buffer_len == DABDEV_DCSR_MS_NOTIFY_CHANNEL__HEADER_LEN)
				{
					volatile static int count=0;

#if defined(CONFIG_ENABLE_CLASS_DEV_DAB) && (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_COMPONENT)
					OSALUTIL_s32TracePrintf(0, TR_LEVEL_COMPONENT, TR_CLASS_DEV_DAB,
							"OSAL_C_S32_IOCTRL_DAB_DEVICE_DCSR => no PAD data in MS_NOTIFY_CHANNEL found\n");
#endif
#if 0
					/*reset the prio to the initial value*/
					OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif // 0
					OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
					count ++;
					return OSAL_OK;
				}
#if 0
				{
					static char old_buffer[512]= {0};
					char * new_buffer = (char*) s32ArgPar + 2;
					boolean unequal = FALSE;
					int i;

					for (i=0;i<buffer_len;i++)
					{
					 if (old_buffer[i] != new_buffer[i])
					 {
						old_buffer[i] = new_buffer[i];
						unequal = TRUE;
					 }
					}

					if (unequal==FALSE)
					{
					    volatile static int equal_cnt=0;

						/*reset the prio to the initial value*/
						OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

						OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
						equal_cnt++;
						return OSAL_OK;
					}
				}
#endif
        	/*search for a free PAD buffer*/
            for (open_idx2=0;open_idx2<CONFIG_MAX_OPEN_PER_DAB_DEV;open_idx2++)
        	for (tmp_idx2=0; tmp_idx2<PAD_BUFFER_NUMB_PER_DABDEV;tmp_idx2++)
        	{
        	 if (dabdevice_members_data[device_id].pad_buffer[open_idx2][tmp_idx2]!=NULL)
        	 if ((dabdevice_members_data[device_id].pad_buff_subch_id[open_idx2][tmp_idx2]     == subchid)&&
        		 (dabdevice_members_data[device_id].pad_buffer[open_idx2][tmp_idx2]->u32Length >= buffer_len))
        	  {
        	   /*store this buffer temporarily to send it out*/
        	   OSAL_trAsyncControl* aio2 = dabdevice_members_data[device_id].pad_buffer[open_idx2][tmp_idx2];

               /* now copy the PAD data to the pad buffer */
        	   OSAL_pvMemoryCopy (aio2->pvBuffer,(char*) s32ArgPar,buffer_len);

               /*so data copied send it out now*/
        	   /*clean up the members data*/
        	   dabdevice_members_data[device_id].pad_buffer[open_idx2][tmp_idx2]        = NULL;
        	   dabdevice_members_data[device_id].pad_buff_subch_id[open_idx2][tmp_idx2] = 0xFF;

        	   /*store the length and error status*/
        	   aio2->s32Status    = buffer_len;
        	   aio2->u32ErrorCode = OSAL_OK;

        	   /*send it out*/
        	   if(aio2->pCallBack != OSAL_NULL)
        		{
#ifdef ENABLE_REALTIME_TIMEOUT_DIAG
       			dabdevice_members_data[device_id].callback_timeout_check.starttime =
       					   OSAL_ClockGetElapsedTime();
#endif /* ENABLE_REALTIME_TIMEOUT_DIAG */
       			 /*release the sem because it may be used in  the callback*/
        		 OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
        		 /*send out the buffer*/
        		 aio2->pCallBack(aio2->pvArg);
        		 /*and again lock it*/
        		 OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].sem,OSAL_C_TIMEOUT_FOREVER);
#ifdef ENABLE_REALTIME_TIMEOUT_DIAG
     			if( OSAL_ThreadWhoAmI() == DABDEV_ThreadID )
        			if( (OSAL_ClockGetElapsedTime()-dabdevice_members_data[device_id].callback_timeout_check.starttime) > DABDEV_CALLBACK_TIMEOUT )
        			{
        				   dabdevice_members_data[device_id].callback_timeout_check.pad_callback_timeout_counter++;
        			}
#endif /* ENABLE_REALTIME_TIMEOUT_DIAG */
        		 ret_val = OSAL_OK;
        		}
        	   /*break the for loop*/
        	   break;
        	  }
        	}

#endif
        	if (ret_val!=OSAL_OK)
        	{
#if defined(CONFIG_ENABLE_CLASS_DEV_DAB) && (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_COMPONENT)
              OSALUTIL_s32TracePrintf(0, TR_LEVEL_COMPONENT, TR_CLASS_DEV_DAB,
                		"OSAL_C_S32_IOCTRL_DAB_DEVICE_DCSR => no PAD buffer found\n");
#endif
#if 0
      		  /*reset the prio to the initial value*/
      		  OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif // 0
              OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
              return ret_val;
        	}
        	else
        	{
#if 0
        	  /*reset the prio to the initial value*/
        	  OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif // 0
              OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
              return ret_val;
        	}
		}

		/*release the sem before returning*/
		OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
#if 0
		/*wait for the sema to prevent mutiple cmd at a time*/
		OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].wait_until_cmd_ready_sem,OSAL_C_TIMEOUT_FOREVER);
		/*put the dcsr request in the box*/
		if (DABDEV_s32_push_dcsr_request_to_task(init_d, (tU8*) s32ArgPar,
				buffer_len + 2) == OSAL_ERROR)
		{
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_ERRORS)
			OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_DEV_DAB,
					"dabdev->OSAL_C_S32_IOCTRL_DAB_DEVICE_DCSR -> OSAL_ERROR\n");
#endif
#endif
#if 0
			/*reset the prio to the initial value*/
			OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif // 0
			return OSAL_ERROR;
		}
		else
		{
			/*the response should be in the buffer now*/
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_USER_1)
			OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_DEV_DAB,
					"dabdev->OSAL_C_S32_IOCTRL_DAB_DEVICE_DCSR done\n");
#endif
#endif
#if 0
			/*reset the prio to the initial value*/
			OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif // 0
			return OSAL_OK;
		}
#else
		return OSAL_OK;
#endif // 0
	}

    case OSAL_C_S32_IOCTRL_DAB_ASYNC_MS_NOTIFY_CHANNEL_SELECTION:
      /*store the AIO buffer for using it when ll dabdev has ms_notify_channel_selection ready*/
      if (dabdevice_members_data[device_id].ms_notify_channel_selection_used[open_idx] == FALSE)
      {
        dabdevice_members_data[device_id].ms_notify_channel_selection_used[open_idx]   = TRUE;
        dabdevice_members_data[device_id].ms_notify_channel_selection_buffer[open_idx] = (OSAL_trAsyncControl*)s32ArgPar;
      }
      else
  	  {
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_ERRORS)
	   OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_DEV_DAB, "dabdev->OSAL_C_S32_IOCTRL_DAB_ASYNC_MS_NOTIFY_CHANNEL_SELECTION -> OSAL_ERROR\n");
#endif
#endif
#if 0
		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif

		/*release the sem before returning*/
		OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
       return OSAL_ERROR;
	  }
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_USER_1)
	  OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_DEV_DAB, "dabdev->OSAL_C_S32_IOCTRL_DAB_ASYNC_MS_NOTIFY_CHANNEL_SELECTION done\n");
#endif
#endif
#if 0
		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);
#endif

		/*release the sem before returning*/
		OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
	  return OSAL_OK;

#if 0
    case OSAL_C_S32_IOCTRL_DAB_READ_ASYNC_NTF:
		for (tmp_idx = 0; tmp_idx < NTF_BUFFER_NUMB_PER_DABDEV; tmp_idx++)
		{
			if (dabdevice_members_data[device_id].ntf_buffer[open_idx][tmp_idx]
					== OSAL_NULL)
			{
				dabdevice_members_data[device_id].ntf_buffer[open_idx][tmp_idx]
						= (OSAL_trAsyncControl*) s32ArgPar;

				/*reset the prio to the initial value*/
				OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

				/*release the sem before returning*/
				OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
			  	  return OSAL_OK;
			}
		}

		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

		/*release the sem before returning*/
		OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
  	  return OSAL_ERROR;

    case OSAL_C_S32_IOCTRL_DAB_CANCEL_READ_ASYNC_NTF:
		for (tmp_idx = 0; tmp_idx < NTF_BUFFER_NUMB_PER_DABDEV; tmp_idx++)
		{
			if (dabdevice_members_data[device_id].ntf_buffer[open_idx][tmp_idx]
					== (OSAL_trAsyncControl*) s32ArgPar)
			{
				dabdevice_members_data[device_id].ntf_buffer[open_idx][tmp_idx]
						= OSAL_NULL;

				/*reset the prio to the initial value*/
				OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

				/*release the sem before returning*/
				OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
			  	  return OSAL_OK;
			}
		}

		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

		/*release the sem before returning*/
		OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
  	  return OSAL_ERROR;

#ifdef OSAL_C_S32_IOCTRL_DAB_SET_TUNER_PATH
    case OSAL_C_S32_IOCTRL_DAB_SET_TUNER_PATH:
		{
			tU8 dcsr_cmd_set_tuner_path[8];

			dcsr_cmd_set_tuner_path[0] = 0;
			dcsr_cmd_set_tuner_path[1] = 6;
			dcsr_cmd_set_tuner_path[2] = DABDEV_DCSR_CMD_MANUFACTURER_SPECIFIC_COMMAND;
  		    dcsr_cmd_set_tuner_path[3] = DABDEV_DCSR_MS_CMD_SET_TUNER_PATH;
			dcsr_cmd_set_tuner_path[4] = (s32ArgPar>>24) & 0xff;  /*the tuner path*/
			dcsr_cmd_set_tuner_path[5] = (s32ArgPar>>16) & 0xff;  /*the tuner path*/
			dcsr_cmd_set_tuner_path[6] = (s32ArgPar>>8)  & 0xff;  /*the tuner path*/
			dcsr_cmd_set_tuner_path[7] = s32ArgPar       & 0xff;  /*the tuner path*/

			/*release the sem before returning*/
			OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
			/*wait for the sema to prevent mutiple cmd at a time*/
			OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].wait_until_cmd_ready_sem,OSAL_C_TIMEOUT_FOREVER);

			/*put the dcsr request in the box*/
			if (DABDEV_s32_push_dcsr_request_to_task(init_d, &dcsr_cmd_set_tuner_path[0], 8) == OSAL_ERROR)
			{
	#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
	#if (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_ERRORS)
				OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_DEV_DAB,
						"dabdev->OSAL_C_S32_IOCTRL_DAB_SET_TUNER_PATH -> OSAL_ERROR\n");
	#endif
	#endif
				/*reset the prio to the initial value*/
				OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

				return OSAL_ERROR;
			}
			else
			{
				/*the response should be in the buffer now*/
	#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
	#if (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_USER_1)
				OSALUTIL_s32TracePrintf(0, TR_LEVEL_USER_1, TR_CLASS_DEV_DAB,
						"dabdev->OSAL_C_S32_IOCTRL_DAB_SET_TUNER_PATH done\n");
	#endif
	#endif

				/*reset the prio to the initial value*/
				OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

				return OSAL_OK;
			}
		}
#endif /* OSAL_C_S32_IOCTRL_DAB_SET_TUNER_PATH */
#endif

    default:
#ifdef CONFIG_ENABLE_CLASS_DEV_DAB
#if (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_ERRORS)
	    /* this should not happen*/
	    OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_DEV_DAB,
	    		"OSAL_dabdev->OSAL_C_S32_IOCTRL_....... -> undefined switch (%08x %08x %08x)\n", fd, s32FunPar, s32ArgPar);
#endif
#endif

		/*reset the prio to the initial value*/
		OSAL_s32ThreadPriority(tid, u32oldCurrentPriority);

    	/*release the sem before returning*/
    	OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
	    return OSAL_ERROR;
	}

	return OSAL_OK;
}

//----------------------------------------------------------------------
// Global function implementations
//----------------------------------------------------------------------

/*!
*******************************************************************************
  @brief        <b> Generates the manufacturer specific DCSR message 'ms_notify_channel_selection'</b>

  @param[in]    app     				device application
  @param[in]	output_interface_id		id of output interface
  @param[in]	action_onoff			switch subchannel on/off
  @param[in]	mode					audio mode DAB=1, DAB+=2, DMB=3, PM=4, ...
  @param[in]	subch_id				subchannel id
  @param[in]	subch_size				subchannel size

  @return       error status
  @sa           n.a.
  @callgraph
  @callergraph
*******************************************************************************
*/
tS32 add_api_ms_notify_channel_selection(DABMW_mwAppTy app, uint8 output_interface_id, uint8 action_onoff, uint8 mode, uint8 subch_id, uint16 subch_size)
{
    uint8* response_buffer /*= get_response_buffer()*/;
    uint8*  out_buf /*  = & response_buffer[2]*/; /* len in 2 byte */
    uint32  num_bytes = 0, i;
    tU8 device_id, open_idx;
    tS32 ret = OSAL_OK;
    OSAL_trAsyncControl *pAsyncCtrl;

    /* get device_id from app */
    if (app == DABMW_MAIN_AUDIO_DAB_APP)
    {
        device_id = 0;
    }
    else if (app == DABMW_SECONDARY_AUDIO_DAB_APP)
    {
        device_id = 1;
    }
    else
    {
        ret = OSAL_ERROR_INVALID_PARAM;
    }
    if ((ret == OSAL_OK) && 
        (OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].ms_notify_channel_selection_sem, OSAL_C_TIMEOUT_FOREVER) == OSAL_OK))
    {
        open_idx = 0xFF;
        for(i = 0; i < CONFIG_MAX_OPEN_PER_DAB_DEV; i++)
        {
            if ((dabdevice_members_data[device_id].ms_notify_channel_selection_buffer[i] != NULL) &&
                (dabdevice_members_data[device_id].ms_notify_channel_selection_buffer[i]->pvBuffer != NULL) &&
                (dabdevice_members_data[device_id].ms_notify_channel_selection_used[i] == TRUE))
            {
                open_idx = i;
                break;
            }
        }
        if (open_idx < CONFIG_MAX_OPEN_PER_DAB_DEV)
        {
            pAsyncCtrl = (OSAL_trAsyncControl *)(dabdevice_members_data[device_id].ms_notify_channel_selection_buffer[open_idx]);
            response_buffer = pAsyncCtrl->pvBuffer;
            out_buf = &(response_buffer[2]);

#if (defined(CONFIG_ENABLE_CLASS_DEV_DAB) && (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_COMPONENT))
            OSALUTIL_s32TracePrintf(0, TR_LEVEL_COMPONENT, TR_CLASS_DEV_DAB, "\nadd_api_ms_notify_channel_selection() called\n");
            OSALUTIL_s32TracePrintf(0, TR_LEVEL_COMPONENT, TR_CLASS_DEV_DAB, "\nAPI:         add_api_ms_notify_channel_selection(app %d out_if %d act %d mode %d subch %d subsz %d)",
                app, output_interface_id, action_onoff, mode, subch_id, subch_size);
#endif

            *out_buf++ = DABDEV_DCSR_MANUFACTURER_SPECIFIC_NOTIFICATION;
            *out_buf++ = DABDEV_DCSR_MS_NOTIFY_CHANNEL_SELECTION;
    
            //*out_buf++ = device_id; deactivated because of adding output_interface_id Bertram/Klaas
            //                        device_id will be handled by using different LUNs on SSI

            *out_buf++ = output_interface_id;
            *out_buf++ = (tU8)(action_onoff & 0x0F) + (tU8)((mode & 0x0F)<<4) ;
            *out_buf++ = subch_id;
            *out_buf++ = ((subch_size) >> 8) & 0xff;
            *out_buf++ = ((subch_size) >> 0) & 0xff;

            num_bytes = (uint32)(out_buf - &(response_buffer[0]));
            response_buffer[0]= (uint8)((num_bytes-2) >> 8);
            response_buffer[1]=(uint8)((num_bytes - 2) & 0xff);

            pAsyncCtrl->s32Status = num_bytes;

            dabdevice_members_data[device_id].ms_notify_channel_selection_used[open_idx] = FALSE;
            pAsyncCtrl->pCallBack(pAsyncCtrl->pvArg);
        }
        else
        {
            ret = OSAL_ERROR;
        }
        OSAL_s32SemaphorePost(dabdevice_members_data[device_id].ms_notify_channel_selection_sem);
    }
    return ret;
}

//!
//! \brief     <i><b> Forward DAB audio RAW to StreamDecoder </b></i>
//! \details   Find an empty msc_buffer and copy the DAB audio RAW int it and calls
//!            ms_notify_channel_callback with parameter type MSNtfChannelAsyncBufferType
//! \param[in] pBuffer             DAB audio raw data buffer pointer
//! \param[in] dwActualBufferSize  Size in bytes of data pointed by pBuffer
//! \param[in] status              data block status
//! \param[in] pvContext           callback context pointer, this pointer should 
//!                                be set to Receiver handle value (not a pointer)
//! \return    tVoid
//! \sa n.a.
//! \callgraph
//! \callergraph
//!
tVoid DABDEV_CbDataPath_dab_audio_raw(tU8* pBuffer, tU32 dwActualBufferSize, EtalDataBlockStatusTy *status, void* pvContext)
{
	MMMsNotifyChannel *paudio_raw_header = (MMMsNotifyChannel *)pBuffer;
//	MSNtfChannelAsyncBufferType asyncbuf;
	etalReceiverStatusTy *recvp;
	tU8  device_id, open_idx, tmp_idx, subch_id;
	tS32 ret = OSAL_OK, num_bytes;
	OSAL_trAsyncControl *aio;
	tBool broke;

	// MMMsNotifyChannel = OSAL_pvMemBlockCollectionGetRefByKey(in_memblock, CONFIG_STREMDEC_MEMBLKCOLLECTION_KEY__DAB_SUBCH_INFO )

	// get device_id from dab application (pvContext == hReceiver)
	recvp = ETAL_receiverGet((ETAL_HANDLE)(tU32)pvContext);
	if (recvp->MDRConfig.application == DABMW_MAIN_AUDIO_DAB_APP)
	{
		device_id = 0;
	}
	else if (recvp->MDRConfig.application == DABMW_SECONDARY_AUDIO_DAB_APP)
	{
		device_id = 1;
	}
	else
	{
#if (defined(CONFIG_ENABLE_CLASS_DEV_DAB) && (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_ERRORS))
		OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_DEV_DAB, "DABDEV_CbDataPath invalid app");
#endif
		ret = OSAL_ERROR;
	}

	if (paudio_raw_header == OSAL_NULL)
	{
#if (defined(CONFIG_ENABLE_CLASS_DEV_DAB) && (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_ERRORS))
		OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_DEV_DAB, "DABDEV_CbDataPath pBuff NULL");
#endif
		ret = OSAL_ERROR;
	}
	if (ret == OSAL_OK)
	{
		num_bytes = ((paudio_raw_header->msgAccess.len_msb << 8) + paudio_raw_header->msgAccess.len_lsb) + 2;
		subch_id = paudio_raw_header->msgAccess.subch_id;
		// check length
		if (num_bytes != 
			((paudio_raw_header->msgAccess.channel_data_size_msb << 8) + paudio_raw_header->msgAccess.channel_data_size_lsb) + 
			sizeof(MMMsNotifyChannel))
		{
#if (defined(CONFIG_ENABLE_CLASS_DEV_DAB) && (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_ERRORS))
		OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_DEV_DAB, "DABDEV_CbDataPath_dab_audio_raw pBuffer len %d %d", num_bytes, 
		((paudio_raw_header->msgAccess.channel_data_size_msb << 8) + paudio_raw_header->msgAccess.channel_data_size_lsb + sizeof(MMMsNotifyChannel)));
#endif
			ret = OSAL_ERROR;
		}
	}
	if (ret == OSAL_OK)
	{
		/* wait for other tasks to finish */
		if (OSAL_s32SemaphoreWait(dabdevice_members_data[device_id].sem, OSAL_C_TIMEOUT_FOREVER) == OSAL_OK)
		{
			broke = FALSE;
			/* search for this buffer in all instances */
			for (open_idx = 0; ((open_idx < CONFIG_MAX_OPEN_PER_DAB_DEV) && (broke == FALSE)); open_idx++)
			{
				/* search for this buffer in this instance */
				for (tmp_idx = 0; tmp_idx < MSC_BUFFER_NUMB_PER_DABDEV; tmp_idx++)
				{
					/* check if we have a valid msc buffer table entry */
					if ((dabdevice_members_data[device_id].msc_buff_subch_id[open_idx][tmp_idx] != subch_id) ||
						(dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx] == OSAL_NULL) ||
						(dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx]->u32Length < num_bytes) /*||
						(dabdevice_members_data[device_id].msc_buff_at_lldabdev[open_idx][tmp_idx]  != FALSE)*/)
					{
						continue;
					}

					/*store this buffer temporarily to send it out*/
					aio = dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx];

					/*now copy the data to the buffer*/
					OSAL_pvMemoryCopy(aio->pvBuffer, pBuffer, num_bytes);

					/*clean up the members data*/
					dabdevice_members_data[device_id].msc_buffer[open_idx][tmp_idx]        = OSAL_NULL;
					dabdevice_members_data[device_id].msc_buff_subch_id[open_idx][tmp_idx] = 0xFF;
#if (defined(CONFIG_ENABLE_CLASS_DEV_DAB) && (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_COMPONENT))
					{
						static tU8 vl_subch_id = 0;
						static tS32 vl_num_bytes = 0;
						static tU32 vl_cnt = 0;
						if ((subch_id != vl_subch_id) || (vl_num_bytes != num_bytes))
						{
							OSALUTIL_s32TracePrintf(0, TR_LEVEL_COMPONENT, TR_CLASS_DEV_DAB, "DABDEV_CbDataPath_dab_audio_raw subch_id %d, pBuffer len %d num data bytes %d, device_id %d, open_idx %d, tmp_idx %d", 
								subch_id, num_bytes, ((paudio_raw_header->msgAccess.channel_data_size_msb << 8) + paudio_raw_header->msgAccess.channel_data_size_lsb),
								device_id, open_idx, tmp_idx);
							vl_subch_id = subch_id;
							vl_num_bytes = num_bytes;
							// new data !
							vl_cnt = 0;
						}

						vl_cnt++;
#if 0
						if (vl_cnt > 0)
						{
							OSALUTIL_s32TracePrintf(0, TR_LEVEL_COMPONENT, TR_CLASS_DEV_DAB, "DABDEV_CbDataPath_dab_audio_raw subch_id %d, pBuffer len %d num data bytes %d, device_id %d, open_idx %d, tmp_idx %d", 
								subch_id, num_bytes, ((paudio_raw_header->msgAccess.channel_data_size_msb << 8) + paudio_raw_header->msgAccess.channel_data_size_lsb),
								device_id, open_idx, tmp_idx);
							
							// new data !
							vl_cnt = 0;	
						}
#endif

					}

#endif

					/*store the length and error status*/
					aio->s32Status    = num_bytes;
					aio->u32ErrorCode = OSAL_OK;

					/*send it out*/
					if(aio->pCallBack != OSAL_NULL)
					{
						/*send out the buffer*/
						aio->pCallBack(aio->pvArg);
					}
					broke = TRUE;
					break;
				}
			}
			if (broke == FALSE)
			{
#if (defined(CONFIG_ENABLE_CLASS_DEV_DAB) && (CONFIG_ENABLE_CLASS_DEV_DAB >= TR_LEVEL_ERRORS))
				OSALUTIL_s32TracePrintf(0, TR_LEVEL_ERRORS, TR_CLASS_DEV_DAB, "DABDEV_CbDataPath_dab_audio_raw subch_id %d len %d buffer lost", subch_id, num_bytes);
#endif
			}
			/*release the sem before returning*/
			OSAL_s32SemaphorePost(dabdevice_members_data[device_id].sem);
		}
	}
}

//!
//! \brief     <i><b> Check if audio is valid or muted </b></i>
//! \details   Get MDR application from device handle and check if ETAL audio is muted
//! \param[in] fd_dev  device handle
//! \return    TRUE    audio can be sent
//! \return    FALSE   audio is muted or no receiver valid
//! \sa n.a.
//! \callgraph
//! \callergraph
//!
tBool DABMW_Check_SendAudioToAudioChannel(tS32 fd_dev)
{
	ETAL_HANDLE hReceiver;
	tU8 app;
	tU8  init_d    = OSAL_IODESCRIPTOR_2_INTID(fd_dev);
	tU8  device_id = init_d & 0x0f;

	if (device_id == 0)
	{
		app = DABMW_MAIN_AUDIO_DAB_APP;
	}
	else if (device_id == 1)
	{
		app = DABMW_SECONDARY_AUDIO_DAB_APP;
	}
	hReceiver = ETAL_receiverSearchFromApplication(app);
	if (hReceiver != ETAL_INVALID_HANDLE)
	{
		return (ETAL_receiverIsMute(hReceiver) == FALSE);
	}
	return FALSE;
}

//!
//! \brief     <i><b> Check if audio play is allowed </b></i>
//! \details   Get MDR application from device handle and check if ETAL audio is muted
//! \return    TRUE    audio can be sent
//! \return    FALSE   audio is muted or no receiver valid
//! \sa n.a.
//! \callgraph
//! \callergraph
//!
tBool DABMW_Check_PlayAudio(tVoid)
{
	ETAL_HANDLE hReceiver;
	EtalAudioSourceTy source;
	tBool ret;

	ETAL_statusGetAudioSource(&hReceiver, &source);
	if ((hReceiver != ETAL_INVALID_HANDLE) && (source == ETAL_AUDIO_SOURCE_DCOP_STA660))
	{
		ret = TRUE;
	}
	else
	{
		ret = FALSE;
	}

	return ret;
}

#ifdef __cplusplus
}
#endif

// END OF FILE

