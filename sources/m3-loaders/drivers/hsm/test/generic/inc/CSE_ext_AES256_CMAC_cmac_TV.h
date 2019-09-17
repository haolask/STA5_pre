/*#  CAVS 11.0*/
/*#  "CCM-VADT" information*/
/*#  AES CMAC Tag Generation: Key Len: 256*/
/*#  Generated on Tue Mar 15 08:09:25 2011*/

#ifndef _CSE_EXT_AES256_CMAC_H_
#define _CSE_EXT_AES256_CMAC_H_

#include "cse_types.h"

#define C_NB_TEST_CMAC_TVS 12
#define C_NB_TEST_CMAC_TV 8
#define C_NB_TEST_CMAC_VERIFY_TVS 12
#define C_NB_TEST_CMAC_VERIFY_TV 20

#define C_TEST_CMAC_TAG_VERIF_PASS ((uint32_t) 0x50415353)
#define C_TEST_CMAC_TAG_VERIF_FAIL ((uint32_t) 0x4641494C)


typedef struct
{
    const unsigned char* pKey;
    const unsigned char* pMsg;
    const unsigned char* pCmacTag;
} aes256CmacTestVect_stt;

typedef struct
{
    const uint32_t msgSize;                            /* Size of message */
    const uint32_t tagSize;                            /* Size of requested tag */
    aes256CmacTestVect_stt testCmac[C_NB_TEST_CMAC_TV];
} aes256cmac_testVectSerie_stt;

typedef struct
{
    const unsigned char* pKey;
    const unsigned char* pMsg;
    const unsigned char* pCmacTag;
    const uint32_t  * pResult;
} aes256CmacVerifyTestVect_stt;

typedef struct
{
    const uint32_t msgSize;                            /* Size of message */
    const uint32_t tagSize;                            /* Size of requested tag */
    aes256CmacVerifyTestVect_stt verifyTestCmac[C_NB_TEST_CMAC_VERIFY_TV];
} aes256cmac_verifyTestVectSerie_stt;

#endif /*_CSE_EXT_AES256_CMAC_H_ */
