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

#include "config.h"
#include "cse_typedefs.h"

#ifndef _MENU_H_
#define _MENU_H_

/*
 *  Interactive menu entries values
 */
/* CSE Initialization command */
#define CSE_INIT_TEST                   1

/* Status related commands */
#define GET_ID                          2
#define GET_FW_ID                       3
#define GET_STATUS                      4

/* Driver stimulation commands */
#define DRIVER_TEST                     5
#define RNG_TEST                        6
#define MAC_BIT_TEST                    7

/* Key management commands */
#define LOAD_DEFAULT_KEYS               10
#define NVM_KEY_STATUS_TEST             11
#define NVM_UPDATE_ITERATIVE            12
#define NVM_KEYS_ERASE                  13
#define NVM_KEY_UPDATE                  14
#define RAM_KEY_TEST                    15

extern void display_menu( void );
extern uint32_t select_operation(void);
extern uint32_t menu_entry( uint32_t operation, uint32_t* pverbose );
extern void display_test_configuration_settings(void);

#endif //_MENU_H_
/**
 * @}
 */
