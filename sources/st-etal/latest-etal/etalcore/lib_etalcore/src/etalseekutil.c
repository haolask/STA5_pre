//!
//!  \file 		etalseekutil.c
//!  \brief 	<i><b> ETAL utilities </b></i>
//!  \details   Various utilities needed to support DAB manual seek and TML
//!  $Author$
//!  \author 	(original version) Raffaele Belardi
//!  $Revision$
//!  $Date$
//!


#include "osal.h"
#include "etalinternal.h"

#if  defined (CONFIG_ETAL_HAVE_ETALTML) || \
	(defined (CONFIG_ETAL_SUPPORT_DCOP_MDR) && defined (CONFIG_ETAL_HAVE_ALL_API)) || \
	(defined (CONFIG_ETAL_SUPPORT_DCOP_MDR) && defined (CONFIG_ETAL_HAVE_MANUAL_SEEK))

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/

// Radio Band Information
typedef struct
{
    DABMW_systemBandsTy band;
    tU32 minFreq;
    tU32 maxFreq;
    tU32 step;
    DABMW_mwCountryTy country;
} DABMW_radioBandInfoTy;

// DAB Frequency
typedef struct
{
    tChar name[10];
    tU32 value;
} DABMW_DAB_Frequency;

#ifdef CONFIG_ETAL_MDR_DABMW_ON_HOST
extern DABMW_radioBandInfoTy DABMW_radioBandInfoTable[DABMW_AVAILABLE_BAND_NUM];

#else// ndef CONFIG_ETAL_MDR_DABMW_ON_HOST

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/

static DABMW_mwCountryTy DABMW_countryId = DABMW_COUNTRY_EUROPE;

static const DABMW_radioBandInfoTy DABMW_radioBandInfoTable[DABMW_AVAILABLE_BAND_NUM] =
{
    // freq_band                     freq_min    freq_max    freq_step      country
    {DABMW_BAND_NONE,                       0,         0,            0,     DABMW_COUNTRY_NONE   },

	//-------- FM  (HF)
	{DABMW_BAND_FM_EU,                  87500,    108000,           50,     DABMW_COUNTRY_EUROPE },  //This supports  EUROPE, CHINA, CANADA, KOREA
    {DABMW_BAND_FM_US,                  87900,    107900,          200,     DABMW_COUNTRY_USA    },
    {DABMW_BAND_FM_JAPAN,               76000,     90000,          100,     DABMW_COUNTRY_JAPAN  },
    {DABMW_BAND_FM_EAST_EU,             65000,     74000,          100,     DABMW_COUNTRY_EAST_EU},

	//-------- FM  (WX - Weather Band)
    {DABMW_BAND_FM_WEATHER_US,         162400,    162550,           25,     DABMW_COUNTRY_USA    },

	//-------- AM Middle Wave (MW)
    {DABMW_BAND_AM_MW_EU,                 531,      1629,            9,     DABMW_COUNTRY_EUROPE },
    {DABMW_BAND_AM_MW_US,                 530,      1730,           10,     DABMW_COUNTRY_USA    },
    {DABMW_BAND_AM_MW_JAPAN,              531,      1629,            9,     DABMW_COUNTRY_JAPAN  },
    {DABMW_BAND_AM_MW_EAST_EU,            531,      1629,            9,     DABMW_COUNTRY_EAST_EU},

    //-------- DAB BAND-III (VHF)
    {DABMW_BAND_DAB_III,               174928,    239200,         1712,     DABMW_COUNTRY_EUROPE },
    {DABMW_BAND_CHINA_DAB_III,         168160,    221568,         1712,     DABMW_COUNTRY_CHINA  },
    {DABMW_BAND_KOREA_DAB_III,         175280,    214736,         1728,     DABMW_COUNTRY_KOREA  },
   
    //-------- DAB BAND-L  
    {DABMW_BAND_DAB_L,                1452960,   1490624,         1712,     DABMW_COUNTRY_EUROPE },
    {DABMW_BAND_CANADA_DAB_L,         1452816,   1491184,         1744,     DABMW_COUNTRY_CANADA },

	//-------- AM Long Wave (LW)
	{DABMW_BAND_AM_LW,                    144,       288,            9,     DABMW_COUNTRY_ANY },  

    //-------- AM Short Wave (SW)
	{DABMW_BAND_AM_SW1,                  4039,      5180,            1,     DABMW_COUNTRY_ANY },
	{DABMW_BAND_AM_SW2,                  5720,      6330,            1,     DABMW_COUNTRY_ANY },
	{DABMW_BAND_AM_SW3,                  7000,      8050,            1,     DABMW_COUNTRY_ANY },
	{DABMW_BAND_AM_SW4,                  9200,     10020,            1,     DABMW_COUNTRY_ANY },
	{DABMW_BAND_AM_SW5,                 11035,     12250,            1,     DABMW_COUNTRY_ANY },
	{DABMW_BAND_AM_SW6,                 13250,     14280,            1,     DABMW_COUNTRY_ANY },
	{DABMW_BAND_AM_SW7,                 15000,     16050,            1,     DABMW_COUNTRY_ANY },
	{DABMW_BAND_AM_SW8,                 17300,     18180,            1,     DABMW_COUNTRY_ANY },
	{DABMW_BAND_AM_SW9,                 18700,     19250,            1,     DABMW_COUNTRY_ANY },
	{DABMW_BAND_AM_SW10,                21200,     22300,            1,     DABMW_COUNTRY_ANY },

	// DRM30 : band may be on AM / MW or LW 
	// this table is not used for DRM30, so put for now nothing
	{DABMW_BAND_DRM30,                	0,     0,            		 0,     DABMW_COUNTRY_ANY },
};

