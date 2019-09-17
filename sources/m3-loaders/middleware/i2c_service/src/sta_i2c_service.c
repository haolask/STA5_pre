/*
 * sta_i2c_service.c: This file provides all the I2C service functions.
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * Author: APG-MID team
 */

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "sta_map.h"
#include "sta_i2c_service.h"
#include "sta_i2c.h"
#include "sta_mtu.h"
#include "trace.h"

#define   I2C_TXIRQS               (I2C_IRQ_TRANSMIT_FIFO_NEARLY_EMPTY | I2C_IRQ_TRANSMIT_FIFO_NEARLY_EMPTY)
#define   I2C_RXIRQS               (I2C_IRQ_RECEIVE_FIFO_FULL | I2C_IRQ_RECEIVE_FIFO_NEARLY_FULL)

#define   I2C_PORTS                 3

struct i2c_com_handler_s;
struct i2c_port_handler_s;

struct i2c_com_handler_s {
	struct i2c_com_handler_s *next;
	t_i2c *port_id;
	xSemaphoreHandle access_sem;
	struct i2c_config config;
	unsigned addr;
};

typedef enum i2c_operation_e {
	I2C_OPERATION_READ,
	I2C_OPERATION_WRITE
} i2c_operation_t;

struct i2c_port_handler_s {
	struct i2c_port_handler_s *next, *prev;
	t_i2c *port_id;
	struct i2c_com_handler_s *curr_com_ptr;
	xSemaphoreHandle access_sem;
	i2c_operation_t curr_op;
	xSemaphoreHandle done_sem;
	int open_cnt;
	uint8_t *buf_ptr;
	uint16_t buf_pos;
	uint16_t buf_len;
	unsigned slave_addr;
	unsigned mode;
};

struct i2c_handler_s {
	uint32_t bus_speed;
	uint8_t ports;
	struct i2c_com_handler_s *com_head;
	struct i2c_port_handler_s *port_head;
	xSemaphoreHandle open_sem;
};

static struct i2c_config i2c_basecfg = {
	I2C_DGTLFILTER_OFF,
	I2C_STANDARD_MODE,
	8,
	8,
	8,
	0,
	0,
	0,
	I2C_STARTPROC_DISABLED
};

static struct i2c_handler_s *i2c_handler;

/**
 * @brief   Get I2C handler for specific port
 *
 * @param   i2c_port  port of wanted I2C handler
 * @return  os20_i2c_port_handler_t *  pointer to I2C handler, or NULL if not open
 *
 */
static struct i2c_port_handler_s *osal_i2c_get_hdlr_ptr(t_i2c *i2c_port)
{
	struct i2c_port_handler_s *port_hdlr_ptr;

	port_hdlr_ptr = i2c_handler->port_head;

	while (port_hdlr_ptr != NULL) {
		if (port_hdlr_ptr->port_id == i2c_port) {
			return (port_hdlr_ptr);
		} else {
			port_hdlr_ptr = port_hdlr_ptr->next;
		}
	}

	return NULL;
}

/**
 * @brief	I2C service interrupt callback
 *
 * @param	port_hdlr_	ptr Port handler pointer
 * @return	int		error condition
 *
 * */
