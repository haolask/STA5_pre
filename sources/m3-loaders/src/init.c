/**
 * @file init.c
 * @brief Xloaders low level init (C bootstrap)
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author:  Philippe Langlais, ADG-MID Application Team
 */

#include <stdint.h>
#include <stdio.h>
#include "sta_src.h"
#include "trace.h"
#include "sta_common.h"
#ifdef DEBUG_EXCEPTION
#include "sta_wdt.h"
#else
#include "sta_pm.h"
#endif
#include "rpmx_common.h"

#define __init __attribute__((section(".init")))

/* Important!!! in this file, you can't call standard libc functions */

/* Addresses coming from the linker script */
extern uint32_t __vectors_start__;
extern uint32_t __stack_end__;
extern uint32_t __data_load_start__;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __rodata_load_start__;
extern uint32_t __rodata_start__;
extern uint32_t __rodata_end__;
extern uint32_t __text_load_start__;
extern uint32_t __text_start__;
extern uint32_t __text_end__;

extern uint32_t __rodataexidx_load_start__;
extern uint32_t __rodataexidx_start__;
extern uint32_t __rodataexidx_end__;

extern uint32_t __bss_start__;
extern uint32_t __bss_end__;

extern uint32_t __ESRAM_M3_start;
extern uint32_t __ESRAM_M3_end;
extern uint32_t __ESRAM_AP_start;
extern uint32_t __ESRAM_AP_end;
extern uint32_t __DRAM_start;

int main(void);
void reset_handler(void);
#define PendSVCHandler xPortPendSVHandler
#define SysTickHandler xPortSysTickHandler
#define SVCHandler vPortSVCHandler
#define CAN0_IRQHandler CAN0_ISR
#if SOC_ID == SOCID_STA1385
#define C3_HSM_IRQHandler ks_irq_handler
#else
#define C3_HSM_IRQHandler ccc_irq_handler
#endif

/**
 * @struct section_to_move_t
 * @brief defines a section to be moved
 */
struct section_to_move_t {
	uint32_t *load_start; /**< contains the offset in NAND */
	uint32_t *start; /**< contains the start address */
	uint32_t *end; /**< contains the end */
};

void __init exception_handler(uint32_t *p_fault_stack, int exception_no)
{
	/*
	 *  r0 = p_fault_stack[0];
	 *  r1 = p_fault_stack[1];
	 *  r2 = p_fault_stack[2];
	 *  r3 = p_fault_stack[3];
	 *  r12 = p_fault_stack[4];
	 *  lr = p_fault_stack[5];
	 *  pc = p_fault_stack[6];
	 *  psr = p_fault_stack[7];
	 */

#ifdef DEBUG_EXCEPTION
	trace_printf("Exception %d: lr=%08X, pc=%08X, psr=%08X\n", exception_no,
		     p_fault_stack[5], p_fault_stack[6], p_fault_stack[7]);
	wdt_disable();
	wait_forever;
#else
	char info_reboot[80];

	snprintf(info_reboot, sizeof(info_reboot) - 1,
		 "Exception %d: lr=%08X, pc=%08X, psr=%08X\n", exception_no,
		 (unsigned int)p_fault_stack[5],
		 (unsigned int)p_fault_stack[6],
		 (unsigned int)p_fault_stack[7]);
	__pm_reboot(info_reboot, NULL);
#endif
}

/*
 * The default fault handler implementation calls a function called
 * get_registers_from_stack()
 * It is a naked function - in effect this is just an assembly function.
 */
static void __init __attribute__((naked)) default_exception_handler(void)
{
	__asm volatile
		(
		 " tst lr, #4                                         \n"
		 " ite eq                                             \n"
		 " mrseq r0, msp                                      \n"
		 " mrsne r0, psp                                      \n"
		 /* Load the value of the interrupt control register into r1 from the
		  * address held in r3. */
		 " ldr r3, NVIC_INT_CTRL_CONST                        \n"
		 " ldr r1, [r3, #0]                                   \n"
		 /* The VECTACTIVE number is in the least significant byte,
		  * clear all other bits */
		 " uxtb r1, r1                                        \n"
		 " ldr r2, [r0, #24]                                  \n"
		 " ldr r3, handler_address                            \n"
		 " bx r3                                              \n"
		 " handler_address: .word exception_handler           \n"
		 " NVIC_INT_CTRL_CONST: .word 0xe000ed04              \n"
		);
}

