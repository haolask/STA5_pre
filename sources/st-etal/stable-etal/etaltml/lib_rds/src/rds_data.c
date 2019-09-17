//!
//!  \file      rds_data.c
//!  \brief     <i><b> RDS decoder </b></i>
//!  \details   RDS decoder functionality
//!  \author    Alberto Saviotti
//!  \author    (original version) Alberto Saviotti
//!  \version   1.0
//!  \date      2011.09.20
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

//#include "system_app.h"

#include "rds_af_data.h"

#include "rds_eon_data.h"

#include "rds_landscape.h"

#include "rds_data.h"

#include "rds_strategy.h"

// Uncomment the next 3 lines to get traces from the RDS decoder
//#undef TR_LEVEL_COMPONENT
//#define TR_LEVEL_COMPONENT TR_LEVEL_SYSTEM_MIN
//#define CONFIG_ENABLE_CLASS_APP_DABMW TR_LEVEL_COMPONENT

/* EPR change : add a include for purge functions */
//#include "rds_comm.h"
/* END EPR CHANGE */

//
// RDS Table for Groups can be retrieved from GROUP B 5 most significant bits
//
// GROUP0A  : Basic tuning and switching information only
// GROUP0B  : Basic tuning and switching information only
// GROUP1A  : Programme Item Number and slow labelling codes only 
// GROUP1B  : Programme Item Number
// GROUP2A  : RadioText only 
// GROUP2B  : RadioText only
// GROUP3A  : Applications Identification for ODA only
// GROUP3B  : Open Data Applications
// GROUP4A  : Clock-time and date only
// GROUP4B  : Open Data Applications
// GROUP5A  : Transparent Data Channels (32 channels) or ODA
// GROUP5B  : Transparent Data Channels (32 channels) or ODA 
// GROUP6A  : In House applications or ODA
// GROUP6B  : In House applications or ODA
// GROUP7A  : Radio Paging or ODA
// GROUP7B  : Open Data Applications
// GROUP8A  : Traffic Message Channel or ODA
// GROUP8B  : Open Data Applications
// GROUP9A  : Emergency Warning System or ODA
// GROUP9B  : Open Data Applications
// GROUP10A : Programme Type Name
// GROUP10B : Open Data Applications
// GROUP11A : Open Data Applications
// GROUP11B : Open Data Applications
// GROUP12A : Open Data Applications
// GROUP12B : Open Data Applications
// GROUP13A : Enhanced Radio Paging or ODA
// GROUP13B : Open Data Applications
// GROUP14A : Enhanced Other Networks information only
// GROUP14B : Enhanced Other Networks information only 
// GROUP15A : Defined in RBDS [15] only
// GROUP15B : Fast switching information only

#define DABMW_RDS_GROUP_A               0x00
#define DABMW_RDS_GROUP_B               0x01
#define DABMW_RDS_GROUP_C               0x02
#define DABMW_RDS_GROUP_D               0x03

#define DABMW_RDS_GROUP_NONE            0xFF

#define DABMW_RDS_DATA_NB_MAX_PI_HISTORY	10
#define DABMW_RDS_INIT_PI				0x0000


typedef union
{
    tU8 byte[2];

    tU16 dbyte;
} DABMW_charIntTy;

typedef struct 
{
    DABMW_storageSourceTy source;
        
    tU8 BlockCnt;
    tU8 LastBlockCnt;

    tU8 lastGroup;

    tU32 BlockA;
    tU32 BlockB;
    tU32 BlockC;
    tU32 BlockCp;
    tU32 BlockD;
    tU32 BlockLastA;
    tU32 BlockLastB;
    tU32 BlockLastC;
    tU32 BlockLastCp;
    tU32 BlockLastD;

    tU8 RDSData[4];

    tU32 F_GetOneGroup: 1;
    tU32 F_GetBlockA:   1;
    tU32 F_GetBlockB:   1;
    tU32 F_GetBlockC:   1;
    tU32 F_GetBlockCp:  1;
    tU32 F_GetBlockD:   1;
    tU32 F_BlockA:      1;
    tU32 F_BlockB:      1;
    tU32 F_BlockC:      1;
    tU32 F_BlockCp:     1;
    tU32 F_BlockD:      1;
    tU32 F_RDSStation:  1;

    tU32 F_RDSInt:      1;
} DABMW_rdsTy;

#if 0
typedef struct
{
    tU32 value;
    tU32 last;
    tU32 lastValid;
    tBool newValue;
} DABMW_rdsDataTy;
#endif

typedef struct
{
    tU8 value;
    
    // Following 3 values are returned in a bunch so they must stay in the
    // order below
    tU8 lastValid;
    tChar data8String[8];
    tChar data16String[16];
    
    DABMW_storageStatusEnumTy newValue;
} DABMW_ptyDataTy;

typedef struct
{
    tU8 value;
    tU8 lastValid;
    DABMW_storageStatusEnumTy newValue;
} DABMW_msDataTy;

typedef struct
{
    tU8 value;
    tU8 lastValid;
    DABMW_storageStatusEnumTy newValue;
} DABMW_diDataTy;

typedef struct
{
    tU32 value;
    tU32 lastValid;
    tU32 backupValue;
	tU32 lastValidHistory[DABMW_RDS_DATA_NB_MAX_PI_HISTORY];
    tU8 confidenceThr;
    DABMW_storageStatusEnumTy newValue;
} DABMW_piDataTy;

typedef struct
{
    union
    {
        struct
        {
            tU8 taValue:    1;
            tU8 tpValue:    1;
            tU8 padding0:   6;
        } fields;
        
        tU8 value;
    } data;

    union
    {
        struct
        {
            tU8 taValue:    1;
            tU8 tpValue:    1;
            tU8 padding0:   6;
        } fields;
        
        tU8 value;
    } lastData;

    tU8 confidenceThr;
    DABMW_storageStatusEnumTy newValue;
} DABMW_taTpDataTy;

#if 0
typedef enum
{
    DABMW_VALUE_STATUS_IS_STORED
} DABMW_valueStatusEnumTy;
#endif

typedef struct
{
    tU8 label[DABMW_RDS_PS_LENGTH];
    tU8 labelBackup[DABMW_RDS_PS_LENGTH]; 
    tBool get0;
    tBool get1;
    tBool get2;
    tBool get3;
    DABMW_storageStatusEnumTy newValue; 
} DABMW_psDataTy;

typedef struct
{
    DABMW_storageStatusEnumTy afNew;
#ifndef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
    tBool afList;
    tBool afMethodB;
    
    tU8 af[DABMW_AF_LIST_BFR_LEN];
    tU8 afNum;
    tU8 afMethod[2];
#endif // !CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
} DABMW_afTy;

typedef struct
{
    tBool blocksComplete;
    
    tBool availBlock_A;
    tU32 block_A;
    tBool availBlock_B;
    tU32 block_B;
    tBool availBlock_C;
    tU32 block_C;
    tBool availBlock_Cp;
    tU32 block_Cp;
    tBool availBlock_D;
    tU32 block_D;

	/* EPR change
	* add boolean to identify if block has already been used to get PI information
	* when parsing the data..
	* this will avoid to use twice the same block to retrieve PI....
	*/
	tBool availBlock_A_used;
  	tBool availBlock_B_used;
    tBool availBlock_C_used;
    tBool availBlock_Cp_used;
    tBool availBlock_D_used;
	/* END EPR CHANGE */

} DABMW_blockInfoTy;

typedef struct
{
    tBool newCt;
    
    tU8 hour;
    tU8 min;
    tU8 offset;

    tU32 mjd;
} DABMW_timeTy; 

typedef struct
{
    //tBool newRt;
    DABMW_storageStatusEnumTy newValue;
    
    tBool rtabFlag;

    tSInt lastSegNum;
    tU64 blockPresent;
    
    tU8 rt[64];  
} DABMW_rtTy;

typedef struct
{
    tU8 pty:     5;
    tU8 tp:      1;
    tU8 ta:      1;
    tU8 ms:      1;

    tU8 ptyFlag: 5;
    tU8 tpFlag:  1;
    tU8 taFlag:  1;
    tU8 msFlag:  1;    
} DABMW_ptyTpTaMsTy;

static const tChar DABMW_pty8CharTableTy[32][8] =
{
    "None    ",
    "News    ",
    "Affairs ",
    "Info    ",
    "Sport   ",
    "Educate ",
    "Drama   ",
    "Culture ",
    "Science ",
    "Varied  ",
    "Pop M   ",
    "Rock M  ",
    "Easy M  ",
    "Light M ",
    "Classics",
    "Other M ",
    "Weather ",
    "Finance ",
    "Children",
    "Social  ",
    "Religion",
    "Phone In",
    "Travel  ",
    "Leisure ",
    "Jazz    ",
    "Country ",
    "Nation M",
    "Oldies  ",
    "Folk M  ",
    "Document",
    "TEST    ",
    "Alarm ! "
};

static const tChar DABMW_pty16CharTableTy[32][16] =
{
    "None            ",
    "News            ",
    "Current Affairs ",
    "Information     ",
    "Sport           ",
    "Education       ",
    "Drama           ",
    "Cultures        ",
    "Science         ",
    "Varied Speech   ",
    "Pop Music       ",
    "Rock Music      ",
    "Easy Listening  ",
    "Light Classics M",
    "Serious Classics",
    "Other Music     ",
    "Weather & Metr  ",
    "Finance         ",
    "Children's Progs",
    "Social Affairs  ",
    "Religion        ",
    "Phone In        ",
    "Travel & Touring",
    "Leisure & Hobby ",
    "Jazz Music      ",
    "Country Music   ",
    "National Music  ",
    "Oldies Music    ",
    "Folk Music      ",
    "Documentary     ",
    "Alarm Test      ",
    "Alarm - Alarm ! "  
};

typedef struct
{
    DABMW_blockInfoTy DABMW_blockInfo;

    DABMW_rdsTy DABMW_rds;

    DABMW_piDataTy DABMW_pi;

    DABMW_ptyDataTy DABMW_pty;

    DABMW_msDataTy DABMW_ms;

    DABMW_diDataTy DABMW_diData;

    DABMW_taTpDataTy DABMW_taTp;

    DABMW_ptyTpTaMsTy DABMW_ptyTpTaMs;

    DABMW_psDataTy DABMW_ps;

    DABMW_timeTy DABMW_time;
    DABMW_timeTy DABMW_previousTime;

    tU8 DABMW_realGroup;

    DABMW_rtTy DABMW_rt;

    DABMW_afTy DABMW_af;

    tU8 DABMW_criticalDataThr;

	tBool DABMW_IsFastPiDetectionMode;

    tU32 DABMW_blockErrorRatio;
} DABMW_rdsInfoTy;

DABMW_rdsInfoTy DABMW_rdsData[DABMW_RDS_SOURCE_MAX_NUM];

static tVoid DABMW_RdsFetchBlockData (tSInt slot, tU32 data);

static tVoid DABMW_PushToLast (tSInt slot);

static tVoid DABMW_RdsSetBlockData (tSInt slot,
                                    tU8 bflags, tU32 blockA,
                                    tU32 blockB, tU32 blockC, 
                                    tU32 blockCp, tU32 blockD);

static tVoid DABMW_RdsExtrData (tSInt slot, DABMW_RDS_mwAppTy app);

static tVoid DABMW_CleanUpData (tSInt slot);

/* EPR CHANGE 
* add procedure for PI processing
*/
static tBool DABMW_RdsPi_Processing(tSInt slot, DABMW_RDS_mwAppTy app, tU32 curFreq);

/* END EPR CHANGE */

tVoid ETAL_RdsDataPurge(tSInt slot);

#ifdef ETAL_RDS_IMPORT
tVoid ETAL_RdsDataPurge(tSInt slot)
{
	// In general, the RDS DATA PURGE on the CMOS is not needed, because automatically done by STAR
	//

	// so for now, let assume it is correctly managed at right place by STAR and keep it empty
	//
	// else interface is  
 	// 	ETAL_RDSreset_CMOST

}
#endif

tVoid DABMW_RdsDataEvent (DABMW_storageSourceTy source, DABMW_eventTy event)
{
    tSInt slot;
	
    if (DABMW_EVENT_FREQUENCY_TUNED == event)
    {
        // Get the slot to use from the source
#ifdef ETAL_RDS_IMPORT
        slot = ETAL_receiverGetRDSSlot((ETAL_HANDLE)source);
#else
        slot = DABMW_RdsGetSlotFromSource (source);
#endif
        if (DABMW_INVALID_SLOT != slot)
        {
            // Clear PI
            DABMW_rdsData[slot].DABMW_pi.value = 0;
            DABMW_rdsData[slot].DABMW_pi.lastValid = 0;
            DABMW_rdsData[slot].DABMW_pi.backupValue = 0;
            DABMW_rdsData[slot].DABMW_pi.newValue = DABMW_STORAGE_STATUS_IS_EMPTY;
            DABMW_rdsData[slot].DABMW_pi.confidenceThr = 0; 

			OSAL_pvMemorySet(&(DABMW_rdsData[slot].DABMW_pi.lastValidHistory[0]), DABMW_RDS_INIT_PI,sizeof(tU32)*DABMW_RDS_DATA_NB_MAX_PI_HISTORY);
		

            // Clear all other data
            DABMW_CleanUpData (slot);
       

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
            // The db status must be changed because no value must be considered retrieved      
            DABMW_RdsAfDbOnChannelChange ();        
#endif

    		/* EPR change */
    		
    		/* I believe we should also reset the rds data, since not valid any more...
    		        */
    		
    		// Clear rds global variable
    		DABMW_rdsData[slot].DABMW_rds.BlockCnt = 0;
    		DABMW_rdsData[slot].DABMW_rds.LastBlockCnt = 0;
    		OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_rds, 0x00, sizeof(DABMW_rdsData[slot].DABMW_rds));
    		OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_blockInfo, 0x00, sizeof (DABMW_rdsData[slot].DABMW_blockInfo));
    		
    		/* clean the RDS buffer by reading data...
    		* so that at before new RDS data, current available one will not be used
    		*/
#ifdef ETAL_RDS_IMPORT
    		ETAL_RdsDataPurge(slot);
#else
    		DABMW_RdsDataPurge(slot);
#endif           
    		
    		/* END CHANGE EPR */
            // Change EPR : if slot is invalid : do nothing
        
        }
    }
