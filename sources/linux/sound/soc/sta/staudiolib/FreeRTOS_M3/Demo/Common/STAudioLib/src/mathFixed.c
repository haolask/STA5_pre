/***********************************************************************************/
/*!
*
*  \file      mathFixed.c
*
*  \brief     <i><b> STAudioLib maths functions in fixed point </b></i>
*
*  \details
*
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2013/04/22
*
*  \bug       see Readme.txt
*
*  \warning
*
*  This file is part of STAudioLib and is dual licensed,
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
* STAudioLib is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* STAudioLib is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*
*/
/***********************************************************************************/

#include "internal.h"

#if STA_NO_FLOAT

#include "mathFixed.h"

/*
 * /!\ NOTE:
 * The driver internal FIXED representation differs from the
 * Emerald DSP FRACTION representations:
 *
 * CPU FIXED: 		FRAC_1 = 1 << 24
 * DSP FRACTION:	ONE    = (1 << 23)-1 = 0x7FFFFF
 *
 * Use FIXED2FRAC() to update in the DSP
 */

/*
#define FRAC_0_09  		0x0170A3D     // 0.09
#define FRAC_0_25  		0x0400000     // 0.25
#define FRAC_0_364 		0x05D2F1A     // 0.364
#define FRAC_0_475 		0x0799999     // 0.475
#define FRAC_0_75  		0x0C00000     // 0.75
#define FRAC_1_1		0x1199999
#define FRAC_1_125		0x1200000
#define FRAC_1_2		0x1333333
#define FRAC_1_25       0x1400000
#define FRAC_1_4		0x1666666
#define FRAC_1_5        0x1800000
#define FRAC_1_6		0x1999999
#define FRAC_1_8		0x1CCCCCC
*/

#define ABS(x) ((x) >= 0 ? (x) : -(x))


/*******************************************************************************
* Function Name  : F_DIV
* Description    :
* Input          :
* Return         :
*******************************************************************************/
fixpoint F_DIV(fixpoint slNum, fixpoint slDen)
{

  s32 slRetVal;
  u32 ulRetVal;
  u32 ulNum = ABS(slNum);
  u32 ulDen = ABS(slDen);
  u32 ulReduce=1;

  /* Ist der Nenner > 1 ? */
  if (ulDen > FRAC_1)
  {
     /* Dann erstmal so weit reduzieren, bis er < 0.5 ist */
     ulReduce = (ulDen / FRAC_1)+1;
     ulDen = ulDen / ulReduce;
  }


  ulRetVal  = ulNum ;
  ulRetVal  = ulRetVal / ulDen;
  ulRetVal  = ulRetVal  << 24;

  ulNum = (ulNum % ulDen) << 8;
  ulRetVal += (ulNum / ulDen) << 16;
  ulNum = (ulNum % ulDen) << 8;
  ulRetVal += (ulNum / ulDen) <<  8;
  ulNum = (ulNum % ulDen) << 8;
  ulRetVal += (ulNum / ulDen) <<  0;

  ulRetVal /= ulReduce;

  slRetVal = (s32)ulRetVal;

  /* Vorzeichen korrigieren */
  if((slNum < 0) != (slDen < 0))
  {
    slRetVal = -(s32)ulRetVal;
  }


  return slRetVal;
}

/*******************************************************************************
* Function Name  : F_MUL
* Description    :
* Input          :
* Return         :
*******************************************************************************/
#define LONG_ARITHMETIC 0

