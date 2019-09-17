/**
 * @file sta_msp.c
 * @brief This file provides all the MSP firmware functions
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */

#include "utils.h"


#include "sta_msp.h"

#define MSP_MAX_PERIPH	4

/* TCF register */
#define TP1ELEN_MASK	0x3
#define TP1FLEN_MASK	0x7F
#define TDTYP_MASK		0x3
#define TDDLY_MASK		0x3
#define TP2ELEN_MASK	0x3
#define TP2FLEN_MASK	0x7F

/* RCF register */
#define RP1ELEN_MASK	0x3
#define RP1FLEN_MASK	0x7F
#define RDTYP_MASK		0x3
#define RDDLY_MASK		0x3
#define RP2ELEN_MASK	0x3
#define RP2FLEN_MASK	0x7F
#define RBSWAP_MASK		0x3

/**
 * @brief	Reset registers values
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_reset_reg(t_msp *msp)
{
	int i;

	msp->msp_cgr.reg = MSP_REGISTER_RESET_VALUE;
	msp->msp_rcf.reg = MSP_REGISTER_RESET_VALUE;
	msp->msp_srg.reg = MSP_REGISTER_RESET_VALUE;

	msp->msp_mcr.reg = MSP_REGISTER_RESET_VALUE;
	msp->msp_rcv = MSP_REGISTER_RESET_VALUE;
	msp->msp_rcm = MSP_REGISTER_RESET_VALUE;

	for (i = 0; i < MSP_MAX_PERIPH; i++) {
		msp->msp_tce[i] = MSP_REGISTER_RESET_VALUE;
		msp->msp_rce[i] = MSP_REGISTER_RESET_VALUE;
	}

	msp->msp_dmacr.reg = MSP_REGISTER_RESET_VALUE;
	msp->msp_imsc.reg = MSP_REGISTER_RESET_VALUE;
	msp->msp_icr.reg = MSP_IRQ_ALL_CLEARABLE;
}

/**
 * @brief	Configure MSP device
 * @param	msp: MSP register base address
 * @param	cfg: MSP configuration
 * @param	prot_desc: MSP protocol descriptor
 * @return	none
 */
void msp_configure(t_msp *msp, struct msp_config *cfg,
		   struct msp_protocol *prot_desc)
{
	msp_set_tx_clock(msp, cfg->tx_clock_sel);
	msp_set_rx_clock(msp, cfg->rx_clock_sel);
	msp_set_tx_unexp_frame_sync_mode(msp, cfg->tx_unexpect_frame_sync);
	msp_set_rx_unexp_frame_sync_mode(msp, cfg->rx_unexpect_frame_sync);
	msp_set_tx_frame_syncro_selection(msp, cfg->tx_frame_sync_sel);
	msp_set_rx_frame_syncro_selection(msp, cfg->rx_frame_sync_sel);
	msp_set_tx_fifo_mode(msp, cfg->tx_fifo_config);
	msp_set_rx_fifo_mode(msp, cfg->rx_fifo_config);
	msp_set_tx_dma_mode(msp, cfg->tx_msp_dma_mode);
	msp_set_rx_dma_mode(msp, cfg->rx_msp_dma_mode);

	msp_set_sample_rate_gen_clock(msp, cfg->srg_clock_sel);
	msp_set_dir_comp_mode(msp, cfg->msp_direct_companding_mode);
	msp_set_loop_back_mode(msp, cfg->msp_loopback_mode);
	msp_set_tx_data_extra_delay_mode(msp, cfg->tx_extra_delay);

	msp_set_rx2_ph(msp, prot_desc->rx_phase_mode);
	msp_set_rx_start_mode2_ph(msp, prot_desc->rx_phase2_start_mode);
	msp_set_rx_elem_numb1_ph(msp, prot_desc->rx_frame_length_1);
	msp_set_rx_elem_numb2_ph(msp, prot_desc->rx_frame_length_2);
	msp_set_rx_elem_length1_ph(msp, prot_desc->rx_element_length_1);
	msp_set_rx_elem_length2_ph(msp, prot_desc->rx_element_length_2);
	msp_set_rx_data_delay(msp, prot_desc->rx_data_delay);
	msp_set_rx_clock_polarity(msp, prot_desc->rx_clock_pol);

	msp_set_tx2_ph(msp, prot_desc->tx_phase_mode);
	msp_set_tx_start_mode2_ph(msp, prot_desc->tx_phase2_start_mode);
	msp_set_tx_elem_numb1_ph(msp, prot_desc->tx_frame_length_1);
	msp_set_tx_elem_numb2_ph(msp, prot_desc->tx_frame_length_2);
	msp_set_tx_elem_length1_ph(msp, prot_desc->tx_element_length_1);
	msp_set_tx_elem_length2_ph(msp, prot_desc->tx_element_length_2);
	msp_set_tx_data_delay(msp, prot_desc->tx_data_delay);
	msp_set_tx_clock_polarity(msp, prot_desc->tx_clock_pol);

	msp_set_rx_endian_form(msp, prot_desc->rx_bit_transfer_format);
	msp_set_tx_endian_form(msp, prot_desc->tx_bit_transfer_format);

