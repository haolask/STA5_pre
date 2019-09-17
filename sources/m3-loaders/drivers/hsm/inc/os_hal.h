/*
 *  Copyright (C) 2014 STMicroelectronics
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

/*
 *  hal.h
 *  header to leverage freertos services instead of SPC5 components / chibios
 */

/**
 * @file    os_hal.h
 * @brief   HAL / OSAL module header.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef _HAL_H_
#define _HAL_H_

/* OS timing dedicated changes (wrapper to freeRtos) */
#include "FreeRTOS.h"
#include "task.h"

/* console print dedicated changes (wrapper to freeRtos)  */
#include <trace.h>

#include "sta_type.h"
#include "sta_mem_map.h"

/* serial printf replacement */
#define printf				trace_printf

/* OS timer delay */
#define osalThreadSleepMilliseconds	vTaskDelay

#endif /* _HAL_H_ */

/** @} */
