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
 * *
 * @param[in]     *psecret	Pointer to \ref tls_prf_secret: Secret
 *                              descriptor
 * @param[in]     *plabel	Pointer to \ref tls_prf_label: Label descriptor
 * @param[in]     *pseed	Pointer to \ref tls_prf_seed: Seed descriptor
 * @param[in]     hash_algo     Type of HASH algorithm
 * @param[out]    *pprf_output	Pointer to \ref tls_prf_output: PRF output
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
uint32_t cse_tlsv12_prf(struct tls_prf_secret *psecret,
			struct tls_prf_label *plabel,
			struct tls_prf_seed *pseed, vuint32_t hash_algo,
			struct tls_prf_output *pprf_output)
{
	uint32_t ret = CSE_NO_ERR;

	ret = cse_hal_send_cmd5p(CSE_TLSV12_PRF, (uint32_t)psecret,
				 (uint32_t)plabel, (uint32_t)pseed,
				 (uint32_t)hash_algo, (uint32_t)pprf_output);

	return ret;
} /* End of CSE_TLSv12_prf */

/**
 * @}
 */

