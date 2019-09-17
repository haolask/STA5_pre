/**
 * @file sdmmc.c
 * @brief This file provides all core Layer for SDMMC
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "sta_type.h"
#include "sta_pinmux.h"
#include "sta_hsem.h"
#include "sta_mtu.h"
#include "utils.h"
#include "sta_src_a7.h"
#include "hmacsha256.h"

#include "sdmmc.h"
#include "sta_sdhc.h"
#include "sta_mmci.h"

/* Uncomment one MMC port for basic tests at the end of this file */
//#define SDMMC_TESTS

#if defined SDMMC_TESTS
#undef TRACE_LEVEL
#define TRACE_LEVEL TRACE_LEVEL_INFO
#include "sta_mtu.h"
#define MMC_MAX_INSTANCES	2 /* 2 instances for test only */
#else
/* For now manage only one mmc instance */
#define MMC_MAX_INSTANCES	1
#endif /* SDMMC_TESTS */

#include "trace.h"

#define MMC_MAXVOLTTRIAL	0x00FFFFFF /* (0x64)100 times */

#define LOW_SPEED	0
#define HIGH_SPEED	1

#define MMC_LOW_VOLTAGE_WINDOW		0x00000080
#define MMC_HIGH_VOLTAGE_WINDOW		0x00FF8000
#define MMC_HC_WINDOW			0x40000000
#define MMC_SD_CMD8_HV_WINDOW		0x00000100
#define MMC_SD_CMD8_LV_WINDOW		0x00000200
#define MMC_SD_CMD8_CHECK_PATTERN	0x000000AA

#define MMC_STATUS_READY_FOR_DATA	BIT(8)
#define MMC_DEV_STATE_SHIFT		9
#define MMC_DEV_STATE_MASK		(0xf << MMC_DEV_STATE_SHIFT)
#define MMC_DEV_STATE_IDLE		(0 << MMC_DEV_STATE_SHIFT)
#define MMC_DEV_STATE_READY		(1 << MMC_DEV_STATE_SHIFT)
#define MMC_DEV_STATE_IDENT		(2 << MMC_DEV_STATE_SHIFT)
#define MMC_DEV_STATE_STBY		(3 << MMC_DEV_STATE_SHIFT)
#define MMC_DEV_STATE_TRAN		(4 << MMC_DEV_STATE_SHIFT)
#define MMC_DEV_STATE_DATA		(5 << MMC_DEV_STATE_SHIFT)
#define MMC_DEV_STATE_RCV		(6 << MMC_DEV_STATE_SHIFT)
#define MMC_DEV_STATE_PRG		(7 << MMC_DEV_STATE_SHIFT)
#define MMC_DEV_STATE_DIS		(8 << MMC_DEV_STATE_SHIFT)

#define MMC_EXTCSD_SET_CMD		(0 << 24)
#define MMC_EXTCSD_SET_BITS		(1 << 24)
#define MMC_EXTCSD_CLR_BITS		(2 << 24)
#define MMC_EXTCSD_WRITE_BYTES		(3 << 24)
#define MMC_EXTCSD_CMD(x)		(((x) & 0xff) << 16)
#define MMC_EXTCSD_VALUE(x)		(((x) & 0xff) << 8)

#define MMC_CMD_EXTCSD_BOOT_BUS_WIDTH	177
#define MMC_CMD_EXTCSD_PARTITION_CONFIG	179
#define MMC_CMD_EXTCSD_BUS_WIDTH	183
#define MMC_CMD_EXTCSD_HS_TIMING	185
#define MMC_CMD_EXTCSD_RST_N_FUNC	162

#define MMC_VERSION(ctxt)	(((ctxt)->csd[0] >> 26) & 0xf)

/* OCR bit fields definition */
#define OCR_POWERUP		BIT(31)
#define OCR_BYTE_MODE		(0 << 29)
#define OCR_SECTOR_MODE		(2 << 29)
#define OCR_ACCESS_MODE_MASK	(3 << 29)

/* Sizes of RPMB data frame */
#define RPMB_STUFF_LEN		196
#define RPMB_MAC_KEY_LEN	32
#define RPMB_DATA_LEN		256
#define RPMB_NONCE_LEN		16

/* RPMB frame structure */
struct t_rpmb {
	uint8_t stuff[RPMB_STUFF_LEN];
	uint8_t mac_key[RPMB_MAC_KEY_LEN];
	uint8_t data[RPMB_DATA_LEN];
	uint8_t nonce[RPMB_NONCE_LEN];
	uint32_t write_counter;
	uint16_t address;
	uint16_t block_count;
	uint16_t result;
	uint16_t request;
};

#define RPMB_AUTH_KEY_PGM_REQ	0x0001
#define RPMB_WR_CNTR_VAL_REQ	0x0002
#define RPMB_AUTH_DAT_WR_REQ	0x0003
#define RPMB_AUTH_DAT_RD_REQ	0x0004
#define RPMB_RESULT_RD_REQ	0x0005

#define RPMB_AUTH_KEY_PGM_RESP	0x0100
#define RPMB_WR_CNTR_VAL_RESP	0x0200
#define RPMB_AUTH_DAT_WR_RESP	0x0300
#define RPMB_AUTH_DAT_RD_RESP	0x0400

#define RPMB_WRCNT_EXPIRED_ERR	0x80
#define RPMB_GENERAL_FAILURE	0x01
#define RPMB_AUTH_FAILURE	0x02
#define RPMB_COUNTER_FAILURE	0x03
#define RPMB_ADDR_FAILURE	0x04
#define RPMB_WRITE_FAILURE	0x05
#define RPMB_READ_FAILURE	0x06
#define RPMB_KEY_NOT_PROGRAMMED	0x07

