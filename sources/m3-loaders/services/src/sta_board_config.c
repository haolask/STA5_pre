/**
 * @file sta_board_config.c
 * @brief This file provides board configuration functions.
 *
 * Copyright (C) ST-Microelectronics SA 2017
 * @author: APG-MID team
 */

#include "sta_common.h"
#include "sta_i2c_service.h"
#include "sta_board_config.h"
#include "sta_mtu.h"
#include "trace.h"
#include "sta_pinmux.h"


/**
 * apply_board_config - execute Board specific configuration
 */
void brd_cfg(struct sta *context)
{
	if (context->features.hdmi == true)
		configure_hdmi();
	vTaskDelete(NULL);
}


/**
 * apply_board_config - Change i2c bank address for adv7513
 */
int configure_hdmi(void)
{
	int status = HDMI_OK;
	uint8_t write_buf;
	uint8_t rev;
	struct i2c_com_handler_s *i2c_board_handler;

	portTickType timeout = 1000;

	TRACE_INFO("ADV7513: Changing i2c bank address\n");
	/* 200ms are needed for hdmi chip to be ready after powering up */
	mdelay(200);

	pinmux_request("i2c0_mux");

	if (i2c_service_init(I2C_ADV7513_CLOCK)) {
		TRACE_INFO("ADV7513: failed to init I2C\n");
		status = HDMI_ERROR;
		goto error;
	}
	if (i2c_open_port(I2C_ADV7513_PORT, 0, 0, I2C_BUSCTRLMODE_MASTER)) {
		TRACE_INFO("ADV7513: failed to open I2C\n");
		status = HDMI_ERROR;
		goto error;
	}
	i2c_set_port_mode(I2C_ADV7513_PORT, I2C_BUSCTRLMODE_MASTER);
	i2c_board_handler = i2c_create_com(I2C_ADV7513_PORT, I2C_FAST_MODE,
					   I2C_ADV7513_ADDR >> 1);
	if (i2c_board_handler == NULL) {
		TRACE_INFO("ERROR: I2C handler is null\n");
		status = HDMI_ERROR;
		goto error;
	}

	if (i2c_read(i2c_board_handler, I2C_ADV7513_REV_BANK, 1,
		     &rev, 1, &timeout)) {
		TRACE_INFO("ADV7513: failed to read rev id\n");
		status = HDMI_ERROR;
		goto error;
	}

	write_buf = I2C_ADV7513_ENABLE_EXTRA_CFG;
	if (i2c_write(i2c_board_handler, I2C_ADV7513_HPD_CTRL_BANK, 1,
		      &write_buf, 1, 1, &timeout)) {
		TRACE_INFO("ADV7513: failed to write at 0x%x\n",
			   I2C_ADV7513_HPD_CTRL_BANK);
		status = HDMI_ERROR;
		goto error;
	}
	write_buf = I2C_ADV7513_NEW_PCKT_MEM_I2C_ADDR;
	if (i2c_write(i2c_board_handler, I2C_ADV7513_PCKT_MEM_I2C_MAP_BANK, 1,
		      &write_buf, 1, 1, &timeout)) {
		TRACE_INFO("ADV7513: failed to write at 0x%x\n",
			   I2C_ADV7513_PCKT_MEM_I2C_MAP_BANK);
		status = HDMI_ERROR;
		goto error;
	}

	TRACE_INFO("ADV7513: Rev 0x%x, i2c bank address change done.\n", rev);

error:
	if (i2c_reset_port(I2C_ADV7513_PORT)) {
		TRACE_INFO("ADV7513: failed to release I2C\n");
		status = HDMI_ERROR;
	}

	return status;
}
