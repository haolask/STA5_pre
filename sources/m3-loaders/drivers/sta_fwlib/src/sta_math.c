/*
 * @file math_fixed.c
 * @brief This file provides math functions for Emerald
 *
 *   FRACTION (EMERALD Compatible) ONE = (1 << 23) - 1
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: APG-MID team
 */
#include "trace.h"

/* g1 = dec2hex(int(exp((log(10)/20)./10).*ONE + 0.5)) (MATLAB) */
#define G1	0x817b6e
/* 1/g1 = dec2hex(int(1 ./exp((log(10)/20)./10).*ONE + 0.5))  (MATLAB) */
#define OOG1	0x7e88e7
#define a_1_328472725 87063
#define b_2_263226475 148323
#define q16_2000_log2 39456604

/******************************************************************************
 * Function Name  : STA_db2lin
 *-----------------------------------------------------------------------------
 * Description    : db to linear, in fixed point (signed .24)
 *                  Reciprocal of db = 10 * 20 * log(g)
 * Input          : max 240
 * Return         : linear gain in fixed (signed.24) scaled by >> 4
 *****************************************************************************/
long STA_db2lin(long db)
{
	long long gainx, g1;
	int i;

	if (db >= 240)
		db = 240;
	if (db <= -1200)
		return 0;
	if (db == 0)
		return 0x7ffff;

	g1 = db > 0 ? G1 : OOG1;
	gainx = g1;
	if (db < 0)
		db = -db;
	for (i = 2; i <= db; i++)
		gainx = (gainx * g1) >> 23;

	return (long)(gainx >> 4);
}

/******************************************************************************
 * Function Name  : _STA_db2lin
 *-----------------------------------------------------------------------------
 * Description    : linear to db, in fixed point (signed .24)
 *                  Reciprocal of db = 10 * 20 * log(g)
 *                  clamp the returned linear gain to ONE
 * Return         : linear gain in fixed (signed.24) scaled by >> 4
 *
 *  amplitude = (1+r) * 2^n
 *  level (dB/100) = 2000 * log10(amplitude/2^23)
 *                 = 2000 * log10(2) * (ln(amplitude)-23)
 *                 = 2000 * log10(2) * (ln(1+r)+n-23)
 *
 *****************************************************************************/
long STA_lin2db(long amplitude)
{
	long ret;
	long temp32, n, r;
	long long temp64;

	amplitude <<= 4;

	if (amplitude <= 0)
		return -14000;

	/* compute n */
	temp32 = amplitude;
	n = -1;
	while (temp32) {
		n++;
		temp32 >>= 1;
	}

	/* compute 1+r in q16 */
	temp64 = amplitude;
	temp64 <<= 16;
	temp64 >>= n;
	temp32 = temp64;
	/* compute r in q16 */
	r = temp32 - (1 << 16);

	/* compute B = (a*r+b) in q16 */
	temp64 = r;
	temp64 *= a_1_328472725;
	temp64 >>= 16;
	temp64 += b_2_263226475;
	temp32 = temp64;

	/* compute A = (1-r)r in q32 */
	temp64 = 1 << 16;
	temp64 -= r;
	temp64 *= r;

	/* compute A/B + r in q16 */
	temp64 /= temp32;
	temp64 += r;
	temp32 = temp64;

	/* compute C */
	temp64 = n - 23;
	temp64 <<= 16;
	temp64 += temp32;
	temp32 = temp64;

	/* compute 2000*ln*C */
	temp64 = q16_2000_log2;
	temp64 *= temp32;
	temp64 += (1 << 30);
	temp64 >>= 32;
	ret = temp64 / 10;

	return ret;
}
