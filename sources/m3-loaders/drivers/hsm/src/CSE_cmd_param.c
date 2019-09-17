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

/**
 * @file    CSE_cmd_param.c
 * @brief   Message parameters function.
 * @details Expose functions to check command parameters
 *
 *
 * @addtogroup UserAPI Single Call user level API
 * @{
 * @addtogroup API Functions
 * @{
 */

#include "config.h"
#include "cse_typedefs.h"
#include "sta_map.h"
#include "CSE_cmd_param.h"
#include "CSE_ext_ECC.h"

#ifdef ST_TEST_MODE
#include "cse_st_private_cmd.h"
#endif

#ifdef VERBOSE_UNALIGNED_BUF

/*============================================================================*/
/**
 * @brief  unaligned_param_buffer_1
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_1(uint32_t cmd, uint32_t p1)
{
	uint32_t issue = 0;

	switch (cmd) {
	case CSE_LOAD_PLAIN_KEY:  /* load RAM key,  P1 address */
	case CSE_EXTEND_SEED:	/* Re-seed */
	case CSE_RND:  /* Get random PRNG */
	case CSE_DEBUG_CHAL:  /* Debug challenge */
	case CSE_DEBUG_AUTH:  /* Debug authorization */
	case CSE_TRNG_RND:  /* Get random TRNG */
	case CSE_AES256_LOAD_RAM_KEY:  /* Load AES-256 RAM key */
	case CSE_EXT_SET_M3_SHE_REGISTERS_MEM_CONFIG:  /* M3 SHE emulated reg */
	case CSE_EXT_SET_A7_SHE_REGISTERS_MEM_CONFIG:  /* A7 SHE emulated reg */
		if ((p1 % 4) != 0)
			issue = 1;
		break;
	default:
		break;
	}

	return issue;
}

/*============================================================================*/
/**
 * @brief  unaligned_param_buffer_2
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_2(uint32_t cmd, uint32_t p1, uint32_t p2)
{
	uint32_t issue = 0;

	switch (cmd) {
	case CSE_RSA_EXPORT_PUBLIC_KEY:  /* Export of RSA key */
	case CSE_ECC_GENERATE_LOAD_KEY_PAIR:  /* generate ECC key */
	case CSE_ECC_EXPORT_PUBLIC_KEY:  /* export ECC public key */
	case CSE_OTP_GET_CONFIG:  /* Read OTP configuration */
	case CSE_OTP_SET_CONFIG:  /* Write OTP configuration */
	case CSE_OTP_WRITE_SEC_BOOT_KEY:  /* Write OTP encrypted Boot key */
	case CSE_ECC_ECDH_KEY_AGREEMENT: /* ECDH / ECDHe key agreement */
	/* ECDH fixed key agreement (RAM and NVM locations) */
	case CSE_ECC_ECDHfixed_KEY_AGREEMENT:
	/* ECDH fixed key agreement (only RAM locations) */
	case CSE_ECC_ECDHfixed_SERVER_KEY_AGREEMENT:
		if ((p2 % 4) != 0)
			issue = 1;
		break;
	case CSE_ECC_LOAD_RAM_KEY:  /* load ECC RAM key */
		if (((p1 % 4) != 0) || ((p2 % 4) != 0))
			issue = 1;
		break;
	default:
		break;
	}

	return issue;
}

/*============================================================================*/
/**
 * @brief  unaligned_param_buffer_3
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 * @param  InputMessageLength: Parameter 3
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_3(uint32_t cmd, uint32_t p1, uint32_t p2,
				  uint32_t p3)
{
	uint32_t issue = 0;

	switch (cmd) {
	case CSE_RSA_LOAD_RAM_KEY: /* load RSA RAM key */
		if (((p1 % 4) != 0) || ((p2 % 4) != 0) || ((p3 % 4) != 0))
			issue = 1;
		break;
	default:
		break;
	}

	return issue;
}

