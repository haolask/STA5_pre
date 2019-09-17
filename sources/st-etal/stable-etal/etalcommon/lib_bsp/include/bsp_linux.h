//!
//!  \file 		bsp_linux.h
//!  \brief 	<i><b> BSP for Linux-based hardware resources access </b></i>
//!  \details   Primitives to abstract the access to some low level resources
//!             made available by the linux kernel
//!  \author 	Raffaele Belardi
//!

/*
#ifdef CONFIG_ETAL_SUPPORT_CMOST
#include "tunerdriver_internal.h"
#include "tunerdriver.h"
#endif

#ifdef CONFIG_ETAL_SUPPORT_DCOP_HDRADIO
#include "hdradio_internal.h"
#include "HDRADIO_Protocol.h"
#endif
*/

/*
 * Modes for BSP_Linux_OpenGPIO
 */
#define GPIO_OPEN_WRITE  (FALSE)
#define GPIO_OPEN_READ   (TRUE)

#define GPIO_IRQ_NONE    0
#define GPIO_IRQ_RISING  1
#define GPIO_IRQ_FALLING 2

tS32  BSP_Linux_OpenGPIO(tChar *gpio_name, tBool mode, tU32 interrupt_mode, tVoid *IRQCallbackFunction);
tVoid BSP_Linux_WriteGPIO(tS32 fd, tBool req_value);
tBool BSP_Linux_ReadGPIO(tS32 fd);
tVoid BSP_Linux_CloseGPIO(tS32 fd);

tS32  BSP_Linux_OpenDevice(tChar *dev_name);
tS32  BSP_Linux_CloseDevice(tS32 fd);
tVoid BSP_Linux_TransferSpi(tS32 fd, tU8 *buf_wr, tU8 *buf_rd, tU32 len, tBool vI_dataSize32bits);
tVoid BSP_Linux_WaitForInterrupt(tS32 fd);
#if defined(CONFIG_DIGITAL_AUDIO) || defined(CONFIG_ETAL_MDR_AUDIO_CODEC_ON_HOST)
tVoid BSP_Linux_SystemCmd(tChar *cmd_line);
#endif