void __init debug_isr_handler(int vectactive_no)
{
	/* IRQ[0] <=> VECTACIVE 16 */
	trace_printf("Uncaught IRQ %d\n", vectactive_no - 16);
	wait_forever;
}

/*
 * The default interrupt handler implementation calls a function called
 * debug_isr_handler(vectactive_no)
 * It is a naked function - in effect this is just an assembly function.
 */
static void __init __attribute__((naked)) default_isr_handler(void)
{
	__asm volatile
		(
		 /* Load the value of the interrupt control register into r0 from the
		  * address held in r3. */
		 " ldr r3, NVIC_INT_CTRL_CONST                        \n"
		 " ldr r0, [r3, #0]                                   \n"
		 /* The VECTACTIVE number is in the least significant byte,
		  * clear all other bits */
		 " uxtb r0, r0                                        \n"
		 " ldr r1, isr_handler_address                        \n"
		 " bx r1                                              \n"
		 " isr_handler_address: .word debug_isr_handler       \n"
		);
}

/* Default Exception handlers loop forever */
#define DEFAULT_EXCEPTION_HANDLER(name) \
	void __init __attribute__((weak)) name(void) \
	{ \
		default_exception_handler(); /* endless loop */ \
	}

/* Default IRQ handlers loop forever */
#define DEFAULT_ISR_HANDLER(name) \
	void __init __attribute__((weak)) name(void) \
	{ \
		default_isr_handler(); /* endless loop */ \
	}

DEFAULT_EXCEPTION_HANDLER(NMIException)         /* NMIException */
DEFAULT_EXCEPTION_HANDLER(HFHandler)            /* Hardware fault Handler */
DEFAULT_EXCEPTION_HANDLER(MemManageEception)    /* MemManageEception */
DEFAULT_EXCEPTION_HANDLER(BusFaultException)    /* BusFaultException */
DEFAULT_EXCEPTION_HANDLER(UsageFaultException)  /* UsageFaultException */

DEFAULT_EXCEPTION_HANDLER(SVCHandler)
DEFAULT_EXCEPTION_HANDLER(DebugMonitor)
DEFAULT_EXCEPTION_HANDLER(PendSVCHandler)
DEFAULT_EXCEPTION_HANDLER(SysTickHandler)

/* Specific M3 IRQs start here from IRQ0... */
DEFAULT_ISR_HANDLER(PMU_IRQHandler)
DEFAULT_ISR_HANDLER(SRC_IRQHandler)
DEFAULT_ISR_HANDLER(mtu_irq_handler)
DEFAULT_ISR_HANDLER(mailbox_irq_handler)
DEFAULT_ISR_HANDLER(CAN0_IRQHandler)
DEFAULT_ISR_HANDLER(gpio_irq_handler)
DEFAULT_ISR_HANDLER(WDG_A7_IRQHandler)
DEFAULT_ISR_HANDLER(WDG_IRQHandler)
DEFAULT_ISR_HANDLER(SSP0_IRQHandler)
DEFAULT_ISR_HANDLER(RTC_IRQHandler)
DEFAULT_ISR_HANDLER(CAN1_IRQHandler)
DEFAULT_ISR_HANDLER(UART0_IRQHandler)
DEFAULT_ISR_HANDLER(GPIO_S_IRQHandler)
DEFAULT_ISR_HANDLER(EFT3_IRQHandler)
DEFAULT_ISR_HANDLER(EFT4_IRQHandler)
DEFAULT_ISR_HANDLER(I2C0_IRQHandler)
DEFAULT_ISR_HANDLER(EXT0_IRQHandler)
DEFAULT_ISR_HANDLER(EXT1_IRQHandler)
DEFAULT_ISR_HANDLER(EXT2_IRQHandler)
DEFAULT_ISR_HANDLER(EXT3_IRQHandler)
DEFAULT_ISR_HANDLER(EXT4_IRQHandler)
DEFAULT_ISR_HANDLER(EXT5_IRQHandler)
DEFAULT_ISR_HANDLER(EXT6_IRQHandler)
DEFAULT_ISR_HANDLER(EXT7_IRQHandler)
DEFAULT_ISR_HANDLER(EXT8_IRQHandler)
DEFAULT_ISR_HANDLER(EXT9_IRQHandler)
DEFAULT_ISR_HANDLER(EXT10_IRQHandler)
DEFAULT_ISR_HANDLER(EXT11_IRQHandler)
DEFAULT_ISR_HANDLER(EXT12_IRQHandler)
DEFAULT_ISR_HANDLER(EXT13_IRQHandler)
DEFAULT_ISR_HANDLER(EXT14_IRQHandler)
DEFAULT_ISR_HANDLER(EXT15_IRQHandler)
DEFAULT_ISR_HANDLER(M3CTI_IRQHandler)
DEFAULT_ISR_HANDLER(C3_HSM_IRQHandler)
DEFAULT_ISR_HANDLER(GFX_IRQHandler)
DEFAULT_ISR_HANDLER(VDO_IRQHandler)
DEFAULT_ISR_HANDLER(A7NPMU_IRQHandler)
DEFAULT_ISR_HANDLER(A7NCTI_IRQHandler)
DEFAULT_ISR_HANDLER(IRQ38_IRQHandler) /* not mapped ? */
DEFAULT_ISR_HANDLER(A7SDERR_IRQHandler)
DEFAULT_ISR_HANDLER(DDRCTRL_IRQHandler)
DEFAULT_ISR_HANDLER(A7COMMTX_IRQHandler)
DEFAULT_ISR_HANDLER(A7COMMRX_IRQHandler)
DEFAULT_ISR_HANDLER(CAN2COMBINED_IRQHandler)
DEFAULT_ISR_HANDLER(SDMMC2COMBINED_IRQHandler)
DEFAULT_ISR_HANDLER(FSRCINTREQ_IRQHandler)
DEFAULT_ISR_HANDLER(ETHERNET_IRQHandler)
DEFAULT_ISR_HANDLER(A7NAXIERR_IRQHandler)