/*============================================================================*/
/**
 * @brief  unaligned_param_buffer_4
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 * @param  InputMessageLength: Parameter 3
 * @param  InputMessageLength: Parameter 4
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_4(uint32_t cmd, uint32_t p1, uint32_t p2,
				  uint32_t p3, uint32_t p4)
{
	uint32_t issue = 0;

	switch (cmd) {
	case CSE_ENC_ECB:  /* ECB encryption */
	case CSE_ENC_ECB_EXT:
	case CSE_DEC_ECB:  /* ECB decryption */
	case CSE_DEC_ECB_EXT:
	case CSE_AES256_ECB_ENCRYPT:  /* AES-256-ECB encryption */
	case CSE_AES256_ECB_DECRYPT:  /* AES-256-ECB decryption */
	case CSE_AES256_CMAC_GENERATE:  /* AES-256-CMAC generation */
	case CSE_AES256_CMAC_VERIFY:  /* AES-256-CMAC verification */
		if (((p3 % 4) != 0) || ((p4 % 4) != 0))
			issue = 1;
		break;
	case CSE_GENERATE_MAC:  /* CMAC generation/verification */
	case CSE_GENERATE_MAC_EXT:
	case CSE_ECC_ECIES_ENCRYPT:  /* ECC ECIES encryption */
	case CSE_ECC_ECIES_DECRYPT:  /* ECC ECIES decryption */
		if (((p2 % 4) != 0) || ((p3 % 4) != 0) || ((p4 % 4) != 0))
			issue = 1;
		break;
	case CSE_HASH:    /* HASH */
		if (((p2 % 4) != 0) || ((p3 % 4) != 0))
			issue = 1;
		break;
	case CSE_GET_ID:  /* GET ID */
		if (((p1 % 4) != 0) || ((p2 % 4) != 0) || ((p4 % 4) != 0))
			issue = 1;
		break;
	default:
		break;
	}

	return issue;
}

/*============================================================================*/
/**
 * @brief  unaligned_param_buffer_5
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 * @param  InputMessageLength: Parameter 3
 * @param  InputMessageLength: Parameter 4
 * @param  InputMessageLength: Parameter 5
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_5(uint32_t cmd, uint32_t p1, uint32_t p2,
				  uint32_t p3, uint32_t p4, uint32_t p5)
{
	uint32_t issue = 0;

	switch (cmd) {
	case CSE_ENC_CBC:  /* CBC encryption */
	case CSE_ENC_CBC_EXT:
	case CSE_DEC_CBC:  /* CBC decryption */
	case CSE_DEC_CBC_EXT:
	case CSE_AES256_CBC_ENCRYPT:  /* AES-256-CBC encryption */
	case CSE_AES256_CBC_DECRYPT:  /* AES-256-CBC decryption */
	case CSE_AES256_CCM_ENCRYPT:  /* AES-256-CCM encryption */
	case CSE_AES256_CCM_DECRYPT:  /* AES-256-CCM decryption */
	case CSE_AES256_GCM_ENCRYPT:  /* AES-256-GCM encryption */
	case CSE_AES256_GCM_DECRYPT:  /* AES-256-GCM decryption */
		if (((p2 % 4) != 0) || ((p4 % 4) != 0) || ((p5 % 4) != 0))
			issue = 1;
		break;
	case CSE_LOAD_KEY:  /* Load key */
	case CSE_LOAD_KEY_EXT:
	case CSE_EXPORT_RAM_KEY:  /* export RAM key */
	case CSE_RSA_LOAD_KEY:  /* Load RSA key */
	case CSE_RSA_EXPORT_PRIVATE_KEY:  /* Export RSA private key */
	case CSE_ECC_LOAD_KEY:  /* Load ECC key */
	case CSE_ECC_EXPORT_PRIVATE_KEY:  /* Export ECC private key */
	case CSE_AES256_EXPORT_RAM_KEY:  /* Export AES-256 RAM key */
	case CSE_AES256_LOAD_KEY:  /* Load AES-256 key */
		if (((p1 % 4) != 0) || ((p2 % 4) != 0) || ((p3 % 4) != 0) ||
		    ((p4 % 4) != 0) || ((p5 % 4) != 0))
			issue = 1;
		break;
	case CSE_VERIFY_MAC:  /* AES CMAC verification */
	case CSE_VERIFY_MAC_EXT:
		if (((p2 % 4) != 0) || ((p3 % 4) != 0) || ((p4 % 4) != 0))
			issue = 1;
		break;
	case CSE_RSA_PKCS1_15_SIGN:  /* RSA PKCS1_15 Signature generation */
	case CSE_RSA_PKCS1_15_VERIFY:  /* RSA PKCS1_15 Signature verification */
	case CSE_RSA_PKCS1_15_ENCRYPT:  /* RSA PKCS1_15 encryption */
	case CSE_RSA_PKCS1_15_DECRYPT:  /* RSA PKCS1_15 decryption */
