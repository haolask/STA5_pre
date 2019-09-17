/**
 * printf-stdarg.h: This file contains the small printf functions declaration
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef __PRINTF_STDARG_H__
#define __PRINTF_STDARG_H__

#include <stdarg.h>
#include <sta_map.h>

int printf(const char *, ...);
int sprintf(char *buf, const char *, ...);
int snprintf(char *buf, unsigned int count, const char *, ...);

int print(char **out, const char *format, va_list args);
int prints(char **out, const char *string, int width, int pad);
void printstm(const char *buff, unsigned int size);

t_uart **uart_address_array_get(void);
#endif /* __PRINTF_STDARG_H__ */

