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
#include "sta_mem_map.h"
#include "CSE_cmd_param.h"
#include "CSE_ext_ECC.h"


#ifdef VERBOSE_UNALIGNED_BUF

/*============================================================================*/
/**
 * @brief  unaligned_param_buffer_1
 * @param  cmd: Command identifier value
 * @param  p1: Parameter 1
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_1(uint32_t cmd, uint32_t p1)
{
	uint32_t issue = 0;

	switch (cmd) {
	case 0x08:  /* load RAM key,  P1 address */
	case 0x0B:  /* Re-seed */
	case 0x0C:  /* Get random PRNG */
	case 0x12:  /* Debug challenge */
	case 0x13:  /* Debug authorization */
	case 0x14:  /* Get random TRNG */
	case 0x90:  /* Load AES-256 RAM key */
	case 0xAC:  /* M3 SHE emulated registers setting */
	case 0xAE:  /* A7 SHE emulated registers setting */
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
 * @param  cmd: Command identifier value
 * @param  p1: Parameter 1
 * @param  p2: Parameter 2
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_2(uint32_t cmd, uint32_t p1, uint32_t p2)
{
	uint32_t issue = 0;

	switch (cmd) {
	case 0x43:  /* Export of RSA key */
	case 0x63:  /* export ECC public key */
	case 0xA2:  /* Read OTP configuration */
	case 0xA3:  /* Write OTP configuration */
	case 0xA6:  /* Write OTP encrypted Boot key */
		if ((p2 % 4) != 0)
			issue = 1;
		break;
	case 0x60:  /* load ECC RAM key */
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
 * @param  cmd: Command identifier value
 * @param  p1: Parameter 1
 * @param  p2: Parameter 2
 * @param  p3: Parameter 3
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_3(uint32_t cmd, uint32_t p1, uint32_t p2,
				  uint32_t p3)
{
	uint32_t issue = 0;

	switch (cmd) {
	case 0x40: /* load RSA RAM key */
	case 0x6C: /* Regenerate ECC Key Pair */
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
 * @param  cmd: Command identifier value
 * @param  p1: Parameter 1
 * @param  p2: Parameter 2
 * @param  p3: Parameter 3
 * @param  p4: Parameter 4
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_4(uint32_t cmd, uint32_t p1, uint32_t p2,
				  uint32_t p3, uint32_t p4)
{
	uint32_t issue = 0;

	switch (cmd) {
	case 0x01:  /* ECB encryption */
	case 0x81:
	case 0x03:  /* ECB decryption */
	case 0x83:
	case 0x93:  /* AES-256-ECB encryption */
	case 0x94:  /* AES-256-ECB decryption */
	case 0x99:  /* AES-256-CMAC generation */
	case 0x9A:  /* AES-256-CMAC verification */
		if (((p3 % 4) != 0) || ((p4 % 4) != 0))
			issue = 1;
		break;
	case 0x05:  /* CMAC generation/verification */
	case 0x85:
	case 0x67:  /* CSE_ECC_ECIES_ENCRYPT */
	case 0x68:  /* CSE_ECC_ECIES_DECRYPT */
		if (((p2 % 4) != 0) || ((p3 % 4) != 0) || ((p4 % 4) != 0))
			issue = 1;
		break;
	case 0x50:  /* HASH */
	case 0x72:  /* ECC Scalar multiplication */
		if (((p2 % 4) != 0) || ((p3 % 4) != 0))
			issue = 1;
		break;
	case 0x10:  /* GET ID */
		if (((p1 % 4) != 0) || ((p2 % 4) != 0) || ((p4 % 4) != 0))
			issue = 1;
		break;
	case 0x70:  /* ECC ECDSA Digest signature Generation  */
	case 0x71:  /* ECC ECDSA Digest signature Verification */
		if (((p2 % 4) != 0) || ((p4 % 4) != 0))
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
 * @param  cmd: Command identifier value
 * @param  p1: Parameter 1
 * @param  p2: Parameter 2
 * @param  p3: Parameter 3
 * @param  p4: Parameter 4
 * @param  p5: Parameter 5
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
uint32_t unaligned_param_buffer_5(uint32_t cmd, uint32_t p1, uint32_t p2,
				  uint32_t p3, uint32_t p4, uint32_t p5)
{
	uint32_t issue = 0;

	switch (cmd) {
	case 0x02:  /* CBC encryption */
	case 0x82:
	case 0x04:  /* CBC decryption */
	case 0x84:
	case 0x95:  /* AES-256-CBC encryption */
	case 0x96:  /* AES-256-CBC decryption */
	case 0x97:  /* AES-256-CCM encryption */
	case 0x98:  /* AES-256-CCM decryption */
	case 0x9B:  /* AES-256-GCM encryption */
	case 0x9C:  /* AES-256-GCM decryption */
		if (((p2 % 4) != 0) || ((p4 % 4) != 0) || ((p5 % 4) != 0))
			issue = 1;
		break;
	case 0x07:  /* Load key */
	case 0x87:
	case 0x09:  /* export RAM key */
	case 0x41:  /* Load RSA key */
	case 0x44:  /* Export RSA private key */
	case 0x61:  /* Load ECC key */
	case 0x64:  /* Export ECC private key */
	case 0x91:  /* Export AES-256 RAM key */
	case 0x92:  /* Load AES-256 key */
		if (((p1 % 4) != 0) || ((p2 % 4) != 0) || ((p3 % 4) != 0) ||
		    ((p4 % 4) != 0) || ((p5 % 4) != 0))
			issue = 1;
		break;
	case 0x06:  /* AES CMAC verification */
	case 0x86:
		if (((p2 % 4) != 0) || ((p3 % 4) != 0) || ((p4 % 4) != 0))
			issue = 1;
		break;
	case 0x45:  /* RSA PKCS1_15 Signature generation */
	case 0x46:  /* RSA PKCS1_15 Signature verification */
	case 0x47:  /* RSA PKCS1_15 encryption */
	case 0x48:  /* RSA PKCS1_15 decryption */
	case 0x49:  /* RSA mod exp */
	case 0x65:  /* ECC ECDSA signature Generation  */
	case 0x66:  /* ECC ECDSA signature Verification */
		if (((p2 % 4) != 0) || ((p4 % 4) != 0))
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
	if ((*p) && !(*p & MEM_ALIAS_AREA))
		*p += ESRAM_A7_BASE;
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

static void mem_alias_ecies_data_ptr(struct ecies_shared_data_stt *p)
{
	mem_alias_one_ptr((struct param_field_ptr *)(p->pshared_data1_stt));
	mem_alias_one_ptr((struct param_field_ptr *)(p->pshared_data2_stt));

	mem_alias_alignment((uint32_t *)&p->pshared_data1_stt);
	mem_alias_alignment((uint32_t *)&p->pshared_data2_stt);
}

static void mem_alias_ecies_msg_ptr(struct ecies_enc_msg_stt *p)
{
	mem_alias_two_ptr((struct param_field_ptr *)(p->pephem_pub_key));

	mem_alias_alignment((uint32_t *)&p->pephem_pub_key);
	mem_alias_alignment((uint32_t *)&p->pecies_enc_msg);
	mem_alias_alignment((uint32_t *)&p->ptag);
}


/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_1
 * @param  cmd: Command identifier value
 * @param  p1: Parameter 1
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_1(uint32_t cmd, uint32_t *p1)
{
	/* Mem alias update for specific parameter content */
	switch (cmd) {
	case 0x90:  /* Load AES-256 RAM key */
		mem_alias_one_ptr((struct param_field_ptr *)(*p1));
		break;
	default:
		break;
	}

	switch (cmd) {
	case 0x08:  /* load RAM key,  P1 address */
	case 0x0B:  /* Re-seed */
	case 0x0C:  /* Get random PRNG */
	case 0x12:  /* Debug challenge */
	case 0x13:  /* Debug authorization */
	case 0x14:  /* Get random TRNG */
	case 0x90:  /* Load AES-256 RAM key */
	case 0xAC:  /* M3 SHE emulated registers setting */
	case 0xAE:  /* A7 SHE emulated registers setting */
		mem_alias_alignment(p1);
		break;
	default:
		break;
	}
}

/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_2
 * @param  cmd: Command identifier value
 * @param  p1: Parameter 1
 * @param  p2: Parameter 2
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_2(uint32_t cmd, uint32_t *p1, uint32_t *p2)
{
	/* Mem alias update for specific parameter content */
	switch (cmd) {
	case 0x43:  /* Export of RSA key */
	case 0x63:  /* export ECC public key */
		mem_alias_two_ptr((struct param_field_ptr *)(*p2));
		break;
	case 0x60:  /* load ECC RAM key */
		mem_alias_two_ptr((struct param_field_ptr *)(*p1));
		mem_alias_one_ptr((struct param_field_ptr *)(*p2));
		break;
	default:
		break;
	}

	switch (cmd) {
	case 0x60:  /* load ECC RAM key */
		mem_alias_alignment(p1);
		/* intentional fallthrough */
	case 0x43:  /* Export of RSA key */
	case 0x63:  /* export ECC public key */
	case 0xA2:  /* Read OTP configuration */
	case 0xA6:  /* Write OTP encrypted Boot key */
		mem_alias_alignment(p2);
		break;
	default:
		break;
	}
}

/*============================================================================*/
/**
 * @brief  mem_alias_param_buffer_3
 * @param  cmd: Command identifier value
 * @param  p1: Parameter 1
 * @param  p2: Parameter 2
 * @param  p3: Parameter 3
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_3(uint32_t cmd, uint32_t *p1, uint32_t *p2,
			      uint32_t *p3)
{
	switch (cmd) {
	case 0x40:  /* load RSA RAM key */
		/* Mem alias update for specific parameter content */
		mem_alias_one_ptr((struct param_field_ptr *)(*p1));
		mem_alias_one_ptr((struct param_field_ptr *)(*p2));
		mem_alias_one_ptr((struct param_field_ptr *)(*p3));
		/* intentional fallthrough */
	case 0x6C:  /* Regenerate ECC Key Pair */
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
 * @param  cmd: Command identifier value
 * @param  p1: Parameter 1
 * @param  p2: Parameter 2
 * @param  p3: Parameter 3
 * @param  p4: Parameter 4
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_4(uint32_t cmd, uint32_t *p1, uint32_t *p2,
			      uint32_t *p3, uint32_t *p4)
{
	/* Mem alias update for specific parameter content */
	switch (cmd) {
	case 0x99:  /* AES-256-CMAC generation */
	case 0x9A:  /* AES-256-CMAC verification */
		mem_alias_one_ptr((struct param_field_ptr *)(*p3));
		mem_alias_one_ptr((struct param_field_ptr *)(*p4));
		break;
	case 0x67:  /* CSE_ECC_ECIES_ENCRYPT */
		mem_alias_one_ptr((struct param_field_ptr *)(*p2));
		mem_alias_ecies_data_ptr((struct ecies_shared_data_stt *)(*p3));
		mem_alias_ecies_msg_ptr((struct ecies_enc_msg_stt *)(*p4));
		break;
	case 0x68:  /* CSE_ECC_ECIES_DECRYPT */
		mem_alias_ecies_msg_ptr((struct ecies_enc_msg_stt *)(*p2));
		mem_alias_ecies_data_ptr((struct ecies_shared_data_stt *)(*p3));
		mem_alias_one_ptr((struct param_field_ptr *)(*p4));
		break;
	case 0x70:  /* ECC ECDSA Digest signature Generation  */
	case 0x71:  /* ECC ECDSA Digest signature Verification */
		mem_alias_two_ptr((struct param_field_ptr *)(*p4));
		break;
	case 0x72:  /* ECC Scalar multiplication */
		mem_alias_one_ptr((struct param_field_ptr *)(*p2));
		mem_alias_one_ptr((struct param_field_ptr *)(*p3));
		break;
	default:
		break;
	}

	switch (cmd) {
	case 0x01:  /* ECB encryption */
	case 0x81:
	case 0x03:  /* ECB decryption */
	case 0x83:
	case 0x93:  /* AES-256-ECB encryption */
	case 0x94:  /* AES-256-ECB decryption */
	case 0x99:  /* AES-256-CMAC generation */
	case 0x9A:  /* AES-256-CMAC verification */
		mem_alias_alignment(p3);
		mem_alias_alignment(p4);
		break;
	case 0x05:  /* CMAC generation/verification */
	case 0x85:
	case 0x67:  /* CSE_ECC_ECIES_ENCRYPT */
	case 0x68:  /* CSE_ECC_ECIES_DECRYPT */
		mem_alias_alignment(p4);
		/* intentional fallthrough */
	case 0x50:  /* HASH */
	case 0x72:  /* ECC Scalar multiplication */
		mem_alias_alignment(p2);
		mem_alias_alignment(p3);
		break;
	case 0x10:  /* GET ID */
		mem_alias_alignment(p1);
		mem_alias_alignment(p2);
		mem_alias_alignment(p4);
		break;
	case 0x70:  /* ECC ECDSA Digest signature Generation  */
	case 0x71:  /* ECC ECDSA Digest signature Verification */
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
 * @param  cmd: Command identifier value
 * @param  p1: Parameter 1
 * @param  p2: Parameter 2
 * @param  p3: Parameter 3
 * @param  p4: Parameter 4
 * @param  p5: Parameter 5
 *
 * @retval error status: 1 for error and 0 for no issue
 *
 */
void mem_alias_param_buffer_5(uint32_t cmd, uint32_t *p1, uint32_t *p2,
			      uint32_t *p3, uint32_t *p4, uint32_t *p5)
{
	/* Mem alias update for specific parameter content */
	switch (cmd) {
	case 0x65:  /* ECC ECDSA signature Generation  */
	case 0x66:  /* ECC ECDSA signature Verification */
		mem_alias_two_ptr((struct param_field_ptr *)(*p4));
		break;
	case 0x97:  /* AES-256-CCM encryption */
	case 0x98:  /* AES-256-CCM decryption */
		mem_alias_two_ptr((struct param_field_ptr *)(*p2));
		mem_alias_one_ptr((struct param_field_ptr *)(*p4));
		mem_alias_one_ptr((struct param_field_ptr *)(*p5));
		break;
	case 0x9B:  /* AES-256-GCM encryption */
	case 0x9C:  /* AES-256-GCM decryption */
		mem_alias_two_ptr((struct param_field_ptr *)(*p2));
		mem_alias_one_ptr((struct param_field_ptr *)(*p4));
		mem_alias_two_ptr((struct param_field_ptr *)(*p5));
		break;
	default:
		break;
	}

	switch (cmd) {
	case 0x07:  /* Load key */
	case 0x87:
	case 0x09:  /* export RAM key */
	case 0x41:  /* Load RSA key */
	case 0x44:  /* Export RSA private key */
	case 0x61:  /* Load ECC key */
	case 0x64:  /* Export ECC private key */
	case 0x91:  /* Export AES-256 RAM key */
	case 0x92:  /* Load AES-256 key */
		mem_alias_alignment(p1);
		mem_alias_alignment(p3);
		/* intentional fallthrough */
	case 0x02:  /* CBC encryption */
	case 0x82:
	case 0x04:  /* CBC decryption */
	case 0x84:
	case 0x95:  /* AES-256-CBC encryption */
	case 0x96:  /* AES-256-CBC decryption */
	case 0x97:  /* AES-256-CCM encryption */
	case 0x98:  /* AES-256-CCM decryption */
	case 0x9B:  /* AES-256-GCM encryption */
	case 0x9C:  /* AES-256-GCM decryption */
		mem_alias_alignment(p2);
		mem_alias_alignment(p4);
		mem_alias_alignment(p5);
		break;
	case 0x06:  /* AES CMAC verification */
	case 0x86:
		mem_alias_alignment(p3);
		/* intentional fallthrough */
	case 0x45:  /* RSA PKCS1_15 Signature generation */
	case 0x46:  /* RSA PKCS1_15 Signature verification */
	case 0x47:  /* RSA PKCS1_15 encryption */
	case 0x48:  /* RSA PKCS1_15 decryption */
	case 0x49:  /* RSA mod exp */
	case 0x65:  /* ECC ECDSA signature Generation  */
	case 0x66:  /* ECC ECDSA signature Verification */
		mem_alias_alignment(p2);
		mem_alias_alignment(p4);
		break;
	default:
		break;
	}
}

#endif /* MEM_ALIAS_BUF */

