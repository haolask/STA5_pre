/*
 * ============================================================================
 *
 *       Filename:  rom_interface.h
 *
 *    Description:  A5 ROM interface description
 *
 *        Version:  2.0
 *        Created:  October 2017
 *
 *         Author:  Philippe Langlais
 *   Organization:  ADG-MID Application Team
 *
 * ============================================================================
 */

#ifndef __ROM_INTERFACE__
#define __ROM_INTERFACE__

/* ROM functions jump table address */
#define JUMP_TABLE_ROMADDR		0x10047E00

typedef int (*jmp_table_t)();

/*
 * ROM functions table indexes
 */
enum romFNIndex {
/* Api reuse self APIs */
	Api_Reuse_Get_Api_Address,
	Api_Reuse_Get_Secr_Api_Address,
	Api_Reuse_Get_Secr_Context_Address,

/* UART BOOT API functions */
	UART_Init,
	UART_SendData,
	UART_GetData,
	UART_GetData_until_Timer,
	UART_GetNbBytesReceived,

/* TIMER BOOT API functions */
	TIMER_Init,
	TIMER_StartTimer,
	TIMER_StopTimer,
	TIMER_HasTimerReachedZero,

/* FLASH BOOT API functions */
	boot_NANDFsmcInit,
	boot_NandPowerOn,
	boot_NandAutoDetect,
	boot_NandSetConfig,
	boot_NandDirectDetect,
	boot_NandReadPage,
	boot_NandReadMbr,
	boot_NandAddressGen,
	boot_NandECCRead,
	boot_FSMCECCInit,
	boot_NandECCCheckAndFix,
	boot_NandErrorFix,

/* CRC API function */
	calculate_ether_crc32,

/* SDMMC BOOT API functions */
	MMC_Init,
	MMC_PowerON,
	MMC_InitializeCards,
	MMC_ReadBlocks,
	MMC_CmdError,
	MMC_CmdResp1Error,
	MMC_CmdResp2Error,
	MMC_CmdResp3Error,
	MMC_CmdResp6Error,
	MMC_CmdResp7Error,
	MMC_ResetGV,
	MMC_GetMbr,
	MMC_BootStageInit,
	MMC_GetMBR_Xloader_Length,
	MMC_GetMBR_Code_Offset,
	MMC_GetMBR_Header_Crc,
	MMC_ChangeFrequency,

/* GPIO BOOT API functions */
	GPIO_Init,
	GPIO_SetPinConfig,
	GPIO_ReadGpioPin,
	GPIO_Settings_SDMMC1_1_set,
	GPIO_Settings_SDMMC1_2_set,
	GPIO_Settings_SDMMC2_1_set,
	GPIO_Settings_FSMC_x8,
	GPIO_Settings_SQI0,
	GPIO_Settings_SQI1,

/* SQI BOOT API functions */
	SQI_SetDummyCycles,
	SQI_ChangeFrequency,
	SQI_SetReadOpcode,
	SQI_SetWriteConfigOpcode,
	SQI_SetMode,
	SQI_SetCommand,
	SQI_SetCmdType,
	SQI_SetTransferLength,
	SQI_SetAddress,
	SQI_Read,

/* USB BOOT API functions */
	USBD_Init,
	USBD_RegisterClass,
	USBD_DFU_RegisterMedia,

/* Backup/Restore API functions */
	Jump_XLoader_Entry_point,

/* OTP Read functions */
	otp_Set_Firewall,
	otp_Get_Security_Level,
	otp_Is_Empty,
	otp_Read_Word,
	otp_Write_Word,
	otp_Get_Cust_ID,
	otp_Get_Cust_Cert_Ver,
	otp_Get_Secr_Ver,
	otp_Get_Cold_Boot_Info,
	otp_Get_Prv_Boot_Info,
	otp_Incr_Cust_Cert_Ver,
	otp_Incr_Secr_Ver,
	otp_Write_Prv_Boot_Info
};

/*
 * Return values for ROM functions
 */
#define MMC_OK	0

#endif /* __ROM_INTERFACE__ */
