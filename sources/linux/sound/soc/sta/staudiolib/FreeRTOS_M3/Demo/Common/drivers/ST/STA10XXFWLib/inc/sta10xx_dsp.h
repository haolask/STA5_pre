/***********************************************************************************/
/*!
*  \file      sta10xx_dsp.h
*
*  \brief     <i><b> This file provides all the DSP firmware header </b></i>
*
*  \details   This file provides all the DSP firmware header.
*
*  \author    APG-MID Application Team
*
*  \author    (original version) APG-MID Application Team
*
*  \version   0.1
*
*  \date      20130514
*
*  \bug       see Release Note
*
*  \warning   usage policy :
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* FOR MORE INFORMATION PLEASE READ CAREFULLY THE LICENSE AGREEMENT
* LOCATED IN THE ROOT DIRECTORY OF THIS SOFTWARE PACKAGE.
*/
/***********************************************************************************/

#ifndef _STA10XX_DSP_H_
#define _STA10XX_DSP_H_

#ifdef __cplusplus
extern "C" {  /* In case C++ needs to use this header.*/
#endif


#include "sta10xx_type.h"

/*
DSP_BASE[3] is defined in
- sta10xx_dsp.c in FREE_RTOS
- api.c and set at compile time in TKERNEL
- api.c and set at runtime time in LINUX_OS
*/
extern u32 DSP_BASE[3];


//DSP offsets
#define DSP_PRAM_OFFSET(core)				0x0
#define DSP_XRAM_OFFSET(core)				0x80000
#define DSP_YRAM_OFFSET(core)				0xC0000
#define DSP_DBGPORT_OFFSET(core)			0x40000
#define DSP_XIN_OFFSET(core)				(((u32)(core) == 2) ? 0x8c000 : 0x84000)
#define DSP_XOUT_OFFSET(core)				(((u32)(core) == 2) ? 0x8c200 : 0x84200)
#define DSP_DSP2ARM_IRQ_SET_OFFSET(core)	(((u32)(core) == 2) ? 0x90000 : 0x88000)
#define DSP_DSP2ARM_IRQ_CLR_OFFSET(core)	(((u32)(core) == 2) ? 0x90004 : 0x88004)
#define DSP_ARM2DSP_IRQ_EN_OFFSET(core)		(((u32)(core) == 2) ? 0x90008 : 0x88008)
#define DSP_ARM2DSP_IRQ_STS_OFFSET(core)	(((u32)(core) == 2) ? 0x9000C : 0x8800C)

//DSP mem addresses
#define DSP_PRAM_ADDR(core)					( (vu32*)(DSP_BASE[(core)] + DSP_PRAM_OFFSET(core)))
#define DSP_XRAM_ADDR(core)					( (vu32*)(DSP_BASE[(core)] + DSP_XRAM_OFFSET(core)))
#define DSP_YRAM_ADDR(core)					( (vu32*)(DSP_BASE[(core)] + DSP_YRAM_OFFSET(core)))
#define DSP_XIN_ADDR(core)					( (vu32*)(DSP_BASE[(core)] + DSP_XIN_OFFSET(core)))
#define DSP_XOUT_ADDR(core)					( (vu32*)(DSP_BASE[(core)] + DSP_XOUT_OFFSET(core)))

//DSP IRQ registers
#define DSP_DSP2ARM_IRQ_SET(core)			(*(vu32*)(DSP_BASE[(core)] + DSP_DSP2ARM_IRQ_SET_OFFSET(core)))
#define DSP_DSP2ARM_IRQ_CLR(core)			(*(vu32*)(DSP_BASE[(core)] + DSP_DSP2ARM_IRQ_CLR_OFFSET(core)))
#define DSP_ARM2DSP_IRQ_EN(core)			(*(vu32*)(DSP_BASE[(core)] + DSP_ARM2DSP_IRQ_EN_OFFSET(core)))
#define DSP_ARM2DSP_IRQ_STS(core)			(*(vu32*)(DSP_BASE[(core)] + DSP_ARM2DSP_IRQ_STS_OFFSET(core)))

//DSP mems sizes
#define DSP_PRAM_SIZE(core)					(((u32)(core) == 2) ?  8*1024 : 6*1024)
#define DSP_XRAM_SIZE(core)					(((u32)(core) == 2) ? 12*1024 : 4*1024)
#define DSP_YRAM_SIZE(core)					(((u32)(core) == 2) ? 12*1024 : 4*1024)

#define DSP0_PRAM_SIZE						( 6*1024)
#define DSP0_XRAM_SIZE						( 4*1024)
#define DSP0_YRAM_SIZE						( 4*1024)
#define DSP1_PRAM_SIZE						( 6*1024)
#define DSP1_XRAM_SIZE						( 4*1024)
#define DSP1_YRAM_SIZE						( 4*1024)
//ACCORDO2
#define DSP2_PRAM_SIZE						( 8*1024)
#define DSP2_XRAM_SIZE						(12*1024)
#define DSP2_YRAM_SIZE						(12*1024)


//DSP to ARM address conversions
//#define DSP_XMEM2ARM(core, offset)	((u32*)DSP_XRAM_ADDR(core) + (u32)(offset))
//#define DSP_YMEM2ARM(core, offset)	((u32*)DSP_YRAM_ADDR(core) + (u32)(offset))
//#define DSP_ARM2XMEM(core, offset)	((u32*)(((u32)(offset) - (u32)DSP_XRAM_ADDR(core))>>2))
//#define DSP_ARM2YMEM(core, offset)	((u32*)(((u32)(offset) - (u32)DSP_YRAM_ADDR(core))>>2))

#define DSP_DSP2ARM(base,  offset)		((u32*)(base)          + (u32)(offset))
#define DSP_ARM2DSP(base,  offset)		((u32*)(((u32)(offset) - (u32)(base))>>2))


//Sizeof in Words
#define DSP_WSIZEOF(type)				(sizeof(type)>>2)


#ifdef FREE_RTOS
//M3 image addresses from where to load the dsp binaries
extern u32 address_P0, address_X0, address_Y0, size_P0, size_X0, size_Y0;
extern u32 address_P1, address_X1, address_Y1, size_P1, size_X1, size_Y1;
extern u32 address_P2, address_X2, address_Y2, size_P2, size_X2, size_Y2;
#endif

/*
typedef enum {
    DSP_DBG_NULL           = 0x0,
    DSP_DBG_RESET          = 0x1,
    DSP_DBG_BREAK          = 0x2,
    DSP_DBG_RUN            = 0x3,
    DSP_DBG_STEP           = 0x6,
    DSP_DBG_BREAKPOINT_SET = 0xD,
    DSP_DBG_BREAKPOINT_CLR = 0x5,
} DSP_DbgCmd;
*/

typedef enum {
    DSP_0,
    DSP_1,
    DSP_2
} DSP_Core;

typedef enum {
    DSP_PRAM,
    DSP_XRAM,
    DSP_YRAM,
} DSP_MemType;

typedef enum {
	DSP_BINARY_0,
	DSP_BINARY_1,
	DSP_BINARY_2,
} DSP_Binary;

//note: these enum values match the DSP irq line values (and the SSY_EIRx register mask)
typedef enum {
	DSP_IRQ_ARM1 	= 1,
	DSP_IRQ_ARM2	= 2,
	DSP_IRQ_ARM3	= 3,
	DSP_IRQ_FSYNC	= 4,
	DSP_IRQ_CK1		= 5,
	DSP_IRQ_CK2		= 6,
	DSP_IRQ_ARM7	= 7,
} DSP_IRQs;


//DSP IRQ handlers
typedef void (*DSP_IRQHandlerFunc)(DSP_Core core);


bool DSP_ResetMem(DSP_Core core, DSP_MemType memtype);
void DSP_memset(void* ptr, int value, int num);
void DSP_memcpy(void* dst, const void* src, int num);

bool DSP_LoadBinary(DSP_Core core, DSP_MemType memtype, const void* data, int size);
bool DSP_LoadDefaultFw(DSP_Core core, DSP_Binary bin);
bool DSP_LoadBinaryFile(DSP_Core core, const char* path, void* ramBuf);

void DSP_Start(DSP_Core core);
void DSP_Stop(DSP_Core core);

//IRQ
void DSP_SetIRQHandler(DSP_Core core, DSP_IRQHandlerFunc DSP_IRQHandlerFunc);
void DSP0_IRQHandler(void);
void DSP1_IRQHandler(void);
void DSP2_IRQHandler(void);
void DSP_SendIRQ(DSP_Core core, int irqline);
u32  DSP_EnableIRQ(DSP_Core core, int irqline); //returns "non-zero" if the IRQ was enable before enabling it


#ifdef __cplusplus
}
#endif

#endif /* _STA10XX_DSP_H_ */