#if (LONG_ARITHMETIC == 0)
fixpoint F_MUL(fixpoint slFak1, fixpoint slFak2)
{
/* return ulFak1 * ulFak1 and check overflow */
  s32 slFakHi, slComp;
  u32 ulRetVal, ulFak1Low, ulFak2Low, ulFak1High, ulFak2High;

  s32 slRetVal =0;

  ulFak1High = ABS(slFak1) >> 16;
  ulFak2High = ABS(slFak2) >> 16;
  ulFak1Low  = ABS(slFak1) & 0xffff;
  ulFak2Low  = ABS(slFak2) & 0xffff;


  /* check for overflow, no significant bit should get lost through shifting */
  slFakHi = (slFak1 >> 16) * (slFak2 >> 16);

  slComp  = ((u32)0xffffffff << (FRACTION_BASE));
    if (
       (slComp & slFakHi) != 0 &&     /* result is positiv */
       (slComp & slFakHi) !=  slComp  /* result is negativ */
       )
  {
    /*AUD_ASSERT(FALSE);*/
    /*printf("OVERFLOW in dcr_ulMul\n"); */
  }


  ulRetVal = (ulFak1High * ulFak2High  <<  8)
             + (ulFak1High * ulFak2Low   >>  8)
             + (ulFak2High * ulFak1Low   >>  8)
             + ((ulFak1Low  * ulFak2Low)  >> 24);

  slRetVal = (s32)ulRetVal;

  if((slFak1 < 0) != (slFak2 < 0))
  {
      slRetVal = -(s32)ulRetVal;
  }

    return slRetVal;
}
#else //!LONG_ARITHMETIC

#pragma arm

fixpoint F_MUL(volatile fixpoint MUL1, volatile fixpoint MUL2)
{
/*     register r0 = MUL1; */
/*     register r1 = MUL2; */

/*     __asm{ */

/*               smull  r3,r4,r1,r0 */
/*               mov    r4,r4,lsl #8 */
/*               mov    r3,r3,asr #24 */
/*               mov    r0,#0x100 */
/*               sub    r0,r0,#1 */
/*               and    r3,r3,r0 */
/*               orr    r4,r4,r3 */
/*               mov    r0,r4 */
/*           }; */
/*     return r0; */

    fixpoint tmp1;
    fixpoint tmp2;

#ifdef CONFIG_TARGET_CPU_COMPILER_GNU
    __asm__ __volatile__ (
        "smull  %[tmp1], %[tmp2], %[MUL2], %[MUL1]  \n\t"
        "mov    %[tmp2], %[tmp2], lsl #8            \n\t"
        "mov    %[tmp1], %[tmp1], asr #24           \n\t"
        "mov    %[MUL1], #0x100                     \n\t"
        "sub    %[MUL1], %[MUL1], #1                \n\t"
        "and    %[tmp1], %[tmp1], %[MUL1]           \n\t"
        "orr    %[tmp2], %[tmp2], %[tmp1]           \n\t"

        : [tmp1] "+r" (tmp1), [tmp2] "+r" (tmp2),
       	  [MUL1] "+r" (MUL1), [MUL2] "+r" (MUL2)
	);
#else
    __asm{

              smull  tmp1,tmp2,MUL2,MUL1
              mov    tmp2,tmp2,lsl #8
              mov    tmp1,tmp1,asr #24
              mov    MUL1,#0x100
              sub    MUL1,MUL1,#1
              and    tmp1,tmp1,MUL1
	          orr    tmp2,tmp2,tmp1
          };
#endif
    return tmp2;
}

#pragma thumb

#endif //!LONG_ARITHMETIC

/************************************************************************FA*
 *Function          : F_SIN
 *--------------------------------------------------------------------------
 *Description       : approximation for y = sin(x) for x = -PI to PI
 ***********************************************************************FE*/

#define SIN_A1  0x00FFFFCD			      /* A_1 = +0.99999699 */
#define SIN_A3  0xFFD55611			      /* A_3 = -0.16665548 */
#define SIN_A5  0x00022174			      /* A_5 = +0.00832300 */
#define SIN_A7  0xFFFFF33B			      /* A_7 = -0.00019486 */
#define SIN_A9  0x00000025			      /* A_9 = +0.00000225 */


