//!
//!  \file    rif_etalapi_cnv.h
//!  \brief   <i><b> Radio interface etaltmlapi conversion functions entry point </b></i>
//!  \details Radio interface etaltmlapi conversion functions.
//!  \author  Alan Le Fol
//!

#ifndef RIF_ETALTMLAPI_CNV_H
#define RIF_ETALTMLAPI_CNV_H

#ifdef __cplusplus
extern "C" {
#endif

///
// Defines
///

/*!
 * \enum	EtalDataPathType
 * 			Describes the ETAL Datapath data types. Some values
 * 			are also used in the #etalTuner to describe the
 * 			Front End capabilities.
 * \remark	Although these values are defined as a bitmap,
 * 			a Datapath can only have **one** Data Type.
 */
typedef enum
{
	ETAL_DATA_PATH_TYPE_AUDIO			   = 0x80,
	ETAL_DATA_PATH_TYPE_DCOP_AUDIO		   = 0x81,
	ETAL_DATA_PATH_TYPE_DATA_SERVICE	   = 0x82,
	ETAL_DATA_PATH_TYPE_DAB_DATA_RAW	   = 0x83,
	ETAL_DATA_PATH_TYPE_DAB_FIC 		   = 0x84,
	ETAL_DATA_PATH_TYPE_TEXTINFO		   = 0x85,
	ETAL_DATA_PATH_TYPE_FM_RDS			   = 0x86,
	ETAL_DATA_PATH_TYPE_FM_RDS_RAW		   = 0x87,
} EtalDataPathType;

///
// Exported functions
///

extern tSInt rif_etaltmlapi_cnv_checkCmdNumber(tU8 *cmd, tU32 clen);
extern tSInt rif_etaltmlapi_cnv_cmd(tU8 *cmd, tU32 clen, tU8 *resp, tU32 *rlen);
extern tSInt rif_etaltmlapi_cnv_resp_add_pEtalTextInfo(tU8 *resp, tU32 *rlen, EtalTextInfo *ptr);
extern tSInt rif_etaltmlapi_cnv_resp_add_pEtalRDSData(tU8 *resp, tU32 *rlen, EtalRDSData *ptr);


#ifdef __cplusplus
}
#endif

#endif // RIF_ETALTMLAPI_CNV_H
// End of file