static struct t_mmc_ctx context[MMC_MAX_INSTANCES];
static struct hsem_lock *hsem;
static const uint32_t mmc_bases[] = { SDMMC0_BASE, SDMMC1_BASE, SDMMC2_BASE };

static int sdmmc_init_context(struct t_mmc_ctx *mmc, enum t_mmc_port port,
			      const char *pinmux, uint32_t bus_width, int rst_pin)
{
	struct gpio_config pin;

	TRACE_NOTICE("Init MMC%d, %s busw:%d\n", port, pinmux,
		     (bus_width == 2) ? 8 : ((bus_width == 1) ? 4 : 1));

	mmc->port = port;
	mmc->bus_width = bus_width;
	mmc->blksize = MMC_BLOCK_SIZE; /* FIXME must be dynamic */
	mmc->rst_pin = rst_pin;

	/* Set given GPIO configuration */
	pinmux_request(pinmux);

	if (mmc->rst_pin >= 0) {
		pin.direction   = GPIO_DIR_OUTPUT;
		pin.mode        = GPIO_MODE_SOFTWARE;
		pin.level       = GPIO_LEVEL_LEAVE_UNCHANGED;
		pin.trig        = GPIO_TRIG_LEAVE_UNCHANGED;
		gpio_set_pin_config(mmc->rst_pin, &pin);
		gpio_set_gpio_pin(mmc->rst_pin);
	}

	switch (port) {
	case SDMMC0_PORT:
		sdhc_init(mmc, mmc_bases[port]);
		break;
	case SDMMC1_PORT:
	case SDMMC2_PORT:
		mmci_init(mmc, mmc_bases[port]);
		break;
	default:
		return MMC_GENERIC_ERROR;
	}

	mmc->cardtype = MMC_MULTIMEDIA_HC_CARD;

	if (!hsem) {
		struct hsem_lock_req lock;

		lock.lock_cb = NULL;
		lock.id = HSEM_MMC_ID;

		hsem = hsem_lock_request(&lock);
		if (!hsem) {
			TRACE_ERR("%s: hsem err\n");
			return MMC_GENERIC_ERROR;
		}
	}
	return 0;
}

static inline int sdmmc_send_cmd(struct t_mmc_ctx *mmc, uint32_t cmd,
				 uint32_t resp_type, uint32_t arg)
{
	return mmc->ops->send_cmd(mmc, cmd, arg, resp_type, 0);
}

static int sdmmc_get_state(struct t_mmc_ctx *mmc, uint32_t *state)
{
	int error;

	do {
		error = sdmmc_send_cmd(mmc, SDMMC_CMD13, MMC_RESPONSE_R1,
				       mmc->rca << 16);
		if (!error) {
			*state = mmc->resp[0];
			if (*state & MMC_ERRORBITS_MSK) {
				TRACE_ERR("Status error: %x\n", mmc->resp[0]);
				return MMC_STATUS_ERROR;
			}
		}
	} while (!error && ((*state & MMC_STATUS_READY_FOR_DATA) == 0));

	*state &= MMC_DEV_STATE_MASK;

	return error;
}

static int sdmmc_wait_state(struct t_mmc_ctx *mmc, uint32_t state)
{
	uint32_t curr_state;
	int error = 0;

	do {
		error = sdmmc_get_state(mmc, &curr_state);
		if (error)
			return error;
	} while (curr_state != state);

	return error;
}

static int sdmmc_mmc_send_opcond(struct t_mmc_ctx *mmc, uint32_t voltage_mask,
				 uint32_t *response)
{
	int error = 0;
	int count = 0;

	/* Assume it is a MMC-HC Card. Send CMD1 to verify that */
	do {
		error = sdmmc_send_cmd(mmc, SDMMC_CMD1, MMC_RESPONSE_R3,
				       (voltage_mask | MMC_HC_WINDOW));

	} while (!(mmc->resp[0] & OCR_POWERUP) && /* Valid voltage ? */
		 (count++ < MMC_MAXVOLTTRIAL) && !error);

	*response = mmc->resp[0];

	return error;
}

static int sdmmc_sd_send_ifcond(struct t_mmc_ctx *mmc, uint32_t voltage_mask,
				uint32_t *response)
{
	int error = 0;

	error = sdmmc_send_cmd(mmc, SDMMC_CMD8, MMC_RESPONSE_R7, voltage_mask);

	if (error)
		return error;

	/* Manage R7 type response: check if pattern does not match */
	if ((uint8_t)mmc->resp[0] != 0xaa)
		return MMC_GENERIC_ERROR;

	*response = mmc->resp[0];

	return 0;
}

static int sdmmc_sd_send_opcond(struct t_mmc_ctx *mmc, uint32_t voltage_mask,
				uint32_t *response)
{
	int error = 0, retry = 0;

	do {
		/* Send CMD55 APP_CMD with RCA as 0 */
		error = sdmmc_send_cmd(mmc, SDMMC_CMD55, MMC_RESPONSE_R1, 0);

		/* If non error send ACMD41 */
		if (!error && (mmc->resp[0] & 0x20))
			error = sdmmc_send_cmd(mmc, SDMMC_CMD41,
					       MMC_RESPONSE_R3, voltage_mask);
	} while (!(mmc->resp[0] & OCR_POWERUP) && /* Valid voltage ? */
		 (retry++ < MMC_MAXVOLTTRIAL) && !error);

	if (!error) {
		*response = mmc->resp[0];
		return 0;
	} else {
		return -1;
	}
}

static int sdmmc_select_card(struct t_mmc_ctx *mmc)
{
	int error;

	error = sdmmc_send_cmd(mmc, SDMMC_CMD7, MMC_RESPONSE_R1,
			       mmc->rca << 16);
	if (!error)
		error = sdmmc_wait_state(mmc, MMC_DEV_STATE_TRAN);

	return error;
}

