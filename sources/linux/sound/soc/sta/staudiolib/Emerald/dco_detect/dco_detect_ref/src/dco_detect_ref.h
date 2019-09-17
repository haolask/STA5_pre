/***********************************************************************************/
/*!
*  \file      dco_detect_ref.h
*
*  \brief     <i><b> DC Offset Detection </b></i>
*
*  \details   DC Offset Detection based on ST dsp solution for Accordo2
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Christophe Quarre
*
*  \version   1.0
*
*  \date      2015.03.09
*
*  \bug       Unknown
*
*  \warning   
* 
*  This file is part of <component name> and is dual licensed,
*  ST Proprietary License or GNU General Public License version 2.
*
********************************************************************************
*
* Copyright (c) 2014 STMicroelectronics - All Rights Reserved
* Reproduction and Communication of this document is strictly prohibited
* unless specifically authorized in writing by STMicroelectronics.
* FOR MORE INFORMATION PLEASE READ CAREFULLY THE LICENSE AGREEMENT LOCATED
* IN THE ROOT DIRECTORY OF THIS SOFTWARE PACKAGE.
*
********************************************************************************
*
* ALTERNATIVELY, this software may be distributed under the terms of the
* GNU General Public License ("GPL") version 2, in which case the following
* provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* <component_name> is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* <component_name> is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*/
/***********************************************************************************/

#ifndef _DCO_DETECTION_H_
#define _DCO_DETECTION_H_

#define DCO_VERSION     4


#define lfint(acc)  ((acc) >> 23)
#define lfrac(acc)  ((acc) & 0x7FFFFF)
//note: if saturation not set (rsmode(0, SATURATION)), lfrac could be reduced to a single long-to-frac store:
//define lfrac(acc)  (acc)


//---------------------------------------------------------------------------
//   DCO parameters to calibrate
//---------------------------------------------------------------------------

//for 1 channel
typedef struct {
    int             bulkDelay;              //assumed known (measured)
    fraction        fracDelay;              //assumed < 1
    fraction        dcOffset;               //assumed < 1
    fraction        zcThreshold;            //zero croaaing Threshold (assumed |T-| = T+ < 1 )
    unsigned int    hwZCF;                  //hw Zero Crossing Flag coming from DCO_REG (input)
    unsigned int    swZCF;                  //sw estimated ZCF (output)    
} T_DCOParams;


//---------------------------------------------------------------------------
//   Fractional Delay
//---------------------------------------------------------------------------

#define FDELAY_MAX_LENGTH   50

typedef struct {
    fraction        in;
    fraction        out;            // = x[bulkDelay] * (1-fracDelay) + x[bulkDelay-1] * fracDelay
    fraction        *pwrite;
    fraction        *pbulk;         //ptr to x[bulkDelay]
    fraction        *pbefore;       //ptr to x[bulkDelay-1]
    fraction        buf[FDELAY_MAX_LENGTH];
} T_FracDelayXData;

typedef struct {
    int             bulkDelay;      // < FDELAY_MAX_LENGTH
    fraction        fracDelay;      // < 1
} T_FracDelayXParams;

void fracDelay_init(T_FracDelayXData *x, T_FracDelayXParams _YMEM *y);
void fracDelay_update(T_FracDelayXData *x, T_FracDelayXParams _YMEM *y);
void fracDelay(T_FracDelayXData *x, T_FracDelayXParams _YMEM *y);


//---------------------------------------------------------------------------
//   FRAC DELAY algo Selection
//---------------------------------------------------------------------------

#if 1
//Linear Fraction delay
#define T_FRACDELAY_DATA         T_FracDelayXData
#define T_FRACDELAY_PARAMS       T_FracDelayXParams
#define FRACDELAY_INIT           fracDelay_init
#define FRACDELAY_UPDATE         fracDelay_update
#define FRACDELAY                fracDelay
#endif

//---------------------------------------------------------------------------
//   METERING runtime
//---------------------------------------------------------------------------

/*
typedef struct {
    int              scoreWindow;           //e.g 128, 1000,...
    int              scoreCounter;
    int              scoreTmp;
    int              score;                 //queriable ZCF-matching score per scoreWindow samples
} T_DCOScore;

typedef struct {
    int              numMatch1;
    int              numMatch0;
    int              numMatch01;             // = numMatch0 + numMatch1
    int              numMiss;
    int              numFalse;
} T_DCOStats;
*/

//1 ch
typedef struct {
    fraction            in;
    unsigned int        scoreWindow;           //e.g 128, 1000,...
    unsigned int        scoreCounter;
    unsigned int        scoreTmp;
    unsigned int        score;                 //queriable ZCF-matching score per scoreWindow samples
    unsigned int        alertThres;
    unsigned int        statsMatchTmp[4];     //[match0, missed, false, match1] with entry = (zcf<<1 | hwZCF)
    unsigned int        statsMatch[4];        //queriable stats per scoreWindow samples
    T_FRACDELAY_DATA    fdelayData;
} T_MeteringData;

//1 ch
typedef struct {
    T_DCOParams _YMEM*  pDCOparams;
    T_FRACDELAY_PARAMS  fdelayParams;
} T_MeteringParams;