static const DABMW_DAB_Frequency DABMW_DAB_Frequency_ChinaBIII_Table[] =
{
    // Band III China
    {  "6A", 168160 },
    {  "6B", 169872 },
    {  "6C", 171584 },
    {  "6D", 173296 },
    {  "6N", 175008 },
    {  "7A", 176720 },
    {  "7B", 178432 },
    {  "7C", 180144 },
    {  "7D", 181856 },
    {  "8A", 184160 },
    {  "8B", 185872 },
    {  "8C", 187584 },
    {  "8D", 189296 },
    {  "8N", 191008 },
    {  "9A", 192720 },
    {  "9B", 194432 },
    {  "9C", 196144 },
    {  "9D", 197856 },
    {  "10A", 200160 },
    {  "10B", 201872 },
    {  "10C", 203584 },
    {  "10D", 205296 },
    {  "10N", 207008 },
    {  "11A", 208720 },
    {  "11B", 210432 },
    {  "11C", 212144 },
    {  "11D", 213856 },
    {  "12A", 216432 },
    {  "12B", 218144 },
    {  "12C", 219856 },
    {  "12D", 221568 },
    {  "",    0      }
};

static const DABMW_DAB_Frequency DABMW_DAB_Frequency_KoreaBIII_Table[] =
{
    // Band III Korea
    {  "7A", 175280 },
    {  "7B", 177008 },
    {  "7C", 178736 },
    {  "8A", 181280 },
    {  "8B", 183008 },
    {  "8C", 184736 },
    {  "9A", 187280 },
    {  "9B", 189008 },
    {  "9C", 190736 },
    {  "10A", 193280 },
    {  "10B", 195008 },
    {  "10C", 196736 },
    {  "11A", 199280 },
    {  "11B", 201008 },
    {  "11C", 202736 },
    {  "12A", 205280 },
    {  "12B", 207008 },
    {  "12C", 208736 },
    {  "13A", 211280 },
    {  "13B", 213008 },
    {  "13C", 214736 },
    {  "",    0      }
};

static const DABMW_DAB_Frequency DABMW_DAB_Frequency_CanadaBL_Table[] =
{
    // Band L Canada
    { "L1", 1452816 },
    { "L2", 1454560 },
    { "L3", 1456304 },
    { "L4", 1458048 },
    { "L5", 1459792 },
    { "L6", 1461536 },
    { "L7", 1463280 },
    { "L8", 1465024 },
    { "L9", 1466768 },
    { "L10", 1468512 },
    { "L11", 1470256 },
    { "L12", 1472000 },
    { "L13", 1473744 },
    { "L14", 1475488 },
    { "L15", 1477232 },
    { "L16", 1478976 },
    { "L17", 1480720 },
    { "L18", 1482464 },
    { "L19", 1484208 },
    { "L20", 1485952 },
    { "L21", 1487696 },
    { "L22", 1489440 },
    { "L23", 1491184 },
    { "",    0       }
};