fixpoint F_SIN(fixpoint slwValue)
{
   s32 slwMerk, slwSinus;

   sta_assert(-FRAC_PI <= slwValue && slwValue <= FRAC_PI);

   slwMerk     = F_MUL(slwValue, slwValue);	        // slwMerk    = X^2

   slwSinus    = F_MUL(SIN_A9, slwMerk);	        // slwSinus = A_9 * X^2

   slwSinus    = F_MUL((slwSinus + SIN_A7), slwMerk);   // slwSinus = A_9 * X^4 + A7 * X^2

   slwSinus    = F_MUL((slwSinus + SIN_A5), slwMerk);   // slwSinus = A_9 * X^6 + A7 * X^4 + A5 * X^2

   slwSinus    = F_MUL((slwSinus + SIN_A3), slwMerk);   // slwSinus = A_9 * X^8 + A7 * X^6 + A5 * X^4 + A3 * X^2

   slwSinus    = F_MUL((slwSinus + SIN_A1), slwValue);  // slwSinus = A_9 * X^9 + A7 * X^7 + A5 * X^5 + A3 * X^3 + A1 * X

   return slwSinus;				        // return result of approximation
}


/************************************************************************FA*
 *Function          : F_COS
 *--------------------------------------------------------------------------
 *Description       : approximation for y = cos(x) for x = -PI to PI
 ***********************************************************************FE*/

#define COS_A0 0x01000000			      /* A_0 = +1.00000000 */
#define COS_A2 0xFF800150			      /* A_2 = -0.49998002 */
#define COS_A4 0x000AA7A2			      /* A_4 = +0.04162040 */
#define COS_A6 0xFFFFA6A2			      /* A_6 = -0.00136364 */
#define COS_A8 0x00000152			      /* A_8 = +0.00002017*/


fixpoint F_COS(fixpoint slwValue)
{
   s32 slwMerk, slwCosinus;

   sta_assert(-FRAC_PI <= slwValue && slwValue <= FRAC_PI);

   slwMerk     = F_MUL(slwValue, slwValue);	        // slwMerk    = X^2

   slwCosinus  = F_MUL(COS_A8, slwMerk);	        // slwCosinus = A_8 * X^2

   slwCosinus  = F_MUL((slwCosinus + COS_A6), slwMerk); // slwCosinus = A_8 * X^4 + A_6 * X^2

   slwCosinus  = F_MUL((slwCosinus + COS_A4), slwMerk); // slwCosinus = A_8 * X^6 + A_6 * X^4 + A_4 * X^2

   slwCosinus  = F_MUL((slwCosinus + COS_A2), slwMerk); // slwCosinus = A_8 * X^8 + A_6 * X^6 + A_4 * X^4 + A_2 * X^2

   slwCosinus  = slwCosinus + COS_A0;		        // slwCosinus = A_8 * X^8 + A_6 * X^6 + A_4 * X^4 + A_2 * X^2 + A_0

   return slwCosinus;				        // return result of approximation
}


/************************************************************************FA*
 *Function          : F_SQRT
 *--------------------------------------------------------------------------
 *Description       : approximation for y = sqrt(x)
 ***********************************************************************FE*/

#define SQRT_X0 0x01000000			      /* X_0 = +1.00000000 */
#define SQRT_A0 0x01000000			      /* A_0 = +1.00000000 */
#define SQRT_A1 0x00800000			      /* A_1 = +0.50000000 */
#define SQRT_A2 0xFFE00000			      /* A_2 = -0.12500000 */
#define SQRT_A3 0x00100000			      /* A_3 = +0.06250000 */
#define SQRT_A4 0xFFF60000			      /* A_4 = -0.03906250 */
#define SQRT_A5 0x00070000			      /* A_5 = +0.02734375 */

