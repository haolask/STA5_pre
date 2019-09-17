/* 
 * PREREQUISITES:
 * 1) include device memory definition header file
 * 2) define which format is desired (32 bit is default):
 *    #define COEFF_FORMAT_32BIT               ... for 32 bit address and 32 bit data (default)
 *    #define COEFF_FORMAT_24BIT_LITTLE_ENDIAN ... for 3 bytes (24 bit) address, 3 bytes (24 bit) data in little endian format
 *    #define COEFF_FORMAT_24BIT_BIG_ENDIAN    ... for 3 bytes (24 bit) address, 3 bytes (24 bit) data in big endian format
 */

#ifndef TDA7707_CUST_SETTINGS_H
#define TDA7707_CUST_SETTINGS_H

#ifndef _COEFF

#if defined(COEFF_FORMAT_24BIT_LITTLE_ENDIAN) /* 24 bit little endian format */ 

#define _COEFF(c, v) { ((c) & 0xFF), (((c)>>8) & 0xFF), (((c)>>16) & 0xFF) , ((v) & 0xFF), (((v)>>8) & 0xFF), (((v)>>16) & 0xFF)} 

typedef struct
{
    tU8 coeffAddr[3];
    tU8 coeffVal[3];
} tCoeffInit;

#elif defined(COEFF_FORMAT_24BIT_BIG_ENDIAN)   /* 24 bit big endian format */ 

#define _COEFF(c, v) { (((c)>>16) & 0xFF), (((c)>>8) & 0xFF), ((c) & 0xFF), (((v)>>16) & 0xFF), (((v)>>8) & 0xFF), ((v) & 0xFF)} 

typedef struct
{
    tU8 coeffAddr[3];
    tU8 coeffVal[3];
} tCoeffInit;

#else /* 32 bit format */

#define _COEFF(c, v) { c , v } 

typedef struct
{
    tU32 coeffAddr;
    tU32 coeffVal;
} tCoeffInit;

#endif
#endif  /* _COEFF */

#ifdef CMT_DEVICE_INDEPENDENT

const tCoeffInit coeffInit[]=
{
	/* Look at TDA7707_OM_CUT_xx.h to find parameters with corresponding addresses */
    _COEFF( CMT_systemConfig_audioMuteSlope		, 0x003354 ),
    _COEFF( CMT_amCoef_asp_volume         		, 0x3fd930 ),
};

#else /* not defined CMT_DEVICE_INDEPENDENT */

const tCoeffInit coeffInit_TDA7707[]=
{
	/* Look at TDA7707_OM_CUT_xx.h to find parameters with corresponding addresses */
    _COEFF( TDA7707_systemConfig_audioMuteSlope	, 0x003354 ),
    _COEFF( TDA7707_amCoef_asp_volume         	, 0x3fd930 ),
};

#endif /* CMT_DEVICE_INDEPENDENT */ 

#endif /* TDA7707_CUST_SETTINGS_H */

