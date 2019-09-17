//!
//!  \file 		osansi.h
//!  \brief 	<i><b>OSAL Ansi Func Header file</b></i>
//!  \details	This is the headerfile for the OS - Abstraction Layer for
//!             Ansi-Functions. This Header has to be included to use the
//!             standard ANSI functions.
//!  \author 	Luca Pesenti
//!  \author 	(original version) Giovanni Di Martino
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

#ifndef OSANSI_H
#define OSANSI_H

 #ifdef __cplusplus
 extern "C" {
 #endif

#if (OSAL_OS==OSAL_LINUX) || (OSAL_OS==OSAL_OS21) || (OSAL_OS==OSAL_TKERNEL) || (OSAL_OS==OSAL_FREERTOS)

   /******************************************************************************/
   /* defined in string.h                                                        */
   /******************************************************************************/

   #define OSAL_szStringCopy(_DEST_,_SRC_) \
      strcpy((_DEST_),(_SRC_))

   #define OSAL_szStringNCopy(_DEST_,_SRC_,_NCNT_) \
      strncpy((_DEST_),(_SRC_),(_NCNT_))

   #define OSAL_szStringConcat(_DEST_,_SRC_) \
      strcat((_DEST_),(_SRC_))

   #define OSAL_szStringNConcat(_DEST_,_SRC_,_NCNT_) \
      strncat((_DEST_),(_SRC_),(_NCNT_))

   #define OSAL_s32StringCompare(_STR1_,_STR2_) \
      strcmp((_STR1_),(_STR2_))

   #define OSAL_s32StringNCompare(_STR1_,_STR2_,_NCNT_) \
      strncmp((_STR1_),(_STR2_),(_NCNT_))

   #define OSAL_ps8StringSearchChar(_STR_,_CH_) \
      strchr((_STR_),(_CH_))

   #define OSAL_ps8StringRSearchChar(_STR_,_CH_) \
      strrchr((_STR_),(_CH_))

   #define OSAL_pu32StringSegment(_STR1_,_STR2_) \
      strspn((_STR1_),(_STR2_))

   #define OSAL_pu32StringNotSegment(_STR1_,_STR2_) \
      strcspn((_STR1_),(_STR2_))

   #define OSAL_ps8StringBreak(_STR1_,_STR2_) \
      strpbrk((_STR1_),(_STR2_))

   #define OSAL_ps8StringSubString(_STR1_,_STR2_) \
      strstr((_STR1_),(_STR2_))

   #define OSAL_u32StringLength(_STR_) \
      strlen(_STR_)

   #define OSAL_ps8StringToken(_STR1_,_STR2_) \
      strtok((_STR1_),(_STR2_))

   #define OSAL_s32NPrintFormat(...) \
       snprintf(__VA_ARGS__)

   /******************************************************************************/
   /* character manipulation and test                                            */
   /* defined in ctype.h                                                         */
   /******************************************************************************/

   #define OSAL_s32IsAlphaNum(_CH_) \
      isalnum(_CH_)

   #define OSAL_s32IsAlpha(_CH_) \
      isalpha(_CH_)

   #define OSAL_s32IsControl(_CH_) \
      iscntrl(_CH_)

   #define OSAL_s32IsDigit(_CH_) \
      isdigit(_CH_)

   #define OSAL_s32IsGraphical(_CH_) \
      isgraph(_CH_)

   #define OSAL_s32IsLower(_CH_) \
      islower(_CH_)

   #define OSAL_s32IsPrintable(_CH_) \
      isprint(_CH_)

   #define OSAL_s32IsPunctation(_CH_) \
      ispunct(_CH_)

   #define OSAL_s32IsSpace(_CH_) \
      isspace(_CH_)

   #define OSAL_s32IsUpper(_CH_) \
      isupper(_CH_)

   #define OSAL_s32IsHexDigit(_CH_) \
      isxdigit(_CH_)

   #define OSAL_s32ToLower(_CH_) \
      tolower(_CH_)

   #define OSAL_s32ToUpper(_CH_) \
      toupper(_CH_)

   /******************************************************************************/
   /* mathematic manipulation                                                    */
   /* defined in math.h                                                          */
   /******************************************************************************/

   #define OSAL_dSin(_VALUE_) \
     sin(_VALUE_)

   #define OSAL_dCos(_VALUE_) \
      cos(_VALUE_)

   #define OSAL_dTan(_VALUE_) \
      tan(_VALUE_)

   #define OSAL_dArcSin(_VALUE_) \
      asin(_VALUE_)

   #define OSAL_dArcCos(_VALUE_) \
      acos(_VALUE_)

   #define OSAL_dArcTan(_VALUE_) \
      atan(_VALUE_)

   #define OSAL_dArcTan2(_DY_,_DX_) \
      atan2((_DY_),(_DX_))

   #define OSAL_dSinHyp(_VALUE_) \
      sinh(_VALUE_)

   #define OSAL_dCosHyp(_VALUE_) \
      cosh(_VALUE_)

   #define OSAL_dTanHyp(_VALUE_) \
      tanh(_VALUE_)

   #define OSAL_dExp(_VALUE_) \
      exp(_VALUE_)

   #define OSAL_dLog(_VALUE_) \
      log(_VALUE_)

   #define OSAL_dLog10(_VALUE_) \
      log10(_VALUE_)

   #define OSAL_dPow(_OPERAND_,_EXPONENT_) \
      pow((_OPERAND_),(_EXPONENT_))

   #define OSAL_dSqrt(_VALUE_) \
      sqrt(_VALUE_)

   #define OSAL_dCeil(_VALUE_) \
      ceil(_VALUE_)

   #define OSAL_dFloor(_VALUE_) \
      floor(_VALUE_)

   #define OSAL_dFAbsolute(_VALUE_) \
      fabs(_VALUE_)

   #define OSAL_dLDExp(_VALUE_,_NCNT_) \
      ldexp((_VALUE_),(_NCNT_))

   #define OSAL_dFRExp(_VALUE_,_PEXPONENT_) \
      frexp((_VALUE_),(_PEXPONENT_))

   #define OSAL_dModF(_VALUE_,_PINTPART_) \
      modf((_VALUE_),(_PINTPART_))

   #define OSAL_dFMod(_DX_,_DY_) \
      fmod((_DX_),(_DY_))

   /******************************************************************************/
   /* additional functions                                                       */
   /******************************************************************************/

   #define OSAL_dAsciiToDouble(_STRING_) \
      atof(_STRING_)

   #define OSAL_s32AsciiToS32(_STRING_) \
      atoi(_STRING_)

   #define OSAL_dStringToDouble(_STRING_,_PEND_) \
      strtod((_STRING_),(_PEND_))

   #define OSAL_s32StringToS32(_STRING_,_PEND_,_BASE_) \
      strtol((_STRING_),(_PEND_),(_BASE_))

   #define OSAL_u32StringToU32(_STRING_,_PEND_,_BASE_) \
      strtoul((_STRING_),(_PEND_),(_BASE_))

   /**************************************************************************/
   /* defined in stdlib.h                                                    */
   /**************************************************************************/

   #define OSAL_s32Random() \
      rand()

   #define OSAL_vRandomSeed(_SEED_) \
      srand(_SEED_)

   #define OSAL_vBinarySearch(_PKEY_,_PBASE_,_ARRAY_,_SIZE_,_PCMPFUNC_) \
      bsearch((_PKEY_),(_PBASE_),(_ARRAY_),(_SIZE_),(_PCMPFUNC_))

   #define OSAL_vQuickSort(_PBASE_,_NCNT_,_SIZE_,_PSRTFUNC_ ) \
      qsort((_PBASE_),(_NCNT_),(_SIZE_),(_PSRTFUNC_ ))

   #define OSAL_s32Absolute(_VALUE_) \
      abs(_VALUE_)

   /**************************************************************************/
   /* debugging support, define in assert.h                                  */
   /**************************************************************************/

   #define OSAL_vAssert(_INT_) \
      assert(_INT_)

   /**************************************************************************/
   /* variable arguments                                                     */
   /* defined in stdarg.h                                                    */
   /**************************************************************************/


#if defined (CONFIG_HOST_OS_FREERTOS)
   #define OSAL_s32VarNPrintFormat         snprintf
#else
   #define OSAL_s32VarNPrintFormat         vsnprintf
#endif
   #define OSAL_tVarArgList va_list

   #define OSAL_VarArgStart(_LIST_,_LASTARG_) \
      va_start((_LIST_),(_LASTARG_))

   #define OSAL_VarArg(_LIST_,_TYPE_) \
      va_arg((_LIST_),_TYPE_)

   #define OSAL_VarArgEnd(_LIST_) \
      va_end(_LIST_)

   /**************************************************************************/
   /* end of WinNT, VxWorks, Linux, RX732 and WinCE                                       */
   /**************************************************************************/

   /**************************************************************************/
   /* defined in stdio.h                                                     */
   /**************************************************************************/
   #define OSAL_s32PrintFormat             sprintf
   #define OSAL_s32ScanFormat              sscanf

   #define OSAL_fopen(_FILENAME_, _MODE_) \
      fopen(_FILENAME_, _MODE_)

   #define OSAL_freopen(_FILENAME_, _MODE_, _STREAM_) \
      freopen(_FILENAME_, _MODE_, _STREAM_)

   #define OSAL_fflush(_STREAM_) \
      fflush(_STREAM_)

   #define OSAL_fclose(_STREAM_) \
      fclose(_STREAM_)

   #define OSAL_remove(_FILENAME_) \
      remove(_FILENAME_)

   #define OSAL_rename(_OLDNAME_, _NEWNAME_) \
      rename(_OLDNAME_, _NEWNAME_)

   #define OSAL_fseek(_STREAM_, _OFFSET_, _WHENCE_) \
      fseek(_STREAM_, _OFFSET_, _WHENCE_)

   #define OSAL_ftell(_STREAM_) \
      ftell(_STREAM_)

   #define OSAL_fread(_DATA_PTR_, _DATA_SIZE_, _NB_ELEM_, _STREAM_) \
      fread(_DATA_PTR_, _DATA_SIZE_, _NB_ELEM_, _STREAM_)

   #define OSAL_fwrite(_DATA_PTR_, _DATA_SIZE_, _NB_ELEM_, _STREAM_) \
      fwrite(_DATA_PTR_, _DATA_SIZE_, _NB_ELEM_, _STREAM_)

   #define OSAL_clearerr(_STREAM_) \
      clearerr(_STREAM_)

   #define OSAL_feof(_STREAM_) \
      feof(_STREAM_)

   #define OSAL_ferror(_STREAM_) \
      ferror(_STREAM_)

   /**************************************************************************/
   /* defined in sys/types.h                                                 */
   /* defined in dirent.h                                                    */
   /**************************************************************************/
   #define OSAL_opendir(_DIRFILENAME_) \
      opendir(_DIRFILENAME_)

   #define OSAL_readdir(_DIRP_) \
      readdir(_DIRP_)

   #define OSAL_closedir(_DIRFILENAME_) \
      closedir(_DIRFILENAME_)

#endif // OSAL_OS==OSAL_OS21

  #ifdef __cplusplus
  }
  #endif

#else
#error osal.h included several times
#endif  //OSANSI_H

/* End of File */
