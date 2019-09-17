/*
 *  Copyright (C) 2018 STMicroelectronics
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

/**
 * @file	rpmx_common.c
 * @brief	RPMB and RPMC common interface
 *
 * @addtogroup RPMX_COMMON
 * @{
 */
#include "rpmx_common.h"

/**
 * @brief       rpmx_discover
 *              This routine discovers the monotonic counter remote device
 *              family.
 * @param	ctx: NVM context
 * @param       descriptor: pointer to the descriptor of the family.
 * @return      RPMX_LS_DEVICE_FAILURE if the drive is not functional
 *              RPMX_LS_DEVICE_ERR in case it cannot be erased
 *              RPMx_OK if ok
 */
enum RPMx_ErrTy rpmx_discover(t_nvm_ctx *ctx, enum rpmx_family *descriptor)
{
	enum RPMx_ErrTy ret = RPMx_OK;

#if KS_MEMTYPE == NVM_SQI
	ret = sqi_rpmc_discover(ctx, descriptor);
	if (!ret)
		ret = RPMx_INIT_ERROR;
#elif KS_MEMTYPE == NVM_MMC
	*descriptor = RPMX_DEV_FAMILY_3;
#endif

	return ret;
}

/**
 * @brief       rpmx_init
 *              This routine writes the root key to the remote RPMB storage
 * @param	ctx: NVM context
 * @param	descriptor: pointer to the descriptor of the family.
 * @param	payload: Write Root key payload
 * @return      RPMx_OK if ok else error code
 */
enum RPMx_ErrTy rpmx_init(t_nvm_ctx *ctx,
			  enum rpmx_family *descriptor, uint8_t *payload)
{
	enum RPMx_ErrTy ret;

	/* Get the device descriptor to double check the remote storage is known */
	ret = rpmx_discover(ctx, descriptor);
	if (!ret) {
		ret = RPMx_INIT_ERROR;
#if KS_MEMTYPE == NVM_SQI
		switch (sqi_rpmc_init(ctx, (struct sqi_rpmc_wrkr_t *)payload)) {
		case SQI_RPMC_OK:
			ret = RPMx_OK;
			break;
		case SQI_RPMC_ROOT_KEY_AVAILABLE:
			ret = RPMx_ROOT_KEY_AVAILABLE;
			break;
		default:
			break;
		}
#elif KS_MEMTYPE == NVM_MMC
		/* Select RPMB */
		if (!sdmmc_set_part_config(ctx, MMC_PART_RPMB, -1, -1)) {
			/* Write provided RPMB frame containing the key */
			if (!rpmb_request(ctx, (const struct t_rpmb *)payload, true))
				ret = RPMx_OK;

			/* Return to USER partition */
			sdmmc_set_part_config(ctx, MMC_PART_USER, -1, -1);
		}
#endif
	}

	return ret;
}

/** @} */