	msp_set_rx_frame_syncro_polarity(msp, prot_desc->rx_frame_sync_pol);
	msp_set_tx_frame_syncro_polarity(msp, prot_desc->tx_frame_sync_pol);

	msp_set_rx_half_word_swap(msp, prot_desc->rx_half_word_swap);
	msp_set_tx_half_word_swap(msp, prot_desc->tx_half_word_swap);

	msp_set_spi_clock_mode(msp, prot_desc->spi_clk_mode);
	msp_set_spi_burst_mode(msp, prot_desc->spi_burst_mode);
	msp_set_tx_data_type(msp, prot_desc->compression_mode);
	msp_set_rx_data_type(msp, prot_desc->expansion_mode);
}

/**
 * @brief	Write data in DATA register
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_write_data(t_msp *msp, uint32_t data)
{
	msp->msp_dr = data;
}

/**
 * @brief	Read data from DATA register
 * @param	msp: MSP register base address
 * @return	DATA register content
 */
uint32_t msp_read_data(t_msp *msp)
{
	return msp->msp_dr;
}

/**
 * @brief	Configure the Rx FIFO mode (enabled/disabled)
 * @param	msp: MSP register base address
 * @param	mode: MSP_FIFO_DISABLE or MSP_FIFO_ENABLE
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_rx_fifo_mode(t_msp *msp, int mode)
{
	msp->msp_cgr.bit.rffen = !!mode;
}

/**
 * @brief	Provide the Rx FIFO mode (enabled/disabled)
 * @param	msp: MSP register base address
 * @return	Rx fifo mode
 */
int msp_get_rx_fifo_mode(t_msp *msp)
{
	return msp->msp_cgr.bit.rffen;
}

/**
 * @brief	Set RX frame synchro polarity
 * @param	msp: MSP register base address
 * @param	polarity: chosen polarity
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_rx_frame_syncro_polarity(t_msp *msp,
				  int polarity)
{
	msp->msp_cgr.bit.rfspol = !!polarity;
}

/**
 * @brief	Provide RX frame synchro polarity
 * @param	msp: MSP register base address
 * @return	polarity: chosen polarity
 */
int msp_get_rx_frame_syncro_polarity(t_msp *msp)
{
	return msp->msp_cgr.bit.rfspol;
}

/**
 * @brief	Set Direct Companding Mode
 * @param	msp: MSP register base address
 * @param	mode: chosen mode
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_dir_comp_mode(t_msp *msp, int mode)
{
	msp->msp_cgr.bit.dcm = mode;
}

/**
 * @brief	Get Direct Companding Mode
 * @param	msp: MSP register base address
 * @return	mode: chosen mode
 */
int msp_get_dir_comp_mode(t_msp *msp)
{
	return msp->msp_cgr.bit.dcm;
}

/**
 * @brief	Set RX frame synchro selection
 * @param	msp: MSP register base address
 * @param	selection: external or internal source
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_rx_frame_syncro_selection(t_msp *msp, int selection)
{
	msp->msp_cgr.bit.rfssel = !!selection;
}

/**
 * @brief	Get RX frame synchro selection
 * @param	msp: MSP register base address
 * @return	selection mode (see Tx for details)
 */
int msp_get_rx_frame_syncro_selection(t_msp *msp)
{
	return msp->msp_cgr.bit.rfssel << 1;
}

/**
 * @brief	Set RX clock polarity
 * @param	msp: MSP register base address
 * @param	polarity: chosen polarity
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_rx_clock_polarity(t_msp *msp, int polarity)
{
	msp->msp_cgr.bit.rckpol = (~polarity & 1);
}

/**
 * @brief	Get RX clock polarity
 * @param	msp: MSP register base address
 * @return	polarity: chosen polarity
 */
int msp_get_rx_clock_polarity(t_msp *msp)
{
	return (~msp->msp_cgr.bit.rckpol & 1);
}

/**
 * @brief	Set RX clock
 * @param	msp: MSP register base address
 * @param	clock: clock value
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_rx_clock(t_msp *msp, int clock)
{
	msp->msp_cgr.bit.rcksel = !!clock;
}

/**
 * @brief	Get RX clock
 * @param	msp: MSP register base address
 * @return	clock: clock value
 */
int msp_get_rx_clock(t_msp *msp)
{
	return msp->msp_cgr.bit.rcksel;
}

/**
 * @brief	Set Loopback mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_loop_back_mode(t_msp *msp, int mode)
{
	msp->msp_cgr.bit.lbm = !!mode;
}

/**
 * @brief	Get Loopback mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_loop_back_mode(t_msp *msp)
{
	return msp->msp_cgr.bit.lbm;
}

/**
 * @brief	Configure the Tx FIFO mode (enabled/disabled)
 * @param	msp: MSP register base address
 * @param	mode: MSP_FIFO_DISABLE or MSP_FIFO_ENABLE
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_tx_fifo_mode(t_msp *msp, int mode)
{
	msp->msp_cgr.bit.tffen = !!mode;
}

/**
 * @brief	Provide the Tx FIFO mode (enabled/disabled)
 * @param	msp: MSP register base address
 * @return	Rx fifo mode
 */
int msp_get_tx_fifo_mode(t_msp *msp)
{
	return msp->msp_cgr.bit.tffen;
}