#ifdef ST_TEST_MODE
	case CSE_RSA_MOD_EXP_WITH_PUB_EXP:  /* RSA mod exp */
#endif
	case CSE_ECC_ECDSA_SIGN:  /* ECC ECDSA signature Generation  */
	case CSE_ECC_ECDSA_VERIFY:  /* ECC ECDSA signature Verification */
		if (((p2 % 4) != 0) || ((p4 % 4) != 0))
			issue = 1;
		break;
	case CSE_TLSV12_PRF: /* TLS PRF */
		if (((p1 % 4) != 0) || ((p2 % 4) != 0) || ((p3 % 4) != 0) ||
		    ((p5 % 4) != 0))
			issue = 1;
		break;
	default:
		break;
	}

	return issue;
}
#endif /* VERBOSE_UNALIGNED_BUF */


#ifdef MEM_ALIAS_BUF

#define MEM_ALIAS_AREA		0xF0000000

struct param_field_ptr {
	uint8_t *pf1;
	uint32_t f1;
	uint8_t *pf2;
	uint32_t f2;
};


static void mem_alias_alignment(uint32_t *p)
{
	if (!(*p & MEM_ALIAS_AREA))
		*p += M3_SRAM_BASE;
}

static void mem_alias_one_ptr(struct param_field_ptr *p)
{
	mem_alias_alignment((uint32_t *)&p->pf1);
}

static void mem_alias_two_ptr(struct param_field_ptr *p)
{
	mem_alias_alignment((uint32_t *)&p->pf1);
	mem_alias_alignment((uint32_t *)&p->pf2);
}

static void mem_alias_ECIES_Data_ptr(struct ECIES_sharedData_stt *p)
{
	mem_alias_one_ptr((struct param_field_ptr *)(p->pSharedData1_stt));
	mem_alias_one_ptr((struct param_field_ptr *)(p->pSharedData2_stt));

	mem_alias_alignment((uint32_t *)&p->pSharedData1_stt);
	mem_alias_alignment((uint32_t *)&p->pSharedData2_stt);
}

static void mem_alias_ECIES_Msg_ptr(struct ECIES_encMsg_stt *p)
{
	mem_alias_two_ptr((struct param_field_ptr *)(p->pSenderFmrPubKey));

	mem_alias_alignment((uint32_t *)&p->pSenderFmrPubKey);
	mem_alias_alignment((uint32_t *)&p->pEciesEncryptedMessage);
	mem_alias_alignment((uint32_t *)&p->pTag);
}


/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_1
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_1(uint32_t cmd, uint32_t *p1)
{
	/* Mem alias update for specific parameter content */
	switch (cmd) {
	case CSE_AES256_LOAD_RAM_KEY:  /* Load AES-256 RAM key */
		mem_alias_one_ptr((struct param_field_ptr *)(*p1));
		break;
	default:
		break;
	}

	switch (cmd) {
	case CSE_LOAD_PLAIN_KEY:  /* load RAM key,  P1 address */
	case CSE_EXTEND_SEED:  /* Re-seed */
	case CSE_RND:  /* Get random PRNG */
	case CSE_DEBUG_CHAL:  /* Debug challenge */
	case CSE_DEBUG_AUTH:  /* Debug authorization */
	case CSE_TRNG_RND:  /* Get random TRNG */
	case CSE_AES256_LOAD_RAM_KEY:  /* Load AES-256 RAM key */
	case CSE_EXT_SET_M3_SHE_REGISTERS_MEM_CONFIG:  /* M3 SHE emulated reg */
	case CSE_EXT_SET_A7_SHE_REGISTERS_MEM_CONFIG:  /* A7 SHE emulated reg */
		mem_alias_alignment(p1);
		break;
	default:
		break;
	}
}

