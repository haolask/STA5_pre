/**
 *                           (C) 2009 STMicroelectronics
 *   Reproduction and Communication of this document is strictly prohibited
 *      unless specifically authorized in writing by STMicroelectronics.
 *----------------------------------------------------------------------------
 *                                  APG / CRM / SA&PD
 *                   Software Development Group - SW platform & HW Specific
 *----------------------------------------------------------------------------
 * Implements a SPI over SSP protocol.
 */

#include "string.h"
#include "sta_spi_service.h"
#include "sta_spi.h"
#include "sta_mtu.h"
#include "trace.h"

/**
 * Defines and macros
 */
#define SPI_PORTS                 3

#define SVC_SSP_HANDLER_SIZE                   sizeof(svc_ssp_handler_t)
#define SVC_SSP_PORT_HANDLER_SIZE              sizeof(svc_ssp_port_handler_t)
#define SVC_SSP_COM_HANDLER_SIZE               sizeof(svc_ssp_com_handler_t)
#define SVC_SSP_CR0_REG_SPI_DEFAULT_VAL        0xC7
#define SVC_SSP_CR0_REG_MICROWIRE_DEFAULT_VAL  0x470027
#define SVC_SSP_CR1_REG_SPI_DEFAULT_VAL        0x908
#define SVC_SSP_CR1_REG_MICROWIRE_DEFAULT_VAL  0x908
#define SVC_SSP_CR0_REG_ADDRESS                (SSP_REG_START_ADDR + 0x00)
#define SVC_SSP_CR1_REG_ADDRESS                (SSP_REG_START_ADDR + 0x04)

/**
 * @brief Handler for specific SPI peripheral
 */
typedef struct svc_ssp_com_hooks_s
{
	svc_ssp_hook_t pre_cb;  /**< callback to use before any R/w operation */
	void *pre_cb_param;     /**< parameter fro pre_cb */
	svc_ssp_hook_t post_cb; /**< callback to use after any R/w operation */
	void *post_cb_param;    /**< parameter fro post_cb */
} svc_ssp_com_hooks_t;

/**
 * @brief SSP COM handler
 */
struct svc_ssp_com_handler_s
{
	svc_ssp_com_handler_t *next; /**< Pointer to next COM in the queue */
	t_spi *port_id;
	xSemaphoreHandle access_sem; /**< access semaphore to com */
	svc_ssp_com_hooks_t hooks;   /**< Pre/post callbacks for specified COM */
	uint32_t out_clk;            /**< Output frequency of COM */
	uint8_t buf_size;            /**< Buffer item size of COM (8/16/32) */
	LLD_SSP_ConfigTy config;     /**< SSP configuration for COM */
};

/**
 * @brief SSP Port handler
 */
typedef struct svc_ssp_port_handler_s svc_ssp_port_handler_t;

struct svc_ssp_port_handler_s
{
	svc_ssp_port_handler_t *next;         /**< Next SSP port */
	t_spi                  *port_id;
	svc_ssp_com_handler_t  *curr_com_ptr; /**< COM currently using the port */
	xSemaphoreHandle       access_sem;    /**< access semaphore to port */
	xSemaphoreHandle       done_sem;      /**< semaphore to signal end of transfer */
	void                   *tx_buf_ptr;   /**< Pointer to TX buffer */
	void                   *rx_buf_ptr;   /**< Pointer to RX buffer */
	uint16_t               tx_pos;        /**< Position in TX buffer */
	uint16_t               rx_pos;        /**< Position in RX buffer */
	uint16_t               len;           /**< Length of I/O request */
};

/**
 * @brief SSP svc_mcu handler
 */
typedef struct svc_ssp_handler_s
{
	uint32_t bus_speed;                /**< Bus speed */
	uint8_t ports;
	svc_ssp_com_handler_t *com_head;   /**< Linked list of COMs */
	svc_ssp_port_handler_t *port_head; /**< Linked list of ports */
} svc_ssp_handler_t;

/**
 * Global variable definitions
 */

