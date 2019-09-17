/***********************************************************************************/
/*!
*  \file      audio_cd_deemphasis.h
*
*  \brief     <i><b> CD De-emphasis block </b></i>
*
*  \details   CD De-emphasis block
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Jan Ingerle
*
*  \version   1.0
*
*  \date      2007.07.17
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

#ifndef _CDDEEPHASIS_H_
#define _CDDEEPHASIS_H_

// ----------------------- Defs -----------------------------------

#define DEEM_MASK_BIT 0x00000020
#define DEEM_MODE_OFF 0x000000
#define DEEM_MODE_ON 0x7FFFFF
#define DEEM_MODE_AUTO 0x800000
#define DEEM_OFF 0x000000
#define DEEM_ON 0x000001

// -------------------- Type Defs ---------------------------------

typedef struct{
   int deemIsOn;
   fraction deemFilterCoeffs[3];
} T_CDDeemphasisParam;

typedef struct{
   fraction deemFilterDelay[4];        
   fraction _XMEM *p_inL;
   fraction _XMEM *p_outL;
   fraction _XMEM *p_inR;
   fraction _XMEM *p_outR; 
} T_CDDeemphasisData;


// -------------------- Memory Defs ---------------------------------


// -------------------- Function Prototypes -----------------------

void audio_cd_deemphasis_init(T_CDDeemphasisData _XMEM *data, T_CDDeemphasisParam _YMEM *params);
void audio_cd_deemphasis(T_CDDeemphasisData _XMEM *data, T_CDDeemphasisParam _YMEM *params);

#endif
