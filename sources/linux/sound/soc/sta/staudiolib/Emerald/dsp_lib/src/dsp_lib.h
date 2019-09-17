/*
 * dsp_lib.h
 *
 * Project: dsp_lib
 *
 * Created on 2015/07/17 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * CODE FOR EMERALD DSP
 *
 */

#ifndef _DSP_LIB_H_
#define _DSP_LIB_H_


//---------------------------------------------
//When compiling for Emerald
#ifdef EMERALDCC
#include "emerald.h"	        // This must be always included
#define FRAC(fl)				(fl)
#else
//---------------------------------------------
//When compiling for ARM
//From emerald.h
typedef long 				    fraction;
#define _XMEM
#define _YMEM
#define FRAC(f)					(MIN((fraction)((f)*0x1000000 + 1)/2, 0x7FFFFF))
#pragma pack(push, 1)           /* set struct pack mode to exact fit - no padding */
#endif  /* EMERALDCC */
//---------------------------------------------


#define DSP_LIB_VERSION         1

#define DSP_LOADED_CODE         0xfeed
#define DSP_RUNNING_CODE        0xf00d

#define DSP_X_IF_OFFSET         0x10
#define DSP_Y_IF_OFFSET         0x10

//#define DSP_MAX_IN              10
//#define DSP_MAX_OUT             10


//typedef void (*T_DSP_FUNC)(void *x, void _YMEM *y);
typedef void (*T_DSP_FUNC)(void);

//---------------------------------------------------------------------------
//   DUMMY_ADD
//---------------------------------------------------------------------------

typedef struct {
    int             a;
    int             b;
    int             res;
} T_DSP_DUMMY_ADD_X;

//void dsp_dummy_add(T_DSP_DUMMY_ADD_X *x);
void dsp_dummy_add(void);

//---------------------------------------------------------------------------
//   DUMMY1
//---------------------------------------------------------------------------

typedef struct {
    int             param1;
    int             param2;
    int             param3;
} T_DSP_DUMMY1_X;

typedef struct {
    int             param4;
    int             param5;
    int             param6;
} T_DSP_DUMMY1_Y;

//void dsp_dummy1(T_DSP_DUMMY1_X *x, T_DSP_DUMMY1_Y _YMEM *y);
void dsp_dummy1(void);


//---------------------------------------------------------------------------
//   X/Y interfaces
//---------------------------------------------------------------------------

typedef struct {
    unsigned int            loaded_code;
    unsigned int            running_code;
    unsigned int            lib_version;
    unsigned int            init_counter;
    unsigned int            flag_busy;

//  fraction                in[DSP_MAX_IN];
//  fraction                out[DSP_MAX_OUT];
//  fraction                *pIn;
//  fraction                *pOut;

    T_DSP_FUNC              invoked_func;

    T_DSP_DUMMY_ADD_X       dummy_add_xparams;

    T_DSP_DUMMY1_X          dummy1_xparams;

} T_DSP_X_IF;


typedef struct {
    fraction                loaded_code;

    T_DSP_FUNC              dummy_add_func;

    T_DSP_FUNC              dummy1_func;
    T_DSP_DUMMY1_Y          dummy1_yparams;

} T_DSP_Y_IF;


#endif /*_DSP_LIB_H_ */


