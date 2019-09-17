//!
//!  \file      rds_eon_data.c
//!  \brief     <i><b> RDS EON decoder </b></i>
//!  \details   RDS EON decoder functionality
//!  \author    Alberto Saviotti
//!  \author    (original version) Alberto Saviotti
//!  \version   1.0
//!  \date      2012.02.02
//!  \bug       Unknown
//!  \warning   None
//!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osal.h"

#ifdef CONFIG_ETALTML_HAVE_RDS

#include "etalinternal.h"

#include "dabmw_import.h"

#include "rds_data.h"

#include "rds_eon_data.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

typedef union
{
    tU8 byte[2];

    tU16 dbyte;
} DABMW_charIntTy;

#if 0
typedef union
{
    struct
    {
        tU8 b0:         1;
        tU8 b1:         1;
        tU8 b2:         1;
        tU8 b3:         1;
        tU8 b4:         1;
        tU8 b5:         1;
        tU8 b6:         1;
        tU8 b7:         1;
    } field;

    tU8 reg;
} DABMW_charBFTy;
#endif

typedef struct
{
    tU32 eonSave_pi;
    tU8 eonSave_ps[DABMW_RDS_PS_LENGTH];
    tU8 eonSave_af[25];
    tU8 eonSave_afNum;

    //DABMW_charBFTy eonSave_Flags;   
    tBool eonSave_tp;
    tBool eonSave_tpLast;
    tBool eonSave_ta;
    tBool eonSave_taLast;
    tBool eonSave_newTp;
    tBool eonSave_newTa;
} DABMW_eonTy;

static DABMW_eonTy DABMW_eon[DABMW_RDS_SOURCE_MAX_NUM][DABMW_EON_BUFFER_SIZE]; 