static int osal_i2c_callback(struct i2c_port_handler_s *port_hdlr_ptr)
{
	uint32_t irq_status;

	irq_status = i2c_get_interrupt_status(port_hdlr_ptr->port_id, I2C_IRQ_ALL);

	if (irq_status & I2C_IRQ_BUS_ERROR) {
		i2c_interrupt_disable(port_hdlr_ptr->port_id, I2C_IRQ_ALL);
		i2c_clear_interrupt(port_hdlr_ptr->port_id, I2C_IRQ_ALL);
	} else if (i2c_get_operating_mode(port_hdlr_ptr->port_id) == I2C_BUSCTRLMODE_MASTER) {
		if (irq_status & I2C_IRQ_MASTER_TRANSACTION_DONE) {
			i2c_interrupt_disable(port_hdlr_ptr->port_id, I2C_IRQ_ALL);

			if (i2c_get_master_mode(port_hdlr_ptr->port_id) == I2C_MSTMODE_READ) {
				port_hdlr_ptr->buf_pos += i2c_read_fifo(port_hdlr_ptr->port_id, &port_hdlr_ptr->buf_ptr[port_hdlr_ptr->buf_pos],
						  port_hdlr_ptr->buf_len - port_hdlr_ptr->buf_pos);
			}

			i2c_clear_interrupt(port_hdlr_ptr->port_id, I2C_IRQ_ALL_CLEARABLE);

			xSemaphoreGiveFromISR(port_hdlr_ptr->done_sem, NULL);
		} else if (port_hdlr_ptr->curr_op == I2C_OPERATION_READ) {
			if (irq_status & I2C_IRQ_MASTER_TRANSACTION_DONE_WS) {
				i2c_interrupt_disable(port_hdlr_ptr->port_id, I2C_IRQ_MASTER_TRANSACTION_DONE_WS);

				i2c_clear_interrupt(port_hdlr_ptr->port_id, I2C_IRQ_MASTER_TRANSACTION_DONE_WS);

				i2c_interrupt_enable(port_hdlr_ptr->port_id, I2C_IRQ_MASTER_TRANSACTION_DONE);
			}
		} else if (port_hdlr_ptr->curr_op == I2C_OPERATION_WRITE) {
			if (irq_status & I2C_TXIRQS) {
				i2c_interrupt_disable(port_hdlr_ptr->port_id, I2C_TXIRQS);

				i2c_clear_interrupt(port_hdlr_ptr->port_id, I2C_TXIRQS & I2C_IRQ_ALL_CLEARABLE);

				port_hdlr_ptr->buf_pos += i2c_write_fifo(port_hdlr_ptr->port_id, &port_hdlr_ptr->buf_ptr[port_hdlr_ptr->buf_pos],
						   port_hdlr_ptr->buf_len - port_hdlr_ptr->buf_pos);

				if (port_hdlr_ptr->buf_pos == port_hdlr_ptr->buf_len) {
					i2c_interrupt_enable(port_hdlr_ptr->port_id, I2C_IRQ_MASTER_TRANSACTION_DONE);
				} else {
					i2c_interrupt_enable(port_hdlr_ptr->port_id, I2C_TXIRQS);
				}
			}
		} else if (port_hdlr_ptr->curr_op == I2C_OPERATION_READ) {
			if (irq_status & I2C_RXIRQS) {
				i2c_interrupt_disable(port_hdlr_ptr->port_id, I2C_RXIRQS);

				i2c_clear_interrupt(port_hdlr_ptr->port_id, I2C_RXIRQS & I2C_IRQ_ALL_CLEARABLE);

				port_hdlr_ptr->buf_pos += i2c_read_fifo(port_hdlr_ptr->port_id, &port_hdlr_ptr->buf_ptr[port_hdlr_ptr->buf_pos],
						  port_hdlr_ptr->buf_len - port_hdlr_ptr->buf_pos);

				if (port_hdlr_ptr->buf_pos == port_hdlr_ptr->buf_len) {
					i2c_interrupt_enable(port_hdlr_ptr->port_id, I2C_IRQ_MASTER_TRANSACTION_DONE);
				} else {
					i2c_interrupt_enable(port_hdlr_ptr->port_id, I2C_RXIRQS);
				}
			}
		}
	}
#ifdef I2C_SLAVE_SUPPORT
	else if (i2c_get_operating_mode(port_hdlr_ptr->port_id) == I2C_BUSCTRLMODE_SLAVE) {
		if (irq_status & I2C_IRQ_SLAVE_TRANSACTION_DONE) {
			i2c_interrupt_disable(port_hdlr_ptr->port_id, I2C_IRQ_ALL);

			if (port_hdlr_ptr->curr_op == I2C_OPERATION_READ) {
				port_hdlr_ptr->buf_pos += i2c_read_fifo(port_hdlr_ptr->port_id, &port_hdlr_ptr->buf_ptr[port_hdlr_ptr->buf_pos],
						  port_hdlr_ptr->buf_len - port_hdlr_ptr->buf_pos);
			}

			i2c_clear_interrupt(port_hdlr_ptr->port_id, I2C_IRQ_ALL_CLEARABLE);

			xSemaphoreGiveFromISR(port_hdlr_ptr->done_sem, NULL);
		} else if (irq_status & I2C_IRQ_WRITE_TO_SLAVE_REQUEST) {
			i2c_interrupt_disable(port_hdlr_ptr->port_id, I2C_IRQ_WRITE_TO_SLAVE_REQUEST);
			i2c_clear_interrupt(port_hdlr_ptr->port_id, I2C_IRQ_WRITE_TO_SLAVE_REQUEST);

			xSemaphoreGiveFromISR(port_hdlr_ptr->done_sem, NULL);
		} else if (irq_status & I2C_IRQ_READ_FROM_SLAVE_REQUEST) {
			i2c_interrupt_disable(port_hdlr_ptr->port_id, I2C_IRQ_READ_FROM_SLAVE_REQUEST);
			i2c_clear_interrupt(port_hdlr_ptr->port_id, I2C_IRQ_READ_FROM_SLAVE_REQUEST);

			port_hdlr_ptr->buf_pos += i2c_write_fifo(port_hdlr_ptr->port_id, &port_hdlr_ptr->buf_ptr[port_hdlr_ptr->buf_pos],
					   port_hdlr_ptr->buf_len - port_hdlr_ptr->buf_pos);

			xSemaphoreGiveFromISR(port_hdlr_ptr->done_sem, NULL);
		}
	}
#endif

	return 0;
}

/**
 * @brief Initialize I2C service
 *
 * @param bus_speed		Bus speed
 * @return				error condition
 *
 */
int i2c_service_init(uint32_t bus_speed)
{
	unsigned ports;

	if (i2c_handler != NULL) {
		return 0;
	}

	i2c_handler = (struct i2c_handler_s *) pvPortMalloc(sizeof(struct i2c_handler_s));
	if (i2c_handler == NULL) {
		return -ENOMEM;
	}

	ports = I2C_PORTS;

	i2c_handler->bus_speed = bus_speed;
	i2c_handler->ports = ports;
	i2c_handler->com_head = NULL;
	i2c_handler->port_head = NULL;

	i2c_basecfg.input_freq = bus_speed;

	i2c_handler->open_sem = xSemaphoreCreateCounting(1, 1);

	return 0;
}