fixpoint F_SQRT(fixpoint slwValue)
{
   s32 slwSqrt, slwMerk, slwCorrect;			// temp variables for sqrt - approximation

   slwCorrect = FRAC_1;					// init "slwCorrect" (1.000000)


  // scaling of data input for optimizing approximation

   if(slwValue == 0x00000000)				// slwValue == 0.000000 ??
   {
     slwCorrect = 0x00000000;				// slwCorrect = 0.000000
   }
   else
   {
     while(slwValue < 0x00800000)			// slwValue < 0.500000 ??
     {
       slwValue   = slwValue << 1;			// slwValue   = slwValue * 2.0
       slwCorrect = F_MUL(slwCorrect, 0x00B50900);	// slwCorrect = slwCorrect * sqrt(1/2)
     }

     while(slwValue > 0x01000000) 			// slwValue > 1.000000 ??
     {
       slwValue   = slwValue >> 1;			// slwValue   = slwValue / 2.0
       slwCorrect = F_MUL(slwCorrect, FRAC_SQRT2);	// slwCorrect = slwCorrect * sqrt(2)
     }
   }

  // polynomial approximation for y = sqrt(x)

   slwMerk     = slwValue - SQRT_X0;			// slwMerk = X_d = (X - X_0)

   slwSqrt     = F_MUL(SQRT_A5,slwMerk);		// A_5 * X_d

   slwSqrt     = F_MUL((SQRT_A4+slwSqrt),slwMerk);      // A_4 * X_d + A_5 * X_d^2

   slwSqrt     = F_MUL((SQRT_A3+slwSqrt),slwMerk);      // A_3 * X_d + A_4 * X_d^2 + A_5 * X_d^3

   slwSqrt     = F_MUL((SQRT_A2+slwSqrt),slwMerk);      // A_2 * X_d + A_3 * X_d^2 + A_4 * X_d^3 + A_5 * X_d^4

   slwSqrt     = F_MUL((SQRT_A1+slwSqrt),slwMerk);      // A_1 * X_d + A_2 * X_d^2 + A_3 * X_d^3 + A_4 * X_d^4 + A_5 * X_d^5

   slwSqrt     = SQRT_A0 + slwSqrt;		        // A_0 + A_1 * X_d + A_2 * X_d^2 + A_3 * X_d^3 + A_4 * X_d^4 + A_5 * X_d^5

   slwSqrt     = F_MUL(slwSqrt, slwCorrect);	        // correct result of approximation

   return slwSqrt;
}


/******************************************************************************
 * @Function          : u32 DB_to_Linear(s16 swDBWert, s32 slwMaxVolume)
 *-----------------------------------------------------------------------------
 * @Description       : This function will calculate the linear value of a dB
 *                      value ( format: 1/128 dB stepping and positive values
 *                      are equal to negativ dB values. The second paramter is
 *                      the max.limit ( 0 dB value ) for the calculation. This
 *                      value must be in the format: value ( in 1/128 dB stepping
 *                      * 2^16 / 6.02 )
 *-----------------------------------------------------------------------------
 * @Parameter(s)      : swDBWert = Dämpfung 1/128 dB, slwOffset für 0dB Lage
 *****************************************************************************/
#if 0
const u32 uwNewDBTable[] =
{
  0x80000000L,0x7C5B289DL,0x78D0DF9CL,0x75606374L,0x7208F81DL,0x6EC9E6ECL,0x6BA27E65L,
  0x6892121FL,0x6597FA95L,0x62B39509L,0x5FE4435EL,0x5D296BF8L,0x5A82799AL,0x57EEDB48L,
  0x556E0424L,0x52FF6B55L,0x50A28BE6L,0x4E56E4ACL,0x4C1BF829L,0x49F14C70L,0x47D66B0FL,
  0x45CAE0F2L,0x43CE3E4BL,0x41E0167DL,0x40000000L
};


u32 NewDB_to_Linear(s16 swDBWert, s32 slwOffset)
{
   u32 ulwRest, ulwDiff, ulwresult;
   s32 slwLinear;
   u8  ubShift;
   u8  ubIndex;

   slwLinear  = ( swDBWert * 0x2A85 ) + slwOffset; /* DB_WERT * 2^16/6.02dB */

   if( slwLinear < 0 )
   {
      slwLinear = 0;
   }

   ubShift   = ( slwLinear >> 23 );            /* Anzahl wie oft 6.02 in unseren dB Wert paßt                */
   ulwRest   = ( slwLinear & 0x7FFFFF ) * 24;  /* Nun nehmen wir den Rest * 24 um zu erfahen welchen         */
                                               /* 1/4 dB Wert wir aus der Tabelle holen müssen               */
   ubIndex   =   ulwRest >> 23;                /* und ermitteln den Index auf das zugehörige Tabellenelement */

   /* Nun wird noch eine lineare Näherung über den Rest berechnet */
   ulwRest   = ( ulwRest & 0x7FFFFF ) >> 7;    /* Rest auf 16 Bit begrenzen und mit der auf 16Bit begrenzten */
                                               /* Differenz der Stützstellen multipilzieren */

   ulwDiff   = (( uwNewDBTable[ubIndex] - uwNewDBTable[ubIndex+1] ) >> 16 ) * ulwRest;

   /* Tabellenwert um lineare Näherung des Restes verringern und n mal shiften */
   ulwresult = ( uwNewDBTable[ubIndex] - ulwDiff ) >> ubShift;

   /* 0dB Abfangen */
   if( ulwresult < 0x80000000 )
   {
      return ulwresult;
   }
   else
   {
      return 0x7FFFFFFF;
   }

}

