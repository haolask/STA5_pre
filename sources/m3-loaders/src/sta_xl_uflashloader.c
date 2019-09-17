/**
 * @file sta_xl_uflasherloader.c
 * @brief M3 Xloader U-boot FlashLoader entry point file
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#include <errno.h>
#include <string.h>

#include "lzg.h" /* For uncompressing AP loader */
#include "utils.h"

#include "sta_common.h"
#include "trace.h"
#include "sta_uart.h"
#include "sta_mtu.h"
#include "sta_a7.h"
#include "sta_ddr.h"
#include "sta_pinmux.h"
#include "sdmmc.h"
#include "sta_platform.h"
#include "sta_usb.h"

/**
 * Fixed address data
 * Updated by flashloader at flashing time:
 *	- USB DFU Download mode by default else serial
 *	- Default MMC boot device number SDMMC2
 */
volatile const struct {
	char	dl_mode;
	uint8_t mmc_bootdev_num;
} fixed_data __attribute__ ((section("FIXED_ADDR_DATA"))) = { 'U', 1 };

/* M3 context */
static struct sta context;

/* Values given by the linker */
extern uint32_t __compressed_uboot_start__; /* defined in Linker file */

/* System configuration structure */
extern struct system_config SystemConfig;

/**
 * @brief	Update SoC and board IDs from OTP_VR4 register or from
 * an autodetection mecanism
 */
static int platform_init(struct sta *context)
{
	int err;

	if (!context)
		return -EINVAL;

	err = m3_get_board_id();
	if (err)
		goto end;

	/* SoC specifities */
	switch (get_soc_id()) {
	case SOCID_STA1295:
	case SOCID_STA1195:
	case SOCID_STA1275:
		context->features.uart_console_no = UART_P3;
		context->trace_port = UART3;
		break;
	case SOCID_STA1385:
		context->features.uart_console_no = UART_P2;
		context->trace_port = UART2;
		break;
	default:
		break;
	}

	/* Reset ETH0 PHY for next boot stages */
	platform_eth0_reset_phy();

	m3_init_trace(context);

	platform_welcome_message(context);
end:
	return err;
}

/**
 * @brief	initializes platform HW interfaces
 * @param	context: main application context
 * @return	0 if no error, not 0 otherwise
 */
static int xl_uflasher_hardware_setup(struct sta *context)
{
	int err;

	/**
	 * Identify the SoC at early boot stage to discriminate
	 * specific settings
	 */
	m3_get_soc_id();

	if (srcm3_wdg_reset(src_m3_regs)) {
		sta_ddr_axi_reset();
		sta_ddr_core_reset();
		srcm3_reset(src_m3_regs);
	}

	err = platform_early_init(context);
	if (err)
		goto end;

	err = m3_lowlevel_core_setup();
	if (err)
		goto end;

	err = m3_core_setup(context);
	if (err)
		goto end;

	err = platform_init(context);
	if (err)
		goto end;

	/**
	 * Set mmc boot dev number from mmc_bootdev_num variable content
	 * set by host flashLoader
	 */
	set_mmc_boot_dev(fixed_data.mmc_bootdev_num);
	/* Workaround for A5 cut3: Clear SDI DATA CONTROL register */
	switch (fixed_data.mmc_bootdev_num) {
	case SDMMC1_PORT:
		((t_mmc *)SDMMC1_BASE)->mmc_data_ctrl = 0;
		break;
	case SDMMC2_PORT:
		((t_mmc *)SDMMC2_BASE)->mmc_data_ctrl = 0;
		break;
	}

	/* Always divide A7 timers clock by 8 */
	a7_timers_clk_set();

	err = sta_ddr_init(DDR_BOOT_REASON_COLD_BOOT);
	if (err)
		goto end;

	platform_recopy_bootinfo_in_ddr(false);

	err = usb_phy_init();
end:
	if (err)
		TRACE_ERR("%s: failed to init hardware, err:%d\n",
			  __func__, err);
	return err;
}

/**
 * @brief	prints UART error messages according to status
 * @param	status: error code
 */
static void xlu_rx_uart_error(int status)
{
	switch (status) {
	case 0:
		/* No error */
		break;

	default:
		TRACE_ERR("%s: UART RX ERROR %d\n", __func__, status);
		break;
	}
}

/**
 * @brief	checks the received address range and applies
 * offset if in the ESRAM range
 * @param	addr: address to check
 * @return	corrected address
 */
static uint32_t xlu_check_address(uint32_t addr)
{
	return addr >= (0 + ESRAM_A7_SIZE) ? addr : addr + ESRAM_A7_BASE;
}

/**
 * @brief	sends one character for ack
 * @param	port: the uart handle
 * @param	ack: and the character to send
 */
static void xlu_uart_send_ack(t_uart * port, char ack)
{
	TRACE_VERBOSE(">%c\n", ack);
	uart_send_data(port, &ack, 1);
}

/**
 * @brief	loads Uboot flasher from UART
 * @param	port: UART port to use
 * @return	0 if no error, not 0 otherwise
 */