#ifdef ETAL_RDS_IMPORT
    // TODO RDS disabling not yet supported by ETAL
#else
	else if (DABMW_EVENT_RDS_DISABLED == event)
    {
        // Get the slot to use from the source
#ifdef ETAL_RDS_IMPORT
        slot = ETAL_receiverGetRDSSlot((ETAL_HANDLE)source);
#else
        slot = DABMW_RdsGetSlotFromSource (source);
#endif

        if (DABMW_INVALID_SLOT != slot)
        {
            // Clear PI
            DABMW_rdsData[slot].DABMW_pi.value = 0;
            DABMW_rdsData[slot].DABMW_pi.lastValid = 0;
            DABMW_rdsData[slot].DABMW_pi.newValue = DABMW_STORAGE_STATUS_IS_EMPTY;
            DABMW_rdsData[slot].DABMW_pi.confidenceThr = 0; 

			OSAL_pvMemorySet(&(DABMW_rdsData[slot].DABMW_pi.lastValidHistory[0]), 0x00,sizeof(tU32)*DABMW_RDS_DATA_NB_MAX_PI_HISTORY);
			

            // Clear all other data
            DABMW_CleanUpData (slot);
       

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
            // The db status must be changed because no value must be considered retrieved      
            DABMW_RdsAfDbOnChannelChange ();        
#endif

    		/* EPR change */
    		
    		/* I believe we should also reset the rds data, since not valid any more...
    		        */
    		
    		// Clear rds global variable
    		DABMW_rdsData[slot].DABMW_rds.BlockCnt = 0;
    		DABMW_rdsData[slot].DABMW_rds.LastBlockCnt = 0;
    		OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_rds, 0x00, sizeof(DABMW_rdsData[slot].DABMW_rds));
    		OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_blockInfo, 0x00, sizeof (DABMW_rdsData[slot].DABMW_blockInfo));
    		
        
        }
		
       // no purge : this is disabled
       //
    }
#endif // ETAL_RDS_IMPORT
	
}

tVoid DABMW_RdsMain (DABMW_storageSourceTy source, tU32 data, tU32 error_ratio)
{
    tSInt slot;
    DABMW_RDS_mwAppTy app;
    
    // Get the slot to use from the source
#ifdef ETAL_RDS_IMPORT
    slot = ETAL_receiverGetRDSSlot((ETAL_HANDLE)source);
#else
    slot = DABMW_RdsGetSlotFromSource (source);
#endif

    if (DABMW_INVALID_SLOT == slot)
    {
        return;
    }

    // Save the source identifier
    DABMW_rdsData[slot].DABMW_rds.source = source;    
    DABMW_rdsData[slot].DABMW_blockErrorRatio = error_ratio;    

    // Calculate app from source
#ifdef ETAL_RDS_IMPORT
	app = (DABMW_RDS_mwAppTy) source;
#else
    if (DABMW_STORAGE_SOURCE_IS_RDS_1 == source)
    {
        app = DABMW_MAIN_RDS_APP;
    }
    else if (DABMW_STORAGE_SOURCE_IS_RDS_2 == source)
    {
        app = DABMW_SECONDARY_RDS_APP;
    }
    else
    {
        app = DABMW_NONE_APP;
    }
#endif


    DABMW_RdsFetchBlockData (slot, data);

    DABMW_RdsExtrData (slot, app);
}


tSInt DABMW_RdsGetSlotFromSource (DABMW_storageSourceTy source)
{

	tSInt slot = DABMW_INVALID_SLOT;

#ifdef ETAL_RDS_IMPORT
	slot = ETAL_receiverGetRDSSlot((ETAL_HANDLE)source);

#else

    tSInt slot = DABMW_INVALID_SLOT;

    if (DABMW_STORAGE_SOURCE_IS_RDS_1 == source)
    {
        slot = 0;
    }
    else if (DABMW_STORAGE_SOURCE_IS_RDS_2 == source)
    {
        slot = 1;
    }
    // EPR TMP CHANGE 
    // currently only 2 slots supported
    // so we should return slot 0 or 1 not 2
#ifdef CONFIG_TARGET_APP_AMFM_3RD_PATH_CONTROL
        else if (DABMW_STORAGE_SOURCE_IS_RDS_3 == source)
        {
            slot = 2;
        }
    /* END EPR TMP CHANGE */
#endif

#endif // ETAL_RDS_IMPORT

    return slot;



}

tSInt DABMW_RdsDataInit (tVoid)
{
    tSInt res = OSAL_OK;
    tSInt cnt;

    for (cnt = 0; cnt < DABMW_RDS_SOURCE_MAX_NUM; cnt++)
    {
        // Init globals
        DABMW_rdsData[cnt].DABMW_criticalDataThr = DABMW_RDS_DEFAULT_CRITDATA_THR;
		DABMW_rdsData[cnt].DABMW_IsFastPiDetectionMode = false;
		

        // Clear rds global variable
        DABMW_rdsData[cnt].DABMW_rds.BlockCnt = 0;
        DABMW_rdsData[cnt].DABMW_rds.LastBlockCnt = 0;
        OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[cnt].DABMW_blockInfo, DABMW_RDS_INIT_PI, sizeof (DABMW_rdsData[cnt].DABMW_blockInfo));

        // Clear PI
 
		/* EPR change */
		/* reset blocked used */
		DABMW_rdsData[cnt].DABMW_blockInfo.availBlock_A_used = false;
		DABMW_rdsData[cnt].DABMW_blockInfo.availBlock_B_used = false;
		DABMW_rdsData[cnt].DABMW_blockInfo.availBlock_C_used = false;
		DABMW_rdsData[cnt].DABMW_blockInfo.availBlock_D_used = false;
		
		/*	 
		* set last PI to invalid 
		*        DABMW_rdsData[cnt].DABMW_pi.lastValid = 0;
        DABMW_rdsData[cnt].DABMW_pi.value = 0;
		*/
		 DABMW_rdsData[cnt].DABMW_pi.lastValid = DABMW_INVALID_RDS_PI_VALUE;
		DABMW_rdsData[cnt].DABMW_pi.value = DABMW_INVALID_RDS_PI_VALUE; 
		/* END EPR CHANGE */

        DABMW_rdsData[cnt].DABMW_pi.lastValid = 0;
        DABMW_rdsData[cnt].DABMW_pi.newValue = DABMW_STORAGE_STATUS_IS_EMPTY;
        DABMW_rdsData[cnt].DABMW_pi.confidenceThr = 0; 

		OSAL_pvMemorySet(&(DABMW_rdsData[cnt].DABMW_pi.lastValidHistory[0]), DABMW_RDS_INIT_PI,sizeof(tU32)*DABMW_RDS_DATA_NB_MAX_PI_HISTORY);


        // Init previous time 
        DABMW_rdsData[cnt].DABMW_previousTime.newCt = DABMW_STORAGE_STATUS_IS_EMPTY;
        DABMW_rdsData[cnt].DABMW_previousTime.hour = 0;
        DABMW_rdsData[cnt].DABMW_previousTime.min = 0;
        DABMW_rdsData[cnt].DABMW_previousTime.mjd = 0;          

        // Clear all data but PI
        DABMW_CleanUpData (cnt);
    }
    
    return res;
}

// For DataSetup : manage the rds slot
tSInt DABMW_RdsDataSetup (DABMW_storageSourceTy source, tU8 criticalDataThr)
{
    tSInt res = OSAL_OK;
    tSInt cnt;
	tSInt vl_slot;

	if (DABMW_STORAGE_SOURCE_IS_RDS_ALL == source)
		{
		
    for (cnt = 0; cnt < DABMW_RDS_SOURCE_MAX_NUM; cnt++)
    {
        DABMW_rdsData[cnt].DABMW_criticalDataThr = criticalDataThr;
    }
		}	
	else
		{
		
		// Get the slot to use from the source
#ifdef ETAL_RDS_IMPORT
   	 vl_slot = ETAL_receiverGetRDSSlot((ETAL_HANDLE)source);
#else
    	vl_slot = DABMW_RdsGetSlotFromSource (source);
#endif
	

	    if (DABMW_INVALID_SLOT == vl_slot)
	    	{	
	    	return (OSAL_ERROR);
	    	}

		DABMW_rdsData[vl_slot].DABMW_criticalDataThr = criticalDataThr;
		}

    return res;
}


tSInt DABMW_RdsGetPty (tSInt slot, tVoid** rdsDataPtr, tBool forcedGet)
{
    tSInt res; 

    // Send back the PTY if it is available
    if (DABMW_STORAGE_STATUS_IS_STORED == DABMW_rdsData[slot].DABMW_pty.newValue)
    {
        *rdsDataPtr = &DABMW_rdsData[slot].DABMW_pty.lastValid;

        DABMW_rdsData[slot].DABMW_pty.newValue = DABMW_STORAGE_STATUS_IS_RETRIEVED;
        
        res = DABMW_RDS_PTY_LENGTH;
    }
    else if (DABMW_STORAGE_STATUS_IS_RETRIEVED == DABMW_rdsData[slot].DABMW_pty.newValue && 
             true == forcedGet)
    {
        *rdsDataPtr = &DABMW_rdsData[slot].DABMW_pty.lastValid;
        
        res = DABMW_RDS_PTY_LENGTH;
    }
    else
    {
        res = 0;
    }

    return res;
}

tSInt DABMW_RdsGetTime (tSInt slot, tVoid** rdsDataPtr, tBool forcedGet)
{
    tSInt res;  

    // Send back the PTY if it is available
    if (DABMW_STORAGE_STATUS_IS_STORED == DABMW_rdsData[slot].DABMW_time.newCt)
    {
        *rdsDataPtr = &DABMW_rdsData[slot].DABMW_time.hour;

        DABMW_rdsData[slot].DABMW_time.newCt = DABMW_STORAGE_STATUS_IS_RETRIEVED;
        
        res = DABMW_RDS_TIME_LENGTH;
    }
    else if (DABMW_STORAGE_STATUS_IS_RETRIEVED == DABMW_rdsData[slot].DABMW_time.newCt && 
             true == forcedGet)
    {
        *rdsDataPtr = &DABMW_rdsData[slot].DABMW_time.hour;
        
        res = DABMW_RDS_TIME_LENGTH;
    }
    else
    {
        res = 0;
    }

    return res;
}

tSInt DABMW_RdsGetPs (tSInt slot, tVoid** rdsDataPtr, tBool forcedGet)
{
    tSInt res;

    // Send back the PS if it is available
    if (DABMW_STORAGE_STATUS_IS_STORED == DABMW_rdsData[slot].DABMW_ps.newValue)
    {
        *rdsDataPtr = DABMW_rdsData[slot].DABMW_ps.labelBackup;

        DABMW_rdsData[slot].DABMW_ps.newValue = DABMW_STORAGE_STATUS_IS_RETRIEVED;
        
        res = DABMW_RDS_PS_LENGTH;
    }
    else if (DABMW_STORAGE_STATUS_IS_RETRIEVED == DABMW_rdsData[slot].DABMW_ps.newValue && 
             true == forcedGet)
    {
        *rdsDataPtr = DABMW_rdsData[slot].DABMW_ps.labelBackup;
        
        res = DABMW_RDS_PS_LENGTH;
    }
    else
    {
        res = 0;
    }

    return res;
}

tSInt DABMW_RdsGetPi (tSInt slot, tVoid** rdsDataPtr, tBool forcedGet)
{
    tSInt res; 

    // Send back the PI if it is available and verified
    if (DABMW_STORAGE_STATUS_IS_VERIFIED == DABMW_rdsData[slot].DABMW_pi.newValue)
    {
        *rdsDataPtr = &DABMW_rdsData[slot].DABMW_pi.lastValid;

        DABMW_rdsData[slot].DABMW_pi.newValue = DABMW_STORAGE_STATUS_IS_USED;
        
        res = DABMW_RDS_PI_LENGTH;
    }
    else if (DABMW_STORAGE_STATUS_IS_USED == DABMW_rdsData[slot].DABMW_pi.newValue && 
             true == forcedGet)
    {
        *rdsDataPtr = &DABMW_rdsData[slot].DABMW_pi.lastValid;
        
        res = DABMW_RDS_PI_LENGTH;
    }    
    else
    {
        res = 0;
    }

    return res;
}

tSInt DABMW_RdsCheckPi (tSInt slot, tPU32 piPtr)
{
    tSInt res = 0;

    // Send back the PI if it is available and verified
    if (DABMW_rdsData[slot].DABMW_pi.newValue >= DABMW_STORAGE_STATUS_IS_VERIFIED)
    {
        *piPtr = DABMW_rdsData[slot].DABMW_pi.lastValid;
        
        res = DABMW_RDS_PI_LENGTH;
    }

    return res;
}

