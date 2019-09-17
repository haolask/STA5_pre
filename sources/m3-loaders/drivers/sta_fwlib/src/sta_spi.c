/**
 *  @file     sta_spi.c
 *  @brief    <i><b>Low level driver source of Synchronous Serial Port </b></i>
 *  @author   Fulvio Boggia
 *  @author   (original version) HCL, Luigi Cotignano, Emanuela Zaccaria
 *  @version  1.1
 *  @date     2011.10.18
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "sta_spi.h"
#include "sta_pinmux.h"
#include "sta_nvic.h"
#include "trace.h"

/* Defines depending on SSP Type: Lite or Full */
#define LLD_SSP_PERIPHID0 0x22
#define LLD_SSP_PERIPHID1 0x00
#define LLD_SSP_PERIPHID2 0x08
#define LLD_SSP_PERIPHID3 0x01
#define LLD_SSP_PCELLID0  0x0D
#define LLD_SSP_PCELLID1  0xF0
#define LLD_SSP_PCELLID2  0x05
#define LLD_SSP_PCELLID3  0xB1

/**
 * @brief   Initializes the SSP registers
 *
 * @param   spi address of SSP peripheral
 * @retval  0 if no error
 */
int LLD_SSP_Init(t_spi *spi)
{
	struct nvic_chnl irq_chnl;

	spi->sspcr0.reg   = 0x0000;	/* Clear serial clock rate, clock phase, clock polarity, frame format, data size */
	spi->sspcr1.reg   = 0x0000;	/* Clear Loop back mode, disable SSPid(id), configure as master */
	spi->sspcpsr.reg  = 0x0000;	/* Clear clock prescale register */
	spi->sspimsc.reg  = 0x0000;	/* Mask all interrupts */

	/* Program the GPIO on which SSP ports signals are atached */
	switch ((uint32_t)spi) {
	case SPI1_BASE:
		pinmux_request("spi1_mux");
		irq_chnl.id = EXT6_IRQChannel;
		break;

	case SPI2_BASE:
		pinmux_request("spi2_mux");
		irq_chnl.id = EXT10_IRQChannel;
		break;

	default:
		TRACE_ERR("SPI_Init: invalid SPI port\n");
		return -1;
	}

	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = true;
	nvic_chnl_init(&irq_chnl);

	return 0;
}

/**
 * @brief   Resets SSP and flush the FIFO
 *
 * @param   spi address of SSP peripheral
 * @retval  0 if no error
 */
int LLD_SSP_Reset(t_spi *spi)
{
	spi->sspimsc.reg = ~(LLD_SSP_IMSC_MASK_RORIM | LLD_SSP_IMSC_MASK_RTIM
			     | LLD_SSP_IMSC_MASK_RXIM | LLD_SSP_IMSC_MASK_TXIM);
	spi->sspicr.reg = (LLD_SSP_ICR_MASK_RORIC | LLD_SSP_ICR_MASK_RTIC);
	spi->sspcr0.reg = LLD_SSP_ALLZERO;
	spi->sspcr1.reg = LLD_SSP_ALLZERO;

	/* flush receive fifo */
	return LLD_SSP_FIFOFlush(spi);
}

/**
 * @brief   Calculates the value of effective frequency
 *
 * @param   ssp_freq            Frequency of the clock that feeds SSP HW IP
 * @param   target_freq         Target frequency when SSP is master
 * @param   p_effective_freq    Closest frequency that SSP can effectively achieve
 * @param   p_clock_parameters  Clock parameters corresponding to effectiveFreq
 * @retval  0 if no error
 */
int LLD_SSP_ResolveClockFrequency(uint32_t ssp_freq,
					  uint32_t target_freq,
					  uint32_t *p_effective_freq,
					  LLD_SSP_ClockParamsTy *p_clock_parameters)
{
	int status = 0;
	uint32_t cpsdvsr = 2;
	uint32_t scr = 0;
	bool freq_found = false;
	uint32_t max_tclk;
	uint32_t min_tclk;

	if ((p_effective_freq == NULL) || (p_clock_parameters == NULL))
		return -1;

	/* Calculate effective frequency only in master mode, o.w. return error */
	max_tclk = (ssp_freq / (MIN_CPSDVR * (1 + MIN_SCR ))); /* cpsdvscr = 2 & scr 0 */
	min_tclk = (ssp_freq / (MAX_CPSDVR * (1 + MAX_SCR ))); /* cpsdvsr = 254 & scr = 255 */

	if ( (target_freq <= max_tclk) && (target_freq >= min_tclk)) {
		while( (cpsdvsr <= MAX_CPSDVR) && !freq_found) {
			while( (scr <= MAX_SCR) && !freq_found) {
				if ((ssp_freq / (cpsdvsr * (1 + scr)))
				    > target_freq) {
					scr += 1;
				} else {
					/* This bool is made true when effective frequency >= target frequency is found */
					freq_found = true;
					if ((ssp_freq / (cpsdvsr * (1 + scr)))
					    != target_freq) {
						if ( scr == MIN_SCR) {
							cpsdvsr -= 2;
							scr = MAX_SCR;
						} else {
							scr -= 1;
						}
					}
				}
			}

			if ( freq_found == false) {
				cpsdvsr += 2;
				scr = MIN_SCR;
			}
		}

		if ( cpsdvsr != 0) {
			*p_effective_freq = ssp_freq / (cpsdvsr * (1 + scr));
			p_clock_parameters->cpsdvsr = (uint8_t) (cpsdvsr & LLD_SSP_LAST8BITS);
			p_clock_parameters->scr = (uint8_t) (scr & LLD_SSP_LAST8BITS);
		}
	} else {
		status = -1;
		*p_effective_freq = 0;
	}

	return status;
}

