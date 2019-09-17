/*#  CAVS 11.0*/
/*#  "CCM-VADT" information*/
/*#  AES Keylen: 256*/
/*#  Generated on Tue Mar 15 08:09:25 2011*/

#ifndef _CSE_EXT_AES256_CMAC_DVPT_H_
#define _CSE_EXT_AES256_CMAC_DVPT_H_

#include "cse_types.h"

#define C_TEST_CCM_DVPT_PASS ((uint32_t) 0x50415353)
#define C_TEST_CCM_DVPT_FAIL ((uint32_t) 0x4641494C)

#define C_NB_TEST_CCM_DVPT_TV 15
#define C_NB_TEST_CCM_DVPT_TVS 16


typedef struct
{
    const unsigned char* pNonce;
    const unsigned char* pAdata;
    const unsigned char* pCT;
    const uint32_t * pResult;
    const unsigned char* pPayload;
} aes256CcmDvptTest_vect_stt;

typedef struct
{
    const uint32_t AlenSize;                        /* Size of Associated Data */
    const uint32_t PlenSize;                        /* Size of Payload Data */
    const uint32_t NlenSize;                        /* Size of Nonce */
    const uint32_t TlenSize;                        /* Size of Tag */
    const unsigned char* aes256ccm_Key_Alen;        /* AES256 Key */
    const aes256CcmDvptTest_vect_stt testCcmDvpt[C_NB_TEST_CCM_DVPT_TV];
} aes256ccm_dvpt_test_vect_stt;

#endif /* _CSE_EXT_AES256_CMAC_DVPT_H_ */
