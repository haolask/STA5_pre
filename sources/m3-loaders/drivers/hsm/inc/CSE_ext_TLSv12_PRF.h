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
struct TLS_PRF_Secret {
	uint8_t  *address;    /*!< pointer to buffer */
	uint32_t  byteSize;   /*!< size in bytes */
};

/**
 * @brief Structure type for TLS V1.2 Label
 */
struct TLS_PRF_Label {
	uint8_t  *address;    /*!< pointer to buffer */
	uint32_t  byteSize;   /*!< size in bytes */
};

/**
 * @brief Structure type for TLS V1.2 Seed
 */
struct TLS_PRF_Seed {
	uint8_t  *address;    /*!< pointer to buffer */
	uint32_t  byteSize;   /*!< size in bytes */
};

/**
 * @brief Structure type for TLS V1.2 PRF output
 */
struct TLS_PRF_PrfOutput {
	uint8_t  *address;    /*!< pointer to buffer */
	uint32_t  byteSize;   /*!< size in bytes */
};

/*============================================================================*/
/**
 * @brief       Implement TLS V1.2 Pseudo Random Function operation
 * @details
 * *
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
			struct TLS_PRF_Seed *P_pSeed,
			vuint32_t P_hashAlgo,
			struct TLS_PRF_PrfOutput *P_pPRFOutput);
#endif /* SOURCE_CSE_DRIVER_INCLUDE_CSE_EXT_TLSV12_PRF_H_ */
