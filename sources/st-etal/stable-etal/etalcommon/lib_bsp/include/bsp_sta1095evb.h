//!
//!  \file 		bsp_sta1095evb.h
//!  \brief 	<i><b> ETAL BSP for STA1095 EVB </b></i>
//!  \details   Low level drivers for the Accordo2 EVB
//!  \author 	Raffaele Belardi
//!

#ifndef _BSP_STA1095EVB_H_
#define _BSP_STA1095EVB_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_ETAL_SUPPORT_CMOST
#include "tunerdriver_internal.h"
#include "tunerdriver.h"
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
#include "hdradio_internal.h"
#include "HDRADIO_Protocol.h"
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
#include "DAB_Protocol.h"
#include "DAB_internal.h"
#endif

/**************************************
 * Types
 *************************************/
typedef enum
{
	BSP_WRITE_OPERATION,
	BSP_READ_OPERATION
} BSPPrintOperationTy;

#if defined(CONFIG_DIGITAL_AUDIO) || defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
typedef enum
{
	BSP_AUDIO_SELECT_RX1,
	BSP_AUDIO_SELECT_RX2
} BSPAudioSelectTy;
#endif

#if defined(CONFIG_HOST_OS_LINUX) && defined(CONFIG_BOARD_ACCORDO5) && defined (CONFIG_ETAL_SUPPORT_EARLY_TUNER)
typedef struct
{
	tU32 Freq;
	tU32 Band;
}BSPCtxBackupEarlyAudioTy;
#endif
/**************************************
 * defines
 *************************************/

/**************************************
 * Functions
 *************************************/
#ifdef CONFIG_ETAL_SUPPORT_CMOST
tSInt BSP_DeviceReset_CMOST(tU32 deviceID);
tS32  BSP_BusConfig_CMOST(tU32 deviceID);
tS32  BSP_BusUnconfig_CMOST(tU32 deviceID);
tS32  BSP_Write_CMOST(tU32 deviceID, tU8* buf, tU32 len);
tS32  BSP_Read_CMOST(tU32 deviceID, tU8* buf, tU32 len);
tVoid BSP_TransferSpi_CMOST(tU32 deviceID, tU8 *buf_wr, tU8 *buf_rd, tU32 len);
tS32 BSP_Write_CMOST_SPI(tU32 deviceID, tU8* buf, tU32 len);
tS32 BSP_WriteRead_CMOST_SPI(tU32 deviceID, tU8 *pI_InputBuf, tU16 vI_ByteToWrite, tU8* pO_OutBuf, tU16 vI_ByteToRead, tBool vI_ChipSelectControl);
tSInt BSP_SetCS_CMOST_SPI(tU32 deviceID, tBool vI_value);
tVoid BSP_WaitForRDSInterrupt_CMOST(tU32 deviceID);
tVoid BSP_setSignallingGPIO(tBool val);
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
tSInt BSP_DeviceReset_MDR(tU32 deviceID);
tSInt BSP_DeviceInit_MDR(tU32 deviceID);
tVoid BSP_DeviceDeinit_MDR(tVoid);
tVoid BSP_TransferSpi_MDR(tU8 *buf_wr, tU8 *buf_rd, tU32 len);
tU8   BSP_SteciReadREQ_MDR(tVoid);
tVoid BSP_SteciSetCS_MDR(tBool value);
tVoid BSP_SteciSetBOOT_MDR(tBool value);
tSInt BSP_BusConfigSPIFrequency_MDR(tU32 deviceID);
#elif defined(CONFIG_ETAL_SUPPORT_DCOP_RESET_LIGHT_FREERTOS)
tSInt BSP_DeviceReset_MDR(tU32 deviceID);
tVoid BSP_SteciSetBOOT_MDR(tU32 GPIOnb, tBool value);
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
tSInt BSP_DeviceReset_HDRADIO(tU32 deviceID);
tSInt BSP_BusConfig_HDRADIO(tU32 deviceID);
tSInt BSP_BusConfigSPIMode_HDRADIO(tU32 deviceID);
tSInt BSP_BusConfigSPIFrequency_HDRADIO(tU32 deviceID);
tSInt BSP_BusConfigSPIDataSize_HDRADIO(tU32 deviceID, tBool vI_dataSizeIs32bits);
tSInt BSP_BusUnconfig_HDRADIO(tU32 deviceID);
tSInt BSP_DeviceInit_HDRADIO(tU32 deviceID);
tSInt BSP_DeviceDeinit_HDRADIO(tU32 deviceID);
tS32  BSP_Write_HDRADIO(tU32 deviceID, tU8* buf, tU32 len);
tS32  BSP_Read_HDRADIO(tU32 deviceID, tU8** pBuf);
tS32  BSP_Raw_Read_HDRADIO(tU32 deviceID, tU8 *pBuf, tU32 len);
tVoid BSP_TransferSpi_HDRADIO(tU32 deviceID, tU8 *buf_wr, tU8 *buf_rd, tU32 len);
tVoid BSP_SPI0DriveCS_HDRADIO(tBool value);
#endif

#if defined(CONFIG_DIGITAL_AUDIO) || defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
tVoid BSP_AudioSelect(BSPAudioSelectTy src);
#endif

#if defined(CONFIG_HOST_OS_LINUX) && defined(CONFIG_BOARD_ACCORDO5) && defined (CONFIG_ETAL_SUPPORT_EARLY_TUNER)
tS32 BSP_backup_context_for_early_audio(const BSPCtxBackupEarlyAudioTy *CtxEarlyAudio);
#endif

#ifdef __cplusplus
}
#endif
#endif // _BSP_STA1095EVB_H_