tSInt DABMW_RdsForceGetPiData (tSInt slot, DABMW_storageStatusEnumTy *pPIStatus, tU32 *pCurrentPI, tU32 * pLastPI, tU32 * pBackupPI)
{
	tSInt res = 0; 

	*pPIStatus = DABMW_rdsData[slot].DABMW_pi.newValue;
	*pCurrentPI = DABMW_rdsData[slot].DABMW_pi.value;
	*pLastPI = DABMW_rdsData[slot].DABMW_pi.lastValid;
	*pBackupPI = DABMW_rdsData[slot].DABMW_pi.backupValue;

	return res;
}

tSInt DABMW_RdsForceGetTPTAData (tSInt slot, tVoid ** ppData)
{
    tSInt res = 0; 
    *ppData = &DABMW_rdsData[slot].DABMW_taTp;
    return res;
}

tSInt DABMW_RdsForceGetPTYData (tSInt slot, tVoid** ppData)
{
    tSInt res = 0; 

    *ppData = &DABMW_rdsData[slot].DABMW_pty;
    return res;
}

tSInt DABMW_RdsForceGetF_RDSStation (tSInt slot, tBool * pData)
{
    tSInt res = 0; 

    *pData = (DABMW_rdsData[slot].DABMW_rds.F_RDSStation == 1);

    return res;
}

tSInt DABMW_RdsGetTpTa (tSInt slot, tVoid** rdsDataPtr, tBool forcedGet)
{
    tSInt res; 

    // Send back the PI if it is available and verified
    if (DABMW_STORAGE_STATUS_IS_VERIFIED == DABMW_rdsData[slot].DABMW_taTp.newValue)
    {
        *rdsDataPtr = &DABMW_rdsData[slot].DABMW_taTp.lastData.value;

        DABMW_rdsData[slot].DABMW_taTp.newValue = DABMW_STORAGE_STATUS_IS_USED;
        
        res = DABMW_RDS_TPTA_LENGTH;
    }
    else if (DABMW_STORAGE_STATUS_IS_USED == DABMW_rdsData[slot].DABMW_taTp.newValue && 
             true == forcedGet)
    {
        *rdsDataPtr = &DABMW_rdsData[slot].DABMW_taTp.lastData.value;
        
        res = DABMW_RDS_TPTA_LENGTH;
    }    
    else
    {
        res = 0;
    }

    return res;
}
    
tSInt DABMW_RdsGetPtyTpTaMs (tSInt slot, tVoid** rdsDataPtr, tBool forcedGet)
{
    tSInt res = 0; 

    // Send back the PTY if it is available and verified
    if (DABMW_STORAGE_STATUS_IS_STORED == DABMW_rdsData[slot].DABMW_pty.newValue ||
        (DABMW_STORAGE_STATUS_IS_RETRIEVED == DABMW_rdsData[slot].DABMW_pty.newValue && true == forcedGet))
    {
        DABMW_rdsData[slot].DABMW_ptyTpTaMs.pty = DABMW_rdsData[slot].DABMW_pty.lastValid;

        DABMW_rdsData[slot].DABMW_ptyTpTaMs.ptyFlag = 0x1F;
        
        DABMW_rdsData[slot].DABMW_pty.newValue = DABMW_STORAGE_STATUS_IS_RETRIEVED;

        res = DABMW_RDS_PTYTPTAMS_LENGTH;
    }
    else
    {
        DABMW_rdsData[slot].DABMW_ptyTpTaMs.pty = CLEAR;

        DABMW_rdsData[slot].DABMW_ptyTpTaMs.ptyFlag = CLEAR;
    }
    
    // Send back the TP/TA if it is available and verified
    if (DABMW_STORAGE_STATUS_IS_VERIFIED == DABMW_rdsData[slot].DABMW_taTp.newValue ||
        (DABMW_STORAGE_STATUS_IS_USED == DABMW_rdsData[slot].DABMW_taTp.newValue && true == forcedGet))
    {
        DABMW_rdsData[slot].DABMW_ptyTpTaMs.tp  = DABMW_rdsData[slot].DABMW_taTp.lastData.fields.tpValue;
        DABMW_rdsData[slot].DABMW_ptyTpTaMs.ta  = DABMW_rdsData[slot].DABMW_taTp.lastData.fields.taValue;

        DABMW_rdsData[slot].DABMW_ptyTpTaMs.tpFlag = SET;
        DABMW_rdsData[slot].DABMW_ptyTpTaMs.taFlag = SET;

        DABMW_rdsData[slot].DABMW_taTp.newValue = DABMW_STORAGE_STATUS_IS_USED;

        res = DABMW_RDS_PTYTPTAMS_LENGTH;
    }
    else
    {
        DABMW_rdsData[slot].DABMW_ptyTpTaMs.tpFlag = CLEAR;
        DABMW_rdsData[slot].DABMW_ptyTpTaMs.taFlag = CLEAR;    
    }

    // M/S    
    if (DABMW_STORAGE_STATUS_IS_STORED == DABMW_rdsData[slot].DABMW_ms.newValue ||
        (DABMW_STORAGE_STATUS_IS_RETRIEVED == DABMW_rdsData[slot].DABMW_ms.newValue && true == forcedGet))
    {
        DABMW_rdsData[slot].DABMW_ptyTpTaMs.ms = DABMW_rdsData[slot].DABMW_ms.lastValid;
    
        DABMW_rdsData[slot].DABMW_ptyTpTaMs.msFlag = SET;
        
        DABMW_rdsData[slot].DABMW_ms.newValue = DABMW_STORAGE_STATUS_IS_RETRIEVED;

        res = DABMW_RDS_PTYTPTAMS_LENGTH;
    }
    else
    {
        DABMW_rdsData[slot].DABMW_ptyTpTaMs.ms = CLEAR;

        DABMW_rdsData[slot].DABMW_ptyTpTaMs.msFlag = CLEAR;
    }

    *rdsDataPtr = &DABMW_rdsData[slot].DABMW_ptyTpTaMs;

    return res;
}

tSInt DABMW_RdsGetDi (tSInt slot, tPU8 dataPtr, tBool forcedGet)
{
    tSInt res = 0;

    if (DABMW_STORAGE_STATUS_IS_STORED == DABMW_rdsData[slot].DABMW_diData.newValue ||
        (DABMW_STORAGE_STATUS_IS_RETRIEVED == DABMW_rdsData[slot].DABMW_diData.newValue && true == forcedGet))
    {
        *dataPtr = DABMW_rdsData[slot].DABMW_diData.value;
        
        DABMW_rdsData[slot].DABMW_diData.newValue = DABMW_STORAGE_STATUS_IS_RETRIEVED;

        res = DABMW_RDS_DI_LENGTH;
    }

    return res;
}

tSInt DABMW_RdsGetRt (tSInt slot, tPU8 dataPtr, tBool forcedGet)
{
    tSInt res;
    
    // Send back the PI if it is available and verified
    if (DABMW_STORAGE_STATUS_IS_VERIFIED == DABMW_rdsData[slot].DABMW_rt.newValue)
    {
        OSAL_pvMemoryCopy ((tPVoid)dataPtr, (tPVoid)&DABMW_rdsData[slot].DABMW_rt.rt[0], 64);

        DABMW_rdsData[slot].DABMW_rt.newValue = DABMW_STORAGE_STATUS_IS_RETRIEVED;
        
        res = DABMW_RDS_RT_LENGTH;
    }
    else if ((DABMW_STORAGE_STATUS_IS_VERIFIED == DABMW_rdsData[slot].DABMW_rt.newValue ||  
              DABMW_STORAGE_STATUS_IS_RETRIEVED == DABMW_rdsData[slot].DABMW_rt.newValue) &&
             true == forcedGet)
    {
        OSAL_pvMemoryCopy ((tPVoid)dataPtr, (tPVoid)&DABMW_rdsData[slot].DABMW_rt.rt[0], 64);
        
        res = DABMW_RDS_RT_LENGTH;
    }    
    else
    {
        res = 0;
    }

    return res;
}

tSInt DABMW_RdsGetAf(tSInt slot, ETAL_HANDLE hReceiver, tPU8 dataPtr, tBool forcedGet, tU32 piVal, tU32 baseFreq, 
                          tBool mode, tU32 maxNumberRetrieved)
{
    tSInt res = 0;

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
    if (DABMW_rdsData[slot].DABMW_af.afNew == DABMW_STORAGE_STATUS_IS_STORED)
    {
        // Force Get AF List when PI just changed
        res = DABMW_RdsGetAfList(hReceiver, dataPtr, /*forcedGet*/ TRUE, piVal, baseFreq, mode, DABMW_AF_LIST_BFR_LEN + 4);
        DABMW_rdsData[slot].DABMW_af.afNew = DABMW_STORAGE_STATUS_IS_RETRIEVED;
//printf("DABMW_RdsGetAf forceGet slot:%d PI:%x Freq:%d\n", slot, piVal, baseFreq);
    }
    else
    {
        res = DABMW_RdsGetAfList(hReceiver, dataPtr, forcedGet, piVal, baseFreq, mode, DABMW_AF_LIST_BFR_LEN + 4);
//printf("DABMW_RdsGetAf slot:%d PI:%d Freq:%d\n", slot, piVal, baseFreq);
    }
#else
// TODO: send AF when new af list is received without using AM FM Landscape.
#endif // CONFIG_ETALTML_HAVE_AMFMLANDSCAPE

    return res;
}

static tVoid DABMW_RdsFetchBlockData (tSInt slot, tU32 data)
{
    DABMW_charIntTy temp;
    tU8 group;
    DABMW_rdsTy block;

    // Save the data in temporary buffer
    block.RDSData[0] = ((data >> 24) & (tU8)0xFF);
    block.RDSData[1] = ((data >> 16) & (tU8)0xFF);
    block.RDSData[2] = ((data >>  8) & (tU8)0xFF);
    block.RDSData[3] = ((data >>  0) & (tU8)0xFF);

    // Check the block type:
    // - BLOCK A: BB[1] = 0 && BB[0]  = 0
    // - BLOCK B: BB[1] = 0 && BB[0]  = 1
    // - BLOCK C: BB[1] = 1 && BB[0]  = 0
    // - BLOCK D: BB[1] = 1 && BB[0]  = 1
#ifdef ETAL_RDS_IMPORT
    // CMOST response is different from STA610's
    group = (block.RDSData[1] & (tU8)0x03) >> 0;
#else
    group = (block.RDSData[1] & (tU8)0x30) >> 4;
#endif

    // EPR CHANGE codex #285979
    
    // data are already filtered when receive here
    // using parameters which are controllable thru RDS enabling.
    // remove this additionnal check which bypass the setup parameters
    //
    /*
    // Check the data goodness, this is not necessary because already done,
    // it is left here for testing.
    // Good data: 
    // - CP  = 0, bits 27 -> 31
    // - BM  = 0
    // - SYN = 1, bit 24
    // - DOK = 1, bit 25
    // - Z   = 1, bit 23    
    if (((block.RDSData[1] & 0x80) != 0x80) ||
        ((block.RDSData[0] & 0x03) != 0x03))
    {
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockA  = 0;
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockB  = 0;
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockC  = 0;
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockCp = 0;
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockD  = 0;
        DABMW_rdsData[slot].DABMW_rds.BlockD       = 0;

        DABMW_rdsData[slot].DABMW_rds.lastGroup    = DABMW_RDS_GROUP_NONE;

        return;
    }
        */
        
    // END EPR CHANGE 
    
    // Protect against block E. It should not used but if we found it just trash:
//#ifdef ETAL_RDS_IMPORT
    // CMOST response is different from STA610's
    // - BLOCKE  = 0, bit 19 since TDA7707 FW v4.3.5 and TDA7708 FW v3.1.3
//    if (((block.RDSData[1] & 0x08) != 0x00))
//#else
    // - BE  = 0, bit 22
    if (((block.RDSData[1] & 0x40) != 0x00))
//#endif
    {
        return;
    }

    // Protect against wrong sequence
    if ((DABMW_rdsData[slot].DABMW_rds.lastGroup != DABMW_RDS_GROUP_B) &&
        ((group == DABMW_RDS_GROUP_C) || (group == DABMW_RDS_GROUP_D)))
    {
        return;
    }

    // Save the data in the RDS structure
    DABMW_rdsData[slot].DABMW_rds.RDSData[0] = block.RDSData[0];
    DABMW_rdsData[slot].DABMW_rds.RDSData[1] = block.RDSData[1];
    DABMW_rdsData[slot].DABMW_rds.RDSData[2] = block.RDSData[2];
    DABMW_rdsData[slot].DABMW_rds.RDSData[3] = block.RDSData[3];

    // Save the real RDS data only (no control flags)
    temp.byte[1] = DABMW_rdsData[slot].DABMW_rds.RDSData[2];
    temp.byte[0] = DABMW_rdsData[slot].DABMW_rds.RDSData[3];
    
    if (DABMW_RDS_GROUP_A == group)
    {
        DABMW_rdsData[slot].DABMW_rds.lastGroup    = DABMW_RDS_GROUP_A;
        
        DABMW_rdsData[slot].DABMW_rds.BlockCnt     = 1;
        
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockA  = 1;
        DABMW_rdsData[slot].DABMW_rds.BlockA       = temp.dbyte;
        DABMW_rdsData[slot].DABMW_rds.F_RDSStation = 1;

        DABMW_rdsData[slot].DABMW_rds.F_GetBlockB  = 0;
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockC  = 0;
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockCp = 0;
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockD  = 0;     

		// This is a new block => mark it 
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A_used = false;
    }
    else if (DABMW_RDS_GROUP_B == group)
    {
        DABMW_rdsData[slot].DABMW_rds.lastGroup    = DABMW_RDS_GROUP_B;
        
        DABMW_rdsData[slot].DABMW_rds.BlockCnt     = 2;

        DABMW_rdsData[slot].DABMW_rds.F_GetBlockB  = 1;
        DABMW_rdsData[slot].DABMW_rds.BlockB       = temp.dbyte;
        DABMW_rdsData[slot].DABMW_rds.F_RDSStation = 1;
        
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockC  = 0;
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockCp = 0;
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockD  = 0;     
		
		// This is a new block => mark it 
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B_used = false;
    }       
    else if (DABMW_RDS_GROUP_C == group)
    {
        DABMW_rdsData[slot].DABMW_rds.BlockCnt     = 3;

        DABMW_rdsData[slot].DABMW_rds.F_GetBlockC  = 1;
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockCp = 1;
        DABMW_rdsData[slot].DABMW_rds.BlockC       = temp.dbyte;
        DABMW_rdsData[slot].DABMW_rds.BlockCp      = DABMW_rdsData[slot].DABMW_rds.BlockC;
        DABMW_rdsData[slot].DABMW_rds.F_RDSStation = 1;
        
        DABMW_rdsData[slot].DABMW_rds.F_GetBlockD = 0;

		// This is a new block => mark it 
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C_used = false;
    }
    else if (DABMW_RDS_GROUP_D == group)
    {
        DABMW_rdsData[slot].DABMW_rds.BlockCnt     = 4;

        DABMW_rdsData[slot].DABMW_rds.F_GetBlockD  = 1;
        DABMW_rdsData[slot].DABMW_rds.BlockD       = temp.dbyte;
        DABMW_rdsData[slot].DABMW_rds.F_RDSStation = 1;
		
		// This is a new block => mark it 
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D_used = false;
    }

    // Set the Last Block Count indication
    DABMW_rdsData[slot].DABMW_rds.LastBlockCnt = DABMW_rdsData[slot].DABMW_rds.BlockCnt;
    
    // Push data for every block because we want to decode the PI as soon as possible
    DABMW_PushToLast (slot);
}

