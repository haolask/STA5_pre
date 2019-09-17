//!
//!  \file 		osmemory.h
//!  \brief 	<i><b> OSAL Memory-Functions Header File </b></i>
//!  \details	This is the headerfile for the OS - Abstraction Layer for
//!             Memory-Functions. This Header has to be included to use the
//!             functions for memory handling
//!  \author 	Luca Pesenti
//!  \author 	(original version) Giovanni Di Martino
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

#ifndef OSMEMORY_H
#define OSMEMORY_H

#ifdef __cplusplus
extern "C"
{
#endif

   /******************************************************************************/
   /* defined in string.h                                                        */
   /******************************************************************************/
#if defined(CONFIG_HOST_OS_FREERTOS)
   #define OSAL_pvMemoryMove memmove
#else //CONFIG_HOST_OS_FREERTOS
   #define OSAL_pvMemoryMove(_DEST_,_SRC_,_SIZE_) \
      (memmove((_DEST_),(_SRC_),(_SIZE_)))
#endif //CONFIG_HOST_OS_FREERTOS

//-----------------------------------------------------------------------
// function prototypes (scope: global)
//-----------------------------------------------------------------------
extern tPVoid OSAL_pvMemoryAllocate(tU32 u32size);
extern tPVoid OSAL_pvMemoryCAllocate(tU32 nobj, tU32 u32size);
extern tPVoid OSAL_pvMemoryReAllocate(tPVoid pBlock, tU32 u32size);
extern tVoid OSAL_vMemoryFree(tPVoid pBlock);
extern tPVoid /*@alt void@*/OSAL_pvMemoryCopy(tPVoid pvDest, tPCVoid pvSource, tU32 u32size);
#if 0
extern tPVoid OSAL_pvMemoryMove(tPVoid pDest, tPCVoid pSource, tU32 size);
extern tPVoid OSAL_pvMemorySearchChar(tPCVoid pSource, tS32 ch, tU32 nBytes);
#endif
extern tS32 OSAL_s32MemoryCompare(tPCVoid pcovFirst, tPCVoid pcovSecond, tU32 u32Bytes);
extern tPVoid /*@alt void@*/OSAL_pvMemorySet(tPVoid pvSource, tU32 u32Char, tU32 u32Bytes);

#ifdef __cplusplus
}
#endif


#else
#error osmemory.h included several times
#endif


// EOF