static const DABMW_DAB_Frequency DABMW_DAB_Frequency_BIII_Table[] =
{
    // Band III Europe
    {  "5A", 174928 },
    {  "5B", 176640 },
    {  "5C", 178352 },
    {  "5D", 180064 },
    {  "6A", 181936 },
    {  "6B", 183648 },
    {  "6C", 185360 },
    {  "6D", 187072 },
    {  "7A", 188928 },
    {  "7B", 190640 },
    {  "7C", 192352 },
    {  "7D", 194064 },
    {  "8A", 195936 },
    {  "8B", 197648 },
    {  "8C", 199360 },
    {  "8D", 201072 },
    {  "9A", 202928 },
    {  "9B", 204640 },
    {  "9C", 206352 },
    {  "9D", 208064 },
    { "10A", 209936 },
    { "10N", 210096 },
    { "10B", 211648 },
    { "10C", 213360 },
    { "10D", 215072 },
    { "11A", 216928 },
    { "11N", 217088 },
    { "11B", 218640 },
    { "11C", 220352 },
    { "11D", 222064 },
    { "12A", 223936 },
    { "12N", 224096 },
    { "12B", 225648 },
    { "12C", 227360 },
    { "12D", 229072 },
    { "13A", 230784 },
    { "13B", 232496 },
    { "13C", 234208 },
    { "13D", 235776 },
    { "13E", 237488 },
    { "13F", 239200 },
    { "",    0      }
};

static const DABMW_DAB_Frequency DABMW_DAB_Frequency_BL_Table[] =
{
    // Band L Europe
    { "LA", 1452960 },
    { "LB", 1454672 },
    { "LC", 1456384 },
    { "LD", 1458096 },
    { "LE", 1459808 },
    { "LF", 1461520 },
    { "LG", 1463232 },
    { "LH", 1464944 },
    { "LI", 1466656 },
    { "LJ", 1468368 },
    { "LK", 1470080 },
    { "LL", 1471792 },
    { "LM", 1473504 },
    { "LN", 1475216 },
    { "LO", 1476928 },
    { "LP", 1478640 },
    { "LQ", 1480352 },
    { "LR", 1482064 },
    { "LS", 1483776 },
    { "LT", 1485488 },
    { "LU", 1487200 },
    { "LV", 1488912 },
    { "LW", 1490624 },
    { "",   0       }
};


static tSInt DABMW_SizeOfBandTable(const DABMW_DAB_Frequency* bandTable);

static tSInt DABMW_DabFrequencyToIndex(const DABMW_DAB_Frequency* band, tU32 frequency);

static tSInt DABMW_MaxindexOfBandValue(DABMW_systemBandsTy bandValue);

static tSInt DABMW_AmFmFrequencyToIndex(DABMW_systemBandsTy bandValue, tU32 frequency,  tU32 offset);

static tBool DABMW_IsFrequencyInBand(DABMW_systemBandsTy bandValue, tU32 frequency);

static tBool DABMW_IsFrequencyInBandTable(const DABMW_DAB_Frequency* band, tU32 frequency);

static const DABMW_DAB_Frequency* DABMW_GetDabBandTablefromFrequency(ETAL_HANDLE hReceiver, tU32 frequency, DABMW_mwCountryTy country);

/*****************************************************************
| Local types
|----------------------------------------------------------------*/

static tBool DABMW_IsFrequencyInBand(DABMW_systemBandsTy bandValue, tU32 frequency)
{
    return (((tU32)(DABMW_radioBandInfoTable[bandValue].maxFreq) >= frequency) &&
            ((tU32)(DABMW_radioBandInfoTable[bandValue].minFreq) <= frequency));
}


DABMW_mwCountryTy DABMW_GetCountry (tVoid)
{
    return DABMW_countryId;
}

tVoid DABMW_SetCountry (DABMW_mwCountryTy country)
{
    DABMW_countryId = country;
}

static tSInt DABMW_SizeOfBandTable(const DABMW_DAB_Frequency* bandTable)
{

    tSInt index = 0;

    while (bandTable[index].value != 0)
    {
        index++;
    }

    return index;
}

static tSInt DABMW_DabFrequencyToIndex(const DABMW_DAB_Frequency* band, tU32 frequency)
{
    tSInt i;
    tSInt index = -1;  // not found

    for (i = 0; i < DABMW_SizeOfBandTable(band); i++)
    {
        if (band[i].value == frequency)
        {
            index = i;
            break;
        }
    }

    return index;
}