static int sdmmc_get_extcsd(struct t_mmc_ctx *mmc)
{
	int error = 0;

	if (!mmc->extcsd_valid) {
		error = mmc->ops->read_extcsd(mmc, mmc->extcsd);
		mmc->extcsd_valid = true;
	}

	return error;
}

static int sdmmc_discover_card(struct t_mmc_ctx *mmc)
{
	uint32_t voltage_mask = 0;
	uint32_t retry = 0;
	uint32_t ocr = 0;
	bool card_identified = false;
	int error = 0;
	uint32_t type_memory = 0;

	while (!card_identified) {
		sdmmc_send_cmd(mmc, SDMMC_CMD0, 0, 0);

		switch (type_memory) {
		case 0:
			/* test if external memory is Multimedia card */
			voltage_mask = MMC_HIGH_VOLTAGE_WINDOW;
			mmc->cardtype = MMC_MULTIMEDIA_HC_CARD;
			if (!sdmmc_mmc_send_opcond(mmc, voltage_mask, &ocr)) {
				card_identified = true;
				/* It is a standard capacity MMC */
				if (!(ocr & MMC_HC_WINDOW))
					mmc->cardtype = MMC_MULTIMEDIA_CARD;
			} else {
				type_memory++;
			}
			break;

		case 1:
			/*
			 * test if external memory is Secure digital card
			 * high capacity high voltage
			 */
			voltage_mask = MMC_SD_CMD8_HV_WINDOW |
					MMC_SD_CMD8_CHECK_PATTERN;
			mmc->cardtype = MMC_SECURE_DIGITAL_HC_CARD;
			if (!sdmmc_sd_send_ifcond(mmc, voltage_mask, &ocr))
				card_identified = true;
			else
				if (++retry > 5)
					type_memory++;
			break;

		case 2:
			/* Secure digital card low voltage ? */
			voltage_mask = MMC_SD_CMD8_LV_WINDOW |
					MMC_SD_CMD8_CHECK_PATTERN;
			if (!sdmmc_sd_send_ifcond(mmc, voltage_mask, &ocr)) {
				/* It is a Secure digital High Capacity card */
				mmc->cardtype = MMC_SECURE_DIGITAL_HC_CARD;
			} else {
				/* It is a Secure digital standard card */
				mmc->cardtype = MMC_SECURE_DIGITAL_CARD;
			}
			card_identified = true;
			break;

		default:
			break;
		}
	}

	/* For SD and SDHC send ACMD41 to voltage range validation */
	if (IS_SDCARD(mmc)) {
		/* Start operation condition validation (CMD55 + ACMD41) */
		voltage_mask = (MMC_HIGH_VOLTAGE_WINDOW | MMC_HC_WINDOW);
		error = sdmmc_sd_send_opcond(mmc, voltage_mask, &ocr);

		/* It is a SD (or SDHC) card */
		if (!error) {
			/* It is a standard capacity SD */
			if (!(ocr & MMC_HC_WINDOW))
				mmc->cardtype = MMC_SECURE_DIGITAL_CARD;
		}
	}

	/* Save OCR in context */
	mmc->ocr = ocr;

	return error;
}

static int sdmmc_set_extcsd(struct t_mmc_ctx *mmc, uint32_t ext_cmd,
			    uint32_t value)
{
	uint32_t arg, state;
	int error;

	arg = MMC_EXTCSD_WRITE_BYTES | MMC_EXTCSD_CMD(ext_cmd) |
		MMC_EXTCSD_VALUE(value);
	error = sdmmc_send_cmd(mmc, SDMMC_CMD6, MMC_RESPONSE_R1, arg);

	/* Wait to exit program state */
	do {
		error = sdmmc_get_state(mmc, &state);
	} while (!error && (state == MMC_DEV_STATE_PRG));

	return error;
}

static int sdmmc_set_buswidth(struct t_mmc_ctx *mmc)
{
	int error = 0;

	if (mmc->bus_width == MMC_1_BIT_WIDE)
		return 0;

	if (IS_MMC(mmc)) {
		if (MMC_VERSION(mmc) == 4)
			error = sdmmc_set_extcsd(mmc, MMC_CMD_EXTCSD_BUS_WIDTH,
						 mmc->bus_width);

	} else {
		/* SDCard => Send CMD55 and CMD6 if 4bits */
		if (mmc->bus_width == MMC_4_BIT_WIDE) {
			error = sdmmc_send_cmd(mmc, SDMMC_CMD55,
					       MMC_RESPONSE_R1, mmc->rca << 16);
			if (!error)
				error = sdmmc_send_cmd(mmc, SDMMC_CMD6,
						       MMC_RESPONSE_R1, 2);
		}
	}

	/* Set wide bus operation for the controller */
	if (!error)
		mmc->ops->set_bus_width(mmc, mmc->bus_width);

	return error;
}

static int sdmmc_enable_hw_reset(struct t_mmc_ctx *mmc)
{
	if (mmc->rst_pin < 0)
		return 0;

	if (IS_MMC(mmc))
		return sdmmc_set_extcsd(mmc, MMC_CMD_EXTCSD_RST_N_FUNC, 1);
	else
		return 0;
}

