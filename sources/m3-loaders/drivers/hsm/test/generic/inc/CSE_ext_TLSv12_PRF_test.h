/*
 * CSE_ext_TLSv12_PRF_test.h
 *
 *  Created on: 3 avr. 2018
 */

#ifndef SOURCE_CSE_DRIVER_TEST_INC_CSE_EXT_TLSV12_PRF_TEST_H_
#define SOURCE_CSE_DRIVER_TEST_INC_CSE_EXT_TLSV12_PRF_TEST_H_

#include "cse_types.h"
#include"config.h"

/**
  * @brief  TLS V1.2 secret generation Test
  *
  * @param verbose
  *
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t tlsv12_prf_test(uint32_t P_verbose);

#endif /* SOURCE_CSE_DRIVER_TEST_INC_CSE_EXT_TLSV12_PRF_TEST_H_ */
