#include "interpolation.h"
#include <Windows.h>

static Ts8 tun_ph_qual_fs_steps[TUN_PH_FS_STEPS_SIZE] = TUN_PH_FS_STEPS;

static Ts8 tun_ph_qual_usn_steps[TUN_PH_USN_STEPS_SIZE] = TUN_PH_USN_STEPS;

static Ts8 tun_ph_qual_wam_steps[TUN_PH_WAM_STEPS_SIZE] = TUN_PH_WAM_STEPS;

static Tu8 tun_ph_qual_fm_qual[TUN_PH_USN_STEPS_SIZE][TUN_PH_WAM_STEPS_SIZE][TUN_PH_FS_STEPS_SIZE] =
    { /* values are in external quality !! Do not use values below TUN_PH_MIN_QUAL and above TUN_PH_MAX_QUAL!!*/
        { /* usn = 0 */
            /*fs =           0,    5,   10,   15,   20,   25,   30,   35,   40,   45,   50,   55,   60,   65,   70,   75*/
            { /*wam =  0*/  30u,  35u,  45u,  55u,  65u,  75u,  80u,  80u,  85u,  90u,  90u,  95u, 100u, 100u, 100u, 100u},
            { /*wam =  8*/  25u,  30u,  40u,  50u,  60u,  70u,  75u,  75u,  80u,  85u,  85u,  90u,  95u,  95u,  95u,  95u},
            { /*wam = 13*/  15u,  20u,  30u,  40u,  50u,  60u,  65u,  65u,  70u,  75u,  75u,  80u,  85u,  85u,  85u,  85u},
            { /*wam = 18*/  10u,  15u,  25u,  35u,  42u,  50u,  55u,  55u,  60u,  65u,  65u,  70u,  75u,  75u,  75u,  75u},
            { /*wam = 25*/  10u,  15u,  25u,  35u,  40u,  45u,  50u,  50u,  55u,  60u,  60u,  65u,  70u,  70u,  70u,  70u}
        },
        { /* usn = 5 */
            /*fs =           0,    5,   10,   15,   20,   25,   30,   35,   40,   45,   50,   55,   60,   65,   70,   75*/
            { /*wam =  0*/  25u,  30u,  40u,  50u,  60u,  70u,  75u,  77u,  82u,  85u,  87u,  92u,  95u,  95u,  95u,  95u},
            { /*wam =  8*/  22u,  27u,  37u,  46u,  55u,  63u,  68u,  71u,  75u,  78u,  81u,  86u,  90u,  91u,  92u,  92u},
            { /*wam = 13*/  17u,  21u,  30u,  37u,  45u,  52u,  57u,  60u,  62u,  67u,  70u,  75u,  80u,  82u,  85u,  85u},
            { /*wam = 18*/  12u,  15u,  22u,  30u,  36u,  42u,  47u,  50u,  52u,  57u,  60u,  65u,  70u,  72u,  75u,  75u},
            { /*wam = 25*/  10u,  12u,  20u,  27u,  32u,  37u,  42u,  45u,  47u,  52u,  55u,  60u,  65u,  67u,  70u,  70u}
        },
        { /* usn = 15 */
            /*fs =           0,    5,   10,   15,   20,   25,   30,   35,   40,   45,   50,   55,   60,   65,   70,   75*/
            { /*wam =  0*/  15u,  20u,  27u,  32u,  40u,  47u,  52u,  57u,  60u,  62u,  67u,  72u,  75u,  77u,  82u,  85u},
            { /*wam =  8*/  15u,  20u,  27u,  31u,  36u,  41u,  46u,  51u,  52u,  56u,  61u,  66u,  70u,  73u,  80u,  82u},
            { /*wam = 13*/  15u,  17u,  22u,  25u,  28u,  32u,  36u,  40u,  40u,  45u,  50u,  55u,  60u,  65u,  72u,  75u},
            { /*wam = 18*/  12u,  12u,  15u,  17u,  21u,  26u,  30u,  32u,  32u,  36u,  41u,  46u,  50u,  55u,  62u,  65u},
            { /*wam = 25*/  10u,  10u,  12u,  15u,  17u,  22u,  27u,  30u,  30u,  32u,  37u,  42u,  45u,  50u,  57u,  60u}
        },
        { /* usn = 25 */
            /*fs =           0,    5,   10,   15,   20,   25,   30,   35,   40,   45,   50,   55,   60,   65,   70,   75*/
            { /*wam =  0*/  10u,  12u,  15u,  15u,  20u,  25u,  30u,  35u,  35u,  40u,  45u,  45u,  45u,  45u,  45u,  45u},
            { /*wam =  8*/  10u,  12u,  15u,  15u,  18u,  22u,  26u,  30u,  30u,  35u,  40u,  45u,  45u,  45u,  45u,  45u},
            { /*wam = 13*/  10u,  11u,  12u,  12u,  15u,  18u,  21u,  22u,  22u,  26u,  30u,  35u,  40u,  45u,  45u,  45u},
            { /*wam = 18*/  10u,  10u,  10u,  10u,  11u,  15u,  17u,  18u,  20u,  21u,  23u,  27u,  30u,  35u,  45u,  45u},
            { /*wam = 25*/  10u,  10u,  10u,  10u,  10u,  12u,  15u,  17u,  20u,  20u,  22u,  25u,  25u,  30u,  40u,  45u}
        },
        { /* usn = 35 */
            /*fs =           0,    5,   10,   15,   20,   25,   30,   35,   40,   45,   50,   55,   60,   65,   70,   75*/
            { /*wam =  0*/  10u,  10u,  10u,  10u,  15u,  20u,  25u,  30u,  30u,  35u,  40u,  45u,  45u,  45u,  45u,  45u},
            { /*wam =  8*/  10u,  10u,  10u,  10u,  15u,  20u,  22u,  25u,  25u,  30u,  35u,  40u,  45u,  45u,  45u,  45u},
            { /*wam = 13*/  10u,  10u,  10u,  10u,  12u,  17u,  20u,  20u,  20u,  22u,  25u,  30u,  35u,  40u,  45u,  45u},
            { /*wam = 18*/  10u,  10u,  10u,  10u,  10u,  12u,  15u,  17u,  20u,  20u,  20u,  22u,  25u,  30u,  40u,  45u},
            { /*wam = 25*/  10u,  10u,  10u,  10u,  10u,  10u,  10u,  15u,  20u,  20u,  20u,  20u,  20u,  25u,  35u,  40u}
        }
    };

