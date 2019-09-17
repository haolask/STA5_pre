/*#  CAVS 11.0*/
/*#  "CCM-VADT" information*/
/*#  AES Keylen: 256*/
/*#  Generated on Tue Mar 15 08:09:25 2011*/

#ifndef _CSE_EXT_AES256_CMAC_VADT_H_
#define _CSE_EXT_AES256_CMAC_VADT_H_

#include "cse_types.h"

/* 
Plen = 24
Nlen = 13
Tlen = 16
*/

#define C_PLEN_VADT_SIZE 24
#define C_NLEN_VADT_SIZE 13
#define C_TLEN_VADT_SIZE 16

#define C_NB_TEST_CCM_VADT 33
#define C_NB_TEST_CCM 10

typedef struct
{
    const unsigned char* pAdata;
    const unsigned char* pPayload;
    const unsigned char* pCT;
} aes256CcmVdadtTest_vect_stt;

typedef struct
{
    const uint32_t AlenSize;                        /* Size of Associated Data */
    const unsigned char* aes256ccm_Key_Alen;        /* AES256 Key */
    const unsigned char* aes256ccm_Nonce_Alen;      /* Nonce */
    aes256CcmVdadtTest_vect_stt testCcmVadt[10];
} aes256ccm_vadt_test_vect_stt;

#endif /* _CSE_EXT_AES256_CMAC_VADT_H_ */
