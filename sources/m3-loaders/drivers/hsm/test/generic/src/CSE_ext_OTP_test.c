/*
    CSE driver test - Copyright (C) 2017 STMicroelectronics

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    CSE_ext_OTP_test.c
 * @brief   CSE _ext OTP commands tests
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */


#include <string.h> /* for memcmp */
#include "serialprintf.h"

#include "CSE_Constants.h"
#include "CSE_ext_OTP.h"

#include "err_codes.h"
#include "config.h"

#define OTP_ECC_MODE_CFG_IDX	 	127U

#define PUBLIC_OTP_MAX_IDX 			 71U
#define PUBLIC_OTP_MIN_IDX			 16U


#define SECURITY_LEVEL_TEST_VALUE   1
#define CUSTOMER_ID_TEST_VALUE 0xA5A5

#define CUSTOMER_CERT_HASH_INFO_TEST_VALUE   3
ALIGN const uint8_t ROOT_HASH_TEST_VALUE[64] = { /* SHA256 HASH */
                                          0x11,0xA4,0xB1,0xF2,0xC4,0x6C,0x88,0x1B,0x08,0x53,0x96,0xA7,0x9F,0x58,0x1F,0x4B,
                                          0x11,0xA6,0x01,0x48,0xCF,0x23,0x99,0xD4,0xE6,0x15,0x4B,0x47,0x39,0xAE,0xA2,0x96,
                                          /* another SHA256 HASH - only to fill the area */
                                          0x41,0xB8,0x05,0xEA,0x7A,0xC0,0x14,0xE2,0x35,0x56,0xE9,0x8B,0xB3,0x74,0x70,0x2A,
                                          0x08,0x34,0x42,0x68,0xF9,0x24,0x89,0xA0,0x2F,0x08,0x80,0x84,0x93,0x94,0xA1,0xE4
                                          };

/**
 * @brief          OTP read test
 * @details        try reading OTP area
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *                 sec_boot_key_enabled  (set it to 0 if test has to be ran on a part where secure boot key is not yet programmed - all public words available)
 *                 set it to 1 if secure boot key is programmed --> some words are no longer availble as public words
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
uint32_t otp_read_test( uint32_t verbose, uint32_t sec_boot_key_enabled )
{
    uint32_t pass = 1;
    uint32_t value;
    uint32_t idx;
    int ret_otp;

#ifndef HSM_RELEASE
    enum t_boot_security_level level;
    uint32_t cid;
    uint32_t version;
    struct t_safemem_boot_cfg boot_params;
    ALIGN uint8_t hash[64];
    uint32_t root_hash_info;
#endif

    /* OTP test */
    if(verbose)
    {
        printf("*\n* public OTP read test\n*\n");
	}

    for( idx = 0; idx < 16; idx ++)
    {
        ret_otp = otp_Read_Word( idx, &value );
        /* read attempt MUST FAIL */
        if(ret_otp == OTP_SUCCESS )
		{
			if(verbose)
			{
				printf("OTP [%03d] = 0x%08x - accessible but shouldn't be\n", idx, value );
			}
			pass = 0;
		}
		else
		{
			if(verbose)
			{
				printf("OTP [%03d] = ----------\n", idx);
			}
		}
    }
    if(verbose)
    {
        printf(" pass = %d\n", pass);
    }

	for( idx = 16; idx < 72; idx ++)
    {
        ret_otp = otp_Read_Word( idx, &value );
        if(ret_otp == OTP_SUCCESS )
		{
			if(verbose)
			{
				printf("OTP [%03d] = 0x%08x\n", idx, value );
			}
		}
		else
		{
			if( sec_boot_key_enabled && (idx>=16) && (idx<=23) )
            {
                if(verbose)
                {
                    printf("OTP [%03d] = ----------  Since Secure Boot KEY enabled\n", idx);
                }
            }
            else
            {
                if(verbose)
                {
                    printf("OTP [%03d] = ----------\n", idx);
                }
                pass = 0;
            }
		}
    }

    if(verbose)
    {
        printf(" pass = %d\n", pass);
    }

    for( idx = 72; idx < 130; idx ++)
    {
        ret_otp = otp_Read_Word( idx, &value );
        /* read attempt MUST FAIL */
        if(ret_otp == OTP_SUCCESS )
		{
			if(verbose)
			{
				printf("OTP [%03d] = 0x%08x - accessible but shouldn't be\n", idx, value );
			}
			pass = 0;
		}
		else
		{
			if(verbose)
			{
				printf("OTP [%03d] = ----------\n", idx);
			}
		}
    }

    if(verbose)
    {
        printf(" pass = %d\n", pass);
    }