/**
 * @brief Open I2C Port
 *
 * @param i2c_port		Port number (hw specific)
 * @param irq_pri		Interrupt priority of I2C service
 * @param slave_addr	Own address (7 bit)
 * @param start_mode	Initial operating mode (MASTER/SLAVE)
 * @return				error condition
 */
int i2c_open_port(t_i2c *i2c_port, int irq_pri, unsigned slave_addr,
		  uint8_t start_mode)
{
	struct i2c_port_handler_s *port_hdlr_ptr;

	xSemaphoreTake(i2c_handler->open_sem, portMAX_DELAY);
	/* Check if port was already open */
	port_hdlr_ptr = osal_i2c_get_hdlr_ptr(i2c_port);
	if (port_hdlr_ptr) {
		port_hdlr_ptr->open_cnt++;
		xSemaphoreGive(i2c_handler->open_sem);
		return 0;
	}

	port_hdlr_ptr =
		(struct i2c_port_handler_s *)
		pvPortMalloc(sizeof(struct i2c_port_handler_s));

	if (port_hdlr_ptr == NULL) {
		xSemaphoreGive(i2c_handler->open_sem);
		return -ENODEV;
	}

	port_hdlr_ptr->access_sem = xSemaphoreCreateCounting(10, 0);
	port_hdlr_ptr->done_sem = xSemaphoreCreateCounting(10, 0);
	port_hdlr_ptr->open_cnt = 1;

	if ((port_hdlr_ptr->access_sem == NULL)
	    || (port_hdlr_ptr->done_sem == NULL)) {
		vSemaphoreDelete(port_hdlr_ptr->done_sem);
		vSemaphoreDelete(port_hdlr_ptr->access_sem);

		vPortFree(port_hdlr_ptr);
		xSemaphoreGive(i2c_handler->open_sem);

		return -ENODEV;
	}
	port_hdlr_ptr->prev = NULL;
	port_hdlr_ptr->next = i2c_handler->port_head;
	i2c_handler->port_head = port_hdlr_ptr;

	/* Init required HW */
	i2c_init(i2c_port);

	port_hdlr_ptr->port_id = i2c_port;
	port_hdlr_ptr->curr_com_ptr = NULL;
	port_hdlr_ptr->buf_ptr = NULL;
	port_hdlr_ptr->buf_pos = 0;
	port_hdlr_ptr->buf_len = 0;
	port_hdlr_ptr->slave_addr = slave_addr;

	i2c_reset_reg(port_hdlr_ptr->port_id);

	if (start_mode == I2C_BUSCTRLMODE_SLAVE) {
		port_hdlr_ptr->mode = I2C_BUSCTRLMODE_SLAVE;
		i2c_config(port_hdlr_ptr->port_id, &i2c_basecfg);
		i2c_set_slave_addr(port_hdlr_ptr->port_id, slave_addr);
		i2c_enable(port_hdlr_ptr->port_id);
	} else {
		port_hdlr_ptr->mode = I2C_BUSCTRLMODE_MASTER;
	}

	xSemaphoreGive(port_hdlr_ptr->access_sem);
	xSemaphoreGive(i2c_handler->open_sem);

	return 0;
}

/**
 * @brief Create COM on I2C port
 *
 * @param i2c_port		I2C port used
 * @param speed_mode	Baud Rate
 * @param address		Slave Address (7 or 10 bits)
 * @return				COM handler pointer
 *
 */
struct i2c_com_handler_s *i2c_create_com(t_i2c *i2c_port,
				  uint32_t speed_mode, uint16_t address)
{
	struct i2c_com_handler_s *com_hdlr_ptr;
	struct i2c_com_handler_s **last_com_hdlr_ptr_ptr;
	struct i2c_port_handler_s *port_hdlr_ptr;

	/* Check if I2C service was initialized */
	if (i2c_handler == NULL) {
		return NULL;
	}

	/* check if handler for specified port was initialized */
	xSemaphoreTake(i2c_handler->open_sem, portMAX_DELAY);
	port_hdlr_ptr = osal_i2c_get_hdlr_ptr(i2c_port);
	xSemaphoreGive(i2c_handler->open_sem);
	if (port_hdlr_ptr == NULL) {
		return NULL;
	}

	/* allocate memory for COM handler and exit if no space is available */
	com_hdlr_ptr = (struct i2c_com_handler_s *) pvPortMalloc(sizeof(struct i2c_com_handler_s));

	if (com_hdlr_ptr == NULL) {
		return NULL;
	}

	/* create access semaphore and exit if no space is available */
	com_hdlr_ptr->access_sem = xSemaphoreCreateCounting(10, 0);

	if (com_hdlr_ptr->access_sem == NULL) {
		vSemaphoreDelete(com_hdlr_ptr->access_sem);
		return NULL;
	}

	/* Enqueue new COM handler on COM queue */
	last_com_hdlr_ptr_ptr = &i2c_handler->com_head;

	while (*last_com_hdlr_ptr_ptr != NULL) {
		last_com_hdlr_ptr_ptr = &(*last_com_hdlr_ptr_ptr)->next;
	}

	*last_com_hdlr_ptr_ptr = com_hdlr_ptr;

	com_hdlr_ptr->next = NULL;

	/* Configure COM parameters */
	memcpy(&com_hdlr_ptr->config, &i2c_basecfg, sizeof(struct i2c_config));

	com_hdlr_ptr->port_id = i2c_port;
	com_hdlr_ptr->config.speed_mode = speed_mode;
	com_hdlr_ptr->config.slave_address = port_hdlr_ptr->slave_addr;
	com_hdlr_ptr->addr = address;

	xSemaphoreGive(com_hdlr_ptr->access_sem);

	return com_hdlr_ptr;
}

