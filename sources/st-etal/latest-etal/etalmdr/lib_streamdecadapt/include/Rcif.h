////////////////////////////////////////////////////////////////////
///  \file  rcif.h
///  \brief <i><b>RCIF header file.</b></i>
///  \author Emanuela Zaccaria
///  \version 0.1
///  \date  March 25th, 2010
///  \bug Unknown.
///  \warning None.
////////////////////////////////////////////////////////////////////
#ifndef RCIF_H
#define RCIF_H
#if 0
#include "types.h"


#define TOT_RCIF_ARGS            12

#define RCIF_POLLING_NO_RTOS       1
#define RCIF_INTERRUPT             2
#define RCIF_POLLING_SEMAPHORE_WS  3
#define RCIF_POLLING_TASK_DELAY    4



/*lint -esym(530,RCIF_Descr)*/
volatile typedef struct _RCIF_Descr
{
   tU32   RCIF_Cmd_ID;     // ID of the remote function
   tU32   RCIF_Timeout;    // It can be set to TIMEOUT_IMMEDIATE or TIMEOUT_INFINITY
   tU32   RCIF_BuffAddr[TOT_RCIF_ARGS]; // Buffer of addresses to the function arguments
   tU32   RCIF_Return;                  // Return value of the remote function
   struct RCIF_Descr   *RCIF_Next;     // Pointer to the next structure
} RCIF_Descr;


//////////////////////////////////////////////////////////////////////////////
//  RCIF_buffer_handler fields;
//  free_ptr        : pointer to free region in data buffer
//  free_size       : free size available
//////////////////////////////////////////////////////////////////////////////
typedef /*volatile*/ struct _RCIF_Buffer
{
  tU32 *free_ptr;
  tU32 free_size;
} RCIF_buffer_handler;



//////////////////////////////////////////////////////////////////////////////
/// pFunction: Enum of all the remote function IDs
//  (*function_address)() : pointer to a specific function (defined in sorting.h)
//  num_of_args           : number of arguments per function
//////////////////////////////////////////////////////////////////////////////
typedef struct _pfun
{
	tS32 (*function_address)();
	tU32 num_of_args;
	tU32 add_of_args;
	tU32 num_of_fields;  // optional (necessary only for arrays);
} pFunction;



tVoid  RCIF_CleanFreeList(tU32 , tU32, volatile RCIF_Descr  **);
tVoid  RCIF_InitDescrFreeList(tU32,  tU32, volatile RCIF_Descr  **);
RCIF_Descr * RCIF_Parse_Fill_Item(tU32 *, tU32, tU32, volatile RCIF_Descr **);
RCIF_Descr * RCIF_PUSH(tU32, volatile RCIF_Descr  **);
RCIF_Descr * RCIF_POP(tU32, volatile RCIF_Descr  **);
tU32  RCIF_DescrHeadSize(RCIF_Descr  *) ;
RCIF_Descr * RCIF_FreeItem (RCIF_Descr  *, volatile RCIF_Descr  **) ;
RCIF_Descr * RCIF_ParseResult(volatile RCIF_Descr  **);
RCIF_Descr * RCIF_FillResult(tU32, tU32 , RCIF_Descr *,  RCIF_Descr  *) ;
RCIF_Descr *  RCIF_ParseIDfun(volatile RCIF_Descr  **);
RCIF_Descr *RCIF_GetHandler(volatile RCIF_Descr  **);
RCIF_Descr * RCIF_Call(tU32 *, tU32, tU32, RCIF_Descr  *, RCIF_Descr  *);
tU32 *RCIF_SharedMemoryAlloc(tU32, RCIF_buffer_handler  *, tU32 *, tU32);
tU32 RCIF_Set_Value(tU32 *, tU32 *,  tU32);
RCIF_Descr * RCIF_ReleaseHandler(RCIF_buffer_handler *, RCIF_buffer_handler *, RCIF_Descr  *, volatile RCIF_Descr **, tU32);
RCIF_Descr  * RCIF_RemoteCall(tU32, volatile RCIF_Descr  **,  RCIF_buffer_handler  *, tU32 *, tU32 *);
tU32 RCIF_ResetBuf (tU32 *, tU32 );
tVoid RCIF_ReceiveCall(RCIF_Descr *  descp,  pFunction *);

tU32 RCIF_ME_wait_1(volatile tU32 *, volatile tU32 *);
tU32 RCIF_ME_wait_2(volatile tU32 *, volatile tU32 *);
tU32 RCIF_ME_synch (volatile tU32 *, volatile tU32 *);
#endif
#endif //  RCIF_H

