//!
//!  \file 		tri_types.h
//!  \brief 	<i><b> OSAL Trace Types Header File </b></i>
//!  \details	This file contains the various trace typedefs.
//!  \author 	Maik Scholz
//!  \author 	(original version) Maik Scholz
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!
#ifndef TRI_TYPES_H
#define TRI_TYPES_H

/*-------------------------------------
      definition of trace level
  -------------------------------------*/

#define TR_LEVEL_FATAL        0
#define TR_LEVEL_ERRORS       1
#define TR_LEVEL_SYSTEM_MIN   2
#define TR_LEVEL_SYSTEM       3
#define TR_LEVEL_COMPONENT    4
#define TR_LEVEL_USER_1       5
#define TR_LEVEL_USER_2       6
#define TR_LEVEL_USER_3       7
#define TR_LEVEL_USER_4       8
#define TR_LEVEL_MAX          TR_LEVEL_USER_4

/*-------------------------------------------------------------------
      definition of old trace classes (should be removed by time)
  -------------------------------------------------------------------*/

typedef enum
{
     TR_CLASS_OSALCORE                          = 0x01030000L,
     TR_CLASS_TUNERDRIVER                       = 0x02000000L,
     TR_CLASS_APP_ETAL                          = 0x03000000L,
     TR_CLASS_APP_ETAL_COMM                     = 0x03000001L,
     TR_CLASS_APP_ETAL_CMD                      = 0x03000002L,
     TR_CLASS_APP_ETAL_API                      = 0x03000004L,
     TR_CLASS_APP_ETAL_TEST                     = 0x03000005L,
     TR_CLASS_CMOST                             = 0x04000000L,
     TR_CLASS_STECI                             = 0x05000000L,
     TR_CLASS_BOOT                              = 0x06000000L,
     TR_CLASS_HDRADIO                           = 0x07000000L,
     TR_CLASS_BSP                               = 0x08000000L,
     TR_CLASS_EXTERNAL                          = 0x09000000L,
     TR_CLASS_APP_RDS_STRATEGY                  = 0x0A000000L,
     TR_CLASS_APP_IPFORWARD                     = 0x0D000000L,
     TR_CLASS_APP_DABMW                         = 0x0F000000L,
     TR_CLASS_APP_DABMW_SF                      = 0x0F000001L,
     TR_LAST_CLASS
}TR_tenTraceClass;

#endif