/**
 * @brief	Set TX frame synchro polarity
 * @param	msp: MSP register base address
 * @param	polarity: chosen polarity
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_tx_frame_syncro_polarity(t_msp *msp,
				  int polarity)
{
	msp->msp_cgr.bit.tfspol = !!polarity;
}

/**
 * @brief	Provide TX frame synchro polarity
 * @param	msp: MSP register base address
 * @return	polarity: chosen polarity
 */
int msp_get_tx_frame_syncro_polarity(t_msp *msp)
{
	return msp->msp_cgr.bit.tfspol;
}

/**
 * @brief	Set TX frame synchro selection
 * @param	msp: MSP register base address
 * @param	selection: external or internal source
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_tx_frame_syncro_selection(t_msp *msp, int selection)
{
	msp->msp_cgr.bit.tfssel = selection;
}

/**
 * @brief	Get TX frame synchro selection
 * @param	msp: MSP register base address
 * @return	selection mode (see Tx for details)
 */
int msp_get_tx_frame_syncro_selection(t_msp *msp)
{
	return msp->msp_cgr.bit.tfssel << 1;
}

/**
 * @brief	Set TX clock polarity
 * @param	msp: MSP register base address
 * @param	polarity: chosen polarity
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_tx_clock_polarity(t_msp *msp, int polarity)
{
	msp->msp_cgr.bit.tckpol = !!polarity;
}

/**
 * @brief	Get TX clock polarity
 * @param	msp: MSP register base address
 * @return	polarity: chosen polarity
 */
int msp_get_tx_clock_polarity(t_msp *msp)
{
	return msp->msp_cgr.bit.tckpol;
}

/**
 * @brief	Set TX clock
 * @param	msp: MSP register base address
 * @param	clock: clock value
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_tx_clock(t_msp *msp, int clock)
{
	msp->msp_cgr.bit.tcksel = clock;
}

/**
 * @brief	Get TX clock
 * @param	msp: MSP register base address
 * @return	clock: clock value
 */
int msp_get_tx_clock(t_msp *msp)
{
	return msp->msp_cgr.bit.tcksel;
}

/**
 * @brief	Set TX Extra delay mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_tx_data_extra_delay_mode(t_msp *msp, int mode)
{
	msp->msp_cgr.bit.txddl = !!mode;
}

/**
 * @brief	Get TX Extra delay mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_tx_data_extra_delay_mode(t_msp *msp)
{
	return msp->msp_cgr.bit.txddl;
}

/**
 * @brief	Disable sampling rate generation
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_dis_sample_rate_gen(t_msp *msp)
{
	msp->msp_cgr.bit.sgen = 0;
}

/**
 * @brief	Enable sampling rate generation
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_en_sample_rate_gen(t_msp *msp)
{
	uint32_t dummy;

	msp->msp_cgr.bit.sgen = 1;

	/* wait 2 serial clock cycles */
	{
		uint8_t delay;
		for (delay = 0; delay < 2; delay++) ;
		dummy = msp->msp_srg.bit.frwid;
	}

	(void) dummy;
}

/**
 * @brief	Set sample generator Clock Polarity
 * @param	msp: MSP register base address
 * @param	polarity: chosen polarity
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_sample_rate_gen_clock_polarity(t_msp *msp,
				       int polarity)
{
	msp->msp_cgr.bit.sckpol = !!polarity;
}

/**
 * @brief	Get sample generator clock polarity
 * @param	msp: MSP register base address
 * @return	polarity: chosen polarity
 */
int msp_get_sample_rate_gen_clock_polarity(t_msp *msp)
{
	return msp->msp_cgr.bit.sckpol;
}


/**
 * @brief	Set sample generator clock
 * @param	msp: MSP register base address
 * @param	clock: clock value
 * @return	0 if no error, not 0 otherwise
 */
void msp_set_sample_rate_gen_clock(t_msp *msp, int clock)
{
	msp->msp_cgr.bit.scksel = clock;
}

/**
 * @brief	Get sample generator clock
 * @param	msp: MSP register base address
 * @return	clock: clock value
 */
int msp_get_sample_rate_gen_clock(t_msp *msp)
{
	return msp->msp_cgr.bit.scksel;
}

/**
 * @brief	Disable frame generation
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_dis_frame_gen(t_msp *msp)
{
	msp->msp_cgr.bit.fgen = 0;
}

/**
 * @brief	Enable frame generation
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_en_frame_gen(t_msp *msp)
{
	msp->msp_cgr.bit.fgen = 1;
}

/**
 * @brief	Set SPI clock mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 * @return	none
 */
void msp_set_spi_clock_mode(t_msp *msp, int mode)
{
	msp->msp_cgr.bit.spickml = mode;

	/* wait 2 serial clock cycles */
	{
		volatile uint32_t dummy;
		uint8_t delay;

		for (delay = 0; delay < 2; delay++);

		dummy = msp->msp_srg.bit.frwid;

		(void) dummy;
	}
}

/**
 * @brief	Get SPI clock mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_spi_clock_mode(t_msp *msp)
{
	return msp->msp_srg.bit.frwid;
}

/**
 * @brief	Set SPI burst mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 * @return	none
 */
void msp_set_spi_burst_mode(t_msp *msp, int mode)
{
	msp->msp_cgr.bit.spibme = !!mode;
}

