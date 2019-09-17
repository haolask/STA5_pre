//!
//!  \file 		osevent.h
//!  \brief 	<i><b> OSAL Event Functions Header File </b></i>
//!  \details	This is the headerfile for the OSAL Abstraction Layer for
//!             common internal data. This Header has to be included
//!				to use functions for Event Handling.
//!  \author 	Luca Pesenti
//!  \author 	(original version) Giovanni Di Martino
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

#ifndef OSEVENT_H
#define OSEVENT_H

#ifdef __cplusplus
extern "C"
{
#endif

//-----------------------------------------------------------------------
//typedefs and struct defs (scope: global)
//-----------------------------------------------------------------------
//!
//! \enum	OSAL_tenEventMaskFlag
//!			list of the way to link different events
//!
typedef enum {
       OSAL_EN_EVENTMASK_AND,
       OSAL_EN_EVENTMASK_OR,
       OSAL_EN_EVENTMASK_XOR,
       OSAL_EN_EVENTMASK_REPLACE
} OSAL_tenEventMaskFlag;

//!
//! \typedef	OSAL_tEventMask
//!			The type of the event mask
//!
typedef tU32 OSAL_tEventMask;
//!
//! \typedef OSAL_tEventHandle
//!			The OSAL event Handle Type
//!
typedef tU32 OSAL_tEventHandle;
//-----------------------------------------------------------------------
// function prototypes (scope: global)
//-----------------------------------------------------------------------
#if defined(CONFIG_HOST_OS_LINUX) || defined (CONFIG_HOST_OS_WIN32)
extern void Linux_initEvent(void);
extern void Linux_deinitEvent(void);
#endif
extern    tS32 OSAL_s32EventCreate(tCString coszName, OSAL_tEventHandle* phEvent);
#ifndef OSAL_EVENT_SKIP_NAMES
    extern    tS32 OSAL_s32EventOpen(tCString coszName, OSAL_tEventHandle* phEvent);
    extern    tS32 OSAL_s32EventClose(OSAL_tEventHandle hEvent);
    extern    tS32 OSAL_s32EventDelete(tCString coszName);
#else
    extern      tS32 OSAL_s32EventFree(OSAL_tEventHandle hEvent);
#endif
extern	  tS32 OSAL_s32EventWaitForDemo(OSAL_tEventHandle hEvent,OSAL_tEventMask mask, OSAL_tenEventMaskFlag enFlags, OSAL_tMSecond msec,OSAL_tEventMask* pResultMask);
extern    tS32 OSAL_s32EventWait(OSAL_tEventHandle hEvent,OSAL_tEventMask mask, OSAL_tenEventMaskFlag enFlags, OSAL_tMSecond msec,OSAL_tEventMask* pResultMask);
extern    tS32 OSAL_s32EventPost(OSAL_tEventHandle hEvent,OSAL_tEventMask mask, OSAL_tenEventMaskFlag enFlags);
extern    tS32 OSAL_s32EventStatus(OSAL_tEventHandle hEvent,OSAL_tEventMask mask, OSAL_tEventMask* pMask);
extern    tS32 OSAL_s32EventClear(OSAL_tEventHandle hEvent, OSAL_tEventMask mask);

   #ifdef __cplusplus
}
#endif


#else
#error osevent.h included several times
#endif

// EOF