/* Stack top and vectors handlers table */
void * const _vectors[] __attribute__((section(".vectors"))) = {
	&__stack_end__,
	reset_handler,
	NMIException,        /* NMIException */
	HFHandler,           /* HFHandler */
	MemManageEception,   /* MemManageEception */
	BusFaultException,   /* BusFaultException */
	UsageFaultException, /* UsageFaultException */
	0, /* Reserved */
	0, /* Reserved */
	0, /* Reserved */
	0, /* Reserved */
	SVCHandler,
	DebugMonitor,
	0, /* Reserved */
	PendSVCHandler,
	SysTickHandler,

	/* Fixed M3 IRQs start here from IRQ0.
	 *
	 * As stated in NVIC chapter of Corte-M3 TRM:
	 * VECTACTIVE bitfield of Interrupt Control State Register
	 * contains the interrupt number of the currently running ISR,
	 * including NMI and Hard Fault. A shared handler can use VECTACTIVE
	 * to determine which interrupt invoked it. You can subtract 16 from
	 * the VECTACTIVE field to index into the Interrupt Clear/Set Enable,
	 * Interrupt Clear Pending/SetPending and Interrupt Priority Registers.
	 * INTISR[0] has vector number 16.
	 *
	 * In other words, reading 16 in VECTACTIVE would mean that INTISR[0]
	 * is pending, which is bound to "PMU_IRQHandler" in our case.
	 */
	PMU_IRQHandler,
	SRC_IRQHandler,
	mtu_irq_handler,
	mailbox_irq_handler,
	CAN0_IRQHandler,
	gpio_irq_handler,
	WDG_A7_IRQHandler,
	WDG_IRQHandler,
	SSP0_IRQHandler,
	RTC_IRQHandler,
	CAN1_IRQHandler,
	UART0_IRQHandler,
	GPIO_S_IRQHandler,
	EFT3_IRQHandler,
	EFT4_IRQHandler,
	I2C0_IRQHandler,

	/* Delaration of the sixteen external interrupt service routines
	 * that can be dynamically bound to the sixteen M3 external interrupt lines,
	 * by programming the M3_IRQITF.
	 */
	EXT0_IRQHandler,
	EXT1_IRQHandler,
	EXT2_IRQHandler,
	EXT3_IRQHandler,
	EXT4_IRQHandler,
	EXT5_IRQHandler,
	EXT6_IRQHandler,
	EXT7_IRQHandler,
	EXT8_IRQHandler,
	EXT9_IRQHandler,
	EXT10_IRQHandler,
	EXT11_IRQHandler,
	EXT12_IRQHandler,
	EXT13_IRQHandler,
	EXT14_IRQHandler,
	EXT15_IRQHandler,

	/* Fixed M3 IRQs continue here from IRQ32. */
	M3CTI_IRQHandler,
	C3_HSM_IRQHandler,
	GFX_IRQHandler,
	VDO_IRQHandler,
	A7NPMU_IRQHandler,
	A7NCTI_IRQHandler,
	IRQ38_IRQHandler, /* not mapped ? */
	A7SDERR_IRQHandler,
	DDRCTRL_IRQHandler,
	A7COMMTX_IRQHandler,
	A7COMMRX_IRQHandler,
	CAN2COMBINED_IRQHandler,
	SDMMC2COMBINED_IRQHandler,
	FSRCINTREQ_IRQHandler,
	ETHERNET_IRQHandler,
	A7NAXIERR_IRQHandler,
};