/************************************************************************FA*
 *Function          :  F_DBToLinear
 *--------------------------------------------------------------------------
 *Description       :
 ***********************************************************************FE*/
fixpoint F_DBToLinear(s32 swDBValue)
{
   if(swDBValue == 0)
   {
      /* 0dB werden besonders genau zurückgeliefert */
      return(FRAC_1);
   }
   else
   /* Wir können maximal 42.144dB anheben */
   if(swDBValue > 5394)
   {
      swDBValue =  5394;
   }

   return( NewDB_to_Linear( -1 * swDBValue, 0x3800000 ) ); /* Offset ist 42.144 * 128/dB * 2^16 / 6.02 */
}

#endif

/************************************************************************FA*
 *Function          :  QtoFRAC
 *--------------------------------------------------------------------------
 *Description       :
 ***********************************************************************FE*/
fixpoint QtoFRAC(u8 Qfac)
{
   /* Die Auflösung von Q ist in 1/10 steps */

   fixpoint NewQ = Qfac * 0x199999L; /* Also erstmal Q in FRAC umrechnen */
   return NewQ;
}


/************************************************************************FA*
 *Function          :  QCorrect
 *--------------------------------------------------------------------------
 *Description       :
 ***********************************************************************FE*/
//NOT USED
/*
fixpoint QCorrect_BP(fixpoint Qfac, fixpoint V0, u16 frequency)
{

   if( V0 < FRAC_1 )
   {
      Qfac = F_MUL( Qfac, V0);
   }

   if(frequency > 5000)
   {
      Qfac = F_MUL(Qfac, (FRAC_1 - 0x000003B7L*(frequency - 5000)));
   }

   return Qfac;
}
*/
/************************************************************************FA*
 *Function          : QCorrect
 *--------------------------------------------------------------------------
 *Description       : ARM Routine for implementation of MATLAB script
 *                    "Qcorrection" from Joe Henkel.
 ***********************************************************************FE*/
