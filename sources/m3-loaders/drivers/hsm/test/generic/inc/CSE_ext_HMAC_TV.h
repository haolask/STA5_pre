/*
* CSE_ext_HMAC_TV.h
*
*  Created on: 26 juil. 2017
*      Author: pierre guillemin
*/

/* #  CAVS 11.0
#  HMAC information
#  Hash sizes tested: 20 28 32 48 64
#  Generated on Mon Feb 28 20:38:43 2011
*/
#ifndef _CSE_EXT_HMAC_VADT_H_
#define _CSE_EXT_HMAC_VADT_H_

#include "cse_types.h"
#include "config.h"

/* Structure for HMAC tests */
typedef struct
{
    const ALIGN uint8_t key[145];
    const ALIGN uint8_t msg[128];
    const ALIGN uint8_t tag[64];
} hmacTestVectorData_stt;

typedef struct
{
    const uint32_t keySize;
    const uint32_t tagSize;
    hmacTestVectorData_stt hmac_TV;
} hmac_testVect_stt;

#define C_TV_MESSAGE_SIZE 128

#ifdef FULL_HMAC_TV_SET
#define C_NB_TV_HMAC_SHA1_RESTRICT 75
#define C_NB_TV_HMAC_SHA224_RESTRICT 82
#define C_NB_TV_HMAC_SHA256_RESTRICT 30
#define C_NB_TV_HMAC_SHA384_RESTRICT 82
#define C_NB_TV_HMAC_SHA512_RESTRICT 82

#define C_NB_HMAC_SHA1_RESTRICT 45
#define C_NB_HMAC_SHA224_RESTRICT 45
#define C_NB_HMAC_SHA256_RESTRICT 30
#define C_NB_HMAC_SHA384_RESTRICT 82
#define C_NB_HMAC_SHA512_RESTRICT 82

#define C_NB_HMAC_SHA1_RESTRICT_KEY 180
#define C_NB_HMAC_SHA224_RESTRICT_KEY 225
#define C_NB_HMAC_SHA256_RESTRICT_KEY 135
#define C_NB_HMAC_SHA384_RESTRICT_KEY 180
#define C_NB_HMAC_SHA512_RESTRICT_KEY 225

#define C_NB_TV_HMAC_SHA1 300
#define C_NB_TV_HMAC_SHA224 375
#define C_NB_TV_HMAC_SHA256 225
#define C_NB_TV_HMAC_SHA384 300
#define C_NB_TV_HMAC_SHA512 375

#else

#define C_NB_TV_HMAC_SHA1_RESTRICT 5
#define C_NB_TV_HMAC_SHA224_RESTRICT 5
#define C_NB_TV_HMAC_SHA256_RESTRICT 5
#define C_NB_TV_HMAC_SHA384_RESTRICT 5
#define C_NB_TV_HMAC_SHA512_RESTRICT 5

#define C_NB_HMAC_SHA1_RESTRICT 5
#define C_NB_HMAC_SHA224_RESTRICT 5
#define C_NB_HMAC_SHA256_RESTRICT 5
#define C_NB_HMAC_SHA384_RESTRICT 5
#define C_NB_HMAC_SHA512_RESTRICT 5

#define C_NB_HMAC_SHA1_RESTRICT_KEY 5
#define C_NB_HMAC_SHA224_RESTRICT_KEY 5
#define C_NB_HMAC_SHA256_RESTRICT_KEY 5
#define C_NB_HMAC_SHA384_RESTRICT_KEY 5
#define C_NB_HMAC_SHA512_RESTRICT_KEY 5

#define C_NB_TV_HMAC_SHA1 5
#define C_NB_TV_HMAC_SHA224 5
#define C_NB_TV_HMAC_SHA256 5
#define C_NB_TV_HMAC_SHA384 5
#define C_NB_TV_HMAC_SHA512 5

#endif
#endif /* _CSE_EXT_HMAC_VADT_H_ */
