//!
//!  \file      rds_landscape.c
//!  \brief     <i><b> RDS landscape source file </b></i>
//!  \details   RDS landscape related management
//!  \author    Alberto Saviotti
//!  \author    (original version) Alberto Saviotti
//!  \version   1.0
//!  \date      2012.02.06
//!  \bug       Unknown
//!  \warning   None
//!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "osal.h"

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE

#include "etalinternal.h"
#include "dabmw_import.h"

#if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)
    #include "nvm_comm.h"
#endif // #if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)

//#include "system_status.h"

//#include "system_app.h"

#include "rds_data.h"

//#include "rds_mngr.h"

#include "rds_af_data.h"

#include "rds_landscape.h"

//#ifdef __cplusplus
//extern "C" {
//#endif

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
#define DABMW_RDS_MAX_NUM_REGISTERED_CALLBACK       ((tU8)6)
#endif

// The algorithm for data storage and removal from the amfm db could be the following:
// - On new PI a new entry in the db is created
// - On new frequency available (without PI) discovered by BACKGROUND scan
//   a new entry with INVALID PI is created in the db
// - Items are removed from the db if they are not available in the last 'x' 
//   background scans if the background scan is given with the REMOVAL parameter. 
//
// The db shall store the last used frequency together with an array of available
// frequencies. The latter could be the AF db
#ifndef CONFIG_TARGET_CPU_COMPILER_GNU
typedef __packed struct
#else
typedef struct
#endif
{
    // Database status
    DABMW_storageStatusEnumTy status;
    DABMW_storageStatusEnumTy sendStatus;

    // Stored values: statis RDS information (PI, PS and AF list) and frequency
    tU32 piValue;
    tU32 frequency;
	OSAL_tMSecond pi_StoredTime;
    tU8 label[DABMW_RDS_PS_LENGTH]; 
    DABMW_afLandscapeTy afDb;

    tU8 PTY;

    // Quality value
    tU8 overallQ;
} DABMW_amfmLandscapeTy;

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
typedef struct
{
    tBool usedSlot;
    DABMW_AmFmRdsCallbackTy clbkFnct;
    tU32 registeredFrequency;
    //tU32 pi;
    tBool onlyNewPi;
    tBool onlyNewPs;
    tBool onlyNewAfList;
    tPVoid parPtr;
} DABMW_amFmRdsCallbackInformationTy;

typedef struct
{
    tSInt stationNumber;
    tSInt stationNumberWithRds;
} DABMW_amfmLandscapeGlobalsTy;

static DABMW_amFmRdsCallbackInformationTy DABMW_amFmRdsCallbackInformation[DABMW_RDS_MAX_NUM_REGISTERED_CALLBACK];
#endif // CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL

// AM/FM landscape db
static DABMW_amfmLandscapeTy DABMW_amfmLandscape[DABMW_AMFM_LANSCAPE_SIZE];

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
static DABMW_amfmLandscapeGlobalsTy DABMW_amfmLandscapeGlobals;
#endif

static tU32 DABMW_AmFmLandscapeSearchForFreq (tU32 frequency);
    
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
static tVoid DABMW_RdsCheckForRegisteredUsers (tU32 piVal, tBool isAPs, tU32 frequency);

static tVoid DABMW_RdsCheckForAfListRegisteredUsers (tU32 piVal, tU32 frequency, DABMW_afStorageTy *dataPtr);
#endif

static tVoid DABMW_EraseAmFmDb (tVoid);

tSInt DABMW_AmFmLandscapeSetFrequencyWithoutPi (tU32 frequency, tU32 overallQuality)
{
    tSInt res = OSAL_OK;
    tSInt cnt;
    tU32 slot;

    // Get an already used slot for that frequency, if a slot is found the
    // only information we have to store is the overall quality, if not,
    // we store the frequency in a new slot initializing all other data 
    // to invalid because still not available
    slot = DABMW_AmFmLandscapeSearchForFreq (frequency);

    // If this is a new frequency allocate a slot to it
    if (DABMW_INVALID_DATA == slot)
    {
        for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {
            if (DABMW_STORAGE_STATUS_IS_EMPTY == DABMW_amfmLandscape[cnt].status)
            {                
                DABMW_amfmLandscape[cnt].frequency = frequency;
                DABMW_amfmLandscape[cnt].overallQ = overallQuality;
                DABMW_amfmLandscape[cnt].piValue = DABMW_INVALID_DATA;
                DABMW_amfmLandscape[cnt].status = DABMW_STORAGE_STATUS_IS_USED;
                DABMW_amfmLandscape[cnt].sendStatus = DABMW_STORAGE_STATUS_IS_STORED;

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
                DABMW_amfmLandscapeGlobals.stationNumber++;
#endif

                break;
            }
        }   
    }
    else
    {
        // Store in the slot only the new overall quality indicator
        DABMW_amfmLandscape[slot].overallQ = overallQuality;
    }
    
    return res;
}

tSInt DABMW_AmFmLandscapeSetPi (tU32 piVal, tU32 freq)
{
    tSInt res = OSAL_OK;
    tU32 cnt;
    tBool found = false;
	tSInt vl_oldestCnt;
	OSAL_tMSecond vl_OldestStoredTime;

	
    // If the frequency is invalid return with error
    if (0 == freq || DABMW_INVALID_FREQUENCY == freq)
    {
        return res;
    }

    // Search the PI to find if a slot has been already allocated to it

	/* EPR change
	* get the found cnt if reuse of same slot : same freq / same PI
	*/
	cnt = DABMW_AmFmLandscapeSearchForPi (piVal, freq);
    if (DABMW_INVALID_DATA != cnt)
	/* END EPR CHANGE */
    	{
        found = true;
    	}

    // If the same frequency is already stored but the PI is invalid or different
    // update it
    if (false == found)
    {
        for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {
            if (DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_amfmLandscape[cnt].status &&
                freq == DABMW_amfmLandscape[cnt].frequency &&
                piVal != DABMW_amfmLandscape[cnt].piValue)
            {                 
                DABMW_amfmLandscape[cnt].piValue = piVal;
                DABMW_amfmLandscape[cnt].status = DABMW_STORAGE_STATUS_IS_USED;
                DABMW_amfmLandscape[cnt].sendStatus = DABMW_STORAGE_STATUS_IS_STORED;

                found = true;
                
                break;
            }
        }   
    }  

    // If this is a new PI or it is a duplicated PI but on a new frequency
    // allocate a slot to it
    if (false == found)
    {
        for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {
            if (DABMW_STORAGE_STATUS_IS_EMPTY == DABMW_amfmLandscape[cnt].status)
            {                               
                DABMW_amfmLandscape[cnt].piValue = piVal;
                DABMW_amfmLandscape[cnt].frequency = freq;
                DABMW_amfmLandscape[cnt].status = DABMW_STORAGE_STATUS_IS_USED;
                DABMW_amfmLandscape[cnt].sendStatus = DABMW_STORAGE_STATUS_IS_STORED;

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
                DABMW_amfmLandscapeGlobals.stationNumber++;
                DABMW_amfmLandscapeGlobals.stationNumberWithRds++;          
#endif

                found = true;
                
                break;
            }
        }   
    }



	// UPDATE : if nothing found, release the older one 
	//
	
	// If this is a new PI or it is a duplicated PI but on a new frequency
    // allocate a slot to it
    if (false == found)
    {
    	vl_OldestStoredTime = 0;
		vl_oldestCnt = 0;
		
        for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {
        	if (DABMW_amfmLandscape[cnt].pi_StoredTime > vl_OldestStoredTime)
        	{
        		vl_OldestStoredTime = DABMW_amfmLandscape[cnt].pi_StoredTime;
				vl_oldestCnt = cnt;
        	}
        }   

		// now take it : vl_oldestCnt
	   // start by erasing
	   
		OSAL_pvMemorySet ((tPVoid)&DABMW_amfmLandscape[vl_oldestCnt], 0x00, sizeof(DABMW_amfmLandscapeTy)); 
		
	   DABMW_amfmLandscape[vl_oldestCnt].piValue = piVal;
       DABMW_amfmLandscape[vl_oldestCnt].frequency = freq;
       DABMW_amfmLandscape[vl_oldestCnt].status = DABMW_STORAGE_STATUS_IS_USED;
       DABMW_amfmLandscape[vl_oldestCnt].sendStatus = DABMW_STORAGE_STATUS_IS_STORED;
		
	   found = true;
		
    }
	

	if (true == found)
		{
		/* it means the pi is available somehow
		* store the validity time
		*/
		DABMW_amfmLandscape[cnt].pi_StoredTime = OSAL_ClockGetElapsedTime();
		}
	
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
    // Call registered users if any. This shall be done in any case
    DABMW_RdsCheckForRegisteredUsers (piVal, false, freq);
#endif

    return res;
}


