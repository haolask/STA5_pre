/***********************************************************************************/
/*!
*  \file      DSPLib.h
*
*  \brief     <i><b> dummy dsp lib </b></i>
*
*  \details   dummy dsp lib
*
*  \author    Quarre Christophe
*
*  \author    (original version) Quarre Christophe
*
*  \version
*
*  \date      2015/07/21
*
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

//#include "sta10xx_type.h"

bool DSPLib_Init(void);

//DUMMY_ADD
int DSPLib_dummy_add(int a, int b);

//DUMMY1
void DSPLib_dummy1(int a, int b, int c, int res[3]);
void DSPLib_dummy1_async(int a, int b, int c);
void DSPLib_dummy1_getresult(int res[3]);
