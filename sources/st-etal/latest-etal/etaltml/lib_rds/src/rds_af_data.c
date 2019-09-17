//!
//!  \file      rds_af_data.c
//!  \brief     <i><b> RDS AF decoder </b></i>
//!  \details   RDS AF decoder functionality
//!  \author    Alberto Saviotti
//!  \author    (original version) Alberto Saviotti
//!  \version   1.0
//!  \date      2011.10.06
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

//#include "system_status.h"

#include "rds_landscape.h"

#include "rds_af_data.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

//#define DABMW_AF_ENTRY_NOT_TO_BE_USED               0
//#define DABMW_AF_ENTRY_FILLER_CODE                  1
//#define DABMW_AF_ENTRY_NOT_ASSIGNED                 2
//#define DABMW_AF_ENTRY_NO_AF_EXISTS                 3
//#define DABMW_AF_ENTRY_AF_FOLLOWS                   4
//#define DABMW_AF_ENTRY_LFMF_FREQUENCY_FOLLOW        5

typedef struct
{
    DABMW_storageStatusEnumTy status;
    tBool afList;
    tU8 afMethod;
    tU8 afNum;
    tU8 afRxNum;

    tU16 tempVal;
    tU8 tempNum;
    tU16 afVal[DABMW_AF_LIST_BFR_LEN_16BITS];
} DABMW_afNewTy;

static DABMW_afNewTy DABMW_afNew[DABMW_RDS_SOURCE_MAX_NUM];

tSInt DABMW_RdsAfDataInit (tVoid)
{
    tSInt res = OSAL_OK;
    tSInt cnt;

    for (cnt = 0; cnt < DABMW_RDS_SOURCE_MAX_NUM; cnt++)
    {    
        // Init the structure used to catch teh data and build the AF list
        DABMW_afNew[cnt].status = DABMW_STORAGE_STATUS_IS_EMPTY;
        DABMW_afNew[cnt].afList = false;
        DABMW_afNew[cnt].afMethod = 0xFF;
        DABMW_afNew[cnt].afNum = 0;
        DABMW_afNew[cnt].afRxNum = 0;
        DABMW_afNew[cnt].tempNum = 0;
        OSAL_pvMemorySet ((tPVoid)&DABMW_afNew[cnt].afVal[0], 0x00, sizeof(tU16)*DABMW_AF_LIST_BFR_LEN_16BITS);  
    }
    
    return res;
}

