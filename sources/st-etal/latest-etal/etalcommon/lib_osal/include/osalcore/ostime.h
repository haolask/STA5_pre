//!
//!  \file 		ostime.h
//!  \brief 	<i><b> OSAL Timer-Functions Header File </b></i>
//!  \details	This is the headerfile for the OS - Abstraction Layer for
//!             Timer-Functions. This Header has to be included to use the
//!             functions for Timer control
//!  \author 	Luca Pesenti
//!  \author 	(original version) Giovanni Di Martino
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

#ifndef OSTIME_H
#define OSTIME_H

#ifdef __cplusplus
extern "C"
{
#endif

//-----------------------------------------------------------------------
//typedefs and struct defs (scope: global)
//-----------------------------------------------------------------------
//!
//! \struct OSAL_trTimeDate
//!			This struct type stores the time information
//!
typedef struct
{
   tS32 s32Second;             // second               [0...61] 1)
   tS32 s32Minute;             // minute               [0...59]
   tS32 s32Hour;               // hour                 [0...23]
   tS32 s32Day;                // day                  [1...31]
   tS32 s32Month;              // month                [1...12]
   tS32 s32Year;               // year since 1900
   tS32 s32Weekday;            // day since Sunday     [0...6]
   tS32 s32Yearday;            // day since JAN-01     [0...365]
   tS32 s32Daylightsaving;     // daylight saving time [-1, 0, +1] 2)
} OSAL_trTimeDate;

//!
//! \typedef The type of the OSAL time variable
//!
typedef tU32 OSAL_tMSecond;

//!
//! \typedef The type of the OSAL time variable in microseconds
//!
typedef tU32 OSAL_tuSecond;

#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
typedef timer_t OSAL_tTimerHandle;
#endif 

//-----------------------------------------------------------------------
// variable declaration (scope: global)
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// function prototypes (scope: global)
//-----------------------------------------------------------------------
#ifdef CONFIG_ETAL_CPU_IMPROVEMENT
extern tU32 OSAL_u32TimerGetResolution( tVoid );
extern tS32 OSAL_s32TimerCreate(OSAL_tpfCallback pCallback,tPVoid pvArg, OSAL_tTimerHandle* phTimer);
extern tS32 OSAL_s32TimerDelete(OSAL_tTimerHandle hTimer);
extern tS32 OSAL_s32TimerGetTime(OSAL_tTimerHandle hTimer, OSAL_tMSecond* pMSec, OSAL_tMSecond* pInterval);
extern tS32 OSAL_s32TimerSetTime(OSAL_tTimerHandle hTimer, OSAL_tMSecond msec, OSAL_tMSecond interval);
#endif

extern OSAL_tMSecond OSAL_ClockGetElapsedTime( void );
extern void OSAL_ClockResetTime(void);

#ifdef __cplusplus
}
#endif

#else
#error ostime.h included several times
#endif


// EOF