static tSInt DABMW_MaxindexOfBandValue(DABMW_systemBandsTy bandValue)
{
    tSInt sizeOfBandValue, indexOfBandValue = 0;

    if (DABMW_BAND_NONE != bandValue && (DABMW_IsFMBand(bandValue) || DABMW_IsAMBand(bandValue)))
    {
        // TODO should there be a protection against negative values?
        sizeOfBandValue   = (tSInt)(DABMW_radioBandInfoTable[bandValue].maxFreq - DABMW_radioBandInfoTable[bandValue].minFreq);
        indexOfBandValue = sizeOfBandValue / DABMW_radioBandInfoTable[bandValue].step;
    }

    return indexOfBandValue;
}


static tSInt DABMW_AmFmFrequencyToIndex(DABMW_systemBandsTy bandValue, tU32 frequency,  tU32 offset)
{
    tSInt  index = 0;  // not found
    tU32 previousFreq, deltaFrequency;

    if (offset == 0)
    {
        previousFreq =  DABMW_radioBandInfoTable[bandValue].minFreq;
    }
    else
    {
        previousFreq = offset ;
    }

    if (DABMW_IsFrequencyInBand(bandValue, frequency) && DABMW_IsFrequencyInBand(bandValue, previousFreq))
    {
        if (previousFreq <= frequency)
        {
            deltaFrequency  = (frequency - previousFreq);
        }
        else
        {
            deltaFrequency  = (previousFreq - frequency);
        }

        index  =  (tSInt) deltaFrequency / DABMW_radioBandInfoTable[bandValue].step;

    }
    else
    {
        index = -1 ;
    }

    return index;
}

/* TML, DAB seek */
tU32 DABMW_GetNextFrequencyFromFreq (tU32 frequency, DABMW_systemBandsTy systemBand, tBool up)
{
    tU32 frequency_step = 0;
    tU32 frequency_min = DABMW_INVALID_FREQUENCY;
    tS32 freqIndex = -1;
		tU32 retval;

    int band_table_size = 0;
    const DABMW_DAB_Frequency* p_bandDAB = NULL;

    DABMW_mwCountryTy countryId;

	tBool bearer_isDab = false;

    // Get the Country Id storend into database
    countryId = DABMW_GetCountry();

	if (DABMW_IsFMBand(systemBand))
		{
		bearer_isDab = false;
		}
	else if (DABMW_IsDABBand (systemBand))
		{
		bearer_isDab = true;
		}
	else
		{
		frequency = DABMW_INVALID_FREQUENCY;
		bearer_isDab = false;
		}


    // Get the band pointer and frequency index: DAB uses a fixed array table, AM/FM use a relative index addressing
    if (DABMW_INVALID_FREQUENCY != frequency)
    {
        if (true == bearer_isDab)
        {
            // Get the DAB band table parameters
            /* app not used : so set to none */
            p_bandDAB = DABMW_GetDabBandTablefromFrequency(DABMW_NONE_APP, frequency, countryId);
            band_table_size = DABMW_SizeOfBandTable(p_bandDAB); //Size of the array (number of items)
            freqIndex = DABMW_DabFrequencyToIndex(p_bandDAB, frequency); //relative index of initial frequency

        }
        else 
        {
            // Get the AM/FM band table parameter/
            band_table_size = DABMW_MaxindexOfBandValue(systemBand); //Number of frequency channels/indexes
            freqIndex = DABMW_AmFmFrequencyToIndex(systemBand, frequency, 0); //relative index of initial frequency from the lower band limit
            frequency_step = DABMW_GetBandInfoTable_step(systemBand, countryId) ;
            frequency_min = DABMW_GetSystemBandMinFreq(systemBand) ;
        }
    }

    // Select automatically the first frequency in case not any radio control or tune command have been issued yet
    // Default band limits are used for each application
    if ((-1 != freqIndex) && ((NULL != p_bandDAB) || (DABMW_BAND_NONE != systemBand)) && (band_table_size != 0)) // band_table_size check redundant but avoids lint warning
    {
        // Select Frequency Value depending on the direction
        if (true == up)
        {
            //Get next index (forward)
            freqIndex = (freqIndex+1) % band_table_size;
        }
        else
        {
            //Get previous index (backward)
            freqIndex = (band_table_size + (freqIndex-1)) % band_table_size;
        }

        //Get next frequency (forward/backward)
        if ((true == bearer_isDab) && (p_bandDAB != NULL)) // p_bandDAB check redundant but avoids lint warning
        {
            frequency = p_bandDAB[freqIndex].value;
        }
        else if (false == bearer_isDab)
        {
            frequency = frequency_min + (freqIndex* frequency_step);
        }
				else
				{
					/* Nothing to do */
				}

    }
    else
    {
        retval = DABMW_INVALID_FREQUENCY;
        goto exit;
    }

		retval = frequency;
		
exit:
    return retval;
}

