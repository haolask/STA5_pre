/*____________________________________________________________________________
| FILE:         types.h
| PROJECT:      Newcastle - STA895
| SW-COMPONENT:
|_____________________________________________________________________________
| DESCRIPTION:  types definition
|_____________________________________________________________________________
| COPYRIGHT:    (c) 2008 STMicroelectronics, Arzano (NA) - ITALY
| HISTORY:
| Date      | Modification               | Author
|_____________________________________________________________________________
| 28.01.08  | Initial version            | L. Cotignano
|____________________________________________________________________________*/

#ifndef _MYTYPES_H_
#define _MYTYPES_H_

//-----------------------------------------------------------------------
// included headers (scope: module-global)
//-----------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif

//-----------------------------------------------------------------------
// defines (scope: global)
//-----------------------------------------------------------------------
// Description:
// Further keywords (section 5.1.2)
#define INLINE              __inline    // specifies a routine to be in line
#define CONST               const       // defines a constant item

#ifndef NULL
    #ifdef __cplusplus
        #define NULL            0
    #else
        #define NULL            ((void*)0)
    #endif
#endif

#ifndef TRUE
    #define TRUE                ((tBool)1)
#endif
#ifndef SET
    #define SET                 TRUE
#endif
#ifndef true
    #define true                TRUE
#endif

#ifndef FALSE
    #define FALSE               ((tBool)0)
#endif
#ifndef CLEAR
    #define CLEAR               FALSE
#endif
#ifndef false
    #define false               FALSE
#endif

#ifndef OSAL_OK
	#define OSAL_OK            ((tS32)  0)
#endif
#ifndef OSAL_ERROR
	#define OSAL_ERROR         ((tS32) -1)
#endif

//-----------------------------------------------------------------------
// typedefs (scope: global)
//-----------------------------------------------------------------------
typedef unsigned char           tBool;

#ifndef OSAL_PLAIN_TYPES_DEFINED
#define OSAL_PLAIN_TYPES_DEFINED
typedef unsigned char           tU8;
typedef signed char             tS8;

typedef unsigned short          tU16;
typedef signed short            tS16;

typedef unsigned int            tU32;
typedef int                     tS32;

typedef tU8*                    tPU8;
typedef tU16*                   tPU16;
typedef tU32*                   tPU32;

typedef float                   tF32;
typedef double                  tF64;

typedef unsigned long long      tU64;
typedef long long               tS64;

// TKernel exception leading to warning : 
// put tVoid as void define
#if defined (CONFIG_HOST_OS_TKERNEL)  || defined(CONFIG_HOST_OS_FREERTOS)
#define tVoid					void
#else
typedef void                    tVoid;
#endif

typedef tVoid*                  tPVoid;
typedef const tVoid*            tPCVoid;

typedef char*                   tString;
typedef const char*             tCString;
#endif // OSAL_PLAIN_TYPES_DEFINED

typedef unsigned char           tUChar;
typedef signed char             tSChar;
typedef char                    tChar;
typedef char*                   tPChar;

typedef unsigned short          tUShort;
typedef signed short            tShort;

typedef unsigned int            tUInt;
typedef signed int              tSInt;
typedef int                     tInt;

typedef unsigned long           tULong;
typedef long                    tSLong;

typedef float                   tSFloat;
typedef double                  tSDouble;
typedef long double             tSLDouble;

typedef long                    tLong;

typedef float                   tFloat;
typedef double                  tDouble;
typedef long double             tLDouble;

typedef tU8                     uint8;     // for etalmdr
typedef tU16                    uint16;    // for etalmdr

/*
 * definitions needed to reduce (sp)lint warnings
 *
 * - sizeof returns a size_t which is not defined in the OSAL types
 *   so we cast it to an unsigned int
 *
 * - functions returning non-void and the return value not being checked
 *   trigger many errors. We cope with these case by case with the following
 *   macros:
 *
 *   {backslash}*@alt void@*{backslash} declares that a function may return
 *   void in addition to the declared value. I found no way to define this as
 *   a macro so in the code you'll find the comment as it is.
 *
 *   LINT_IGNORE_RET notifies lint that we don't want to check
 *   the return value for a particular function invocation; it must be inserted
 *   immediately before the function call, sourrounded by parenthesis e.g. (LINT_IGNORE_RET)
 */
#define sizeof(_x_)         (tU32)(sizeof(_x_))
#define LINT_IGNORE_RET     void       /* sourround with parenthesis (as in the type cast) */

#ifdef __cplusplus
}
#endif

#endif  // _MYTYPES_H_

// End of file