fixpoint QCorrect(fixpoint Qfac, fixpoint V0, u16 frequency)
{

   fixpoint aqc, bqc, cqc, aq15, bq15, cq15, c0, c1;		    // temp variables
   fixpoint k, q2corr, qcomp, qoff15, qoffcorr, qoffset, q2; 	    // temp variables


   aqc		   = 0xFCE1678C;				    // aqc  = -3.119513761 (in Frac)
   bqc	 	   = 0xFF7B65EB;				    // bqc  = -0.517976106 (in Frac)
   cqc		   = 0x01059831;				    // cqc  = +1.021853509 (in Frac)

   aq15		   = 0x0001BDC6;				    // aq15 = +0.006801984 (in Frac)
   bq15		   = 0xFFF04E95;				    // bq15 = -0.061300985 (in Frac)
   cq15		   = 0x00195DF3;				    // cq15 = +0.099089801 (in Frac)

   c0		   = 0x00543569;				    // c0   = +0.328940000 (in Frac)
   c1		   = 0x03A6E978;				    // c1   = +3.652000000 (in Frac)



   if ( V0 < FRAC_1 )						    // check if gain < 1.00
   {
      Qfac 	   = F_MUL( Qfac, V0);				    // Qfac = Qfac * V0
   }

   if(frequency < 2000)						    // check if frequency < 2.000 Hz
   {
     return Qfac;						    // no correction needed !!
   }

   k 		   = frequency * 0x0000016F;			    // k = fc/fs

   q2corr	   = F_MUL(aqc, F_MUL(k, k)) + F_MUL(bqc, k) + cqc; // q2corr = aqc * k^2 + bqc * k + cqc

   if (Qfac >= FRAC_4)						    // qcomp = min(4, q)
    qcomp	   = FRAC_4;
   else
    qcomp	   = Qfac;

   qoff15	   = F_MUL(aq15, F_MUL(qcomp, qcomp)) + F_MUL(bq15, qcomp) + cq15;  // qoff15 = aq15*qcomp^2 + bq15*qcomp + cq15

   qoffcorr	   = FRAC_1 - F_MUL(c1, ABS(c0 - k));		   // qoffcorr = 1 - 3.652* abs(0.32894 - k);

   if (qoffcorr < 0x00000000)					   // qoffcorr = max (0, (1 - 3.652* abs(0.32894 - k)));
   {
     qoffcorr	   = 0x00000000;				   // qoffcorr = 0
   }

   qoffset	   = F_MUL(qoffcorr, qoff15);			   // qoffset = qoffcorr * qoff15;

   q2		   = q2corr + qoffset;				   // q2      = q2corr + qoffset;

   Qfac	   	   = F_MUL(q2, Qfac);			           // Qfac   = q2 * q;

   return Qfac;							   // return corrected quality factor
}


//==============================================================================
// FRACTION (EMERALD Compatible) ONE = (1 << 23) - 1
//==============================================================================


//note: g1 and 1/g1 are always > 0

// dbscale = [1 2 3 4 5 6 7 8 9 10 100] (MATLAB)
//g1 = dec2hex(int(exp((log(10)/20)./dbscale).*ONE + 0.5)) (MATLAB)
//g1 LUT with dbscale from 1 to 10
static const s32 g_g1_lut[11]  = {0x8F9E4C, 0x87959F, 0x8501F5, 0x83BCD7, 0x82FB43, 0x827AD4, 0x821F64, 0x81DAFA, 0x81A5DD, 0x817B6E, 0x8025BE};

//1/g1 = dec2hex(int(1 ./exp((log(10)/20)./dbscale).*ONE + 0.5))  (MATLAB)
//1/g1 LUT with dbscale from 1 to 10
static const s32 g_oog1_lut[11]= {0x721482, 0x78D6FC, 0x7B2E4D, 0x7C5E4D, 0x7D161B, 0x7D913B, 0x7DE978, 0x7E2BCE, 0x7E5F7E, 0x7E88E7, 0x7FDA4B};


/*******************************************************************************
* Function Name  : _STA_db2lin
*-------------------------------------------------------------------------------
* Description    : Generic db to linear, in fixed point (signed .24)
*                  Reciprocal of db = dbscale * 20 * log(g)
*-------------------------------------------------------------------------------
* Input          : db:      Gain in dB, prescaled by 'dbscale'
*                : dbscale: dbscale = 1 to 10 or 100
*                : clamp:   if true, clamp the returned linear gain to ONE
* Output         : rshift:  if rshift != NULL, scales the returned linear gain to stay below ONE
* Return         : linear gain in fixed (signed.24) scaled by >> rshift
*******************************************************************************/