/**
 * @brief	Get SPI burst mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_spi_burst_mode(t_msp *msp)
{
	return msp->msp_cgr.bit.spibme;
}

/**
 * @brief	Set Tx Element Length phase1
 * @param	msp: MSP register base address
 * @param	length: length value
 */
void msp_set_tx_elem_length1_ph(t_msp *msp, int length)
{
	msp->msp_tcf.bit.tp1elen = length & TP1ELEN_MASK;
}

/**
 * @brief	Get Tx Element Length phase1
 * @param	msp: MSP register base address
 * @return	length: length value
 */
int msp_get_tx_elem_length1_ph(t_msp *msp)
{
	return (msp->msp_tcf.bit.tp1elen & TP1ELEN_MASK);
}

/**
 * @brief	Set Tx Element Number phase1
 * @param	msp: MSP register base address
 * @param	amount: amount value
 */
void msp_set_tx_elem_numb1_ph(t_msp *msp, uint8_t amount)
{
	msp->msp_tcf.bit.tp1flen = ((amount - 1) & TP1FLEN_MASK);
}

/**
 * @brief	Get Tx Element Length phase1
 * @param	msp: MSP register base address
 * @return	amount: amount value
 */
uint8_t msp_get_tx_elem_numb1_ph(t_msp *msp)
{
	return ((msp->msp_tcf.bit.tp1flen & TP1FLEN_MASK) + 1);
}

/**
 * @brief	Set Tx data type
 * @param	msp: MSP register base address
 */
void msp_set_tx_data_type(t_msp *msp, int type)
{
	msp->msp_tcf.bit.tdtyp = (type & TDTYP_MASK);
}

/**
 * @brief	Get Tx data type
 * @param	msp: MSP register base address
 * @return	type: type value
 */
int msp_get_tx_data_type(t_msp *msp)
{
	return (msp->msp_tcf.bit.tdtyp & TDTYP_MASK);
}

/**
 * @brief	Set Tx bit endian format
 * @param	msp: MSP register base address
 * @param	type: type value
 */
void msp_set_tx_endian_form(t_msp *msp, int type)
{
	msp->msp_tcf.bit.tendn = !!type;
}

/**
 * @brief	Get Tx bit endian format
 * @param	msp: MSP register base address
 * @return	type: type value
 */
int msp_get_tx_endian_form(t_msp *msp)
{
	return msp->msp_tcf.bit.tendn;
}

/**
 * @brief	Set Tx data delay
 * @param	msp: MSP register base address
 * @param	type: type value
 */
void msp_set_tx_data_delay(t_msp *msp, int delay)
{
	msp->msp_tcf.bit.tddly = (delay & TDDLY_MASK);
}

/**
 * @brief	Get Tx data delay
 * @param	msp: MSP register base address
 * @return	type: type value
 */
int msp_get_tx_data_delay(t_msp *msp)
{
	return (msp->msp_tcf.bit.tddly & TDDLY_MASK);
}

/**
 * @brief	Set Tx unexpected frame bit endian format
 * @param	msp: MSP register base address
 * @param	mode: mode value
 */
void msp_set_tx_unexp_frame_sync_mode(t_msp *msp,
				 int mode)
{
	msp->msp_tcf.bit.tfsig = !!mode;
}

/**
 * @brief	Get Tx unexpected frame bit endian format
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_tx_unexp_frame_sync_mode(t_msp *msp)
{
	return msp->msp_tcf.bit.tfsig;
}

/**
 * @brief	Set Tx Element Length phase2
 * @param	msp: MSP register base address
 * @param	length: length value
 */
void msp_set_tx_elem_length2_ph(t_msp *msp, int length)
{
	msp->msp_tcf.bit.tp2elen = length & TP2ELEN_MASK;
}

/**
 * @brief	Get Tx Element Length phase2
 * @param	msp: MSP register base address
 * @return	length: length value
 */
int msp_get_tx_elem_length2_ph(t_msp *msp)
{
	return msp->msp_tcf.bit.tp2elen & TP2ELEN_MASK;
}

/**
 * @brief	Set Tx Element Number phase2
 * @param	msp: MSP register base address
 * @param	amount: amount value
 */
void msp_set_tx_elem_numb2_ph(t_msp *msp, uint8_t amount)
{
	msp->msp_tcf.bit.tp2flen = ((amount - 1) & TP2FLEN_MASK);
}

/**
 * @brief	Get Tx Element Length phase2
 * @param	msp: MSP register base address
 * @return	amount: amount value
 */
uint8_t msp_get_tx_elem_numb2_ph(t_msp *msp)
{
	return (msp->msp_tcf.bit.tp2flen & TP2FLEN_MASK) + 1;
}

/**
 * @brief	Set Tx Start Mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 */
void msp_set_tx_start_mode2_ph(t_msp *msp, int mode)
{
	msp->msp_tcf.bit.tp2sm = !!mode;
}

/**
 * @brief	Get Tx Start Mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_tx_start_mode2_ph(t_msp *msp)
{
	return msp->msp_tcf.bit.tp2sm;
}

/**
 * @brief	Set Tx phase Mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 */
void msp_set_tx2_ph(t_msp *msp, int mode)
{
	msp->msp_tcf.bit.tp2en = !!mode;
}