static int sdmmc_initialize_card(struct t_mmc_ctx *mmc)
{
	int error = 0;

	/* Set default rca parameter in context */
	mmc->rca = 0x06;  /* Same value as ATF to avoid reinit */

	/* Send ALL_SEND_CID (CMD2) for all the card but SDIO ones */
	if (!IS_SDIO(mmc)) {
		error = sdmmc_send_cmd(mmc, SDMMC_CMD2, MMC_RESPONSE_R2, 0);
		/* CID = R2 type response */
		mmc->cid[0] = mmc->resp[0];
		mmc->cid[1] = mmc->resp[1];
		mmc->cid[2] = mmc->resp[2];
		mmc->cid[3] = mmc->resp[3];
		if (error)
			return error;
	}

	/*
	 * Send CMD3 for all Card types:
	 *	Get relative addr for mmc
	 *	Set relative addr for sd
	 */
	error = sdmmc_send_cmd(mmc, SDMMC_CMD3, MMC_RESPONSE_R6,
			       mmc->rca << 16);
	if (error)
		return error;
	if (!IS_MMC(mmc))
		mmc->rca = (mmc->resp[0] >> 16);

	/* Send SEND_CSD (CMD9) for all the card but SDIO ones */
	if (!IS_SDIO(mmc)) {
		error = sdmmc_send_cmd(mmc, SDMMC_CMD9, MMC_RESPONSE_R2,
				       mmc->rca << 16);
		/* CSD = R2 type response */
		mmc->csd[0] = mmc->resp[0];
		mmc->csd[1] = mmc->resp[1];
		mmc->csd[2] = mmc->resp[2];
		mmc->csd[3] = mmc->resp[3];
		if (error)
			return error;
	}

	error = sdmmc_select_card(mmc);

	/* Card get initialized */
	return error;
}

/* MMC context management functions */
static struct t_mmc_ctx *reserve_mmc_instance(void)
{
	uint32_t i;

	/*  Search a free instance */
	for (i = 0; i < NELEMS(context); i++) {
		if (context[i].base == 0)
			return &context[i];
	}

	return 0; /* No instances available */
}

static struct t_mmc_ctx *search_mmc_instance(enum t_mmc_port port)
{
	uint32_t i;

	for (i = 0; i < NELEMS(context); i++) {
		if (context[i].base == mmc_bases[port])
			return &context[i];
	}

	return 0; /* Not found */
}

static inline void free_mmc_instance(struct t_mmc_ctx *mmc)
{
	if (mmc) {
		memset(mmc, 0, sizeof(*mmc));
	}
}

/**
 * @brief	Initialize an MMC instance
 * @param	port - the MMC port number to initialize
 * @param	force - to force full reinit
 * @return	mmc context or 0 if error
 */
struct t_mmc_ctx *sdmmc_init(enum t_mmc_port port, bool force)
{
	uint32_t error = 0;
	int bus_width;
	const char *pinmux = NULL;
	struct t_mmc_ctx *mmc;
	int rst_pin = -1;

	/* Set right pinnmux and buswidth following board */
	bus_width = MMC_4_BIT_WIDE;
	switch (get_board_id()) {
	case BOARD_TC3P_CARRIER:
	case BOARD_TC3P_MTP:
		switch (port) {
		case SDMMC0_PORT:
			/* Telmo rev B or C => SD-card */
			if (get_mmc_boot_dev() == SDMMC0_PORT) {
				/* FIXME: Rev A and D SDMMC0 may be an emmc */
				bus_width = MMC_8_BIT_WIDE;
				pinmux = "emmc0_mux";
			} else {
				pinmux = "sdmmc0_mux";
			}
			break;

		case SDMMC1_PORT:
			/* On Telmo board rev B and C, SDMMC1 is the EMMC */
			if (get_mmc_boot_dev() == SDMMC1_PORT)
				bus_width = MMC_8_BIT_WIDE;

			pinmux = "sdmmc1_mux";
			rst_pin = A7_GPIO(86);
			break;

		default:
			break;
		}
		break;

	default: /* Other boards (A5) SDMMC2 is the EMMC */
		if (get_mmc_boot_dev() == SDMMC2_PORT) {
			bus_width = MMC_8_BIT_WIDE;
			pinmux = "sdmmc2_mux";
			rst_pin = A7_GPIO(50);
		} else {
			/* First unmux all possible mmc1 muxes */
			pinmux_request("sdmmc1_unmux");
			pinmux = "sdmmc1_mux";
		}
		break;
	}

	mmc = search_mmc_instance(port);
	if (mmc) /* Already Initialized => just return mmc ctx */
		return mmc;

	/* Search a free instance */
	mmc = reserve_mmc_instance();

	if (!mmc)
		return 0; /* No instance available */

	error = sdmmc_init_context(mmc, port, pinmux, bus_width, rst_pin);

	if (!error && (get_boot_dev_base() != mmc_bases[port] || force)) {
		/* first time init */
		if (!force)
			hsem_lock(hsem);
		error = mmc->ops->hw_init(mmc, IO_VOLT_330);
		if (!error)
			error = sdmmc_discover_card(mmc);
		if (!error)
			error = sdmmc_initialize_card(mmc);
		if (!error)
			error = mmc->ops->change_frequency(mmc, 50000000);
		if (!error)
			error = sdmmc_set_buswidth(mmc);
		if (!error)
			error = sdmmc_enable_hw_reset(mmc);
		if (error) {
			if (!force)
				hsem_unlock(hsem);
			TRACE_ERR("%s: Err %d\n", __func__, error);
			return 0;
		}

		/* Save mmc context in shared data for next init */
		if (!get_boot_dev_base()) {
			set_boot_dev_base(mmc->base);
			set_mmc_bus_width(mmc->bus_width);
			set_mmc_card_type(mmc->cardtype);
			set_mmc_rca(mmc->rca);
			set_mmc_ocr(mmc->ocr);
			set_mmc_csd((uint8_t *)mmc->csd);
		}
		if (!force)
			hsem_unlock(hsem);
	} else {
		/* Get saved context */
		mmc->cardtype = get_mmc_card_type();
		mmc->rca = get_mmc_rca();
		mmc->ocr = get_mmc_ocr();
		mmc->bus_width = get_mmc_bus_width();
		memcpy(mmc->csd, get_mmc_csd(), sizeof(mmc->csd));
	}

	return mmc;
}