Tu8 interpolation_flag = INTERPOLATION_OFS_ENABLE;
Tu32 u32_ofs_threshold = INTERPOLATION_OFS_THRESHOLD;

/*===========================================================================*/
/*  Tu8 QUAL_CalcFmQual                                            */
/*===========================================================================*/
Tu32 QUAL_CalcFmQual(Ts32 s32_BBFieldStrength, Tu32 u32_UltrasonicNoise, Tu32 u32_Multipath ,Tu32* q_BBFieldStrength, Tu32* q_no_Multipath ,Tu32 u32_FrequencyOffset)
{
	//Ts32 s32_FrequencyOffset = (Ts32)u32_FrequencyOffset;
    Ts32 s32_UltrasonicNoise = (Ts32)u32_UltrasonicNoise;
	Tu32 quality;
	
	if(interpolation_flag == INTERPOLATION_OFS_ENABLE)
	{
		s32_UltrasonicNoise = (s32_UltrasonicNoise + u32_FrequencyOffset);
		if(s32_UltrasonicNoise > 127)
		{
			s32_UltrasonicNoise = 127;
		} 
		else if(s32_UltrasonicNoise < -128)
		{
			s32_UltrasonicNoise = -128;
		}
		else
		{
		}

		quality = QUAL_CalcQuality(s32_BBFieldStrength, (s32_UltrasonicNoise), u32_Multipath, q_BBFieldStrength, q_no_Multipath);
	}
	else
	{
		if(u32_FrequencyOffset < u32_ofs_threshold)
		{
			quality = QUAL_CalcQuality(u32_FrequencyOffset, (s32_UltrasonicNoise), u32_Multipath, q_BBFieldStrength, q_no_Multipath);
		}
		else
		{
			quality = 10;
		}	
	}
	
	return quality;
}

/*===========================================================================*/
/*  Tu8 QUAL_CalcQuality                                            */
/*===========================================================================*/

