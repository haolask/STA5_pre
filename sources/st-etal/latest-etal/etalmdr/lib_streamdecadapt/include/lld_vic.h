//_______________________________________________________________________________
//| FILE:         lld_vic.h
//| PROJECT:      ADR3 ROM
//| SW-COMPONENT: LLD
//|______________________________________________________________________________
//| DESCRIPTION : Vector Interrupt Controller low level driver
//|______________________________________________________________________________
//| COPYRIGHT:    (c) 2009 STMicroelectronics, Naples (ITALY)
//| HISTORY:
//| Date      | Modification               | Author
//|______________________________________________________________________________
//| 12.06.09  | Initial revision           | L. Cotignano
//|______________________________________________________________________________

#ifndef _LLD_VIC_H_
#define _LLD_VIC_H_

/*!
 * @file    lld_vic.h
 * @brief   VIC Low Level Driver header file
 */

//----------------------------------------------------------------------
// defines
//----------------------------------------------------------------------
/*! @brief It's the VIC IP base address */
#define LLD_VIC_ADDRESS                     (VIC_REG_START_ADDR)
/*! @brief It's the VIC's vectored interrupts number */
#define LLD_VIC_INTERRUPT_PRIORITIES        (16)
/*! @brief It's the VIC's registers width */
#define LLD_VIC_REGISTER_WIDTH              (32)
/*! @brief It's the VIC's channels number */
#define LLD_VIC_INTERRUPT_CHANNELS          (64)

/*! @brief It's the macro to read any of the 32 bits VIC's register */
#define LLD_VIC_ReadRegister(reg)           READ32(reg)
/*! @brief It's the macro to write any of the 32 bits VIC's register */
#define LLD_VIC_WriteRegister(reg,value)    WRITE32(reg,value)

//----------------------------------------------------------------------
// VIC registers
//----------------------------------------------------------------------
/*! @brief VIC registers offset
 *  @code
 */
#define LLD_VIC_IRQSR0                      (LLD_VIC_ADDRESS+0x000)
#define LLD_VIC_FIQSR0                      (LLD_VIC_ADDRESS+0x004)
#define LLD_VIC_RIS0                        (LLD_VIC_ADDRESS+0x008)
#define LLD_VIC_ISEL0                       (LLD_VIC_ADDRESS+0x00C)
#define LLD_VIC_IENS0                       (LLD_VIC_ADDRESS+0x010)
#define LLD_VIC_IENC0                       (LLD_VIC_ADDRESS+0x014)
#define LLD_VIC_SWISR0                      (LLD_VIC_ADDRESS+0x018)
#define LLD_VIC_SWICR0                      (LLD_VIC_ADDRESS+0x01C)

#define LLD_VIC_IRQSR1                      (LLD_VIC_ADDRESS+0x020)
#define LLD_VIC_FIQSR1                      (LLD_VIC_ADDRESS+0x024)
#define LLD_VIC_RIS1                        (LLD_VIC_ADDRESS+0x028)
#define LLD_VIC_ISEL1                       (LLD_VIC_ADDRESS+0x02C)
#define LLD_VIC_IENS1                       (LLD_VIC_ADDRESS+0x030)
#define LLD_VIC_IENC1                       (LLD_VIC_ADDRESS+0x034)
#define LLD_VIC_SWISR1                      (LLD_VIC_ADDRESS+0x038)
#define LLD_VIC_SWICR1                      (LLD_VIC_ADDRESS+0x03C)