static int xlu_load_uboot_from_uart(uint32_t port)
{
	int ret;
	char rx_char;
	uint32_t ram_addr, baudrate, dlength;
	uint8_t *start_addr = NULL;
	uint8_t dsize;
	bool loading = true;
	t_uart *p_uart;

	/**
	 * Init UART with default configuration
	 */
	p_uart = uart_init(port, BR115200BAUD, NOPARITY_BIT,
			   ONE_STOPBIT, DATABITS_8);
	if (!p_uart)
		return false;

	/**
	 * Request connexion to remote PC
	 */
	TRACE_VERBOSE("Connexion request\n");
	xlu_uart_send_ack(p_uart, 0x81);

	/* Wait 20s for connexion confirmation: 'O'*/
	ret = uart_get_data_until_Timer(p_uart, &rx_char, 1, 20000000);
	if (ret == 0 && rx_char == 'O') {
		TRACE_VERBOSE("Connexion confirmed\n");
	} else {
		TRACE_ERR("%s: Error connexion\n", __func__);
		loading = false;
	}

	/**
	 * Connexion established
	 * Wait for ufalsher command sent over UART
	 */
	/* CMD waiting loop */
	while (loading == true) {
		/** Wait for command character **/
		TRACE_VERBOSE("?\n");
		ret = uart_get_data(p_uart, &rx_char, 1);
		if (ret != 0) {
			/* End: error handling */
			loading = false;
			break;
		}
		TRACE_VERBOSE("< %c:\n", rx_char);

		switch (rx_char) {
			/** UART speed change */
		case 'R':
			/**
			 * Wait (1s) for UART speed:
			 * baudrate (4 bytes in little endian)
			 */
			ret = uart_get_data_until_Timer(p_uart,
					(char *)&baudrate, 4, 1000000);
			if (ret == 0) {
				TRACE_VERBOSE("New UART speed: 0x%x\n",
					      baudrate);
				uart_init(port, baudrate, NOPARITY_BIT,
					  ONE_STOPBIT, DATABITS_8);
				mdelay(10); /* 10ms */
				xlu_uart_send_ack(p_uart, 'O');
			} else {
				TRACE_VERBOSE("%s: UART speed change error\n",
					      __func__);
				loading = false;
			}
			break;

			/** Set address for next load or jump */
		case 'S':
			/**
			 * Wait (1s) for RAM address:
			 * @RAM (4 bytes in little endian)
			 */
			ret = uart_get_data_until_Timer(p_uart,
							(char *)&ram_addr,
							4, 1000000);
			if (ret == 0) {
				start_addr = (uint8_t *)xlu_check_address(ram_addr);
				TRACE_VERBOSE("0x%x\n", start_addr);
				xlu_uart_send_ack(p_uart, 'O');
			} else {
				TRACE_ERR("%s: Set address error\n", __func__);
				loading = false;
			}
			break;

			/** Load uflasher binary file */
		case 'L':
			/* Wait for File binary */
			/* 1st: wait (1s) for the size */
			ret = uart_get_data_until_Timer(p_uart, (char *)&dsize,
							1, 1000000);
			if (ret == 0) {
				dlength = (dsize + 1) << 2;
				TRACE_VERBOSE("@:%x %d\n", start_addr, dlength);

				/* 2nd: wait (2s) for the data */
				ret = uart_get_data_until_Timer(p_uart,
						(char *)start_addr,
						dlength, 2000000);
				if (ret == 0) {
					start_addr += dlength;
					xlu_uart_send_ack(p_uart, 'O');
				}
			}
			if (ret != 0) {
				TRACE_ERR("%s:Load file error\n", __func__);
				loading = false;
			}
			break;

			/** Jump AP start address and launch it */
		case 'J':
			TRACE_VERBOSE("Start AP @%x\n", start_addr);
			/* Remove AP core from reset */
			a7_start((uint32_t)start_addr);

			/* uflasher loading complete */
			loading = false;
			break;

		default:
			TRACE_ERR("UART unknown command\n");
			loading = false;
			break;
		}
	}

	if (ret != 0)
	{
		/* Rx UART error handling */
		xlu_rx_uart_error(ret);
		return false;
	} else {
		return true;
	}
}

/**
 * @brief	main entry point function
 * @param	none
 * @return	never
 */
int main(void)
{
	context.bin = BIN_NAME_XLU;

	/* Setup all required HW interfaces */
	xl_uflasher_hardware_setup(&context);

	if (fixed_data.dl_mode == 'U') {
		uint32_t shadow_address = ESRAM_A7_BASE;
		uint8_t *enc_buf = (uint8_t *)&__compressed_uboot_start__;
		lzg_uint32_t enc_size;

		if (enc_buf[0] != 'L' || enc_buf[1] != 'Z') {
			/* This is the true shadowing address */
			shadow_address = *(uint32_t *)enc_buf;
			enc_buf += 4;
		}
		uint32_t dec_size = LZG_DecodedSize(enc_buf, &enc_size);

		/* uncompress AP code in DRAM */
		if (LZG_Decode(enc_buf, enc_size, (uint8_t *)shadow_address,
			       dec_size)) {
			/* Tell U-boot to boot in fastboot mode */
			set_uboot_fastboot_mode();

			/* Remove AP core from reset */
			a7_start(shadow_address);
		} else {
			TRACE_ERR("Can't uncompress AP loader\n");
			return 1;
		}
	} else {
		/* Load U-boot flashloaer through UART */
		xlu_load_uboot_from_uart(context.features.uart_console_no);
	}

	return 0;
}