static tVoid DABMW_PushToLast (tSInt slot)
{
    tU8 blockflags;
    
    DABMW_rdsData[slot].DABMW_rds.BlockLastA   = DABMW_rdsData[slot].DABMW_rds.BlockA;
    DABMW_rdsData[slot].DABMW_rds.BlockLastB   = DABMW_rdsData[slot].DABMW_rds.BlockB;
    DABMW_rdsData[slot].DABMW_rds.BlockLastC   = DABMW_rdsData[slot].DABMW_rds.BlockC;
    DABMW_rdsData[slot].DABMW_rds.BlockLastCp  = DABMW_rdsData[slot].DABMW_rds.BlockCp;
    DABMW_rdsData[slot].DABMW_rds.BlockLastD   = DABMW_rdsData[slot].DABMW_rds.BlockD;

    DABMW_rdsData[slot].DABMW_rds.F_BlockA     = DABMW_rdsData[slot].DABMW_rds.F_GetBlockA;
    DABMW_rdsData[slot].DABMW_rds.F_BlockB     = DABMW_rdsData[slot].DABMW_rds.F_GetBlockB;
    DABMW_rdsData[slot].DABMW_rds.F_BlockC     = DABMW_rdsData[slot].DABMW_rds.F_GetBlockC;
    DABMW_rdsData[slot].DABMW_rds.F_BlockCp    = DABMW_rdsData[slot].DABMW_rds.F_GetBlockCp;
    DABMW_rdsData[slot].DABMW_rds.F_BlockD     = DABMW_rdsData[slot].DABMW_rds.F_GetBlockD;


	/* EPR CHANGE 
	* Now we may be call here for every block processing 
	* therefore the status for block A to D should be reset only when block complete
	*
	* BEFORE
    DABMW_rdsData[slot].DABMW_rds.F_GetBlockA  = 0;
    DABMW_rdsData[slot].DABMW_rds.F_GetBlockB  = 0;
    DABMW_rdsData[slot].DABMW_rds.F_GetBlockC  = 0;
    DABMW_rdsData[slot].DABMW_rds.F_GetBlockCp = 0;
    DABMW_rdsData[slot].DABMW_rds.F_GetBlockD  = 0;
	*AFTER
	*/
	if ((1 == DABMW_rdsData[slot].DABMW_rds.F_GetBlockA)
	   && (1 == DABMW_rdsData[slot].DABMW_rds.F_GetBlockB)
	   && (1 == DABMW_rdsData[slot].DABMW_rds.F_GetBlockC)
	   && (1 == DABMW_rdsData[slot].DABMW_rds.F_GetBlockD)
	   )
		{
 		DABMW_rdsData[slot].DABMW_rds.F_GetBlockA  = 0;
    	DABMW_rdsData[slot].DABMW_rds.F_GetBlockB  = 0;
    	DABMW_rdsData[slot].DABMW_rds.F_GetBlockC  = 0;
    	DABMW_rdsData[slot].DABMW_rds.F_GetBlockCp = 0;
    	DABMW_rdsData[slot].DABMW_rds.F_GetBlockD  = 0;
    DABMW_rdsData[slot].DABMW_rds.F_GetOneGroup = 1;
		}
	/* END EPR change */

    blockflags = 0;
    if (DABMW_rdsData[slot].DABMW_rds.F_BlockA)
    {
        blockflags |= 0x01;
    }
    if (DABMW_rdsData[slot].DABMW_rds.F_BlockB)
    {
        blockflags |= 0x02;
    }
    if (DABMW_rdsData[slot].DABMW_rds.F_BlockC)
    {
        blockflags |= 0x04;
    }
    if (DABMW_rdsData[slot].DABMW_rds.F_BlockCp)
    {
        blockflags |= 0x08;
    }
    if (DABMW_rdsData[slot].DABMW_rds.F_BlockD)
    {
        blockflags |= 0x10;
    }
    
    DABMW_RdsSetBlockData (slot,
                           blockflags, 
                           DABMW_rdsData[slot].DABMW_rds.BlockLastA, 
                           DABMW_rdsData[slot].DABMW_rds.BlockLastB, 
                           DABMW_rdsData[slot].DABMW_rds.BlockLastC, 
                           DABMW_rdsData[slot].DABMW_rds.BlockLastCp, 
                           DABMW_rdsData[slot].DABMW_rds.BlockLastD);
    
}

static tVoid DABMW_RdsSetBlockData (tSInt slot,
                                    tU8 bflags, tU32 blockA,
                                    tU32 blockB, tU32 blockC, 
                                    tU32 blockCp, tU32 blockD)
{
    if ((bflags & 0x01) == 0)
    {
        DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A = false;
    }
    else
    {
        DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A = true;
        DABMW_rdsData[slot].DABMW_blockInfo.block_A = blockA;
    }

    if ((bflags & 0x02) == 0)
    {
        DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B = false;
    }
    else
    {
        DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B = true;
        DABMW_rdsData[slot].DABMW_blockInfo.block_B = blockB;        
    }

    if ((bflags & 0x04) == 0)
    {
        DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C = false;        
    }
    else
    {
        DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C = true;
        DABMW_rdsData[slot].DABMW_blockInfo.block_C = blockC;            
    }

    if ((bflags & 0x08) == 0)
    {
        DABMW_rdsData[slot].DABMW_blockInfo.availBlock_Cp = false;   
    }
    else
    {
        DABMW_rdsData[slot].DABMW_blockInfo.availBlock_Cp = true;
        DABMW_rdsData[slot].DABMW_blockInfo.block_Cp = blockCp;           
    }

    if ((bflags & 0x10) == 0)
    {
        DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D = false; 
    }
    else
    {
        DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D = true;
        DABMW_rdsData[slot].DABMW_blockInfo.block_D = blockD;           
    }

	/* change EPR : block complete only if A, B, C or Cp, D present
	* 
	*		   DABMW_rdsData[slot].DABMW_blockInfo.blocksComplete = true;   
	*/
	
	if ((true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A )
		&& (true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B )
		&& (true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D)
		&& ((true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C) || (true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_Cp))
		)
		{
    DABMW_rdsData[slot].DABMW_blockInfo.blocksComplete = true;    
		}
	/*END EPR */
 
}

static tVoid DABMW_RdsExtrData (tSInt slot, DABMW_RDS_mwAppTy app)
{
    tU8 i;
    tU32 temp;
    DABMW_charIntTy AFtemp;
    tSInt resCmp;
    tU8 tmpData[2];
    tU64 cnt;
    tBool rtComplete;
    tBool flagVal;
    tU8 tmp;
    tU32 curFreq;
    tU8 diVal;
	// variable to track the rt data changes
	tBool vl_rtChange = false;
	tU8	vl_newRtByte;
	

	/* EPR CHANGE */
	/* Get current Freq here
	*/
    // Calculate the current frequency based on the application
#ifdef ETAL_RDS_IMPORT
    curFreq = ETAL_receiverGetFrequency((ETAL_HANDLE)app);
#else
    if (DABMW_MAIN_RDS_APP == app)
    {
        curFreq = DABMW_GetFrequencyFromApp (DABMW_MAIN_AMFM_APP);                        
    }
    else if (DABMW_SECONDARY_RDS_APP == app)
    {
        curFreq = DABMW_GetFrequencyFromApp (DABMW_BACKGROUND_AMFM_APP);
    }
    else
    {
        curFreq = DABMW_INVALID_FREQUENCY;
    }
#endif

	/* Process PI */
	if (DABMW_RdsPi_Processing(slot, app, curFreq))
    {
		// Now I know that a new PI has been confirmed  so I can clean all data
		// received because it is belonging to a new channel	
            
		DABMW_CleanUpData (slot);

        // Change afNew start to stored to force an AF list notification on following DABMW_RdsGetAf
        DABMW_rdsData[slot].DABMW_af.afNew = DABMW_STORAGE_STATUS_IS_STORED;
    }
	
	/* END EPR CHANGE */
    
    // If an entire group of info is not collected just return waiting to have it
    if (false == DABMW_rdsData[slot].DABMW_blockInfo.blocksComplete)
    { 
        return;
    }


	// If in FAST PI detection mode : no need to do further processing
	if (true == DABMW_rdsData[slot].DABMW_IsFastPiDetectionMode)
		{
		  return;
		}
	

    // Clear the group indication because this group is now used
    DABMW_rdsData[slot].DABMW_blockInfo.blocksComplete = false;

    // Get the group indication from the 5 MSB bits of the BLOCK B
    DABMW_rdsData[slot].DABMW_realGroup = (tU8)((DABMW_rdsData[slot].DABMW_blockInfo.block_B >> 11) & (tU8)0x1F);   


#ifdef  CONFIG_ETALTML_HAVE_RDS_STRATEGY
	//Received a new group, to indicate it's RDS station or not sure
	ETALTML_RDS_Strategy_RDSStation_Indicator(slot);
#endif

	/* EPR CHANGE 
	* move current freq calculation beginning of procedure
	* move PI processing beginning of procedure
	* so no check on Block A anymore here
	*/
           

    // If the PI is not retrieved and verified just return here and do not 
    // store data. All data must be stored only when the PI is recognized
    // as valid
    if (DABMW_rdsData[slot].DABMW_pi.newValue < DABMW_STORAGE_STATUS_IS_VERIFIED)
    {
		/* block is complete but PI not valid
		* clean the info for next block processing
		*/
		/* EPR Change
		*/
	
		// data have been proceed now we should clean info up  to next full block
		DABMW_rdsData[slot].DABMW_blockInfo.blocksComplete = false;
		DABMW_rdsData[slot].DABMW_rds.BlockCnt	   = 0;
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A = false;
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B = false;
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C = false;
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_Cp = false;
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D = false;
		OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_blockInfo, 0x00, sizeof (DABMW_rdsData[slot].DABMW_blockInfo));
		
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A_used = false;
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B_used = false;
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C_used = false;
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D_used = false;
		
		/* END EPR Change í*/
	
        return;
    }

    // M/S
    if (DABMW_GROUP_0A == DABMW_rdsData[slot].DABMW_realGroup || 
        DABMW_GROUP_0B == DABMW_rdsData[slot].DABMW_realGroup || 
        DABMW_GROUP_15B == DABMW_rdsData[slot].DABMW_realGroup)
    {
        DABMW_rdsData[slot].DABMW_ms.value = (tU8)((DABMW_rdsData[slot].DABMW_blockInfo.block_B & 0x0008) >> 3);

            // codex #284301
            // If no value currently stored : take that one as valid
            // => ie if storage = empty, take it. 
            //
        if ((DABMW_rdsData[slot].DABMW_ms.lastValid != DABMW_rdsData[slot].DABMW_ms.value)
            || (DABMW_STORAGE_STATUS_IS_EMPTY == DABMW_rdsData[slot].DABMW_ms.newValue))
        {
            DABMW_rdsData[slot].DABMW_ms.lastValid = DABMW_rdsData[slot].DABMW_ms.value;
            
            DABMW_rdsData[slot].DABMW_ms.newValue = DABMW_STORAGE_STATUS_IS_STORED;
        }
    }

    // DI
    if (DABMW_GROUP_0A == DABMW_rdsData[slot].DABMW_realGroup || 
        DABMW_GROUP_0B == DABMW_rdsData[slot].DABMW_realGroup)
    {
        // Get the address
        tmp = (tU8)3 - (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_B & (tU32)0x03);

        // Set the presence bit for this address
        DABMW_rdsData[slot].DABMW_diData.value |= (tU8)0x10 << tmp;

        // Get the bit value
        diVal = (tU8)(((DABMW_rdsData[slot].DABMW_blockInfo.block_B & (tU32)0x0004) >> 2) & (tU32)0x01);

        // Set or clear the bit depending on the retrieved value
        if ((tU8)0x01 == diVal)
        {
            DABMW_rdsData[slot].DABMW_diData.value |= (tU8)0x01 << tmp;
        }
        else
        {
            DABMW_rdsData[slot].DABMW_diData.value &= ~((tU8)0x01 << tmp);
        }
        
        if ((DABMW_rdsData[slot].DABMW_diData.value & (tU8)0xF0) == (tU8)0xF0)
        {
            // Set the status to STORED if the value has changed
            // codex #284301
            // If no value currently stored : take that one as valid
            // => ie if storage = empty, take it. 
            //
            if ((DABMW_rdsData[slot].DABMW_diData.value != DABMW_rdsData[slot].DABMW_diData.lastValid)
                || (DABMW_STORAGE_STATUS_IS_EMPTY == DABMW_rdsData[slot].DABMW_diData.newValue))
            {
                DABMW_rdsData[slot].DABMW_diData.lastValid = DABMW_rdsData[slot].DABMW_diData.value;
                
                DABMW_rdsData[slot].DABMW_diData.newValue = DABMW_STORAGE_STATUS_IS_STORED;
            }
        }
    }

    if (DABMW_GROUP_15B == DABMW_rdsData[slot].DABMW_realGroup)
    {
        // Get the address
        tmp = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D & (tU32)0x03);

        // Set the presence bit for this address
        DABMW_rdsData[slot].DABMW_diData.value |= (tU8)0x10 << tmp;

        // Get the bit value
        tmp = (tU8)(((DABMW_rdsData[slot].DABMW_blockInfo.block_D & (tU32)0x0004) >> 2) & (tU32)0x01);

        // Set or clear the bit depending on the retrieved value
        if (1 == tmp)
        {
            DABMW_rdsData[slot].DABMW_diData.value |= (tU8)0x01 << tmp;
        }
        else
        {
            DABMW_rdsData[slot].DABMW_diData.value ^= (tU8)0x01 << tmp;
        }
        
        if ((DABMW_rdsData[slot].DABMW_diData.value & (tU8)0xF0) == (tU8)0xF0)
        {        
            DABMW_rdsData[slot].DABMW_diData.newValue = DABMW_STORAGE_STATUS_IS_STORED;
        }
    }    
    
    // PTY, TP, TA, 
    if (true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A && 
        true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B)
    {
#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf (0, TR_LEVEL_COMPONENT, TR_CLASS_APP_DABMW, 
            "DABMW RDS: PTY, TP and TA found, blocks %c - %c\n", 'A', 'B');
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)    

        // PTY
        DABMW_rdsData[slot].DABMW_pty.value = (tU8)((DABMW_rdsData[slot].DABMW_blockInfo.block_B & 0x03E0) >> 5);

          // Set the status to STORED if the value has changed
            // codex #284301
            // If no value currently stored : take that one as valid
            // => ie if storage = empty, take it. 
            //
        if ((DABMW_rdsData[slot].DABMW_pty.lastValid != DABMW_rdsData[slot].DABMW_pty.value)
            || (DABMW_STORAGE_STATUS_IS_EMPTY == DABMW_rdsData[slot].DABMW_pty.newValue))
        {
            DABMW_rdsData[slot].DABMW_pty.lastValid = DABMW_rdsData[slot].DABMW_pty.value;

            if (DABMW_rdsData[slot].DABMW_pty.lastValid < 32)
            {
                OSAL_pvMemoryCopy ((tPVoid)&DABMW_rdsData[slot].DABMW_pty.data8String[0], (tPVoid)&DABMW_pty8CharTableTy[DABMW_rdsData[slot].DABMW_pty.lastValid], 8);
                OSAL_pvMemoryCopy ((tPVoid)&DABMW_rdsData[slot].DABMW_pty.data16String[0], (tPVoid)&DABMW_pty16CharTableTy[DABMW_rdsData[slot].DABMW_pty.lastValid], 16);
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
                DABMW_AmFmLandscapeSetPTY(DABMW_rdsData[slot].DABMW_pi.value, DABMW_rdsData[slot].DABMW_pty.lastValid, curFreq);
#endif
            }
            else
            {
                OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_pty.data8String[0], 0x00, 8);
                OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_pty.data16String[0], 0x00, 16);
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
                DABMW_AmFmLandscapeSetPTY(DABMW_rdsData[slot].DABMW_pi.value, 0x00, curFreq);