/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_2
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_2(uint32_t cmd, uint32_t *p1, uint32_t *p2)
{
	/* Mem alias update for specific parameter content */
	switch (cmd) {
	case CSE_RSA_EXPORT_PUBLIC_KEY:  /* Export of RSA key */
	case CSE_ECC_EXPORT_PUBLIC_KEY:  /* export ECC public key */
		mem_alias_two_ptr((struct param_field_ptr *)(*p2));
		break;
	case CSE_ECC_LOAD_RAM_KEY:  /* load ECC RAM key */
	case CSE_ECC_ECDHfixed_SERVER_KEY_AGREEMENT:  /* ECDH fixed key agreement (only RAM locations) */
		mem_alias_two_ptr((struct param_field_ptr *)(*p1));
		mem_alias_one_ptr((struct param_field_ptr *)(*p2));
		break;
	case CSE_ECC_ECDH_KEY_AGREEMENT:  /* ECDH / ECDHe key agreement */
	case CSE_ECC_ECDHfixed_KEY_AGREEMENT:  /* ECDH fixed key agreement (RAM and NVM locations) */
		mem_alias_one_ptr((struct param_field_ptr *)(*p2));
		break;
	default:
		break;
	}

	switch (cmd) {
	case CSE_EXT_UPDATE_SERVICE_SET:
		mem_alias_alignment(p1);
		break;
	case CSE_ECC_LOAD_RAM_KEY:  /* load ECC RAM key */
	/* ECDH fixed key agreement (only RAM locations) */
	case CSE_ECC_ECDHfixed_SERVER_KEY_AGREEMENT:
		mem_alias_alignment(p1);
		/* intentional fallthrough */
	case CSE_RSA_EXPORT_PUBLIC_KEY:  /* Export of RSA key */
	case CSE_ECC_EXPORT_PUBLIC_KEY:  /* export ECC public key */
	case CSE_OTP_GET_CONFIG:  /* Read OTP configuration */
	case CSE_OTP_WRITE_SEC_BOOT_KEY:  /* Write OTP encrypted Boot key */
	case CSE_ECC_ECDH_KEY_AGREEMENT: /* ECDH / ECDHe key agreement */
	/* ECDH fixed key agreement (RAM and NVM locations) */
	case CSE_ECC_ECDHfixed_KEY_AGREEMENT:
		mem_alias_alignment(p2);
		break;
	default:
		break;
	}
}

/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_3
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 * @param  InputMessageLength: Parameter 3
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_3(uint32_t cmd, uint32_t *p1, uint32_t *p2,
			      uint32_t *p3)
{
	switch (cmd) {
	case CSE_RSA_LOAD_RAM_KEY:  /* load RSA RAM key */
		/* Mem alias update for specific parameter content */
		mem_alias_one_ptr((struct param_field_ptr *)(*p1));
		mem_alias_one_ptr((struct param_field_ptr *)(*p2));
		mem_alias_one_ptr((struct param_field_ptr *)(*p3));

		mem_alias_alignment(p1);
		mem_alias_alignment(p2);
		mem_alias_alignment(p3);
		break;
	default:
		break;
	}
}

