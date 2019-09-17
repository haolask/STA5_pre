/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/
/*  **********************
    ***** DISCLAIMER *****
    **********************
    "Long File Names" is an intelluectual property of 
    "Microsoft Corporation". In this module LFN can be used exclusively when 
    propedeutic for flashing WinCE6.0 artifacts.
    LFN is used  for copying Storage Based Image auxialiary files.
 */

#ifndef _INTEGER_
#define _INTEGER_

/* These types must be 16-bit, 32-bit or larger integer */
typedef int				INT;
typedef unsigned int	UINT;

/* These types must be 8-bit integer */
typedef char			CHAR;
typedef uint8_t	UCHAR;
typedef uint8_t	BYTE;

/* These types must be 16-bit integer */
typedef short			SHORT;
typedef uint16_t	USHORT;
typedef uint16_t	WORD;
typedef uint16_t	WCHAR;

/* These types must be 32-bit integer */
typedef long			LONG;
typedef uint32_t	ULONG;
typedef uint32_t	DWORD;

#endif