/**< Standard SSP_configuration for SPI protocol */
static const LLD_SSP_ConfigTy svc_ssp_basecfg_spi = {
	LLD_SSP_INTERFACE_MOTOROLA_SPI,
	LLD_SSP_MASTER,
	true,
	{MIN_CPSDVR, MIN_SCR},
	LLD_SSP_RX_MSB_TX_MSB,
	LLD_SSP_DATA_BITS_8,
	LLD_SSP_RX_8_OR_MORE_ELEM,
	LLD_SSP_TX_8_OR_MORE_EMPTY_LOC,
	LLD_SSP_CLK_RISING_EDGE,
	LLD_SSP_CLK_POL_IDLE_HIGH,
	(LLD_SSP_MicrowireCtrlTy)0,
	(LLD_SSP_MicrowireWaitStatelTy)0,
	(LLD_SSP_DuplexTy)0,
	false
};

/**< Standard SSP_configuration for NS MicroWire protocol */
static const LLD_SSP_ConfigTy svc_ssp_basecfg_microwire = {
	LLD_SSP_INTERFACE_NATIONAL_MICROWIRE,
	LLD_SSP_MASTER,
	true,
	{MIN_CPSDVR, MIN_SCR},
	LLD_SSP_RX_MSB_TX_MSB,
	LLD_SSP_DATA_BITS_8,
	LLD_SSP_RX_8_OR_MORE_ELEM,
	LLD_SSP_TX_8_OR_MORE_EMPTY_LOC,
	(LLD_SSP_ClkPhaseTy)0,
	(LLD_SSP_ClkPolarityTy)0,
	LLD_SSP_BITS_8,
	LLD_SSP_MICROWIRE_WAIT_ZERO,
	LLD_SSP_MICROWIRE_CHANNEL_HALF_DUPLEX,
	false
};

static svc_ssp_handler_t *svc_ssp_handler;
static uint32_t SSP_CR0_reg_value;

/**
 * Function implementations (scope: module-local)
 */

/**
 * @brief Extract a given size data from buffer
 *
 * @param buf_ptr Pointer to buffer
 * @param pos Position in buffer
 * @param fifo_item_size Size of each item in buffer
 * @retval value from buffer in given fifo_item_size
 */
static uint32_t svc_ssp_buf_read_data(void *buf_ptr, uint32_t pos,
				      uint32_t fifo_item_size)
{
	uint32_t data;

	if (fifo_item_size == 1)
		data = ((uint8_t *)buf_ptr)[pos];
	else if (fifo_item_size == 2)
		data = ((uint16_t *)buf_ptr)[pos];
	else if (fifo_item_size == 4)
		data = ((uint32_t *)buf_ptr)[pos];
	else
		data = 0;

	return data;
}

/**
 * @brief Set a given size data into a buffer
 *
 * @param buf_ptr Pointer to buffer
 * @param pos Position in buffer
 * @param data data to set
 * @param fifo_item_size Size of each item in buffer
 * @retval void
 */
static  void svc_ssp_buf_write_data(void *buf_ptr, uint32_t pos,
				    uint32_t data, uint32_t fifo_item_size)
{
	if (fifo_item_size == 1)
		((uint8_t *)buf_ptr)[pos] = data;
	else if (fifo_item_size == 2)
		((uint16_t *)buf_ptr)[pos] = data;
	else if (fifo_item_size == 4)
		((uint32_t *)buf_ptr)[pos] = data;
}

/**
 * @brief   Get SSP handler for specific port
 *
 * @param   ssp_port  port of wanted SSP handler
 * @retval  svc_ssp_port_handler_t * pointer to SSP handler or NULL if not open
 */
static svc_ssp_port_handler_t *svc_ssp_get_hdlr_ptr(t_spi *ssp_port)
{
	svc_ssp_port_handler_t *port_hdlr_ptr;

	port_hdlr_ptr = svc_ssp_handler->port_head;

	while (port_hdlr_ptr != NULL) {
		if (port_hdlr_ptr->port_id == ssp_port)
			return port_hdlr_ptr;
		else
			port_hdlr_ptr = port_hdlr_ptr->next;
	}

	return NULL;
}