/* Linker sections to move in final position */
const struct section_to_move_t sections[]
		__attribute__((section(".roinitdata"))) = {
	{ &__data_load_start__, &__data_start__, &__data_end__ },
	{ &__rodata_load_start__, &__rodata_start__, &__rodata_end__ },
	{ &__rodataexidx_load_start__, &__rodataexidx_start__, &__rodataexidx_end__ },
	{ &__text_load_start__, &__text_start__, &__text_end__ },
};

static uint16_t mpu_region_no;

/********************** Setup MPU ***************************/
#define MPU_BASE               0xE000ED90
/* For full description see:
 * http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0552a/Cihjddef.html */
typedef volatile struct {
	union {
		struct {
			uint32_t separate:1;
			uint32_t reserved0:7;
			uint32_t dregion:8;
			uint32_t iregion:8;
			uint32_t reserved1:8;
		} bit;
		uint32_t reg;
	} type;		/* offset 0x0 - 32 bits */
	union {
		struct {
			uint32_t enable:1;
			uint32_t privdefena:1;
			uint32_t hfnmiena:1;
			uint32_t reserved:29;
		} bit;
		uint32_t reg;
	} cr;		/* offset 0x4 - 32 bits */
	uint32_t rnr; /* offset 0x8 - 32 bits */
	union {
		struct {
			uint32_t rgn_number:4;
			uint32_t valid:1;
			uint32_t reserved:1;
			uint32_t addr:26;
		} bit;
		uint32_t reg;
	} rbar;	/* offset 0xC - 32 bits */
	union {
		struct {
			uint32_t enable:1;
			uint32_t rgn_size:5;
			uint32_t reserved0:2;
			uint32_t srd:8;
			uint32_t b:1;
			uint32_t c:1;
			uint32_t s:1;
			uint32_t tex:3;
			uint32_t reserved1:2;
			uint32_t ap:3;
			uint32_t reserved2:1;
			uint32_t xn:1;
			uint32_t reserved3:3;
		} bit;
		uint32_t reg;
	} rasr;	/* offset 0x10 - 32 bits */
} t_mpu_regs;
#define mpu_regs ((t_mpu_regs *)MPU_BASE)

/* Constants required to access and manipulate the MPU. */
#define MPU_ENABLE              0x01
#define MPU_RGN_VALID           0x10

/* MPU Region Attribute and Size Register fields */
#define MPU_RGN_ENABLE          0
#define MPU_RGN_SIZE            1

#define MPU_RGN_RW              (0x03 << 24)
#define MPU_RGN_PRIVILEGED_RO   (0x05 << 24)
#define MPU_RGN_RO              (0x06 << 24)
#define MPU_RGN_PRIVILEGED_RW   (0x01 << 24)
#define MPU_RGN_NOACCESS_ALL    (0x00 << 24)

#define MPU_RGN_XN              (0x01 << 28)

/* C, B and TEX[2:0] bits only have semantic meanings when grouped */
#define MPU_RGN_STRONGLY_ORDERED        (0x00 << 16)
#define MPU_RGN_CACHEABLE_BUFFERABLE    (0x03 << 16)
#define MPU_RGN_NOCACHEABLE_BUFFERABLE  (0x01 << 16)
#define MPU_RGN_NORMAL                  MPU_RGN_CACHEABLE_BUFFERABLE

static uint32_t __init
get_mpu_region_size_order(uint32_t size_in_bytes)
{
	uint32_t region_size, ret;

	if (size_in_bytes == 0) /* 0 => 4GB */
		return 32;

	/* 32 is the smallest region size, 32 is the largest valid value for ret */
	for (ret = 5, region_size = 32; ret <= 32; (region_size <<= 1)) {
		if (size_in_bytes <= region_size)
			break;
		else
			ret++;
	}
	return ret;
}

