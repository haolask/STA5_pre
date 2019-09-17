/*
 * CSE_ext_TLSv12_PRF_TV.h
 *
 *  Created on: 3 avr. 2018
  */

#ifndef SOURCE_CSE_DRIVER_TEST_INC_CSE_EXT_TLSV12_PRF_TV_H_
#define SOURCE_CSE_DRIVER_TEST_INC_CSE_EXT_TLSV12_PRF_TV_H_

#include "cse_types.h"

#define C_NB_TEST_TLSv12_PRF_TV 30

typedef struct
{
    const uint8_t *pSecret;            /* Secret byte array */
    uint32_t secretSize;
    const uint8_t *pLabel;             /* Label byte array */
    uint32_t labelSize;
    const uint8_t *pSeed;              /* Seed byte array */
    uint32_t seedSize;
    const uint8_t *pExpectedPrfOutput; /* Expected PRF output byte array */
    uint32_t expectedPrfOutputSize;
    const uint32_t shaID;              /* SHA algorithm Identifier */
} tlsv12_prf_stt;    /* TLSv1.2 PRF test vectors descriptor */


#endif /* SOURCE_CSE_DRIVER_TEST_INC_CSE_EXT_TLSV12_PRF_TV_H_ */
