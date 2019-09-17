/***********************************************************************************/
/*!
*  \file      lin2db.h
*
*  \brief     <i><b> linear to dB </b></i>
*
*  \details   
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Ryo Tsutsui, Christophe Quarre
*
*  \version   1.0
*
*  \date      2017.01.26
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

#if 1
#ifdef EMERALDCC

#include "emerald.h"          /* This must be always included */


//---------------------------------------------------------------------------
//  FAST_LOG2
//---------------------------------------------------------------------------
extern fraction g_lin2dBFactor; //= -20log(2) * (1 << 23 - LOG2SHIFT(=16) + DBSHIFT(=8) )


#if 0
//returns -log2 in Q16 (x in dx0)
int log2_q16(fraction x)
{
    register int      clz  at(acc[0]);
    register int      log2 at(acc[0]);
    register int      rest at(acc[1]);
    
    if (x > 0) 
    {
        clz    = cls(x);            //note: cls returns cls-1!! 
        clz   += 1;
        rest   = x << clz;
     // bit_reset(rest, 23);      //can't compile this line, crashing EDE... 
        /$ bit_reset	acc1, acc1, #23 $/ 
        rest >>= 7;
        log2   = (clz << 16) - rest;
        return log2;
    }
    else 
        return 0x7fffff; //MAX
}

int log2_ryo(fraction x)
{
	register fraction xplus1 at(acc[1]);
    register int      clz  at(acc[0]);
    register int      log2 at(acc[0]);
    register int      rest at(acc[1]);
    
    //workaround: add epsilon to avoid computing cls(0), which returns 0 instead of -inf.
    //This obligates to first divide x by 2 to avoid that 0x7fffff becomes 0x800000 (= -1 !)
    xplus1 = (x >> 1) + 1;

    clz    = cls(xplus1);     
    rest   = xplus1 << clz;
    /$ bit_reset	acc1, acc1, #22 $/     //equivalent to minus "one" in Q22
    rest >>= 6;
    log2   = (clz << 16) - rest;
        
    return log2;
}


//returns 20log(x23) in Q8 (x in dx0)
int lin2dB_q8(fraction x)
{
    register int      clz  at(acc[0]);
    register int      log2 at(acc[0]);
    register int      rest at(acc[1]);

    //compute dB =   20log(2) * log2(x) in Q8
    //    dB(Q8) = {-20log(2) * (1<< 23 - 16 + 8)} * (-log2_q16(x))
        
    if (x > 0) 
    {
        //first, compute -log2(x) in Q16
        clz    = cls(x);            //note: cls returns cls-1!! 
        clz   += 1;
        rest   = x << clz;
     // bit_reset(rest, 23);      //can't compile this line, crashing EDE... 
        /$ bit_reset	acc1, acc1, #23 $/ 
        rest >>= 7;
        log2   = (clz << 16) - rest;
        
        //then map to dB(Q8)
        log2 *= g_lin2dBFactor;
        return log2;
    }
    else 
        return 0x800000; // MIN
}

#endif



//computes -log2(x23) in Q16
//x in dx0, res in acc0
#macro LOG2_Q16(x, bitresetbit)
{
    register int      clz  at(acc[0]);
    register int      log2 at(acc[0]);
    register int      rest at(acc[1]);
    
    if (x > 0) 
    {
        clz    = cls(x);            //note: cls returns cls-1!! 
        clz   += 1;
        rest   = x << clz;
     // bit_reset(rest, 23);                //can't compile this line, crashing EDE... 
     // /$ bit_reset	acc1, acc1, #23 $/  //can't put in macro because of #23
        /$ bit_reset	acc1, acc1, bitresetbit $/ 
        rest >>= 7;
        log2   = (clz << 16) - rest;
    }
    else
    {
        log2   = 0x7fffff;
    }
}
#endm



//returns 20log(x23) in Q8
//x in dx0, res in acc0
#macro LIN2DB_Q8(x, bitresetbit)
{
    register int      clz  at(acc[0]);
    register int      log  at(acc[0]);
    register int      rest at(acc[1]);

    //compute dB =   20log(2) * log2(x) in Q8
    //    dB(Q8) = {-20log(2) * (1 << 23 - 16 + 8)} * (-log2_q16(x))
        
    if (x > 0) 
    {
        //first, compute -log2(x) in Q16
        clz    = cls(x);            //note: cls returns cls-1!! 
        clz   += 1;
        rest   = x << clz;
     // bit_reset(rest, 23);      //can't compile this line, crashing EDE... 
        /$ bit_reset	acc1, acc1, bitresetbit $/ 
        rest >>= 7;
        log    = (clz << 16) - rest;
        
        //then map to dB(Q8)
        log *= g_lin2dBFactor;
    }
    else {
        log = 0x800000; // MIN
    }
}
#endm

#endif //EMERALDCC
#endif //0