static void __init
mpu_setup_region(unsigned int number, uint32_t base_addr,
			uint32_t size, uint32_t properties)
{
	uint32_t size_data;
	int size_order = get_mpu_region_size_order(size);

	if (number >= mpu_regs->type.bit.dregion)
		wait_forever; /* Panic so loop forever */

	/* ARM MPU Datasheet says:
	 * The Base address of a region must always be aligned to the region size.
	 * This means, for example, that a 32kB region must have a base address
	 * aligned to a 32kB boundary.
	 */
	if ((base_addr & ((1 << size_order) - 1)) != 0)
		base_addr &= ~((1 << size_order) - 1); /* Align Region base */

	/* Writing N to bits 5:1 (RSR_SZ)  specifies region size 2^N+1 */
	size_data = ((size_order - 1) << MPU_RGN_SIZE) | 1 << MPU_RGN_ENABLE;

	mpu_regs->rbar.reg = base_addr | MPU_RGN_VALID | number;
	mpu_regs->rasr.reg = properties | size_data;
}

uint16_t __init mpu_setup(void)
{
	mpu_region_no = 0;

	/* M3 ESRAM + M3 ROM - 512 KB */
	mpu_setup_region(mpu_region_no++, M3_SRAM_BASE, 512 * 1024,
			 MPU_RGN_NORMAL | MPU_RGN_RW);

	/* Peripherals - 1GB */
	mpu_setup_region(mpu_region_no++, M3_PERIPH_BASE, 1024 * 1024 * 1024,
			 MPU_RGN_XN | MPU_RGN_STRONGLY_ORDERED | MPU_RGN_RW);

#if SOC_ID == SOCID_STA1385
	/* Flash cache slave region - 256MB */
	mpu_setup_region(mpu_region_no++, FC_SLAVE_BASE, 256 * 1024 * 1024,
			 MPU_RGN_NORMAL | MPU_RGN_RO);
#endif

#if defined(NAND)
	/* NAND - 256 KB is enough */
	mpu_setup_region(mpu_region_no++, NAND_CS0_BASE, 256 * 1024,
			 MPU_RGN_XN | MPU_RGN_STRONGLY_ORDERED | MPU_RGN_RW);
#else
	/* Always map SQI (256 MB) by default */
	mpu_setup_region(mpu_region_no++, SQI0_NOR_BB_BASE, 256 * 1024 * 1024,
			 MPU_RGN_STRONGLY_ORDERED | MPU_RGN_RO);
#endif

	/* AP ESRAM - 512 KB */
	mpu_setup_region(mpu_region_no++, (uint32_t)&__ESRAM_AP_start,
			 (uint8_t *)&__ESRAM_AP_end - (uint8_t *)&__ESRAM_AP_start,
			 MPU_RGN_NORMAL | MPU_RGN_RW);

	/* AP DDRAM - 512 MB first half of DDR */
	mpu_setup_region(mpu_region_no++, (uint32_t)&__DRAM_start,
			 512 * 1024 * 1024, MPU_RGN_NORMAL | MPU_RGN_RW);

#if DDRAM_SIZE > (512 * 1024 * 1024)
	/* AP DDRAM - 512 MB second half of DDR */
	mpu_setup_region(mpu_region_no++, 0xC0000000, 512 * 1024 * 1024,
			 MPU_RGN_NORMAL | MPU_RGN_RW);
#endif

	/* Backup RAM mapping space - 512 Bytes */
	mpu_setup_region(mpu_region_no++, BACKUP_RAM_BASE, BACKUP_RAM_SIZE,
			 MPU_RGN_XN | MPU_RGN_NORMAL | MPU_RGN_RW);

	/* Enable MPU */
	mpu_regs->cr.reg = MPU_ENABLE;

	return mpu_region_no;
}

void __init cstartup(void)
{
	/* Copy with 32bits aligned pointers */
	uint32_t *dst;
	uint32_t i;
	uint16_t num_regions;

	num_regions = mpu_setup();

	/* Copy sections if necessary to final position in RAM */
	for (i = 0; i < sizeof(sections) / sizeof(struct section_to_move_t); i++) {
		uint32_t *src, *end;

		src = sections[i].load_start;
		dst = sections[i].start;
		if (dst != src) {
			end = sections[i].end;
			while (dst < end)
				*dst++ = *src++;
		}
	}

	/* Clear the bss section */
	dst = (uint32_t *)&__bss_start__;
	while (dst < (uint32_t *)&__bss_end__)
		*dst++ = 0;

	/* Restore mpu_region_no: cleaned by BSS init */
	mpu_region_no = num_regions;

	main();
}