tSInt DABMW_RdsAfDataPush (tSInt slot, tU8 data, tU8 pos)
{
    tBool found;
    tSInt cnt;

    // Init tempVal because thsi is the first half of a couple
    if (0 == pos)
    {
        DABMW_afNew[slot].tempVal = 0;
    }    

    // Check invalid values
    if (DABMW_AF_ENTRY_NOT_TO_BE_USED_VALUE == data)
    {
        // Invalid value
        return OSAL_ERROR;
    }

    // Check if the data is no AF exists value
    if (DABMW_AF_ENTRY_NO_AF_EXISTS_VALUE == data)
    {
        // Re-init all, no AF exists
        return DABMW_RdsAfDataReInit (slot);
    }

    // Check of the data is LF/MF frequency follow
    if (DABMW_AF_ENTRY_LFMF_FREQUENCY_FOLLOW_VALUE == data)
    {
        DABMW_afNew[slot].tempVal = ((tU16)DABMW_AF_ENTRY_LFMF_FREQUENCY_FOLLOW_VALUE << (tU16)8) & (tU16)0xFF00;

        return OSAL_OK;
    }

    // Check if the data is the number of AF
    if (data >= 225 && data <= 249)
    {
        // Frequency number can be only in pos 0
        if (0 == pos)
        {
            // Received freq num can be zeroed, a new list starts if the af num
            // is new
            if (DABMW_afNew[slot].afNum != (data - DABMW_AF_ENTRY_AF_NUM_BASE_VALUE + 1))
            {
                DABMW_RdsAfDataReInit (slot);
            
                DABMW_afNew[slot].afNum = data - DABMW_AF_ENTRY_AF_NUM_BASE_VALUE + 1;
                DABMW_afNew[slot].status = DABMW_STORAGE_STATUS_IS_STORED;
            }

            return OSAL_OK;
        }
        else
        {
            // Invalid value
            return OSAL_ERROR;
        }
    }

    // If this point is reached data is a frequency value

    // If no freq num is already stored just return and discard the frame
    if (0 == DABMW_afNew[slot].afNum)
    {
        // Invalid sequence, re-init and return
        DABMW_RdsAfDataReInit (slot);
        
        return OSAL_ERROR;
    }
    
    if (0 == pos)
    {
        DABMW_afNew[slot].tempVal = ((tU16)data << (tU16)8) & (tU16)0xFF00;
        DABMW_afNew[slot].tempNum++;
    }
    else
    {
        DABMW_afNew[slot].tempVal |= ((tU16)data << (tU16)0) & (tU16)0x00FF;

        if ((tU16)DABMW_AF_ENTRY_FILLER_CODE_VALUE != (DABMW_afNew[slot].tempVal & (tU16)0x00FF))
        {
            DABMW_afNew[slot].tempNum++;
        }
        
        // Check existing values for a duplicated entry, in that case
        // just return because it is already catched
        found = false;
        for (cnt = 0; cnt < DABMW_AF_LIST_BFR_LEN_16BITS; cnt++)
        {
            if (DABMW_afNew[slot].afVal[cnt] == DABMW_afNew[slot].tempVal)
            {
                found = true;

                break;
            }
        }

        if (false == found)
        {
            // If this point is reached a new frequency is found
            for (cnt = 0; cnt < DABMW_AF_LIST_BFR_LEN_16BITS; cnt++)
            {
                // Search for a free slot
                if (0 == DABMW_afNew[slot].afVal[cnt])
                {
                    DABMW_afNew[slot].afVal[cnt]= DABMW_afNew[slot].tempVal;

                    // Increment the retrieved frequency number
                    DABMW_afNew[slot].afRxNum += DABMW_afNew[slot].tempNum;

                    break;
                }
            }      
        }    

        // Re-start counting
        DABMW_afNew[slot].tempNum = 0;
    }

    return OSAL_OK;
}

tSInt DABMW_RdsAfDataMngr (tSInt slot, tU32 piVal, tU32 freq)
{
    tSInt res = OSAL_OK;

    // Return if the current list is not completed
    if ((DABMW_afNew[slot].afRxNum != DABMW_afNew[slot].afNum) || (0 == DABMW_afNew[slot].afNum))
    {
        // No error is returned, if no list is completed is correct to return here
        return res; 
    }

    // If the list is complete start searching for storing place
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
    res = DABMW_AmFmLandscapeSetAf (piVal, DABMW_afNew[slot].afRxNum,(tPU16)&DABMW_afNew[slot].afVal[0], freq);
#endif
    if (DABMW_STORAGE_STATUS_IS_STORED == DABMW_afNew[slot].status)
    {
       // The list must be removed from the DABMW_afNew[slot] structure
       // and left on the DABMW_afLandscape only
       DABMW_RdsAfDataReInit (slot); 
    }

    return res;
}

tSInt DABMW_RdsAfDataReInit (tSInt slot)
{
    tSInt res = OSAL_OK;    

    // Re-init the temporary storage structure
    DABMW_afNew[slot].status = DABMW_STORAGE_STATUS_IS_EMPTY;
    DABMW_afNew[slot].afList = false;
    DABMW_afNew[slot].afMethod = 0xFF;
    DABMW_afNew[slot].afNum = 0;
    DABMW_afNew[slot].afRxNum = 0;
    DABMW_afNew[slot].tempNum = 0;
    OSAL_pvMemorySet ((tPVoid)&DABMW_afNew[slot].afVal[0], 0x00,sizeof(DABMW_afNew[slot].afVal));    

    return res;
}

//#ifdef __cplusplus
//}
//#endif

#endif // CONFIG_ETALTML_HAVE_RDS

// End of file

