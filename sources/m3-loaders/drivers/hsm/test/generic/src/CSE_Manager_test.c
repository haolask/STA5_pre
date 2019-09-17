/*
    SPC5-CRYPTO - Copyright (C) 2014 STMicroelectronics

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
 * @file    CSE_Manager_test.c
 * @brief   CSE Manager commands tests
 * @details
 *
 *
 * @addtogroup CSE_driver_test
 * @{
 */


#include <string.h> /* for memcmp */
#include "serialprintf.h"
#include "serial_input.h"

#include "CSE_HAL.h"
#include "CSE_AES_HW_test.h"
#include "CSE_ext_manager.h"
#include "CSE_Manager_test.h"

/* General commands */
#include "CSE_Constants.h"
#include "CSE_Manager.h"

/* AES operations & keys dedicated CSE commands */
#include "CSE_AES_HW_Modes.h"
#include "CSE_Key.h"

/* TRNG dedicated CSE commands */
#include "CSE_RNG.h"

volatile struct CSE_HSM_tag* A7_she_reg_addr = (volatile struct CSE_HSM_tag*)&SHE_REGS1_REMOTE;


/**
 * @brief          Chip UID test
 * @details        Asks for the chip UID and reports the value
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
int CSE_GetID_test(int verbose)
{
	uint32_t challenge[4] = { 0x00000000, 0x00000001, 0x00000002, 0x00000003 };
	uint32_t UID[4]  = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
	uint32_t MAC[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
	uint32_t status = 0;
	uint32_t ret;
	int i;

	if(verbose)
	{
		printf("\n");
		printf("Get ID \n");
	}

	/* Clear buffers */
	for(i=0;i<4;i++)
	{
		UID[i] = 0x00000000;
		MAC[i] = 0x00000000;
	}
	status = 0;

	ret = CSE_GetId( challenge, (uint32_t*)UID, (uint32_t*)&status, (uint32_t*)MAC);

	if(verbose)
	{
		printf("ERC : %x\n", ret );
		display_buf("challenge ", (uint8_t*)challenge, 16 );
		display_buf("UID       ", (uint8_t*)UID, 16 );
		display_buf("MAC       ", (uint8_t*)MAC, 16 );
		printf(     "status    %x", status);
	}
	else
	{
		display_buf("UID       ", (uint8_t*)UID, 16 );
	}
	printf("\n");
	return( ret == CSE_NO_ERR);
}

/**
 * @brief          Firmware ID test
 * @details        Asks for the firmware version ID and reports the value
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
int CSE_Get_Firmware_ID_test( int verbose )
{
   	uint32_t firmware_id = 0;
   	uint32_t ret;
   	if(verbose)
   	{
		printf("\n");
		printf("Get FW ID \n");
   	}
   	/* Clear buffers */
   	firmware_id = 0;
   	ret = CSE_GetFWId( &firmware_id );
   	if(verbose)
   	{
   		printf("ret : %x\n", ret );
		printf("firmware ID : %x", firmware_id);
		printf("\n");
   	}
   	return(ret == CSE_NO_ERR);
}

/**
 * @brief          CSE init test
 * @details        Calls CSE_init command and display returned error code and status register
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 *
 */
void CSE_init_test( uint32_t verbose )
{
	uint32_t firmware_id = 0;
	uint32_t ret;
	if(verbose)
	{
		printf("\n");
		printf("CSE_InitCSE\n");
	}

	/* Clear buffers */
	firmware_id = 0;
	ret = CSE_InitCSE( &firmware_id );
	if(verbose)
	{
		printf("ret : %x\n", ret );
		printf("firmware ID : %x", firmware_id);
		printf("\n");
	}
}

/**
 * @brief          Cancel while alreay execution a service request test
 * @details        Calls CANCEL command during on-going crypto operation then try again to ask for crypto services
 *
 * @param[in]      verbose		enable display of input, computed and expected values when set to 1
 * @return         Error code
 * @retval 0  	   When test failed
 * @retval 1   	   When test succeeded
 *
 */
