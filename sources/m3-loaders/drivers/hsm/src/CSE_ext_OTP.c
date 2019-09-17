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
 * @file    CSE_ext_OTP.c
 * @brief   CSE OTP management module for telemaco3p module.
 * @details Set of functions used to manage access OTP area via the HSM.
 *
 *
 * @addtogroup TC3P_driver
 * @{
 * @addtogroup API Functions
 * @{
 */

#include "cse_typedefs.h"
#include "CSE_Constants.h"
#include "CSE_HAL.h"
#include "CSE_ext_OTP.h"

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

/*============================================================================*/
/*!
 * @brief	otp_Read_Word:
 * @details	This routine reads a word from OTP

 * @param[in]	: word_addr:		progressive word address
 * @param[out]	: word_to_read:		pointer to store the read value
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Read_Word(uint32_t word_addr, uint32_t *pword_to_read)
{
	/* Just ask the CSE to read value from OTP */
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd1p(CSE_OTP_READ_PUBLIC, word_addr);

	if (hsm_ret == CSE_NO_ERR) {
		*pword_to_read = CSE->P2.R;
		ret = OTP_SUCCESS;
	}

	return ret;
}

/*!
 * @brief	otp_Write_Word:
 *		This routine writes a word to OTP
 * @param	: word_addr:		progressive word address
 * @param	: word_to_read:		value to be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Write_Word(uint32_t word_addr, uint32_t word_to_write)
{
	/* Just ask the CSE to write value to OTP specified location */
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd2p(CSE_OTP_WRITE_PUBLIC, word_addr,
				     word_to_write);

	if (hsm_ret == CSE_NO_ERR)
		ret = OTP_SUCCESS;

	return ret;
}

#ifdef HSM_RELEASE
/*!
 * @brief	tp_Get_Security_Level:
 * @details	This routine returns the Security level value word from OTP
 *              If an error occur in the HSM service request, the returned
 *		security level variable is not updated
 * @param[out]	plevel - pointer to a t_boot_security_level variable where
 *		the value will be written
 * @return	: OTP error condition
 * @retva	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Get_Security_Level(enum t_boot_security_level *plevel)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd1p(CSE_OTP_GET_CONFIG, SECURITY_LEVEL);
	if (hsm_ret == CSE_NO_ERR) {
		*plevel = (enum t_boot_security_level)(CSE->P2.R);
		ret = OTP_SUCCESS;
	}

	return ret;
}
#endif

#ifndef HSM_RELEASE
t_bool otp_Is_Empty(void)
{
	t_bool ret = FALSE;

	return ret;
}

/*!
 * @brief	otp_Get_Cust_ID:
 * @details	This routine returns the Customer ID value word from OTP
 *              If an error occur in the HSM service request, the returned
 *		variable is not updated
 * @param[out]	pcid - pointer to a 32b unsigned integer variable where the
 *		value will be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Get_Cust_ID(t_uint32 *pcid)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd1p(CSE_OTP_GET_CONFIG, CUSTOMER_ID);

	if (hsm_ret == CSE_NO_ERR) {
		*pcid = CSE->P2.R;
		ret = OTP_SUCCESS;
	}

	return ret;
}

/*!
 * @brief	otp_Get_Cust_Cert_Ver:
 * @details	This routine returns the Customer Certificate Version word from
 *		OTP
 * @param[out]	pver - pointer to a 32b unsigned integer where the value will
 *		be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Get_Cust_Cert_Ver(t_uint32  *pver)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd1p(CSE_OTP_GET_CONFIG, CUSTOMER_CERT_VER);

	if (hsm_ret == CSE_NO_ERR) {
		*pver = CSE->P2.R;
		ret = OTP_SUCCESS;
	}

	return ret;
}

/*!
 * @brief	otp_Get_Secr_Ver:
 * @details	This routine returns the Security version value word from OTP
 *              If an error occur in the HSM service request, the returned
 *		security level variable is not updated
 * @param[out]	pver - pointer to a 32b unsigned integer variable where the
 *		value will be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Get_Secr_Ver(t_uint32  *pver)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd1p(CSE_OTP_GET_CONFIG, SECURITY_VERSION);

	if (hsm_ret == CSE_NO_ERR) {
		*pver = CSE->P2.R;
		ret = OTP_SUCCESS;
	}

	return ret;
}

/*!
 * @brief	otp_Get_Boot_Parameters
 * @details	This routine returns the Boot parameters value from OTP
 *              If an error occur in the HSM service request, the returned
 *		security level variable is not updated
 * @param[out]	pver - pointer to a t_safemem_boot_cfg variable where the value
 *		will be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Get_Boot_Parameters(struct t_safemem_boot_cfg *pinfo)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd1p(CSE_OTP_GET_CONFIG, BOOT_PARAMETERS);

	if (hsm_ret == CSE_NO_ERR) {
		pinfo->SAFEMEM_BOOT_CONFIG.WORD = CSE->P2.R;
		ret = OTP_SUCCESS;
	}

	return ret;
}

/*!
 * @brief	otp_Get_Customer_Certificate_Hash_Info
 * @details	This routine returns the customer root certificate hash
 *		information from OTP (ie. the type of hash used and if root
 *		customer certificate was programmed)
 * @param[out]	proot_hash_info - pointer to a 32b unsigned integer where the
 *		value will be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Get_Customer_Certificate_Hash_Info(uint32_t *proot_hash_info)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd1p(CSE_OTP_GET_CONFIG,
				     CUSTOMER_CERT_HASH_INFO);
	if (hsm_ret == CSE_NO_ERR) {
		*proot_hash_info = CSE->P2.R;
		ret = OTP_SUCCESS;
	}

	return ret;
}

/*!
 * @brief	otp_Get_Customer_Certificate_Hash
 * @details	This routine first check if the customer root certificate hash
 *		has been written and enabled
 *		If yes, copies the associated 512bits from OTP to the specified
 *		destination
 * @param[out]	phash - pointer to a byte array where the value will be written
 *		- must be at least 64 bytes long (512bits)
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE - if customer hash was not written already, or
 *		errors while reading OTP
 * @retval	: OTP_SUCCESS
 */