/**
 * @brief SSP svc_mcu interrupt callback
 *
 * @param port_hdlr_ptr Port handler pointer
 * @retval void
 */
static void svc_ssp_callback(svc_ssp_port_handler_t *port_hdlr_ptr)
{
	svc_ssp_com_handler_t *com_hdlr_ptr = port_hdlr_ptr->curr_com_ptr;
	uint32_t rx_pos = port_hdlr_ptr->rx_pos;
	uint32_t tx_pos = port_hdlr_ptr->tx_pos;
	uint32_t len = port_hdlr_ptr->len;
	uint32_t tx_cnt = 0;
	uint32_t data;

	LLD_SSP_IRQSrcTy irq_status = LLD_SSP_GetIRQSrc(port_hdlr_ptr->port_id);

	/* Handler RX interrupts */
	if (irq_status &
	    (LLD_SSP_IRQ_SRC_RECEIVE | LLD_SSP_IRQ_SRC_RECEIVE_TIMEOUT)) {
		/* Save all available data from RX FIFO */
		while (LLD_SSP_IsRxFifoNotEmpty(port_hdlr_ptr->port_id) &&
		       (rx_pos < len)) {
			LLD_SSP_GetData(port_hdlr_ptr->port_id, &data);

			if (port_hdlr_ptr->rx_buf_ptr != NULL)
				svc_ssp_buf_write_data(port_hdlr_ptr->rx_buf_ptr,
						       rx_pos, data,
						       com_hdlr_ptr->buf_size);

			rx_pos++;

			/*
			 * If there is a remaining data to transmit,
			 * send it and this will then trig a new data in RX FIFO
			 */
			if ((LLD_SSP_IsTxFifoNotFull(port_hdlr_ptr->port_id)) &&
			    (tx_pos < len)) {
				data = svc_ssp_buf_read_data(port_hdlr_ptr->tx_buf_ptr,
							     tx_pos++,
							     com_hdlr_ptr->buf_size);
				LLD_SSP_SetData(port_hdlr_ptr->port_id, data);
				tx_cnt++;
			}
		}

		port_hdlr_ptr->rx_pos = rx_pos;

		/* If all data were received, disable port and RX IRQ */
		if (port_hdlr_ptr->rx_pos == port_hdlr_ptr->len) {
			LLD_SSP_DisableIRQSrc(port_hdlr_ptr->port_id,
					      LLD_SSP_IRQ_SRC_RECEIVE |
					      LLD_SSP_IRQ_SRC_RECEIVE_TIMEOUT);
			LLD_SSP_Disable(port_hdlr_ptr->port_id);
			LLD_SSP_ResetClock(port_hdlr_ptr->port_id);
			xSemaphoreGiveFromISR(port_hdlr_ptr->done_sem, NULL);
		}

		LLD_SSP_ClearIRQSrc(port_hdlr_ptr->port_id,
				    LLD_SSP_IRQ_SRC_RECEIVE |
				    LLD_SSP_IRQ_SRC_RECEIVE_TIMEOUT);
	}

	/* Handler TX interrupts */
	if (LLD_SSP_GetIRQConfig(port_hdlr_ptr->port_id)
	    & LLD_SSP_IRQ_SRC_TRANSMIT) {
		/* Write all possible data to TX FIFO */
		while ((LLD_SSP_IsTxFifoNotFull(port_hdlr_ptr->port_id))
		       && (tx_pos < len) && (tx_cnt < 32)) {
			uint32_t data = svc_ssp_buf_read_data(port_hdlr_ptr->tx_buf_ptr,
							      tx_pos++, com_hdlr_ptr->buf_size);
			LLD_SSP_SetData(port_hdlr_ptr->port_id, data);
			tx_cnt++;
		}

		port_hdlr_ptr->tx_pos = tx_pos;

		/* If all data where transmitted, disable TX interrupts */
		if (tx_pos == len)
			LLD_SSP_DisableIRQSrc(port_hdlr_ptr->port_id,
					      LLD_SSP_IRQ_SRC_TRANSMIT);

		LLD_SSP_ClearIRQSrc(port_hdlr_ptr->port_id,
				    LLD_SSP_IRQ_SRC_TRANSMIT);
	}
}