#endif
            }
            
            DABMW_rdsData[slot].DABMW_pty.newValue = DABMW_STORAGE_STATUS_IS_STORED;
        }

        // TP
        if (DABMW_rdsData[slot].DABMW_blockInfo.block_B & 0x0400)
        { 
            DABMW_rdsData[slot].DABMW_taTp.data.fields.tpValue = SET;
        }
        else
        { 
            DABMW_rdsData[slot].DABMW_taTp.data.fields.tpValue = CLEAR;
        }
        
        // TA
        if (DABMW_GROUP_0A == DABMW_rdsData[slot].DABMW_realGroup || 
            DABMW_GROUP_0B == DABMW_rdsData[slot].DABMW_realGroup || 
            DABMW_GROUP_15B == DABMW_rdsData[slot].DABMW_realGroup)
        {
            if (DABMW_rdsData[slot].DABMW_blockInfo.block_B & 0x0010)
            { 
                DABMW_rdsData[slot].DABMW_taTp.data.fields.taValue = SET;
            }
            else
            { 
                DABMW_rdsData[slot].DABMW_taTp.data.fields.taValue = CLEAR;
            }
        }
        
        // TA/TP recognition for ongoing traffic
        if (DABMW_rdsData[slot].DABMW_taTp.lastData.fields.taValue == DABMW_rdsData[slot].DABMW_taTp.data.fields.taValue && 
            DABMW_rdsData[slot].DABMW_taTp.lastData.fields.tpValue == DABMW_rdsData[slot].DABMW_taTp.data.fields.tpValue)
        {
            DABMW_rdsData[slot].DABMW_taTp.confidenceThr++;

            if (DABMW_rdsData[slot].DABMW_taTp.confidenceThr >= DABMW_rdsData[slot].DABMW_criticalDataThr &&
                DABMW_rdsData[slot].DABMW_taTp.newValue < DABMW_STORAGE_STATUS_IS_VERIFIED)
            {
                DABMW_rdsData[slot].DABMW_taTp.newValue = DABMW_STORAGE_STATUS_IS_VERIFIED;
            }
        }
        else
        {
            DABMW_rdsData[slot].DABMW_taTp.confidenceThr = 0;
        }
        
        // TA/TP update the last values
                // Set the status to STORED if the value has changed
            // codex #284301
            // If no value currently stored : take that one as valid
            // => ie if storage = empty, take it. 
            //

        if (DABMW_STORAGE_STATUS_IS_EMPTY == DABMW_rdsData[slot].DABMW_taTp.newValue)
        {
            DABMW_rdsData[slot].DABMW_taTp.lastData.fields.taValue = DABMW_rdsData[slot].DABMW_taTp.data.fields.taValue;

            DABMW_rdsData[slot].DABMW_taTp.lastData.fields.tpValue = DABMW_rdsData[slot].DABMW_taTp.data.fields.tpValue;
                
            DABMW_rdsData[slot].DABMW_taTp.newValue = DABMW_STORAGE_STATUS_IS_STORED;
        }
        else 
        {
            if (DABMW_rdsData[slot].DABMW_taTp.lastData.fields.taValue != DABMW_rdsData[slot].DABMW_taTp.data.fields.taValue)
            {
                DABMW_rdsData[slot].DABMW_taTp.lastData.fields.taValue = DABMW_rdsData[slot].DABMW_taTp.data.fields.taValue;
                
                DABMW_rdsData[slot].DABMW_taTp.newValue = DABMW_STORAGE_STATUS_IS_STORED;
            }
                
            if (DABMW_rdsData[slot].DABMW_taTp.lastData.fields.tpValue != DABMW_rdsData[slot].DABMW_taTp.data.fields.tpValue)
            {
                DABMW_rdsData[slot].DABMW_taTp.lastData.fields.tpValue = DABMW_rdsData[slot].DABMW_taTp.data.fields.tpValue;
                
                DABMW_rdsData[slot].DABMW_taTp.newValue = DABMW_STORAGE_STATUS_IS_STORED;
            } 
        }
    }
        
    // PS (Program Service, i.e. Station Label)
    if (true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A && true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B && 
        (DABMW_GROUP_0A == DABMW_rdsData[slot].DABMW_realGroup || 
         DABMW_GROUP_0B == DABMW_rdsData[slot].DABMW_realGroup) && true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D)
    {
#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf (0, TR_LEVEL_COMPONENT, TR_CLASS_APP_DABMW, 
            "DABMW RDS: PS found, blocks %c - %c - %c on group # %d\n", 'A', 'B', 'D', DABMW_rdsData[slot].DABMW_realGroup);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)    

        i = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_B) & 0x03;
        
        if ((DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x00FF) >= 0x20 && (DABMW_rdsData[slot].DABMW_blockInfo.block_D >> 8) >= 0x20)
        {
            switch(i)
            {
                case 0:
                    DABMW_rdsData[slot].DABMW_ps.get0 = true;     

                    tmpData[0] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x00FF);
                    tmpData[1] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D >> 8);

                    if (DABMW_rdsData[slot].DABMW_ps.label[0] != tmpData[0] || 
                        DABMW_rdsData[slot].DABMW_ps.label[1] != tmpData[1])
                    {
                        DABMW_rdsData[slot].DABMW_ps.get1 = false; 
                        DABMW_rdsData[slot].DABMW_ps.get2 = false; 
                        DABMW_rdsData[slot].DABMW_ps.get3 = false; 
                    }
                    
                    DABMW_rdsData[slot].DABMW_ps.label[1] = tmpData[0];
                    DABMW_rdsData[slot].DABMW_ps.label[0] = tmpData[1];
                    break;
                case 1:
                    DABMW_rdsData[slot].DABMW_ps.get1 = true;

                    tmpData[0] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x00FF);
                    tmpData[1] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D >> 8);

                    if (DABMW_rdsData[slot].DABMW_ps.label[3] != tmpData[0] || 
                        DABMW_rdsData[slot].DABMW_ps.label[2] != tmpData[1])
                    {
                        DABMW_rdsData[slot].DABMW_ps.get0 = false; 
                        DABMW_rdsData[slot].DABMW_ps.get2 = false; 
                        DABMW_rdsData[slot].DABMW_ps.get3 = false; 
                    }
                    
                    DABMW_rdsData[slot].DABMW_ps.label[3] = tmpData[0];
                    DABMW_rdsData[slot].DABMW_ps.label[2] = tmpData[1];
                    break;
                case 2:
                    DABMW_rdsData[slot].DABMW_ps.get2 = true; 

                    tmpData[0] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x00FF);
                    tmpData[1] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D >> 8);

                    if (DABMW_rdsData[slot].DABMW_ps.label[5] != tmpData[0] || 
                        DABMW_rdsData[slot].DABMW_ps.label[4] != tmpData[1])
                    {
                        DABMW_rdsData[slot].DABMW_ps.get0 = false; 
                        DABMW_rdsData[slot].DABMW_ps.get1 = false; 
                        DABMW_rdsData[slot].DABMW_ps.get3 = false; 
                    }
                    
                    DABMW_rdsData[slot].DABMW_ps.label[5] = tmpData[0];
                    DABMW_rdsData[slot].DABMW_ps.label[4] = tmpData[1];
                    break;
                case 3:
                    DABMW_rdsData[slot].DABMW_ps.get3 = true;  

                    tmpData[0] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x00FF);
                    tmpData[1] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D >> 8);

                    if (DABMW_rdsData[slot].DABMW_ps.label[7] != tmpData[0] || 
                        DABMW_rdsData[slot].DABMW_ps.label[6] != tmpData[1])
                    {
                        DABMW_rdsData[slot].DABMW_ps.get0 = false; 
                        DABMW_rdsData[slot].DABMW_ps.get1 = false; 
                        DABMW_rdsData[slot].DABMW_ps.get2 = false; 
                    }
                    
                    DABMW_rdsData[slot].DABMW_ps.label[7] = tmpData[0];
                    DABMW_rdsData[slot].DABMW_ps.label[6] = tmpData[1];
                    break;
            }
        }
        
        if (DABMW_rdsData[slot].DABMW_ps.get0 && DABMW_rdsData[slot].DABMW_ps.get1 && 
            DABMW_rdsData[slot].DABMW_ps.get2 && DABMW_rdsData[slot].DABMW_ps.get3)
        {
            DABMW_rdsData[slot].DABMW_ps.get0 = false;  
            DABMW_rdsData[slot].DABMW_ps.get1 = false; 
            DABMW_rdsData[slot].DABMW_ps.get2 = false; 
            DABMW_rdsData[slot].DABMW_ps.get3 = false; 

            // Compare the old label with the new one to check identity
            resCmp = OSAL_s32MemoryCompare ((tPVoid)&DABMW_rdsData[slot].DABMW_ps.labelBackup[0], (tPVoid)&DABMW_rdsData[slot].DABMW_ps.label[0], DABMW_RDS_PS_LENGTH);
            
            // Save the label in the backup space
            if (0 != resCmp)
            {
                OSAL_pvMemoryCopy ((tPVoid)&DABMW_rdsData[slot].DABMW_ps.labelBackup[0], (tPVoid)DABMW_rdsData[slot].DABMW_ps.label, DABMW_RDS_PS_LENGTH);

                // Signal a new label
                DABMW_rdsData[slot].DABMW_ps.newValue = DABMW_STORAGE_STATUS_IS_STORED;           

                // Ask for current frequency
#ifdef ETAL_RDS_IMPORT
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
                curFreq = ETAL_receiverGetFrequency((ETAL_HANDLE)app);
                if (DABMW_INVALID_FREQUENCY != curFreq)
                {
                    DABMW_AmFmLandscapeSetPs (DABMW_rdsData[slot].DABMW_pi.value, (tPU8)&DABMW_rdsData[slot].DABMW_ps.labelBackup[0], curFreq);
                }                
#endif
#else
                if (DABMW_MAIN_RDS_APP == app)
                {
                    curFreq = DABMW_GetFrequencyFromApp (DABMW_MAIN_AMFM_APP);

                    if (DABMW_INVALID_FREQUENCY != curFreq)
                    {
                        // Call the db manager
                        DABMW_AmFmLandscapeSetPs (DABMW_rdsData[slot].DABMW_pi.value, (tPU8)&DABMW_rdsData[slot].DABMW_ps.labelBackup[0], curFreq);
                    }                
                }
                else if (DABMW_SECONDARY_RDS_APP == app)
                {
                    curFreq = DABMW_GetFrequencyFromApp (DABMW_BACKGROUND_AMFM_APP);

                    if (DABMW_INVALID_FREQUENCY != curFreq)
                    {
                        // Call the db manager
                        DABMW_AmFmLandscapeSetPs (DABMW_rdsData[slot].DABMW_pi.value, (tPU8)&DABMW_rdsData[slot].DABMW_ps.labelBackup[0], curFreq);
                    }                 
                }             
#endif // ETAL_RDS_IMPORT
            }

#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
            OSALUTIL_s32TracePrintf (0, TR_LEVEL_COMPONENT, TR_CLASS_APP_DABMW, 
                "DABMW RDS: New label stored, %s\n", &DABMW_rdsData[slot].DABMW_ps.label[0]);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)                   
        }
    }

    // TIME
    if (true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B && (DABMW_GROUP_4A == DABMW_rdsData[slot].DABMW_realGroup) && 
        true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C && true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D)
    {
#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf (0, TR_LEVEL_COMPONENT, TR_CLASS_APP_DABMW, 
            "DABMW RDS: TIME found, blocks %c - %c - %c on group # %d\n", 'B', 'C', 'D', DABMW_rdsData[slot].DABMW_realGroup);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)   

        // Copy current to previous
        DABMW_rdsData[slot].DABMW_previousTime.hour = DABMW_rdsData[slot].DABMW_time.hour;
        DABMW_rdsData[slot].DABMW_previousTime.min = DABMW_rdsData[slot].DABMW_time.min;
        DABMW_rdsData[slot].DABMW_previousTime.offset = DABMW_rdsData[slot].DABMW_time.offset;
        DABMW_rdsData[slot].DABMW_previousTime.mjd = DABMW_rdsData[slot].DABMW_time.mjd;
        DABMW_rdsData[slot].DABMW_previousTime.newCt = DABMW_STORAGE_STATUS_IS_STORED;

        DABMW_rdsData[slot].DABMW_time.hour   = ((tU8) ((DABMW_rdsData[slot].DABMW_blockInfo.block_D >> (tU8)12)  & (tU8)0x0F)) + ((tU8)((DABMW_rdsData[slot].DABMW_blockInfo.block_C & 0x0001) << 4) & (tU8)0x10);
        DABMW_rdsData[slot].DABMW_time.min    = ((tU8) ((DABMW_rdsData[slot].DABMW_blockInfo.block_D >> (tU8)6)   & (tU8)0x3F));
        DABMW_rdsData[slot].DABMW_time.offset = ((tU8) ((DABMW_rdsData[slot].DABMW_blockInfo.block_D >> (tU8)0)   & (tU8)0x3F));
        DABMW_rdsData[slot].DABMW_time.mjd    = ((tU32)((DABMW_rdsData[slot].DABMW_blockInfo.block_B << (tU32)15) & (tU32)0x18000)) + ((tU32)((DABMW_rdsData[slot].DABMW_blockInfo.block_C >> (tU32)1) & (tU32)0x07FFF));

        temp = (tU32)DABMW_rdsData[slot].DABMW_time.hour * 60 + (tU32)DABMW_rdsData[slot].DABMW_time.min + 1440;
        
        if (DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x0020)
        {
            temp -= ((DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x001F) * 30);
        }
        else
        {
            temp += ((DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x001F) * 30);
        }
        
        temp = temp % 1440;
        DABMW_rdsData[slot].DABMW_time.hour = temp / 60;
        DABMW_rdsData[slot].DABMW_time.min = temp % 60;

        if (DABMW_STORAGE_STATUS_IS_EMPTY == DABMW_rdsData[slot].DABMW_time.newCt)
        {
            DABMW_rdsData[slot].DABMW_time.newCt = DABMW_STORAGE_STATUS_IS_STORED;
        }
        else if (DABMW_STORAGE_STATUS_IS_RETRIEVED == DABMW_rdsData[slot].DABMW_time.newCt)
        {
            if (DABMW_rdsData[slot].DABMW_time.hour != DABMW_rdsData[slot].DABMW_previousTime.hour ||
                DABMW_rdsData[slot].DABMW_time.min != DABMW_rdsData[slot].DABMW_previousTime.min ||
                DABMW_rdsData[slot].DABMW_time.offset != DABMW_rdsData[slot].DABMW_previousTime.offset ||
                DABMW_rdsData[slot].DABMW_time.mjd != DABMW_rdsData[slot].DABMW_previousTime.mjd)
            {
                DABMW_rdsData[slot].DABMW_time.newCt = DABMW_STORAGE_STATUS_IS_STORED;
            }
        }
    }

    // AF
    if (true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A && true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B && 
        (DABMW_GROUP_0A == DABMW_rdsData[slot].DABMW_realGroup) && true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C)
    {
#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf (0, TR_LEVEL_COMPONENT, TR_CLASS_APP_DABMW, 
            "DABMW RDS: AF found, blocks %c - %c - %c on group # %d\n", 'A', 'B', 'C', DABMW_rdsData[slot].DABMW_realGroup);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)   

        AFtemp.dbyte = DABMW_rdsData[slot].DABMW_blockInfo.block_C;

        // Check the data type
        DABMW_RdsAfDataPush (slot, AFtemp.byte[1], 0);
        DABMW_RdsAfDataPush (slot, AFtemp.byte[0], 1);