int otp_Get_Customer_Certificate_Hash(uint8_t *phash)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd2p(CSE_OTP_GET_CONFIG, CUSTOMER_CERT_HASH,
				     (uint32_t)phash);

	if (hsm_ret == CSE_NO_ERR)
		ret = OTP_SUCCESS;

	return ret;
}

/*
 * Update/Writing functions
 */

/*!
 * @brief	otp_Set_Security_Level
 * @details	This routine writes the provided Security level value to OTP
 *		If Security level field was already written, the request is
 *		rejected
 * @param[in]	level - value to write
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Set_Security_Level(enum t_boot_security_level level)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd2p(CSE_OTP_SET_CONFIG, SECURITY_LEVEL,
				     (uint32_t)level);

	if (hsm_ret == CSE_NO_ERR)
		ret = OTP_SUCCESS;

	return ret;
}

/*!
 * @brief	otp_Set_Cust_ID:
 * @details	This routine writes the provided Customer ID value word to OTP
 *		If Security level field was already written, the request is
 *		rejected
 * @param[out]	cid - v lue to write
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Set_Cust_ID(t_uint32 cid)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd2p(CSE_OTP_SET_CONFIG, CUSTOMER_ID, cid);

	if (hsm_ret == CSE_NO_ERR)
		ret = OTP_SUCCESS;

	return ret;
}

/*!
 * @brief	otp_Incr_Cust_Cert_Ver
 * @details	This routine increments the Customer Certificate Version word in
 *		OTP
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Incr_Cust_Cert_Ver(void)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd1p(CSE_OTP_SET_CONFIG, CUSTOMER_CERT_VER);

	if (hsm_ret == CSE_NO_ERR)
		ret = OTP_SUCCESS;

	return ret;
}

/*!
 * @brief	otp_Get_Secr_Ver:
 * @details	This routine increments the Security version value word in OTP
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Incr_Secr_Ver(void)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd1p(CSE_OTP_SET_CONFIG, SECURITY_VERSION);

	if (hsm_ret == CSE_NO_ERR)
		ret = OTP_SUCCESS;

	return ret;
}

/*!
 * @brief	otp_Set_Boot_Parameters
 * @details	This routine writes the provided Boot parameters value to OTP
 *		If Boot info field was already written, the request is rejected
 * @param[in]	info - value to write
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Set_Boot_Parameters(struct t_safemem_boot_cfg info)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd2p(CSE_OTP_SET_CONFIG, BOOT_PARAMETERS,
				     info.SAFEMEM_BOOT_CONFIG.WORD);

	if (hsm_ret == CSE_NO_ERR)
		ret = OTP_SUCCESS;

	return ret;
}

/*!
 * @brief	otp_Set_Root_Certificate_Hash_Info
 * @details	This routine write the provided customer root certificate hash
 *		information in OTP (ie. the type of hash used and if root
 *		customer certificate is programmed)
 * @param[in]	root_hash_info - 32b unsigned integer with the information
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Set_Customer_Certificate_Hash_Info(uint32_t root_hash_info)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd2p(CSE_OTP_SET_CONFIG,
				     CUSTOMER_CERT_HASH_INFO,
				     (uint32_t)root_hash_info);

	if (hsm_ret == CSE_NO_ERR)
		ret = OTP_SUCCESS;

	return ret;
}

/*!
 * @brief	otp_Set_Root_Certificate_Hash
 * @details	This routine first check if the customer root certificate hash
 *		has been written and enabled
 *		If not, it writes the provided value in the OTP
 * @param[out]	phash - pointer to a byte array containing the value to be
 *		written - must be at least 64 bytes long (512bits)
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE - if customer hash was written already, or errors
 *		while reading OTP
 * @retval	: OTP_SUCCESS
 */