/**
 * @brief	Get Tx phase Mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_tx2_ph(t_msp *msp)
{
	return msp->msp_tcf.bit.tp2en;
}

/**
 * @brief	Set Tx half word swap
 * @param	msp: MSP register base address
 * @param	swap: swap value
 */
void msp_set_tx_half_word_swap(t_msp *msp, int swap)
{
	msp->msp_tcf.bit.tbswap = !!swap;
}

/**
 * @brief	Get Tx half word swap
 * @param	msp: MSP register base address
 * @return	swap: swap value
 */
int msp_get_tx_half_word_swap(t_msp *msp)
{
	return msp->msp_tcf.bit.tbswap;
}

/**
 * @brief	Set Rx Element Length phase1
 * @param	msp: MSP register base address
 * @param	length: length value
 */
void msp_set_rx_elem_length1_ph(t_msp *msp, int length)
{
	msp->msp_rcf.bit.rp1elen = length;
}

/**
 * @brief	Get Tx Element Length phase1
 * @param	msp: MSP register base address
 * @return	length: length value
 */
int msp_get_rx_elem_length1_ph(t_msp *msp)
{
	return (msp->msp_rcf.bit.rp1elen & RP1ELEN_MASK);
}

/**
 * @brief	Set Rx Element Number phase1
 * @param	msp: MSP register base address
 * @param	amount: amount value
 */
void msp_set_rx_elem_numb1_ph(t_msp *msp, uint8_t amount)
{
	msp->msp_rcf.bit.rp1flen = ((amount - 1) & RP1FLEN_MASK);
}

/**
 * @brief	Get Rx Element Length phase1
 * @param	msp: MSP register base address
 * @return	amount: amount value
 */
uint8_t msp_get_rx_elem_numb1_ph(t_msp *msp)
{
	return msp->msp_rcf.bit.rp1flen + 1;
}

/**
 * @brief	Set Rx data type
 * @param	msp: MSP register base address
 * @return	type: type value
 */
void msp_set_rx_data_type(t_msp *msp, int type)
{
	msp->msp_rcf.bit.rdtyp = type & RDTYP_MASK;
}

/**
 * @brief	Get Rx data type
 * @param	msp: MSP register base address
 * @return	type: type value
 */
int msp_get_rx_data_type(t_msp *msp)
{
	return msp->msp_rcf.bit.rdtyp;
}

/**
 * @brief	Set Rx bit endian format
 * @param	msp: MSP register base address
 * @param	type: type value
 */
void msp_set_rx_endian_form(t_msp *msp, int type)
{
	msp->msp_rcf.bit.rendn = !!type;
}

/**
 * @brief	Get Rx bit endian format
 * @param	msp: MSP register base address
 * @return	type: type value
 */
int msp_get_rx_endian_form(t_msp *msp)
{
	return msp->msp_rcf.bit.rendn;

}

/**
 * @brief	Set Rx data delay
 * @param	msp: MSP register base address
 * @param	type: type value
 */
void msp_set_rx_data_delay(t_msp *msp, int delay)
{
	msp->msp_rcf.bit.rddly = delay & RDDLY_MASK;
}

/**
 * @brief	Get Rx data delay
 * @param	msp: MSP register base address
 * @return	type: type value
 */
int msp_get_rx_data_delay(t_msp *msp)
{
	return (msp->msp_rcf.bit.rddly & RDDLY_MASK);
}

/**
 * @brief	Set Rx unexpected frame bit endian format
 * @param	msp: MSP register base address
 * @param	mode: mode value
 */
void msp_set_rx_unexp_frame_sync_mode(t_msp *msp,
				 int mode)
{
	msp->msp_rcf.bit.rfsig = !!mode;
}

/**
 * @brief	Get Rx unexpected frame bit endian format
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_rx_unexp_frame_sync_mode(t_msp *msp)
{
	return msp->msp_rcf.bit.rfsig;
}

/**
 * @brief	Set Rx Element Length phase2
 * @param	msp: MSP register base address
 * @param	length: length value
 */
void msp_set_rx_elem_length2_ph(t_msp *msp, int length)
{
	msp->msp_rcf.bit.rp2elen = length & RP2ELEN_MASK;
}

/**
 * @brief	Get Rx Element Length phase2
 * @param	msp: MSP register base address
 * @return	length: length value
 */
int msp_get_rx_elem_length2_ph(t_msp *msp)
{
	return (msp->msp_rcf.bit.rp2elen & RP2ELEN_MASK);
}

/**
 * @brief	Set Rx Element Number phase2
 * @param	msp: MSP register base address
 * @param	amount: amount value
 */
void msp_set_rx_elem_numb2_ph(t_msp *msp, uint8_t amount)
{
	msp->msp_rcf.bit.rp2flen = (amount - 1) & RP2FLEN_MASK;
}

/**
 * @brief	Get Tx Element Length phase2
 * @param	msp: MSP register base address
 * @return	amount: amount value
 */
uint8_t msp_get_rx_elem_numb2_ph(t_msp *msp)
{
	return (msp->msp_rcf.bit.rp2flen + 1);
}

/**
 * @brief	Set Rx Start Mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 */
