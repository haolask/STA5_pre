/*
 * CSE_ext_ECC_ECDH_SHORT_TV.h
 *
 *  Created on: 27 mars 2018
 */

#ifndef _CSE_EXT_ECC_ECDH_UT_TV_H_
#define _CSE_EXT_ECC_ECDH_UT_TV_H_

#include "cse_typedefs.h"

#define C_NB_TEST_ECDH_SHORT_TV 10
#define C_NB_TEST_ECDH_TV 10
#define C_NB_TEST_ECDH_KEY_PAIR_TV 100

typedef struct
{
    const uint8_t *pEcdhPrivateKey;               /* Private Key byte array */
    const uint8_t *pEcdhPublicKeyX;               /* Public Key X byte array */
    const uint8_t *pEcdhPublicKeyY;               /* Public Key Y byte array */
    const uint8_t *pEcdhExpectedOutput;           /* Expected output byte array */
    const uint32_t ecdhCurveID;                   /* Elliptic Curve Identifier */
} ecc_ecdh_testVector_stt;    /* ECDH test test vectors */

typedef struct
{
    const uint8_t *pEcdhPrivateKey;               /* Private Key byte array */
    uint32_t privKeySize;
    const uint8_t *pEcdhPublicKeyX;               /* Public Key X byte array */
    uint32_t pubKeyXSize;
    const uint8_t *pEcdhPublicKeyY;               /* Public Key Y byte array */
    uint32_t pubKeyYSize;
    const uint8_t *pEcdhExpectedOutput;           /* Expected output byte array */
    uint32_t outputSize;
    const uint32_t ecdhCurveID;                   /* Elliptic Curve Identifier */
} ecc_ecdh_keyPair_testVector_stt;    /* ECDH test test vectors */

#endif /* _CSE_EXT_ECC_ECDH_UT_TV_H_ */