static tBool DABMW_IsFrequencyInBandTable(const DABMW_DAB_Frequency* band, tU32 frequency)
{
    tU32 min = band[0].value;
    tU32 max = band[DABMW_SizeOfBandTable(band) - 1].value;
		tBool retval;

    if (frequency >= min && frequency <= max)
		{ 
        retval = TRUE;
		}
		else
		{
			  retval = FALSE;
		}

    return retval;
}

static const DABMW_DAB_Frequency* DABMW_GetDabBandTablefromFrequency(ETAL_HANDLE hReceiver, tU32 frequency, DABMW_mwCountryTy country)
{
    const DABMW_DAB_Frequency* p_DAB_bandTable;

    switch (DABMW_GetReceiverTableSystemBand (hReceiver, frequency, country))
    {

        case DABMW_BAND_DAB_III:
            p_DAB_bandTable = DABMW_DAB_Frequency_BIII_Table;
        break;
            
        case DABMW_BAND_CHINA_DAB_III:
            p_DAB_bandTable = DABMW_DAB_Frequency_ChinaBIII_Table;
        break;
            
        case DABMW_BAND_KOREA_DAB_III:
            p_DAB_bandTable = DABMW_DAB_Frequency_KoreaBIII_Table;
        break;
            
        case DABMW_BAND_DAB_L:
            p_DAB_bandTable = DABMW_DAB_Frequency_BL_Table;
        break;
            
        case DABMW_BAND_CANADA_DAB_L:
            p_DAB_bandTable = DABMW_DAB_Frequency_CanadaBL_Table;
        break;
            
        default:
            p_DAB_bandTable = NULL;
        break;

    }

    /* Control if the frequency is contained onto the DAB tables */
    if (FALSE == DABMW_IsFrequencyInBandTable(p_DAB_bandTable, frequency))
    {
        p_DAB_bandTable = NULL;
    }

    return p_DAB_bandTable;
}

tU32 DABMW_GetBandInfoTable_step (DABMW_systemBandsTy bandValue, DABMW_mwCountryTy countryId)
{
   tSInt cnt;
		tU32 retval = DABMW_INVALID_DATA;
		
    // Find the lower frequency of belonging for the selected band and country
    if (DABMW_COUNTRY_NONE != countryId)
    {
        for (cnt = 0; cnt < DABMW_AVAILABLE_BAND_NUM; cnt++)
        {
            if (DABMW_radioBandInfoTable[cnt].country == countryId && DABMW_radioBandInfoTable[cnt].band == bandValue )
            {
                retval = DABMW_radioBandInfoTable[cnt].step;
                goto exit;
            }
        }
    }

exit:
   return retval;
}

tU32 DABMW_GetSystemBandMinFreq (DABMW_systemBandsTy bandValue)
{
    tSInt cnt;
		tU32 retval = DABMW_INVALID_FREQUENCY;

    // Find the lower frequency of belonging for the selected band and country
    if (DABMW_COUNTRY_NONE != DABMW_GetCountry())
    {
        for (cnt = 0; cnt < DABMW_AVAILABLE_BAND_NUM; cnt++)
        {
            if (DABMW_radioBandInfoTable[cnt].country == DABMW_GetCountry() &&
                DABMW_radioBandInfoTable[cnt].band == bandValue)
            {
                retval = DABMW_radioBandInfoTable[cnt].minFreq;
                goto exit;
            }
        }
    }

exit:
    return retval;
}
#endif

