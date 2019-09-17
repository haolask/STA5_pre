///
//! \file          stm_types.h
//! \brief         STMicroelectronics custom types
//! \author        STMicroelectronics
//!
//! Project        MSR
//! Sw component   lib_common
//!
//! Copyright (c)  2015 STMicroelectronics
//!
//! History
//! Date        | Modification               | Author
//! 2015.07.20  | Initial version            | STMicroelectronics
///

#ifndef _STMTYPES_H_
#define _STMTYPES_H_

//-----------------------------------------------------------------------
// defines (scope: global)
//-----------------------------------------------------------------------
//! \brief Size of the void pointer: machine dependent
#if (defined __x86_64__) && (!defined __ILP32__)
    #define VOIDPOINTERSIZE             64
#else
    #define VOIDPOINTERSIZE             32
#endif // #if (defined __x86_64__) && (!defined __ILP32__)

//! \brief Inline platform independent definition
#define INLINE                      __inline    // Specifies a routine to be in line

//! \brief Constant platform independent definition
#define CONST                       const       // Defines a constant item

//! \brief NULL definition
#ifndef NULL
    #ifdef __cplusplus
        #define NULL                    0
    #else
        #define NULL                    ((void *)0)
    #endif
#endif

#ifndef TRUE
    //! \brief True value definition
    #define TRUE                        1
#endif

//! \brief Set value definition
#define SET                         TRUE

#ifndef true
    //! \brief True value definition
    #define true                        TRUE
#endif

#ifndef FALSE
    //! \brief False value definition
    #define FALSE                       0
#endif

//! \brief Clear value definition
#define CLEAR                       FALSE

#ifndef false
    //! \brief False value definition
    #define false                       FALSE
#endif

//! \brief Array size
#define ARRAY_SIZE(x)               (sizeof(x) / sizeof((x)[0]))

//! \brief Size of boolean type: 1 bytes
#define SIZE_BOOL                   sizeof(tBool)

//! \brief Size of char type: 1 bytes
#define SIZE_CHAR                   sizeof(tS8)

//! \brief Size of short type: 2 bytes
#define SIZE_SHORT                  sizeof(tS16)

//! \brief Size of integer type: 4 bytes
#define SIZE_INT                    sizeof(tS32)

//! \brief Size of long type: 4 bytes
#define SIZE_LONG                   sizeof(tS32)

//! \brief Size of float type: 4 bytes
#define SIZE_FLOAT                  sizeof(float)

//! \brief Size of doubl;e type: 8 bytes
#define SIZE_DOUBLE                 sizeof(double)

//! \brief Size of tComplex type: 4 bytes
#define SIZE_COMPLEX                sizeof(tComplex)

//! \brief 'void' declaration
#define tVoid                       void

//-----------------------------------------------------------------------
// typedefs (scope: global)
//-----------------------------------------------------------------------
//! \brief Unsigned long long type (length is not guarantee by standards)
typedef unsigned long long int      tU64;

//! \brief Signed long long type (length is not guarantee by standards)
typedef signed  long long int       tS64;

#if (VOIDPOINTERSIZE == 64)
    //! \brief Cast of void pointer
    typedef signed long long int        tCastVoidPtr;

    //! \brief Cast of unsigned void pointer
    typedef unsigned long long int      tCastUnsignedVoidPtr;
#else
    //! \brief Cast of void pointer
    typedef signed int                  tCastVoidPtr;

    //! \brief Cast of unsigned void pointer
    typedef unsigned int                tCastUnsignedVoidPtr;
#endif // #if (VOIDPOINTERSIZE == 64)

//! \brief Boolean type
#ifdef __cplusplus
    #define tBool                       bool
#else
    typedef unsigned char               tBool;
#endif // #ifndef bool

//! \brief Unsigned char type
typedef unsigned char               tU8;

//! \brief Signed char type
typedef char                        tS8;

//! \brief Unsigned short type
typedef unsigned short              tU16;

//! \brief Signed short type
typedef short                       tS16;

//! \brief Unsigned integer type
typedef unsigned int                tU32;

//! \brief Signed integer type
typedef int                         tS32;

//! \brief Unsigned char pointer type
typedef tU8 *                       tPU8;

//! \brief Signed char pointer type
typedef tS8 *                       tPS8;

//! \brief Unsigned short pointer type
typedef tU16 *                      tPU16;

//! \brief Signed short pointer type
typedef tS16 *                      tPS16;

//! \brief Unsigned int pointer type
typedef tU32 *                      tPU32;

//! \brief Signed int pointer type
typedef tS32 *                      tPS32;

//! \brief Signed float type
typedef float                       tF32;

//! \brief Signed double type
typedef double                      tF64;

//! \brief Pointer to unsigned 64 bits variable
typedef unsigned long long *        tPU64;

//! \brief Pointer to signed 64 bits variable
typedef signed long long *          tPS64;

//! \brief Pointer to char array
typedef char *                      tString;

//! \brief Pointer to constant char array
typedef const char *                tCString;

//! \brief Signed char type
typedef char                        tChar;

//! \brief Unsigned char type
typedef unsigned char               tUChar;

//! \brief Pointer to unsigned char array
typedef unsigned char *             tPUChar;