/**
  * Public functions
  */

/**
 * @brief Initialize SSP svc_mcu
 *
 * @param partition Partition to use to allocate memory
 * @param bus_speed Bus speed
 * @retval 0 if all is ok, else error
 */
int svc_ssp_init(uint32_t bus_speed)
{
	uint32_t ports;

	if (svc_ssp_handler != NULL)
		return 0;

	svc_ssp_handler = (svc_ssp_handler_t *) pvPortMalloc(sizeof(svc_ssp_handler_t));

	if (svc_ssp_handler == NULL)
		return -1;

	ports = SPI_PORTS;

	svc_ssp_handler->bus_speed   = bus_speed;
	svc_ssp_handler->ports       = ports;

	svc_ssp_handler->com_head    = NULL;
	svc_ssp_handler->port_head   = NULL;

	return 0;
}

/**
 * @brief Open SSP Port
 *
 * @param ssp_port Port number (hw specific)
 * @param irq_pri Interrupt priority of SSP svc_mcu
 * @retval 0 if all is ok, else error
 */
int svc_ssp_open_port(t_spi *ssp_port, int irq_pri)
{
	svc_ssp_port_handler_t *last_port_hdlr_ptr, *port_hdlr_ptr;
	int status = 0;

	if (svc_ssp_handler == NULL)
		return -1;

	/**< Check if port was already open */
	last_port_hdlr_ptr = svc_ssp_handler->port_head;
	port_hdlr_ptr = svc_ssp_handler->port_head;

	while (port_hdlr_ptr != NULL) {
		if (port_hdlr_ptr->port_id == ssp_port) {
			return -1;
		} else {
			last_port_hdlr_ptr = port_hdlr_ptr;
			port_hdlr_ptr = port_hdlr_ptr->next;
		}
	}

	port_hdlr_ptr = (svc_ssp_port_handler_t *)pvPortMalloc(sizeof(svc_ssp_port_handler_t));
	if(last_port_hdlr_ptr != NULL)
		last_port_hdlr_ptr->next = port_hdlr_ptr;

	if (port_hdlr_ptr == NULL)
		return -1;

	port_hdlr_ptr->access_sem = xSemaphoreCreateCounting(10, 0);
	port_hdlr_ptr->done_sem = xSemaphoreCreateCounting(10, 0);

	if ((port_hdlr_ptr->access_sem == NULL) ||
	    (port_hdlr_ptr->done_sem == NULL)) {
		vSemaphoreDelete(port_hdlr_ptr->done_sem);
		vSemaphoreDelete(port_hdlr_ptr->access_sem);

		vPortFree(port_hdlr_ptr);

		return -1;
	}


	port_hdlr_ptr->port_id      = ssp_port;

	port_hdlr_ptr->curr_com_ptr = NULL;

	port_hdlr_ptr->tx_buf_ptr   = NULL;
	port_hdlr_ptr->tx_pos       = 0;
	port_hdlr_ptr->rx_buf_ptr   = NULL;
	port_hdlr_ptr->rx_pos       = 0;
	port_hdlr_ptr->len          = 0;

	status = LLD_SSP_Init(ssp_port);
	if (!status) {
		LLD_SSP_Disable(ssp_port);
		LLD_SSP_ResetClock(ssp_port);
		LLD_SSP_Reset(ssp_port);
	}

	if (last_port_hdlr_ptr == NULL)
		svc_ssp_handler->port_head = port_hdlr_ptr;
	else
		last_port_hdlr_ptr = port_hdlr_ptr;

	xSemaphoreGive(port_hdlr_ptr->access_sem);

	return status;
}

