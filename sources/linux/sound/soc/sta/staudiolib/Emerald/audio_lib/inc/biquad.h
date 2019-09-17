/***********************************************************************************/
/*!
*  \file      biquad.h
*
*  \brief     <i><b> biquad variable definitions </b></i>
*
*  \details   biquad variable definitions
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Ondrej Trubac
*
*  \version   1.0
*
*  \date      2010.10.26
*
*  \bug       Unknown
*
*  \warning   
* 
*  This file is part of <STAudioLib> and is dual licensed,
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
* <STAudioLib> is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
* <STAudioLib> is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* Please refer to <http://www.gnu.org/licenses/>.
*/
/***********************************************************************************/

#ifndef BIQUAD_H 
#define BIQUAD_H

/*---------------------------------------------------------------------------*/
/* Defines                                                                   */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type definitions                                                          */
/*---------------------------------------------------------------------------*/
typedef struct{  
  fraction b0;
  fraction b1;
  fraction b2;
  int bshift;
  fraction a1_half;
  fraction a2;
  int oshift;   
} T_Biquad_Coefs;

typedef struct {    
  fraction xn_high;
  fraction xn_low;  
} T_Biquad_DP_Input;

typedef struct {    
  fraction xn_1_high;
  fraction xn_1_low;
  fraction xn_2_high;
  fraction xn_2_low;
  fraction yn_1_high;
  fraction yn_1_low;
  fraction yn_2_high;         
  fraction yn_2_low;
  fraction yn_high;
  fraction yn_low;
} T_Biquad_DP_Data;

typedef struct {    
  fraction xn_1;
  fraction xn_2;
  fraction yn_1;
  fraction yn_2;         
  fraction yn;
} T_Biquad_SP_Data;

typedef struct {
  T_Biquad_DP_Input in;
  T_Biquad_DP_Data data;
} T_Biquad_DP_Stage_1;

typedef struct {
  T_Biquad_DP_Input in;
  T_Biquad_DP_Data data[2];
} T_Biquad_DP_Stage_2;

typedef struct {
  T_Biquad_DP_Input in;
  T_Biquad_DP_Data data[3];
} T_Biquad_DP_Stage_3;

typedef struct {
  T_Biquad_DP_Input in;
  T_Biquad_DP_Data data[4];
} T_Biquad_DP_Stage_4;

typedef struct {
  T_Biquad_DP_Input in;
  T_Biquad_DP_Data data[5];
} T_Biquad_DP_Stage_5;

typedef struct {
  T_Biquad_DP_Input in;
  T_Biquad_DP_Data data[6];
} T_Biquad_DP_Stage_6;

typedef struct {
  T_Biquad_DP_Input in;
  T_Biquad_DP_Data data[7];
} T_Biquad_DP_Stage_7;

// Single precision data structures
typedef struct {
  fraction xn;
  T_Biquad_SP_Data data;
} T_Biquad_SP_Stage_1;

typedef struct {
  fraction xn;
  T_Biquad_SP_Data data[2];
} T_Biquad_SP_Stage_2;

typedef struct {
  fraction xn;
  T_Biquad_SP_Data data[3];
} T_Biquad_SP_Stage_3;

typedef struct {
  fraction xn;
  T_Biquad_SP_Data data[4];
} T_Biquad_SP_Stage_4;

typedef struct {
  fraction xn;
  T_Biquad_SP_Data data[5];
} T_Biquad_SP_Stage_5;

typedef struct {
  fraction xn;
  T_Biquad_SP_Data data[6];
} T_Biquad_SP_Stage_6;

typedef struct {
  int update_id;
  void *target_coefs;
  T_Biquad_Coefs new_coefs;
  int list_len;
  void *list_ptr;
} T_Biquad_Coefs_Update;

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
//void biquad_dp(T_Biquad_DP_Stage_1 _XMEM *biqdata, T_Biquad_Coefs _YMEM *biqcoef);
void biquad_dp_chained(void);
//void biquad_sp(T_Biquad_SP_Stage_1 _XMEM *biqdata, T_Biquad_Coefs _YMEM *biqcoef);
void biquad_sp_chained(void);
void biquad_update(T_Biquad_Coefs_Update _XMEM *pt );
void biquad_update_init(T_Biquad_Coefs_Update _XMEM *pt) ;
void audio_biquad_init(int bshift,int oshift,T_Biquad_Coefs _YMEM * coefs );

/*
#ifdef EMERALDCC
#macro biquad_dp(biqdata,biqcoefs)
{
    ax0 = biqdata;
    ay0 = biqcoefs;
    biquad_dp_simple();
}
#endm

#macro biquad_sp(biqdata,biqcoefs)
{
    ax0 = biqdata;
    ay0 = biqcoefs;
    biquad_sp_simple();
}
#endm
#endif //EMERALDCC
*/

#endif /* BIQUAD_H  */