/**
 * @brief	Free the given MMC instance
 * @param	mmc - the MMC instance to free
 * @return	None
 */
void sdmmc_remove(struct t_mmc_ctx *mmc)
{
	free_mmc_instance(mmc);
}

int sdmmc_reset(enum t_mmc_port port)
{
	struct t_mmc_ctx *mmc;

	/* Search the instance with this port */
	mmc = search_mmc_instance(port);
	if (mmc->rst_pin < 0)
		return MMC_NOT_SUPPORTED_ERROR;

	gpio_clear_gpio_pin(mmc->rst_pin);
	mdelay(10);
	gpio_set_gpio_pin(mmc->rst_pin);
	return 0;
}

int sdmmc_init_boot(enum t_mmc_port port, uint8_t *buffer, uint32_t boot_size,
		    const char *pinmux, uint32_t bus_width, bool high_speed)
{
	int error = 0;

	error = sdmmc_init_context(&context[0], port, pinmux, bus_width, -1);
	if (error)
		return error;
	return context[0].ops->init_boot(&context[0], boot_size,
					 (uint32_t *)buffer, (high_speed != 0));
}

/**
 * @brief	This API is used to read the data from the MMC device
 * @param	sector - address in sector where to read
 * @param	buffer - buffer to hold the data read from the device
 * @param	length - length in bytes of the buffer
 * @return	read length or error if < 0
 */
int sdmmc_read(struct t_mmc_ctx *mmc, uint32_t sector, void *buffer,
	       uint32_t length)
{
	int error = 0;
	uint32_t blkcnt, spare;
	uint32_t addr = 0;

	if (length > MMC_TRANSFERT_MAX_SIZE)
		length = MMC_TRANSFERT_MAX_SIZE;
	blkcnt = length / mmc->blksize;
	spare = length % mmc->blksize;

	hsem_lock(hsem);
	if (blkcnt) {
		if ((mmc->ocr & OCR_ACCESS_MODE_MASK) == OCR_BYTE_MODE)
			addr = sector * mmc->blksize;
		else
			addr = sector;

		/* Reading data */
		error = mmc->ops->data_cmd(mmc,
					   blkcnt > 1 ?
						SDMMC_CMD18 : SDMMC_CMD17,
					   addr, buffer,
					   blkcnt * mmc->blksize);
		if (error)
			goto read_error;

		if (blkcnt > 1)
			sdmmc_send_cmd(mmc, SDMMC_CMD12, MMC_RESPONSE_R1B, 0);
	}

	if (spare) {
		uint8_t spare_buf[MMC_BLOCK_SIZE];

		if ((mmc->ocr & OCR_ACCESS_MODE_MASK) == OCR_BYTE_MODE)
			addr = (sector + blkcnt) * mmc->blksize;
		else
			addr = sector + blkcnt;

		/* Reading data */
		error = mmc->ops->data_cmd(mmc, SDMMC_CMD17,
					   addr, spare_buf, mmc->blksize);
		if (!error)
			memcpy((uint8_t *)(buffer + (blkcnt * mmc->blksize)),
			       spare_buf, spare);
	}

read_error:
	if (error)
		TRACE_ERR("%s: err=%d\n", __func__, error);
	hsem_unlock(hsem);

	return error ? error : (int)length;
}

/**
 * @brief	This API is used to write the data to the MMC device
 * @param	sector - address in sector where to read
 * @param	buffer - buffer holding the data to write to device
 * @param	length - length in bytes of the buffer
 * @return	written length or error if < 0
 */
int sdmmc_write(struct t_mmc_ctx *mmc, uint32_t sector, void *buffer,
		uint32_t length)
{
	int error = 0;
	uint32_t blkcnt, spare;
	uint32_t state, addr = 0;

	if (length > MMC_TRANSFERT_MAX_SIZE)
		length = MMC_TRANSFERT_MAX_SIZE;
	blkcnt = length / mmc->blksize;
	spare = length % mmc->blksize;

	hsem_lock(hsem);
	if (blkcnt) {
		if ((mmc->ocr & OCR_ACCESS_MODE_MASK) == OCR_BYTE_MODE)
			addr = sector * mmc->blksize;
		else
			addr = sector;
		/* Sending  data */
		error = mmc->ops->data_cmd(mmc,
					   blkcnt > 1 ? SDMMC_CMD25 : SDMMC_CMD24,
					   addr, buffer, blkcnt * mmc->blksize);
		if (error)
			goto write_error;

		if (blkcnt > 1)
			sdmmc_send_cmd(mmc, SDMMC_CMD12, MMC_RESPONSE_R1B, 0);
		/* Wait ready status */
		do {
			sdmmc_get_state(mmc, &state);
		} while (state == MMC_DEV_STATE_PRG);
	}

	if (!error && spare) {
		uint8_t spare_buf[MMC_BLOCK_SIZE];

		if ((mmc->ocr & OCR_ACCESS_MODE_MASK) == OCR_BYTE_MODE)
			addr = (sector + blkcnt) * mmc->blksize;
		else
			addr = sector + blkcnt;
		memset(spare_buf, 0, mmc->blksize);

		memcpy(spare_buf, buffer + (blkcnt * mmc->blksize), spare);
		error = mmc->ops->data_cmd(mmc, SDMMC_CMD24,
					   addr, spare_buf, mmc->blksize);
		if (error)
			goto write_error;

		/* Wait ready status */
		do {
			sdmmc_get_state(mmc, &state);
		} while (state == MMC_DEV_STATE_PRG);
	}

write_error:
	if (error)
		TRACE_ERR("%s: err=%d\n", __func__, error);
	hsem_unlock(hsem);

	return error ? error : (int)length;
}