#ifndef HSM_RELEASE
    /*
     * specific fields
     */
    ret_otp = otp_Get_Security_Level(&level);
    if(ret_otp == OTP_SUCCESS)
    {
        if(verbose)
        {
            printf("Security level : 0x%08x\n", (uint32_t)level );
        }
    }
    else
    {
        if(verbose)
        {
            printf("Security level : ----------\n");
        }
        pass = 0;
    }

    ret_otp = otp_Get_Cust_ID (&cid);
    if(ret_otp == OTP_SUCCESS)
    {
        if(verbose)
        {
            printf("Customer ID : 0x%08x\n", (uint32_t)cid );
        }
    }
    else
    {
        if(verbose)
        {
            printf("Customer ID : ----------\n");
        }
        pass = 0;
    }

    ret_otp = otp_Get_Cust_Cert_Ver (&version);
    if(ret_otp == OTP_SUCCESS)
    {
        if(verbose)
        {
            printf("Customer Cert Ver : 0x%08x\n", (uint32_t)version );
        }
    }
    else
    {
        if(verbose)
        {
            printf("Customer Cert Ver : ----------\n");
        }
        pass = 0;
    }

    ret_otp = otp_Get_Secr_Ver(&version);
    if(ret_otp == OTP_SUCCESS)
    {
        if(verbose)
        {
            printf("Security version : 0x%08x\n", (uint32_t)version );
        }
    }
    else
    {
        if(verbose)
        {
            printf("Security version : ----------\n");
        }
        pass = 0;
    }

    ret_otp = otp_Get_Boot_Parameters(&boot_params);
    if(ret_otp == OTP_SUCCESS)
    {
        if(verbose)
        {
            printf("Boot params : 0x%08x\n", boot_params.SAFEMEM_BOOT_CONFIG.WORD);
        }
    }
    else
    {
        if(verbose)
        {
            printf("Boot params : ----------\n");
        }
        pass = 0;
    }

    ret_otp = otp_Get_Customer_Certificate_Hash_Info(&root_hash_info);
    if(ret_otp == OTP_SUCCESS)
    {
        if(verbose)
        {
            printf("Customer certificate hash info : 0x%08x\n", root_hash_info);
        }
    }
    else
    {
        if(verbose)
        {
            printf("Customer certificate hash info : ----------\n");
        }
        pass = 0;
    }
    if(verbose)
    {
        printf(" pass = %d\n", pass);
    }

    ret_otp = otp_Get_Customer_Certificate_Hash(hash);
    if(ret_otp == OTP_SUCCESS)
    {
        if(verbose)
        {
            printf("Hash : \n");
            display_buf("", hash, 64);
        }
    }
    else
    {
        if(verbose)
        {
            printf("Hash : ----------\n");
            if( root_hash_info != 0)
            {
                printf("Should have been accessible\n");
            }
        }
        if( root_hash_info != 0)
        {
            pass = 0;
        }
    }
#endif /* HSM_RELEASE */
	return(pass);
}

