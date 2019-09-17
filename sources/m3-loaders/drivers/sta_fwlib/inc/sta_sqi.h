/**
 * @file sta_sqi.h
 * @brief This file provides all the SQI firmware functions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#ifndef _SQI_CTRL_H_
#define _SQI_CTRL_H_

#include "sta_map.h"

#define SQI_INSTANCES_MAX 2

/**
 * @brief SQI context
 * @flash_id: Flash identifier
 * @manuf_id: Manufacturer identifier
 * @mode_rx: read mode
 * @mode_tx: write mode
 * @mode_rac: Register Access Command mode
 */
struct t_sqi_ctx {
	t_sqi *regs;
	uint32_t mmap_base;
	uint16_t flash_id;
	uint16_t jdec_extid;
	uint8_t manuf_id;
	bool	four_bytes_address;
	uint8_t mode_rx;
	uint8_t mode_tx;
	uint8_t mode_rac;
	uint8_t rpmx_family; /* See enum rpmx_family for valid values */
};

#define SQI_MODE_SPI				0x0 /* CAD: 1/1/1 wire */
#define SQI_MODE_QPI				0x1 /* CAD: 4/4/4 wires */
#define SQI_MODE_QSPI				0x2 /* CAD: 1/4/4 wires */
#define SQI_MODE_QSPI2				0x3 /* CAD: 1/1/4 wires */

/*
 * SQI_QUAD_MODE values:
 *	SQI_MODE_QSPI (CMD: 1 wire, ADDR/DATA: 4 wires)
 *	SQI_MODE_QPI (CMD/ADDR/DATA: 4 wires) but it's not compatible with ROM CODE
 * after reset or watchdog
 */
#define SQI_QUAD_MODE	SQI_MODE_QPI

#define SQI_DUMMY_CYCLES_SHIFT		15

#define SQI_DUMMYCYCLES_SPI_8		0
#define SQI_DUMMYCYCLES_SPI_10		4
#define SQI_DUMMYCYCLES_SPI_6		5
#define SQI_DUMMYCYCLES_SPI_4		6

#define SQI_DUMMYCYCLES_QPI_2		0
#define SQI_DUMMYCYCLES_QPI_4		1
#define SQI_DUMMYCYCLES_QPI_6		2
#define SQI_DUMMYCYCLES_QPI_8		3
#define SQI_DUMMYCYCLES_QPI_10		4

#define SQI_DUMMYCYCLES_SQI1_4		1
#define SQI_DUMMYCYCLES_SQI1_6		2
#define SQI_DUMMYCYCLES_SQI1_8		3
#define SQI_DUMMYCYCLES_SQI1_10		4

#define SQI_DUMMYCYCLES_SQI2_4		1
#define SQI_DUMMYCYCLES_SQI2_6		5
#define SQI_DUMMYCYCLES_SQI2_8		3
#define SQI_DUMMYCYCLES_SQI2_10		4

/**
 * enum sqi_erase_size - defines the granularity to be taken into account
 * during block erase operation
 */
enum sqi_erase_size {
	SQI_4KB_GRANULARITY,
	SQI_32KB_GRANULARITY,
	SQI_64KB_GRANULARITY,
	SQI_CHIP_ERASE,
	SQI_INVAL,
};

/**
 * @brief  Enable/disable QPI Read Mode
 * @param  ctx: SQI context
 * @return none
 */
void sqi_set_qpi_mode(struct t_sqi_ctx *ctx, bool enable);

/**
 * @brief Setup HW to enable SQI peripheral
 * @param  instance: instance numero 0 or 1
 * @return 0 if error, pointer to SQI context otherwise
 */
struct t_sqi_ctx *sqi_init(uint8_t instance);

/**
 * @brief deinit/reset SQI to normal SPI mode
 * @param  ctx: SQI context
 * @return None
 */
void sqi_deinit(struct t_sqi_ctx *ctx);

/**
 * @brief  Enable extended 32bits address mode
 * @param  ctx: SQI context
 * @return none
 */
void sqi_enable_4B(struct t_sqi_ctx *ctx);

/**
 * @brief  Disable extended 32bits address mode
 * @param  mode: the operating mode (SQI/SPI)
 * @return none
 */
void sqi_disable_4B(struct t_sqi_ctx *ctx);

/**
 * @brief  Erase a memory block
 * @param  ctx: SQI context
 * @param  addr: address belonging to erasable block
 * @param  granularity: erase granularity @see sqi_erase_size
 * @return 0 if no error, not 0 otherwise
 */
int sqi_erase_block(struct t_sqi_ctx *ctx, uint32_t addr, int granularity);