tSInt DABMW_AmFmLandscapeSetPs (tU32 piVal, tPU8 psPtr, tU32 freq)
{
    tSInt res = OSAL_OK;
    tU32 piSlot = DABMW_INVALID_DATA;
    tSInt resCmp;

    // Search the PI to find if a slot has been already allocated to it
    piSlot = DABMW_AmFmLandscapeSearchForPi (piVal, freq);

    // If the PI is available, store the PS after checking if it is different
    if (DABMW_INVALID_DATA != piSlot)
    {
        resCmp = OSAL_s32MemoryCompare ((tPVoid)&DABMW_amfmLandscape[piSlot].label[0], (tPVoid)psPtr, DABMW_RDS_PS_LENGTH);

        if (0 != resCmp)
        {
            OSAL_pvMemoryCopy ((tPVoid)&DABMW_amfmLandscape[piSlot].label[0], (tPVoid)psPtr, DABMW_RDS_PS_LENGTH);

            // Set to stored, this will reverd RETRIEVED status
            DABMW_amfmLandscape[piSlot].sendStatus = DABMW_STORAGE_STATUS_IS_STORED;

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
            // Signal to PS registered users that a new PS has arrived
            DABMW_RdsCheckForRegisteredUsers (piVal, true, freq);  
#endif
        }
    }

    return res;
}

tSInt DABMW_AmFmLandscapeSetPTY (tU32 piVal, tU8 pty, tU32 freq)
{
    tSInt res = OSAL_OK;
    tU32 piSlot = DABMW_INVALID_DATA;

    piSlot = DABMW_AmFmLandscapeSearchForPi (piVal, freq);

    if (DABMW_INVALID_DATA != piSlot)
    {
        DABMW_amfmLandscape[piSlot].PTY = pty;
    }

    return res;
}

tSInt DABMW_AmFmLandscapeGetPTY (tU32 piVal, tU8 *pty, tU32 freq)
{
    tU32 piSlot = DABMW_INVALID_DATA;

    piSlot = DABMW_AmFmLandscapeSearchForPi (piVal, freq);

    if (DABMW_INVALID_DATA != piSlot)
    {
        *pty = DABMW_amfmLandscape[piSlot].PTY;
        return 1;
    }
    return 0;
}

tSInt DABMW_AmFmLandscapeSetFreq (tU32 piVal, tU32 freq)
{
    tSInt res = OSAL_OK;
    tU32 piSlot = DABMW_INVALID_DATA;

    // Search the PI to find if a slot has been already allocated to it
    piSlot = DABMW_AmFmLandscapeSearchForPi (piVal, freq);    

    // If the PI is available, store the PS after checking if it is different
    if (DABMW_INVALID_DATA != piSlot)
    {
        DABMW_amfmLandscape[piSlot].sendStatus = DABMW_STORAGE_STATUS_IS_STORED;
        DABMW_amfmLandscape[piSlot].frequency = freq;

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
        // On a new PI must be called the function taking care of clients waiting for this PI to be received
        DABMW_RdsCheckForRegisteredUsers (piVal, false, freq);        
#endif
    }    

    return res;
}

tU32 DABMW_AmFmLandscapeGetFreqFromPi (tU32 piVal)
{
    tU32 frequency = DABMW_INVALID_FREQUENCY;
    tU32 piSlot;

    // Search the PI to find if a slot has been already allocated to it
    piSlot = DABMW_AmFmLandscapeSearchForPi (piVal, DABMW_INVALID_FREQUENCY);    

    // If the PI is available, store the PS after checking if it is different
    if (DABMW_INVALID_DATA != piSlot)
    {
        frequency = DABMW_amfmLandscape[piSlot].frequency;    
    }    

    return frequency;
}

