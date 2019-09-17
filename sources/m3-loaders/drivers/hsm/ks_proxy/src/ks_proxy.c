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
 * @file	ks_proxy.c
 * @brief	Proxy to handle key storage.
 * @details	Set of functions used to manage the key storage.
 *
 *
 * @addtogroup KS_PROXY
 * @{
 */
#include <string.h>
#include "sta_common.h"
#include "trace.h"
#include "sta_image.h"
#include "cse_client.h"
#include "cmd_mailbox.h"
#include "ks_proxy.h"

t_nvm_ctx *g_ks_nvm_ctx;

static uint32_t ks_sector; /* NVM offset where key storage is placed */

/*
 * Common exchanged data with M3 HSM ROM code counterpart
 */
static struct {
	/* sdmmc RPMB buffer, always below ls_current */
	uint8_t rpmb_payload_buff[512];
	/* Definition of the key image storage in RAM */
	struct lsblock_t ls_current;
} hsm_common_buffers;

#define LS_CURRENT hsm_common_buffers.ls_current

/*
 * Private functions
 */
#if KS_MEMTYPE == NVM_MMC
static enum RPMx_ErrTy rpmb_read_counter_tunelled(t_nvm_ctx *ctx,
						  void *rpmb_frame) {
	enum RPMx_ErrTy ret = RPMx_INIT_ERROR;

	/* Select RPMB */
	if (!sdmmc_set_part_config(ctx, MMC_PART_RPMB, -1, -1)) {
		/* Write provided RPMB frame */
		if (!rpmb_request(ctx, rpmb_frame, true)) {
			/* Read RPMB response frame to rpmb_frame */
			if (!sdmmc_read(ctx, 0, rpmb_frame, 512))
				ret = RPMx_OK;
		}

		/* Return to USER partition */
		sdmmc_set_part_config(ctx, MMC_PART_USER, -1, -1);
	}

	return ret;
}
#endif /* NVM_MMC */

/**
 * @brief	Perform NVM autotest
 * @details	Try writing payload and then erase block used
 *		for Key storage
 */
void NVM_autotest(void)
{
	enum RPMx_ErrTy ret = RPMx_OK;

	/* Clean the payload container */
	memset((uint8_t *)&LS_CURRENT, 0, sizeof(struct lsblock_t));

	/* Set some test values */
	((uint8_t *)&LS_CURRENT.payload)[0] = 1;
	((uint8_t *)&LS_CURRENT.payload)[1] = 2;
	((uint8_t *)&LS_CURRENT.payload)[2] = 3;
	((uint8_t *)&LS_CURRENT.payload)[3] = 4;

	/* Test NVM write capabilities */
	ret = ls_store(g_ks_nvm_ctx, ks_sector, &LS_CURRENT,
		       sizeof(struct lspayload_t));
	if (ret) {
		TRACE_ERR("%s - KS store\n", __func__);
	} else {
		/* Test NVM read capabilities */
		ret = ls_discover(g_ks_nvm_ctx, ks_sector);
		if (ret != RPMx_LS_DEVICE_INIT)
			TRACE_ERR("%s - KS discover\n", __func__);
		else {
			ret = ls_load(g_ks_nvm_ctx, ks_sector, &LS_CURRENT,
				      sizeof(struct lspayload_t));
			if (ret)
				TRACE_ERR("%s - KS load\n", __func__);
			else
				TRACE_NOTICE("%s - NVM KS success\n", __func__);
		}
	}

	/* Erase instance block */
	ret = ls_erase(g_ks_nvm_ctx, ks_sector);
	if (ret)
		TRACE_ERR("%s - KS erase\n", __func__);
}

/**
 * @brief	Dump key storage area
 * @details	Read all sectors used for Key Storage and display them
 */
void NVM_dump_LS_area(void)
{
#if KS_MEMTYPE == NVM_MMC
	enum RPMx_ErrTy ret = RPMx_OK;

	ret = ls_dump_ks_area(g_ks_nvm_ctx);
	if (ret)
		TRACE_ERR("%s - KS dump LS area\n", __func__);
#else
	TRACE_NOTICE("%s - only eMMC Key storage dump supported\n", __func__);
#endif
}

/**
 * @brief       M3 Service dispatcher initialisation
 * @detail      Initialize NVM driver and get key storage NVM offset
 *
 * @param	None
 * @return	0 if OK else error code
 */
enum RPMx_ErrTy ks_service_init(void)
{
	enum RPMx_ErrTy ret = RPMx_OK;

#if KS_MEMTYPE == NVM_SQI
	g_ks_nvm_ctx = sqi_init(0);
#elif KS_MEMTYPE == NVM_MMC
	/* Always SDMMC1 on TC3P boards */
	g_ks_nvm_ctx = sdmmc_init(SDMMC1_PORT, false);
#endif
	if (g_ks_nvm_ctx) {
		if (get_partition_info(KEY_STORAGE_ID, &ks_sector, NULL)) {
			TRACE_ERR("%s: KS part not found\n", __func__);
			ret = RPMx_LS_DEVICE_ERR;
		}
	} else {
		TRACE_ERR("%s: NVM init failed\n", __func__);
		ret = RPMx_LS_DEVICE_ERR;
	}

	return ret;
}

/**
 * @brief       M3 Service dispatcher for external key storage and monotonic
 *              counters
 *
 * @param	None
 * @return	None
 */
void ks_service_dispatcher(void)
{
	/* Device descriptor is always returned */
	enum rpmx_family desc = 0;
	/* Size of reply payload */
	enum RPMx_ErrTy rpmx_error = RPMx_OK;
	enum lserror_t ls_error = LS_NOERROR;
	int resp_size = 0;

	/* Send the answer eHSM is waiting for */
	switch (mb_cmd) {
		/*********************/
		/* LOAD STORE IOCTLS */
		/*********************/
	case MBX_CMD_LS_INIT:
		rpmx_error = ls_discover(g_ks_nvm_ctx, ks_sector);

		/* Clear message buffer */
		memset(&mb_cmd_data, 0, sizeof(mb_cmd_data));

		/* Set the buffer address we're going to use */
		mb_cmd_data.address = &LS_CURRENT;
		resp_size = sizeof(mb_cmd_data.address);

		switch (rpmx_error) {
		case RPMx_LS_DEVICE_INIT:
			break;
		case RPMx_LS_DEVICE_ERR:
			ls_error = LS_FLASH_FAILURE;
			break;
		case RPMx_LS_DEVICE_WARNING:
			ls_error = LS_WARNING;
			break;
		default:
			ls_error = LS_INTERNAL;
			break;
		}
		break;

	case MBX_CMD_LS_DEINIT:
		ls_deinit(g_ks_nvm_ctx);
		break;

	case MBX_CMD_LS_LOAD:
		rpmx_error = ls_load(g_ks_nvm_ctx, ks_sector,
				     &LS_CURRENT.payload, mb_cmd_data.size);
		/* Clear message buffer */
		memset(&mb_cmd_data, 0, sizeof(mb_cmd_data));
		switch (rpmx_error) {
		case RPMx_OK:
			break;
		case RPMx_LS_DEVICE_ERR:
			ls_error = LS_NOTFOUND;
			break;
		default:
			ls_error = LS_INTERNAL;
			break;
		}
		break;

	case MBX_CMD_LS_STORE:
		rpmx_error = ls_store(g_ks_nvm_ctx, ks_sector,
				      LS_CURRENT.payload, mb_cmd_data.size);
		/* Clear message buffer */
		memset(&mb_cmd_data, 0, sizeof(mb_cmd_data));
		switch (rpmx_error) {
		case RPMx_OK:
			break;
		case RPMx_LS_DEVICE_ERR:
			ls_error = LS_NOTINIT;
			break;
		case RPMx_LS_DEVICE_FAILURE:
			ls_error = LS_FLASH_FAILURE;
			break;
		default:
			ls_error = LS_INTERNAL;
			break;
		}
		break;

	case MBX_CMD_LS_ERASE:
		rpmx_error = ls_erase(g_ks_nvm_ctx, ks_sector);
		/* Clear message buffer */
		memset(&mb_cmd_data, 0, sizeof(mb_cmd_data));
		switch (rpmx_error) {
		case RPMx_OK:
			break;
		case RPMx_LS_DEVICE_ERR:
			ls_error = LS_FLASH_FAILURE;
			break;
		default:
			ls_error = LS_INTERNAL;
			break;
		}
		break;

		/*
		 * KS specific commands for TC3P Cut2 BH and higher
		 * Remark: MBX_CMD_LS_ERASE, MBX_CMD_LS_STORE and
		 * MBX_CMD_LS_LOAD commands are obsolete for such cut
		 */
	case MBX_CMD_LS_READ:
		rpmx_error = ls_read(g_ks_nvm_ctx, mb_cmd_data.rw.id,
				     mb_cmd_data.rw.data, &mb_cmd_data.rw.size);
		resp_size = sizeof(struct ks_nvm_rw_t) + mb_cmd_data.rw.size -
			    sizeof(mb_cmd_data.rw.data);
		if (rpmx_error == RPMx_KO)
			ls_error = LS_NOTFOUND;
		else if (rpmx_error != RPMx_OK)
			ls_error = LS_FLASH_FAILURE;
		break;

	case MBX_CMD_LS_WRITE:
		rpmx_error = ls_write(g_ks_nvm_ctx, mb_cmd_data.rw.id,
				      mb_cmd_data.rw.data,
				      mb_cmd_data.rw.size);
		if (rpmx_error != RPMx_OK)
			ls_error = LS_FLASH_FAILURE;
		break;

	case MBX_CMD_LS_WRITE_KEY_AND_KVT:
		rpmx_error = ls_write_key_and_kvt(g_ks_nvm_ctx,
					mb_cmd_data.write_key_and_kvt.id,
					mb_cmd_data.write_key_and_kvt.kvt_data,
					mb_cmd_data.write_key_and_kvt.kvt_size,
					mb_cmd_data.write_key_and_kvt.key_data,
					mb_cmd_data.write_key_and_kvt.key_size);
		if (rpmx_error != RPMx_OK)
			ls_error = LS_FLASH_FAILURE;
		break;

		/**********************/
		/* M. COUNTERS IOCTLS */
		/**********************/
	case MBX_CMD_MC_INIT:
		rpmx_error = rpmx_discover(g_ks_nvm_ctx, &desc);
		/* Clear message buffer */
		memset(&mb_cmd_data, 0, sizeof(mb_cmd_data));
		mb_cmd_data.mc.device = desc;
		resp_size = sizeof(mb_cmd_data.mc);
		if (rpmx_error == RPMx_OK)
			mb_cmd_data.mc.status = MC_RPMX_DEVICE_INIT;
		else
			mb_cmd_data.mc.status = MC_RPMX_DEVICE_ERR;
		break;

	case MBX_CMD_MC_SET_ROOT_KEY:
#if KS_MEMTYPE == NVM_SQI
		rpmx_error = rpmx_init(g_ks_nvm_ctx, &desc, mb_cmd_data.data);
#elif KS_MEMTYPE == NVM_MMC
		rpmx_error = rpmx_init(g_ks_nvm_ctx, &desc,
				       hsm_common_buffers.rpmb_payload_buff);
#endif
		/* Clear message buffer */
		memset(&mb_cmd_data, 0, sizeof(mb_cmd_data));
		mb_cmd_data.mc.device = desc;
		resp_size = sizeof(mb_cmd_data.mc);
		switch (rpmx_error) {
		case RPMx_OK:
			mb_cmd_data.mc.status = MC_RPMX_ROOTKEY_INIT;
			break;
		case RPMx_ROOT_KEY_AVAILABLE:
			mb_cmd_data.mc.status = MC_RPMX_ROOTKEY_AVAIL;
			break;
		default:
			mb_cmd_data.mc.status = MC_RPMX_ROOTKEY_ERR;
			break;
		}
		break;

	case MBX_CMD_MC_SET_SESS_KEY:
#if KS_MEMTYPE == NVM_SQI
		rpmx_discover(g_ks_nvm_ctx, &desc);
		/* Clear message buffer */
		memset(&mb_cmd_data, 0, sizeof(mb_cmd_data));
		resp_size = sizeof(mb_cmd_data.mc);
		mb_cmd_data.mc.device = desc;
		if (sqi_rpmc_update_hmac_key_register(g_ks_nvm_ctx,
						      &mb_cmd_data.uhkr) == SQI_RPMC_OK)
			mb_cmd_data.mc.status = MC_RPMX_SESSKEY_INIT;
		else
			mb_cmd_data.mc.status = MC_RPMX_SESSKEY_ERR;
#endif
		break;

	case MBX_CMD_MC_GET_VALUE:
		rpmx_discover(g_ks_nvm_ctx, &desc);
		/* Clear message buffer */
		memset(&mb_cmd_data, 0, sizeof(mb_cmd_data));
		resp_size = sizeof(mb_cmd_data.mc);
		mb_cmd_data.mc.device = desc;
#if KS_MEMTYPE == NVM_SQI
		if (sqi_rpmc_request_monotonic_counter(g_ks_nvm_ctx,
						       &mb_cmd_data.rmc)
								== SQI_RPMC_OK)
			mb_cmd_data.mc.status = MC_RPMX_VALID_VALUE;
		else
			mb_cmd_data.mc.status = MC_RPMX_NO_VALUE;
#elif KS_MEMTYPE == NVM_MMC
		if (rpmb_read_counter_tunelled(g_ks_nvm_ctx,
				hsm_common_buffers.rpmb_payload_buff) == RPMx_OK)
			mb_cmd_data.mc.status = MC_RPMX_VALID_VALUE;
		else
			mb_cmd_data.mc.status = MC_RPMX_NO_VALUE;
#endif
		break;

	case MBX_CMD_MC_READ_VALUE:
#if KS_MEMTYPE == NVM_SQI
		resp_size = sizeof(mb_cmd_data.reply);
		sqi_rpmc_read(g_ks_nvm_ctx, &mb_cmd_data.reply);
#elif KS_MEMTYPE == NVM_MMC
		rpmb_read_counter_tunelled(g_ks_nvm_ctx,
					   hsm_common_buffers.rpmb_payload_buff);
#endif
		break;

	case MBX_CMD_MC_INC_VALUE:
		resp_size = sizeof(mb_cmd_data.mc);
#if KS_MEMTYPE == NVM_SQI
		rpmx_error = sqi_rpmc_increment_monotonic_counter(g_ks_nvm_ctx,
							 &mb_cmd_data.imc);
		/* Clear message buffer */
		memset(&mb_cmd_data, 0, sizeof(mb_cmd_data));
		if (rpmx_error == SQI_RPMC_OK)
			mb_cmd_data.mc.status = MC_RPMX_VALID_VALUE;
		else
			mb_cmd_data.mc.status = MC_RPMX_NO_VALUE;
#elif KS_MEMTYPE == NVM_MMC
		rpmx_error = RPMx_INIT_ERROR;
		/* Select RPMB */
		if (!sdmmc_set_part_config(g_ks_nvm_ctx, MMC_PART_RPMB, -1, -1)) {
			/* Write provided RPMB frame */
			if (!rpmb_request(g_ks_nvm_ctx,
					  (const struct t_rpmb *)hsm_common_buffers.rpmb_payload_buff, true))
				rpmx_error = RPMx_OK;

			/* Return to USER partition */
			sdmmc_set_part_config(g_ks_nvm_ctx, MMC_PART_USER, -1, -1);
		}

		/* Clear message buffer */
		memset(&mb_cmd_data, 0, sizeof(mb_cmd_data));
		if (rpmx_error == RPMx_OK)
			mb_cmd_data.mc.status = MC_RPMX_VALID_VALUE;
		else
			mb_cmd_data.mc.status = MC_RPMX_NO_VALUE;
#endif
		break;

	default:
		break;
	}

	g_mbx_buffer.sts = ls_error;

	/* Copy the answer for current service request */
	if (resp_size)
		memcpy(g_mbx_buffer.data, mb_cmd_data.data, resp_size);

	/* Notify the answer is ready */
	notify();

	/* Enable interrupt mask */
	MBOX_M3_HSM->IMR = MBX_IPSR_ANSWER_MASK;
}

/** @} */