#define LLD_VIC_PER                         (LLD_VIC_ADDRESS+0x040)
#define LLD_VIC_VAR                         (LLD_VIC_ADDRESS+0x050)
#define LLD_VIC_DVAR                        (LLD_VIC_ADDRESS+0x054)
#define LLD_VIC_VCR0                        (LLD_VIC_ADDRESS+0x200)
#define LLD_VIC_VCR1                        (LLD_VIC_ADDRESS+0x204)
#define LLD_VIC_VCR2                        (LLD_VIC_ADDRESS+0x208)
#define LLD_VIC_VCR3                        (LLD_VIC_ADDRESS+0x20C)
#define LLD_VIC_VCR4                        (LLD_VIC_ADDRESS+0x210)
#define LLD_VIC_VCR5                        (LLD_VIC_ADDRESS+0x214)
#define LLD_VIC_VCR6                        (LLD_VIC_ADDRESS+0x218)
#define LLD_VIC_VCR7                        (LLD_VIC_ADDRESS+0x21C)
#define LLD_VIC_VCR8                        (LLD_VIC_ADDRESS+0x220)
#define LLD_VIC_VCR9                        (LLD_VIC_ADDRESS+0x224)
#define LLD_VIC_VCR10                       (LLD_VIC_ADDRESS+0x228)
#define LLD_VIC_VCR11                       (LLD_VIC_ADDRESS+0x22C)
#define LLD_VIC_VCR12                       (LLD_VIC_ADDRESS+0x230)
#define LLD_VIC_VCR13                       (LLD_VIC_ADDRESS+0x234)
#define LLD_VIC_VCR14                       (LLD_VIC_ADDRESS+0x238)
#define LLD_VIC_VCR15                       (LLD_VIC_ADDRESS+0x23C)
#define LLD_VIC_VAR0                        (LLD_VIC_ADDRESS+0x100)
#define LLD_VIC_VAR1                        (LLD_VIC_ADDRESS+0x104)
#define LLD_VIC_VAR2                        (LLD_VIC_ADDRESS+0x108)
#define LLD_VIC_VAR3                        (LLD_VIC_ADDRESS+0x10C)
#define LLD_VIC_VAR4                        (LLD_VIC_ADDRESS+0x110)
#define LLD_VIC_VAR5                        (LLD_VIC_ADDRESS+0x114)
#define LLD_VIC_VAR6                        (LLD_VIC_ADDRESS+0x118)
#define LLD_VIC_VAR7                        (LLD_VIC_ADDRESS+0x11C)
#define LLD_VIC_VAR8                        (LLD_VIC_ADDRESS+0x120)
#define LLD_VIC_VAR9                        (LLD_VIC_ADDRESS+0x124)
#define LLD_VIC_VAR10                       (LLD_VIC_ADDRESS+0x128)
#define LLD_VIC_VAR11                       (LLD_VIC_ADDRESS+0x12C)
#define LLD_VIC_VAR12                       (LLD_VIC_ADDRESS+0x130)
#define LLD_VIC_VAR13                       (LLD_VIC_ADDRESS+0x134)
#define LLD_VIC_VAR14                       (LLD_VIC_ADDRESS+0x138)
#define LLD_VIC_VAR15                       (LLD_VIC_ADDRESS+0x13C)
#define LLD_VIC_PERIPHID0                   (LLD_VIC_ADDRESS+0xFE0)
#define LLD_VIC_PERIPHID1                   (LLD_VIC_ADDRESS+0xFE4)
#define LLD_VIC_PERIPHID2                   (LLD_VIC_ADDRESS+0xFE8)
#define LLD_VIC_PERIPHID3                   (LLD_VIC_ADDRESS+0xFEC)
#define LLD_VIC_CELLID0                     (LLD_VIC_ADDRESS+0xFF0)
#define LLD_VIC_CELLID1                     (LLD_VIC_ADDRESS+0xFF4)
#define LLD_VIC_CELLID2                     (LLD_VIC_ADDRESS+0xFF8)
#define LLD_VIC_CELLID3                     (LLD_VIC_ADDRESS+0xFFC)

/*! @endcode */

//----------------------------------------------------------------------
// typedefs
//----------------------------------------------------------------------
/*! @brief It's the type definition for VIC channels (0..63) */
typedef tU8 LLD_VIC_ChannelTy;

/*! @brief It's the type definition for VIC priorities */
typedef enum
{
	LLD_VIC_PRIORITY0,						/*!< It's the enum value for VIC's priority number 0 (highest priority) */
	LLD_VIC_PRIORITY1,						/*!< It's the enum value for VIC's priority number 1 */
	LLD_VIC_PRIORITY2,                      /*!< It's the enum value for VIC's priority number 2 */
	LLD_VIC_PRIORITY3,						/*!< It's the enum value for VIC's priority number 3 */
	LLD_VIC_PRIORITY4,						/*!< It's the enum value for VIC's priority number 4 */
	LLD_VIC_PRIORITY5,						/*!< It's the enum value for VIC's priority number 5 */
	LLD_VIC_PRIORITY6,						/*!< It's the enum value for VIC's priority number 6 */
	LLD_VIC_PRIORITY7,						/*!< It's the enum value for VIC's priority number 7 */
	LLD_VIC_PRIORITY8,						/*!< It's the enum value for VIC's priority number 8 */
	LLD_VIC_PRIORITY9,						/*!< It's the enum value for VIC's priority number 9 */
	LLD_VIC_PRIORITY10,						/*!< It's the enum value for VIC's priority number 10 */
	LLD_VIC_PRIORITY11,						/*!< It's the enum value for VIC's priority number 11 */
	LLD_VIC_PRIORITY12,						/*!< It's the enum value for VIC's priority number 12 */
	LLD_VIC_PRIORITY13,						/*!< It's the enum value for VIC's priority number 13 */
	LLD_VIC_PRIORITY14,						/*!< It's the enum value for VIC's priority number 14 */
	LLD_VIC_PRIORITY15,						/*!< It's the enum value for VIC's priority number 15 (lowest priority) */
} LLD_VIC_PriorityLevelTy;