tSInt DABMW_AmFmLandscapeSetAf (tU32 piVal, tU8 rxFreqNum, tPU16 afValPtr, tU32 freq)
{
    tSInt cnt;
    tBool found = false;
    tU32 piSlot = DABMW_INVALID_DATA;
    tSInt cpCnt;
    tU8 comparedFreqNum;
    tU8 avValTmpBfr[DABMW_AF_LIST_BFR_LEN];
    tSInt tmpRxFreqNum = (tSInt)rxFreqNum;
    tU32 baseFreq;
    tBool isAfListTypeB;
    tU8 tmpData;
    tSInt dstCnt, srcCnt;
    tBool fault = false;
	tSInt vl_StoredAfCnt;

    // Copy the data to a local buffer
    OSAL_pvMemorySet ((tPVoid)&avValTmpBfr[0], 0x00, DABMW_AF_LIST_BFR_LEN); 

    avValTmpBfr[0] = (tU8)(*afValPtr & (tU16)0xFF);
    tmpRxFreqNum--;
    dstCnt = 1;
    srcCnt = 1;
    while (tmpRxFreqNum > 0)
    {
        tmpData = (tU8)(*(afValPtr + srcCnt) & (tU16)0xFF);
        if (DABMW_AF_ENTRY_FILLER_CODE_VALUE != tmpData)
        {
            avValTmpBfr[dstCnt] = tmpData;
            dstCnt++;
            tmpRxFreqNum--;
        }

        tmpData = (tU8)((*(afValPtr + srcCnt) >> 8) & (tU16)0xFF); 
        if (DABMW_AF_ENTRY_FILLER_CODE_VALUE != tmpData)
        {
            avValTmpBfr[dstCnt] = tmpData; 
            dstCnt++;
            tmpRxFreqNum--;            
        }    

        srcCnt++;
    }

    // Check if the type is A or B and calculate base frequency (it is the same
    // for AF list type A and type B)
    if (avValTmpBfr[0] == avValTmpBfr[1] ||
        avValTmpBfr[0] == avValTmpBfr[2] ||
        avValTmpBfr[0] == avValTmpBfr[3])
    {
        isAfListTypeB = true;

        // Calculate base frequency and call registered users
        baseFreq = 87500 + avValTmpBfr[0] * 100;        

        // Sanity check: the received list is valid ONLY if the base frequency
        // (stored in avValTmpBfr[0] is present until the end of the list, if not could
        // be that we mixed 2 different lists)
        for (cnt = 1; cnt < (rxFreqNum - 1); cnt += 2)
        {
            if (avValTmpBfr[cnt]       != avValTmpBfr[0] &&
                avValTmpBfr[(cnt + 1)] != avValTmpBfr[0])
            {
                fault = true;

                break;
            }
        }

        // Return because we are in error
        if (true == fault)
        {
            return OSAL_ERROR;
        }
    }
    else
    {
        isAfListTypeB = false;

        // For type A the frequency is not inside the buffer
        baseFreq = freq;                
    } 
    
    // Search the PI to find if a slot has been already allocated to it
    piSlot = DABMW_AmFmLandscapeSearchForPi (piVal, baseFreq);

    // If the piSlot is invalid no data shall be stored
    if (DABMW_INVALID_DATA == piSlot)
    {
        return OSAL_OK;
    }    

    // Clear the confirmatin number for the AF list
    if (DABMW_amfmLandscape[piSlot].afDb.afListTypeConfirmed < DABMW_AF_LIST_TYPE_CONFIRMED_NUMBER)
    {
        if (DABMW_amfmLandscape[piSlot].afDb.isAfListTypeB == isAfListTypeB)
        {
            DABMW_amfmLandscape[piSlot].afDb.afListTypeConfirmed++;
        }
        else
        {
            // Reset all the list of this channel because they could be wrong
            OSAL_pvMemorySet ((tPVoid)&DABMW_amfmLandscape[piSlot].afDb, 0x00, sizeof (DABMW_afLandscapeTy)); 

            // Set type to the new one
            DABMW_amfmLandscape[piSlot].afDb.isAfListTypeB = isAfListTypeB;            
        }        
    }
    
    // If the confidence level has not been reached then return
    if (DABMW_amfmLandscape[piSlot].afDb.afListTypeConfirmed < DABMW_AF_LIST_TYPE_CONFIRMED_NUMBER)
    {
        // Not an error, we are just in the phase when we understand the AF list type
        return OSAL_OK;
    }    

    // Completed list, analyze it and copy to AF db if it is 
    // not a duplicate ones.
    // If the pi match than search for the current list to see if it 
    // is a duplicated one            
    for (cnt = 0; cnt < DABMW_AF_LIST_NUMBER_PER_PI; cnt++)
    {
        // Check if the storage space for the list already used
        if (DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].status)                
        {
            // Compare the stored list with the current one to see if it is
            // already available
            
            //
            // codex artefact #282956
            //
            // Duplication of an AF list is not only if the full list is received in same order, but if the list is received with same contents
            
            // for a given slot/frequency, we see  AF list, which have identical contents, but stored in different order 
            // ex : List 1 ==> afNum = 13, af = (184, 32, 2, 105, 33, 130, 129, 141, 138, 147, 144, 164, 155, 0,
            // List 2 ==> afNum = 13, af = (184, 105, 33, 130, 129, 147, 144, 164, 155, 32, 2, 141, 138, 0,
            // List 1 & 2 should be considered equivalent
            // this happens typically if a block is lost (error on the block for instance), then the list will be completed only when the lost block is read ok...this will result in a AF list built in different order...
            //
            // extend the Check for all AF in the list.
            // loop on all received AF
            // for each one, check if it is stored in the AF 
            //
            // for type B : check the couples...
            // 
            
            comparedFreqNum = 0;
			if (false == DABMW_amfmLandscape[piSlot].afDb.isAfListTypeB)
            {
                // type A : check one to one
	            for (cpCnt = 0; cpCnt < rxFreqNum; cpCnt++)
	            {
    
   
	            	for (vl_StoredAfCnt = 0; vl_StoredAfCnt < DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].afNum; vl_StoredAfCnt++)
	            	{
		                if (DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[vl_StoredAfCnt] == avValTmpBfr[cpCnt])
		                {
		                    comparedFreqNum++;
							// move to next freq
							break;
		                }
	            	}
            	}
			}
			else
			{
				// type B / check by couple	
				// type B coding
				//	[0] = the freq for which we have the AF
				//	[1 & 2] = the couple to inform the AF . 
				// 
				if (DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[0] == avValTmpBfr[0])
				{
					// we have the same base freq, let's look to the couples
					//
					comparedFreqNum = 1;
					for (cpCnt = 1; cpCnt < rxFreqNum; cpCnt+=2)
		            {
				        		
		            	for (vl_StoredAfCnt = 1; vl_StoredAfCnt < DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].afNum; vl_StoredAfCnt+=2)
		            	{
			                if ((DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[vl_StoredAfCnt] == avValTmpBfr[cpCnt])
								&& (DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[vl_StoredAfCnt+1] == avValTmpBfr[cpCnt+1]))
			                {
			                	// 2 good freq check : increment by 2
			                    comparedFreqNum+=2;
								// move to next freq
								break;
			                }
		            	}
					}
				}
            	
			}                       
            
            // Check if all frequencies are the same
            if (comparedFreqNum >= rxFreqNum)
            {
               found = true; 

               break;
            }
        }
    }

    // Store everything if the pi was never stored, 
    // else store only the list if it was not already found
    if (false == found)
    {
        for (cnt = 0; cnt < DABMW_AF_LIST_NUMBER_PER_PI; cnt++)
        {
            // Check if the storage space for the list already used
            if (DABMW_STORAGE_STATUS_IS_EMPTY == DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].status)
            {
                // Set status
                DABMW_amfmLandscape[piSlot].afDb.status = DABMW_STORAGE_STATUS_IS_USED;
                
                // Copy data
                for (cpCnt = 0; cpCnt < rxFreqNum; cpCnt++)
                {
                    DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[cpCnt] = avValTmpBfr[cpCnt];
                }

                DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].afNum = rxFreqNum;

                DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].status = DABMW_STORAGE_STATUS_IS_USED;

                DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].sendStatus = DABMW_STORAGE_STATUS_IS_STORED;

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
                DABMW_RdsCheckForAfListRegisteredUsers (DABMW_amfmLandscape[piSlot].piValue, 
                        baseFreq, &DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt]);                 
#endif
                
                break;
            }
        }
    }

    return OSAL_OK;
}

tSInt DABMW_RdsLandscapeInit (tBool nvmRead)
{
    tSInt res = OSAL_OK;    
    tSInt cnt;
#if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)  
    tU32 tmpSize;
    tU32 dataSizeToRead;
    tU32 dataSizeReadFromNvm;
#endif // #if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)      

    if (false == nvmRead)
    {
        // Erase the landscape db
        DABMW_EraseAmFmDb ();
    }
    else
    {
#if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)
        tmpSize = (tU32)sizeof(DABMW_amfmLandscapeTy) * (tU32)DABMW_AMFM_LANSCAPE_SIZE;  

        // Get the payload data from NVM
        dataSizeToRead = (tU16)tmpSize;

        res = DABMW_NvmFileRead (DABMW_NVM_DATATYPE_DB_AMFM_PAYLOAD, DABMW_NVM_USERID,
                                 dataSizeToRead, (tPU8)&DABMW_amfmLandscape,
                                 (tPS32)&dataSizeReadFromNvm);
#else
        res = OSAL_ERROR;
#endif // #if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)

        if (OSAL_ERROR == res)
        {
            // Erase the landscape db
            DABMW_EraseAmFmDb ();
        }
        else
        {
            // Set number of available stations
            for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
            {
                if (DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_amfmLandscape[cnt].status)
                {
                    DABMW_amfmLandscape[cnt].PTY = 0x00; // invalidate PTY, it changes dynamically
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
                    DABMW_amfmLandscapeGlobals.stationNumber++;
#endif
                }

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
                if (0 != DABMW_amfmLandscape[cnt].piValue &&
                    DABMW_INVALID_RDS_PI_VALUE != DABMW_amfmLandscape[cnt].piValue)
                {
                    DABMW_amfmLandscapeGlobals.stationNumberWithRds++;
                }
#endif
            }
        }
    }
    
    return res;
}

tSInt DABMW_RdsLandscapeClose (tVoid)
{
    tSInt res = OSAL_OK;    
#if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)    
    tU32 dataSizeToWrite;
    tU32 tmpSize;
#endif // #if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)   

#if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)
    tmpSize = (tU32)sizeof(DABMW_amfmLandscapeTy) * (tU32)DABMW_AMFM_LANSCAPE_SIZE;  

    dataSizeToWrite = tmpSize;

    // Write the header data to NVM
    res = DABMW_NvmFileWrite (DABMW_NVM_DATATYPE_DB_AMFM_PAYLOAD, DABMW_NVM_USERID,
                              (tPU8)&DABMW_amfmLandscape, dataSizeToWrite);
#else
    res = OSAL_ERROR;
#endif // #if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)    
        
    return res;
}

