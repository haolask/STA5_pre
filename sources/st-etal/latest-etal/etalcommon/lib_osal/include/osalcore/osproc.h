//!
//!  \file 		osproc.h
//!  \brief 	<i><b> OSAL Thread-Functions Header File </b></i>
//!  \details	This is the headerfile for the OS - Abstraction Layer for
//!             Process and Thread-Functions. This Header has to be
//!             included to use the functions for Thread control
//!  \author 	Luca Pesenti
//!  \author 	(original version) Giovanni Di Martino
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

#ifndef OSPROC_H
#define OSPROC_H

#ifdef __cplusplus
extern "C"
{
#endif

//-----------------------------------------------------------------------
//typedefs and struct defs (scope: global)
//-----------------------------------------------------------------------

//!
//! \typedef Pointer to the routine associated to the thread
//!
#ifdef CONFIG_HOST_OS_TKERNEL
typedef tVoid (*OSAL_tpfThreadEntry) (tSInt stacd, tPVoid thread_param);
#else
typedef tVoid (*OSAL_tpfThreadEntry) ( tPVoid );
#endif
//!
//! \typedef The OSAL Process Handle Type
//!
typedef tS32 OSAL_tProcessID;
//!
//! \typedef The OSAL Thread Handle Type
//!
typedef int OSAL_tThreadID;

//!
//! struct This struct type store the function used to create a new Thread
//!
typedef struct
{
  tString szName;                   // Name
  tU32 u32Priority;                 // Priority
  tS32 s32StackSize;                // Stack Size
  OSAL_tpfThreadEntry pfEntry;      // Started routine
  tPVoid pvArg;                     // Argument of the Started routine
} OSAL_trThreadAttribute;

//-----------------------------------------------------------------------
// function prototypes (scope: global)
//-----------------------------------------------------------------------
extern tSInt Linux_initProcesses(void);
extern tSInt Linux_deinitProcesses(void);
extern OSAL_tThreadID OSAL_ThreadCreate(const OSAL_trThreadAttribute* pcorAttr);
extern OSAL_tThreadID OSAL_ThreadSpawn(const OSAL_trThreadAttribute* pcorAttr);
extern tS32 OSAL_s32ThreadDelete(OSAL_tThreadID tid);
extern tS32 /*@alt void@*/ OSAL_s32ThreadWait(OSAL_tMSecond msec);
extern tS32 OSAL_s32ThreadWait_us( OSAL_tuSecond usec );
extern OSAL_tThreadID OSAL_ThreadWhoAmI( void );
#if CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST
extern tS32 OSAL_s32GetPriority(OSAL_tThreadID tid, tU32 * pu32CurrentPriority);
#endif
extern tVoid OSAL_vThreadExit( void );

#define OSAL_C_U32_THREAD_SLEEP_NEVER   0

#ifdef __cplusplus
}
#endif

#else
#error osproc.h included several times
#endif

// EOF