//! \brief Pointer to constant char array
typedef const unsigned char *       tCPUChar;

//! \brief Signed char type
typedef char                        tSChar;

//! \brief Pointer to char array
typedef char *                      tPChar;

//! \brief Pointer to constant char array
//typedef const char *                tCPChar;

//! \brief Signed short type
typedef unsigned short              tUShort;

//! \brief Signed short type
typedef short                       tShort;

//! \brief Unsigned integer type
typedef unsigned int                tUInt;

//! \brief Signed integer type
typedef int                         tSInt;

//! \brief Signed integer type
typedef int *                       tPSInt;

//! \brief Long unsigned integer type
typedef unsigned long               tULong;

//! \brief Long signed integer type
typedef long                        tSLong;

//! \brief Float signed type
typedef float                       tSFloat;

//! \brief Double signed integer type
typedef double                      tSDouble;

//! \brief Long double type, implementation dependent
typedef long double                 tSLDouble;

//! \brief Unicode type (16 bits)
typedef unsigned short              tUnicode;

//! \brief Bitfield type: unsigned 32 bits field
typedef unsigned int                tBitfield;

//! \brief Long type
typedef long                        tLong;

//! \brief Float type. According to standard it is 32 bits:
//!        1 bit sign, 8 bits exponent, 23 bits mantissa
typedef float                       tFloat;

//! \brief Double type. According to standard it is 64 bits:
//!        1 bit sign, 11 bits exponent, 52 bits mantissa
typedef double                      tDouble;

//! \brief Long double type
typedef long double                 tLDouble;

//! \brief Constant signed char pointer
typedef char                        tCS8;

//! \brief Constant signed short pointer
typedef const unsigned short        tCS16;

//! \brief Constant signed integer pointer
typedef const unsigned short        tCS32;

//! \brief Constant unsigned char pointer
typedef char                        tCU8;

//! \brief Constant unsigned short pointer
typedef const unsigned short        tCU16;

//! \brief Constant unsigned integer pointer
typedef const unsigned short        tCU32;

//! \brief Constant unsigned long long pointer
typedef const unsigned long long    tCU64;

//! \brief Constant signed long long pointer
typedef const signed long long      tCS64;

//! \brief Constant unsigned float type
typedef const tU32                  tCUF32;

//! \brief Constant signed float type
typedef const tF32                  tCSF32;

//! \brief Constant unsigned double type
typedef const unsigned long long    tCUF64;

//! \brief Constant signed double type
typedef const tF64                  tCSF64;

//! \brief Volatile boolean
typedef volatile unsigned char      tVBool;

//! \brief Volatile boolean pointer
typedef volatile unsigned char *    tVPBool;

//! \brief Volatile unsigned char
typedef volatile unsigned char      tVU8;

//! \brief Volatile unsigned char pointer
typedef volatile unsigned char *    tVPU8;

//! \brief Volatile signed char
typedef volatile char               tVS8;

//! \brief Volatile signed char pointer
typedef volatile char *             tVPS8;

//! \brief Volatile unsigned short
typedef volatile unsigned short     tVU16;

//! \brief Volatile unsigned integer pointer
typedef volatile unsigned short *   tVPU16;

//! \brief Volatile signed integer
typedef volatile short              tVS16;

//! \brief Volatile signed short pointer
typedef volatile short *            tVPS16;

//! \brief Volatile unsigned integer
typedef volatile unsigned long      tVU32;

//! \brief Volatile unsigned integer pointer
typedef volatile unsigned long *    tVPU32;

//! \brief Volatile signed integer
typedef volatile long               tVS32;

//! \brief Volatile signed integer pointer
typedef volatile long *             tVPS32;

//! \brief Sample type to be used for I/Q samples
typedef tS16                        tSample;

//! \brief
typedef const tS8 *                 tPCS8;

//! \brief
typedef const tS16 *                tPCS16;

//! \brief
typedef const tS32 *                tPCS32;

//! \brief
typedef const tU8 *                 tPCU8;

//! \brief
typedef const tU16 *                tPCU16;

//! \brief
typedef const tU32 *                tPCU32;

//! \brief
typedef const tF32 *                tPCF32;

//! \brief
typedef const tF64 *                tPCF64;

//! \brief
typedef const unsigned long long *  tPCU64;

//! \brief
typedef const signed long long *    tPCS64;

//! \brief 'void *' declaration
typedef tVoid *                     tPVoid;

//! \brief 'const void *' declaration
typedef const tVoid *               tPCVoid;

//! \brief 'volatile void *' declaration
typedef volatile tVoid *            tVPVoid;

//! \brief Pointer to const char (C++11 forbids converting a string constant to char *)
typedef char const *                tCharCP;

//! \brief Complex type
typedef struct
{
    tSample re;     //!< Real part
    tSample im;     //!< Imaginary part
} tComplex;

//! \brief Error check enum
typedef enum
{
    CHECK_ERROR,
    A_GREATER_B,
    B_GREATER_A
} tSort;

//! \brief Pin level enum
typedef enum
{
    LEVEL_LOW,
    LEVEL_HIGH
} tLevel;

#endif  // _STMTYPES_H_

// End of file
