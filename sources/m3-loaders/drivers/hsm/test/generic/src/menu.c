/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

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

/* Inclusion of the main header files of all the imported components in the
   order specified in the application wizard. The file is generated
   automatically.*/

#include <string.h>

#include "cse_client.h"
#include "config.h"
#include "serialprintf.h"
#include "serial_input.h"

#include "CSE_AES_HW_test.h"
#include "CSE_Debug_support_test.h"
#include "CSE_Manager_test.h"
#include "CSE_NVM_KEY_test.h"
#include "CSE_RAM_KEY_test.h"
#include "CSE_RNG_test.h"
#include "menu.h"
#include "Test_CSE_HW.h"
#include "test_values.h"

void display_menu( void )
{
    printf("\nTest Menu\n");
    printf("=========\n\n");
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("|                                                             |\n");
    printf("|                      SHE / SHE+ services                    |\n");
    printf("|                                                             |\n");
    printf("+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    printf("\nA/ CSE Initialization command\n");
    printf("-----------------------------\n");
    printf(" %d - Initialization of CSE\n",CSE_INIT_TEST);

    printf("\nB/ CSE status related commands\n");
    printf("------------------------------\n");
    printf(" %d - Get Chip Unique ID\n", GET_ID);
    printf(" %d - Get Firmware version ID\n", GET_FW_ID);
    printf(" %d - Display CSE status registers\n",GET_STATUS);

    printf("\nC/ SHE & SHE+ driver stimulation commands\n");
    printf("-----------------------------------------\n");
    printf(" %d - Run driver tests (ECB, CBC, CMAC, RNG, TRNG, RAM_KEY)\n", DRIVER_TEST);
    printf(" %d - Run RNG tests (uses PRNG and TRNG)\n", RNG_TEST);
    printf(" %d - Run CMAC bit tests\n", MAC_BIT_TEST);

    printf("\nD/ SHE & SHE+ key management commands\n");
    printf("-------------------------------------\n");
    printf(" %d - Load default Key set (assumes chip is virgin)\n", LOAD_DEFAULT_KEYS);
    printf(" %d - Show NVM key status (availability for enc/mac)\n", NVM_KEY_STATUS_TEST);
    printf(" %d - NVM keys iterative update tests\n", NVM_UPDATE_ITERATIVE);
    printf(" %d - Erase Keys (perform debug challenge/authorization)\n", NVM_KEYS_ERASE);
    printf(" %d - Update NVM Key with user provided values\n", NVM_KEY_UPDATE);
    printf(" %d - RAM KEY load/export/load protected test\n", RAM_KEY_TEST );
    printf("\n");
}

uint32_t select_operation(void)
{
    uint32_t choice = 1;

    printf("\n\n");
    get_int("Please select test command (enter for menu) : ", &choice);

    return(choice);
}

uint32_t menu_entry( uint32_t operation, uint32_t* pverbose )
{
    uint32_t ret = 0;
    uint32_t pass = 0;

    /* Check it is a supported option and call the associated code */
    switch(operation) {
        case CSE_INIT_TEST:
            CSE_init_test(*pverbose);
            break;
        case GET_ID:
            CSE_GetID_test(*pverbose);
            break;
        case GET_FW_ID:
            CSE_Get_Firmware_ID_test(*pverbose);
            break;
        case GET_STATUS:
            display_CSE_status();
            break;
        case DRIVER_TEST:
            test_CSE_driver(*pverbose);
            break;
        case RNG_TEST:
            pass = CSE_TRNG_test(*pverbose);
            display_pass_fail(pass);
            pass = CSE_PRNG_test(*pverbose);
            display_pass_fail(pass);
            break;
        case MAC_BIT_TEST:
            test_MAC(*pverbose);
            break;
        case LOAD_DEFAULT_KEYS:
            load_default_key_set( *pverbose );
            break;
        case NVM_KEY_STATUS_TEST:
            pass = CSE_NVM_Key_AES_test(*pverbose);
            display_pass_fail(pass);
            break;
        case NVM_UPDATE_ITERATIVE:
            iterative_nvm_key_update_test(*pverbose);
            break;
        case NVM_KEYS_ERASE:
            erase_keys_test(*pverbose);
            break;
        case NVM_KEY_UPDATE:
            pass = CSE_NVM_key_update_interactive_test(*pverbose);
            display_pass_fail(pass);
            break;
        case RAM_KEY_TEST:
            ram_key_test(*pverbose);
            break;
        default:
            ret = 1;
        break;
    }
    return(ret);
}