/* TML, DAB seek*/
DABMW_systemBandsTy DABMW_TranslateEtalBandToDabmwBand(EtalFrequencyBand vI_etalBand)
{
	DABMW_systemBandsTy vl_dabmwBand = DABMW_BAND_NONE;


	switch(vI_etalBand)
	{
		case ETAL_BAND_FMEU:
			vl_dabmwBand = DABMW_BAND_FM_EU;
			break;
		case ETAL_BAND_FMUS:
			vl_dabmwBand = DABMW_BAND_FM_US;
			break;
		case ETAL_BAND_FMJP:
			vl_dabmwBand = DABMW_BAND_FM_JAPAN;
			break;
		case ETAL_BAND_FMEEU:
			vl_dabmwBand = DABMW_BAND_FM_EAST_EU;
			break;
		case ETAL_BAND_WB:
			vl_dabmwBand = DABMW_BAND_FM_WEATHER_US;
			break;

		case ETAL_BAND_MWEU:
			vl_dabmwBand = DABMW_BAND_AM_MW_EU;
			break;
		case ETAL_BAND_MWUS:
			vl_dabmwBand = DABMW_BAND_AM_MW_US;
			break;

		case ETAL_BAND_DAB3:
			vl_dabmwBand = DABMW_BAND_DAB_III;
			break;
		case ETAL_BAND_DABL:
			vl_dabmwBand = DABMW_BAND_DAB_L;
			break;

		case ETAL_BAND_LW:
			vl_dabmwBand = DABMW_BAND_AM_LW;
			break;
		case ETAL_BAND_SW:
			vl_dabmwBand = DABMW_BAND_AM_SW1;
			break;

		case ETAL_BAND_DRM30:
			vl_dabmwBand = DABMW_BAND_DRM30;
			break;

		default:
			vl_dabmwBand = DABMW_BAND_NONE;
			break;
	}

	return(vl_dabmwBand);

}


// This functions check if the band that is going to be selected is different
// from the current one and in case return true
DABMW_systemBandsTy DABMW_GetReceiverTableSystemBand (ETAL_HANDLE hReceiver, tU32 frequency, DABMW_mwCountryTy countryId)
{
    tSInt cnt;
    DABMW_systemBandsTy retval = DABMW_BAND_NONE;
    
    // For DRM the band cannot be deduced from the frequency and the country 
    // information so a different approach is used to return the proper band.
    // If DABMW_NONE_APP is passed the normal checks are applied and the band
    // is considered not DRM
    if (((ETAL_INVALID_HANDLE != hReceiver) && (ETAL_BCAST_STD_DRM != ETAL_receiverGetStandard(hReceiver)))
		|| (ETAL_INVALID_HANDLE == hReceiver))
    {
        // Find the band of belonging for the selected frequency
        if (DABMW_COUNTRY_NONE != countryId)
        {
            for (cnt = 0; cnt < DABMW_AVAILABLE_BAND_NUM; cnt++)
            {
                if ((DABMW_radioBandInfoTable[cnt].country == countryId || 
				     DABMW_COUNTRY_ANY == DABMW_radioBandInfoTable[cnt].country)  &&
                    DABMW_radioBandInfoTable[cnt].maxFreq >= frequency &&
                    DABMW_radioBandInfoTable[cnt].minFreq <= frequency)
                {
                    retval = DABMW_radioBandInfoTable[cnt].band;
                    goto exit;
                }
            }
        }
        else
        {
            // We use the global country ID because an invalid one was passed
            for (cnt = 0; cnt < DABMW_AVAILABLE_BAND_NUM; cnt++)
            {
                if (DABMW_radioBandInfoTable[cnt].country == DABMW_GetCountry() &&
                    DABMW_radioBandInfoTable[cnt].maxFreq >= frequency &&
                    DABMW_radioBandInfoTable[cnt].minFreq <= frequency)
                {
                    retval = DABMW_radioBandInfoTable[cnt].band;
                    goto exit;
                }
            }
        }
    }
    else
    {
        retval = DABMW_BAND_DRM30; 
        goto exit;
    }
 
 exit:       
    return retval;
}
#endif //#ifndef CONFIG_ETAL_MDR_DABMW_ON_HOST

tBool DABMW_IsFMBand (DABMW_systemBandsTy band)
{
    return ((DABMW_BAND_FM_EU == band) || (DABMW_BAND_FM_US == band) || \
            (DABMW_BAND_FM_EAST_EU == band ) || (DABMW_BAND_FM_JAPAN == band ) || (DABMW_BAND_FM_WEATHER_US == band));
}

tBool DABMW_IsAMBand (DABMW_systemBandsTy band)
{
    return ((DABMW_BAND_AM_MW_EU == band) || (DABMW_BAND_AM_MW_US == band) || \
            (DABMW_BAND_AM_MW_EAST_EU == band ) || (DABMW_BAND_AM_MW_JAPAN == band ));
}