s32 _STA_db2lin(s32 db, s32 dbscale, s32 *rshift, u32 flags)
{
	s64  g1;
	s64  gainx;
	int i;

	sta_assert(((1 <= dbscale) && (dbscale <= 10)) || (dbscale == 100));

	//MUTE
	if (db <= -120 * dbscale)
	{
		gainx = 0;
	}
	//FLAT
	else if (db == 0)
	{
		gainx = ONE;
	}
	//dB to linear
	else
	{
		if (dbscale == 100) {
			dbscale = 11;} //to fetch g1 from the LUT

		//choose g1
		if (db > 0) {
			g1 = g_g1_lut[dbscale-1];
		} else {
			db = -db; //abs
			g1 = g_oog1_lut[dbscale-1]; // = 1/g1
		}

		//compute linear gain by iterations
		gainx = g1;
		for (i = 2; i <= db; i++) {
			gainx = (gainx * g1) >> ONESHIFT; //(approx to /ONE)
		}
	}

	//set/applying shift
	if (rshift)
	{
		//Auto-shift: downscale to stay below ONE
		if (flags & DB_AUTOSHIFT)
		{
			s32 _rshift = 0;
			while (gainx > ONE) {
				gainx  >>= 1;
				_rshift += 1;
			}
			*rshift = _rshift;
		}
		//Fixed shift: prescaling by shift
		else
		{
			gainx >>= *rshift;
		}
	}

	//clamping
	if ((flags & DB_CLAMP) && gainx > ONE) {
		gainx = ONE;} //Clamped


	return gainx;
}

s32 _STA_lin2db(s32 amplitude)
{
    s32 ret = 0;
    s32 temp32, n, r;
    s64 temp64;

	if (amplitude > 0)
	{
#define a_1_328472725 87063
#define b_2_263226475 148323
#define q16_2000_log2 39456604

    // amplitude = (1+r) * 2^n
    // level (dB/100) = 2000 * log10(amplitude/2^23)
    // = 2000 * log10(2) * (log2(amplitude)-23)
    // = 2000 * log10(2) * (log2(1+r)+n-23)

        // compute n
        temp32 = amplitude;
        n = -1;
        while (temp32)
        {
            n++;
            temp32 >>= 1;
        }

        // compute 1+r in q16
        temp64 = amplitude;
        temp64 <<= 16;

        temp64 >>= n;
        temp32 = temp64;

        // compute r in q16
        r = temp32-(1<<16);

        // compute B = (a*r+b) in q16
        temp64 = r;
        temp64 *= a_1_328472725;
        temp64 >>= 16;
        temp64 += b_2_263226475;
        temp32 = temp64;

        // compute A = (1-r)r in q32
        temp64 = 1<<16;
        temp64 -= r;
        temp64 *= r;

        // compute A/B + r in q16
#ifdef LINUX_OS
		temp64 = div64_s64(temp64,temp32);
#else
		temp64 /= temp32;
#endif
        temp64 += r;
        temp32 = temp64;

        // compute C
        temp64 = n-23;
        temp64 <<= 16;
        temp64 += temp32;
        temp32 = temp64;

        // compute 2000*log2*C
        temp64 = q16_2000_log2;
        temp64 *= temp32;
        temp64 += 1<<30;
        temp64 >>= 32;
        ret = temp64;

	} else {
		ret = -14000;
	}

	return ret;
}