/**
 * @brief Create COM on SSP port
 *
 * @param ssp_port SSP port used
 * @param ssp_mode Interface type
 * @param out_clk Out clock frequency
 * @param data_size Size of each data transmitted
 * @param pre_cb Callback called before any transfer
 * @param pre_cb_param Parameter passed to pre_cb
 * @param post_cb Callback called after any transfer
 * @param post_cb_param Parameter passed to post_cb
 * @retval COM handler pointer
 */
svc_ssp_com_handler_t *svc_ssp_create_com(t_spi *ssp_port,
					  LLD_SSP_InterfaceTy ssp_mode,
					  uint32_t out_clk,
					  LLD_SSP_DataSizeTy data_size,
					  svc_ssp_hook_t pre_cb,
					  svc_ssp_hook_param_t pre_cb_param,
					  svc_ssp_hook_t post_cb,
					  svc_ssp_hook_param_t post_cb_param)
{
	svc_ssp_com_handler_t *com_hdlr_ptr;
	svc_ssp_com_handler_t **last_com_hdlr_ptr_ptr;
	uint8_t buf_size;
	uint32_t effective_out_clk;
	const LLD_SSP_ConfigTy *ssp_config;

	/* Check if it's safe to create a new com */

	/* Check if SSP svc_mcu was initialized */
	if (svc_ssp_handler == NULL)
		return NULL;

	/* check if handler for specified port was initialized */
	if (svc_ssp_get_hdlr_ptr(ssp_port) == NULL)
		return NULL;

	/* check if data size is ok and configure in/out buffer size */
	if ((data_size >= LLD_SSP_DATA_BITS_4) && (data_size <= LLD_SSP_DATA_BITS_8))
		buf_size = 1;
	else if (data_size <= LLD_SSP_DATA_BITS_16)
		buf_size = 2;
	else if (data_size <= LLD_SSP_DATA_BITS_32)
		buf_size = 4;
	else
		return NULL;

	/* check if SSP mode is supported */
	switch (ssp_mode) {
	case LLD_SSP_INTERFACE_MOTOROLA_SPI:
		ssp_config = &svc_ssp_basecfg_spi;
		break;

	case LLD_SSP_INTERFACE_NATIONAL_MICROWIRE:
		ssp_config = &svc_ssp_basecfg_microwire;
		break;

	default:
		ssp_config = NULL;
		break;
	}

	if (ssp_config == NULL)
		return NULL;

	/* allocate memory for COM handler and exit if no space is available */
	com_hdlr_ptr = (svc_ssp_com_handler_t *)pvPortMalloc(sizeof(svc_ssp_com_handler_t));

	if (com_hdlr_ptr == NULL)
		return NULL;

	/* create access semaphore and exit if no space is available */
	com_hdlr_ptr->access_sem = xSemaphoreCreateCounting(10, 0);

	if (com_hdlr_ptr->access_sem == NULL) {
		vSemaphoreDelete(com_hdlr_ptr->access_sem);
		vPortFree(com_hdlr_ptr);

		return NULL;
	}

	/* Enqueue new COM handler on COM queue for specified port */
	last_com_hdlr_ptr_ptr = &svc_ssp_handler->com_head;

	while (*last_com_hdlr_ptr_ptr != NULL)
		last_com_hdlr_ptr_ptr = &(*last_com_hdlr_ptr_ptr)->next;

	*last_com_hdlr_ptr_ptr = com_hdlr_ptr;

	com_hdlr_ptr->next = NULL;

	/* Configure COM parameters */
	com_hdlr_ptr->port_id             = ssp_port;
	com_hdlr_ptr->hooks.pre_cb        = pre_cb;
	com_hdlr_ptr->hooks.pre_cb_param  = pre_cb_param;
	com_hdlr_ptr->hooks.post_cb       = post_cb;
	com_hdlr_ptr->hooks.post_cb_param = post_cb_param;
	com_hdlr_ptr->buf_size            = buf_size;

	/* Configure SSP configuration for specified COM */
	memcpy(&com_hdlr_ptr->config, ssp_config, sizeof(LLD_SSP_ConfigTy));
	com_hdlr_ptr->config.data_size = data_size;

	com_hdlr_ptr->out_clk = out_clk;
	LLD_SSP_ResolveClockFrequency(svc_ssp_handler->bus_speed, out_clk,
				      &effective_out_clk,
				      &com_hdlr_ptr->config.clk_freq);

	xSemaphoreGive(com_hdlr_ptr->access_sem);

	return com_hdlr_ptr;
}