#define I2C_READWRITE_DELAY 50

/**@brief Write buffer on I2C COM
 *
 * @param i2c_com_hdlr    COM handler pointer
 * @param addr_start      peripheral start address
 * @param addr_bytes      number of bytes of start address
 * @param out_buf         pointer to buffer to write
 * @param len             bytes to write
 * @param bytes_in_a_row  bytes to send during a single write operation
 * @param timeout         timeout
 * @return				  error_condition
 *
 */
int i2c_write(struct i2c_com_handler_s * i2c_com_hdlr, uint32_t addr_start, uint32_t addr_bytes,
	      uint8_t * out_buf, uint32_t len, uint32_t bytes_in_a_row, portTickType * timeout)
{
	int error = 0;
	bool per_found = false;
#ifdef I2C_SLAVE_SUPPORT
	uint32_t i2c_id_tmp = NULL;
#endif

	if ((len == 0) || (i2c_com_hdlr == NULL)) {
		return -ENODEV;
	} else {
		struct i2c_port_handler_s *port_hdlr_ptr;
		uint32_t written_bytes = 0;
		uint32_t curr_bytes_to_write;

		/* Access COM handler */
		xSemaphoreTake(i2c_com_hdlr->access_sem, portMAX_DELAY);

		xSemaphoreTake(i2c_handler->open_sem, portMAX_DELAY);
		port_hdlr_ptr = osal_i2c_get_hdlr_ptr(i2c_com_hdlr->port_id);
		xSemaphoreGive(i2c_handler->open_sem);
		if (port_hdlr_ptr == NULL)
			return -ENODEV;

		/* Access Port handler */
		xSemaphoreTake(port_hdlr_ptr->access_sem, portMAX_DELAY);

		if (i2c_get_controller_status(port_hdlr_ptr->port_id) == I2C_CTRLSTATUS_ABORT) {
			return -ECONNABORTED;
		}

		/* If previous COM using the port was different, update I2C configuration */
		if (port_hdlr_ptr->curr_com_ptr != i2c_com_hdlr) {
			i2c_config(port_hdlr_ptr->port_id, &i2c_com_hdlr->config);
			if (port_hdlr_ptr->mode == I2C_BUSCTRLMODE_MASTER) {
				i2c_set_operating_mode(port_hdlr_ptr->port_id, I2C_BUSCTRLMODE_MASTER);
			}
			port_hdlr_ptr->curr_com_ptr = i2c_com_hdlr;
		}

		port_hdlr_ptr->buf_ptr = NULL;
		port_hdlr_ptr->buf_len = 0;
		port_hdlr_ptr->buf_pos = 0;

		/* Loop until all bytes are written or an error occurs */
		while ((written_bytes < len) && (error != -1)) {
			uint8_t status;
			uint8_t abort_cause;
			bool writing = true;

			/* Configure transmission on port */
			if (len - written_bytes > bytes_in_a_row) {
				curr_bytes_to_write = bytes_in_a_row;
			} else {
				curr_bytes_to_write = len - written_bytes;
			}

			port_hdlr_ptr->curr_op = I2C_OPERATION_WRITE;
			port_hdlr_ptr->buf_ptr = &out_buf[written_bytes];
			port_hdlr_ptr->buf_len = curr_bytes_to_write;
			port_hdlr_ptr->buf_pos = 0;

			/*
			   Try to start writing. If a NACK_ADDR occurs at first transmission, returns an error
			   otherwise restart a write after a timeout until no error occurs
			 */

			if (i2c_get_operating_mode(port_hdlr_ptr->port_id) ==
			    I2C_BUSCTRLMODE_MASTER) {
				while (writing == true) {
					port_hdlr_ptr->buf_len = curr_bytes_to_write;
					port_hdlr_ptr->buf_pos = 0;

					i2c_clear_interrupt(port_hdlr_ptr->port_id, I2C_IRQ_ALL_CLEARABLE);

					i2c_set_mcr(port_hdlr_ptr->port_id,
						    I2C_MSTMODE_WRITE,
						    i2c_com_hdlr->addr,
						    I2C_STARTPROC_DISABLED,
						    I2C_STOPCOND_STOP,
						    curr_bytes_to_write +
						    addr_bytes);
					i2c_enable(port_hdlr_ptr->port_id);

					/* Send start address for write operation */
					if (addr_bytes == 4) {
						i2c_write_fifo(port_hdlr_ptr->port_id, &(((uint8_t *)(&addr_start))[3]), 1);
						i2c_write_fifo(port_hdlr_ptr->port_id, &(((uint8_t *)(&addr_start))[2]), 1);
					}
					if (addr_bytes >= 2) {
						i2c_write_fifo(port_hdlr_ptr->port_id, &(((uint8_t *)(&addr_start))[1]), 1);
					}
					if (addr_bytes >= 1) {
						i2c_write_fifo(port_hdlr_ptr->port_id, &(((uint8_t *)(&addr_start))[0]), 1);
					}

					/* Send bytes to FIFO until it is full or no more bytes must be written */
					port_hdlr_ptr->buf_pos +=
					    i2c_write_fifo(port_hdlr_ptr->port_id, &port_hdlr_ptr->buf_ptr[port_hdlr_ptr->buf_pos],port_hdlr_ptr->buf_len - port_hdlr_ptr->buf_pos);

					/* Wait a delay depending on bus speed and check operation status */
					mtu_wait_delay(I2C_READWRITE_DELAY);

					status = i2c_get_controller_status(port_hdlr_ptr->port_id);
					abort_cause = i2c_get_abort_cause(port_hdlr_ptr->port_id);

					/* If, after timeout, status is abort, check the cause */
					if (status == I2C_CTRLSTATUS_ABORT) {
						switch (abort_cause) {
						case I2C_ABORTCAUSE_NACK_ADDR:
							if (per_found == false) {
									/**< peripheral does not answer at all, not connected */
								error = -1;
								writing = false;
								break;
							} else {
									/**< peripheral answered before, so it's busy. Retry. */
								i2c_reset_reg(port_hdlr_ptr->port_id);
								i2c_config(port_hdlr_ptr->port_id, &i2c_com_hdlr->config);
								i2c_flush_tx(port_hdlr_ptr->port_id);
								i2c_set_operating_mode(port_hdlr_ptr->port_id, I2C_BUSCTRLMODE_MASTER);
							}
							break;

						default:
							error = -1;
							writing = false;
							break;
						}
					}
					/* otherwise all is ok and wait for end of write */
					else {
						per_found = true;

						i2c_interrupt_enable(port_hdlr_ptr->port_id,
						     I2C_TXIRQS |
						     I2C_IRQ_BUS_ERROR |
						     I2C_IRQ_MASTER_TRANSACTION_DONE);

						if (xSemaphoreTake(port_hdlr_ptr->done_sem, *timeout) == pdFALSE) {
							/* Semaphore wait has timed out */
							status = i2c_get_controller_status(port_hdlr_ptr->port_id);
							abort_cause = i2c_get_abort_cause(port_hdlr_ptr->port_id);

							if (status == I2C_CTRLSTATUS_ABORT)
							{
								if ((abort_cause == I2C_ABORTCAUSE_BERR_START)
								    || (abort_cause == I2C_ABORTCAUSE_BERR_STOP))
								{
									error = -ECONNABORTED;
								} else {
									/* error not handled, lookup! */
									TRACE_ERR("I2C MASTER WRITE ERROR\n");
									return -EIO;
									while (1) ;
								}
							}
						}
						writing = false;
					}
				}
			}
#ifdef I2C_SLAVE_SUPPORT
			else {
				/* Case Slave: wait to be selected for a read from slave. The callback will write to the fifo */
				i2c_id_tmp = port_hdlr_ptr->port_id;
				i2c_clear_interrupt(port_hdlr_ptr->port_id,
						    I2C_IRQ_ALL_CLEARABLE);
				i2c_interrupt_enable(i2c_id_tmp,
						     I2C_RXIRQS |
						     I2C_IRQ_BUS_ERROR |
						     I2C_IRQ_READ_FROM_SLAVE_REQUEST);
				i2c_enable(port_hdlr_ptr->port_id);

				if (xSemaphoreTake(port_hdlr_ptr->done_sem, *timeout) == -1) {
					status = i2c_get_controller_status(port_hdlr_ptr->port_id);
					abort_cause = i2c_get_abort_cause(port_hdlr_ptr->port_id);

					if (status == I2C_CTRLSTATUS_ABORT) {
						if ((abort_cause == I2C_ABORTCAUSE_BERR_START)
						    || (abort_cause == I2C_ABORTCAUSE_BERR_STOP))
						{
							error = -ECONNABORTED;
							goto write_end;
						} else {
							/* error not handled, lookup! */
							error = -EIO;
							goto write_end;
						}
					} else {
						error = -EIO;
						goto write_end;
					}
				}

				/* Case Slave: wait for transaction completion signaled by slave transaction done */
				i2c_interrupt_enable(i2c_id_tmp,
						     I2C_RXIRQS |
						     I2C_IRQ_BUS_ERROR |
						     I2C_IRQ_SLAVE_TRANSACTION_DONE);
				i2c_enable(port_hdlr_ptr->port_id);

				if (xSemaphoreTake(port_hdlr_ptr->done_sem, *timeout) == -1) {
					status = i2c_get_controller_status
					    (port_hdlr_ptr->port_id);
					abort_cause = i2c_get_abort_cause(port_hdlr_ptr->port_id);

					if (status == I2C_CTRLSTATUS_ABORT) {
						if ((abort_cause == I2C_ABORTCAUSE_BERR_START)
						    || (abort_cause == I2C_ABORTCAUSE_BERR_STOP))
						{
							error = -ECONNABORTED;
							goto write_end;
						} else {
							/* error not handled, lookup! */
							error = -EIO;
							goto write_end;
						}
					} else {
						error = -EIO;
						goto write_end;
					}
				}
			}
#endif
			/* Update write handler for next write */
			written_bytes += curr_bytes_to_write;
			addr_start += bytes_in_a_row;
		}

#ifdef I2C_SLAVE_SUPPORT
write_end:
#endif

		i2c_clear_interrupt(port_hdlr_ptr->port_id, I2C_IRQ_ALL_CLEARABLE);
		i2c_interrupt_disable(port_hdlr_ptr->port_id, I2C_IRQ_ALL);
		i2c_flush_rx(port_hdlr_ptr->port_id);
		i2c_flush_tx(port_hdlr_ptr->port_id);

		xSemaphoreGive(port_hdlr_ptr->access_sem);
		xSemaphoreGive(i2c_com_hdlr->access_sem);

	}

	return error;
}