void msp_set_rx_start_mode2_ph(t_msp *msp, int mode)
{
	msp->msp_rcf.bit.rp2sm = !!mode;
}

/**
 * @brief	Get Rx Start Mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_rx_start_mode2_ph(t_msp *msp)
{
	return msp->msp_rcf.bit.rp2sm;
}

/**
 * @brief	Set Rx phase Mode
 * @param	msp: MSP register base address
 * @param	mode: mode value
 */
void msp_set_rx2_ph(t_msp *msp, int mode)
{
	msp->msp_rcf.bit.rp2en = !!mode;
}

/**
 * @brief	Get Rx phase Mode
 * @param	msp: MSP register base address
 * @return	mode: mode value
 */
int msp_get_rx2_ph(t_msp *msp)
{
	return (msp->msp_rcf.bit.rp2en);
}

/**
 * @brief	Set Rx half word swap
 * @param	msp: MSP register base address
 * @param	swap: swap value
 */
void msp_set_rx_half_word_swap(t_msp *msp, int swap)
{
	msp->msp_rcf.bit.rbswap = swap & RBSWAP_MASK;
}

/**
 * @brief	Get Rx half word swap
 * @param	msp: MSP register base address
 * @return	swap: swap value
 */
int msp_get_rx_half_word_swap(t_msp *msp)
{
	return msp->msp_rcf.bit.rbswap;
}

/**
 * @brief	Configure sample rate generator
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_config_sample_rate_gen(t_msp *msp, uint32_t clkfrq, uint32_t frmfrq,
			     uint32_t dataclk, uint8_t width)
{
	uint16_t divisor;

	if ((dataclk - 1) < 8192)
		msp->msp_srg.bit.frper = dataclk - 1;
	else
		return;

	divisor = (clkfrq / (frmfrq * dataclk)) - 1;

	if (divisor < 1024)
		msp->msp_srg.bit.sckdiv = divisor;

	if (width > 0 && width <= 64)
		msp->msp_srg.bit.frwid = width - 1;
	/* wait 2 serial clock cycles */
	{
		volatile uint32_t dummy;
		uint8_t delay;

		for (delay = 0; delay < 2; delay++) ;

		dummy = msp->msp_srg.bit.frwid;

		(void) dummy;
	}
}

/**
 * @brief	Get sample freq
 * @param	msp: MSP register base address
 * @return	none
 */
void msp_get_sample_freq(t_msp *msp, uint32_t clkfrq, uint32_t *smplfrq,
		       uint32_t *period)
{
	*period = msp->msp_srg.bit.frper + 1;
	*smplfrq = clkfrq/((msp->msp_srg.bit.sckdiv + 1)*(*period));
}

/**
 * @brief	Set active frame width
 * @param	msp: MSP register base address
 * @param	frame width
 */
void msp_set_active_frame_width(t_msp *msp, uint8_t width)
{
	if (width > 0 && width <= 64)
		msp->msp_srg.bit.frwid = width - 1;
}

/**
 * @brief	Get active frame width
 * @param	msp: MSP register base address
 * @return	frame width
 */
uint8_t msp_get_frame_width(t_msp *msp)
{
	return (msp->msp_srg.bit.frwid + 1);
}

/**
 * @brief	Set frame period
 * @param	msp: MSP register base address
 * @param	period
 */
void msp_set_frame_period(t_msp *msp, uint16_t period)
{
	if (period > 0 && period <= 8192)
		msp->msp_srg.bit.frper = period-1;
}

/**
 * @brief	Get frame period
 * @param	msp: MSP register base address
 * @return	period
 */
uint16_t msp_get_frame_period(t_msp *msp)
{
	return (msp->msp_srg.bit.frper + 1);
}

/**
 * @brief	Get flag status
 * @param	msp: MSP register base address
 * @param	flag
 * @return	flag status
 */
bool msp_get_flag_status(t_msp *msp, uint32_t flag)
{
	return (msp->msp_flr.reg & flag);
}

/**
 * @brief	Set Tx DMA mode
 * @param	msp: MSP register base address
 * @param	mode
 */
void msp_set_tx_dma_mode(t_msp *msp, int mode)
{
	msp->msp_dmacr.bit.tdmae = mode;
}

/**
 * @brief	Get Tx DMA mode
 * @param	msp: MSP register base address
 * @return	dma mode
 */
int msp_get_tx_dma_mode(t_msp *msp)
{
	return msp->msp_dmacr.bit.tdmae;
}

/**
 * @brief	Set Rx DMA mode
 * @param	msp: MSP register base address
 * @param	mode
 */
void msp_set_rx_dma_mode(t_msp *msp, int mode)
{
	msp->msp_dmacr.bit.rdmae = mode;
}

/**
 * @brief	Get Rx DMA mode
 * @param	msp: MSP register base address
 * @return	dma mode
 */
int msp_get_rx_dma_mode(t_msp *msp)
{
	return msp->msp_dmacr.bit.rdmae;
}

/**
 * @brief	IRQ enable
 * @param	msp: MSP register base address
 * @param	IRQ
 */
void msp_irq_enable(t_msp *msp, int irq)
{
	msp->msp_imsc.reg |= irq;
}

/**
 * @brief	IRQ disable
 * @param	msp: MSP register base address
 * @param	IRQ
 */