/**
 * @brief Change protocol for given SSP peripheral
 *
 * @param ssp_com_hdlr COM handler pointer
 * @param ssp_mode Interface type
 * @retval 0 if all is ok, else error
 */
int svc_ssp_com_setmode(svc_ssp_com_handler_t *ssp_com_hdlr,
			LLD_SSP_InterfaceTy ssp_mode)
{
	const LLD_SSP_ConfigTy *ssp_config;

	if (ssp_com_hdlr == NULL)
		return -1;

	/* check if SSP mode is supported */
	switch (ssp_mode) {
	case LLD_SSP_INTERFACE_MOTOROLA_SPI:
		ssp_config = &svc_ssp_basecfg_spi;
		break;

	case LLD_SSP_INTERFACE_NATIONAL_MICROWIRE:
		ssp_config = &svc_ssp_basecfg_microwire;
		break;

	default:
		ssp_config = NULL;
		break;
	}

	if (ssp_config == NULL)
		return -1;

	/* Access COM handler */
	xSemaphoreTake(ssp_com_hdlr->access_sem, portMAX_DELAY);

	if (ssp_com_hdlr->config.iface != ssp_mode) {
		/* Save custom setting from config */
		LLD_SSP_DataSizeTy data_size = ssp_com_hdlr->config.data_size;
		LLD_SSP_ClockParamsTy clock_cfg = ssp_com_hdlr->config.clk_freq;

		memcpy(&ssp_com_hdlr->config, ssp_config,
		       sizeof(LLD_SSP_ConfigTy));

		ssp_com_hdlr->config.data_size  = data_size;
		ssp_com_hdlr->config.clk_freq   = clock_cfg;

		/* Reset current port to reconfigure at next write */
		svc_ssp_handler->port_head->curr_com_ptr = NULL;
	}

	xSemaphoreGive(ssp_com_hdlr->access_sem);

	return 0;
}

/**
 * @brief Get protocol for given SSP peripheral
 *
 * @param ssp_com_hdlr COM handler pointer
 * @param ssp_mode_ptr Pointer to interface type
 * @retval 0 if all is ok, else error
 */
int svc_ssp_com_getmode(svc_ssp_com_handler_t *ssp_com_hdlr,
			LLD_SSP_InterfaceTy *ssp_mode_ptr)
{
	if (ssp_com_hdlr == NULL)
		return -1;
	xSemaphoreTake(ssp_com_hdlr->access_sem, portMAX_DELAY);
	*ssp_mode_ptr = ssp_com_hdlr->config.iface;
	xSemaphoreGive(ssp_com_hdlr->access_sem);

	return 0;
}

/**
 * @brief Write buffer on SSP COM
 *
 * @param ssp_com_hdlr COM handler pointer
 * @param out_buf Pointer to buffer to transmit
 * @param len size of data to transfer
 * @param in_buf Pointer to buffer for receiving
 * @retval OS_SUCCESS if all is ok, else error
 */