/**
 * @brief Read buffer on I2C COM
 *
 * @param i2c_com_hdlr  COM handler pointer
 * @param addr_start    peripheral start address
 * @param addr_bytes    number of bytes of start address
 * @param in_buf        pointer to buffer to fill
 * @param len           bytes to receive
 * @param timeout       timeout
 * @return				error_condition
 *
 */
int i2c_read(struct i2c_com_handler_s *i2c_com_hdlr, uint32_t addr_start, uint32_t addr_bytes,
	     uint8_t * in_buf, uint32_t len, portTickType * timeout)
{
	int error = 0;
	t_i2c *i2c_id_tmp = 0;

	if ((len == 0) || (i2c_com_hdlr == NULL)) {
		return -EINVAL;
	} else {
		struct i2c_port_handler_s *port_hdlr_ptr;
#ifdef I2C_SLAVE_SUPPORT
		uint32_t i2c_id;
#endif

		/* Access COM handler */
		xSemaphoreTake(i2c_com_hdlr->access_sem, portMAX_DELAY);

		xSemaphoreTake(i2c_handler->open_sem, portMAX_DELAY);
		port_hdlr_ptr = osal_i2c_get_hdlr_ptr(i2c_com_hdlr->port_id);
		xSemaphoreGive(i2c_handler->open_sem);
		if (port_hdlr_ptr == NULL)
			return -ENODEV;

		/* Access Port handler */
		xSemaphoreTake(port_hdlr_ptr->access_sem, portMAX_DELAY);

		if (i2c_get_controller_status(port_hdlr_ptr->port_id) == I2C_CTRLSTATUS_ABORT) {
			return -ECONNABORTED;
		}

		/* If previous COM using the port was different, update I2C configuration */
		if (port_hdlr_ptr->curr_com_ptr != i2c_com_hdlr) {
			i2c_config(port_hdlr_ptr->port_id,
				   &i2c_com_hdlr->config);

			if (port_hdlr_ptr->mode == I2C_BUSCTRLMODE_MASTER) {
				i2c_set_operating_mode(port_hdlr_ptr->port_id,
						       I2C_BUSCTRLMODE_MASTER);
			}

			port_hdlr_ptr->curr_com_ptr = i2c_com_hdlr;
		}

		port_hdlr_ptr->buf_ptr = NULL;
		port_hdlr_ptr->buf_len = 0;
		port_hdlr_ptr->buf_pos = 0;

		/* Loop until all bytes are read */
		while ((port_hdlr_ptr->buf_len < len) && (error != -1)) {
			uint8_t status;
			uint8_t abort_cause;

			/* Configure transmission on port */
			port_hdlr_ptr->curr_op = I2C_OPERATION_READ;
			port_hdlr_ptr->buf_ptr = &in_buf[0];
			port_hdlr_ptr->buf_len = len;
			port_hdlr_ptr->buf_pos = 0;

			if (i2c_get_operating_mode(port_hdlr_ptr->port_id) ==
			    I2C_BUSCTRLMODE_MASTER) {
				/* Send address if provided */
				if (addr_bytes > 0) {
					i2c_set_mcr(port_hdlr_ptr->port_id,
						    I2C_MSTMODE_WRITE,
						    i2c_com_hdlr->addr,
						    I2C_STARTPROC_DISABLED,
						    I2C_STOPCOND_REPEATEDSTART,
						    addr_bytes);
					i2c_enable(port_hdlr_ptr->port_id);

					if (addr_bytes == 4) {
						i2c_write_fifo(port_hdlr_ptr->port_id, &(((uint8_t*)(&addr_start))[3]), 1);
						i2c_write_fifo(port_hdlr_ptr->port_id, &(((uint8_t*)(&addr_start))[2]), 1);
					}
					if (addr_bytes >= 2) {
						i2c_write_fifo(port_hdlr_ptr->port_id, &(((uint8_t*)(&addr_start))[1]), 1);
					}
					if (addr_bytes >= 1) {
						i2c_write_fifo(port_hdlr_ptr->port_id, &(((uint8_t*)(&addr_start))[0]), 1);
					}

					/* Wait a delay depending on bus speed and check operation status */
					mtu_wait_delay(I2C_READWRITE_DELAY);

					status = i2c_get_controller_status(port_hdlr_ptr->port_id);
					abort_cause = i2c_get_abort_cause(port_hdlr_ptr->port_id);

					if (status == I2C_CTRLSTATUS_ABORT) {
						switch (abort_cause) {
						case I2C_ABORTCAUSE_NACK_ADDR:
							/* peripheral does not answer at all, not ready or not connected */

							error = -ECONNREFUSED;
							break;

						default:
							error = -ECONNABORTED;
							break;
						}
					}

					mtu_wait_delay(I2C_READWRITE_DELAY);
				}

				if (error == 0) {
					/* Case Master: wait for master transaction done */
					i2c_id_tmp = port_hdlr_ptr->port_id;
					i2c_clear_interrupt(port_hdlr_ptr->port_id,
							    I2C_IRQ_ALL_CLEARABLE);
					i2c_interrupt_enable(i2c_id_tmp, I2C_RXIRQS | I2C_IRQ_BUS_ERROR | I2C_IRQ_MASTER_TRANSACTION_DONE_WS);	// | I2C_IRQ_MASTER_TRANSACTION_DONE);

					i2c_set_mcr(port_hdlr_ptr->port_id,
						    I2C_MSTMODE_READ,
						    i2c_com_hdlr->addr,
						    I2C_STARTPROC_DISABLED,
						    I2C_STOPCOND_STOP, len);
					i2c_enable(port_hdlr_ptr->port_id);

					if (xSemaphoreTake(port_hdlr_ptr->done_sem,*timeout)
							== pdFALSE) {
						status = i2c_get_controller_status(port_hdlr_ptr->port_id);
						if (status == I2C_CTRLSTATUS_ABORT)
							error = -ECONNABORTED;
					}
				}
			}
#ifdef I2C_SLAVE_SUPPORT
			else {
				/* Case Slave: wait to be selected for a write to slave */
				i2c_id_tmp = i2c_id;
				i2c_clear_interrupt(port_hdlr_ptr->port_id,
						    I2C_IRQ_ALL_CLEARABLE);
				i2c_interrupt_enable(i2c_id_tmp,
						     I2C_RXIRQS |
						     I2C_IRQ_BUS_ERROR |
						     I2C_IRQ_WRITE_TO_SLAVE_REQUEST);
				i2c_enable(port_hdlr_ptr->port_id);

				if (xSemaphoreTake
				    (port_hdlr_ptr->done_sem, *timeout) == -1) {
					status = i2c_get_controller_status(port_hdlr_ptr->port_id);
					abort_cause = i2c_get_abort_cause(port_hdlr_ptr->port_id);

					if (status == I2C_CTRLSTATUS_ABORT) {
						if ((abort_cause == I2C_ABORTCAUSE_BERR_START)
						    || (abort_cause == I2C_ABORTCAUSE_BERR_STOP))
						{
							error = -1;
							goto read_end;
						} else {
							error = -1;
							goto read_end;
						}
					} else {
						error = -1;
						goto read_end;
					}
				}

				/* Case Slave: wait for completion, signaled by Slave Transaction Done */
				i2c_interrupt_enable(i2c_id_tmp,
						     I2C_RXIRQS |
						     I2C_IRQ_BUS_ERROR |
						     I2C_IRQ_SLAVE_TRANSACTION_DONE);
				i2c_enable(port_hdlr_ptr->port_id);

				if (xSemaphoreTake
				    (port_hdlr_ptr->done_sem, *timeout) == -1) {
					status = i2c_get_controller_status(port_hdlr_ptr->port_id);
					abort_cause = i2c_get_abort_cause(port_hdlr_ptr->port_id);

					if (status == I2C_CTRLSTATUS_ABORT) {
						if ((abort_cause ==
						     I2C_ABORTCAUSE_BERR_START)
						    || (abort_cause ==
							I2C_ABORTCAUSE_BERR_STOP))
						{
							error = -1;
							goto read_end;
						} else {
							error = -1;
							goto read_end;
						}
					} else {
						error = -1;
						goto read_end;
					}
				}
			}
#endif
		}

#ifdef I2C_SLAVE_SUPPORT
read_end:
#endif
		i2c_clear_interrupt(port_hdlr_ptr->port_id,
				    I2C_IRQ_ALL_CLEARABLE);
		i2c_interrupt_disable(port_hdlr_ptr->port_id, I2C_IRQ_ALL);
		i2c_flush_rx(port_hdlr_ptr->port_id);
		i2c_flush_tx(port_hdlr_ptr->port_id);

		xSemaphoreGive(port_hdlr_ptr->access_sem);
		xSemaphoreGive(i2c_com_hdlr->access_sem);
	}

	return error;
}