/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_4
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 * @param  InputMessageLength: Parameter 3
 * @param  InputMessageLength: Parameter 4
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_4(uint32_t cmd, uint32_t *p1, uint32_t *p2,
			      uint32_t *p3, uint32_t *p4)
{
	/* Mem alias update for specific parameter content */
	switch (cmd) {
	case CSE_AES256_CMAC_GENERATE:  /* AES-256-CMAC generation */
	case CSE_AES256_CMAC_VERIFY:  /* AES-256-CMAC verification */
		mem_alias_one_ptr((struct param_field_ptr *)(*p3));
		mem_alias_one_ptr((struct param_field_ptr *)(*p4));
		break;
	case CSE_ECC_ECIES_ENCRYPT:  /* ECC ECIES encryption */
		mem_alias_one_ptr((struct param_field_ptr *)(*p2));
		mem_alias_ECIES_Data_ptr((struct ECIES_sharedData_stt *)(*p3));
		mem_alias_ECIES_Msg_ptr((struct ECIES_encMsg_stt *)(*p4));
		break;
	case CSE_ECC_ECIES_DECRYPT:  /* ECC ECIES decryption */
		mem_alias_ECIES_Msg_ptr((struct ECIES_encMsg_stt *)(*p2));
		mem_alias_ECIES_Data_ptr((struct ECIES_sharedData_stt *)(*p3));
		mem_alias_one_ptr((struct param_field_ptr *)(*p4));
		break;
	default:
		break;
	}

	switch (cmd) {
#ifdef ST_TEST_MODE
	case CSE_DEBUG_PATCH:
		mem_alias_alignment(p2);
		break;
#endif
	case CSE_ENC_ECB:  /* ECB encryption */
	case CSE_ENC_ECB_EXT:
	case CSE_DEC_ECB:  /* ECB decryption */
	case CSE_DEC_ECB_EXT:
	case CSE_AES256_ECB_ENCRYPT:  /* AES-256-ECB encryption */
	case CSE_AES256_ECB_DECRYPT:  /* AES-256-ECB decryption */
	case CSE_AES256_CMAC_GENERATE:  /* AES-256-CMAC generation */
	case CSE_AES256_CMAC_VERIFY:  /* AES-256-CMAC verification */
		mem_alias_alignment(p3);
		mem_alias_alignment(p4);
		break;
	case CSE_GENERATE_MAC:  /* CMAC generation/verification */
	case CSE_GENERATE_MAC_EXT:
	case CSE_ECC_ECIES_ENCRYPT:  /* ECC ECIES encryption */
	case CSE_ECC_ECIES_DECRYPT:  /* ECC ECIES decryption */
		mem_alias_alignment(p4);
		/* intentional fallthrough */
	case CSE_HASH:    /* HASH */
		mem_alias_alignment(p2);
		mem_alias_alignment(p3);
		break;
	case CSE_GET_ID:  /* GET ID */
		mem_alias_alignment(p1);
		mem_alias_alignment(p2);
		mem_alias_alignment(p4);
		break;		
	default:
		break;
	}
}