/**
 * @brief	This function send commands to set the configuration partition
 * @param	mmc - the MMC instance to use
 * @param	part: hardware partition number to access or -1 if no change
 * @param	part_enable: boot partition to enable or -1 if no change
 * @param	bootack: ack required in boot operation or -1 if no change
 * @return	0 if no error, not 0 otherwise
 */
int sdmmc_set_part_config(struct t_mmc_ctx *mmc, int part,
			  int part_enable, int bootack)
{
	int error = 0;
	struct mmc_ext_csd_t *extcsd = (struct mmc_ext_csd_t *)mmc->extcsd;

	if (IS_MMC(mmc)) {
		hsem_lock(hsem);
		error = sdmmc_get_extcsd(mmc);
		if (error)
			return error;

		if (part != -1)
			extcsd->boot_config.bit.part_access = part;
		if (part_enable != -1)
			extcsd->boot_config.bit.boot_parten = part_enable;
		if (bootack != -1)
			extcsd->boot_config.bit.boot_ack = bootack;

		error = sdmmc_set_extcsd(mmc, MMC_CMD_EXTCSD_PARTITION_CONFIG,
					 extcsd->boot_config.reg);
		hsem_unlock(hsem);
	} else {
		error = MMC_NOT_SUPPORTED_ERROR;
	}

	return error;
}

/**
 * @brief	This function send commands to set the bus configuration
 *		in boot operation
 * @param	uint32_t boot_mode - full speed or high speed
 * @param	uint32_t rst_boot_bus_w - retain or not conf after boot
 * @param	 uint32_t boot_bus_width - bus width during boot operation
 * @return	0 if no error, not 0 otherwise
 */
int sdmmc_set_boot_part(uint32_t boot_mode, uint32_t rst_boot_bus_w,
			uint32_t boot_bus_width)
{
	int error;
	struct mmc_ext_csd_t *cmd8_buffer =
		(struct mmc_ext_csd_t *)context[0].extcsd;

	if (IS_MMC(&context[0])) {
		hsem_lock(hsem);
		error = sdmmc_get_extcsd(&context[0]);
		if (!error) {
			cmd8_buffer->boot_bus_width = (boot_mode << 3)
				| (rst_boot_bus_w << 2) | boot_bus_width;

			error = sdmmc_set_extcsd(&context[0],
						 MMC_CMD_EXTCSD_BOOT_BUS_WIDTH,
						 cmd8_buffer->boot_bus_width << 8);
		}
		hsem_unlock(hsem);

	} else {
		error = MMC_NOT_SUPPORTED_ERROR;
	}

	return error;
}

/* ---------------------- RPMB relative functions -------------------------- */

static void rpmb_hmac(const uint8_t *key, uint8_t *buff, int len,
		      uint8_t *output)
{
	sha256_ctx ctx;
	int i;
	uint8_t ipad[SHA256_BLOCK_SIZE];
	uint8_t opad[SHA256_BLOCK_SIZE];

	/**
	 * See JEDEC eMMC Standard 4.41 or higher,
	 * Chapter: 7.6.16.3 Message Authentication Code Calculation
	 * and RFC4634 July 2006
	 */

	for (i = 0; i < RPMB_MAC_KEY_LEN; i++) {
		ipad[i] = key[i] ^ 0x36;
		opad[i] = key[i] ^ 0x5c;
	}
	/* Remaining pad bytes are '\0' XOR'd with ipad and opad values */
	for ( ; i < SHA256_BLOCK_SIZE; i++) {
		ipad[i] = 0x36;
		opad[i] = 0x5c;
	}
	sha256_init(&ctx);
	sha256_update(&ctx, ipad, sizeof(ipad));
	sha256_update(&ctx, buff, len);
	sha256_final(&ctx, output);

	/* Second pass: start with outer pad */
	sha256_init(&ctx);
	sha256_update(&ctx, opad, sizeof(opad));

	/* add previous hash */
	sha256_update(&ctx, output, RPMB_MAC_KEY_LEN);

	sha256_final(&ctx, output);
}

static int rpmb_read_response(struct t_mmc_ctx *mmc, struct t_rpmb *frame,
			      uint32_t expected_resp)
{
	int error;

	error = sdmmc_send_cmd(mmc, SDMMC_CMD23, MMC_RESPONSE_R1, 1);
	if (!error)
		error = mmc->ops->data_cmd(mmc, SDMMC_CMD18, 0, frame,
					   sizeof(*frame));
	if (!error) {
		/* Check response and status */
		if (swap16(frame->request) != expected_resp)
			return MMC_RPMB_BAD_RESP;

		error = swap16(frame->result);
		if (error) {
			trace_printf("RPMB: %s [%d]\n",
				     (error & RPMB_WRCNT_EXPIRED_ERR) ?
				     "Write counter expired" : "error", error);
			switch (error & ~RPMB_WRCNT_EXPIRED_ERR) {
			case RPMB_AUTH_FAILURE:
				error = MMC_RPMB_AUTH_FAILURE_ERROR;
				break;
			case RPMB_COUNTER_FAILURE:
				error = MMC_RPMB_COUNTER_FAILURE_ERROR;
				break;
			case RPMB_ADDR_FAILURE:
				error = MMC_RPMB_ADDR_FAILURE_ERROR;
				break;
			case RPMB_WRITE_FAILURE:
				error = MMC_RPMB_WRITE_FAILURE_ERROR;
				break;
			case RPMB_READ_FAILURE:
				error = MMC_RPMB_READ_FAILURE_ERROR;
				break;
			case RPMB_KEY_NOT_PROGRAMMED:
				error = MMC_RPMB_KEY_NOT_PROGRAMMED;
				break;
			case RPMB_GENERAL_FAILURE:
			default:
				error = MMC_RPMB_GENERAL_FAILURE_ERROR;
				break;
			}
		}
	}

	return error;
}