/**
 * @brief   Change I2C bus mode
 *
 * @param   i2c_port    port to change
 * @param   mode        new I2C mode (MASTER/SLAVE)
 * @return			error_condition
 *
 */
int i2c_set_port_mode(t_i2c *i2c_port, uint8_t mode)
{
	struct i2c_port_handler_s *port_hdlr_ptr;

	xSemaphoreTake(i2c_handler->open_sem, portMAX_DELAY);
	port_hdlr_ptr = osal_i2c_get_hdlr_ptr(i2c_port);
	xSemaphoreGive(i2c_handler->open_sem);
	if (port_hdlr_ptr == NULL)
		return -ENODEV;

	/**< Access Port handler */
	xSemaphoreTake(port_hdlr_ptr->access_sem, portMAX_DELAY);

	if (mode != i2c_get_operating_mode(port_hdlr_ptr->port_id)) {
		i2c_disable(port_hdlr_ptr->port_id);
		i2c_interrupt_disable(port_hdlr_ptr->port_id, I2C_IRQ_ALL);
		i2c_reset_reg(port_hdlr_ptr->port_id);
		i2c_flush_rx(port_hdlr_ptr->port_id);
		i2c_flush_tx(port_hdlr_ptr->port_id);

		if (mode == I2C_BUSCTRLMODE_MASTER) {
			/**< Switch to master mode (default is slave listening) */
			port_hdlr_ptr->mode = I2C_BUSCTRLMODE_MASTER;
			i2c_set_operating_mode(port_hdlr_ptr->port_id,
					       I2C_BUSCTRLMODE_MASTER);
		} else if (mode == I2C_BUSCTRLMODE_SLAVE) {
			/**< switch I2C in slave mode */
			i2c_config(port_hdlr_ptr->port_id, &i2c_basecfg);
			port_hdlr_ptr->mode = I2C_BUSCTRLMODE_SLAVE;
			i2c_set_operating_mode(port_hdlr_ptr->port_id,
					       I2C_BUSCTRLMODE_SLAVE);

			/**< enables interrupts to answer to requests... */
			i2c_enable(port_hdlr_ptr->port_id);
		}
	}

	/**< Release Port handler */
	xSemaphoreGive(port_hdlr_ptr->access_sem);

	return 0;
}

