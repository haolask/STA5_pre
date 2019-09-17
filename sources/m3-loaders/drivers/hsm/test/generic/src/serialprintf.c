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
 * @file    serialprintf.c
 * @brief   Mini printf-like functionality.
 *
 * @addtogroup serialprintf
 * @{
 */
#include "serialprintf.h"

#define MAX_LENGTH 256

#define MAX_FILLER 11
#define FLOAT_PRECISION 100000

extern void printchar(char **str, int c);


/* Using HAL */
void serialprintf_init( void )
{
	/*
	* Activates the serial driver 3 using the driver default configuration.
	*/

}

void write_char( uint8_t c )
{
    printchar( 0, c );
}

static char *long_to_string_with_divisor(char *p, long num, unsigned radix,
                                         long divisor) {
  int i;
  char *q;
  long l, ll;

  l = num;
  if (divisor == 0) {
    ll = num;
  }
  else {
    ll = divisor;
  }

  q = p + MAX_FILLER;
  do {
    i = (int)(l % radix);
    i += '0';
    if (i > '9')
      i += 'A' - '0' - 10;
    *--q = i;
    l /= radix;
  } while ((ll /= radix) != 0);

  i = (int)(p + MAX_FILLER - q);
  do
    *p++ = *q++;
  while (--i);

  return p;
}

static char *ltoa(char *p, long num, unsigned radix) {

  return long_to_string_with_divisor(p, num, radix, 0);
}

#if SERIALPRINTF_USE_FLOAT
static char *ftoa(char *p, double num)
{
  long l;
  unsigned long precision = FLOAT_PRECISION;

  l = num;
  p = long_to_string_with_divisor(p, l, 10, 0);
  *p++ = '.';
  l = (num - l) * precision;
  return long_to_string_with_divisor(p, l, 10, precision / 10);
}
#endif

/**
 * @brief   System formatted output function.
 * @details This function implements a minimal @p vprintf()-like functionality
 *          with output on serial port.
 *          The general parameters format is: %[-][width|*][.precision|*][l|L]p.
 *          The following parameter types (p) are supported:
 *          - <b>x</b> hexadecimal integer.
 *          - <b>X</b> hexadecimal long.
 *          - <b>o</b> octal integer.
 *          - <b>O</b> octal long.
 *          - <b>d</b> decimal signed integer.
 *          - <b>D</b> decimal signed long.
 *          - <b>u</b> decimal unsigned integer.
 *          - <b>U</b> decimal unsigned long.
 *          - <b>c</b> character.
 *          - <b>s</b> string.
 *          .
 *
 * @param[in] fmt       formatting string
 * @param[in] ap        list of parameters
 *
 * @api
 */
void serialvprintf(const char *fmt, va_list ap) {
  char *p, *s, c, filler;
  int i, precision, width;
  bool is_long, left_align;
  long l;
#if CHPRINTF_USE_FLOAT
  float f;
  char tmpbuf[2*MAX_FILLER + 1];
#else
  char tmpbuf[MAX_FILLER + 1];
#endif

  while (true) {
    c = *fmt++;
    if (c == 0)
    {
    	//return;
    	break;
    }

    if (c != '%') {
      //chnWrite(&SD1, (uint8_t*)&c, 1);
      write_char(c);
      continue;
    }
    p = tmpbuf;
    s = tmpbuf;
    left_align = false;
    if (*fmt == '-') {
      fmt++;
      left_align = true;
    }
    filler = ' ';
    if ((*fmt == '.') || (*fmt == '0')) {
      fmt++;
      filler = '0';
    }
    width = 0;
    while (true) {
      c = *fmt++;
      if ( (c >= '0') && (c <= '9'))
      c -= '0';
      else if (c == '*')
      c = va_arg(ap, int);
      else
      break;
      width = width * 10 + c;
    }
    precision = 0;
    if (c == '.') {
      while (true) {
        c = *fmt++;
        if (c >= '0' && c <= '9')
        c -= '0';
        else if (c == '*')
        c = va_arg(ap, int);
        else
        break;
        precision *= 10;
        precision += c;
      }
    }
    /* Long modifier.*/
    if (c == 'l' || c == 'L') {
      is_long = true;
      if (*fmt)
        c = *fmt++;
    }
    else
      is_long = (c >= 'A') && (c <= 'Z');

    /* Command decoding.*/
    switch (c) {
    case 'c':
      filler = ' ';
      *p++ = va_arg(ap, int);
      break;
    case 's':
      filler = ' ';
      if ((s = va_arg(ap, char *)) == 0)
        s = "(null)";
      if (precision == 0)
        precision = 32767;
      for (p = s; *p && (--precision >= 0); p++)
        ;
      break;
    case 'D':
    case 'd':
    case 'I':
    case 'i':
      if (is_long)
      l = va_arg(ap, long);
      else
      l = va_arg(ap, int);
      if (l < 0) {
        *p++ = '-';
        l = -l;
      }
      p = ltoa(p, l, 10);
      break;
#if CHPRINTF_USE_FLOAT
      case 'f':
      f = (float) va_arg(ap, double);
      if (f < 0)
      {
        *p++ = '-';
        f = -f;
      }
      p = ftoa(p, f);
      break;
#endif
    case 'X':
    case 'x':
      c = 16;
      goto unsigned_common;
    case 'U':
    case 'u':
      c = 10;
      goto unsigned_common;
    case 'O':
    case 'o':
      c = 8;
      unsigned_common:
      if (is_long)
      l = va_arg(ap, unsigned long);
      else
      l = va_arg(ap, unsigned int);
      p = ltoa(p, l, c);
      break;
    default:
      *p++ = c;
      break;
    }
    i = (int)(p - s);
    if ((width -= i) < 0)
      width = 0;
    if (left_align == false)
      width = -width;
    if (width < 0) {
      if (*s == '-' && filler == '0') {
        //chnWrite(&SD1, (uint8_t*)s++, 1);
    	  write_char(*s++);
        i--;
      }
      do
      {
    	  //chnWrite(&SD1, (uint8_t*)&filler, 1);
    	  write_char(filler);
      }
      while (++width != 0);
    }
    while (--i >= 0)
    {
    	//chnWrite(&SD1, (uint8_t*)s++, 1);
    	write_char(*s++);
    }

    while (width) {
      //chnWrite(&SD1, (uint8_t*)&filler, 1);
    	write_char(filler);
      width--;
    }
  }
}

void serialdisplay_word(vuint32_t word) {
  serialprintf("%08x", word);
  osalThreadSleepMilliseconds(100);
}

void serialdisplay_array(uint8_t* buffer, int byte_count) {
  int i;
  uint8_t* buf_pt = (uint8_t*)buffer;

  for (i = 0; i < byte_count; i++) {
    serialprintf("%02x", buf_pt[i]);
    if (i % 1 == 15)
      serialprintf(" ");
  }

  serialprintf("\r\n");
  osalThreadSleepMilliseconds(100);
}

#if 0
void serialprint_string( char * string )
{
  int cnt = 0;
  int i = 0;

  for(i=0; i<MAX_LENGTH; i++)
  {
    if(string[i] == 0)
    {
      cnt = i;
      // exit the 'for' loop
      break;
    }
  }

  if(cnt!=0)
  {
    //chnWrite(&SD1, (uint8_t*)string, cnt);
    put_string_n( string, cnt );
  }
  else
  {
    // either we didn't found the end of string char before the MAX_LENGTH, or the string was empty
    //--> do not display anything
  }
}
#endif

void display_buf(char* cmt, uint8_t* buf, uint32_t size) {
  //serialprint_string(cmt);
  printf(cmt);
  serialdisplay_array(buf, (int)size);
}

/** @} */