void msp_irq_disable(t_msp *msp, int irq)
{
	msp->msp_imsc.reg &= ~irq;
}

uint16_t msp_get_irq_config(t_msp *msp)
{
	return msp->msp_imsc.reg;
}

uint16_t msp_get_raw_irq_status(t_msp *msp)
{
	return msp->msp_ris.reg;
}

uint16_t msp_get_irq_status(t_msp *msp)
{
	return msp->msp_mis.reg;
}

void msp_irq_clear(t_msp *msp, int irq)
{
	msp->msp_icr.reg = irq;
}

void msp_en_rx_multi_ch(t_msp *msp)
{
	msp->msp_mcr.bit.rmcen = 0;
}

void msp_dis_rx_multi_ch(t_msp *msp)
{
	msp->msp_mcr.bit.rmcen = 1;
}

int msp_get_rx_multi_ch_sub_frame(t_msp *msp)
{
	return msp->msp_mcr.bit.rmcsf;
}

void msp_set_multi_ch_compare_md(t_msp *msp, int mode)
{
	msp->msp_mcr.bit.rmcmp = mode;
}

int msp_get_multi_ch_compare_md(t_msp *msp)
{
	return msp->msp_mcr.bit.rmcmp;
}

void msp_en_tx_multi_ch(t_msp *msp)
{
	msp->msp_mcr.bit.tmcen = 1;
}

void msp_dis_tx_multi_ch(t_msp *msp)
{
	msp->msp_mcr.bit.tmcen = 0;
}

int msp_get_tx_multi_ch_sub_frame(t_msp *msp)
{
	return msp->msp_mcr.bit.tmcsf;
}

/**
 * @brief	Compare Rx channel register
 * @param	msp: MSP register base address
 * @return	RCM value
 */
void msp_set_rx_multi_compare_reg(t_msp *msp, uint32_t mask)
{
	msp->msp_rcv |= mask;
}

/**
 * @brief	Get Rx channel register
 * @param	msp: MSP register base address
 * @return	RCM value
 */
uint32_t msp_get_rx_multi_compare_reg(t_msp *msp)
{
	return msp->msp_rcv;
}

/**
 * @brief	Enable Rx channel register
 * @param	msp: MSP register base address
 * @param	channel
 */
void msp_en_rx_pin_multi_ch_compare(t_msp *msp, uint8_t channel)
{
	if (channel < 32)
		msp->msp_rcv |= BIT(channel);
}

/**
 * @brief	Disable Rx channel register
 * @param	msp: MSP register base address
 * @param	channel
 */
void msp_dis_rx_pin_multi_ch_compare(t_msp *msp, uint8_t channel)
{
	if (channel < 32)
		msp->msp_rcv &= ~BIT(channel);
}

/**
 * @brief	Compare Rx channel register
 * @param	msp: MSP register base address
 * @return	RCM value
 */
void msp_set_rx_multi_compare_mask(t_msp *msp, uint32_t mask)
{
	msp->msp_rcm |= mask;
}

/**
 * @brief	Get Rx channel register
 * @param	msp: MSP register base address
 * @return	RCM value
 */
uint32_t msp_get_rx_multi_compare_mask(t_msp *msp)
{
	return msp->msp_rcm;
}

/**
 * @brief	Enable Rx channel register
 * @param	msp: MSP register base address
 * @param	channel
 */
void msp_en_rx_pin_multi_ch_compareMask(t_msp *msp, uint8_t channel)
{
	if (channel < 32)
		msp->msp_rcm |= BIT(channel);
}

/**
 * @brief	Disable Rx channel register
 * @param	msp: MSP register base address
 * @param	channel
 */
void msp_dis_rx_pin_multi_ch_compareMask(t_msp *msp, uint8_t channel)
{
	if (channel < 32)
		msp->msp_rcm &= ~BIT(channel);
}

/**
 * @brief	Set enable channel register
 * @param	msp: MSP register base address
 * @param	bank: bank to be addressed
 * @param	reg: new register value
 */
void msp_set_tx_multi_ch_enable(t_msp *msp, uint8_t bank, uint32_t reg)
{
	if (bank < 4)
		msp->msp_tce[bank] = reg;
}

/**
 * @brief	Get enable channel register
 * @param	msp: MSP register base address
 * @param	bank: bank to be addressed
 */
uint32_t msp_get_tx_multi_ch_enable(t_msp *msp, uint8_t bank)
{
	return msp->msp_tce[bank];
}

/**
 * @brief	Enable channel
 * @param	msp: MSP register base address
 * @param	channel: channel to be enabled
 */
void msp_en_pin_tx_multi_ch(t_msp *msp, uint8_t channel)
{
	if (channel < 128)
		msp->msp_tce[channel >> 6] |= BIT(channel & 0x1F);
}

/**
 * @brief	Disable channel
 * @param	msp: MSP register base address
 * @param	channel: channel to be disabled
 */
void msp_dis_pin_tx_multi_ch(t_msp *msp, uint8_t channel)
{
	if (channel < 128)
		msp->msp_tce[channel >> 6] &= ~BIT(channel & 0x1F);
}

/**
 * @brief	Set enable channel register
 * @param	msp: MSP register base address
 * @param	bank: bank to be addressed
 * @param	reg: new register value
 */
