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

#include "os_hal.h"

#include "cse_typedefs.h"
#include "sta_uart.h"
#include "trace.h"

#include <stdlib.h>
#include "serialprintf.h"


static t_uart * const uart_address[] = {
	uart0_regs, uart1_regs, uart2_regs, uart3_regs
};


uint8_t get_char(void) {
  uint8_t car;
  enum port_e port = trace_port();
  int ret;

  do {
	osalThreadSleepMilliseconds(2);
	ret = uart_get_data_until_Timer(uart_address[port], (char*)&car, 1, 10);
  } while (ret != 0);

  return (car);
}


void get_int(char* message, uint32_t* pval) {
#define MAX_CHARS_INT 10

  int nb_cars = 0;
  char string[MAX_CHARS_INT + 1]; //10 max
  int end = 0;
  uint8_t car;

  serialprintf(message);

  while (!end) {

	car = get_char();

    if ((car >= '0') && (car <= '9')) {
      if (nb_cars == MAX_CHARS_INT) {
        serialprintf("%c", 0x8); // backspace
        nb_cars--;
      }
      serialprintf("%c", car);
      string[nb_cars] = car;

      nb_cars++;
    }

    if (car == 0x8) // backspace
        {
      if (nb_cars > 0) {
        nb_cars--;
        serialprintf("%c %c", car, car);
      }
    }

    /* if CR, end of input */
    if (car == 0xD) {
      serialprintf("%c", car);
      serialprintf("\r\n");
      string[nb_cars] = 0; // end of string

      end = 1;
    }
  }

  /* convert to integer */
  *pval = atoi(string);

}

void get_hex(char* message, uint8_t* pval, uint32_t byte_size) {
#define MAX_HEX 64

  uint32_t nb_cars = 0;
  char string[MAX_HEX + 1]; //32 hex digit max -> 16 data bytes
  int end = 0;
  uint8_t car;
//	uint32_t err;
  int i;
  char digit;
  uint32_t digit_val;
  uint32_t MSB_val, LSB_val;
  uint32_t MAX_chars = MAX_HEX;

  serialprintf("%s", message);

  if (byte_size * 2 < MAX_HEX) {
    MAX_chars = byte_size * 2;
  }
  else {
    MAX_chars = MAX_HEX;
  }

  while (!end) {

    car = get_char();

    if (((car >= '0') && (car <= '9')) || ((car >= 'A') && (car <= 'F'))
        || ((car >= 'a') && (car <= 'f'))) {
      if (nb_cars == MAX_chars) {
        printf("%c", 0x8); // backspace
        nb_cars--;
      }
      printf("%c", car);
      string[nb_cars] = car;

      nb_cars++;
    }

    if (car == 0x8) // backspace
        {
      if (nb_cars > 0) {
        nb_cars--;
        printf("%c %c", car, car);
      }
    }

    /* if CR, end of input */
    if (car == 0xD) {
      serialprintf("%c", car);
      serialprintf("\r\n");
      string[nb_cars] = 0; // end of string

      end = 1;
    }
  }

  /* clear end of 'string' */
  for (i = nb_cars; i < MAX_HEX; i++) {
    string[i] = '0';
  }

  /* convert to integer */
  for (i = 0; i < (int)byte_size; i++) {
    digit_val = 0;
    /* MSB */
    digit = string[2 * i];
    if ((digit >= '0') && (digit <= '9')) {
      digit_val = digit - '0';
    }
    if ((digit >= 'A') && (digit <= 'F')) {
      digit_val = digit - 'A' + 10;
    }
    if ((digit >= 'a') && (digit <= 'f')) {
      digit_val = digit - 'a' + 10;
    }
    MSB_val = digit_val;

    /* LSB */
    digit = string[2 * i + 1];
    if ((digit >= '0') && (digit <= '9')) {
      digit_val = digit - '0';
    }
    if ((digit >= 'A') && (digit <= 'F')) {
      digit_val = digit - 'A' + 10;
    }
    if ((digit >= 'a') && (digit <= 'f')) {
      digit_val = digit - 'a' + 10;
    }
    LSB_val = digit_val;

    pval[i] = MSB_val * 16 + LSB_val;
  }

}