#ifdef  CONFIG_ETALTML_HAVE_RDS_STRATEGY
        //Add to RDS Strategy AF data management
        AFtemp.dbyte = DABMW_rdsData[slot].DABMW_blockInfo.block_C;
        ETALTML_RDS_Strategy_AFData_Decode(slot, *((RDS_charIntTy*)&AFtemp));
#endif
	 
        if (DABMW_rdsData[slot].DABMW_pi.newValue >= DABMW_STORAGE_STATUS_IS_VERIFIED)
        {
            DABMW_RdsAfDataMngr (slot, DABMW_rdsData[slot].DABMW_pi.value, curFreq);
        }
    }

    // PS (RBDS)
    if (true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A && 
        true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B && 
        true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C && 
        true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D && 
        (DABMW_GROUP_15A == DABMW_rdsData[slot].DABMW_realGroup))
    {
#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf (0, TR_LEVEL_COMPONENT, TR_CLASS_APP_DABMW, 
            "DABMW RDS: PS (RBDS) found, blocks %c - %c - %c - %c on group # %d\n", 'A', 'B', 'C', 'D', DABMW_rdsData[slot].DABMW_realGroup);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT) 

        i = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_B) & 0x01;
        
        if ((DABMW_rdsData[slot].DABMW_blockInfo.block_C & 0x00FF) >= 0x20 && (DABMW_rdsData[slot].DABMW_blockInfo.block_C >> 8) >= 0x20 &&
            (DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x00FF) >= 0x20 && (DABMW_rdsData[slot].DABMW_blockInfo.block_D >> 8) >= 0x20)
        {
            switch(i)
            {
                case 0:
                    DABMW_rdsData[slot].DABMW_ps.get0 = true;  
                    DABMW_rdsData[slot].DABMW_ps.get1 = true; 
                    DABMW_rdsData[slot].DABMW_ps.get2 = false; 
                    DABMW_rdsData[slot].DABMW_ps.get3 = false;                     
                    DABMW_rdsData[slot].DABMW_ps.label[1] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_C & 0x00FF);
                    DABMW_rdsData[slot].DABMW_ps.label[0] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_C >> 8);
                    DABMW_rdsData[slot].DABMW_ps.label[3] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x00FF);
                    DABMW_rdsData[slot].DABMW_ps.label[2] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D >> 8);
                    break;
                case 1:
                    DABMW_rdsData[slot].DABMW_ps.get2 = true; 
                    DABMW_rdsData[slot].DABMW_ps.get3 = true; 
                    DABMW_rdsData[slot].DABMW_ps.label[5] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_C & 0x00FF);
                    DABMW_rdsData[slot].DABMW_ps.label[4] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_C >> 8);
                    DABMW_rdsData[slot].DABMW_ps.label[7] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x00FF);
                    DABMW_rdsData[slot].DABMW_ps.label[6] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D >> 8);
                    break;
            }
        }
        
        if (DABMW_rdsData[slot].DABMW_ps.get0 && DABMW_rdsData[slot].DABMW_ps.get1 && 
            DABMW_rdsData[slot].DABMW_ps.get2 && DABMW_rdsData[slot].DABMW_ps.get3)
        {
            DABMW_rdsData[slot].DABMW_ps.get0 = false;  
            DABMW_rdsData[slot].DABMW_ps.get1 = false; 
            DABMW_rdsData[slot].DABMW_ps.get2 = false; 
            DABMW_rdsData[slot].DABMW_ps.get3 = false; 

            // Compare the old label with the new one to check identity
            resCmp = OSAL_s32MemoryCompare ((tPVoid)&DABMW_rdsData[slot].DABMW_ps.labelBackup[0], (tPVoid)&DABMW_rdsData[slot].DABMW_ps.label[0], DABMW_RDS_PS_LENGTH);
                
            if (0 != resCmp)
            {
                OSAL_pvMemoryCopy ((tPVoid)&DABMW_rdsData[slot].DABMW_ps.labelBackup[0], (tPVoid)DABMW_rdsData[slot].DABMW_ps.label, DABMW_RDS_PS_LENGTH);

                // Signal a new label
                DABMW_rdsData[slot].DABMW_ps.newValue = DABMW_STORAGE_STATUS_IS_STORED;                    
            }

#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
            OSALUTIL_s32TracePrintf (0, TR_LEVEL_COMPONENT, TR_CLASS_APP_DABMW, 
                "DABMW RDS: New label stored, %s\n", &DABMW_rdsData[slot].DABMW_ps.label[0]);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)                   
        }
    }
    
    // EON
    DABMW_RdsEonDataMngr (slot, DABMW_rdsData[slot].DABMW_pi.value, DABMW_rdsData[slot].DABMW_realGroup,
    DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A, DABMW_rdsData[slot].DABMW_blockInfo.block_A, 
    DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B, DABMW_rdsData[slot].DABMW_blockInfo.block_B, 
    DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C, DABMW_rdsData[slot].DABMW_blockInfo.block_C, 
    DABMW_rdsData[slot].DABMW_blockInfo.availBlock_Cp, DABMW_rdsData[slot].DABMW_blockInfo.block_Cp, 
    DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D, DABMW_rdsData[slot].DABMW_blockInfo.block_D);
    
#ifdef CONFIG_ETALTML_HAVE_RDS_STRATEGY_EON
    ETALTML_RDS_Strategy_EONData_Decode (slot, DABMW_rdsData[slot].DABMW_pi.value, DABMW_rdsData[slot].DABMW_realGroup,
    DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A, DABMW_rdsData[slot].DABMW_blockInfo.block_A, 
    DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B, DABMW_rdsData[slot].DABMW_blockInfo.block_B, 
    DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C, DABMW_rdsData[slot].DABMW_blockInfo.block_C, 
    DABMW_rdsData[slot].DABMW_blockInfo.availBlock_Cp, DABMW_rdsData[slot].DABMW_blockInfo.block_Cp, 
    DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D, DABMW_rdsData[slot].DABMW_blockInfo.block_D);