tSInt DABMW_RdsEonDataMngr (tSInt slot, 
                            tU32 piVal, tU8 realGroup,
                            tBool availBlock_A, tU32 block_A, 
                            tBool availBlock_B, tU32 block_B, 
                            tBool availBlock_C, tU32 block_C,
                            tBool availBlock_Cp, tU32 block_Cp,
                            tBool availBlock_D, tU32 block_D)
{
    tSInt res = OSAL_OK;
    tU32 temp;
    DABMW_charIntTy AFtemp;
    tU8 i, j, flag;

    (tVoid) piVal;  // lint
    (tVoid) block_A;  // lint
    (tVoid) block_Cp;  // lint
 
    // EON
    if (true == availBlock_A && true == availBlock_B && 
        (DABMW_GROUP_14A == realGroup) && true == availBlock_C && 
        true == availBlock_D)
    {
#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf (0, TR_LEVEL_COMPONENT, TR_CLASS_APP_DABMW, 
            "DABMW RDS: EON found, blocks %c - %c - %c - %c on group # %d\n", 'A', 'B', 'C', 'D', realGroup);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT) 

        temp = (block_B) & 0x000F;

        switch(temp)
        {
            // EON PS
            case 0:
                for(j = 0; j < DABMW_EON_BUFFER_SIZE; j++)
                {
                    if (DABMW_eon[slot][j].eonSave_pi == block_D)
                    {
                        DABMW_eon[slot][j].eonSave_ps[1] = (tU8)(block_C & 0x00FF);
                        DABMW_eon[slot][j].eonSave_ps[0] = (tU8)(block_C >> 8);
                    }
                }
                break;
            case 1:
                for(j = 0; j < DABMW_EON_BUFFER_SIZE; j++)
                {
                    if (DABMW_eon[slot][j].eonSave_pi == block_D)
                    {
                        DABMW_eon[slot][j].eonSave_ps[3] = (tU8)(block_C & 0x00FF);
                        DABMW_eon[slot][j].eonSave_ps[2] = (tU8)(block_C >> 8);
                    }
                }
                break;
            case 2:
                for(j = 0; j < DABMW_EON_BUFFER_SIZE; j++)
                {
                    if (DABMW_eon[slot][j].eonSave_pi == block_D)
                    {
                        DABMW_eon[slot][j].eonSave_ps[5] = (tU8)(block_C & 0x00FF);
                        DABMW_eon[slot][j].eonSave_ps[4] = (tU8)(block_C >> 8);
                    }
                }
                break;
            case 3:
                for(j = 0; j < DABMW_EON_BUFFER_SIZE; j++)
                {
                    if (DABMW_eon[slot][j].eonSave_pi == block_D)
                    {
                        DABMW_eon[slot][j].eonSave_ps[7] = (tU8)(block_C & 0x00FF);
                        DABMW_eon[slot][j].eonSave_ps[6] = (tU8)(block_C >> 8);
                    }
                }
                break;

            // EON AF
            case 4:
                AFtemp.dbyte = block_C;
 
                for(j = 0; j < DABMW_EON_BUFFER_SIZE; j++)
                {
                    if (DABMW_eon[slot][j].eonSave_pi == 0 || DABMW_eon[slot][j].eonSave_pi == block_D)
                    {
                        if (DABMW_eon[slot][j].eonSave_pi == 0)
                        {
                            DABMW_eon[slot][j].eonSave_pi = block_D;
                        }

                        for(i = 0; i < 25; i++)
                        {
                            if (AFtemp.byte[0] == DABMW_eon[slot][j].eonSave_af[i])AFtemp.byte[0]=0xFF;
                            if (AFtemp.byte[1] == DABMW_eon[slot][j].eonSave_af[i])AFtemp.byte[1] = 0xFF;
                        }
                        if (AFtemp.byte[0] < 205 && DABMW_eon[slot][j].eonSave_afNum < 25)
                        {
                            DABMW_eon[slot][j].eonSave_af[DABMW_eon[slot][j].eonSave_afNum] = AFtemp.byte[0];
                            DABMW_eon[slot][j].eonSave_afNum++;
                        }
                        if (AFtemp.byte[1] < 205 && DABMW_eon[slot][j].eonSave_afNum < 25)
                        {
                            DABMW_eon[slot][j].eonSave_af[DABMW_eon[slot][j].eonSave_afNum] = AFtemp.byte[1];
                            DABMW_eon[slot][j].eonSave_afNum++;
                        }
                        break;
                    }
                }
                
                
                break;

            case 5:
            case 6:
            case 7:
            case 8:
                AFtemp.dbyte = block_C;

                for(j = 0; j < DABMW_EON_BUFFER_SIZE; j++)
                {
                    if (DABMW_eon[slot][j].eonSave_pi ==0 || DABMW_eon[slot][j].eonSave_pi == block_D)
                    {
                        if (DABMW_eon[slot][j].eonSave_pi == 0)
                        {
                            DABMW_eon[slot][j].eonSave_pi = block_D;
                        }

                        for(i = 0; i < 25; i++)
                        {
                            if (AFtemp.byte[1] == DABMW_eon[slot][j].eonSave_af[i])AFtemp.byte[1] = 0xFF;
                        }
                        
                        if (AFtemp.byte[1] < 205 && DABMW_eon[slot][j].eonSave_afNum < 25)
                        {
                            DABMW_eon[slot][j].eonSave_af[DABMW_eon[slot][j].eonSave_afNum] = AFtemp.byte[1];
                            DABMW_eon[slot][j].eonSave_afNum++;
                        }

                        break;
                    }
                }

                break;

        }
    }

    // EON TP
    // EON TA
    if (true == availBlock_A && true == availBlock_B && 
        true == availBlock_Cp && true == availBlock_D && 
        (DABMW_GROUP_14B == realGroup))
    {
#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf (0, TR_LEVEL_COMPONENT, TR_CLASS_APP_DABMW, 
            "DABMW RDS: EON TP/TA found, blocks %c - %c - %c* - %c on group # %d\n", 'A', 'B', 'C', 'D', realGroup);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT) 

        flag = 0;
        
        for(j = 0; j < DABMW_EON_BUFFER_SIZE; j++)
        {
            if (DABMW_eon[slot][j].eonSave_pi == block_D && DABMW_eon[slot][j].eonSave_pi != 0)
            {

                if (block_B & 0x0010)
                    DABMW_eon[slot][j].eonSave_tp = true;
                else
                    DABMW_eon[slot][j].eonSave_tp = false;
                
                if (DABMW_eon[slot][j].eonSave_tpLast != DABMW_eon[slot][j].eonSave_tp)
                {
                    DABMW_eon[slot][j].eonSave_tpLast = DABMW_eon[slot][j].eonSave_tp;
                    DABMW_eon[slot][j].eonSave_newTp = true;
                }

                if (block_B & 0x0008)
                    DABMW_eon[slot][j].eonSave_ta = true;
                else
                    DABMW_eon[slot][j].eonSave_ta = true;
                
                if (DABMW_eon[slot][j].eonSave_taLast != DABMW_eon[slot][j].eonSave_ta)
                {
                    DABMW_eon[slot][j].eonSave_taLast = DABMW_eon[slot][j].eonSave_ta;
                    DABMW_eon[slot][j].eonSave_newTa = true;
                }

                flag = 1;
            }
        }
        if (flag == 0)
        {
            
        }
    }

    return res;
}

tVoid DABMW_RdsEonCleanUpData (tSInt slot)
{
    tSInt i, j;
    
    for (j = 0; j < DABMW_EON_BUFFER_SIZE; j++)
    {
        DABMW_eon[slot][j].eonSave_pi = 0;
        DABMW_eon[slot][j].eonSave_afNum = 0;
        
        for (i = 0; i < 25; i++)
        {
            DABMW_eon[slot][j].eonSave_af[i] = 0xFF;
        }
        
        for(i = 0; i < DABMW_RDS_PS_LENGTH; i++)
        {
            DABMW_eon[slot][j].eonSave_ps[i] = 0;
        }
    }   
}

//#ifdef __cplusplus
//}
//#endif

#endif // CONFIG_ETALTML_HAVE_RDS

// End of file