uint32_t cancel_while_mac_test( uint32_t verbose )
{
	/* Need to access specific internal variables & function of the SPC5x HAL for notification */
	extern uint32_t answer_available;
	extern void CSE_HSM_notify_new_req( void );

	uint64_t long_MAC_payload_size = 16000000; //2MBytes
	uint32_t AES_MAC[4] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
	uint32_t ret;
	uint32_t pass = 1;
	uint32_t pass2 = 1;

	if(verbose)
	{
		printf("Test Cancel command behavior \n");
		printf("Trig a MAC computation over 2Mbytes - first waiting for answer (let the operation finish properly)\n");
	}

	CSE->P1.R = CSE_RAM_KEY;
	CSE->P2.R = (uint32_t)(&long_MAC_payload_size);
	CSE->P3.R = (uint32_t)0x09000000; // Large Flash Blocks (both for McK2, Eiger1.1 and Eiger 2)
	CSE->P4.R = (uint32_t)AES_MAC;
	CSE->CMD.R = CSE_GENERATE_MAC;
	answer_available = 0;

	/*
	 * This is enough to run command with B3M CSE HW engine
	 * But we need an additional notification for the CSE running on HSM
	 */
	notify_new_request_available();

	/* Wait at least for cmd Ack */
	// Should add some timeout, in case HSM is not working
	/* wait for request to be handled (HSM will clear bit when request will be read ) */
	while( new_request_available() )
	{
	}

	/* Now wait for command completion */
	//CSE_HSM_wait_for_answer();
	//while (CSE->SR.B.BSY == 1) { } /*wait until CSE is idle*/

   	/* wait until we're notified by the ISR */
   	while (answer_available == 0) {}

   	/* Clear notification variable */
   	answer_available = 0;

   	/* Test error code */
   	ret = CSE->ECR.R;
   	if( ret != CSE_NO_ERR)
   	{
   		if(verbose)
   		{
   			printf(" error code AES MAC generate over 2 MB: %02x\n", ret );
   		}
   		pass = 0;
   	}
   	else
   	{
   		if(verbose)
   		{
   			printf(" AES MAC generate over 2 MB successful: (ret = %02x)\n", ret );
   		}
   	}


   	/*
	 * 2 - Send AES MAC generation command and insert a Cancel command while not yet finished
	 *
	 * !!! Only do it if previous test succeeded, if not there's already an issue.
	 */
   	if( ret == CSE_NO_ERR)
   	{

		if(verbose)
		{
			printf("\n");
			printf("Now trig a MAC computation over 2Mbytes but not waiting for answer this time - and aborting it via cancel\n");
   		}

		/* clean AES_MAC */
		AES_MAC[0] = 0;
		AES_MAC[1] = 0;
		AES_MAC[2] = 0;
		AES_MAC[3] = 0;

		/*
		 *	Send AES MAC generation command 'manually', since driver function is waiting for the result
		 */
		CSE->P1.R = CSE_RAM_KEY;
		CSE->P2.R = (uint32_t)(&long_MAC_payload_size);
		CSE->P3.R = (uint32_t)0x09000000; // Large Flash Blocks (both for McK2, Eiger1.1 and Eiger 2)
		CSE->P4.R = (uint32_t)AES_MAC;
		CSE->CMD.R = CSE_GENERATE_MAC;
		notify_new_request_available();

		// Since CSE_HSM is a sw implementation, BSY bit is not set immediately
		// need to wait for confirmation that request has been seen
		// - either because BSY is set by HSM
		// - or the NEW_REQ bit is cleared by HSM
		// then we can start polling for BSY to be reset

		// !!! Using BSY bit may lead to race conditions, safer to use the ACK mechanism

		/* Wait at least for cmd Ack */
		// Should add some timeout, in case HSM is not working
		/* wait for request to be handled (HSM will clear bit when request will be read ) */
		while( new_request_available() )
		{
		}

		/*
		 * Now send CANCEL command (still manually using registers)
		 */
		CSE->CMD.R = CSE_CANCEL;
		notify_new_request_available();

		/* Wait at least for cmd Ack */

		// Should add some timeout, in case HSM is not working
		/* wait for request to be handled (HSM will clear bit when request will be read ) */
		while( new_request_available() )
		{
		}

		/*
		 * Now wait for command completion
		 *
		 * This time we should get answer from CANCEL command --> CSE_NO_ERR
		 */

		/* wait until we're notified by the ISR */
		while (answer_available == 0) {}

		/* Clear notification variable */
		answer_available = 0;

		/* Test error code */
		ret = CSE->ECR.R;

		if( (AES_MAC[0] != 0) || (AES_MAC[1] != 0) || (AES_MAC[2] != 0) || (AES_MAC[3] != 0) )
		{
			if(verbose)
			{
				printf(" CANCEL command was not taken into account since MAC was computed anyway \n");
			}
			pass = 0;
		}

		if(ret != CSE_NO_ERR)
		{
			if(verbose)
			{
				printf(" CANCEL command failed, ret = %02x\n", ret );
			}
			pass = 0;
		}
		else
		{
			if(verbose)
			{
				printf(" Cancel command succeeded (ret = %02x)\n", ret );
			}
		}


		/* 3 - Check HSM is still alive and able to perform crypto operation, both in register and dma mode */
		pass2 = CSE_AES_HW_test( verbose );

		if(pass2)
		{
			if(verbose)
			{
				printf("\nAES tests after cmac abort succeeded\n");
			}
		}
		else
		{
			if(verbose)
			{
				printf("\nAES tests after cmac abort failed\n");
			}
			pass = 0;
		}
		if(verbose)
		{
			printf("current CSE ECR : %02x\n", CSE->ECR.R);
		}
   	}

   	return( pass );
}

/**
 * @brief          Management of M3 emulated registers configuration
 * @details        Allow to see and change adresses used in emulated registers configuration
 *
 * @param[in]      verbose    enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0	   When test failed
 * @retval 1	   When test succeeded
 *
 */