/**
 * @brief   Configures SSP registers
 *
 * @param   spi       address of SSP peripheral
 * @param   p_config  SSP configuration
 * @retval  0 if no error
 */
int LLD_SSP_SetConfiguration(t_spi *spi, LLD_SSP_ConfigTy *p_config)
{
	int status = 0;
	LLD_SSP_EndianessTy endianess;
	uint32_t prescale_dvsr;

	/* Device is enabled, it can NOT be configured */
	if ( (spi->sspcr1.bit.sse) || (p_config == NULL))
		return -1;

	/* Set FRF bit to interface type */
	spi->sspcr0.bit.frf = p_config->iface;

	/* Set MS bit to master/slave hierarchy */
	spi->sspcr1.bit.ms = (uint32_t) p_config->hierarchy;

	/* Set LBM bit, loop back mode */
	spi->sspcr1.bit.lbm = (uint32_t)(p_config->loopback == true) ? 1 : 0;

	/* Set SCR and CPSDVSR bits to clock parameters */
	if (LLD_SSP_IsMaster(spi)) {
		prescale_dvsr = (uint32_t)p_config->clk_freq.cpsdvsr;
		if ((prescale_dvsr % 2) != 0)
			prescale_dvsr = prescale_dvsr - 1; /* make it even */

		spi->sspcpsr.bit.cpsdvsr =  prescale_dvsr;
		spi->sspcr0.bit.scr = p_config->clk_freq.scr;
	}

	/* Set Endianness */
	endianess = p_config->endian;

	if (endianess == LLD_SSP_RX_MSB_TX_MSB) {
		spi->sspcr1.bit.rendn = 0;
		spi->sspcr1.bit.tendn = 0;
	} else if ( endianess == LLD_SSP_RX_MSB_TX_LSB) {
		spi->sspcr1.bit.rendn = 0;
		spi->sspcr1.bit.tendn = 1;
	} else if ( endianess == LLD_SSP_RX_LSB_TX_MSB) {
		spi->sspcr1.bit.rendn = 1;
		spi->sspcr1.bit.tendn = 0;
	} else if ( endianess == LLD_SSP_RX_LSB_TX_LSB) {
		spi->sspcr1.bit.rendn = 1;
		spi->sspcr1.bit.tendn = 1;
	} else {
		status = -1;
	}

	if (status == 0) {
		/* Set Data Frame size */
		spi->sspcr0.bit.dss = p_config->data_size;

		/* Set g_ssp_system_context.tx_com_mode and */
		/* g_ssp_system_context.rx_com_mode to the communication mode */
		spi->sspcr1.bit.rxiflsel = (uint32_t) p_config->rx_lev_trig;
		spi->sspcr1.bit.txiflsel = (uint32_t) p_config->tx_lev_trig;

		/* Set clock phase and polarity */
		if (p_config->iface == LLD_SSP_INTERFACE_MOTOROLA_SPI) {
			spi->sspcr0.bit.spo = p_config->clk_pol;
			spi->sspcr0.bit.sph = p_config->clk_phase;
		}

		if (LLD_SSP_INTERFACE_NATIONAL_MICROWIRE == p_config->iface) {
			spi->sspcr0.bit.css = p_config->ctrl_len;
			spi->sspcr1.bit.mwait = p_config->wait_state;
		}

		/*
		 * Set Half or Full duplex
		 * Full duplex: SSPTXD signal is the output, and SSPRXD is the input.
		 * Configuration used for spi and TI modes.
		 * Half duplex: SSPTXD signal is bidirectional,
		 * driven by the SSP when transmitting a data,
		 * and in input mode when receiving data.
		 */
		spi->sspcr0.bit.halfdup = p_config->duplex;

		/* TO BE VERIFIED. */
		spi->sspcr1.bit.sod = (p_config->slave_tx_disable == true);

		/* Clear Receive timeout and overrun Interrupts */
		spi->sspicr.reg = LLD_SSP_ICR_MASK_RORIC | LLD_SSP_ICR_MASK_RTIC;
		spi->sspimsc.reg = LLD_SSP_DISABLE_ALL_INTERRUPT;
	}

	return status;
}

/**
 * @brief   Gets data from the receive FIFO
 *
 * @param   spi     address of SSP peripheral
 * @param   p_data  pointer where to save data to receive
 * @retval  0 if no error
 */
int LLD_SSP_GetData(t_spi *spi, uint32_t *p_data)
{
	if ( spi->sspsr.bit.rne) {
		*p_data = spi->sspdr.reg;
		return 0;
	}

	return -1;
}

/**
 * @brief   Puts data in the transmit FIFO
 *
 * @param   spi     address of SSP peripheral
 * @param   data    data to transfer
 * @retval  0 if no error
 */
int LLD_SSP_SetData(t_spi *spi, uint32_t data)
{
	if ( spi->sspsr.bit.tnf) {
		spi->sspdr.reg = data;
		return 0;
	}

	return -1;
}

/**
 * @brief   Gets the value of status register
 *
 * @param   spi         address of SSP peripheral
 * @param   status_ptr  pointer where to save status
 * @retval  0 if no error
 */
int LLD_SSP_GetFIFOStatus(t_spi *spi, uint32_t *status_ptr)
{
	if (status_ptr) {
		*status_ptr = spi->sspsr.reg;
		return -1;
	}

	return 0;
}

/**
 * @brief   Flushes content of RX FIFO losing its content
 *
 * @param   spi         address of SSP peripheral
 * @retval  0 if no error
 */
int LLD_SSP_FIFOFlush(t_spi *spi)
{
	while (spi->sspsr.bit.rne)
		spi->sspdr.reg;

	return 0;
}

#ifdef __cplusplus
}
#endif

