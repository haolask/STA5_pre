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
 * CSE_ext_TLSv12_PRF.h
 *
 *  Created on: 3 avr. 2018
 */

#ifndef SOURCE_CSE_DRIVER_INCLUDE_CSE_EXT_TLSV12_PRF_H_
#define SOURCE_CSE_DRIVER_INCLUDE_CSE_EXT_TLSV12_PRF_H_

/**
 * @brief Structure type for TLS V1.2 Secret
 */
struct tls_prf_secret {
	uint8_t *paddress;	/*!< pointer to buffer */
	uint32_t size;		/*!< size in bytes */
};

/**
 * @brief Structure type for TLS V1.2 Label
 */
struct tls_prf_label {
	uint8_t *paddress;	/*!< pointer to buffer */
	uint32_t size;		/*!< size in bytes */
};

/**
 * @brief Structure type for TLS V1.2 Seed
 */
struct tls_prf_seed {
	uint8_t *paddress;	/*!< pointer to buffer */
	uint32_t size;		/*!< size in bytes */
};

/**
 * @brief Structure type for TLS V1.2 PRF output
 */
struct tls_prf_output {
	uint8_t *paddress;	/*!< pointer to buffer */
	uint32_t size;		/*!< size in bytes */
};

/*============================================================================*/
/**
 * @brief       Implement TLS V1.2 Pseudo Random Function operation
 * @details
 * *
 * @param[in]     *psecret	Pointer to \ref tls_prf_secret: Secret
 *				descriptor
 * @param[in]     *plabel	Pointer to \ref tls_prf_label: Label descriptor
 * @param[in]     *pseed	Pointer to \ref tls_prf_seed: Seed descriptor
 * @param[in]     hash_algo	Type of HASH algorithm
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
			struct tls_prf_output *pprf_output);
#endif /* SOURCE_CSE_DRIVER_INCLUDE_CSE_EXT_TLSV12_PRF_H_ */