/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_5
 * @param  InputMessage: Command identifier value
 * @param  InputMessageLength: Parameter 1
 * @param  InputMessageLength: Parameter 2
 * @param  InputMessageLength: Parameter 3
 * @param  InputMessageLength: Parameter 4
 * @param  InputMessageLength: Parameter 5
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_5(uint32_t cmd, uint32_t *p1, uint32_t *p2,
			      uint32_t *p3, uint32_t *p4, uint32_t *p5)
{
	/* Mem alias update for specific parameter content */
	switch (cmd) {
	case CSE_ECC_ECDSA_SIGN:  /* ECC ECDSA signature Generation  */
	case CSE_ECC_ECDSA_VERIFY:  /* ECC ECDSA signature Verification */
		mem_alias_two_ptr((struct param_field_ptr *)(*p4));
		break;
	case CSE_AES256_CCM_ENCRYPT:  /* AES-256-CCM encryption */
	case CSE_AES256_CCM_DECRYPT:  /* AES-256-CCM decryption */
		mem_alias_two_ptr((struct param_field_ptr *)(*p2));
		mem_alias_one_ptr((struct param_field_ptr *)(*p4));
		mem_alias_one_ptr((struct param_field_ptr *)(*p5));
		break;
	case CSE_AES256_GCM_ENCRYPT:  /* AES-256-GCM encryption */
	case CSE_AES256_GCM_DECRYPT:  /* AES-256-GCM decryption */
		mem_alias_two_ptr((struct param_field_ptr *)(*p2));
		mem_alias_one_ptr((struct param_field_ptr *)(*p4));
		mem_alias_two_ptr((struct param_field_ptr *)(*p5));
		break;
	case CSE_TLSV12_PRF:  /* TLS PRF */
		mem_alias_one_ptr((struct param_field_ptr *)(*p1));
		mem_alias_one_ptr((struct param_field_ptr *)(*p2));
		mem_alias_one_ptr((struct param_field_ptr *)(*p3));
		mem_alias_one_ptr((struct param_field_ptr *)(*p5));
		break;
	default:
		break;
	}

	switch (cmd) {
	case CSE_LOAD_KEY:  /* Load key */
	case CSE_LOAD_KEY_EXT:
	case CSE_EXPORT_RAM_KEY:  /* export RAM key */
	case CSE_RSA_LOAD_KEY:  /* Load RSA key */
	case CSE_RSA_EXPORT_PRIVATE_KEY:  /* Export RSA private key */
	case CSE_ECC_LOAD_KEY:  /* Load ECC key */
	case CSE_ECC_EXPORT_PRIVATE_KEY:  /* Export ECC private key */
	case CSE_AES256_EXPORT_RAM_KEY:  /* Export AES-256 RAM key */
	case CSE_AES256_LOAD_KEY:  /* Load AES-256 key */
		mem_alias_alignment(p1);
		mem_alias_alignment(p3);
		/* intentional fallthrough */
	case CSE_ENC_CBC:  /* CBC encryption */
	case CSE_ENC_CBC_EXT:
	case CSE_DEC_CBC:  /* CBC decryption */
	case CSE_DEC_CBC_EXT:
	case CSE_AES256_CBC_ENCRYPT:  /* AES-256-CBC encryption */
	case CSE_AES256_CBC_DECRYPT:  /* AES-256-CBC decryption */
	case CSE_AES256_CCM_ENCRYPT:  /* AES-256-CCM encryption */
	case CSE_AES256_CCM_DECRYPT:  /* AES-256-CCM decryption */
	case CSE_AES256_GCM_ENCRYPT:  /* AES-256-GCM encryption */
	case CSE_AES256_GCM_DECRYPT:  /* AES-256-GCM decryption */
		mem_alias_alignment(p2);
		mem_alias_alignment(p4);
		mem_alias_alignment(p5);
		break;
	case CSE_VERIFY_MAC:  /* AES CMAC verification */
	case CSE_VERIFY_MAC_EXT:
		mem_alias_alignment(p3);
		/* intentional fallthrough */
	case CSE_RSA_PKCS1_15_SIGN:  /* RSA PKCS1_15 Signature generation */
	case CSE_RSA_PKCS1_15_VERIFY:  /* RSA PKCS1_15 Signature verification */
	case CSE_RSA_PKCS1_15_ENCRYPT:  /* RSA PKCS1_15 encryption */
	case CSE_RSA_PKCS1_15_DECRYPT:  /* RSA PKCS1_15 decryption */
#ifdef ST_TEST_MODE
	case CSE_RSA_MOD_EXP_WITH_PUB_EXP:  /* RSA mod exp */
#endif
	case CSE_ECC_ECDSA_SIGN:  /* ECC ECDSA signature Generation  */
	case CSE_ECC_ECDSA_VERIFY:  /* ECC ECDSA signature Verification */
		mem_alias_alignment(p2);
		mem_alias_alignment(p4);
		break;
	case CSE_TLSV12_PRF:  /* TLS PRF */
		mem_alias_alignment(p1);
		mem_alias_alignment(p2);
		mem_alias_alignment(p3);
		mem_alias_alignment(p5);
		break;
	default:
		break;
	}
}

#endif /* MEM_ALIAS_BUF */