int svc_ssp_write(svc_ssp_com_handler_t *ssp_com_hdlr,
		  void *out_buf, uint32_t len, void *in_buf)
{
	svc_ssp_port_handler_t *port_hdlr_ptr;
	t_spi *ssp_id;

	if ((len == 0) || (ssp_com_hdlr == NULL))
		return -1;

	if ((ssp_com_hdlr->config.iface == LLD_SSP_INTERFACE_NATIONAL_MICROWIRE)
	    && (len > 2))
		return -1;

	/* Access COM handler */
	xSemaphoreTake(ssp_com_hdlr->access_sem, portMAX_DELAY);

	port_hdlr_ptr = svc_ssp_get_hdlr_ptr(ssp_com_hdlr->port_id);
	ssp_id = port_hdlr_ptr->port_id;

	/* Access Port handler */
	xSemaphoreTake(port_hdlr_ptr->access_sem, portMAX_DELAY);

	/* If previous COM using the port was different, update SSP configuration */
	/*
	if (port_hdlr_ptr->curr_com_ptr != ssp_com_hdlr) {
		LLD_SSP_Reset(ssp_id);
		LLD_SSP_SetConfiguration(ssp_id, &ssp_com_hdlr->config);
	}
	*/

	SSP_CR0_reg_value = ((ssp_com_hdlr->config.data_size & 0x1F) |
			     ((ssp_com_hdlr->config.clk_phase << 7) & 0x80) |
			     ((ssp_com_hdlr->config.clk_pol << 4) & 0x40));

	if (port_hdlr_ptr->curr_com_ptr == NULL) {
		LLD_SSP_Reset(ssp_id);
		LLD_SSP_SetConfiguration(ssp_id, &ssp_com_hdlr->config);
	} else {
		if (port_hdlr_ptr->curr_com_ptr != ssp_com_hdlr) {
			LLD_SSP_SetDataFrameSize(ssp_id,
						 ssp_com_hdlr->config.data_size);
			LLD_SSP_SetClock(ssp_id, &ssp_com_hdlr->config);
		}
	}


	/* Configure transmission on port */
	port_hdlr_ptr->curr_com_ptr   = ssp_com_hdlr;
	port_hdlr_ptr->tx_buf_ptr     = out_buf;
	port_hdlr_ptr->rx_buf_ptr     = in_buf;
	port_hdlr_ptr->len            = len;
	port_hdlr_ptr->tx_pos         = 0;
	port_hdlr_ptr->rx_pos         = 0;

	LLD_SSP_SetClock(ssp_id, &ssp_com_hdlr->config);

	/* Execute callback to be used before transmission */
	if (ssp_com_hdlr->hooks.pre_cb != NULL)
		ssp_com_hdlr->hooks.pre_cb(ssp_com_hdlr->hooks.pre_cb_param);

	/* Enable peripheral */
	LLD_SSP_EnableIRQSrc(ssp_id, LLD_SSP_IRQ_SRC_TRANSMIT |
			     LLD_SSP_IRQ_SRC_RECEIVE |
			     LLD_SSP_IRQ_SRC_RECEIVE_TIMEOUT);
	LLD_SSP_Enable(ssp_id);

	xSemaphoreTake(port_hdlr_ptr->done_sem, portMAX_DELAY);

	/* Execute callback to be used after transmission */
	if (ssp_com_hdlr->hooks.post_cb != NULL)
		ssp_com_hdlr->hooks.post_cb(ssp_com_hdlr->hooks.post_cb_param);

	LLD_SSP_DisableIRQSrc(ssp_id, LLD_SSP_IRQ_SRC_TRANSMIT |
			      LLD_SSP_IRQ_SRC_RECEIVE);

	port_hdlr_ptr->tx_buf_ptr     = NULL;
	port_hdlr_ptr->rx_buf_ptr     = NULL;
	port_hdlr_ptr->len            = 0;
	port_hdlr_ptr->tx_pos         = 0;
	port_hdlr_ptr->rx_pos         = 0;

	xSemaphoreGive(port_hdlr_ptr->access_sem);

	xSemaphoreGive(ssp_com_hdlr->access_sem);

	return 0;
}

void SPI0_IRQHandler(void)
{
    svc_ssp_callback(svc_ssp_get_hdlr_ptr(spi0_regs));
}

void SPI1_IRQHandler(void)
{
    svc_ssp_callback(svc_ssp_get_hdlr_ptr(spi1_regs));
}

void SPI2_IRQHandler(void)
{
    svc_ssp_callback(svc_ssp_get_hdlr_ptr(spi2_regs));
}

