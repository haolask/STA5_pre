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

#include "cse_typedefs.h"

/**
 * @brief   Type of a mailbox registers structure.
 */
struct mbox_type_def {
	/* Interrupt Request Status Register */
	volatile uint32_t                 IRSR;
	/* Interrupt Pending Status Register */
	volatile uint32_t                 IPSR;
	/* Interrupt Mask Register */
	volatile uint32_t                 IMR;
};

/* mailbox A7 -> eHSM */
#define MBOX_A7_HSM ((struct mbox_type_def *)ehsm_context.MBOX_A7)
