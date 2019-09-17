/*
 * CSE_ext_extendedDriverTests_HMAC.c
 *
 *  Created on: 31 january 2018
 *      Author: pierre guillemin
 */

/* The file provides a series of tests for HMAC MAC TAG generation and verification with only one test vector.
 * It uses RAM key. */

#include "config.h"
#include "serialprintf.h"

#include "cse_types.h"
#include "CSE_ext_extendedDriverTestsFuncList.h"
#include "CSE_ext_HMAC_tests.h"
#include "err_codes.h"


uint32_t test_CSE_extendedDriver_HMAC_macGeneration(uint32_t P_verbose, uint32_t P_reducedTestVectorSet)
{

    return(hmac_macGeneration_test(P_verbose, P_reducedTestVectorSet));

} /* End of test_CSE_extendedDriver_HMAC_macGeneration */


uint32_t test_CSE_extendedDriver_HMAC_macVerification(uint32_t P_verbose, uint32_t P_reducedTestVectorSet)
{

    return(hmac_macVerification_test(P_verbose, P_reducedTestVectorSet));

} /* End of test_CSE_extendedDriver_HMAC_macVerification */
