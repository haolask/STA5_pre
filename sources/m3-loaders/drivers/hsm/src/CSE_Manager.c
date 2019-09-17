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
 * @file    CSE_manager.c
 * @brief   CSE general management module.
 * @details Set of functions used to check status, version, enable or disable
 *	    features.
 *
 *
 * @addtogroup CSE_driver
 * @{
 * @addtogroup API Functions
 * @{
 */

#include "cse_typedefs.h"
#include "CSE_Constants.h"
#include "CSE_Manager.h"

#include "CSE_HAL.h"

/*============================================================================*/
/**
 * @brief          Reads CSE firmware version.
 * @details        Reads the firmware version - must be called after the
 *		   SECURE_BOOT or the CSE_INIT commands
 *		   This function is a test function which does not provide any
 *		   functional service
 *
 * @param[out]     fw_version pointer to a variable to store firmware revision
 *		   number
 *
 * @return         Error code
 * @retval 0	   When it was possible to read the firmware version
 *		   (ie. last command was CSE_SECURE_BOOT or CSE_INIT)
 * @retval 0xff    When it was not possible to read the firmware version
 *
 * @pre            This function must be called at the very beginning, before
 *		   any useful command has been sent to CSE
 *
 * @implements     only a test & informative function
 *
 * @note           -
 */
uint32_t CSE_ReadCSEFirmwareVersion(uint32_t *fw_version)
{
	/*
	 * The id is only valid if the last command was secure boot,
	 * or cse_init
	 */
	if (CSE->CMD.R == CSE_SECURE_BOOT) {
		*fw_version = CSE->P3.R;
	} else if (CSE->CMD.R == CSE_INIT_CSE) {
		*fw_version = CSE->P1.R;
	} else {
	// Can't get FW version
	return 0xFF;
	}

	return 0;
}

/*============================================================================*/
/**
 * @brief          Get the firmware version ID
 * @details        Provides the firmware version ID value, for CSE-HSM
 *
 * @param[out]     FWId	  pointer to the buffer where UID will be copied
 *
 * @return         Error code
 * @retval 0	   When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_GetFWId(uint32_t *FWId)
{
	uint32_t ret = CSE_NO_ERR;

	CSE->P1.R = 0;

	ret = CSE_HAL_send_cmd(CSE_GET_FWID);

	*FWId = CSE->P1.R;

	return ret;
}

/*============================================================================*/
/**
 * @brief          Get the chip UID ID A brief text description (one line).
 * @details        Provides the chip UID and status, with a MAC generated over
 *		   the provided challenge and the returned ID and status values
 *
 * @param[in]      challenge   pointer to the challenge value (128bits)
 * @param[out]     UID	       pointer to the buffer where UID will be copied
 * @param[out]     status      pointer to the buffer where CSE_SR[24:31] value
 *			       will be copied
 * @param[out]     MAC         pointer to the buffer where MAC will be copied
 *			       (128bits)
 *                             Must be omitted if the function does not have
 *			       parameters.
 *
 * @return         Error code
 * @retval 0       When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_GetId(uint32_t *challenge, uint32_t *UID, uint32_t *status,
		   uint32_t *MAC)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd4p(CSE_GET_ID, (uint32_t)challenge,
				 (uint32_t)UID, 0, (uint32_t)MAC);

	*status = CSE->P3.R;

	return ret;
}

#ifdef HSM_RELEASE
/*============================================================================*/
/**
 * @brief          Initialize the CSE engine
 * @details        Initialize the CSE command processor firmware and data, and
 *		   returns the FW version. Can only be called as the first
 *		   command, when secure boot is not enabled.
 *
 * @param[out]     FW_version  pointer to the buffer where FW version value will
 *		   be copied (32bits)
 *
 * @return         Error code
 * @retval 0       When initialization was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 * @note	   The function must be called before any other command, and
 *		   only when not using secure boot
 */
uint32_t CSE_InitCSE(uint32_t *FW_version)
{
	uint32_t ret = CSE_NO_ERR;

	CSE->P1.R = 0;

	ret = CSE_HAL_send_cmd(CSE_INIT_CSE);

	*FW_version = CSE->P1.R;

	return ret;
}
#endif

/*============================================================================*/
/**
 * @brief          Cancel current command
 * @details
 *
 * @return         Error code
 * @retval 0       When initialization was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 * @note	   The function must be called before any other command, and
 *		   only when not using secure boot
 */
uint32_t CSE_Cancel(void)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd(CSE_CANCEL);

	return ret;
}

#ifndef HSM_RELEASE
/*============================================================================*/
/**
 * @brief          Starts the secure boot process
 * @details        Compute MAC over the Bootloader area and compare it with
 *		   reference BOOT_MAC
 *
 * @param[in]      size        Input sizeof the bootloader (in bytes)
 * @param[in]      start       Address of the beginning of the bootloader
 *			       area
 * @param[out]     FW_version  Pointer to the buffer where FW version value will
 *			       be copied (32bits)
 *
 * @return         Error code
 * @retval 0	   When encryption was performed properly
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_SecureBoot(uint32_t size, uint32_t *start, uint32_t *FW_version)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd2p(CSE_SECURE_BOOT, (uint32_t)size,
				 (uint32_t)start);

	*FW_version = CSE->P3.R;

	return ret;
}

/*============================================================================*/
/**
 * @brief          Finalize the secure boot process disabling the Boot protected
 *		   keys
 * @details        Allow for secure boot process finalization by bootloader
 *		   (ex: check application code)
 *
 * @return         Error code
 * @retval 0	   When command is successful
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_BootFailure(void)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd(CSE_BOOT_FAILURE);

	return ret;
}

/*============================================================================*/
/**
 * @brief          Finalize the secure boot process keeping the Boot protected
 *		   keys, providing the secure boot MAC check was OK
 * @details        Allow for secure boot process finalization by bootloader
 *		   (ex: check application code)
 *
 * @return         Error code
 * @retval 0	   When command is successful
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_BootOK(void)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd(CSE_BOOT_OK);

	return ret;
}
#endif

/*============================================================================*/
/**
 * @brief          Ask CSE for a challenge value to be used to generate the
 *		   debug authorization message
 * @details	   This step is required before calling the CSE_DebugAuth
 *		   command
 *
 * @param[out]     challenge  pointer to the buffer where challenge will be
 *			      written (128bits)

 * @return         Error code
 * @retval 0	   When command is successful
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_DebugChallenge(uint32_t *challenge)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd1p(CSE_DEBUG_CHAL, (uint32_t)challenge);

	return ret;
}

/*============================================================================*/
/**
 * @brief          Provide debug authorization message to CSE
 * @details	   Enable internal debug facility and clears all keys, providing
 *		   none of them is write-protected
 *
 * @param[in]      authorization  pointer to the authorization message computed
 *		   from the challenge provided by CSE_DebugChallenge command
 *
 * @return         Error code
 * @retval 0	   When command is successful
 * @retval 1..21   In case of error - the error code values are the CSE returned
 *		   ones
 *
 * @note           The function is blocking and waits for operation completion
 */
uint32_t CSE_DebugAuth(uint32_t *authorization)
{
	uint32_t ret = CSE_NO_ERR;

	ret = CSE_HAL_send_cmd1p(CSE_DEBUG_AUTH, (uint32_t)authorization);

	return ret;
}

/**
 * @}
 * @}
 */
