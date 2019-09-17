/***********************************************************************************/
/*!
*  \file      audio_delay_line
*
*  \brief     <i><b> Output Delay Line </b></i>
*
*  \details   Output Delay Line
*
*  \author    APG-MID Application Team
*
*  \author    (original version) Jan Ingerle
*
*  \version   1.0
*
*  \date      2009.08.19
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

#ifndef _DELAY_H_
#define _DELAY_H_

// -------------------- Defs ---------------------------------

#define SDELAY_LENGTH 128

// -------------------- Type Defs ---------------------------------


typedef struct{  
  fraction _XMEM *p_write;
  int readDiff;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength;                         
  fraction _XMEM *p_begin;  
  fraction _XMEM *p_end;   
} T_DelayXParam;

typedef struct{  
  fraction _XMEM *p_write_0;
  int readDiff_0;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_0;                         
  fraction _XMEM *p_begin_0;  
  fraction _XMEM *p_end_0;   
  fraction _XMEM *p_write_1;
  int readDiff_1;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_1;                         
  fraction _XMEM *p_begin_1;  
  fraction _XMEM *p_end_1;   
  fraction _XMEM *p_write_2;
  int readDiff_2;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_2;                         
  fraction _XMEM *p_begin_2;  
  fraction _XMEM *p_end_2;   
  fraction _XMEM *p_write_3;
  int readDiff_3;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_3;                         
  fraction _XMEM *p_begin_3;  
  fraction _XMEM *p_end_3;   
} T_DelayX4chParam;

typedef struct{  
  fraction _XMEM *p_write_0;
  int readDiff_0;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_0;                         
  fraction _XMEM *p_begin_0;  
  fraction _XMEM *p_end_0;   
  fraction _XMEM *p_write_1;
  int readDiff_1;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_1;                         
  fraction _XMEM *p_begin_1;  
  fraction _XMEM *p_end_1;   
  fraction _XMEM *p_write_2;
  int readDiff_2;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_2;                         
  fraction _XMEM *p_begin_2;  
  fraction _XMEM *p_end_2;   
  fraction _XMEM *p_write_3;
  int readDiff_3;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_3;                         
  fraction _XMEM *p_begin_3;  
  fraction _XMEM *p_end_3;   
  fraction _XMEM *p_write_4;
  int readDiff_4;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_4;                         
  fraction _XMEM *p_begin_4;  
  fraction _XMEM *p_end_4;   
  fraction _XMEM *p_write_5;
  int readDiff_5;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_5;                         
  fraction _XMEM *p_begin_5;  
  fraction _XMEM *p_end_5;   
} T_DelayX6chParam;


typedef struct{  
  fraction _YMEM *p_write;
  int readDiff;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength;                        
  fraction _YMEM *p_begin;  
  fraction _YMEM *p_end;   
} T_DelayYParam;

typedef struct{  
  fraction _YMEM *p_write_0;
  int readDiff_0;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_0;                        
  fraction _YMEM *p_begin_0;  
  fraction _YMEM *p_end_0;   
  fraction _YMEM *p_write_1;
  int readDiff_1;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_1;                        
  fraction _YMEM *p_begin_1;  
  fraction _YMEM *p_end_1;   
  fraction _YMEM *p_write_2;
  int readDiff_2;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_2;                        
  fraction _YMEM *p_begin_2;  
  fraction _YMEM *p_end_2;   
  fraction _YMEM *p_write_3;
  int readDiff_3;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_3;                        
  fraction _YMEM *p_begin_3;  
  fraction _YMEM *p_end_3;   
} T_DelayY4chParam;


typedef struct{  
  fraction _YMEM *p_write_0;
  int readDiff_0;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_0;                        
  fraction _YMEM *p_begin_0;  
  fraction _YMEM *p_end_0;   
  fraction _YMEM *p_write_1;
  int readDiff_1;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_1;                        
  fraction _YMEM *p_begin_1;  
  fraction _YMEM *p_end_1;   
  fraction _YMEM *p_write_2;
  int readDiff_2;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_2;                        
  fraction _YMEM *p_begin_2;  
  fraction _YMEM *p_end_2;   
  fraction _YMEM *p_write_3;
  int readDiff_3;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_3;                        
  fraction _YMEM *p_begin_3;  
  fraction _YMEM *p_end_3;   
  fraction _YMEM *p_write_4;
  int readDiff_4;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_4;                        
  fraction _YMEM *p_begin_4;  
  fraction _YMEM *p_end_4;   
  fraction _YMEM *p_write_5;
  int readDiff_5;                            // readDiff = delayLineLength - delay - 1
  int delayLineLength_5;                        
  fraction _YMEM *p_begin_5;  
  fraction _YMEM *p_end_5;   
} T_DelayY6chParam;

typedef struct{
   fraction _XMEM *p_in;
   fraction _XMEM *p_out; 
} T_DelayData;

typedef struct{
   fraction _XMEM *p_in_0;
   fraction _XMEM *p_out_0; 
   fraction _XMEM *p_in_1;
   fraction _XMEM *p_out_1; 
   fraction _XMEM *p_in_2;
   fraction _XMEM *p_out_2; 
   fraction _XMEM *p_in_3;
   fraction _XMEM *p_out_3; 
} T_Delay4chData;

typedef struct{
   fraction _XMEM *p_in_0;
   fraction _XMEM *p_out_0; 
   fraction _XMEM *p_in_1;
   fraction _XMEM *p_out_1; 
   fraction _XMEM *p_in_2;
   fraction _XMEM *p_out_2; 
   fraction _XMEM *p_in_3;
   fraction _XMEM *p_out_3; 
   fraction _XMEM *p_in_4;
   fraction _XMEM *p_out_4; 
   fraction _XMEM *p_in_5;
   fraction _XMEM *p_out_5; 
} T_Delay6chData;


// -------------------- Function Prototypes -----------------------

void audio_delay_line_x_1ch_init(T_DelayXParam _YMEM *params);
void audio_delay_line_y_1ch_init(T_DelayYParam _YMEM *params);
void audio_delay_line_sht_x_1ch_init(T_DelayXParam _YMEM *params);
void audio_delay_line_sht_y_1ch_init(T_DelayYParam _YMEM *params);

void audio_delay_line_x_4ch_init(T_DelayX4chParam _YMEM *params);
void audio_delay_line_y_4ch_init(T_DelayY4chParam _YMEM *params);
void audio_delay_line_sht_x_4ch_init(T_DelayX4chParam _YMEM *params);
void audio_delay_line_sht_y_4ch_init(T_DelayY4chParam _YMEM *params);

void audio_delay_line_x_6ch_init(T_DelayX6chParam _YMEM *params);
void audio_delay_line_y_6ch_init(T_DelayY6chParam _YMEM *params);
void audio_delay_line_sht_x_6ch_init(T_DelayX6chParam _YMEM *params);
void audio_delay_line_sht_y_6ch_init(T_DelayY6chParam _YMEM *params);

void audio_delay_line_x_1ch(T_DelayData _XMEM *data, T_DelayXParam _YMEM *params);
void audio_delay_line_y_1ch(T_DelayData _XMEM *data, T_DelayYParam _YMEM *params);
void audio_delay_line_sht_x_1ch(T_DelayData _XMEM *data, T_DelayXParam _YMEM *params);
void audio_delay_line_sht_y_1ch(T_DelayData _XMEM *data, T_DelayYParam _YMEM *params);

void audio_delay_line_x_4ch(T_Delay4chData _XMEM *data, T_DelayX4chParam _YMEM *params);
void audio_delay_line_y_4ch(T_Delay4chData _XMEM *data, T_DelayY4chParam _YMEM *params);
void audio_delay_line_sht_x_4ch(T_Delay4chData _XMEM *data, T_DelayX4chParam _YMEM *params);
void audio_delay_line_sht_y_4ch(T_Delay4chData _XMEM *data, T_DelayY4chParam _YMEM *params);

void audio_delay_line_x_6ch(T_Delay6chData _XMEM *data, T_DelayX6chParam _YMEM *params);
void audio_delay_line_y_6ch(T_Delay6chData _XMEM *data, T_DelayY6chParam _YMEM *params);
void audio_delay_line_sht_x_6ch(T_Delay6chData _XMEM *data, T_DelayX6chParam _YMEM *params);
void audio_delay_line_sht_y_6ch(T_Delay6chData _XMEM *data, T_DelayY6chParam _YMEM *params);



#endif