void metering_init(T_MeteringData *data, T_MeteringParams _YMEM *params, T_DCOParams _YMEM *dcop);
void metering(T_MeteringData *data, T_MeteringParams _YMEM *params);
//void metering_4ch_monozcf(T_Dco1chData *x0, T_Dco1chParams _YMEM *y0);  <-- declared below, after decl T_Dco1chData...

//---------------------------------------------------------------------------
//   FRACDELAY Tuner
//---------------------------------------------------------------------------

//1 ch
typedef struct {
    fraction            in;
    int                 score;         //runtime "earlyDelay vs lateDelay" score per 128 samples (for DEBUG)
    fraction            delayIncr;     //value depending on scores
    int                 scoreCounter;    
    T_FRACDELAY_DATA    earlyData;
    T_FRACDELAY_DATA    lateData;
    void*               pTopData;    //ptr to top 1ch-struct (needed to update other tuners'fracdelay)
} T_DelayTunerData;

//1 ch
typedef struct {
    T_DCOParams _YMEM*  pDCOparams;
    T_FRACDELAY_PARAMS  earlyParams;
    T_FRACDELAY_PARAMS  lateParams;
    void _YMEM*         pTopParams;    //ptr to top 1ch-struct (needed to update other tuners'fracdelay)    
} T_DelayTunerParams;


void delayTuner_init(T_DelayTunerData *data, T_DelayTunerParams _YMEM *params, T_DCOParams _YMEM *dcop);
void delayTuner(T_DelayTunerData *data, T_DelayTunerParams _YMEM *params);

//---------------------------------------------------------------------------
//   DCO Tuner
//---------------------------------------------------------------------------

//1 ch
typedef struct {
    fraction            in;
    int                 score;          //runtime "dcoUp vs dcoDw" score per 128 samples (FOR DEBUG)
    fraction            dcoIncr;        //value depending on scores
    int                 scoreCounter;   
    T_FRACDELAY_DATA    fdelayData;
} T_DcoTunerData;

//1 ch
typedef struct {
    T_DCOParams _YMEM*  pDCOparams;
    fraction            dcoUp;
    fraction            dcoDw;
    T_FRACDELAY_PARAMS  fdelayParams;
} T_DcoTunerParams;


void dcoffTuner_init(T_DcoTunerData *data, T_DcoTunerParams _YMEM *params, T_DCOParams _YMEM *dcop);
void dcoffTuner(T_DcoTunerData *data, T_DcoTunerParams _YMEM *params);

//---------------------------------------------------------------------------
//   THRESHOLD Tuner
//---------------------------------------------------------------------------

//1 ch
typedef struct {
    fraction            in;
    int                 score;          //runtime "thUp vs thDw" score per 128 samples (FOR DEBUG)
    fraction            thresIncr;      //value depending on scores
    int                 scoreCounter;  
    T_FRACDELAY_DATA    fdelayData;
} T_ThresTunerData;

//1 ch
typedef struct {
    T_DCOParams _YMEM*  pDCOparams;
    fraction            thresUp;
    fraction            thresDw;
    T_FRACDELAY_PARAMS  fdelayParams;
} T_ThresTunerParams;


void thresTuner_init(T_ThresTunerData *data, T_ThresTunerParams _YMEM *params, T_DCOParams _YMEM *dcop);
void thresTuner(T_ThresTunerData *data, T_ThresTunerParams _YMEM *params);

//---------------------------------------------------------------------------
//   DCO Detection (Top level)
//---------------------------------------------------------------------------
typedef struct {
    unsigned int          calibrate;
    unsigned int          run;
    T_DCOParams           dcoParams;
    T_DelayTunerParams    dlTunerParams;
    T_DcoTunerParams      dcTunerParams;
    T_ThresTunerParams    thTunerParams;
    T_MeteringParams      meterParams;
} T_Dco1chParams;

typedef struct {
    fraction              indata;
    unsigned int          DCO_REG;          //aka hwZCF aka WinIn (input) (arrived scaled by << 18)
    unsigned int          dspZCFout;        //output of monoZCF in ch0 (rescaled by << 18 for sending to DAC)
    T_DelayTunerData      dlTunerData;
    T_DcoTunerData        dcTunerData;    
    T_ThresTunerData      thTunerData;
    T_MeteringData        meterData;    
} T_Dco1chData;


void dco_detect_1ch_init(T_Dco1chData *x, T_Dco1chParams _YMEM *y);
void dco_detect_1ch(T_Dco1chData *x, T_Dco1chParams _YMEM *y);

void dco_detect_4ch_init(T_Dco1chData *x, T_Dco1chParams _YMEM *y);
void dco_detect_4ch(T_Dco1chData *x, T_Dco1chParams _YMEM *y);
void dco_detect_4ch_monozcf(T_Dco1chData *x, T_Dco1chParams _YMEM *y);

void metering_4ch_monozcf(T_Dco1chData *x0, T_Dco1chParams _YMEM *y0);

//---------------------------------------------------------------------------
//   globals declaration
//---------------------------------------------------------------------------

#endif