/*******************************************************************************
* Function Name  : _STA_TCtoSmoothFactor
*
* Description    : compute k = ooeps^(-1000/(msec*Fs))
*                  = 10^(-10000/(Fs*tc))
*           or     = 100^(-10000/(Fs*tc))=10^(-20000/(Fs*tc))
*
* Input          : tc:  time constant (msec) prescaled by 10
*                       "prescaled" tc <= 10000
*                : alt: return alternate formula: 1 - k
* Return         : k or 1 - k
*******************************************************************************/
s32 _STA_TCtoSmoothFactor(u32 tc, f32 ooeps, bool alt)
{
	//compute
	//incFactor = 10^(-10000 / (Fs*tc))
	//incFactor = 10^((Fs*tc-10000) / (Fs*tc)-1.0)
	//incFactor = 10^((Fs*tc-10000) / (Fs*tc)) / 10.0
	//incFactor = 2^((Fs*tc-10000) / (Fs*tc*log10(2))) / 10.0
	//incFactor = 2^X / 10.0
	//incFactor = 2^(X/4)^4 / 10.0
	//incFactor = (a*X + b)^4 / 10.0

#define tco_m ((s64)445861641) // 1/(4*log10(2)) in Q29
#define tco_a ((s64)642302419) // 1.196381485 in Q29
#define tco_b ((s64)421285875) // 0.784706092 in Q29
#define tco_bits 29 // to compute in Q29

	s64 k, N, D, One;

	sta_assert((ooeps == 10)||(ooeps == 100));

	if (tc == 0)
		return (alt) ? 0x7FFFFF : 0;	//1.0 or 0 in Q23

    // set One to 1.0 in Q29
    One = 1;
    One <<= tco_bits;

    // Numerator = -10000.0 or -20000.0 in Q29
    if (ooeps == 10)
        N = -10000*One;
    else // ooeps == 100
        N = -20000*One;

    // Denominator = Fs*tc in Q29
    D = STA_SAMPLING_FREQ*(s64)tc;

    // X = -10000/(Fs*TC) in Q29
#ifdef LINUX_OS
    k = div64_s64(N,D);
#else
    k = N/D;
#endif

    // X = (Fs*TC-10000)/(Fs*TC) in Q29
    k += One;

    // X *= 1/(4*log10(2)) in Q29
    k *= tco_m;
    k >>= tco_bits;

    // X = a*X+B, an estimate of 2^(X/4) in Q29
    k *= tco_a;
    k >>= tco_bits;
    k += tco_b;

    // X = 2^(X/4)^2 in Q29
    k *= k;
    k >>= tco_bits;

    // X = 2^(X/4)^4 in Q29
    k *= k;
    k >>= tco_bits;

    // X = 2^(X/4)^4 / 10 in Q23
#ifdef LINUX_OS
	k = div64_s64(k, 640);
#else
    k /= 640;
#endif

    // if alternative, return 1.0-X instead of X
	if (alt)
		k = (1<<23) - k;

	return (f32) k;
}

//For LINEAR GAIN
f32 _STA_TCtoLinearFactor(u32 ms)
{
	if (ms == 0)
		return 1;

	return (f32) (ms * STA_SAMPLING_FREQ / 1000);
}

//Calculates k, y0, ymax and ymin from x/y start stop points
void _STA_SetLinLimiter(STALinLimiter *lin, s32 xstart, s32 xstop, s32 ystart, s32 ystop)
{
    if (xstop == xstart) { // avoid division by 0
    // set slope to 0 and output value to middle of range
    // (k*x)/d+y0 will always return y0
        lin->k =  0;
        lin->d =  1;
        lin->y0 = (ystop+ystart)/2;
    }
    else { // compute slope k/d and value at origin y0
        lin->k =  ystop-ystart;
        lin->d =  xstop-xstart;
        lin->y0 = ystart - (lin->k*xstart)/lin->d;
    }

	if(ystart > ystop) {
		lin->ymax = ystart;
		lin->ymin = ystop;
	}
	else {
		lin->ymax = ystop;
		lin->ymin = ystart;
	}
}

//Calculates (k*in)/d + y0, performs also limitation
s32 _STA_GetInterpolatedValue(const STALinLimiter *lin, s32 x)
{
  int ret = (int)( (lin->k * x)/lin->d + lin->y0 );

  //clamp
  if(ret > lin->ymax)      ret = lin->ymax;
  else if(ret < lin->ymin) ret = lin->ymin;

  return ret;
}


#ifdef MIXER_VER_1
//converts mixer full dynamic range TC (ms/10) to frac., 2^(120/(45.6*TC))
void _STA_TC2Frac(u32 tc, s32 *updown)
{
	sta_assert(0);
}
#endif


//USED COMPANDER (and formerly by LIMITER)
//Converts dB (prescaled by scale) to frac.
//note: function taken from ADR3
s32 _STA_DB2Frac(s16 db, s16 scale, bool convtype)
{
	f32 tmp = (db * 0xaa14)/scale;	//0x1102 = FRAC( 1/(10*20*32*log10(2)) )

	//dB must be < 1920 so that db * 0x1102 < 0x7FFFFF
	sta_assert(((-192*scale) <= db) && (db <= (192*scale)));

	if (convtype)
		tmp = FRAC(0.125) - tmp;

	return (s32) tmp;
}


#endif // STA_NO_FLOAT

