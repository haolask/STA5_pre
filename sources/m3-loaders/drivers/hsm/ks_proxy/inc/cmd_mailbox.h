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
 * @file    cmd_mailbox.h
 * @brief   Command Mailbox abstraction header file.
 *
 * @addtogroup CMD_MAILBOX
 * @{
 */

#ifndef _CMD_MAILBOX_H_
#define _CMD_MAILBOX_H_

#include "cse_client.h"
#include "loadstore.h"

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/
#ifndef RESERVE_MBX_BUFFER
#define RESERVE_MBX_BUFFER extern
#endif

#define MAX_PAYLOAD_DATA	868 /* To adjust for KS NVM read needs
				     * Size max RSA key: 868 Bytes
				     * Size max KVT for 100 keys: 848 Bytes*/

/*
 * Timeout in milliseconds for M3 requests.
 */
#define MBX_TIMEOUT     500

/*
 * Mask of KS Physical mailbox
 */
#define MBX_IRSR_REQUEST_MASK   ((uint32_t)(1U << 15))
#define MBX_IPSR_ANSWER_MASK    ((uint32_t)(1U << 15))

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/
typedef enum {
	MBX_CMD_LS_INIT		= 0,
	MBX_CMD_LS_DEINIT	= 1,
	MBX_CMD_LS_LOAD		= 2,
	MBX_CMD_LS_STORE	= 3,
	MBX_CMD_LS_ERASE	= 4,
	MBX_CMD_MC_INIT		= 5,
	MBX_CMD_MC_DEINIT	= 6,
	MBX_CMD_MC_GET_MAX	= 7,
	MBX_CMD_MC_READ		= 8,
	MBX_CMD_MC_INCREASE	= 9,
	MBX_CMD_MC_SET_ROOT_KEY	= 10,
	MBX_CMD_MC_SET_SESS_KEY	= 11,
	MBX_CMD_MC_GET_VALUE	= 12,
	MBX_CMD_MC_READ_VALUE	= 13,
	MBX_CMD_MC_INC_VALUE	= 14,
	MBX_CMD_LS_READ		= 15,
	MBX_CMD_LS_WRITE	= 16,
	MBX_CMD_LS_WRITE_KEY_AND_KVT	= 17
} mbxcmd_t;

typedef enum {
	MBX_STS_NOERROR         = LS_NOERROR,
	MBX_STS_NOTINIT         = LS_NOTINIT,
	MBX_STS_WARNING         = LS_WARNING,
	MBX_STS_NOTFOUND        = LS_NOTFOUND,
	MBX_STS_FLASH_FAILURE   = LS_FLASH_FAILURE,
	MBX_STS_INTERNAL        = LS_INTERNAL
} mbxsts_t;

/**
 * @brief   KS NVM Read/Write payload struct
 */
struct ks_nvm_rw_t {
	uint32_t id;
	uint32_t size;
	uint8_t data[MAX_PAYLOAD_DATA];
};

/**
 * @brief   KS NVM Write Key and Key table payload struct
 */
struct ks_nvm_write_key_and_kvt_t {
	uint32_t id;
	uint32_t kvt_size;
	uint8_t kvt_data[MAX_PAYLOAD_DATA];
	uint32_t key_size;
	uint8_t key_data[MAX_PAYLOAD_DATA];
};

union t_mbx_cmd_data {
	uint8_t data[MAX_PAYLOAD_DATA];
	void *address;
	uint32_t size;
	struct mc_rpmx_msg_t mc;
	struct sqi_rpmc_imc_t imc;
	struct sqi_rpmc_rmc_t rmc;
	struct sqi_rpmc_uhkr_t uhkr;
	struct sqi_rpmc_read_reply_t reply;
	struct ks_nvm_rw_t rw;
	struct ks_nvm_write_key_and_kvt_t write_key_and_kvt;
};

struct mbxbuffer_t {
	mbxcmd_t      cmd;
	mbxsts_t      sts;
	uint8_t       data[sizeof(union t_mbx_cmd_data)];
};

/*===========================================================================*/
/* Public variables                                                          */
/*===========================================================================*/
RESERVE_MBX_BUFFER struct mbxbuffer_t g_mbx_buffer
			__attribute__((section(".ks_mbx_buffer")));

extern union t_mbx_cmd_data mb_cmd_data;
extern uint8_t mb_cmd;

/*===========================================================================*/
/* Public functions                                                          */
/*===========================================================================*/
/**
 * @brief	Sends Key Storage commands
 *
 * @param[in]	cmd     pointer to input message buffer
 * @param[in]   data    pointer of data
 *
 * @return      Command status
 * @retval      0	When the command is well managed
 * @retval      1..5	In case of error
 */
mbxsts_t mbxSendCommand(mbxcmd_t cmd, uint8_t *data);

/**
 * @brief       Notification function
 */
static inline void notify(void)
{
	MBOX_M3_HSM->IRSR = MBX_IRSR_REQUEST_MASK;
}

/**
 * @brief       Answer notification function
 */
static inline bool answer_notification(void)
{
	return (MBOX_M3_HSM->IPSR & MBX_IPSR_ANSWER_MASK) != 0;
}

/**
 * @brief       Clear notification function
 */
static inline void clear_notification(void)
{
	MBOX_M3_HSM->IPSR = ~MBX_IPSR_ANSWER_MASK;
}

#endif /* _CMD_MAILBOX_H_ */

/** @} */