tBool DABMW_IsDABBand (DABMW_systemBandsTy band)
{
    return ((DABMW_BAND_DAB_III == band) ||(DABMW_BAND_CHINA_DAB_III == band) || (DABMW_BAND_KOREA_DAB_III == band) || \
            (DABMW_BAND_DAB_L == band ) || (DABMW_BAND_CANADA_DAB_L == band ));
}


#ifdef CONFIG_ETAL_HAVE_ETALTML

/* These utilities are used only in ETAL TML but they reference some variables
 * defined in this file
 */
tU32 DABMW_GetSystemBandMaxFreq (DABMW_systemBandsTy bandValue)
{
    tSInt cnt;
		tU32 retval = DABMW_INVALID_FREQUENCY;
		
    // Find the lower frequency of belonging for the selected band and country
    if (DABMW_COUNTRY_NONE != DABMW_GetCountry())
    {
        for (cnt = 0; cnt < DABMW_AVAILABLE_BAND_NUM; cnt++)
        {
            if (DABMW_radioBandInfoTable[cnt].country == DABMW_GetCountry() &&
                DABMW_radioBandInfoTable[cnt].band == bandValue)
            {
                retval = DABMW_radioBandInfoTable[cnt].maxFreq;
                goto exit;
            }
        }
    }

exit:
    return retval;
}

EtalFrequencyBand DABMW_TranslateDabmwBandToEtalBand(DABMW_systemBandsTy vI_dabmwBand)
{
	EtalFrequencyBand vl_etalBand = ETAL_BAND_UNDEF;


	switch(vI_dabmwBand)
	{
		case DABMW_BAND_FM_EU:
			vl_etalBand = ETAL_BAND_FMEU;
			break;
		case DABMW_BAND_FM_US:
			vl_etalBand = ETAL_BAND_FMUS;
			break;
		case DABMW_BAND_FM_JAPAN:
			vl_etalBand = ETAL_BAND_FMJP;
			break;
		case DABMW_BAND_FM_EAST_EU:
			vl_etalBand = ETAL_BAND_FMEEU;
			break;
		case DABMW_BAND_FM_WEATHER_US:
			vl_etalBand = ETAL_BAND_WB;
			break;

		case DABMW_BAND_AM_MW_EU:
			vl_etalBand = ETAL_BAND_MWEU;
			break;
		case DABMW_BAND_AM_MW_US:
			vl_etalBand = ETAL_BAND_MWUS;
			break;
		case DABMW_BAND_AM_MW_JAPAN:
			vl_etalBand = ETAL_BAND_MWUS;
			break;
		case DABMW_BAND_AM_MW_EAST_EU:
			vl_etalBand = ETAL_BAND_MWEU;
			break;

		case DABMW_BAND_DAB_III:
			vl_etalBand = ETAL_BAND_DAB3;
			break;
		case DABMW_BAND_CHINA_DAB_III:
			vl_etalBand = ETAL_BAND_DAB3;
			break;
		case DABMW_BAND_KOREA_DAB_III:
			vl_etalBand = ETAL_BAND_DAB3;
			break;
		case DABMW_BAND_DAB_L:
			vl_etalBand = ETAL_BAND_DABL;
			break;
		case DABMW_BAND_CANADA_DAB_L:
			vl_etalBand = ETAL_BAND_DABL;
			break;
			
		case DABMW_BAND_AM_LW:
			vl_etalBand = ETAL_BAND_LW;
			break;
		case DABMW_BAND_AM_SW1:
		case DABMW_BAND_AM_SW2:
		case DABMW_BAND_AM_SW3:
		case DABMW_BAND_AM_SW4:
		case DABMW_BAND_AM_SW5:
		case DABMW_BAND_AM_SW6:
		case DABMW_BAND_AM_SW7:
		case DABMW_BAND_AM_SW8:
		case DABMW_BAND_AM_SW9:
		case DABMW_BAND_AM_SW10:
			vl_etalBand = ETAL_BAND_SW;
			break;	
			
		case DABMW_BAND_DRM30:
			vl_etalBand = ETAL_BAND_DRM30;
			break;	
			
		default:
			vl_etalBand = ETAL_BAND_UNDEF;
			break;	
			
	}

	return(vl_etalBand);

}

#endif // CONFIG_ETAL_HAVE_ETALTML