#ifndef HSM_RELEASE
uint32_t  otp_write_encrypted_boot_key_test( uint32_t verbose )
{
	uint32_t ret = 0;
	uint32_t pass = 1;
	uint32_t bitSize;
	uint8_t SecBootKey[32] = { 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
                               0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};

    if(verbose)
    {
        printf(" otp write secure boot key test\n");
    }

    bitSize = 256;

    ret = otp_write_encrypted_boot_key( bitSize, SecBootKey);

    if(ret != OTP_SUCCESS)
    {
        pass = 0;
    }
    return(pass);
}
#endif /* HSM_RELEASE */


/**
 * @brief          OTP write test
 * @details        try writing - and reading back OTP area
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
uint32_t otp_write_test( uint32_t verbose, uint32_t sec_boot_key_enabled )
{
    uint32_t pass = 1;
    uint32_t value;
    uint32_t idx;
    int ret_otp;
    uint32_t val_to_write;

#ifndef HSM_RELEASE
    enum t_boot_security_level level;
    uint32_t cid;
    uint32_t version;
    struct t_safemem_boot_cfg boot_params;
    ALIGN uint8_t new_root_hash[64];
    uint32_t root_hash_info;
    uint32_t new_version;
    struct t_safemem_boot_cfg new_boot_params;
#endif

    /*
     * OTP test
     */
    if(verbose)
    {
        printf("*\n* public OTP write test\n*\n");
	}

	for( idx = 16; idx < 72; idx ++)
    {
        val_to_write = (idx) * 256 + idx;
        ret_otp = otp_Write_Word( idx, val_to_write );
        if(ret_otp == OTP_SUCCESS )
		{
            ret_otp = otp_Read_Word( idx, &value );
            if(ret_otp == OTP_SUCCESS)
            {
                if( val_to_write == value)
                {
                    if(verbose)
                    {
                        printf(" OTP [%03d] value is the written one : 0x%08x\n", idx, (uint32_t)value );
                    }
                }
                else
                {
                    if(verbose)
                    {
                        printf(" OTP [%03d] value is not the expected one : 0x%08x (0x%08x)\n", idx, (uint32_t)value, val_to_write );
                    }
                    pass = 0;
                }
            }
            else
            {
                if(verbose)
                {
                    printf(" Can't read back OTP [%03d] value\n", idx);
                }
                pass = 0;
            }
		}
		else
		{
			if( sec_boot_key_enabled && (idx>=16) && (idx<=23) )
            {
                if(verbose)
                {
                    printf(" Can't write OTP [%03d] value Since Secure Boot KEY enabled\n", idx);
                }
            }
            else
            {
                if(verbose)
                {
                    printf(" Can't write OTP [%03d] value \n", idx);
                }
                pass = 0;
			}
		}
    }
    if(verbose)
    {
        printf(" pass = %d\n", pass);
    }