tSInt DABMW_RdsLandscapeClear (tBool clearRam)
{
    tSInt res = OSAL_OK;
#if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)    
    tU32 tmpSize;
    tPVoid emptyMemAreaPtr;
#endif // #if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)

    // If we need to clear the RAM let's do it
    if (true == clearRam)
    {        
        // Erase the landscape db
        DABMW_EraseAmFmDb ();
    }

#if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)
    tmpSize = (tU32)sizeof(DABMW_amfmLandscapeTy) * (tU32)DABMW_AMFM_LANSCAPE_SIZE;   

    // Write the header data to NVM, we cannot use the real header because 
    // it could be still valid due to not-erasing of the RAM. Here we use a local malloc
    // that we free just after this operation
    emptyMemAreaPtr = OSAL_pvMemoryAllocate (tmpSize);

    if (NULL != emptyMemAreaPtr)
    {
        // Set all the data to invalid
        OSAL_pvMemorySet ((tPVoid)emptyMemAreaPtr, 0x00, tmpSize); 
    
        res = DABMW_NvmFileWrite (DABMW_NVM_DATATYPE_DB_HEADER, DABMW_NVM_USERID,
                                  (tPU8)emptyMemAreaPtr, tmpSize);

        OSAL_vMemoryFree (emptyMemAreaPtr);
    } 
#else
    res = OSAL_ERROR;
#endif // #if defined (CONFIG_TARGET_DABMW_NVM_COMM_ENABLE)    
        
    return res;
}

tVoid DABMW_RdsAfDbOnChannelChange (tVoid)
{
    tSInt cnt, cnt2;
    
    // Set to STORED the send status

    // TODO  TBD
    // We should reset only the new PI or the new Frequency....
    // why resetting others ?
    //
    for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
    {
        for (cnt2 = 0; cnt2 < DABMW_AF_LIST_NUMBER_PER_PI; cnt2++)
        {
            if (DABMW_STORAGE_STATUS_IS_RETRIEVED == DABMW_amfmLandscape[cnt].afDb.afEntries[cnt2].sendStatus)                
            {
                DABMW_amfmLandscape[cnt].afDb.afEntries[cnt2].sendStatus = DABMW_STORAGE_STATUS_IS_STORED;
            }
        }
    }
}

tSInt DABMW_RdsGetAfList (ETAL_HANDLE hReceiver, tPU8 dataPtr, tBool forcedGet, tU32 piVal, tU32 baseFreq, 
                          tBool mode, tU32 maxNumberRetrieved)
{
    tSInt res = 0;
    tSInt cnt;
    tSInt cnt2;
    tU32 piSlot = DABMW_INVALID_DATA;
    tU32 baseFrequency = DABMW_INVALID_FREQUENCY;
    tU32 calculatedFrequency;
    tU32 currentFrequency;
    tU32 storageIndex = 0;
    tPU8 localPtr;
    tU32 localMax;
    
    tU8  vl_index;
    tBool vl_freqAlreadyStored;

    // Check if the frequency must be checked
    if (DABMW_GET_AF_AUTODETECTMODE == mode)
    {
        // Search the PI to find if a slot has been already allocated to it

        /* EPR CHANGE */

        // EPR comment : here we have a risk, if baseFreq not passed : we find the 1st slot which is in landscape with the PI
        // if several slot exist... this is not handled, and we may not be on the selected current one.
        // so we should 1st select the baseFrequency....
        // in theory... we should do it on right APP 


        // If the base frequency is not passed by the host than it must be retrieved
        // by the STA662
        if (DABMW_INVALID_FREQUENCY == baseFreq)
        {
            // Get the current tuned frequency for the main AMFM application 
            // (for background scan the RDS datacannot be directly retrieved by the HOST)        
#ifdef ETAL_RDS_IMPORT
			baseFrequency = ETAL_receiverGetFrequency(hReceiver);
			if (baseFrequency == ETAL_INVALID_FREQUENCY)
			{
				baseFrequency = DABMW_INVALID_FREQUENCY;
			}
#else
            baseFrequency = DABMW_GetFrequencyFromApp (DABMW_MAIN_AMFM_APP);
#endif
            
            if (DABMW_INVALID_FREQUENCY == baseFrequency)
            {
                // Error, return 0 so no length is computed
                return res;
            }
        }

        
        piSlot = DABMW_AmFmLandscapeSearchForPi (piVal, baseFrequency);
        /* END EPR CHANGE */

        // If piSlot is invalid just return
        if (DABMW_INVALID_DATA == piSlot)
        {
            // Error, return 0 so no length is computed
            return res;
        }

       
        /* EPR CHANGE */
        // we should not make a different handling if baseFreq is set or not.
        // we should just set the frequency to the requested one, but then, same handling.
        else
        {
            baseFrequency = baseFreq;          
        }
        /* END EPR CHANGE */
    }
    else // DABMW_GET_AF_CURRENTSTATUSMODE
    {

        /* if a baseFreq is provided then we should use it */
        /* EPR CHANGE */     
        baseFrequency = baseFreq;
        /* END EPR CHANGE */
        
        baseFreq = DABMW_INVALID_FREQUENCY;

        /* EPR CHANGE */
        /* we should find the right slot : the one corresponding to requested frequency !
                *
                // Search the PI to find if a slot has been already allocated to it
                piSlot = DABMW_AmFmLandscapeSearchForPi (piVal, baseFreq);
                */
        piSlot = DABMW_AmFmLandscapeSearchForPi (piVal, baseFrequency);

        /* END EPR CHANGE */
        
        // baseFrequency handling :
        // we keep it to DABMW_INVALID_FREQUENCY
        // that will be used in getting the AF : 
        // we will provide back all frequencies found for the PI slot, whatever the 'based Freq'
        //

        // If piSlot is invalid just return
        if (DABMW_INVALID_DATA == piSlot)
        {
            // Error, return 0 so no length is computed
            return res;
        }        
    }

    // If PI has not been found return
    if (DABMW_INVALID_DATA != piSlot)
    {

        /* EPR CHANGE  */
        
        // This procedure should only provide the AF num and list, not the PI which is a known input
        // PI in RSP on API is to be handle by the caller
        //
        /*
        // first field in the response is the PI
        *(dataPtr + 0) = (DABMW_amfmLandscape[piSlot].piValue & 0xFF000000) >> 24;
        *(dataPtr + 1) = (DABMW_amfmLandscape[piSlot].piValue & 0x00FF0000) >> 16;
        *(dataPtr + 2) = (DABMW_amfmLandscape[piSlot].piValue & 0x0000FF00) >>  8;
        *(dataPtr + 3) = (DABMW_amfmLandscape[piSlot].piValue & 0x000000FF) >>  0;
        res = 4;
        localPtr = dataPtr + 4;
        localMax = maxNumberRetrieved - 4;
        */
        localPtr = dataPtr;
        localMax = maxNumberRetrieved;
        res = 0;
        /* END EPR CHANGE */
        
        /* EPR CHANGE */
        // we should not reset the storage index in the loop but at building start                        
        storageIndex = 0;
        /* END EPR CHANGE */

        // The data can be available
        for (cnt = 0; cnt < DABMW_AF_LIST_NUMBER_PER_PI; cnt++)
        {
             /* EPR CHANGE */ 
            // If the base frequency has not been passed return the data for the current service
           
            /* same processing whatever the baseFreq parameter
                       // if (DABMW_INVALID_FREQUENCY == baseFreq)

                       // {
                       */
             /* END EPR CHANGE */    
                // Send back the AF list if it is available and verified                
                if ((DABMW_STORAGE_STATUS_IS_STORED == DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].sendStatus) ||
                    (DABMW_STORAGE_STATUS_IS_RETRIEVED == DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].sendStatus &&
                     true == forcedGet))
                {                    
                    if (false == DABMW_amfmLandscape[piSlot].afDb.isAfListTypeB)
                    {
                        // First frequency to be stored is base frequency that for
                        // type A is stored in the first entry of the AF list
                        
                        /* EPR CHANGE */
                        // we should not reset the storage index in the loop but at building start                        
                        //storageIndex = 0;
                        /* END EPR CHANGE */
                            
                        // Do the check for TYPE A
                        cnt2 = 0;                        
                        while (cnt2 < DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].afNum)
                        {
                            if (DABMW_AF_ENTRY_FILLER_CODE_VALUE != DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[cnt2])
                            {
                                /* EPR CHANGE */
                                // do not provide back the frequency if already in

                                vl_freqAlreadyStored = false;
                                for (vl_index = 0; vl_index < storageIndex; vl_index++)
                                {
                                    if (DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[cnt2] == *(localPtr + vl_index))
                                    {
                                        vl_freqAlreadyStored = true;
                                        break;
                                    }
                                    
                                }
                                
                                if ((storageIndex < localMax) && (vl_freqAlreadyStored == false))
                                {
                                    *(localPtr + storageIndex) = DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[cnt2]; 

                                    storageIndex++;                                   
                                }
                                /* END EPR CHANGE */
                            }

                            cnt2++;
                        }

                        DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].sendStatus = DABMW_STORAGE_STATUS_IS_RETRIEVED;

                        // Return the number of frequency retrieved that is the number of available
                        // frequencies minus 
                        res = storageIndex;

                        /* EPR CHANGE */
                        // why do we break ? 
                        // shouldn't we loop on all lists ?
                        // avoid the break and provide the complete list !!
                        // what could be avoided is to provide twice the same
                        // 
                        // break;
                        /* END EPR CHANGE */
                    }
                    else
                    {
                        // Do the check for TYPE B
                        // Calculate the current frequency
                        calculatedFrequency = 87500 + (100 * DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[0]);

                        /* EPR CHANGE */
                        // baseFrequency may be  invalid here if baseFreq was invalid and CURRENTSTATUSMODE 
                        // ie we need to consider that case as well 
                        //
                        // if baseFrequency = invalid, provide all AF whitout filtering the baseFreq
                        //
                        /* END EPR CHANGE */
 
                        if ((baseFrequency == calculatedFrequency)
                            ||
                            (DABMW_INVALID_FREQUENCY == baseFrequency ))
                        {                        
                            cnt2 = 0;

                            // Store base frequency as first one in the response only one
                            if (0 == storageIndex)
                            {
                                *(localPtr + 0) = DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[0];

                                // Next frequency will be stored starting next byte
                                storageIndex = 1;
                            }
                            
                            while (cnt2 < DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].afNum)
                            {
                                // Calculate the frequency again in order to send back for each couple only
                                // the frequency different from the base one
                                calculatedFrequency = 87500 + (100 * DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[cnt2]);

                                currentFrequency = DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[cnt2];
                                
                                if (DABMW_AF_ENTRY_FILLER_CODE_VALUE != currentFrequency &&
                                    calculatedFrequency != baseFrequency)
                                {

                                    /* EPR CHANGE */
                                    // do not provide back the frequency if already in

                                    vl_freqAlreadyStored = false;
                                    for (vl_index = 0; vl_index < storageIndex; vl_index++)
                                    {
                                        if (currentFrequency == *(localPtr + vl_index))
                                        {
                                            vl_freqAlreadyStored = true;
                                            break;
                                        }
                                        
                                    }
                                
                                    if ((storageIndex < localMax) && (false == vl_freqAlreadyStored))
                                    {                                    
                                        *(localPtr + storageIndex) = DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[cnt2]; 

                                        storageIndex++;

                                  }
                                }

                                cnt2++;
                            }

                            DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].sendStatus = DABMW_STORAGE_STATUS_IS_RETRIEVED;

                            // Return the number of frequency retrieved that is the number of available
                            // frequencies minus the number of replicated base frequency. With this method the 
                            // radio looses the national/regional information but this is acceptable
                            res = storageIndex;
                        }
                    }
                }

        /* EPR CHANGE */ 
            /* same processing whatever the baseFreq parameter
 
                      }
                     else
                        {
                            // The base frequency has been passed
                            // Calculate the current frequency
                            calculatedFrequency = 87500 + (100 * DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[0]);
                            
                            if (baseFreq == calculatedFrequency)
                            {                        
                                OSAL_pvMemoryCopy ((tPVoid)localPtr, (tPVoid)&DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].af[0], 
                                    DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].afNum);

                                DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].sendStatus = DABMW_STORAGE_STATUS_IS_RETRIEVED;
                                
                                res += DABMW_amfmLandscape[piSlot].afDb.afEntries[cnt].afNum;

                                // Break the outermost loop
                                cnt = DABMW_AF_LIST_BFR_LEN;

                                break;
                            }           
                         }
                   */
           /* END EPR CHANGE */  
        }            
    }
   
    return res;
}

