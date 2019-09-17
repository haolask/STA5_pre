/*
 * CSE_ext_TLSv12_PRF.c
 *
 *  Created on: 3 avr. 2018
 */

/**
 * @file    CSE_ext_TLSv12_PRF.c
 * @brief   CSE TLS V1.2 Pseudo Random Function PRF module.
 *
 * @addtogroup SHE-ext_driver
 * @{
 */
#include "cse_typedefs.h"
#include "CSE_Constants.h"

#include "CSE_HAL.h"
#include "CSE_ext_TLSv12_PRF.h"

/*============================================================================*/
/**
 * @brief       Implement TLS V1.2 Pseudo Random Function operation
 * @details
 *
 * @param[in]     P_pSecret     Pointer to \ref TLS_PRF_Secret: Secret
 *                              descriptor
 * @param[in]     P_pLabel      Pointer to \ref TLS_PRF_Label: Label descriptor
 * @param[in]     P_pSeed       Pointer to \ref TLS_PRF_Seed: Seed descriptor
 * @param[in]     P_hashAlgo    Type of HASH algorithm
 * @param[out]    P_pPRFOutput  Pointer to \ref TLS_PRF_PrfOutput: PRF output
 *                              descriptor
 *
 * @return        Error code
 * @retval 0      When ECDH key agreement was successfully performed
 * @retval 1..21  In case of error - the error code values are the CSE returned
 *                ones
 *
 * @api
 *
 * @implements
 *
 * @note          The function is blocking and waits for operation completion
 */
uint32_t CSE_TLSv12_prf(struct TLS_PRF_Secret *P_pSecret,
			struct TLS_PRF_Label *P_pLabel,
			struct TLS_PRF_Seed *P_pSeed, vuint32_t P_hashAlgo,
			struct TLS_PRF_PrfOutput *P_pPRFOutput)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd5p(CSE_TLSV12_PRF, (uint32_t)P_pSecret,
				 (uint32_t)P_pLabel, (uint32_t)P_pSeed,
				 (uint32_t)P_hashAlgo, (uint32_t)P_pPRFOutput);

	return ret;
} /* End of CSE_TLSv12_prf */

/**
 * @}
 */

