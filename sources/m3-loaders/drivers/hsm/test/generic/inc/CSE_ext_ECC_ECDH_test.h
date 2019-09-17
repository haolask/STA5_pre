/*
 * CSE_ext_ECC_ECDH_test.h
 *
 *  Created on: 6 avr. 2018
  */

#ifndef _CSE_EXT_ECC_ECDH_TEST_H_
#define _CSE_EXT_ECC_ECDH_TEST_H_

/**
  * @brief  ECC ECDH-fixed Key Agreement fixed Test
  * @detail The function performs the ECDH Key Agreement operation
  *
  * @param verbose
  *
  * @retval error status: ECC_SUCCESS, ECC_ERR_BAD_PARAMETER, CSE_GENERAL_ERR
*/

uint32_t ecc_ecdh_keyAgreement_fixed_test(uint32_t P_verbose, uint32_t reduced_test_vector_nb);
uint32_t ecc_ecdh_keyAgreementClientServer_test(uint32_t P_verbose, uint32_t reduced_test_vector_set);
uint32_t ecc_ecdh_serverKeyAgreement_test(uint32_t P_verbose, uint32_t reduced_test_vector_nb);
uint32_t ecc_ecdh_keyAgreementClientServer_test_with_PK(uint32_t P_verbose, uint32_t reduced_test_vector_nb);

#endif /* _CSE_EXT_ECC_ECDH_TEST_H_ */
