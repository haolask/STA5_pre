/***********************************************************************************/
/*!
*  \file      compiler_switches.h
*
*  \brief     <i><b> Compile switches </b></i>
*
*  \details   Compile switches
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Christophe Quarre
*
*  \version   1.0
*
*  \date      2013/04/15
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

//NOTE: This Header is required for compiling audio_lib.
 
#ifndef _COMPILER_SWITCHES_H_
#define _COMPILER_SWITCHES_H_
       

//legacy from audio_lib
#define DEBUG_LEVEL 0

//legacy from HIT2
//#define SIMULATOR 0
//#define COUNT_FREE_CYCLES 1

//Accordo2
//Choose one of the 3 dsp targets
//#######################################################################################
//IMPORTANT: also, in the EDE "Project Option":
// - select the corresponding memory_config in "Project Option -> Memory..."
// - select the executable name (prefix) in "Project Option -> Output Generation..."
//#######################################################################################
#define ACCORDO2_DSP0
//#define ACCORDO2_DSP1
//#define ACCORDO2_DSP2
//#define ACCORDO5_DSP2


#endif