static int rpmb_status(struct t_mmc_ctx *mmc, struct t_rpmb *frame,
		       uint32_t expected)
{
	int error;

	memset(frame, 0, sizeof(*frame));
	frame->request = swap16(RPMB_RESULT_RD_REQ);
	error = rpmb_request(mmc, frame, false);

	if (!error)
		error = rpmb_read_response(mmc, frame, expected);

	return error;
}

/**
 * @brief	Write a RPMB frame
 *		First, you have to select RPMB partition
 * @param	mmc: mmc context to access
 * @param	frame: the rpmb frame to write
 * @param	reliable_write: set to true for an authenticated write
 * @return	0 if no error, not 0 otherwise
 */
int rpmb_request(struct t_mmc_ctx *mmc, const void *frame, bool reliable_write)
{
	int error;
	uint32_t state;
	uint32_t arg = (reliable_write ? BIT(31) : 0) | 1;

	error = sdmmc_send_cmd(mmc, SDMMC_CMD23, MMC_RESPONSE_R1, arg);
	if (!error)
		error = mmc->ops->data_cmd(mmc, SDMMC_CMD25, 0, (void *)frame,
					   sizeof(struct t_rpmb));
	/* Wait ready status */
	do {
		sdmmc_get_state(mmc, &state);
	} while (state == MMC_DEV_STATE_PRG);

	return error;
}

/**
 * @brief	Writing key (one time programming)
 *		First, you have to select RPMB partition
 * @param	mmc: mmc context to access
 * @param	key - The key to write
 * @return	0 if no error, not 0 otherwise
 */
int sdmmc_rpmb_write_key(struct t_mmc_ctx *mmc, const uint8_t *key)
{
	int error;
	struct t_rpmb frame;

	hsem_lock(hsem);
	memset(&frame, 0, sizeof(frame));
	frame.request = swap16(RPMB_AUTH_KEY_PGM_REQ);
	memcpy(frame.mac_key, key, sizeof(frame.mac_key));
	error = rpmb_request(mmc, &frame, true);
	if (!error)
		/* read the operation status */
		error = rpmb_status(mmc, &frame, RPMB_AUTH_KEY_PGM_RESP);
	hsem_unlock(hsem);

	return error;
}

/**
 * @brief	Reading counter number in mmc
 * @param	mmc: mmc context to access
 * @param	p_counter: returned counter
 * @return	0 if no error, not 0 otherwise
 */
int sdmmc_rpmb_read_counter(struct t_mmc_ctx *mmc, uint32_t *p_counter)
{
	int error;
	struct t_rpmb frame;

	hsem_lock(hsem);
	memset(&frame, 0, sizeof(frame));
	frame.request = swap16(RPMB_WR_CNTR_VAL_REQ);
	error = rpmb_request(mmc, &frame, false);

	if (!error) {
		error = rpmb_read_response(mmc, &frame, RPMB_WR_CNTR_VAL_RESP);
		if (!error)
			*p_counter = swap32(frame.write_counter);
	}
	hsem_unlock(hsem);

	return error;
}

/**
 * @brief	Autenticated Read buffers
 * @param	mmc: mmc context to access
 * @param	address: RPMB block address to read
 * @param	buffer: output read buffer
 * @param	length: length to read
 * @param	key: The RPMB key to use (optionnal)
 * @return	0 if no error else error code
 */
int sdmmc_rpmb_read(struct t_mmc_ctx *mmc, uint32_t address,
		    void *buffer, uint32_t length, const uint8_t *key)
{
	int i, error = 0;
	uint32_t read_len;
	struct t_rpmb frame;

	hsem_lock(hsem);
	for (i = 0; length != 0; i++) {
		read_len = (length >= RPMB_DATA_LEN) ? RPMB_DATA_LEN : length;
		memset(&frame, 0, sizeof(struct t_rpmb));
		frame.address = swap16(address + i);
		frame.request = swap16(RPMB_AUTH_DAT_RD_REQ);
		error = rpmb_request(mmc, &frame, false);
		if (!error)
			error = rpmb_read_response(mmc, &frame,
						   RPMB_AUTH_DAT_RD_RESP);
		if (error)
			break;

		/* Check the HMAC if necessary */
		if (key) {
			uint8_t hmac[RPMB_MAC_KEY_LEN];

			rpmb_hmac(key, frame.data, 284, hmac);
			if (memcmp(hmac, frame.mac_key, RPMB_MAC_KEY_LEN)) {
				TRACE_ERR("RPMB MAC error @ block #%d\n", i);
				break;
			}
		}
		memcpy(buffer + i * RPMB_DATA_LEN, frame.data, read_len);
		length -= read_len;
	}
	hsem_unlock(hsem);

	return error;
}

/**
 * @brief	Autenticated Write buffers
 * @param	mmc: mmc context to access
 * @param	address: RPMB block address to write
 * @param	buffer: input buffer to write
 * @param	length: length to write
 * @param	key: The RPMB key to use
 * @return	0 if no error else error code
 */
int sdmmc_rpmb_write(struct t_mmc_ctx *mmc, uint32_t address,
		     void *buffer, uint32_t length, const uint8_t *key)
{
	int i, error = 0;
	uint32_t write_len, write_count;
	struct t_rpmb frame;

	hsem_lock(hsem);
	for (i = 0; length != 0; i++) {
		error = sdmmc_rpmb_read_counter(mmc, &write_count);
		if (error)
			return error;
		write_len = (length >= RPMB_DATA_LEN) ? RPMB_DATA_LEN : length;
		memset(&frame, 0, sizeof(struct t_rpmb));
		memcpy(frame.data, buffer + i * RPMB_DATA_LEN, write_len);
		frame.write_counter = swap32(write_count);
		frame.address = swap16(address + i);
		frame.block_count = swap16(1);
		frame.request = swap16(RPMB_AUTH_DAT_WR_REQ);
		rpmb_hmac(key, frame.data, 284, frame.mac_key);

		error = rpmb_request(mmc, &frame, true);
		if (!error)
			error = rpmb_status(mmc, &frame, RPMB_AUTH_DAT_WR_RESP);
		if (error)
			break;
		length -= write_len;
	}
	hsem_unlock(hsem);

	return error;
}

