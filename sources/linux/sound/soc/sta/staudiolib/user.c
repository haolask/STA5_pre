/*
 * Copyright (C) ST Microelectronics 2017
 *
 * Author:	Gian Antonio Sampietro <gianantonio.sampietro@st.com>,
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include "user.h"

typedef struct {
} USER_params_t;

typedef struct {
} USER_data_t;

typedef struct {
} USER_coeffs_t;

void USER_InitModule(STA_UserModuleInfo* info)
{
}

void USER_UpdateDspCoefs(STA_UserModuleInfo* info)
{
}

void USER_InitDspData(STA_UserModuleInfo* info)
{
}

STA_UserModuleInfo user_info = {
	.m_dspType 			= 0,
	.m_nin  			= 0,
	.m_nout 			= 0,
	.m_sizeofParams 	= sizeof(USER_params_t),
	.m_wsizeofData  	= WSIZEOF(USER_data_t) + 2,
	.m_wsizeofCoefs 	= WSIZEOF(USER_coeffs_t),
	.InitModule 		= USER_InitModule,
	.InitDspData 		= USER_InitDspData,
	.UpdateDspCoefs 	= USER_UpdateDspCoefs,
	.GetDspInAddr 		= NULL,
	.GetDspOutAddr 		= NULL,
	.SetMode 		= NULL,
	.AdjustDspMemSize 	= NULL,
};