/**
 * @brief  Read data
 * @param  ctx: SQI context
 * @param  addr: address belonging to eresable block
 * @param  buffer: buffer pointer
 * @param  length: buffer length
 * @return length if no error, < 0 otherwise
 */
int sqi_read(struct t_sqi_ctx *ctx, uint32_t addr,
		 void *buffer, uint32_t length);
/**
 * @brief  Write data
 * @param  ctx: SQI context
 * @param  addr: address belonging to eresable block
 * @param  buffer: buffer pointer
 * @param  length: buffer length
 * @return 0 if no error, not 0 otherwise
 */
int sqi_write(struct t_sqi_ctx *ctx, uint32_t addr,
		 uint32_t *buffer, uint32_t length);

/**
 * @brief  Basic SQI_tests
 */
void sqi_tests(struct t_sqi_ctx *ctx);

/**
 * @brief   SQI RPMC command ID to be sent to the RPMC device
 */
enum sqi_rpmc_cmd_id
{
	SQI_RPMC_CMD_WRKR = 0,
	SQI_RPMC_CMD_UHKR = 1,
	SQI_RPMC_CMD_IMC  = 2,
	SQI_RPMC_CMD_RMC  = 3,
	SQI_RPMC_CMD_RD   = 4
};

/**
 * @brief   SQI RPMC error codes enum
 */
enum sqi_rpmc_err
{
	SQI_RPMC_OK                     =   0,   /* !< successful completion */
	SQI_RPMC_KO                     =  -1,   /* !< generic failure */
	SQI_RPMC_BUSY_ERROR             =  -2,   /* !< card busy error */
	SQI_RPMC_SIG_ERROR              =  -3,   /* !< signature error */
	SQI_RPMC_PAYLOAD_ERROR          =  -4,   /* !< payload error */
	SQI_RPMC_INIT_ERROR             =  -5,   /* !< counter not initialized */
	SQI_RPMC_ROOT_KEY_AVAILABLE     =  -6,   /* !< ROOT KEY already there */
};

/**
 * @brief   SQI RPMC OP2 payload struct
 */
struct sqi_rpmc_read_reply_t
{
	uint8_t extended_status;
	uint8_t tag[12];
	uint8_t counter_read_data[4];
	uint8_t signature[32];
};

/**
 * @brief   SQI RPMC Write Root Key Register command payload type
 */
struct sqi_rpmc_wrkr_t
{
    uint8_t opcode;
    uint8_t cmd_type;
    uint8_t cnt_addr;
    uint8_t rsvd;
    uint8_t root_key[32];
    uint8_t truncated_sig[28];
};

/**
 * @brief   SQI RPMC Update HMAC key register command payload type
 */
struct sqi_rpmc_uhkr_t
{
	uint8_t opcode;
	uint8_t cmd_type;
	uint8_t cnt_addr;
	uint8_t rsvd;
	uint8_t key_data[4];
	uint8_t sig[32];
};

/**
 * @brief   SQI RPMC Increment monotonic counter payload type
 */
struct sqi_rpmc_imc_t
{
	uint8_t opcode;
	uint8_t cmd_type;
	uint8_t cnt_addr;
	uint8_t rsvd;
	uint8_t cnt_data[4];
	uint8_t sig[32];
};

/**
 * @brief   SQI RPMC Request monotonic counter payload type
 */
struct sqi_rpmc_rmc_t
{
	uint8_t opcode;
	uint8_t cmd_type;
	uint8_t cnt_addr;
	uint8_t rsvd;
	uint8_t tag[12];
	uint8_t sig[32];
};

/* RPMC Monotonic counter functions */
enum sqi_rpmc_err sqi_rpmc_init(struct t_sqi_ctx *ctx,
				struct sqi_rpmc_wrkr_t *payload);
enum sqi_rpmc_err sqi_rpmc_discover(struct t_sqi_ctx *ctx, uint8_t *);
enum sqi_rpmc_err sqi_rpmc_update_hmac_key_register(struct t_sqi_ctx *ctx,
						    struct sqi_rpmc_uhkr_t *);
enum sqi_rpmc_err sqi_rpmc_increment_monotonic_counter(struct t_sqi_ctx *ctx,
						       struct sqi_rpmc_imc_t *);
enum sqi_rpmc_err sqi_rpmc_request_monotonic_counter(struct t_sqi_ctx *ctx,
						     struct sqi_rpmc_rmc_t *);
void sqi_rpmc_read(struct t_sqi_ctx *ctx, struct sqi_rpmc_read_reply_t *);
enum sqi_rpmc_err sqi_rpmc_message_signature(struct t_sqi_ctx *ctx,
					     uint8_t *, uint8_t *, uint8_t *);
#endif /* _SQI_CTRL_H_ */