/* ------------------ Basic unitary test ----------------------------------- */

#if defined SDMMC_TESTS
void sdmmc_tests(void)
{
	int err;
	struct t_mmc_ctx *mmc;
	uint32_t i;
	static uint32_t sdmmc_buf[4096 / 4];
	uint32_t sector = 0x10000;
	uint32_t start;
	uint32_t speed;
	uint32_t len;
	int port = get_mmc_boot_dev(); /* The MMC boot port to test */

	TRACE_NOTICE("SDMMC Init port: %d\n", port);
	mmc = sdmmc_init(port, true);
	if (!mmc) {
		TRACE_ERR("Can't Init SDMMC\n");
		return;
	}

	len = MMC_BLOCK_SIZE;
	TRACE_NOTICE("SDMMC Writing 1 sector #%08X... ", sector);
	for (i = 0; i < len; i++)
		*((char *)sdmmc_buf + i) = 0x11;
	err = sdmmc_write(mmc, sector, sdmmc_buf, len);
	if (err < 0)
		goto err;

	TRACE_NOTICE("SDMMC Reading 1 sector #%08X... ", sector);
	memset(sdmmc_buf, 0, len);
	err = sdmmc_read(mmc, sector, sdmmc_buf, len);
	if (err < 0)
		goto err;
	/* Test memory */
	for (i = 0; i < len; i++) {
		char val = *((char *)sdmmc_buf + i);

		if (val != 0x11)
			TRACE_NOTICE("ERROR: @%d: %d\n", i, val);
	}

	len = sizeof(sdmmc_buf);
	TRACE_NOTICE("SDMMC Writing sector #%08X, len:%d... ", sector, len);
	for (i = 0; i < len; i++)
		*((char *)sdmmc_buf + i) = (char)i;
	start = mtu_get_time(0);

	err = sdmmc_write(mmc, sector, sdmmc_buf, len);
	if (err < 0)
		goto err;

	speed = ((len >> 10) * 1000000) / mtu_get_time(start);
	TRACE_NOTICE("OK at %dKB/s\n", speed);

	TRACE_NOTICE("SDMMC Reading sector #%08X, len:%d... ", sector, len);
	memset(sdmmc_buf, 0, len);
	start = mtu_get_time(0);
	err = sdmmc_read(mmc, sector, sdmmc_buf, len);
	if (err < 0)
		goto err;
	speed = ((len >> 10) * 1000000) / mtu_get_time(start);
	/* Test memory */
	for (i = 0; i < len; i++) {
		char val = *((char *)sdmmc_buf + i);

		if (val != (char)i) {
			TRACE_NOTICE("ERROR: @%d: %d\n", i, val);
			goto err;
		}
	}
	TRACE_NOTICE("OK\n");

	if (IS_MMC(mmc)) {
		const uint8_t rpmb_key[RPMB_MAC_KEY_LEN] = {
			'S', 'T', '-', 'A', 'C', 'R', '5', '-',
			'T', 'C', '3', 'P', ' ', ' ', ' ', ' ',
			' ', ' ', ' ', ' ', 'P', '3', 'C', 'T',
			'-', '5', 'R', 'C', 'A', '-', 'T', 'S',
		};
		uint32_t w_counter;

		TRACE_NOTICE("RPMB tests, select RPMB...\n");
		/* Select RPMB */
		err = sdmmc_set_part_config(mmc, MMC_PART_RPMB, -1, -1);
		if (err)
			goto err;

		TRACE_NOTICE("Write key ");
		err = sdmmc_rpmb_write_key(mmc, rpmb_key);
		if (err)
			TRACE_NOTICE("Ignore error: key may be already written\n");
		else
			TRACE_NOTICE("OK\n");
		TRACE_NOTICE("Read write counter: ");
		err = sdmmc_rpmb_read_counter(mmc, &w_counter);
		if (err)
			goto exit_rpmb;
		TRACE_NOTICE("%d\n", w_counter);

		TRACE_NOTICE("Write 2 RPMB buffers at offset 10: ");
		err = sdmmc_rpmb_write(mmc, 10, sdmmc_buf, 512, rpmb_key);
		if (err)
			goto exit_rpmb;
		TRACE_NOTICE("OK\n");

		TRACE_NOTICE("Re-read write counter: ");
		err = sdmmc_rpmb_read_counter(mmc, &w_counter);
		if (err)
			goto exit_rpmb;
		TRACE_NOTICE("%d\n", w_counter);

		TRACE_NOTICE("Read 2 RPMB buffers: ");
		err = sdmmc_rpmb_read(mmc, 10, sdmmc_buf, 512, rpmb_key);
		if (err)
			goto exit_rpmb;
		TRACE_NOTICE("OK\n");

		/* Test memory */
		for (i = 0; i < len; i++) {
			char val = *((char *)sdmmc_buf + i);

			if (val != (char)i) {
				TRACE_NOTICE("ERROR: @%d: %d\n", i, val);
				break;
			}
		}
exit_rpmb:
		/* Return to default user partition */
		sdmmc_set_part_config(mmc, MMC_PART_USER, -1, -1);
		if (err)
			goto err;
	}

	TRACE_NOTICE("OK at %dKB/s\n", speed);
	return;

err:
	TRACE_NOTICE("Test KO, err=%d\n", err);
}

#else

void sdmmc_tests(void) {}

#endif /* SDMMC Tests */