uint32_t m3_emul_reg_config_management( uint32_t verbose )
{
    uint32_t answer;
    uint8_t temp[10];
    uint32_t new_address;
    uint32_t m3_new_reg_address;
    uint32_t ret = 0;

    printf("Adresses used for emulated registers are : \n");
    printf(" - 0x%08X for M3 Emulated Registers\n", (uint32_t)SHE_REGS0_REMOTE_BASE);
    printf("Please check the address is the correct one\n");

    get_int("Is the address correct (0: No, 1: Yes) ?", &answer );

    if(answer == 0)
    {
        get_hex("Please enter new address for M3 Emulated Registers address : ", temp, 4);
        new_address = (temp[0] << 24) + (temp[1] << 16) + (temp[2] <<8) + temp[3];
        m3_new_reg_address = new_address;
        printf("New address for M3 Emulated Registers : %08x\n", m3_new_reg_address );

        get_int("Do you want to change it with entered value (0: No, 1: Yes) ?", &answer );

        if(answer == 1)
        {
            ret = CSE_API_ext_set_m3_she_reg_mem_config( m3_new_reg_address );
            if(ret == CSE_NO_ERR )
            {
                /* update M3 addresses for she_register area */
                SHE_REGS0_REMOTE_BASE = m3_new_reg_address;

                printf("SHE register address changed to %08x (M3) \n", m3_new_reg_address );

		CSE = (volatile struct CSE_HSM_tag *)SHE_REGS0_REMOTE_BASE;
	    }
            else
            {
                printf("Attempt to change she register address failed, ERC = %02d\n", ret);
            }
	}
    }
    return(ret);
}

/**
 * @brief          Management of A7 emulated registers configuration
 * @details        Allow to see and change adresses used in emulated registers configuration
 *
 * @param[in]      verbose    enable display of input, computed and expected values when set to 1
 *
 * @return         Error code
 * @retval 0	   When test failed
 * @retval 1	   When test succeeded
 *
 */
uint32_t a7_emul_reg_config_management( uint32_t verbose )
{
    uint32_t answer;
    uint8_t temp[10];
    uint32_t new_address;
    uint32_t a7_new_reg_address;
    uint32_t ret = 0;

    printf("Address used for emulated registers are : \n");
    printf(" - 0x%08X for A7 Emulated Registers\n", (uint32_t)A7_she_reg_addr);
    printf("Please check this address is the correct ones\n");

    get_int("Is the address correct (0: No, 1: Yes) ?", &answer );

    if(answer == 0)
    {
        get_hex("Please enter new address for A7 Emulated Registers address : ", temp, 4);
        new_address = (temp[0] << 24) + (temp[1] << 16) + (temp[2] <<8) + temp[3];
        a7_new_reg_address = new_address;
        printf("New address for A7 Emulated Registers : %08x\n", a7_new_reg_address );


        get_int("Do you want to change it with entered value (0: No, 1: Yes) ?", &answer );

        if(answer == 1)
        {
            ret = CSE_API_ext_set_a7_she_reg_mem_config( a7_new_reg_address );
            if(ret == CSE_NO_ERR )
            {
                /* update M3 addresses for she_register area */
                A7_she_reg_addr = (volatile struct CSE_HSM_tag *)a7_new_reg_address;

                printf("SHE register address changed to %08x (A7)\n", a7_new_reg_address );

                CSE = A7_she_reg_addr;
            }
            else
            {
                printf("Attempt to change she register address failed, ERC = %02d\n", ret);
            }
	}
    }
    return(ret);
}

/**
 * @brief	Management of key storage initialization command
 * @details	This function handles key storage initialization command
 *		configuring mailbox command buffer.
 *
 * @param[in]	verbose    enable display of input, computed and expected
 *		values when set to 1
 *
 * @return	Error code
 * @retval 0	When test failed
 * @retval 1	When test succeeded
 *
 */
uint32_t remote_key_import_config_management(uint32_t verbose)
{
	uint8_t tmp[10];
	uint32_t buffer;
	uint32_t ret = 0;
	uint32_t mbx_buffer = 0x7005FF80;
	uint32_t answer;

	printf("\nThe address used for exchanging KS proxy command is: 0x%08x\n",
	       mbx_buffer);
	get_int("Is the address correct (0: No, 1: Yes) ?", &answer );
	printf("\n");

	if (answer == 0) {
		get_hex("Please enter the address to exchange KS proxy commands :\n",
			 tmp, 4);
		buffer = (tmp[0] << 24) + (tmp[1] << 16) + (tmp[2] <<8) + tmp[3];
		mbx_buffer = buffer;
		printf("\nNew address for KS proxy commands is : 0x%08x\n",
		       mbx_buffer);
	}

	ret = CSE_ext_set_ext_remote_key_import(mbx_buffer);
	if (verbose)
		printf("\nReturned ERC : %02x\n", ret);

	return ret;
}

/**
 * @}
 */
