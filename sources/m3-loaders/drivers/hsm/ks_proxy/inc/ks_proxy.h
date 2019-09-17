/*
 *  Copyright (C) 2018 STMicroelectronics
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @file	ks_proxy.h
 * @brief	Proxy to handle key storage.
 * @details	Set of functions used to manage the key storage.
 */
#ifndef _KS_PROXY_H_
#define _KS_PROXY_H_

#include "rpmx_common.h"

extern t_nvm_ctx *g_ks_nvm_ctx;

/**
 * @brief	Perform NVM autotest
 * @details	Try writing payload and then erase block used
 *		for Key storage
 */
void NVM_autotest(void);

/**
 * @brief	Dump key storage area
 * @details	Read all sectors used for Key Storage and display them
 */
void NVM_dump_LS_area(void);

/**
 * @brief       M3 Service dispatcher initialisation
 * @details     Get key storage NVM offset
 *
 * @param	None
 * @return	0 if OK else error code
 */
enum RPMx_ErrTy ks_service_init(void);

/**
 * @brief       M3 Service dispatcher for external key storage and monotonic
 *              counters
 *
 * @param	None
 * @return	None
 */
void ks_service_dispatcher(void);


#endif /* _KS_PROXY_H_ */
