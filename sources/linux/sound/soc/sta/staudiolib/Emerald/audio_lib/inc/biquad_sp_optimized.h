/***********************************************************************************/
/*!
*  \file      biquad_sp_optimized.h
*
*  \brief     <i><b> biquad variable definitions in Simple Precision </b></i>
*
*  \details   biquad variable definitions in Simple Precision
*
*  \author    APG-MID Application Team
*
*  \history   2015.12.12: original version (Christophe Quarre)
*
*  \version   1.0
*
*  \bug       Unknown
*
*  \warning   
* 
*  This file is part of DSP ST Audio and is dual licensed,
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
* DSP ST Audio is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* DSP ST Audio is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*/
/***********************************************************************************/

#ifndef BIQUAD_OPTIM_H 
#define BIQUAD_OPTIM_H

/*---------------------------------------------------------------------------*/
/* Defines                                                                   */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type definitions                                                          */
/*---------------------------------------------------------------------------*/

//biquad coefs (1 stage) (all coefs are halved)
typedef struct{  
  fraction b0;
  fraction b1;
  fraction b2;
  fraction a2;
  int bshift;
  fraction a1;
} T_Biquad_Optim_SP_Coefs;

//biquad data (1 stage)
typedef struct {    
  fraction xn;          //input
  fraction xn_1;
  fraction xn_2;
  fraction yn_2;
  fraction yn_1;         //output: y(n-1) = y(n)
} T_Biquad_Optim_SP_Data;

//biquad data (next chained stage)
typedef struct {    
//fraction xn;          //input from previous stage yn_1
  fraction xn_1;
  fraction xn_2;
  fraction yn_2;
  fraction yn_1;         //output: y(n-1) = y(n)
} T_Biquad_Optim_SP_Chained_Data;


//---------------------------------------------------------------------------
//  used for testing
//---------------------------------------------------------------------------

#define BIQUAD_OPTIM_MAX_STAGE      16

//chained coefs
typedef struct {    
  T_Biquad_Optim_SP_Coefs       coefs[BIQUAD_OPTIM_MAX_STAGE];
} T_Biquad_Optim_Nbands_SP_Coefs;

//chained data
typedef struct {
  fraction                        xn;
  T_Biquad_Optim_SP_Chained_Data  data[BIQUAD_OPTIM_MAX_STAGE];
} T_Biquad_Optim_Nbands_SP_Data;



/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/*
function: void biquad_optimized_sp(void) 
detail:   IIR Direct Form I:
 
    y(n) = 2 ((b0*x(n) + b1*x(n-1) + b2*x(n-2)) << bshift - a1*y(n-1) - a2*y(n-2)) 
 
    WARNING:
    - The data MUST be in the specific order:         d = [x0(n), x0(n-1), x0(n-2), y0(n-2), y0(n-1), ?.]
    - he coefficients MUST be in the specific order:  c = [b00, b01, b02, a02, a01, ...]
    - the coefficients are halved

    NOTE:
    - This biquad can be chained: e.g.
    loop(3) {
        biquad_sp_optimized_macro();
    }
        
*/   

//implemented in biquad_sp_optimized.c
//void biquad_sp_optimized_chainable(T_Biquad_Optim_Nbands_SP_Data *data, T_Biquad_Optim_Nbands_SP_Coefs _YMEM *coefs) 
void biquad_sp_optimized_chainable(void);

#endif /* BIQUAD_OPTIM_H  */

