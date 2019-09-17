/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2017, STMicroelectronics International N.V.
 * All rights reserved.
 */

#ifndef __PTA_HSM_H
#define __PTA_HSM_H

#define PTA_HSM_UUID \
		{ 0xa18f50d6, 0x80e6, 0x4cfe, \
			{ 0xb7, 0x66, 0x7f, 0x51, 0x8f, 0x31, 0x76, 0xe3 } }


/*
 * Get Firmware version: share the eHSM Firmware ID.
 *
 * [out]    value            Firmware ID
 */
#define PTA_HSM_CMD_GET_FW_ID		1

/*
 * TLS PRF function: derive a secret with variable lengths depending on the
 * step of the TLS protocol used.
 *
 * [in]     memref[0]        secret descriptor
 * [in]     memref[1]        label descriptor
 * [in]     memref[2]        seed descriptor
 * [in/out] memref[3]        in : 1st word of buffer initialized with size
 *                                reserved to buffer receiving the derived
 *                                secret
 *                                2nd word of buffer set to hash algorithm
 *                                identifier
 *                           out: TLS output descriptor
 */
#define PTA_HSM_CMD_TLSV12_PRF		2

/*
 * ECC regenerate NVM key service: regenerate a new ECC key pair with security
 * control (SHE compliant) and store it inside an non-empty ECC NVM slot.
 *
 * [in]     memref[0]        M1 message
 * [in]     memref[1]        M2 message
 * [in]     memref[2]        M3 message
 * [in]     value            Elliptic curve identifier
 */
#define PTA_HSM_CMD_ECC_KEY_REGEN	3

/*
 * Key storage initialization: trigger the external key image loading by the
 * eHSM.
 *
 * [in]     value            Address exchanging Key Storage requests
 */
#define PTA_HSM_CMD_KS_INIT		4

/*
 * Export of ECC Public key: export outside eHSM the public key part of the ECC
 * key pair identified by the index provided.
 *
 * [in]     value            Index of ECC key for which the public key part is
 *			     exported
 * [out]    memref[0]        Public key x coordinate
 * [out]    memref[1]        Public key y coordinate
 */
#define PTA_HSM_CMD_PUB_KEY_EXP		5

/*
 * Load ECC RAM key: load ECC key pair values provided in clear text in the
 * HSM ECC RAM key slot.
 *
 * [in]     memref[0]        Public key x coordinate
 * [in]     memref[1]        Public key y coordinate
 * [in]     memref[2]        Private key d coordinate
 * [in]     value	     Elliptic curve identifier
 */
#define PTA_HSM_CMD_LOAD_ECC_RAM_KEY	6

/*
 * ECC Scalar Multiplication and Addition service: compute
 *   ((d * r) mod n + e) mod n
 *   with :
 *     - d being an ECC private key
 *     - r being a scalar multiplier
 *     - e being a scalar addend
 *     - n being the order of the generator point
 *
 * [in]     memref[0]        Pointer to seed key pair descriptor
 *                           - memref buffer is filled with fields concatenated
 *                           in the respective order:
 *                           curve id, public key_x, public key_y and private key_d
 *                           - memref size is set to the ECC key size
 *                           multiplied by 3 + curve id size (in bytes)
 * [in]     memref[1]        pointer to multiplier descriptor
 * [in]     memref[2]        Pointer to addend descriptor
 * [out]    value            Key index of the calculated key
 */
#define PTA_HSM_CMD_ECC_SCALAR_MUL_ADD	8

/*
 * Export of ECC key pair in protected form: export outside eHSM the ECC public
 * and private key in protected form.
 *
 * [in/out] memref[0]        Pointer to response message M1 (out)
 *                           - in: index of the key to export is provided in
 *                           the 1st location of the memref buffer
 * [in/out] memref[1]        Pointer to response message M2 (out)
 *                           - in: size of the memory allocated for the M2
 *                           message is provided in the 1st location of the
 *                           memref buffer
 * [out]    memref[2]        Pointer to response message M3
 * [out]    memref[3]        Pointer to response message M4 & M5
 *                           - memref buffer is filled with M4 and M5 messages
 *                           concatenated in the respective order
 *                           - memref size is set to the M4 message size (32
 *                           bytes) + M5 message size (16 bytes)
 */
#define PTA_HSM_CMD_EXP_ECC_KEY_PROT	9

/*
 * Load of ECC key pair in protected form: load into eHSM the ECC public
 * and private key in protected form in RAM or NVM locations.
 *
 * [in]     memref[0]        Pointer to response message M1
 * [in]     memref[1]        Pointer to response message M2
 * [in]     memref[2]        Pointer to response message M3
 * [in/out] memref[3]        Pointer to public key descriptor (out)
 *                           - out: memref buffer is filled with the public,
 *                                  key x and y (in the respective order) of
 *                                  the loaded ECC key pair.
 *                                  memref size is set to the ECC key size
 *                                  multiplied by 2 (in bytes)
 *                           - in:  memref buffer is filled with M4 and M5
 *                                  messages concatenated in the respective
 *                                  order.
 *                                  Only use for key loading checking.
 */
#define PTA_HSM_CMD_LOAD_ECC_KEY_PROT	10

#endif /*__PTA_HSM_H*/

