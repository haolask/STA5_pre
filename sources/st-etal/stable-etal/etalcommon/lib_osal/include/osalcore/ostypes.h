//!
//!  \file 		ostypes.h
//!  \brief 	<i><b> OSAL Types Header File </b></i>
//!  \details	This file contains the various typedefs for the
//!             different OSAL platforms.
//!  \author 	Luca Pesenti
//!  \author 	(original version) Giovanni Di Martino
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

#ifndef OSTYPES_H
#define OSTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------
// typedefs
//----------------------------------------------------------------------
//  PLAIN TYPES
#ifndef OSAL_PLAIN_TYPES_DEFINED
#define OSAL_PLAIN_TYPES_DEFINED
typedef unsigned char			tU8;
typedef signed char      		tS8;

typedef unsigned short			tU16;
typedef signed short			tS16;

typedef unsigned int		    tU32;
typedef int					    tS32;

typedef float				    tF32;
typedef double					tF64;

typedef unsigned long long      tU64;
typedef long long               tS64;
// TKernel exception leading to warning : 
// put tVoid as void define
#ifdef CONFIG_HOST_OS_TKERNEL
#define tVoid					void
#else
typedef void                    tVoid;
#endif

typedef char *					tString;
typedef const char *			tCString;

typedef tVoid*                   tPVoid;
typedef const tVoid*             tPCVoid;
#endif // OSAL_PLAIN_TYPES_DEFINED

// POINTER TYPES
typedef tS8*                    tPS8;
typedef tS16*                   tPS16;
typedef tS32*                   tPS32;

typedef tF32*                   tPF32;
typedef tF64*                   tPF64;

typedef tU64*					tPU64;
typedef tS64*					tPS64;

// CONSTANT TYPES
typedef const tS8               tCS8;
typedef const tS16              tCS16;
typedef const tS32              tCS32;

typedef const tU8               tCU8;
typedef const tU16              tCU16;
typedef const tU32              tCU32;

typedef const tF32              tCF32;
typedef const tF64              tCF64;

typedef const tU64				tCU64;
typedef const tS64				tCS64;

// CONSTANT POINTER TYPES
typedef const tS8*              tPCS8;
typedef const tS16*             tPCS16;
typedef const tS32*             tPCS32;

typedef const tU8*              tPCU8;
typedef const tU16*             tPCU16;
typedef const tU32*             tPCU32;

typedef const tF32*             tPCF32;
typedef const tF64*             tPCF64;

typedef const tU64*				tPCU64;
typedef const tS64*				tPCS64;

typedef void (*OSAL_tpfCallback) ( tPVoid );

typedef enum {
         OSAL_EN_READONLY        =0x0001,
         OSAL_EN_WRITEONLY       =0x0002,
         OSAL_EN_READWRITE       =0x0004,
         OSAL_EN_APPEND             =0x0008,
         OSAL_EN_TEXT               =0x0010,
         OSAL_EN_BINARY          =0x0020
} OSAL_tenAccess;

// FOR OSAL IO

typedef tS16 OSAL_tIODescriptor;
#define OSAL_IODESCRIPTOR_BITS       16
#define OSAL_IODESCRIPTOR_INTID_BITS  8
#define OSAL_IODESCRIPTOR_2_DEVID(_fd_) ((OSAL_tenDevID)(((_fd_)>>8)&0xff))
#define OSAL_IODESCRIPTOR_2_INTID(_fd_) ((tU16)(((_fd_)>>0)&0xff))

#define OSAL_IODESCRIPTOR(_devid_,_inid_) ((_devid_<<8)|(( _inid_)&0xff))

#define OSAL_IOOPEN_AVAILABLE  256

// typedefs for AMFM, tuner, rds and audio devices

//----------------------------------------------------------------------
// typedefs
//----------------------------------------------------------------------
//!
//! \enum tMessageType
//!
typedef enum
{
  CMD_ACCEPTED = 0x00,
  CMD_REJECTED = 0x01,
  CMD_INTERMEDIATE = 0x02,
  CMD_NOT_IMPLEMENTED = 0x03,
  CMD_BUSY = 0x04,
  CMD_SYNTAX_ERROR = 0x05,
  NOTF_DONE = 0x80,
  NOTF_ONGOING = 0x81,
  NOTF_CANT_FINISH = 0x82,
  NOTF_ERROR = 0x83,
  NOTF_INFO = 0x84,
  NOTF_REQUEST = 0x85,
} tMessageType;

//!
//! \struct tCmdParams
//! This structure is used to pass audio command parameters to/from commands
//!
typedef struct
{
  //! cmd number
  tU16 cmd;
  //! Pointer to output data buffer
  tU8* buf;
  //! Output data buffer length (8 bits)
  tU8 length;
} tIOParams;

typedef struct
{
  //! input params
  tIOParams in;
  //! output params
  tIOParams out;
  //! Callback pointer
  OSAL_tpfCallback pCallBack;
} tCmdParams;


#ifdef __cplusplus
}
#endif

#else
#error ostypes.h included several times
#endif  //OSTYPES_H


// EOF