/*! @brief It's the type definition for VIC interrupt assignment status */
typedef enum
{
	LLD_VIC_VECTORED_INTERRUPT,	            /*!< The channel is assigned to a vectored interrupt */
	LLD_VIC_NON_VECTORED_INTERRUPT,			/*!< The channel is assigned to a non vectored interrupt */
	LLD_VIC_NON_VECTORED_INITIALIZED,		/*!< The function pointers array for non vectored interrupts has been initialized */
	LLD_VIC_NON_VECTORED_NOT_INITIALIZED,   /*!< The function pointers array for non vectored interrupts has not been initialized */
	LLD_VIC_NOT_ASSIGNED,					/*!< The channel is not assigned to any interrupt source */
}LLD_VIC_ErrorStatusTy;

/*! @brief It's the type definition for VIC's interrupts types */
typedef enum
{
    LLD_VIC_IRQ_INTERRUPT,                  /*!< The channel is dedicated to IRQ interrupts only */
    LLD_VIC_FIQ_INTERRUPT					/*!< The channel is dedicated to FIQ interrupts only */
}LLD_VIC_TypeTy;

/*! @brief It's the type definition for VIC Control Register */
typedef VicVcrTy LLD_VIC_VcrTy;

/*! @brief It's the type definition for the generic ISR function pointer */
typedef tVoid (*LLD_VIC_IsrHandlerTy)(tVoid);

/*! @brief It's the type definition for the function pointers array */
typedef LLD_VIC_IsrHandlerTy LLD_VIC_NonVectoredTableTy[64];

//----------------------------------------------------------------------
// function prototypes
//----------------------------------------------------------------------
tVoid                   LLD_VIC_DefaultISR(tVoid);
tVoid                   LLD_VIC_ClearAllPriorities(tVoid);
LLD_VIC_ErrorStatusTy   LLD_VIC_VectoredChannelConfig(LLD_VIC_ChannelTy channel, LLD_VIC_PriorityLevelTy priority, LLD_VIC_IsrHandlerTy isr);
tVoid                   LLD_VIC_NonVectoredInterruptManager(LLD_VIC_NonVectoredTableTy *table);
LLD_VIC_ErrorStatusTy   LLD_VIC_NonVectoredTableInit(LLD_VIC_IsrHandlerTy isr, LLD_VIC_NonVectoredTableTy *table);
LLD_VIC_ErrorStatusTy   LLD_VIC_NonVectoredChannelConfig(LLD_VIC_ChannelTy channel, LLD_VIC_NonVectoredTableTy *table, LLD_VIC_IsrHandlerTy isr);
tVoid                   LLD_VIC_SetProtectionStatus(tBool flag);
tBool                   LLD_VIC_GetProtectionStatus(tVoid);
tVoid                   LLD_VIC_SetCurrentInterruptAddress(LLD_VIC_IsrHandlerTy isr);
LLD_VIC_IsrHandlerTy    LLD_VIC_GetCurrentInterruptAddress(tVoid);
tVoid                   LLD_VIC_SetDefaultInterruptAddress(LLD_VIC_IsrHandlerTy isr);
LLD_VIC_IsrHandlerTy    LLD_VIC_GetDefaultInterruptAddress(tVoid);
tVoid                   LLD_VIC_SetVectoredInterruptAddress(LLD_VIC_PriorityLevelTy priority, LLD_VIC_IsrHandlerTy isr);
LLD_VIC_IsrHandlerTy    LLD_VIC_GetVectoredInterruptAddress(LLD_VIC_PriorityLevelTy priority);
tVoid                   LLD_VIC_SetVectoredInterruptControlRegister(LLD_VIC_ChannelTy channel, LLD_VIC_PriorityLevelTy priority, tBool enable_bit);
LLD_VIC_VcrTy           *LLD_VIC_GetVectoredInterruptControlRegister(LLD_VIC_PriorityLevelTy priority);
tVoid                   LLD_VIC_SetInterruptType(LLD_VIC_ChannelTy channel, LLD_VIC_TypeTy type);
LLD_VIC_TypeTy          LLD_VIC_GetInterruptType(LLD_VIC_ChannelTy channel);
tVoid                   LLD_VIC_EnableChannel(LLD_VIC_ChannelTy channel);
tVoid                   LLD_VIC_DisableChannel(LLD_VIC_ChannelTy channel);
tVoid                   LLD_VIC_DisableAllChannels(tVoid);
tVoid                   LLD_VIC_EnableAllChannels(tVoid);
tVoid                   LLD_VIC_BackupAndDisableAllChannels(tU64 *interrupt_status);
tVoid                   LLD_VIC_RestoreAllChannelsStatus(tU64 *interrupt_status);
tVoid                   LLD_VIC_GenerateSoftwareInterrupt(LLD_VIC_ChannelTy channel);
tVoid                   LLD_VIC_ClearSoftwareInterrupt(LLD_VIC_ChannelTy channel);
tU64                    LLD_VIC_GetRawInterruptStatus(tVoid);
tU64                    LLD_VIC_GetIRQInterruptStatus(tVoid);
tU64                    LLD_VIC_GetFIQInterruptStatus(tVoid);
tVoid                   LLD_VIC_SetDefaultISR(LLD_VIC_IsrHandlerTy isr);

