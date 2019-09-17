/***********************************************************************************/
/*!
*  \file      audio_peak_detector.h
*
*  \brief     <i><b> Peak Detector Blocks </b></i>
*
*  \details   Peak Detector Blocks
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Olivier DOUVENOT
*
*  \version   1.0
*
*  \date      2016.06.30
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

#ifndef _PEAK_DETECTOR_H_
#define _PEAK_DETECTOR_H_


typedef struct{
   fraction _XMEM *ins; // pointer to one sample per input channel from channel 1 to channel N
} T_PeakDetectData;


typedef struct {
    fraction        peak; // in read for peak detection, in write to reset to 0
} T_PeakDetect1chParams;


typedef struct {
    int                           numCh; // N channels
    T_PeakDetect1chParams _YMEM*  pch;   // pointer to the N peak from channel 1 to channel N
} T_PeakDetectNchParams;


void audio_peak_detector_Nch(T_PeakDetectData _XMEM *data, T_PeakDetectNchParams _YMEM *params);

#endif
