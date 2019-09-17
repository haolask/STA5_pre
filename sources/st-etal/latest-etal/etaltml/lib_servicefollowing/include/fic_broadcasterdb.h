//!
//!  \file         fic_broadcasterdb.h
//!  \brief     <i><b> Header file for broadcaster database management </b></i>
//!  \details    This file is the broadcaster database management headers.
//!  \author     Alberto Saviotti
//!  \author     (original version) Alberto Saviotti
//!  \version     1.0
//!  \date         2012.03.05
//!  \bug         Unknown
//!  \warning    None
//!

#ifndef __FIC_BROADCASTERDB_H__
#define __FIC_BROADCASTERDB_H__

typedef enum
{
    DABMW_BRDB_SEGMENT_EMPTY   = 0,
    DABMW_BRDB_SEGMENT_STORED  = 1,
    DABMW_BRDB_SEGMENT_CHECKED = 2
} DABMW_brDbSegmentStatusTy;

typedef struct DABMW_brDbDataSegment_tag DABMW_brDbDataSegmentTy;
struct DABMW_brDbDataSegment_tag
{
    DABMW_brDbSegmentStatusTy segmentStatus;
    tVoid* data; // valid only if not NULL and segmentStatus!=DABMW_BRDB_SEGMENT_EMPTY
    DABMW_brDbDataSegmentTy *next;
};

#define DABMW_BRDB_UNKNOWN_FIG021_CONTROL  0x1F
#define DABMW_BRDB_UNKNOWN_FIG021_RM       0x0F

/*
 * DABMW_BrDbFreqListTy
 *
 * Description of a single frequency on which a service can be tuned.
 *
 * control
 *  Control Field from FIG 0/21 of DABMW_BRDB_UNKNOWN_FIG021_CONTROL
 *  if the entry was filled from the LandscapeDb instead of
 *  the BroadcastDb.
 *
 * freq
 *  Frequency expressed in KHz.
 *
 */
typedef struct
{
    tU8  control:       5;
    tU32 freq:         24;
} DABMW_BrDbFreqListTy;

/*
 * DABMW_BrDbLSFrequencyInfoTy
 *
 * Describes the frequencies on which some service can be tuned.
 *
 * regionId
 *  RegionId from FIG 0/21 or 0 if not indicated there.
 *  Also 0 if the entry was filled from the LandscapeDb instead of
 *  the BroadcastDb (rm==DABMW_BRDB_UNKNOWN_FIG021_RM).
 *
 * rm
 *  Range and Modulation from FIG 0/21 or DABMW_BRDB_UNKNOWN_FIG021_RM
 *  if the entry was filled from the LandscapeDb instead of
 *  the BroadcastDb.
 *
 * continuity
 *  Continuity Flag from FIG 0/21. The interpretation depends
 *  on the rm field, see EN 300 401 par 8.1.8 for details.
 *  This field should be ignored if rm==DABMW_BRDB_UNKNOWN_FIG021_RM.
 *
 * size
 *  Number of valid elements in the freqList array.
 *
 * freqList[DABMW_FREQ_LIST_MAX_NUMBER]
 *  List of frequencies on which a service can be found.
 *
 */
typedef struct
{
    tU16 regionId:      11;
    tU8  rm:             4;
    tU8  continuity:     1;
    tU8  size;  /* number of elements in the freqList array */
    DABMW_BrDbFreqListTy freqList[DABMW_FREQ_LIST_MAX_NUMBER];
} DABMW_BrDbLSFrequencyInfoTy;

/* 
 * DABMW_BrDbLinkageSetTy
 *
 * Describes a set of services (DAB, FM, other) that relate one to the other.
 *
 * sh
 *   Soft/Hard link, directly from FIG 0/6.
 *
 * la
 *   Link Actuator, directly from FIG 0/6; this reflects the current state
 *   of the flag in the database, so it may change over time.
 *
 * size
 *   number of elements in the id, kindOfId, frequencyArray and eid arrays,
 *   or 0 if there was an error filling the arrays. In this case the entry
 *   should be ignored and freed.
 *
 * id[size]
 *   array containing one id per element. The type of the i'th element
 *   is described by kindOfId[i]. id's are taken from FIG 0/6.
 *
 * kindOfId[size]
 *   array describing the id[] elements.
 *
 * eid[size]
 *   array containing the EnsembleId of the id[]'s of type DABMW_ID_IS_SID_TYPE.
 *   For other types eid[i] is 0.
 *   Eid is gathered first from the LandscapeDb and if not present there
 *   from the FIG 0/24; in the first case it includes the ECC (so it is 24 bits)
 *   in the latter case it is 16bits
 *
 * frequencyArray[size]
 *   array containing the frequency for the service.
 *   For id's of type DABMW_ID_IS_SID_TYPE this is the frequency of the ensemble.
 *   For id's of type DABMW_ID_IS_RDS_PI_CODE_TYPE this is the frequency of the FM station.
 *
 */
typedef struct
{
    /* from FIG 0/6 */
    tU8  sh:            1;
    tU8  la:            1;
    tU32 size;
    tU32 *id;
    DABMW_idTy *kindOfId;

    /* from FIG 0/21 */
    DABMW_BrDbLSFrequencyInfoTy *frequencyArray;

    /* from FIG 0/24 */
    tU32 *eid;

    /* internal use, don't change */
    OSAL_tMSecond timestampFig06;
    OSAL_tMSecond timestampFig021;
    OSAL_tMSecond timestampFig024;
	tU32 dbKey; /* from FIG 0/6 */
	tU32 ensembleId; 
} DABMW_BrDbLinkageSetTy;

typedef enum
{
    DABMW_BRDB_LS_OK,
    DABMW_BRDB_LS_ACTIVE_STATE_CHANGE,
    DABMW_BRDB_LS_CHANGE,
    DABMW_BRDB_LS_ERROR
} DABMW_BrDbLinkageSetErrorTy;

#define DABMW_BRDB_LS_GET_HARDLINK  0x01
#define DABMW_BRDB_LS_GET_SOFTLINK  0x02
#define DABMW_BRDB_LS_GET_ACTIVE    0x04
#define DABMW_BRDB_LS_GET_INACTIVE  0x08

#endif // #ifndef __FIC_BROADCASTERDB_H__

// End of file