int otp_Set_Customer_Certificate_Hash(uint8_t *phash)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd2p(CSE_OTP_SET_CONFIG,
				     CUSTOMER_CERT_HASH, (uint32_t)phash);

	if (hsm_ret == CSE_NO_ERR)
		ret = OTP_SUCCESS;

	return ret;
}

/*!
 * @brief	otp_write_encrypted_boot_key
 * @details	This routine writes the Encrypted Boot Key and config. bits
 *		in OTP (present and size).
 *		In case the key was already written, an error will be returned
 * @param[in]	bitSize - unsigned integer telling the key size in bits
 *		(only 128 and 256 values are supported)
 * @param[in]	pSecBootKey - pointer to a byte array containing the key to
 *		write (could be 128 or 256 bits)
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE - if Encrypted Boot Key was not written already,
 *		or errors while accessing OTP
 * @retval	: OTP_SUCCESS
 */
int otp_write_encrypted_boot_key(uint32_t bitSize, uint8_t *pSecBootKey)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd2p(CSE_OTP_WRITE_SEC_BOOT_KEY, bitSize,
				     (uint32_t)pSecBootKey);

	if (hsm_ret == CSE_NO_ERR)
		ret = OTP_SUCCESS;

	return ret;
}

/*!
 * @brief	otp_Word_Set_Ecc_Public_Word
 * @details	This routine configure the chunk enclosing the specified word as
 *		ECC protected - only if word_address belongs to the public range
 * @param	: word_addr:	progressive word address
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE - if error while accessing OTP or word_addr not
 *		in the public word range
 * @retval	: OTP_SUCCESS
 */
int otp_Word_Set_Ecc_Public_Word(uint32_t word_addr)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd2p(CSE_OTP_SET_CONFIG, ECC, word_addr);

	if (hsm_ret == CSE_NO_ERR)
		ret = OTP_SUCCESS;

	return ret;
}

/*!
 * @brief	otp_Word_Get_Ecc_Public_Word
 * @details	This routine returns the configuration status of the chunk
 *		enclosing the specified word - only if word_address belongs to
 *		the public range
 * @param	: word_addr:	progressive word address
 * @param	: p_ecc_set:	pointer to unsigned int
 *				(will be set to 1 if word has ECC, 0 if not )
 *				(if word_addr is not valid, p_ecc_set is not
 *				updated)
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE - if error while accessing OTP or word_addr not
 *		in the public word range
 * @retval	: OTP_SUCCESS
 */
int otp_Word_Get_Ecc_Public_Word(uint32_t word_addr, uint32_t *p_ecc_set)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd2p(CSE_OTP_GET_CONFIG, ECC, word_addr);

	if (hsm_ret == CSE_NO_ERR) {
		*p_ecc_set = CSE->P2.R;
		ret = OTP_SUCCESS;
	}

	return ret;
}

/*!
 * @brief	otp_Set_monotonic_counter_selected_type:
 * @details	This routine writes the provided selected type for monotonic
 *		counter (0 -> ram, 1 -> otp, 2 -> NOR SQI, 3 -> eMMC )
 * @param[in]	type - value to write
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int  otp_Set_monotonic_counter_selected_type(t_uint32 type)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd2p(CSE_OTP_SET_CONFIG, MONOTONIC_COUNTER,
				     type);

	if (hsm_ret == CSE_NO_ERR)
		ret = OTP_SUCCESS;

	return ret;
}

/*!
 * @brief	otp_Get_monotonic_counter_selected_type:
 * @details	This routine returns the curent selected type for monotonic
 *		counter (0 -> ram, 1 -> otp, 2 -> NOR SQI, 3 -> eMMC )
 *		If an error occur in the HSM service request, the returned
 *		variable is not updated
 * @param[out]	ptype - pointer to a 32b unsigned integer variable where the
 *		value will be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
int otp_Get_monotonic_counter_selected_type(t_uint32 *ptype)
{
	uint32_t  hsm_ret = CSE_NO_ERR;
	int       ret = OTP_FAILURE;

	hsm_ret = CSE_HAL_send_cmd2p(CSE_OTP_GET_CONFIG, MONOTONIC_COUNTER,
				     (uint32_t)ptype);

	if (hsm_ret == CSE_NO_ERR) {
		*ptype = CSE->P2.R;
		ret = OTP_SUCCESS;
	}

	return ret;
}
#endif

/**
 * @}
 * @}
 */