#endif

    // RT 2A
    if (true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A && true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B && 
        (DABMW_GROUP_2A == DABMW_rdsData[slot].DABMW_realGroup) && true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C && true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D)
    {
#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf (0, TR_LEVEL_COMPONENT, TR_CLASS_APP_DABMW, 
            "DABMW RDS: RT 2A found, blocks %c - %c - %c - %c on group # %d\n", 'A', 'B', 'C', 'D', DABMW_rdsData[slot].DABMW_realGroup);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT) 

        flagVal = (tBool)((DABMW_rdsData[slot].DABMW_blockInfo.block_B >> (tU32)4) & (tU32)0x01);

        if (flagVal != DABMW_rdsData[slot].DABMW_rt.rtabFlag)
        {
            DABMW_rdsData[slot].DABMW_rt.rtabFlag = flagVal;                        

            DABMW_rdsData[slot].DABMW_rt.newValue = DABMW_STORAGE_STATUS_IS_EMPTY;
            DABMW_rdsData[slot].DABMW_rt.lastSegNum = -1;
            DABMW_rdsData[slot].DABMW_rt.blockPresent = 0;    

            OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_rt.rt[0], 0x00, 64);
        }
        
        i = ((tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_B) & (tU8)0x0F) * 4;

		
		// codex artefact #284598
		// manage correctly the rtabFlag 
		// IEC 62106, section 3.1.5.3 extract
		// An important feature of type 2 groups is the Text A/B flag contained in the second block. Two cases occur:
		//
		// - If the receiver detects a change in the flag (from binary "0" to binary "1" or vice-versa), then the whole RadioText
		// display shall be cleared and the newly received RadioText message segments shall be written into the display.
		//		==> that part is managed above with the rtabFlag
		//		==> Note that, at init, rtabFlag is not set, but the data are cleared
		//
		// - If the receiver detects no change in the flag, then the received text segments or characters shall be written into the
		// existing displayed message and those segments or characters for which no update is received shall be left unchanged.
		//
		// 		==> this is not managed
		//		==> In auto-mode, we should reset the flag from DABMW_STORAGE_STATUS_IS_RETRIEVED to VERIFIED for that case

		// add a flag and check if rt data changes
		vl_newRtByte = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_C >> 8);
		if (DABMW_rdsData[slot].DABMW_rt.rt[i] != vl_newRtByte)
			{
			vl_rtChange = true;
			}
		
		vl_newRtByte = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_C & 0x00FF);
		if (DABMW_rdsData[slot].DABMW_rt.rt[i+1] != vl_newRtByte)
			{
			vl_rtChange = true;
			}

		vl_newRtByte = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D >> 8);
		if (DABMW_rdsData[slot].DABMW_rt.rt[i+2] != vl_newRtByte)
			{
			vl_rtChange = true;
			}


		vl_newRtByte = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x00FF);
		if (DABMW_rdsData[slot].DABMW_rt.rt[i+3] != vl_newRtByte)
			{
			vl_rtChange = true;
			}
		
        DABMW_rdsData[slot].DABMW_rt.rt[i]     = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_C >> 8);
        DABMW_rdsData[slot].DABMW_rt.rt[i + 1] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_C & 0x00FF);
        DABMW_rdsData[slot].DABMW_rt.rt[i + 2] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D >> 8);
        DABMW_rdsData[slot].DABMW_rt.rt[i + 3] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x00FF);

        // Fill present blocks information
        DABMW_rdsData[slot].DABMW_rt.blockPresent |= ((tU64)0x01 << (tU64)i);
        DABMW_rdsData[slot].DABMW_rt.blockPresent |= ((tU64)0x01 << (tU64)(i + 1));
        DABMW_rdsData[slot].DABMW_rt.blockPresent |= ((tU64)0x01 << (tU64)(i + 2));
        DABMW_rdsData[slot].DABMW_rt.blockPresent |= ((tU64)0x01 << (tU64)(i + 3));
        
        // Check for last block
        if (63 == (i + 3))
        {
            DABMW_rdsData[slot].DABMW_rt.lastSegNum = 63;
        }
        else if ((tU8)0x0D == DABMW_rdsData[slot].DABMW_rt.rt[i])
        {
            DABMW_rdsData[slot].DABMW_rt.lastSegNum = i;
        }
        else if ((tU8)0x0D == DABMW_rdsData[slot].DABMW_rt.rt[i + 1])
        {
            DABMW_rdsData[slot].DABMW_rt.lastSegNum = i + 1;
        }
        else if ((tU8)0x0D == DABMW_rdsData[slot].DABMW_rt.rt[i + 2])
        {
            DABMW_rdsData[slot].DABMW_rt.lastSegNum = i + 2;
        }
        else if ((tU8)0x0D == DABMW_rdsData[slot].DABMW_rt.rt[i + 3])
        {
            DABMW_rdsData[slot].DABMW_rt.lastSegNum = i + 3;
        }

        // Check completeness
        if (DABMW_rdsData[slot].DABMW_rt.lastSegNum > -1)            
        {
            rtComplete = true;
            for (cnt = 0; cnt <= (tU64)DABMW_rdsData[slot].DABMW_rt.lastSegNum; cnt++)
            {
                if (0 == ((tU64)0x01 & (DABMW_rdsData[slot].DABMW_rt.blockPresent >> cnt)))
                {
                    rtComplete = false;
                    
                    break;
                }
            }
        }
        else
        {
            rtComplete = false;
        }

        if (true == rtComplete)
        {
            if (DABMW_STORAGE_STATUS_IS_RETRIEVED != DABMW_rdsData[slot].DABMW_rt.newValue)
            {
                DABMW_rdsData[slot].DABMW_rt.newValue = DABMW_STORAGE_STATUS_IS_VERIFIED;
            }
			else if (true == vl_rtChange )
			{
				// codex artefact #284598
				// we have a change in existing RT 
				// change status to verified to broadcast it in auto mode to user
				DABMW_rdsData[slot].DABMW_rt.newValue = DABMW_STORAGE_STATUS_IS_VERIFIED;
			}
        }
        else
        {
            DABMW_rdsData[slot].DABMW_rt.newValue = DABMW_STORAGE_STATUS_IS_STORED;
        }
    }

    // RT 2B
    if (true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A && true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B && 
        (DABMW_GROUP_2B == DABMW_rdsData[slot].DABMW_realGroup) && true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D)
    {
#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf (0, TR_LEVEL_COMPONENT, TR_CLASS_APP_DABMW, 
            "DABMW RDS: RT 2B found, blocks %c - %c - %c - %c on group # %d\n", 'A', 'B', 'D', DABMW_rdsData[slot].DABMW_realGroup);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT) 

        flagVal = (tBool)((DABMW_rdsData[slot].DABMW_blockInfo.block_B >> (tU32)4) & (tU32)0x01);

        if (flagVal != DABMW_rdsData[slot].DABMW_rt.rtabFlag)
        {
            DABMW_rdsData[slot].DABMW_rt.rtabFlag = flagVal;                            

            DABMW_rdsData[slot].DABMW_rt.newValue = DABMW_STORAGE_STATUS_IS_EMPTY;
            DABMW_rdsData[slot].DABMW_rt.lastSegNum = -1;
            DABMW_rdsData[slot].DABMW_rt.blockPresent = 0;    

            OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_rt.rt[0], 0x00, 64);
        }
        
        i = ((tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_B) & (tU8)0x0F) * 2;
        DABMW_rdsData[slot].DABMW_rt.rt[i]     = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D >> 8);
        DABMW_rdsData[slot].DABMW_rt.rt[i + 1] = (tU8)(DABMW_rdsData[slot].DABMW_blockInfo.block_D & 0x00FF);

        // Fill present blocks information
        DABMW_rdsData[slot].DABMW_rt.blockPresent |= ((tU64)0x01 << (tU64)i);
        DABMW_rdsData[slot].DABMW_rt.blockPresent |= ((tU64)0x01 << (tU64)(i + 1));

        // Check for last block
        if (63 == (i + 3))
        {
            DABMW_rdsData[slot].DABMW_rt.lastSegNum = 63;
        }
        else if ((tU8)0x0D == DABMW_rdsData[slot].DABMW_rt.rt[i])
        {
            DABMW_rdsData[slot].DABMW_rt.lastSegNum = i;
        }
        else if ((tU8)0x0D == DABMW_rdsData[slot].DABMW_rt.rt[i + 1])
        {
            DABMW_rdsData[slot].DABMW_rt.lastSegNum = i + 1;
        }

        // Check completeness
        if (DABMW_rdsData[slot].DABMW_rt.lastSegNum > -1)            
        {
            rtComplete = true;
            for (cnt = 0; cnt <= (tU64)DABMW_rdsData[slot].DABMW_rt.lastSegNum; cnt++)
            {
                if (0 == ((tU64)0x01 & (DABMW_rdsData[slot].DABMW_rt.blockPresent >> cnt)))
                {
                    rtComplete = false;
                    
                    break;
                }
            }
        }
        else
        {
            rtComplete = false;
        }

        if (true == rtComplete)
        {
            if (DABMW_STORAGE_STATUS_IS_RETRIEVED != DABMW_rdsData[slot].DABMW_rt.newValue)
            {
                DABMW_rdsData[slot].DABMW_rt.newValue = DABMW_STORAGE_STATUS_IS_VERIFIED;
            }
        }
        else
        {
            DABMW_rdsData[slot].DABMW_rt.newValue = DABMW_STORAGE_STATUS_IS_STORED;
        }

#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
        OSALUTIL_s32TracePrintf (0, TR_LEVEL_COMPONENT, TR_CLASS_APP_DABMW, 
            "DABMW RDS: New label stored, %s\n", &DABMW_rdsData[slot].DABMW_rt.rt[0]);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)              

    }    
	
	/* EPR Change
	*/

	// data have been proceed now we should clean info up  to next full block
	DABMW_rdsData[slot].DABMW_blockInfo.blocksComplete = false;
	DABMW_rdsData[slot].DABMW_rds.BlockCnt     = 0;
	DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A = false;
	DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B = false;
	DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C = false;
	DABMW_rdsData[slot].DABMW_blockInfo.availBlock_Cp = false;
	DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D = false;

	OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_blockInfo, 0x00, sizeof (DABMW_rdsData[slot].DABMW_blockInfo));
	

	DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A_used = false;
	DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B_used = false;
	DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C_used = false;
	DABMW_rdsData[slot].DABMW_blockInfo.availBlock_D_used = false;
   
	/* END EPR Change */
	
}

tVoid DABMW_CleanUpTPTA (tSInt slot)
{
    // Clear TA/TP
    DABMW_rdsData[slot].DABMW_taTp.newValue = DABMW_STORAGE_STATUS_IS_EMPTY; 
    OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_taTp, 0x00, sizeof(DABMW_rdsData[slot].DABMW_taTp));    
}

static tVoid DABMW_CleanUpData (tSInt slot)
{
    // Clear PTY
    DABMW_rdsData[slot].DABMW_pty.newValue = DABMW_STORAGE_STATUS_IS_EMPTY;  
    DABMW_rdsData[slot].DABMW_pty.value = 0;
    DABMW_rdsData[slot].DABMW_pty.lastValid = DABMW_INVALID_DATA_BYTE;       

    // Clear TA/TP
    DABMW_rdsData[slot].DABMW_taTp.newValue = DABMW_STORAGE_STATUS_IS_EMPTY; 
    OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_taTp, 0x00, sizeof(DABMW_rdsData[slot].DABMW_taTp));    

    // Clear PS   
    DABMW_rdsData[slot].DABMW_ps.newValue = DABMW_STORAGE_STATUS_IS_EMPTY;   
    DABMW_rdsData[slot].DABMW_ps.get0 = false;
    DABMW_rdsData[slot].DABMW_ps.get1 = false;
    DABMW_rdsData[slot].DABMW_ps.get2 = false;
    DABMW_rdsData[slot].DABMW_ps.get3 = false; 
    OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_ps.label[0], 0x00, DABMW_RDS_PS_LENGTH);
    OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_ps.labelBackup[0], 0x00, DABMW_RDS_PS_LENGTH);    

    // Clear time
    DABMW_rdsData[slot].DABMW_time.newCt = DABMW_STORAGE_STATUS_IS_EMPTY;  
    DABMW_rdsData[slot].DABMW_time.hour = 0;
    DABMW_rdsData[slot].DABMW_time.min = 0;
    DABMW_rdsData[slot].DABMW_time.mjd = 0;        

    // Clear RT
    DABMW_rdsData[slot].DABMW_rt.newValue = DABMW_STORAGE_STATUS_IS_EMPTY;
    DABMW_rdsData[slot].DABMW_rt.lastSegNum = -1;
    DABMW_rdsData[slot].DABMW_rt.blockPresent = 0;
    OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_rt.rt[0], 0x00, DABMW_RDS_RT_LENGTH);

    // Clear M/S
    // codex #284301
    // Valid values for MS are 0 or 1. 
    // last_valie should not be init to a valid value 
    //
    DABMW_rdsData[slot].DABMW_ms.value = 0;
    DABMW_rdsData[slot].DABMW_ms.lastValid = 0xFF;    
    DABMW_rdsData[slot].DABMW_ms.newValue = DABMW_STORAGE_STATUS_IS_EMPTY;

    // Clear DI
    DABMW_rdsData[slot].DABMW_diData.value = 0;
    DABMW_rdsData[slot].DABMW_diData.lastValid = DABMW_INVALID_DATA_BYTE;
    DABMW_rdsData[slot].DABMW_diData.newValue = DABMW_STORAGE_STATUS_IS_EMPTY;    
    
    // Clear PTY/TP/TA/M/S
    OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_ptyTpTaMs, 0x00, sizeof(DABMW_ptyTpTaMsTy));

    // Clear and initialize RDS raw
    OSAL_pvMemorySet ((tPVoid)&DABMW_rdsData[slot].DABMW_rds, 0x00, sizeof(DABMW_rdsTy));
    DABMW_rdsData[slot].DABMW_rds.lastGroup = DABMW_RDS_GROUP_NONE;

    // AF
    DABMW_RdsAfDataReInit (slot);
    DABMW_rdsData[slot].DABMW_af.afNew = DABMW_STORAGE_STATUS_IS_EMPTY;