Tu32 QUAL_CalcQuality(Ts32 s32_BBFieldStrength, Tu32 u32_UltrasonicNoise, Tu32 u32_Multipath ,Tu32* q_BBFieldStrength, Tu32* q_no_Multipath)
{
    Tu32 q = TUN_QUAL_ERROR; /* calculated quality */
	Ts32 s32_UltrasonicNoise = (Ts32)u32_UltrasonicNoise;
	Ts32 s32_Multipath = (Ts32)u32_Multipath;
	
    Ts32 fs1, fs2;    /* supporting points in quality table for field strength */
    Ts32 fsi1, fsi2;  /* index of  supporting points in quality table for field strength */
    Tu32 dfs;        /* delta field strength between fs1 and fs2 */
    Tu32 rfs;        /* division rest */
    Tu32 pfs1, pfs2; /* scaler for fs interpolation */

    Tu32 usn1, usn2;    /* supporting points in quality table for usnacent channel */
    Ts32 usni1, usni2;  /* index of supporting points in quality table for usnacent channel */
    Tu32 dusn;         /* delta usnacent channel between usn1 and usn2 */
    Tu32 rusn;         /* devision rest */
    Tu32 pusn1, pusn2; /* scaler for usn interpolation */

    Tu32 wam1, wam2;     /* supporting points in quality table for multipath */
    Ts32 wami1, wami2;   /* index of supporting points in quality table for multipath */
    Tu32 dwam;         /* delta multipath sensor between wam1 and wam2 */
    Tu32 rwam;         /* devision rest */
    Tu32 pwam1, pwam2;  /* scaler for wam interpolation */

    Tu32 qab, qcd, qef, qgh; /* interpolated quality in fs directions */
    Tu32 qabef, qcdgh;       /* interpolated quality in usn direction */
    Tu32 q_tu32; /* Tu32 quality */

    /* check input ranges and limit them to the valid ranges */
    s32_BBFieldStrength = ((s32_BBFieldStrength < TUN_PH_FS_MIN) ? TUN_PH_FS_MIN : ((s32_BBFieldStrength > TUN_PH_FS_MAX) ? TUN_PH_FS_MAX: s32_BBFieldStrength));
	s32_UltrasonicNoise = ((s32_UltrasonicNoise < TUN_PH_USN_MIN) ? TUN_PH_USN_MIN : ((s32_UltrasonicNoise > TUN_PH_USN_MAX) ? TUN_PH_USN_MAX : s32_UltrasonicNoise));
	s32_Multipath = ((s32_Multipath < TUN_PH_WAM_MIN) ? TUN_PH_WAM_MIN : ((s32_Multipath > TUN_PH_WAM_MAX) ? TUN_PH_WAM_MAX : s32_Multipath));

    /* find supporting points for interpolation */
    /*lint -save -e440 [MISRA 2004 Rule 13.5, required] */
    for(fsi2=1; tun_ph_qual_fs_steps[fsi2] < s32_BBFieldStrength; fsi2++)
    /*lint -restore */
    {
       /* do nothing just count*/
    }
    fs2 = tun_ph_qual_fs_steps[fsi2];
    fsi1 = (fsi2 - 1);
    fs1 = tun_ph_qual_fs_steps[fsi1];

    /*lint -save -e440 [MISRA 2004 Rule 13.5, required] */
    for(usni2=1; tun_ph_qual_usn_steps[usni2] < s32_UltrasonicNoise; usni2++)
    /*lint -restore */
    {
       /* do nothing just count*/
    }
    usn2 = tun_ph_qual_usn_steps[usni2];
    usni1 = (usni2 - 1);
    usn1 = tun_ph_qual_usn_steps[usni1];

    /*lint -save -e440 [MISRA 2004 Rule 13.5, required] */
    for(wami2=1; tun_ph_qual_wam_steps[wami2] < s32_Multipath; wami2++)
    /*lint -restore */
    {
       /* do nothing just count*/
    }
    wam2 = tun_ph_qual_wam_steps[wami2];
    wami1 = (Ts32)(wami2 - 1);
    wam1 = tun_ph_qual_wam_steps[wami1];

    /* interpolation algorithm comes here */
    /* Interpolation in fs direction is done by adding the scaled qualities for fs1 and fs2.
       Interpolation for q[fs] between q[fs1] and q[fs2] is done by whighting q[fs1] and q[fs2]
       with the distance of fs to fs1 and fs2.
       exawamle for fs1 = 100 and fs2 = 200 and fs = 125
       scaling = (fs-fs1) / (fs2-fs1)  = (125-100)/(200 - 100) = 0.25
       => q[fs2] scaled with 25%
       => q[fs1] scaled with 100% - 25% = 75%
       q = q[fs1] * (1-0.25) + q[fs2] * 0.25
       To make this in integer the scaled is multiplied by 128 (1<<7)
       To remove the scaling the result have to be shifted 7 bits to the right!
       q = q[fs1] * (1-0.25) + q[fs2] * 0.25
    */
    {
        Ts32 sdfs = fs2 - fs1;
        Ts32 srfs = s32_BBFieldStrength - fs1;
        /* calculate the delta between fs1 and fs2 */
        dfs = (Tu32)sdfs;
        /* calculate the position of fs relative to fs1*/
        rfs = (Tu32)srfs;
    }
    /* calculate the scaling values for fs1 and fs2*/
	pfs2 = (Tu32)(LIB_SHIFTLEFT(rfs, 7u)) / dfs;
    pfs1 = 128u - pfs2; /* (1 << 7) - pfs1; */

    /* calculate the interpolation for all 8 quality points in fs direction */
    /* the result values are the quality multiplied by 128!! */
    qab = ((tun_ph_qual_fm_qual[usni1][wami1][fsi1] * pfs1) + (tun_ph_qual_fm_qual[usni1][wami1][fsi2] * pfs2));
    qcd = ((tun_ph_qual_fm_qual[usni1][wami2][fsi1] * pfs1) + (tun_ph_qual_fm_qual[usni1][wami2][fsi2] * pfs2));
    qef = ((tun_ph_qual_fm_qual[usni2][wami1][fsi1] * pfs1) + (tun_ph_qual_fm_qual[usni2][wami1][fsi2] * pfs2));
    qgh = ((tun_ph_qual_fm_qual[usni2][wami2][fsi1] * pfs1) + (tun_ph_qual_fm_qual[usni2][wami2][fsi2] * pfs2));

    if(q_BBFieldStrength != NULL)
    {
		*q_BBFieldStrength = (Tu8)(LIB_SHIFTRIGHT(qab, 7u));
    }
    /* just a block for tewamorary variables */
    {
        Ts32 sdusn = (Ts32)usn2 - (Ts32)usn1;
        Ts32 srusn = (Ts32)u32_UltrasonicNoise - (Ts32)usn1;
        /* calculate the delta between usn1 and usn2 */
        dusn = (Tu32)sdusn;
        /* calculate the position relative to usn1*/
        rusn = (Tu32)srusn;
    }
    /* calculate the scaling factor for usn1 and usn2*/
	pusn2 = (Tu32)(LIB_SHIFTLEFT(rusn, 7u)) / dusn;
    pusn1 = 128u - pusn2; /* (1<<7) - pusn2 */

    /* calculate the interpolation in usn resolution*/
	qabef = LIB_SHIFTRIGHT(((qab * pusn1) + (qef * pusn2)), 7u);
	if(q_no_Multipath!=NULL)
	{
		*q_no_Multipath = (Tu8)(LIB_SHIFTRIGHT(qabef, 7u));
	}
	qcdgh = LIB_SHIFTRIGHT(((qcd * pusn1) + (qgh * pusn2)), 7u);

    {
        Ts32 sdwam = (Ts32)wam2 - (Ts32)wam1; /* sdwam is just for MISRA cowamliant type casts */
        Ts32 srwam = (Ts32)u32_Multipath - (Ts32)wam1;  /* srwam is just for MISRA cowamliant type casts */
        dwam = (Tu32)sdwam;
        rwam = (Tu32)srwam;
    }
	pwam2 = (Tu32)(LIB_SHIFTLEFT(rwam, 7u)) / dwam;
    pwam1 = 128u - pwam2; /* (1<<7) - pwam2 */

    /* calculate the interpolation in wam direction */
    q_tu32 = (qabef * pwam1) + (qcdgh * pwam2);
    /* calculate the quality by removing the two time 128 multiplications */
	q = (Tu8)(LIB_SHIFTRIGHT(q_tu32, 14u));
	if ((LIB_AND(q_tu32, 0x3fffu)) >= 0x2000u)
	{   
		/* round up in case division rest is bigger than 0.5 */
		q++;
	}
	return q;
}