void msp_set_rx_multi_ch_enable(t_msp *msp, uint8_t bank, uint32_t reg)
{
	if (bank < 4)
		msp->msp_rce[bank] = reg;
}

/**
 * @brief	Get enable channel register
 * @param	msp: MSP register base address
 * @param	bank: bank to be addressed
 */
uint32_t msp_get_rx_multi_ch_enable(t_msp *msp, uint8_t bank)
{
	return msp->msp_rce[bank];
}

/**
 * @brief	Enable channel
 * @param	msp: MSP register base address
 * @param	channel: channel to be enabled
 */
void msp_en_pin_rx_multi_ch(t_msp *msp, uint8_t channel)
{
	if (channel < 128)
		msp->msp_rce[channel >> 6] |= BIT(channel & 0x1F);
}

/**
 * @brief	Disable channel
 * @param	msp: MSP register base address
 * @param	channel: channel to be disabled
 */
void msp_dis_pin_rx_multi_ch(t_msp *msp, uint8_t channel)
{
	if (channel < 128)
		msp->msp_rce[channel >> 6] &= ~BIT(channel & 0x1F);
}

/**
 * @brief	Wait Tx completion
 * @param	msp: MSP register base address
 */
void msp_wait_tx_complete(t_msp *msp)
{
	while (msp->msp_flr.bit.tbusy == 1);
}

/**
 * @brief	Wait Rx completion
 * @param	msp: MSP register base address
 */
void msp_wait_rx_complete(t_msp *msp)
{
	while (msp->msp_flr.bit.rfe == 1);
}

/**
 * @brief	Empty Rx Fifo
 * @param	msp: MSP register base address
 */
void msp_empty_rx_fifo(t_msp *msp)
{
	volatile uint32_t dummy;
	uint8_t rffen_temp = msp->msp_cgr.bit.rffen;

	msp->msp_cgr.bit.rffen = 1;

	while (msp->msp_flr.bit.rfe == 0) {
		dummy = msp->msp_dr;
	}

	msp->msp_cgr.bit.rffen = rffen_temp;

	(void) dummy;
}

/**
 * @brief	Empty Tx Fifo
 * @param	msp: MSP register base address
 */
void msp_empty_tx_fifo(t_msp *msp)
{
	volatile uint32_t dummy;

	msp->msp_cgr.bit.tffen = 1;

	msp->msp_tstcr = 0x3; /* enable the test config */

	while (msp->msp_flr.bit.tfe == 0) {
		dummy = msp->msp_tstdr;
	}

	msp->msp_tstcr &= ~0x3; /* disable the test config */

	(void) dummy;
}

/**
 * @brief	Disable Rx and Tx
 * @param	msp: MSP register base address
 */
void msp_disable_tx_rx(t_msp *msp)
{
	/* In order to disable the MSP, first SGEN is cleared then TXEN/RXEN
	 and finally also FGEN. */
	msp_dis_sample_rate_gen(msp);
	msp->msp_cgr.bit.txen = 0;
	msp->msp_cgr.bit.rxen = 0;
	msp_irq_clear(msp, MSP_IRQ_ALL_CLEARABLE);
	msp_dis_frame_gen(msp);
}

/**
 * @brief	Disable Tx
 * @param	msp: MSP register base address
 */
void msp_disable_tx(t_msp *msp)
{
	msp_dis_sample_rate_gen(msp);
	msp->msp_cgr.bit.txen = 0;
	msp_irq_clear(msp, MSP_IRQ_ALL_CLEARABLE);
	msp_dis_frame_gen(msp);
}

/**
 * @brief	Disable Rx
 * @param	msp: MSP register base address
 */
void msp_disable_rx(t_msp *msp)
{
	msp_dis_sample_rate_gen(msp);
	msp->msp_cgr.bit.rxen = 0;
	msp_irq_clear(msp, MSP_IRQ_ALL_CLEARABLE);
	msp_dis_frame_gen(msp);
}

/**
 * @brief	Enable Tx and Rx
 * @param	msp: MSP register base address
 */
void msp_enable_tx_rx(t_msp *msp)
{
	msp_en_sample_rate_gen(msp);
	msp->msp_cgr.bit.txen = 1;
	msp->msp_cgr.bit.rxen = 1;
	msp_en_frame_gen(msp);
}

/**
 * @brief	Enable Tx
 * @param	msp: MSP register base address
 */
void msp_enable_tx(t_msp *msp)
{
	msp_en_sample_rate_gen(msp);
	msp->msp_cgr.bit.txen = 1;
	msp_en_frame_gen(msp);
}

/**
 * @brief	Enable Rx
 * @param	msp: MSP register base address
 */
void msp_enable_rx(t_msp *msp)
{
	msp_en_sample_rate_gen(msp);
	msp->msp_cgr.bit.rxen = 1;
	msp_en_frame_gen(msp);
}


/**
 * @brief	MSP irq handler SPI burst mode
 */
void msp0_irq_handler(void)
{
	return;
}

/**
 * @brief	MSP irq handler SPI burst mode
 */
void msp1_irq_handler(void)
{
	return;
}

/**
 * @brief	MSP irq handler SPI burst mode
 */
void msp2_irq_handler(void)
{
	return;
}