/**
 * @brief   Reset I2C port
 *
 * @param   i2c_port    I2C device to reset
 * @return				error_condition
 *
 */
int i2c_reset_port(t_i2c *i2c_port)
{
	struct i2c_port_handler_s *port_hdlr_ptr;
	uint8_t curr_mode;

	xSemaphoreTake(i2c_handler->open_sem, portMAX_DELAY);
	port_hdlr_ptr = osal_i2c_get_hdlr_ptr(i2c_port);
	if (!port_hdlr_ptr) {
		xSemaphoreGive(i2c_handler->open_sem);
		return -ENODEV;
	}
	port_hdlr_ptr->open_cnt--;
	if (port_hdlr_ptr->open_cnt) {
		xSemaphoreGive(i2c_handler->open_sem);
		return 0;
	}

	curr_mode = i2c_get_operating_mode(port_hdlr_ptr->port_id);

	i2c_disable(port_hdlr_ptr->port_id);
	i2c_interrupt_disable(port_hdlr_ptr->port_id, I2C_IRQ_ALL);
	i2c_reset_reg(port_hdlr_ptr->port_id);
	i2c_flush_rx(port_hdlr_ptr->port_id);
	i2c_flush_tx(port_hdlr_ptr->port_id);

	if (curr_mode == I2C_BUSCTRLMODE_MASTER) {
		/**< Switch to master mode (default is slave listening) */
		port_hdlr_ptr->mode = I2C_BUSCTRLMODE_MASTER;
		i2c_set_operating_mode(port_hdlr_ptr->port_id,
				       I2C_BUSCTRLMODE_MASTER);
	} else if (curr_mode == I2C_BUSCTRLMODE_SLAVE) {
		/**< switch I2C in slave mode */
		i2c_config(port_hdlr_ptr->port_id, &i2c_basecfg);
		i2c_set_slave_addr(port_hdlr_ptr->port_id,
				   port_hdlr_ptr->slave_addr);
		port_hdlr_ptr->mode = I2C_BUSCTRLMODE_SLAVE;
		i2c_set_operating_mode(port_hdlr_ptr->port_id,
				       I2C_BUSCTRLMODE_SLAVE);

		/**< enables interrupts to answer to requests... */
		i2c_enable(port_hdlr_ptr->port_id);
	}

	port_hdlr_ptr->curr_com_ptr = NULL;

	/**< Release Port handler */
	i2c_exit(i2c_port);
	if (port_hdlr_ptr->prev)
		port_hdlr_ptr->prev->next = port_hdlr_ptr->next;
	else
		i2c_handler->port_head = port_hdlr_ptr->next;
	if (port_hdlr_ptr->next)
		port_hdlr_ptr->next->prev = port_hdlr_ptr->prev;

	vSemaphoreDelete(port_hdlr_ptr->done_sem);
	vSemaphoreDelete(port_hdlr_ptr->access_sem);
	vPortFree(port_hdlr_ptr);

	xSemaphoreGive(i2c_handler->open_sem);

	return 0;
}

void I2C0_IRQHandler(void)
{
	osal_i2c_callback(osal_i2c_get_hdlr_ptr(i2c0_regs));
	return;
}

void I2C1_IRQHandler(void)
{
	osal_i2c_callback(osal_i2c_get_hdlr_ptr(i2c1_regs));
	return;
}

void I2C2_IRQHandler(void)
{
	osal_i2c_callback(osal_i2c_get_hdlr_ptr(i2c2_regs));
	return;
}