#ifndef HSM_RELEASE
    /* specific fields */
    /*
     * Security Level
     */
    if(verbose)
    {
        printf("*\n* Security level write test\n*\n");
	}
    ret_otp = otp_Set_Security_Level ( SECURITY_LEVEL_TEST_VALUE );
    if(ret_otp == OTP_SUCCESS)
    {
        ret_otp = otp_Get_Security_Level(&level);
        if(ret_otp == OTP_SUCCESS)
        {
            if( SECURITY_LEVEL_TEST_VALUE == level)
            {
                if(verbose)
                {
                    printf(" Security level is the expected one  : 0x%08x\n", (uint32_t)level );
                }
            }
            else
            {
                if(verbose)
                {
                    printf(" Security level is not the expected one : 0x%08x (0x%08x)\n", (uint32_t)level, SECURITY_LEVEL_TEST_VALUE );
                }
                pass = 0;
            }
        }
        else
        {
            if(verbose)
            {
                printf(" Can't read back Security level value\n");
            }
            pass = 0;
        }
    }
    else
    {
        if(verbose)
        {
            printf(" Can't write Security level value\n");
        }
        pass = 0;
    }
    if(verbose)
    {
        printf(" pass = %d\n", pass);
    }

    /*
     * Customer ID
     */
    if(verbose)
    {
        printf("*\n* Customer ID write test\n*\n");
	}
    ret_otp = otp_Set_Cust_ID ( CUSTOMER_ID_TEST_VALUE );
    if(ret_otp == OTP_SUCCESS)
    {
        ret_otp = otp_Get_Cust_ID ( &cid );
        if(ret_otp == OTP_SUCCESS)
        {
            if( CUSTOMER_ID_TEST_VALUE == cid)
            {
                if(verbose)
                {
                    printf(" Customer ID is the expected one     : 0x%08x\n", (uint32_t)cid );
                }
            }
            else
            {
                if(verbose)
                {
                    printf(" Customer ID is not the expected one : 0x%08x (0x%08x)\n", (uint32_t)cid, CUSTOMER_ID_TEST_VALUE );
                }
                pass = 0;
            }
        }
        else
        {
            if(verbose)
            {
                printf(" Can't read back customer ID value\n");
            }
            pass = 0;
        }
    }
    else
    {
        if(verbose)
        {
            printf(" Can't write customer ID value\n");
        }
        pass = 0;
    }


    if(verbose)
    {
        printf("*\n* Customer cert version write test\n*\n");
	}
    ret_otp = otp_Get_Cust_Cert_Ver (&version);
    if(ret_otp == OTP_SUCCESS)
    {
        if(verbose)
        {
            printf(" Customer cert ver : 0x%08x\n", (uint32_t)version );
        }
        ret_otp = otp_Incr_Cust_Cert_Ver();
        if(ret_otp == OTP_SUCCESS)
        {
            ret_otp = otp_Get_Cust_Cert_Ver (&new_version);
            if(ret_otp == OTP_SUCCESS)
            {
                if( new_version == version + 1 )
                {
                    if(verbose)
                    {
                        printf(" Customer cert ver was incremented : 0x%08x\n", (uint32_t)new_version );
                    }
                }
                else
                {
                    if(verbose)
                    {
                        printf(" Customer cert ver was not incremented : 0x%08x\n", (uint32_t)new_version );
                    }
                    pass = 0;
                }
            }
            else
            {
                if(verbose)
                {
                    printf(" Can't read customer Cert ver after increment\n");
                }
                pass = 0;
            }
        }
        else
        {
            if(verbose)
            {
                printf(" Can't increment customer Cert ver value\n");
            }
            pass = 0;
        }
    }
    else
    {
        if(verbose)
        {
            printf(" Customer cert ver : ----------\n");
        }
        pass = 0;
    }

    if(verbose)
    {
        printf("*\n* Security version write test\n*\n");
	}
    ret_otp = otp_Get_Secr_Ver (&version);
    if(ret_otp == OTP_SUCCESS)
    {
        if(verbose)
        {
            printf(" Security ver : 0x%08x\n", (uint32_t)version );
        }
        ret_otp = otp_Incr_Secr_Ver();
        if(ret_otp == OTP_SUCCESS)
        {
            ret_otp = otp_Get_Secr_Ver (&new_version);
            if(ret_otp == OTP_SUCCESS)
            {
                if( new_version == version + 1 )
                {
                    if(verbose)
                    {
                        printf(" Security ver was incremented : 0x%08x\n", (uint32_t)new_version );
                    }
                }
                else
                {
                    if(verbose)
                    {
                        printf(" Security ver was not incremented : 0x%08x\n", (uint32_t)new_version );
                    }
                    pass = 0;
                }
            }
            else
            {
                if(verbose)
                {
                    printf(" Can't read Security ver after increment\n");
                }
                pass = 0;
            }
        }
        else
        {
            if(verbose)
            {
                printf(" Can't increment Security ver value\n");
            }
            pass = 0;
        }
    }
    else
    {
        if(verbose)
        {
            printf(" Security ver : ----------\n");
        }
        pass = 0;
    }

    if(verbose)
    {
        printf("*\n* Boot parameters write test\n*\n");
	}
    ret_otp = otp_Get_Boot_Parameters(&boot_params);
    if(ret_otp == OTP_SUCCESS)
    {
        if(verbose)
        {
            printf("Initial Boot params : 0x%08x\n", boot_params.SAFEMEM_BOOT_CONFIG.WORD);
        }

        boot_params.SAFEMEM_BOOT_CONFIG.BIT.BOOT_SAFEMEM_BYPASS = 0;
        boot_params.SAFEMEM_BOOT_CONFIG.BIT.BOOT_BYPASS_SEQUENCE = 0;
        boot_params.SAFEMEM_BOOT_CONFIG.BIT.BOOT_PERIPHERAL = 0;
        boot_params.SAFEMEM_BOOT_CONFIG.BIT.BOOT_PERIPHERAL_OPTIONS.NAND_BOOT_OPTION.NAND_BUS_WIDTH = 1;
        boot_params.SAFEMEM_BOOT_CONFIG.BIT.BOOT_PERIPHERAL_OPTIONS.NAND_BOOT_OPTION.NAND_PAGE_TYPE = 0;
        boot_params.SAFEMEM_BOOT_CONFIG.BIT.BOOT_PERIPHERAL_OPTIONS.NAND_BOOT_OPTION.NAND_GPIO_CONFIG = 1;

        /* boot parameters is the lower half word of the structure */
        ret_otp = otp_Set_Boot_Parameters(boot_params);
        if(ret_otp == OTP_SUCCESS)
        {
            ret_otp = otp_Get_Boot_Parameters(&new_boot_params);
            if(ret_otp == OTP_SUCCESS)
            {
                if( (new_boot_params.SAFEMEM_BOOT_CONFIG.WORD&0xFFFF) == (boot_params.SAFEMEM_BOOT_CONFIG.WORD & 0xFFFF) )
                {
                    if(verbose)
                    {
                        printf(" Boot parameters written properly : 0x%08x\n", (uint32_t)new_boot_params.SAFEMEM_BOOT_CONFIG.WORD );
                    }
                }
                else
                {
                    if(verbose)
                    {
                        printf(" Boot parameters was not written: 0x%08x\n", (uint32_t)new_boot_params.SAFEMEM_BOOT_CONFIG.WORD );
                    }
                    pass = 0;
                }
            }
            else
            {
                if(verbose)
                {
                    printf(" Can't read Boot parameters after writing\n");
                }
                pass = 0;
            }
        }
        else
        {
            if(verbose)
            {
                printf(" Can't write Boot parameters value\n");
            }
            pass = 0;
        }
    }
    else
    {
        if(verbose)
        {
            printf(" Boot parameters : ----------\n");
        }
        pass = 0;
    }
    if(verbose)
    {
        printf(" pass = %d\n", pass);
    }

    /*
     * Customer certificate hash value
     */
    if(verbose)
    {
        printf("*\n* Customer root certificate hash value write test\n*\n");
	}
    ret_otp = otp_Set_Customer_Certificate_Hash((uint8_t*)ROOT_HASH_TEST_VALUE);
    if(ret_otp == OTP_SUCCESS)
    {
        ret_otp = otp_Get_Customer_Certificate_Hash(new_root_hash);
        if(ret_otp == OTP_SUCCESS)
        {
            if( memcmp(ROOT_HASH_TEST_VALUE, new_root_hash, 64) == 0)
            {
                if(verbose)
                {
                    printf(" Customer certificate hash written as expected  : \n");
                    display_buf("", new_root_hash, 64);
                }
            }
            else
            {
                if(verbose)
                {
                    printf(" Customer certificate hash is not the expected one : \n");
                    display_buf("  expected : ", (uint8_t*)ROOT_HASH_TEST_VALUE, 64);
                    display_buf("  read     : ", new_root_hash, 64);
                }
                pass = 0;
            }
        }
        else
        {
            if(verbose)
            {
                printf(" Can't read back Customer certificate hash value\n");
            }
            pass = 0;
        }
    }
    else
    {
        if(verbose)
        {
            printf(" Can't write Customer certificate hash value\n");
        }
        pass = 0;
    }
    if(verbose)
    {
        printf(" pass = %d\n", pass);
    }

    /*
     * Root hash info
     */
    if(verbose)
    {
        printf("*\n* Customer certificate root hash info write test\n*\n");
	}
    ret_otp = otp_Set_Customer_Certificate_Hash_Info(CUSTOMER_CERT_HASH_INFO_TEST_VALUE);
    if(ret_otp == OTP_SUCCESS)
    {
        ret_otp = otp_Get_Customer_Certificate_Hash_Info(&root_hash_info);
        if(ret_otp == OTP_SUCCESS)
        {
            if( CUSTOMER_CERT_HASH_INFO_TEST_VALUE == root_hash_info)
            {
                if(verbose)
                {
                    printf(" Customer certificate hash info written as expected  : 0x%08x\n", (uint32_t)root_hash_info );
                }
            }
            else
            {
                if(verbose)
                {
                    printf(" Customer certificate hash info is not the expected one : 0x%08x (0x%08x)\n", (uint32_t)root_hash_info, CUSTOMER_CERT_HASH_INFO_TEST_VALUE );
                }
                pass = 0;
            }
        }
        else
        {
            if(verbose)
            {
                printf(" Can't read back Customer certificate hash info value\n");
            }
            pass = 0;
        }
    }
    else
    {
        if(verbose)
        {
            printf(" Can't write Customer certificate hash info value\n");
        }
        pass = 0;
    }
    if(verbose)
    {
        printf(" pass = %d\n", pass);
    }

    /*
     * ECC configuration
     */
    {
        uint32_t has_ecc;

        if(verbose)
        {
            printf("*\n* public OTP ECC configuration test \n*\n");
        }

        for( idx = 16; idx < 72; idx ++)
        {
            ret_otp = otp_Word_Set_Ecc_Public_Word( idx );
            if(ret_otp == OTP_SUCCESS )
            {
                ret_otp = otp_Word_Get_Ecc_Public_Word( idx, &has_ecc);
                if(ret_otp == OTP_SUCCESS)
                {
                    if( 1 == has_ecc)
                    {
                        if(verbose)
                        {
                            printf(" OTP [%03d] word is configured as ECC\n", idx);
                        }
                    }
                    else
                    {
                        if(verbose)
                        {
                            printf(" OTP [%03d] word was not configured properly as ECC\n", idx);
                        }
                        pass = 0;
                    }
                }
                else
                {
                    if( sec_boot_key_enabled && (idx>=16) && (idx<=23) )
                    {
                        if(verbose)
                        {
                            printf(" Can't get OTP [%03d] word ECC status  Since Secure Boot KEY enabled\n", idx);
                        }
                    }
                    else
                    {
                        if(verbose)
                        {
                            printf(" Can't get OTP [%03d] word ECC status\n", idx);
                        }
                        pass = 0;
                    }
                }
            }
            else
            {
                if( sec_boot_key_enabled && (idx>=16) && (idx<=23) )
                {
                    if(verbose)
                    {
                        printf(" Can't configure OTP [%03d] word as ECC  Since Secure Boot KEY enabled\n", idx);
                    }
                }
                else
                {
                    if(verbose)
                    {
                        printf(" Can't configure OTP [%03d] word as ECC\n", idx);
                    }
                    pass = 0;
                }
            }
        }
    }
    if(verbose)
    {
        printf(" pass = %d\n", pass);
    }
#endif /* HSM_RELEASE */

    return(pass);
}

/**
 * @}
 */
