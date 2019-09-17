/*
    SPC5-CRYPTO - Copyright (C) 2014 STMicroelectronics

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    test_support.c
 * @brief   CSE tests support functions
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */

#ifndef TEST_VALUES_INCLUDED_H
#define TEST_VALUES_INCLUDED_H
#include "cse_typedefs.h"

/* for status display */
#include "serialprintf.h"
#include "serial_input.h"
#include "cse_client.h"
#include "CSE_HAL.h"


void display_CSE_status(void) {
  uint32_t status;
  uint32_t err;
  uint32_t cmd;
  int32_t i;
  uint32_t mask = 0x80;

  cmd = CSE->CMD.R;
  status = CSE->SR.R;
  err = CSE->ECR.R;
  printf("CMD : %08X, SR : %08X, ECR : %08X\n", cmd, status, err);

  printf(" IDB | EDB | RIN | BOK | BFN | BIN |  SB | BSY\n");
  for(i=7; i>=0; i-- )
  {
      printf("  %d   ", ((status&mask) == mask) );
      mask = mask >> 1;
  }
}

void display_pass_fail(int pass)
{
  if (pass)
    printf("- PASSED\n\n");
  else
    printf("- FAILED\n\n");
}

#endif
/**
 * @}
 */