tSInt DABMW_GetAmFmServicesList (tPU8 dstPtr)
{
    tSInt rspLen = 0;
    tSInt cnt;

    for (cnt  = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
    {
        // Check if the slot is used
        if (DABMW_STORAGE_STATUS_IS_USED == DABMW_amfmLandscape[cnt].status)
        {
            // Move the send status to RETRIEVED
            DABMW_amfmLandscape[cnt].sendStatus = DABMW_STORAGE_STATUS_IS_RETRIEVED;

            // Return the PI information
            *(dstPtr + rspLen) = ((DABMW_amfmLandscape[cnt].piValue >> (tU32)24) & (tU32)0xFF); rspLen++;
            *(dstPtr + rspLen) = ((DABMW_amfmLandscape[cnt].piValue >> (tU32)16) & (tU32)0xFF); rspLen++;
            *(dstPtr + rspLen) = ((DABMW_amfmLandscape[cnt].piValue >> (tU32)8)  & (tU32)0xFF); rspLen++;
            *(dstPtr + rspLen) = ((DABMW_amfmLandscape[cnt].piValue >> (tU32)0)  & (tU32)0xFF); rspLen++;

            // Return the frequency information
            *(dstPtr + rspLen) = ((DABMW_amfmLandscape[cnt].frequency >> (tU32)24) & (tU32)0xFF); rspLen++;
            *(dstPtr + rspLen) = ((DABMW_amfmLandscape[cnt].frequency >> (tU32)16) & (tU32)0xFF); rspLen++;
            *(dstPtr + rspLen) = ((DABMW_amfmLandscape[cnt].frequency >> (tU32)8)  & (tU32)0xFF); rspLen++;
            *(dstPtr + rspLen) = ((DABMW_amfmLandscape[cnt].frequency >> (tU32)0)  & (tU32)0xFF); rspLen++;
        }
    } 

    return rspLen;
}

tSInt DABMW_GetAmFmSpecificServiceData (tPU8 dstPtr, tU32 piValue, tU32 freqValue)
{
    tSInt rspLen = 0;
    tSInt cnt, cnt2;

    if (DABMW_INVALID_DATA == piValue && DABMW_INVALID_FREQUENCY == freqValue)
    {
        // The indexing is not possible
        return rspLen;
    }

    // Index by PI
    if (DABMW_INVALID_DATA != piValue)
    {
        for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {
            if (DABMW_STORAGE_STATUS_IS_USED == DABMW_amfmLandscape[cnt].status &&
                DABMW_amfmLandscape[cnt].piValue == piValue)
            {
                // Return the PI information
                *(dstPtr + rspLen) = ((DABMW_amfmLandscape[cnt].piValue >> (tU8)24) & (tU8)0xFF); rspLen++;
                *(dstPtr + rspLen) = ((DABMW_amfmLandscape[cnt].piValue >> (tU8)16) & (tU8)0xFF); rspLen++;
                *(dstPtr + rspLen) = ((DABMW_amfmLandscape[cnt].piValue >> (tU8)8)  & (tU8)0xFF); rspLen++;
                *(dstPtr + rspLen) = ((DABMW_amfmLandscape[cnt].piValue >> (tU8)0)  & (tU8)0xFF); rspLen++;
                
                // Return the PS information
                for (cnt2 = 0; cnt2 < DABMW_RDS_PS_LENGTH; cnt2++)
                {
                    *(dstPtr + rspLen) = DABMW_amfmLandscape[cnt].label[cnt2]; rspLen++;
                }

                return rspLen;
            }
        }
    }

    // Index by FREQ
    if (DABMW_INVALID_FREQUENCY != freqValue)
    {
        for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {        
            if (DABMW_STORAGE_STATUS_IS_USED == DABMW_amfmLandscape[cnt].status &&
                DABMW_amfmLandscape[cnt].frequency == freqValue)
            {
                // Return the PS information
                for (cnt2 = 0; cnt2 < DABMW_RDS_PS_LENGTH; cnt2++)
                {
                    *(dstPtr + rspLen) = DABMW_amfmLandscape[cnt].label[cnt2]; rspLen++;
                }

                return rspLen;
            }
        }
    }
    
    return rspLen;        
}

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
tBool DABMW_AmFmLandscapeRegisterForPiAtFreq (tU32 frequency, tPVoid paramPtr, DABMW_AmFmRdsCallbackTy callback, tBool reuse)
{
    tBool res = false;
    tSInt cnt;

    // Check if an already used slot must be re-used for the same callback.
    // This can happen when an application would like to register for a new
    // frequency using the same handler
    if (true == reuse)
    {
        for (cnt = 0; cnt < DABMW_RDS_MAX_NUM_REGISTERED_CALLBACK; cnt++)
        {
            if (callback == DABMW_amFmRdsCallbackInformation[cnt].clbkFnct)
            {            
                // Set the slot as used
                DABMW_amFmRdsCallbackInformation[cnt].usedSlot = true;

                // Set the requested frequency, if only new PI shall generate a call
                // and the callback to call
                DABMW_amFmRdsCallbackInformation[cnt].registeredFrequency = frequency;
                DABMW_amFmRdsCallbackInformation[cnt].clbkFnct = callback;
                DABMW_amFmRdsCallbackInformation[cnt].onlyNewPi = true; 
                DABMW_amFmRdsCallbackInformation[cnt].onlyNewPs = false;
                DABMW_amFmRdsCallbackInformation[cnt].onlyNewAfList = false; 
                DABMW_amFmRdsCallbackInformation[cnt].parPtr = paramPtr;

                // Signal that all is fine
                res = true;

                break;
            }
        }

    }

	// In case an already used place has not been found then try to store it
    // in a new slot
   if (false == res)
   {
 	  // Search for a free slot    
      for (cnt = 0; cnt < DABMW_RDS_MAX_NUM_REGISTERED_CALLBACK; cnt++)
      {
      	if (false == DABMW_amFmRdsCallbackInformation[cnt].usedSlot)
        {            
        	// Set the slot as used
            DABMW_amFmRdsCallbackInformation[cnt].usedSlot = true;

            // Set the requested frequency, if only new PI shall generate a call
            // and the callback to call
            DABMW_amFmRdsCallbackInformation[cnt].registeredFrequency = frequency;
            DABMW_amFmRdsCallbackInformation[cnt].clbkFnct = callback;
            DABMW_amFmRdsCallbackInformation[cnt].onlyNewPi = true; 
            DABMW_amFmRdsCallbackInformation[cnt].onlyNewPs = false;
            DABMW_amFmRdsCallbackInformation[cnt].onlyNewAfList = false; 
            DABMW_amFmRdsCallbackInformation[cnt].parPtr = paramPtr;

            // Signal that all is fine
            res = true;

            break;
          }
        }
	}

    
    return res;
}

tBool DABMW_AmFmLandscapeRegisterForPsAtFreq (tU32 frequency, tPVoid paramPtr, DABMW_AmFmRdsCallbackTy callback, tBool reuse)
{
    tBool res = false;
    tSInt cnt;

    // Check if an already used slot must be re-used for the same callback.
    // This can happen when an application would like to register for a new
    // frequency using the same handler
    if (true == reuse)
    {
        for (cnt = 0; cnt < DABMW_RDS_MAX_NUM_REGISTERED_CALLBACK; cnt++)
        {
            if (callback == DABMW_amFmRdsCallbackInformation[cnt].clbkFnct)
            {            
                // Set the slot as used
                DABMW_amFmRdsCallbackInformation[cnt].usedSlot = true;

                // Set the requested frequency, if only new PI shall generate a call
                // and the callback to call
                DABMW_amFmRdsCallbackInformation[cnt].registeredFrequency = frequency;
                DABMW_amFmRdsCallbackInformation[cnt].clbkFnct = callback;
                DABMW_amFmRdsCallbackInformation[cnt].onlyNewPi = false; 
                DABMW_amFmRdsCallbackInformation[cnt].onlyNewPs = true;
                DABMW_amFmRdsCallbackInformation[cnt].onlyNewAfList = false; 
                DABMW_amFmRdsCallbackInformation[cnt].parPtr = paramPtr;

                // Signal that all is fine
                res = true;

                break;
            }
        }
    }

	// if still not found
	if (false == res)
    {
        // Search for a free slot    
        for (cnt = 0; cnt < DABMW_RDS_MAX_NUM_REGISTERED_CALLBACK; cnt++)
        {
            if (false == DABMW_amFmRdsCallbackInformation[cnt].usedSlot)
            {            
                // Set the slot as used
                DABMW_amFmRdsCallbackInformation[cnt].usedSlot = true;

                // Set the requested frequency, if only new PI shall generate a call
                // and the callback to call
                DABMW_amFmRdsCallbackInformation[cnt].registeredFrequency = frequency;
                DABMW_amFmRdsCallbackInformation[cnt].clbkFnct = callback;
                DABMW_amFmRdsCallbackInformation[cnt].onlyNewPi = false; 
                DABMW_amFmRdsCallbackInformation[cnt].onlyNewPs = true;
                DABMW_amFmRdsCallbackInformation[cnt].onlyNewAfList = false; 
                DABMW_amFmRdsCallbackInformation[cnt].parPtr = paramPtr;

                // Signal that all is fine
                res = true;

                break;
            }
        }
    }
       
    return res;
}

tBool DABMW_AmFmLandscapeRegisterForAfListAtFreq (tU32 frequency, DABMW_afStorageTy *paramPtr, DABMW_AmFmRdsCallbackTy callback)
{
    tBool res = false;
    tSInt cnt;

    for (cnt = 0; cnt < DABMW_RDS_MAX_NUM_REGISTERED_CALLBACK; cnt++)
    {
        if (false == DABMW_amFmRdsCallbackInformation[cnt].usedSlot)
        {            
            // Set the slot as used
            DABMW_amFmRdsCallbackInformation[cnt].usedSlot = true;

            // Set the requested frequency, if only new PI shall generate a call
            // and the callback to call
            DABMW_amFmRdsCallbackInformation[cnt].registeredFrequency = frequency;
            DABMW_amFmRdsCallbackInformation[cnt].clbkFnct = callback;
            DABMW_amFmRdsCallbackInformation[cnt].onlyNewAfList = true;    
            DABMW_amFmRdsCallbackInformation[cnt].onlyNewPi = false;
            DABMW_amFmRdsCallbackInformation[cnt].onlyNewPs = false;
            DABMW_amFmRdsCallbackInformation[cnt].parPtr = (tPVoid)paramPtr;

            // Signal that all is fine
            res = true;

            break;
        }
    }
    
    return res;
}

tBool DABMW_AmFmLandscapeDeRegister (DABMW_AmFmRdsCallbackTy callback, tU32 frequency)
{
    tBool res = false;
    tSInt cnt;

    for (cnt = 0; cnt < DABMW_RDS_MAX_NUM_REGISTERED_CALLBACK; cnt++)
    {
        if (callback == DABMW_amFmRdsCallbackInformation[cnt].clbkFnct &&
            frequency == DABMW_amFmRdsCallbackInformation[cnt].registeredFrequency)
        {            
            // Set the requested frequency, if only new PI shall generate a call
            // and the callback to call
            DABMW_amFmRdsCallbackInformation[cnt].registeredFrequency = DABMW_INVALID_FREQUENCY;
            DABMW_amFmRdsCallbackInformation[cnt].clbkFnct = (DABMW_AmFmRdsCallbackTy)NULL;
            DABMW_amFmRdsCallbackInformation[cnt].onlyNewPi = false;  
            DABMW_amFmRdsCallbackInformation[cnt].onlyNewPs = false; 
            DABMW_amFmRdsCallbackInformation[cnt].onlyNewAfList = false; 
            DABMW_amFmRdsCallbackInformation[cnt].parPtr = NULL;
            
            // Set the slot as free
            DABMW_amFmRdsCallbackInformation[cnt].usedSlot = false;

            // Signal that all is fine
            res = true;

            break;
        }
    }
    
    return res;
}
#endif // CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL

tU32 DABMW_AmFmLandscapeSearchForPi (tU32 piVal, tU32 freq)
{
    tU32 piSlot = DABMW_INVALID_DATA;
    tSInt cnt;

    // If the passed frequency is invalid then search for first available PI
    if (DABMW_INVALID_FREQUENCY == freq)
    {
        // Search the PI to find if a slot has been already allocated to it
        for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {
            if (DABMW_amfmLandscape[cnt].piValue == piVal && 
                DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_amfmLandscape[cnt].status)
            {
                piSlot = cnt;

                break;
            }
        }
    }
    else
    {
        // Search the PI to find if a slot has been already allocated to it
        for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {
            if (DABMW_amfmLandscape[cnt].piValue == piVal && 
                DABMW_amfmLandscape[cnt].frequency == freq && 
                DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_amfmLandscape[cnt].status)
            {
                piSlot = cnt;

                break;
            }
        }
    }

    return piSlot;
}

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
tSInt DABMW_AmFmNumberOfStations (tVoid)
{
    return DABMW_amfmLandscapeGlobals.stationNumber;
} 
#endif

static tU32 DABMW_AmFmLandscapeSearchForFreq (tU32 frequency)
{
    tU32 slot = DABMW_INVALID_DATA;
    tSInt cnt;

    // Search the PI to find if a slot has been already allocated to it
    for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
    {
        if (DABMW_amfmLandscape[cnt].frequency == frequency && 
            DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_amfmLandscape[cnt].status)
        {
            slot = cnt;

            break;
        }
    }

    return slot;
}

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
static tVoid DABMW_RdsCheckForRegisteredUsers (tU32 piVal, tBool isAPs, tU32 frequency)
{   
    tSInt cnt;
    
    // Check if some client must be called
    for (cnt = 0; cnt < DABMW_RDS_MAX_NUM_REGISTERED_CALLBACK; cnt++)
    {
        if (frequency == DABMW_amFmRdsCallbackInformation[cnt].registeredFrequency &&
            (DABMW_AmFmRdsCallbackTy)NULL != DABMW_amFmRdsCallbackInformation[cnt].clbkFnct)
        {            
            if (true == isAPs && true == DABMW_amFmRdsCallbackInformation[cnt].onlyNewPs)
            {                
                // Call the client
                DABMW_amFmRdsCallbackInformation[cnt].clbkFnct (piVal, frequency, DABMW_amFmRdsCallbackInformation[cnt].parPtr);
            }
            else if (false == isAPs && true == DABMW_amFmRdsCallbackInformation[cnt].onlyNewPi)
            {
                // Call the client
                DABMW_amFmRdsCallbackInformation[cnt].clbkFnct (piVal, frequency, DABMW_amFmRdsCallbackInformation[cnt].parPtr);
            }
        }
    }    
}

static tVoid DABMW_RdsCheckForAfListRegisteredUsers (tU32 piVal, tU32 frequency, DABMW_afStorageTy *dataPtr)
{   
    tSInt cnt;
    
    // Check if some client must be called
    for (cnt = 0; cnt < DABMW_RDS_MAX_NUM_REGISTERED_CALLBACK; cnt++)
    {
        if (frequency == DABMW_amFmRdsCallbackInformation[cnt].registeredFrequency &&
            (DABMW_AmFmRdsCallbackTy)NULL != DABMW_amFmRdsCallbackInformation[cnt].clbkFnct)
        {            
            if (true == DABMW_amFmRdsCallbackInformation[cnt].onlyNewAfList)
            {
                // Copy the data
                OSAL_pvMemoryCopy ((tPVoid)DABMW_amFmRdsCallbackInformation[cnt].parPtr, (tPCVoid)dataPtr, sizeof (DABMW_afStorageTy));
                
                // Call the client
                DABMW_amFmRdsCallbackInformation[cnt].clbkFnct (piVal, frequency, DABMW_amFmRdsCallbackInformation[cnt].parPtr);
            }
        }
    }    
}
#endif // CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL

static tVoid DABMW_EraseAmFmDb (tVoid)
{
    tU32 tmpSize;
    tSInt cnt;
    
    tmpSize = (tU32)sizeof(DABMW_amfmLandscapeTy) * (tU32)DABMW_AMFM_LANSCAPE_SIZE; 
    
    // Init the landscape db
    OSAL_pvMemorySet ((tPVoid)&DABMW_amfmLandscape[0], 0x00, tmpSize); 

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
    DABMW_amfmLandscapeGlobals.stationNumber = 0;
    DABMW_amfmLandscapeGlobals.stationNumberWithRds = 0;
#endif

    for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
    {  
        DABMW_amfmLandscape[cnt].frequency = DABMW_INVALID_FREQUENCY;
    }
}

/* EPR CHANGE 
*
* Add procedure to get all frequency existing for a given PI (ie a list of and not only one)
*/
tSInt DABMW_AmFmLandscapeSearchForPI_FreqList (tPU32 dstPtr, tU32 piValue, tU8 vI_ptrLen)
{
    tSInt rspLen = 0;
    tSInt cnt;

    if (DABMW_INVALID_DATA == piValue)
    {
        // The indexing is not possible
        return rspLen;
    }

    // Index by PI

   	for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {
            if (DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_amfmLandscape[cnt].status &&
                DABMW_amfmLandscape[cnt].piValue == piValue)
            {
                // Return the PI information
                // we may check this fits in the memory
                // 
                if (rspLen < vI_ptrLen)
                    {
                    *(dstPtr + rspLen) = DABMW_amfmLandscape[cnt].frequency;
				
				    rspLen += 1;
                    }
                else
                    {
                        // buffer is full, stop now....
                        break;
                    }
              
            }
        }
    
    
    return rspLen;        
}

/* Add procedure to retrive number of frequency existing for a given PI 
*/
tSInt DABMW_AmFmLandscapeSearchForPI_GetNumber(tU32 piValue)
{
    tSInt rspLen = 0;
    tSInt cnt;

    if (DABMW_INVALID_DATA == piValue)
    {
        // The indexing is not possible
        return rspLen;
    }

    // Index by PI

   	for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {
            if (DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_amfmLandscape[cnt].status &&
                DABMW_amfmLandscape[cnt].piValue == piValue)
            {
                
				rspLen += 1;
              
            }
        }
    
    
    return rspLen;        
}

/* Procedure to check the PI validity*/
tBool DABMW_AmFmLandscapeGetPiValidity(tU32 vI_freq, OSAL_tMSecond vI_piValidityDuration)
{
    tU32 vl_piSlot = DABMW_INVALID_DATA;
	tBool vl_PiValidity = false;
	OSAL_tMSecond vl_systemTime;
    
    // Get the current system time
    vl_systemTime = OSAL_ClockGetElapsedTime();

	// search for the PI slot
	vl_piSlot = DABMW_AmFmLandscapeSearchForFreq(vI_freq);
	
    // If the passed frequency is invalid then search for first available PI
    if (DABMW_INVALID_DATA != vl_piSlot)
    	{
      	if ((vl_systemTime - DABMW_amfmLandscape[vl_piSlot].pi_StoredTime) < vI_piValidityDuration)
			{
			vl_PiValidity = true;
			}
        }

    return vl_PiValidity;
}

/* END EPR addition 
*/

tU32 DABMW_AmFmLandscapeGetPiFromFreq (tU32 vI_freq)
{
    tU32 vI_PiValue = DABMW_INVALID_RDS_PI_VALUE;
    tU32 piSlot;

    // Search the PI to find if a slot has been already allocated to it
    piSlot = DABMW_AmFmLandscapeSearchForFreq(vI_freq);    

    // If the PI is available, store the PS after checking if it is different
    if (DABMW_INVALID_DATA != piSlot)
    {
        vI_PiValue = DABMW_amfmLandscape[piSlot].piValue;    
    }    

    return vI_PiValue;
}

tU32 DABMW_AmFmLandscapeGetValidPiFromFreq(tU32 vI_freq, OSAL_tMSecond vI_piValidityDuration)
{
    tU32 vI_PiValue = DABMW_INVALID_RDS_PI_VALUE;
    tU32 vl_piSlot;
	OSAL_tMSecond vl_systemTime;
    
    // Get the current system time
    vl_systemTime = OSAL_ClockGetElapsedTime();

    // Search the PI to find if a slot has been already allocated to it
    vl_piSlot = DABMW_AmFmLandscapeSearchForFreq(vI_freq);    

    // If the PI is available, store the PS after checking if it is different
    if (DABMW_INVALID_DATA != vl_piSlot)
    {
    	/* Freq found
	    	* check if PI still valid
	    	* else return invalid PI
	    	*/
		if ((vl_systemTime - DABMW_amfmLandscape[vl_piSlot].pi_StoredTime) < vI_piValidityDuration)
			{
				vI_PiValue = DABMW_amfmLandscape[vl_piSlot].piValue;    
			}

		// change : if PI is 0xFFFF, consider also it has invalid
		if (DABMW_INVALID_DATA_U16 == DABMW_amfmLandscape[vl_piSlot].piValue)
			{
			vI_PiValue = DABMW_INVALID_DATA;
			}
 
    }    



    return vI_PiValue;
}

tSInt DABMW_AmFmLandscapeSearchForValidPI_FreqList (tPU32 dstPtr, tU32 piValue, OSAL_tMSecond vI_piValidityDuration)
{
    tSInt rspLen = 0;
    tSInt cnt;
	OSAL_tMSecond vl_systemTime;
    
    // Get the current system time
    vl_systemTime = OSAL_ClockGetElapsedTime();

    if (DABMW_INVALID_DATA == piValue)
    {
        // The indexing is not possible
        return rspLen;
    }

    // Index by PI

   	for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {
            if ((DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_amfmLandscape[cnt].status)
				&& (DABMW_amfmLandscape[cnt].piValue == piValue)
				&& ((vl_systemTime - DABMW_amfmLandscape[cnt].pi_StoredTime) < vI_piValidityDuration)
				)
            {
                // Return the PI information
                *(dstPtr + rspLen) = DABMW_amfmLandscape[cnt].frequency;
				
				rspLen += 1;             
            }
        }
    
    
    return rspLen;        
}

/* Procedure to update the PI stored time
*/
tSInt DABMW_AmFmLandscapeUpdatePiStoreTime(tU32 piVal, tU32 freq)
{
    tSInt res = OSAL_ERROR;
    tU32 cnt;

    // If the frequency is invalid return with error
    if (0 == freq || DABMW_INVALID_FREQUENCY == freq)
    {
        return res;
    }

    // Search the PI to find if a slot has been already allocated to it

	/* EPR change
	* get the found cnt if reuse of same slot : same freq / same PI
	*/
	cnt = DABMW_AmFmLandscapeSearchForPi (piVal, freq);
    if (DABMW_INVALID_DATA != cnt)
    	{
		DABMW_amfmLandscape[cnt].pi_StoredTime = OSAL_ClockGetElapsedTime();
		res = OSAL_OK;
		}
	
    return res;
}
/* END EPR */


// EPR CHANGE 
// ADD Lanscape functionnality
//---------------------
tU32 DABMW_AmFmLandscapeGetNbValid_FreqList (OSAL_tMSecond vI_piValidityDuration)
{
    tSInt rspLen = 0;
    tSInt cnt;
	OSAL_tMSecond vl_systemTime;
    
    // Get the current system time
    vl_systemTime = OSAL_ClockGetElapsedTime();

	// loop on the landscape and store how many valid
	//
   	for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {
            if ((DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_amfmLandscape[cnt].status)
				&& ((vl_systemTime - DABMW_amfmLandscape[cnt].pi_StoredTime) < vI_piValidityDuration)
				)
            {
                // Return the PI information		
				rspLen += 1;             
            }
        }
    
    return rspLen;        
}

/* 
* Add procedure to get all frequency existing for a given PI (ie a list of and not only one)
*/
tU32 DABMW_AmFmLandscapeGetValid_FreqList (EtalRdsLandscapeExternalInfo *dstPtr, tU8 vI_ptrLen, OSAL_tMSecond vI_piValidityDuration)
{
    tSInt rspLen = 0;
    tSInt cnt;
	OSAL_tMSecond vl_systemTime;
    
    // Get the current system time
    vl_systemTime = OSAL_ClockGetElapsedTime();

   	for (cnt = 0; cnt < DABMW_AMFM_LANSCAPE_SIZE; cnt++)
        {

			if ((DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_amfmLandscape[cnt].status)
				&& ((vl_systemTime - DABMW_amfmLandscape[cnt].pi_StoredTime) < vI_piValidityDuration)
				)			              {
                // Return the PI information
                // we may check this fits in the memory
                // 
                if (rspLen < vI_ptrLen)
                    {
                    dstPtr[rspLen].m_frequency = DABMW_amfmLandscape[cnt].frequency;
					dstPtr[rspLen].m_piValue = DABMW_amfmLandscape[cnt].piValue;
					// init the label
					memset(dstPtr[rspLen].m_label, 0x00, ETAL_DEF_MAX_PS_LEN+1);
					memcpy(dstPtr[rspLen].m_label, DABMW_amfmLandscape[cnt].label, DABMW_RDS_PS_LENGTH);				
				    rspLen += 1;
                    }
                else
                    {
                        // buffer is full, stop now....
                        break;
                    }            
            }
        }
    
    
    return rspLen;        
}

tPU8 DABMW_AmFmLandscapeGetPsFromFreq (tU32 vI_freq)
{
    tPU8 vl_Ps = NULL;
    tU32 vl_Slot = DABMW_INVALID_DATA;

    // Search the freq to find if a slot has been already allocated to it
    vl_Slot = DABMW_AmFmLandscapeSearchForFreq(vI_freq);    

    // If the PI is available, store the PS after checking if it is different
    if (DABMW_INVALID_DATA != vl_Slot)
    {
        vl_Ps = DABMW_amfmLandscape[vl_Slot].label;    

		// check if it is not empty 
		// ie if 1st bit is 0x00 => it is empty
		if (DABMW_amfmLandscape[vl_Slot].label[0] == 0x00)
		{
			vl_Ps = NULL;
		}
    }    

	// check if
    return vl_Ps;
}


//#ifdef __cplusplus
//}
//#endif

// End of file
#endif // ETALTML_HAVE_AMFMLANDSCAPE