#if (LLD_VIC_STA660_ROM_USED == TRUE)
#define LLD_VIC_DefaultISR                                   ((tVoid (*)(tVoid))ROM_LLD_Table[ENU_LLD_VIC_DefaultISR])
#define LLD_VIC_ClearAllPriorities                           ((tVoid (*)(tVoid))ROM_LLD_Table[ENU_LLD_VIC_ClearAllPriorities])
#define LLD_VIC_VectoredChannelConfig(a,b,c)                 ((LLD_VIC_ErrorStatusTy (*)(LLD_VIC_ChannelTy,LLD_VIC_PriorityLevelTy,LLD_VIC_IsrHandlerTy))ROM_LLD_Table[ENU_LLD_VIC_VectoredChannelConfig])(a,b,c)
#define LLD_VIC_NonVectoredInterruptManager(a)               ((tVoid (*)(LLD_VIC_NonVectoredTableTy *))ROM_LLD_Table[ENU_LLD_VIC_NonVectoredInterruptManager])(a)
#define LLD_VIC_NonVectoredTableInit(a,b)                    ((LLD_VIC_ErrorStatusTy (*)(LLD_VIC_IsrHandlerTy,LLD_VIC_NonVectoredTableTy *))ROM_LLD_Table[ENU_LLD_VIC_NonVectoredTableInit])(a,b)
#define LLD_VIC_NonVectoredChannelConfig(a,b,c)              ((LLD_VIC_ErrorStatusTy (*)(LLD_VIC_ChannelTy,LLD_VIC_NonVectoredTableTy *,LLD_VIC_IsrHandlerTy))ROM_LLD_Table[ENU_LLD_VIC_NonVectoredChannelConfig])(a,b,c)
#define LLD_VIC_SetProtectionStatus(a)                       ((tVoid (*)(tBool))ROM_LLD_Table[ENU_LLD_VIC_SetProtectionStatus])(a)
#define LLD_VIC_GetProtectionStatus                          ((tBool (*)(tVoid))ROM_LLD_Table[ENU_LLD_VIC_GetProtectionStatus])
#define LLD_VIC_SetCurrentInterruptAddress(a)                ((tVoid (*)(LLD_VIC_IsrHandlerTy))ROM_LLD_Table[ENU_LLD_VIC_SetCurrentInterruptAddress])(a)
#define LLD_VIC_GetCurrentInterruptAddress                   ((LLD_VIC_IsrHandlerTy (*)(tVoid))ROM_LLD_Table[ENU_LLD_VIC_GetCurrentInterruptAddress])
#define LLD_VIC_SetDefaultInterruptAddress(a)                ((tVoid (*)(LLD_VIC_IsrHandlerTy))ROM_LLD_Table[ENU_LLD_VIC_SetDefaultInterruptAddress])(a)
#define LLD_VIC_GetDefaultInterruptAddress                   ((LLD_VIC_IsrHandlerTy (*)(tVoid))ROM_LLD_Table[ENU_LLD_VIC_GetDefaultInterruptAddress])
#define LLD_VIC_SetVectoredInterruptAddress(a,b)             ((tVoid (*)(LLD_VIC_PriorityLevelTy,LLD_VIC_IsrHandlerTy))ROM_LLD_Table[ENU_LLD_VIC_SetVectoredInterruptAddress])(a,b)
#define LLD_VIC_GetVectoredInterruptAddress(a)               ((LLD_VIC_IsrHandlerTy (*)(LLD_VIC_PriorityLevelTy))ROM_LLD_Table[ENU_LLD_VIC_GetVectoredInterruptAddress])(a)
#define LLD_VIC_SetVectoredInterruptControlRegister(a,b,c)   ((tVoid (*)(LLD_VIC_ChannelTy,LLD_VIC_PriorityLevelTy,tBool))ROM_LLD_Table[ENU_LLD_VIC_SetVectoredInterruptControlRegister])(a,b,c)
#define LLD_VIC_GetVectoredInterruptControlRegister(a)       ((LLD_VIC_VcrTy (*)(LLD_VIC_PriorityLevelTy))ROM_LLD_Table[ENU_LLD_VIC_GetVectoredInterruptControlRegister])(a)
#define LLD_VIC_SetInterruptType(a,b)                        ((tVoid (*)(LLD_VIC_ChannelTy,LLD_VIC_TypeTy))ROM_LLD_Table[ENU_LLD_VIC_SetInterruptType])(a,b)
#define LLD_VIC_GetInterruptType(a)                          ((LLD_VIC_TypeTy (*)(LLD_VIC_ChannelTy))ROM_LLD_Table[ENU_LLD_VIC_GetInterruptType])(a)
#define LLD_VIC_EnableChannel(a)                             ((tVoid (*)(LLD_VIC_ChannelTy))ROM_LLD_Table[ENU_LLD_VIC_EnableChannel])(a)
#define LLD_VIC_DisableChannel(a)                            ((tVoid (*)(LLD_VIC_ChannelTy))ROM_LLD_Table[ENU_LLD_VIC_DisableChannel])(a)
#define LLD_VIC_DisableAllChannels                           ((tVoid (*)(tVoid))ROM_LLD_Table[ENU_LLD_VIC_DisableAllChannels])
#define LLD_VIC_EnableAllChannels                            ((tVoid (*)(tVoid))ROM_LLD_Table[ENU_LLD_VIC_EnableAllChannels])
#define LLD_VIC_BackupAndDisableAllChannels(a)               ((tVoid (*)(tU64 *))ROM_LLD_Table[ENU_LLD_VIC_BackupAndDisableAllChannels])(a)
#define LLD_VIC_RestoreAllChannelsStatus(a)                  ((tVoid (*)(tU64 *))ROM_LLD_Table[ENU_LLD_VIC_RestoreAllChannelsStatus])(a)
#define LLD_VIC_GenerateSoftwareInterrupt(a)                 ((tVoid (*)(LLD_VIC_ChannelTy))ROM_LLD_Table[ENU_LLD_VIC_GenerateSoftwareInterrupt])(a)
#define LLD_VIC_ClearSoftwareInterrupt(a)                    ((tVoid (*)(LLD_VIC_ChannelTy))ROM_LLD_Table[ENU_LLD_VIC_ClearSoftwareInterrupt])(a)
#define LLD_VIC_GetRawInterruptStatus                        ((tU64 (*)(tVoid))ROM_LLD_Table[ENU_LLD_VIC_GetRawInterruptStatus])
#define LLD_VIC_GetIRQInterruptStatus                        ((tU64 (*)(tVoid))ROM_LLD_Table[ENU_LLD_VIC_GetIRQInterruptStatus])
#define LLD_VIC_GetFIQInterruptStatus                        ((tU64 (*)(tVoid))ROM_LLD_Table[ENU_LLD_VIC_GetFIQInterruptStatus])
#define LLD_VIC_SetDefaultISR(a)                             ((tVoid (*)(LLD_VIC_IsrHandlerTy))ROM_LLD_Table[ENU_LLD_VIC_SetDefaultISR])(a)
#endif

#endif  // _LLD_VIC_H_

// End of file