#ifndef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
    DABMW_rdsData[slot].DABMW_af.afList = FALSE;
    DABMW_rdsData[slot].DABMW_af.afMethodB = FALSE;
    OSAL_pvMemorySet((tPVoid)DABMW_rdsData[slot].DABMW_af.af, 0x00, DABMW_AF_LIST_BFR_LEN_16BITS);
    DABMW_rdsData[slot].DABMW_af.afNum = 0;
    OSAL_pvMemorySet((tPVoid)DABMW_rdsData[slot].DABMW_af.afMethod, 0x00, 2);
#endif // !CONFIG_ETALTML_HAVE_AMFMLANDSCAPE

    // EON
    DABMW_RdsEonCleanUpData (slot);
}

#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE_FULL
tU32 DABMW_RdsGetErrorRatio (DABMW_storageSourceTy source)
{
    tSInt slot;

#ifdef ETAL_RDS_IMPORT
        slot = ETAL_receiverGetRDSSlot((ETAL_HANDLE)source);
#else
        slot = DABMW_RdsGetSlotFromSource (source);
#endif   

    if (DABMW_INVALID_SLOT == slot)
    {
        return 0;
    }

    return DABMW_rdsData[slot].DABMW_blockErrorRatio;
}
#endif

/* EPR CHANGE 
* add a dedicated procedure handling the PI for Block A and Block_C & D
*/

tBool DABMW_RdsPi_Processing(tSInt slot, DABMW_RDS_mwAppTy app, tU32 curFreq)
{

	tBool vl_PI_read = false;
	tBool vl_PI_verified = false;
    tU8 vl_GroupType;
	tU8 vl_cnt;
	tU8 vl_countInHistory = 0;
	tU8 vl_freeSpaceInHistory;
    tU8 vl_nextInHistory;

	// Check BLOCK A
	/* is it available on not already used ?
	*/
	if ((true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A) 
		&& (false == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A_used))
		{

		// GET PI
		DABMW_rdsData[slot].DABMW_pi.value = DABMW_rdsData[slot].DABMW_blockInfo.block_A;
		// Set block A as used
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_A_used = true;	
		vl_PI_read = true;
		}

	
	/* Check BLOCK B
	* PI in block C if group = 0x01 
	*/

	if ((true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B) && (true == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C) 
		&& ((false ==  DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B_used) && (false == DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C_used)))
		{
		// Get the group indication from the 5 MSB bits of the BLOCK B
		vl_GroupType = (tU8)((DABMW_rdsData[slot].DABMW_blockInfo.block_B >> 11) & (tU8)0x01); 
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_B_used = true;
		DABMW_rdsData[slot].DABMW_blockInfo.availBlock_C_used = true;
	
		if (0x01 == vl_GroupType)
			{
			// PI also available in Block C
						
			// PI
			DABMW_rdsData[slot].DABMW_pi.value = DABMW_rdsData[slot].DABMW_blockInfo.block_C;
			vl_PI_read = true;
			}
		}

	if (vl_PI_read)
		{
		/* check if new PI or confirmation ?
		*/
		// normal PI detection mode

		if (false == DABMW_rdsData[slot].DABMW_IsFastPiDetectionMode)
		{
			if (DABMW_rdsData[slot].DABMW_pi.value != DABMW_rdsData[slot].DABMW_pi.lastValid)
				{
				// New PI case
				
				// EPR TMP LOG
				// add a print information
				if (DABMW_STORAGE_STATUS_IS_EMPTY != DABMW_rdsData[slot].DABMW_pi.newValue)
					{
					// we receive a wrong PI
					// is it because decoding error ?
					//
					
#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT)
					OSALUTIL_s32TracePrintf (0, TR_LEVEL_SYSTEM, TR_CLASS_APP_DABMW, 
								"DABMW_RdsPi_Processing: rds_slot %d, New PI detected old 0x%04x, old_confidence_threshold %d, new 0x%04x\n", 
								slot,
								DABMW_rdsData[slot].DABMW_pi.lastValid, 
								DABMW_rdsData[slot].DABMW_pi.confidenceThr, 
								DABMW_rdsData[slot].DABMW_pi.value);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT) 
					}
				
				// Reset the threshold, it is a new value
				DABMW_rdsData[slot].DABMW_pi.confidenceThr = 0;
				DABMW_rdsData[slot].DABMW_pi.newValue = DABMW_STORAGE_STATUS_IS_STORED; 
					
				DABMW_rdsData[slot].DABMW_pi.lastValid = DABMW_rdsData[slot].DABMW_pi.value;

				}
			else
				{
				// confirmation PI case
				// Increment the threshold it is a duplicate, raise the confidence bar
				if (DABMW_rdsData[slot].DABMW_pi.confidenceThr < (tU8)0xFF)
					{
					DABMW_rdsData[slot].DABMW_pi.confidenceThr++;
					}
				}
			}
		else
			{
			// fast detection mode : loop on history
			vl_countInHistory = 0;
			vl_freeSpaceInHistory = 0xFF;
			
			for (vl_cnt=0;vl_cnt<DABMW_RDS_DATA_NB_MAX_PI_HISTORY; vl_cnt++)
				{
				if (DABMW_RDS_INIT_PI == DABMW_rdsData[slot].DABMW_pi.lastValidHistory[vl_cnt])
					{
					if (vl_freeSpaceInHistory == 0xFF)
						{
						// this is an empty space
						vl_freeSpaceInHistory = vl_cnt;

                        // in theory we may break : there are stored as received, so there should be nothing further...
						}
					}
				else if (DABMW_rdsData[slot].DABMW_pi.value == DABMW_rdsData[slot].DABMW_pi.lastValidHistory[vl_cnt])
					{
					// PI is already stored in history
					vl_countInHistory++;
					}
	
				}

            // check something found for storing
            //with below processing this should always be the case, but let's protect
			if (vl_freeSpaceInHistory == 0xFF)
			    {
                // we have been above the array. 
                // take the 0
                vl_freeSpaceInHistory = 0;
				}
            
            // store the PI in history
            DABMW_rdsData[slot].DABMW_pi.lastValidHistory[vl_freeSpaceInHistory] = DABMW_rdsData[slot].DABMW_pi.value;
			DABMW_rdsData[slot].DABMW_pi.lastValid = DABMW_rdsData[slot].DABMW_pi.value;

            // always make sure to get next free 
            if ((vl_freeSpaceInHistory+1) < DABMW_RDS_DATA_NB_MAX_PI_HISTORY)
                {
                    vl_nextInHistory = vl_freeSpaceInHistory+1;
                }
            else
                {
                    vl_nextInHistory = 0;
                }

            DABMW_rdsData[slot].DABMW_pi.lastValidHistory[vl_nextInHistory] = DABMW_RDS_INIT_PI;
           
                
			// now process : 
			// 
			if (vl_countInHistory > 0)
				{
				// The PI has been found in history
				//
				DABMW_rdsData[slot].DABMW_pi.confidenceThr = vl_countInHistory;                   
				}
			else
				{
				DABMW_rdsData[slot].DABMW_pi.newValue = DABMW_STORAGE_STATUS_IS_STORED; 
				DABMW_rdsData[slot].DABMW_pi.confidenceThr = 0;
				}

			// ADD debug prints
									
		}
		
		// Check if confidence has been reached
		if ((DABMW_rdsData[slot].DABMW_pi.confidenceThr >= DABMW_rdsData[slot].DABMW_criticalDataThr) 
			&& (DABMW_rdsData[slot].DABMW_pi.newValue < DABMW_STORAGE_STATUS_IS_VERIFIED)
	        && (DABMW_rdsData[slot].DABMW_pi.newValue != DABMW_STORAGE_STATUS_IS_EMPTY)
            /* END EPR Change */
			)
			{
			DABMW_rdsData[slot].DABMW_pi.newValue = DABMW_STORAGE_STATUS_IS_VERIFIED;
			if(DABMW_rdsData[slot].DABMW_pi.backupValue != DABMW_rdsData[slot].DABMW_pi.value) 
			{
				if (DABMW_rdsData[slot].DABMW_pi.value != 0) DABMW_rdsData[slot].DABMW_pi.backupValue = DABMW_rdsData[slot].DABMW_pi.value;
			}
	
			// Ask for current frequency
			if (DABMW_INVALID_FREQUENCY != curFreq)
				{
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
				// Call the db to set the entry: PI and freq
				DABMW_AmFmLandscapeSetPi (DABMW_rdsData[slot].DABMW_pi.value, curFreq);
#endif
	
				// Store the PI to the extraction structure
#ifdef ETAL_RDS_IMPORT
				// TODO impement SetApplicationService in ETAL
#else
				DABMW_SetApplicationService (app, DABMW_rdsData[slot].DABMW_pi.value, 
						DABMW_INVALID_DATA_BYTE, false, true, true);
#endif
				}					

			vl_PI_verified = true;
	
			}
		else if ((DABMW_rdsData[slot].DABMW_pi.confidenceThr >= DABMW_rdsData[slot].DABMW_criticalDataThr) &&
			(DABMW_STORAGE_STATUS_IS_VERIFIED == DABMW_rdsData[slot].DABMW_pi.newValue))
			{
			/* We should anyway update the PI store time !! */
#ifdef CONFIG_ETALTML_HAVE_AMFMLANDSCAPE
			// Call the db to set the entry: PI and freq
			DABMW_AmFmLandscapeUpdatePiStoreTime (DABMW_rdsData[slot].DABMW_pi.value, curFreq);
#endif
			
		}

#if defined (CONFIG_ETALTML_HAVE_RDS_STRATEGY) 
		if ((DABMW_rdsData[slot].DABMW_pi.confidenceThr >= DABMW_rdsData[slot].DABMW_criticalDataThr) &&
		     ((DABMW_STORAGE_STATUS_IS_VERIFIED == DABMW_rdsData[slot].DABMW_pi.newValue) ||
		      (DABMW_STORAGE_STATUS_IS_USED == DABMW_rdsData[slot].DABMW_pi.newValue)))
		 {
			//Verfied PI,  start / refresh PI Seek Timer in RDS strategy
			if(DABMW_rdsData[slot].DABMW_pi.backupValue == DABMW_rdsData[slot].DABMW_pi.value) 
			{
				ETALTML_RDS_Strategy_ResetPISeekTimer(slot, FALSE);
			}
		}
#endif

		}

	return vl_PI_verified;

}

// Procedure to control the PI detection mode : FAST or normal
tSInt DABMW_RdsDataSetupPiDetectionMode (DABMW_storageSourceTy source, tBool vI_Isfast_PI_detection)
{

	tSInt vl_slot;
	tSInt vl_res = OSAL_OK;
	
	// Get the slot to use from the source
    vl_slot = DABMW_RdsGetSlotFromSource (source);

    if (DABMW_INVALID_SLOT == vl_slot)
    	{	
    	return (OSAL_ERROR);
    	}

	// set the PI detection mode
	DABMW_rdsData[vl_slot].DABMW_IsFastPiDetectionMode = vI_Isfast_PI_detection;

	return vl_res;
	
}

//Specific trick to increase the PI detection 
// If in FAST PI detection mode
// if we know which PI we target on the frequency : case of AF freq
// we can consider the PI matching detection to be 1 
// ie : AF = PI_1, PI reception of PI_1 one time should be enough
// the probability to have PI_1 decoded wrongly is low... 
// that should enhance the PI detection
//
tSInt DABMW_RdsDataSetupIncreasePiMatchingAF (DABMW_storageSourceTy source, tU32 vI_expected_PI)
{

	tSInt vl_slot;
	tSInt vl_res = OSAL_ERROR;
    tU8 vl_cnt;
    
	// Get the slot to use from the source
    vl_slot = DABMW_RdsGetSlotFromSource (source);

    if (DABMW_INVALID_SLOT == vl_slot)
    	{	
    	return (OSAL_ERROR);
    	}

    // if not in fast detection, do nothing
    if (false == DABMW_rdsData[vl_slot].DABMW_IsFastPiDetectionMode)
        {
        return (OSAL_ERROR);   
        }
    
    // fast detection mode : loop on history
    
    for (vl_cnt=0;vl_cnt<DABMW_RDS_DATA_NB_MAX_PI_HISTORY; vl_cnt++)
        {
        if (DABMW_RDS_INIT_PI == DABMW_rdsData[vl_slot].DABMW_pi.lastValidHistory[vl_cnt])
            {
            DABMW_rdsData[vl_slot].DABMW_pi.lastValidHistory[vl_cnt] = vI_expected_PI; 
            vl_res = OSAL_OK;
#if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_USER_1)
      OSALUTIL_s32TracePrintf (0, TR_LEVEL_USER_1, TR_CLASS_APP_DABMW, 
                                    "DABMW_RdsDataSetupIncreasePiMatchingAF: rds_slot %d, New PI 0x%04x, room in history %d\n",  
                                    vl_slot, vI_expected_PI, vl_cnt);
#endif // #if defined (CONFIG_ENABLE_CLASS_APP_DABMW) && (CONFIG_ENABLE_CLASS_APP_DABMW >= TR_LEVEL_COMPONENT) 
            break;
            }      
        }


    // cheat as well on last_valid pi
    DABMW_rdsData[vl_slot].DABMW_pi.lastValid = vI_expected_PI;
    DABMW_rdsData[vl_slot].DABMW_pi.newValue = DABMW_STORAGE_STATUS_IS_STORED; 

	return vl_res;
	
}


/* END EPR CHANGE */

#endif // CONFIG_ETALTML_HAVE_RDS
// End of file

