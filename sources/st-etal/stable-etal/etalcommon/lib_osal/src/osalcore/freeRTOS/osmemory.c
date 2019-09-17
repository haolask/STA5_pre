//!
//!  \file 		osmemory.c
//!  \brief 	<i><b>OSAL Memory Handling Functions</b></i>
//!  \details	This is the implementation file for the OSAL
//!             (Operating System Abstraction Layer) Memory-Functions.
//!  \author 	Luca Pesenti
//!  \author 	(original version) Luca Pesenti
//!  \version 	1.0
//!  \date 		11.06.2008
//!  \bug 		Unknown
//!  \warning	None
//!

/* DeactivateLintMessage_ID0026 */
/*lint -esym(750, OS*_C) */
#ifndef OSMEMORY_C
#define OSMEMORY_C

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
| includes of component-internal interfaces
| (scope: component-local)
|-----------------------------------------------------------------------*/

/* Standard Library */
#include <stdlib.h>
#include <string.h>


/* osal Header */
#include "osal.h"

/************************************************************************
|function implementation (scope: global)
|-----------------------------------------------------------------------*/

/**
 * @brief     OSAL_pvMemoryAllocate
 *
 * @details   This function reserve on the Heap-memory a block with
 *            the specific size
 *
 * @param     u32size Size of the memory block (I)
 *
 * @return
 *        - Pointer to start memory block
 *        - OSAL_NULL in case of error
 *
 */
tPVoid OSAL_pvMemoryAllocate(tU32 u32size)
{
   return (tPVoid)malloc(u32size);
}

/**
 * @brief     OSAL_pvMemoryCAllocate
 *
 * @details   This function reserve on the Heap-memory a block composed
 *            by nobj of the the specific size
 *
 * @param     u32size Size of the memory block (I)
 * @param     nobj    Number of Elements (I)
 *
 * @return
 *        - Pointer to start memory block
 *        - OSAL_NULL in case of error
 *
 */
tPVoid OSAL_pvMemoryCAllocate(tU32 nobj, tU32 u32size)
{
   return (tPVoid)calloc(nobj,u32size);
}
/**
 * @brief     OSAL_pvMemoryReAllocate
 *
 * @details   This function reserve on the Heap-memory a block with
 *            the specific size
 *
 * @param     u32size Size of the memory block (I)
 *
 * @return
 *        - Pointer to start memory block
 *        - OSAL_NULL in case of error
 *
 */
tPVoid OSAL_pvMemoryReAllocate(tPVoid pBlock, tU32 u32size)
{
   return (tPVoid)realloc(pBlock,u32size);
}
/**
 * @brief     OSAL_vMemoryFree
 *
 * @details   This function releases on the Heap-memory a block with
 *            the specific size
 *
 * @param     pBlock Pointer to start memory block (I)
 *
 * @return    NULL
 *
 */
tVoid OSAL_vMemoryFree(tPVoid pBlock)
{
   free(pBlock);
   return;
}

/**
 * @brief     OSAL_pvMemoryCopy
 *
 * @details   This function copies a memory area with the specified size.
 *
 *
 * @param     pvDest     Start to the target memory (I)
 * @param     pvSource   Start to the source memory (I)
 * @param     u32size    Size of the memory block (I)
 *
 * @return    Starting of the target memory
 *
 */
tPVoid OSAL_pvMemoryCopy( tPVoid pvDest, tPCVoid pvSource, tU32 u32size )
{
   return((tPVoid)memcpy(pvDest,pvSource,u32size));
}

/**
 * @brief      OSAL_pvMemoryMove
 *
 * @details   This function moves a memory block. The old and new memory
 *            area can not overlap.
 *
 *
 * @param     pvDest     Start to the target memory (I)
 * @param     pvSource   Start to the source memory (I)
 * @param     u32Bytes   Number of bytes (I)
 *
 * @return    Starting of the target memory
 *
 */
tPVoid OSAL_pvMemoryMove(tPVoid pvDest, tPCVoid pvSource, tU32 u32Bytes)
{
    return((tPVoid)memmove(pvDest,pvSource,u32Bytes));
}

/**
 * @brief     OSAL_s32MemoryCompare
 *
 * @details   This function compares two memory areas bitwyse.
 *
 *
 * @param     pcovFirst   Starting to the target memory (I)
 * @param     pcovSecond  Starting to the source memory (I)
 * @param     u32Bytes    Number of bytes (I)
 *
 * @return
 *        - = 0  if identical
 *        - > 0  in case the different element in first is greater
 *               the corresponding one in second
 *        - < 0  otherwise
 *
 */
tS32 OSAL_s32MemoryCompare(tPCVoid pcovFirst, tPCVoid pcovSecond, tU32 u32Bytes)
{
   return((tS32)memcmp(pcovFirst,pcovSecond,u32Bytes));
}

/**
 * @brief     OSAL_pvMemorySearchChar
 *
 * @details   This function searches a memory area for a character.
 *
 * @param     pcovSource Starting to the target memory (I)
 * @param     u32Char    character (I)
 * @param     u32Bytes   Number of bytes (I)
 *
 * @return
 *        - Pointer to the first character
 *        - OSAL_NULL in case of error
 *
 */
tPVoid OSAL_pvMemorySearchChar(tPCVoid pcovSource, tS32 u32Char, tU32 u32Bytes)
{
   return((tPVoid)memchr(pcovSource,u32Char,u32Bytes));
}

/**
 * @brief     OSAL_pvMemorySet
 *
 * @details   This function initializes a memory with a specific character.
 *
 * @param     pvSource   Starting to the target memory (I)
 * @param     u32Char    character (I)
 * @param     u32Bytes   Number of bytes (I)
 *
 * @return    Starting of the target memory
 *
 */
tPVoid OSAL_pvMemorySet(tPVoid pvSource, tU32 u32Char, tU32 u32Bytes)
{
   return((tPVoid)memset(pvSource,u32Char,u32Bytes));
}

#ifdef __cplusplus
}
#endif

#endif /* OSMEMORY_C */

/** @} */

/* End of File */
