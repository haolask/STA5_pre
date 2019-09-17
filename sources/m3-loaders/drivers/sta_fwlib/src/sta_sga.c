/**
 * @file sta_sga.c
 * @brief  This file provides all the SGA  engine
 *
 * Copyright (C) ST-Microelectronics SA 2016
 * @author: APG-MID team
 */

#include <stdlib.h>
#include <malloc.h>

#include "sta_sga.h"
#include "sta_nvic.h"
#include "sta_common.h"
#include "trace.h"

#include "sta_mtu.h"

/*  Local functions */
static t_sga_error sga_start_batch(uint8_t batch_id);
static t_sga_error sga_stop_batch(uint8_t batch_id);
static t_sga_error sga_set_tst_register_bit(uint8_t batch_id);
static t_sga_error sga_clear_tst_register_bit(uint8_t batch_id);
static t_sga_error sge_create_context(
				      t_sga_batch_context * ctx_batch,
				      const uint8_t idx);
static t_sga_error sga_link_batch(
				  uint8_t batch_id,
				  uint32_t p_batch_add);

/* Global variables */
t_sga_system_context g_sga_system_context;
t_sga_batch_context  *g_sga_ctx_batch;

/**
 * @brief  Main SGA IRQ Handler
 *
 * @param			VOID
 * @return			VOID
 */
void sga_irq_handler(void)
{
	unsigned long mis = g_sga_system_context.p_sga_registers->sga_mis;
	uint8_t it;

	for (it = 0; it < 32; it++) {
		if (!(mis & (1 << it)))
			continue;

		g_sga_system_context.p_sga_registers->sga_icr = mis;
		switch (it) {
		case 5:
			TRACE_ERR("SGA: AHB Error\n");
			break;
		case 4:
			TRACE_ERR("SGA: Hanging\n");
			break;
		case 3:
			TRACE_ERR("SGA: Instruction FIFO Overrun\n");
			break;
		}
		/* Call the callback if any (IT8 => SendIT2) */
		if ((it - 8) >= SGE_MAX_AVAIL_BATCHES)
			continue;

		g_sga_system_context.p_sga_registers->sga_imsc |= (1 << it);
		g_sga_system_context.p_sga_registers->sga_icr = (1 << it);
		if (g_sga_ctx_batch->int_callback[it - 8])
			(*g_sga_ctx_batch->int_callback[it - 8])();
	}
}

/**
 * @brief  Create the Smart Graphic Accelerator working context
 *
 * @param  ctx_batch		(IN)	placeholder for batch context data.
 * @param  idx			(IN)	batch index to be initialized.
 * @return				SGA error condition
 */
t_sga_error sge_create_context(t_sga_batch_context *ctx_batch,
			       const uint8_t idx)
{
	uint8_t	batch_id = 0;

	if (!ctx_batch) {
		TRACE_ERR("%s: [%d] ctx_batch is invalid\n", __func__, idx);
		return SGA_INVALID_PARAMETER;
	}

	if (idx > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: [%d] idx out of range\n", __func__, idx);
		return SGA_INVALID_PARAMETER;
	}

	/* Get free batch id */
	if (sga_get_batch_id(&batch_id) != SGA_OK) {
		TRACE_ERR("%s: sga_get_batch_id failed\n", __func__);
		return SGA_RESOURCE_NOT_AVAILABLE;
	}

	ctx_batch->batch_id[idx]	= batch_id;
	ctx_batch->int_callback[idx]	= NULL;
	/* General Purpose start at 2 */
	ctx_batch->int_id[idx]		= batch_id + 2;

	return SGA_OK;
}

/**
 * @brief  Register SGA Callback
 *
 * @param  ctx_batch		(IN)	placeholder for batch context data.
 * @param  batch_id		(IN)	batch_id variable
 * @return				SGA error condition
 */
t_sga_error sga_register_int_cb(void *ctx_batch,
				uint8_t batch_id,
				void (*func)(void))
{
	t_sga_batch_context *ctx = (t_sga_batch_context *)ctx_batch;

	if (!ctx_batch) {
		TRACE_ERR("%s: [%d] ctx_batch is invalid\n",
			  __func__,
			  batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (batch_id > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: [%d] idx out of range\n",
			  __func__,
			  batch_id);
		return SGA_INVALID_PARAMETER;
	}

	ctx->int_callback[batch_id]	= func;
	return SGA_OK;
}

/**
 * @brief  Get starting batch address
 *
 * @param  ctx_batch		(IN)	placeholder for batch context data.
 * @param  batch_id		(IN)	batch_id variable
 * @param  addr			(OUT)	Batch start address
 * @return				SGA error condition
 */
t_sga_error sga_get_batch_addr(void *ctx_batch,
			       uint8_t batch_id,
			       t_sga_instr_word	**addr)
{
	t_sga_batch_context *ctx = (t_sga_batch_context *)ctx_batch;

	if (!ctx_batch) {
		TRACE_ERR("%s: [%d] ctx_batch is invalid\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (batch_id > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: [%d] batch_id out of range\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (!addr) {
		TRACE_ERR("%s: addr is NULL\n", __func__);
		return SGA_INVALID_PARAMETER;
	}

	*addr = ctx->batch_start_ptr[batch_id];
	return SGA_OK;
}

/**
 * @brief  Get current running batch
 *
 * @param  ctx_batch		(IN)	placeholder for batch context data.
 * @param  batch_id		(OUT)	Running batch ID
 * @return				SGA error condition
 */
t_sga_error sga_get_curr_batch(void *ctx_batch,
			       uint8_t *batch_id)
{
	t_sga_batch_context *ctx = (t_sga_batch_context *)ctx_batch;

	if (!ctx_batch) {
		TRACE_ERR("%s: ctx_batch is invalid\n", __func__);
		return SGA_INVALID_PARAMETER;
	}

	if (!batch_id) {
		TRACE_ERR("%s: batch_id is NULL\n", __func__);
		return SGA_INVALID_PARAMETER;
	}

	*batch_id = ctx->curr_batch;
	return SGA_OK;
}

/**
 * @brief  Commit Batch
 *
 * @param  ctx_batch		(IN)	placeholder for batch context data.
 * @param  batch_id		(IN)	batch_id variable
 * @param  addr			(OUT)	Batch ending address
 * @return				SGA error condition
 */
t_sga_error sga_commit_batch(
				void			*ctx_batch,
				uint8_t			batch_id,
				t_sga_instr_word	*addr)
{
	t_sga_batch_context	*ctx = (t_sga_batch_context *)ctx_batch;
	t_sga_instr_word	*end_address = NULL;

	if (!ctx_batch) {
		TRACE_ERR("%s: [%d] ctx_batch is invalid\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (batch_id > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: [%d] batch_id out of range\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	end_address = ctx->batch_start_ptr[batch_id + 1] - SGE_END_BATCH_OFFSET;
	if (addr + (3 * sizeof(t_sga_instr_word))  > end_address) {
		TRACE_ERR("%s Error: [%d] addr is outside batch range\n",
			  __func__, batch_id);
		return SGA_INTERNAL_ERROR;
	}

	/* Complete the current batch with Int ID and return */
	*addr++ = CLR_INSTR_TEST_REG	|  ctx->batch_id[batch_id];
	*addr++ = SEND_INTERRUPT	| ctx->int_id[batch_id];
	*addr++ = RETURN;
	ctx->last_inst_addr[batch_id] = addr;

	return SGA_OK;
}

/**
 * @brief  Set output surface
 *
 * @param  ctx_batch		(IN)	placeholder for batch context data.
 * @param  batch_id		(IN)	batch_id variable
 * @param  surface_addr		(IN)	destination surface address
 * @return				SGA error condition
 */
t_sga_error sga_set_surface(void *ctx_batch,
			    uint8_t batch_id,
			    uint32_t surface_addr)
{
	t_sga_batch_context *ctx = (t_sga_batch_context *)ctx_batch;

	if (!ctx_batch) {
		TRACE_ERR("%s: [%d] ctx_batch is invalid\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (batch_id > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: [%d] batch_id out of range\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	ctx->dst_surface_id[batch_id] = surface_addr;
	return SGA_OK;
}

/**
 * @brief  Get output surface
 *
 * @param  ctx_batch		(IN)	placeholder for batch context data.
 * @param  batch_id		(IN)	batch_id variable
 * @param  surface_addr		(OUT)	destination surface address
 * @return				SGA error condition
 */
t_sga_error sga_get_surface(void *ctx_batch,
			    uint8_t batch_id,
			    uint32_t *surface_addr)
{
	t_sga_batch_context *ctx = (t_sga_batch_context *)ctx_batch;

	if (!ctx_batch) {
		TRACE_ERR("%s: [%d] ctx_batch is invalid\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (batch_id > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: [%d] batch_id out of range\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (!surface_addr) {
		TRACE_ERR("%s: NULL pointer\n", __func__);
		return SGA_INVALID_PARAMETER;
	}

	*surface_addr = ctx->dst_surface_id[batch_id];

	return SGA_OK;
}

/**
 * @brief  Set output surface
 *
 * @param  ctx_batch		(IN)	placeholder for batch context data.
 * @param  batch_id		(IN)	batch_id variable
 * @param  surface_addr		(IN)	destination surface address
 * @return				SGA error condition
 */
t_sga_error sga_set_fb_instr_ptr_addr(
					void	*ctx_batch,
					uint8_t		batch_id,
					uint32_t	*addr_instr_ptr)
{
	t_sga_batch_context *ctx = (t_sga_batch_context *)ctx_batch;

	if (!ctx_batch) {
		TRACE_ERR("%s: [%d] ctx_batch is invalid\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (batch_id > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: [%d] batch_id out of range\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	ctx->addr_instr_ptr[batch_id] = addr_instr_ptr;
	return SGA_OK;
}

/**
 * @brief  Get output surface
 *
 * @param  ctx_batch		(IN)	placeholder for batch context data.
 * @param  batch_id		(IN)	batch_id variable
 * @param  surface_addr		(OUT)	destination surface address
 * @return				SGA error condition
 */
t_sga_error sga_get_fb_instr_ptr_addr(
					void		*ctx_batch,
					uint8_t		batch_id,
					uint32_t	**addr_instr_ptr)
{
	t_sga_batch_context *ctx = (t_sga_batch_context *)ctx_batch;

	if (!ctx_batch) {
		TRACE_ERR("%s: [%d] ctx_batch is invalid\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (batch_id > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: [%d] batch_id out of range\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (!addr_instr_ptr) {
		TRACE_ERR("%s: NULL pointer\n", __func__);
		return SGA_INVALID_PARAMETER;
	}

	*addr_instr_ptr = ctx->addr_instr_ptr[batch_id];

	return SGA_OK;
}

/**
 * @brief  Set output surface
 *
 * @param  ctx_batch		(IN)	placeholder for batch context data.
 * @param  batch_id		(IN)	batch_id variable
 * @param  surface_addr		(IN)	destination surface address
 * @return				SGA error condition
 */
t_sga_error sga_set_priv_data(
					void		*ctx_batch,
					uint8_t		batch_id,
					uint32_t	priv)
{
	t_sga_batch_context *ctx = (t_sga_batch_context *)ctx_batch;

	if (!ctx_batch) {
		TRACE_ERR("%s: [%d] ctx_batch is invalid\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (batch_id > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: [%d] batch_id out of range\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	ctx->priv[batch_id] = priv;
	return SGA_OK;
}

/**
 * @brief  Get output surface
 *
 * @param  ctx_batch		(IN)	placeholder for batch context data.
 * @param  batch_id		(IN)	batch_id variable
 * @param  surface_addr		(OUT)	destination surface address
 * @return				SGA error condition
 */
t_sga_error sga_get_priv_data(
					void		*ctx_batch,
					uint8_t		batch_id,
					uint32_t	*priv)
{
	t_sga_batch_context *ctx = (t_sga_batch_context *)ctx_batch;

	if (!ctx_batch) {
		TRACE_ERR("%s: [%d] ctx_batch is invalid\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (batch_id > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: [%d] batch_id out of range\n",
			  __func__, batch_id);
		return SGA_INVALID_PARAMETER;
	}

	if (!priv) {
		TRACE_ERR("%s: NULL pointer\n", __func__);
		return SGA_INVALID_PARAMETER;
	}

	*priv = ctx->priv[batch_id];

	return SGA_OK;
}

/**
 * @brief     Initialize the batch context
 *
 * @param     num_batchs	(IN)	Number of batches for the context
 *					(2 by default)
 * @param     batch_size	(IN)	Size of the batch
 * @param     header_size	(IN)	Size of the batch payload
 * @param     void*		(OUT)	Pointer to the batch Memory Context
 * @param     void*		(OUT)	Pointer to the batch Context
 * @return				SGA error condition
 *
 */
t_sga_error sge_do_init_batch(const uint32_t num_batchs,
			      const uint32_t batch_size,
			      const uint32_t header_size,
			      void **ctx_mem,
			      void **ctx_batch)
{
	t_sga_error		ret;
	t_sga_batch_context	*ctx		= NULL;
	uint32_t		base		= 0;
	int			b;
	uint32_t		batch_phys;

	if (!ctx_mem) {
		TRACE_ERR("%s: Null Memory Context\n", __func__);
		return SGA_INVALID_PARAMETER;
	}

	if (!ctx_batch) {
		TRACE_ERR("%s: Null Batch Context\n", __func__);
		return SGA_INVALID_PARAMETER;
	}

	if (num_batchs > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: Too many requested batches (%d > %d)\n",
			  __func__, num_batchs, SGE_MAX_AVAIL_BATCHES);
		return SGA_RESOURCE_NOT_AVAILABLE;
	}

	base = (uint32_t)memalign(SGA_BUFFER_ALIGNMENT, SGE_BATCHES_SIZE);
	if (!base) {
		TRACE_ERR("%s: Cannot allocate memory for batches\n", __func__);
		return SGA_GENERIC_FAILURE;
	}
	g_sga_system_context.p_sga_batches_addr = (uint32_t *)base;

	ctx = (t_sga_batch_context *)(base + sizeof(t_sga_memory_context));
	g_sga_ctx_batch = ctx;
	batch_phys	= base + header_size;

	TRACE_INFO("Batch Main Address : 0x%08X\n", base);

	ctx->magic	= SGE_SET_MAGIC_WORD;
	ctx->num_batchs	= num_batchs;

	for (b = 0; b < (int)num_batchs; b++) {
		if (!SGE_IS_ALIGN(batch_phys, SGA_MEMORY_ALIGN_MASK)) {
			TRACE_ERR("%s: Batch start address not aligned\n",
				  __func__);
			return SGA_INTERNAL_ERROR;
		}

		TRACE_INFO("Batch [%d] Address : 0x%08X-{%d}\n",
			   b, batch_phys, batch_size);

		ctx->batch_start_ptr[b]	= (t_sga_instr_word *)batch_phys;
		ctx->last_inst_addr[b]	= (t_sga_instr_word *)batch_phys;
		ctx->dst_surface_id[b]	= SGE_NULL_SURFACE;

		ret = sge_create_context(ctx, b);
		if (ret != SGA_OK) {
			TRACE_ERR("%s: Can't create context\n", __func__);
			return SGA_INTERNAL_ERROR;
		}

		ret = sga_link_batch(ctx->batch_id[b], (uint32_t)batch_phys);
		if (ret != SGA_OK) {
			TRACE_ERR("%s: Can't link batch\n", __func__);
			return SGA_INTERNAL_ERROR;
		}

		batch_phys += batch_size;
	}

	ctx->curr_batch = 0;

	/* Last element of the array is used as a sentinel */
	ctx->batch_start_ptr[num_batchs] = (t_sga_instr_word *)batch_phys;
	ctx->curr_batch_surface.curr_addr = ctx->batch_start_ptr[0] +
					    SGE_START_BATCH_OFFSET;
	ctx->curr_batch_surface.batch_end = ctx->batch_start_ptr[1] -
					    SGE_END_BATCH_OFFSET;

	*ctx_mem = (void *)base;
	*ctx_batch = (void *)ctx;

	return SGA_OK;
}

/**
 * @brief    Start the batch, jumping from the main firmware into the
 *           batch instruction pool.
 *
 * @param    ctx_batch			(IN)	Context of the Batch
 * @param    batch_id			(IN)	Batch Id
 * @return					SGA error condition
 */
t_sga_error sge_do_start_batch(void *ctx_batch,
			       unsigned long batch_id)
{
	t_sga_error		ret;
	t_sga_instr_word	*start_address	= NULL;
	t_sga_instr_word	*end_address	= NULL;
	t_sga_instr_word	*curr_address	= NULL;
	t_sga_batch_context	*ctx	= (t_sga_batch_context *)ctx_batch;

	if (!ctx_batch) {
		TRACE_ERR("%s: [%d] invalid ctx_batch\n", __func__, batch_id);
		ret = SGA_INVALID_PARAMETER;
		goto failure;
	}

	if (batch_id > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: [%d] batch_id out of range\n",
			  __func__, batch_id);
		ret = SGA_INVALID_PARAMETER;
		goto failure;
	}

	start_address = ctx->batch_start_ptr[batch_id];
	end_address = ctx->batch_start_ptr[batch_id + 1] - SGE_END_BATCH_OFFSET;
	curr_address = ctx->last_inst_addr[batch_id];

	if (curr_address < start_address) {
		TRACE_ERR("%s: curr_address out of allowed range, Aborting\n",
			  __func__);
		ret = SGA_INTERNAL_ERROR;
		goto failure;
	}

	if (curr_address > end_address) {
		TRACE_ERR("%s: curr_address is outside the range allowed!\n",
			  __func__);
		curr_address = end_address;
	}

	/*
	 * NOW really send to SGA the command to start processing
	 */
	g_sga_system_context.p_sga_registers->sga_imsc	&=
		~(1 << (ctx->int_id[batch_id] + 6));
	ret = sga_start_batch(batch_id);
	ctx->curr_batch = batch_id;

failure:
	return ret;
}

/**
 * @brief     Stops a running batch.
 *
 * @param     ctx_batch			(IN)	Context of the Batch
 * @param     batch_id			(IN)	Target Batch ID
 * @return					SGA error condition
 */
t_sga_error sge_do_stop_batch(void *ctx_batch,
			      unsigned long batch_id)
{
	t_sga_error		ret;
	t_sga_instr_word	*start_address	= NULL;
	t_sga_instr_word	*end_address	= NULL;
	t_sga_instr_word	*curr_address	= NULL;
	t_sga_batch_context	*ctx = (t_sga_batch_context *)ctx_batch;

	if (!ctx_batch) {
		TRACE_ERR("%s: [%d] ctx_batch invalid\n", __func__, batch_id);
		ret = SGA_INVALID_PARAMETER;
		goto failure;
	}

	if (batch_id > SGE_MAX_AVAIL_BATCHES) {
		TRACE_ERR("%s: [%d] batch_id out of range\n",
			  __func__, batch_id);
		ret = SGA_INVALID_PARAMETER;
		goto failure;
	}

	start_address = ctx->batch_start_ptr[batch_id];
	end_address   = ctx->batch_start_ptr[batch_id + 1] -
			SGE_END_BATCH_OFFSET;
	curr_address  = ctx->last_inst_addr[batch_id];

	if (curr_address < start_address) {
		TRACE_ERR("%s: curr_address out of allowed range, Aborting\n",
			  __func__);
		ret = SGA_INTERNAL_ERROR;
		goto failure;
	}

	if (curr_address > end_address) {
		TRACE_ERR("%s: curr_address is outside the range allowed!\n",
			  __func__);
		curr_address = end_address;
	}

	/*
	 * NOW really send to SGA the command to stop processing
	 */
	g_sga_system_context.p_sga_registers->sga_imsc	|=
		(1 << (ctx->int_id[batch_id] + 6));
	ret = sga_stop_batch(ctx->batch_id[batch_id]);

failure:
	return ret;
}

/**
 * @brief     Sets bit in SGA test register
 *
 * @param     bit_nb			(IN)	Bit number
 * @return					SGA error condition
 */
t_sga_error sge_set_tst_register_bit(uint8_t bit_nb)
{
	t_sga_error		ret;

	if ((bit_nb < SGE_MAX_AVAIL_BATCHES) ||
	    (bit_nb > 31)) {
		TRACE_ERR("%s: bit_nb out of allowed range, Aborting\n",
			  __func__);
		ret = SGA_INTERNAL_ERROR;
		goto failure;
	}

	/*
	 * NOW really send to SGA the command to stop processing
	 */
	ret = sga_set_tst_register_bit(bit_nb);

failure:
	return ret;
}

/**
 * @brief     Clears bit in SGA test register
 *
 * @param     bit_nb			(IN)	Bit number
 * @return					SGA error condition
 */
t_sga_error sge_clear_tst_register_bit(uint8_t bit_nb)
{
	t_sga_error		ret;

	if ((bit_nb < SGE_MAX_AVAIL_BATCHES) ||
	    (bit_nb > 31)) {
		TRACE_ERR("%s: bit_nb out of allowed range, Aborting\n",
			  __func__);
		ret = SGA_INTERNAL_ERROR;
		goto failure;
	}

	/*
	 * NOW really send to SGA the command to stop processing
	 */
	ret = sga_clear_tst_register_bit(bit_nb);

failure:
	return ret;
}

/**
 * @brief    Get the batch ID
 *
 * @param    batch_id	(IN)	Pointer to a batch_id variable
 * @return			SGA error condition
 */
t_sga_error sga_get_batch_id(
							 uint8_t *batch_id)
{
	uint8_t i;

	if (!batch_id) {
		TRACE_ERR("%s: NULL batch_id\n", __func__);
		return SGA_INVALID_PARAMETER;
	}

	/*  Check the available batch id */
	for (i = 0; i < SGE_MAX_AVAIL_BATCHES; i++) {
		if (!((g_sga_system_context.batch_sem_id & (1 << i)) >> i)) {
			g_sga_system_context.batch_sem_id |= (1 << i);
			break;
		}
	}

	if (i < SGE_MAX_AVAIL_BATCHES) {
		*batch_id = i;
		return SGA_OK;
	}

	return SGA_RESOURCE_NOT_AVAILABLE;
}

/**
 * @brief    Pick an available batch
 *
 * @param    batch_id		(IN)	Pointer to a batch_id variable
 * @return				SGA error condition
 */
t_sga_error sga_pick_batch(
						   uint8_t *batch_id)
{
	uint8_t i;

	if (!batch_id) {
		TRACE_ERR("%s: NULL batch_id\n", __func__);
		return SGA_INVALID_PARAMETER;
	}

	/* Check the available Batch */
	for (i = SGE_MAX_AVAIL_BATCHES; i < 2 * SGE_MAX_AVAIL_BATCHES; i++) {
		if (!((g_sga_system_context.batch_sem_id & (1 << i)) >> i)) {
			if ((g_sga_system_context.batch_sem_id &
			    (1 << (i - SGE_MAX_AVAIL_BATCHES))) >>
			    (i - SGE_MAX_AVAIL_BATCHES)) {
				g_sga_system_context.batch_sem_id |= (1 << i);
				break;
			}
		}
	}

	if (i < 2 * SGE_MAX_AVAIL_BATCHES) {
		*batch_id = i - SGE_MAX_AVAIL_BATCHES;
		return SGA_OK;
	}

	return SGA_RESOURCE_NOT_AVAILABLE;
}

/**
 * @brief    Release a batch
 *
 * @param    batch_id		(IN)	Batch_id variable
 * @return				SGA error condition
 */
t_sga_error sga_release_batch(
							  uint8_t batch_id)
{
	if (batch_id >= SGE_MAX_AVAIL_BATCHES)
		return SGA_INVALID_PARAMETER;

	g_sga_system_context.batch_sem_id &=
				~(1 << (batch_id + SGE_MAX_AVAIL_BATCHES));

	return SGA_OK;
}

/**
 * @brief   Build the SGA main firmware. Until a link batch command happens,
 * the SGA stays in loop in the main firmware
 *
 * @param    p_phy_main_add		(OUT)	Memory location address to write
 *                                              the main firmware
 * @param    p_phy_default_batch_add	(OUT)	Default batch  firmware address
 * @param    p_no_cmd			(OUT)	Number of written commands
 * @return					SGA error condition
 */
t_sga_error sga_build_main_batch_firmware(uint32_t *p_phy_main_add,
					  uint32_t *p_phy_default_batch_add,
					  uint32_t *p_no_cmd)
{
	uint32_t	main_add, batch_add;
	uint32_t	i = 0;

	/* Check the validity of the input parameters */
	if ((!p_phy_main_add) ||
	    (!p_no_cmd) ||
	    (((uint32_t)p_phy_main_add) & SGA_MEMORY_ALIGN_MASK))
		return SGA_INVALID_PARAMETER;

	g_sga_system_context.p_main_batch_add	=	p_phy_main_add;

	main_add = (uint32_t)p_phy_main_add;
	batch_add = ((uint32_t)p_phy_default_batch_add >> SGA_ADDRESS_ALIGN);

	/* Main Batch firmware */
	for (i = 0; i < SGE_MAX_AVAIL_BATCHES; i++) {
		p_phy_main_add[2 * i]		= TST_INSTR_TEST_REG | i;
		p_phy_main_add[2 * i + 1]	= GOSUB | batch_add;
	}
	i = SGE_MAX_AVAIL_BATCHES << 1;
	p_phy_main_add[i++] = WAIT_INSTR_TEST_REG;
	p_phy_main_add[i++] = GOTO | (main_add >> SGA_ADDRESS_ALIGN);

	/* Insert the the "STOP" command at default
	 * batch firmware memory location
	 */
	*p_phy_default_batch_add = STOP;

	/* Number of commands inserted at given memory location */
	*p_no_cmd = i;

	return SGA_OK;
}

/**
 * @brief    Make the SGA to jump into the main firmware.
 *
 * @param    VOID
 * @return				SGA error condition
 */
t_sga_error sga_run_main_batch_firmware(void)
{
	uint32_t    cmd;

	if (!g_sga_system_context.p_main_batch_add)
		return SGA_MAIN_FIRMWARE_NOT_BUILD;

	cmd = GOTO | (((uint32_t)g_sga_system_context.p_main_batch_add) >>
		      SGA_ADDRESS_ALIGN);

	/* Start the main batch firmware */
	g_sga_system_context.p_sga_registers->sga_instr = cmd;

	/* Cache Configuration */
	g_sga_system_context.p_sga_registers->sga_instr =
					CACHE_CTRL | SGA_CACHE_CONFIGURATION;

	/* Auto Fetch Active */
	g_sga_system_context.p_sga_registers->sga_instr =
						AHB | SGA_AHB_CONFIGURATION;

	/* Enable the SGA Global resume bit */
	SGA_SET_BIT(g_sga_system_context.p_sga_registers->sga_ctcmd,
		    SGA_CTCMD_GRESUME_MASK);

	/* Enable the SGA Instructions resume bit */
	SGA_SET_BIT(g_sga_system_context.p_sga_registers->sga_ctcmd,
		    SGA_CTCMD_IRESUME_MASK);

	return SGA_OK;
}

/**
 * @brief    Make the Main Firmware to jump into a batch pool of instructions.
 *
 * @param    batch_id			(IN)	ID of the batch to jump in
 * @param    p_batch_add		(IN)	Address of the instruction pool
 * @return					SGA error condition
 */
t_sga_error sga_link_batch(
						   uint8_t batch_id,
						   uint32_t p_batch_add)
{
	uint32_t    *p_batch_fw_add;

	if ((p_batch_add == 0) ||
	    (p_batch_add & SGA_MEMORY_ALIGN_MASK) ||
	    (batch_id >= SGE_MAX_AVAIL_BATCHES))
		return SGA_INVALID_PARAMETER;

	/* Replace GOSUB Instruction */
	p_batch_fw_add = g_sga_system_context.p_main_batch_add +
			 (2 * batch_id + 1);

	/*
	 * Halt the main batch firmware executing by setting
	 * Instruction Halt bit in the control command register
	 */
	SGA_SET_BIT(g_sga_system_context.p_sga_registers->sga_ctcmd,
		    SGA_CTCMD_IHALT_MASK);

	/* Update the  batch firmware address in the main batch program */
	*p_batch_fw_add			= GOSUB |
					  (p_batch_add >> SGA_ADDRESS_ALIGN);
	*(uint32_t *)p_batch_add	= RETURN;

	/* Resume instructions execution */
	SGA_SET_BIT(g_sga_system_context.p_sga_registers->sga_ctcmd,
		    SGA_CTCMD_IRESUME_MASK);

	return SGA_OK;
}

/**
 * @brief    Start the execution of a batch acting on SGA registers
 *
 * @param    VOID
 * @param    batch_id		(IN)	ID of the batch to start
 * @return				SGA error condition
 */
t_sga_error sga_start_batch(uint8_t batch_id)
{
	if (batch_id >= SGE_MAX_AVAIL_BATCHES)
		return SGA_INVALID_PARAMETER;

	/* Set the corresponding test register bit */
	g_sga_system_context.p_sga_registers->sga_sitr = (1 << batch_id);

	return SGA_OK;
}

/**
 * @brief    Stop the execution of a batch acting on SGA registers
 *
 * @param    VOID
 * @param    batch_id		(IN)	ID of the batch to stop
 * @return				SGA error condition
 */
t_sga_error sga_stop_batch(uint8_t batch_id)
{
	if (batch_id >= SGE_MAX_AVAIL_BATCHES)
		return SGA_INVALID_PARAMETER;

	/* Set the corresponding test register bit */
	g_sga_system_context.p_sga_registers->sga_citr = (1 << batch_id);

	return SGA_OK;
}

/**
 * @brief    Set specified bit in SGA test register
 *
 * @param    bit_nb		(IN)	bit number
 * @return				SGA error condition
 */
t_sga_error sga_set_tst_register_bit(uint8_t bit_nb)
{
	/* Sets the corresponding test register bit */
	g_sga_system_context.p_sga_registers->sga_sitr = (1 << bit_nb);
	return SGA_OK;
}

/**
 * @brief    Clears specified bit in SGA test register
 *
 * @param    bit_nb		(IN)	bit number
 * @return				SGA error condition
 */
t_sga_error sga_clear_tst_register_bit(uint8_t bit_nb)
{
	/* Clears the corresponding test register bit */
	g_sga_system_context.p_sga_registers->sga_citr = (1 << bit_nb);
	return SGA_OK;
}

/**
 * @brief  De-initialize the Smart Graphic Accelerator
 *
 * @param  VOID
 * @return			SGA error condition
 */
t_sga_error sge_deinit(void)
{
	struct nvic_chnl nvic_deinit_struct;

	SGA_SET_BIT(g_sga_system_context.p_sga_registers->sga_ctcmd,
		    SGA_CTCMD_GHALT_MASK);
	SGA_SET_BIT(g_sga_system_context.p_sga_registers->sga_ctcmd,
		    SGA_CTCMD_GRST_MASK);

	/* Disable SGA interrupt at nVIC level */
	nvic_deinit_struct.id		= EXT2_IRQChannel;
	nvic_deinit_struct.sub_prio	= 0;
	nvic_deinit_struct.preempt_prio	= 0;
	nvic_deinit_struct.enabled	= false;

	nvic_chnl_init(&nvic_deinit_struct);

	g_sga_system_context.p_sga_registers = NULL;

	/* Flag the all semaphores as available resources */
	g_sga_system_context.batch_sem_id		= 0;

	/* Flag the all interrupts as available resources */
	g_sga_system_context.interrupt_id		= 0;

	g_sga_system_context.p_main_batch_add		= NULL;
	g_sga_system_context.p_default_batch_add	= NULL;

	free(g_sga_system_context.p_main_fw_addr);
	g_sga_system_context.p_main_fw_addr = NULL;

	free(g_sga_system_context.p_sga_batches_addr);
	g_sga_system_context.p_sga_batches_addr = NULL;

	return SGA_OK;
}

/**
 * @brief  Initialize the Smart Graphic Accelerator
 *
 * @param  VOID
 * @return			SGA error condition
 */
t_sga_error sge_init(void)
{
	uint32_t nb_of_cmds	=	0;
	struct nvic_chnl nvic_init_struct;
	uint32_t main_fw_addr = 0;

	/* Init SGE register space */
	g_sga_system_context.p_sga_registers = (t_sga_registers *)SGA_BASE;
	g_sga_system_context.p_main_batch_add		= NULL;
	g_sga_system_context.p_default_batch_add	= NULL;

	/* Flag the all semaphores as available resources */
	g_sga_system_context.batch_sem_id		= 0;

	/* Flag the all interrupts as available resources */
	g_sga_system_context.interrupt_id		= 0;

	/* Global initialization */
	SGA_SET_BIT(g_sga_system_context.p_sga_registers->sga_ctcmd,
		    SGA_CTCMD_GINIT_MASK);

	/* Make sure SGA interrupts are disabled */
	g_sga_system_context.p_sga_registers->sga_imsc = SGA_INT_GEN_ALL;

	/* Configure the interrupt clear mode */
	g_sga_system_context.p_sga_registers->sga_gcr	&= ~0x00000001;

	/* Configure the FCLK gating enable */
	g_sga_system_context.p_sga_registers->sga_gcr	&= ~0x00000002;

	/* Configure the HCLK gating enable */
	g_sga_system_context.p_sga_registers->sga_gcr	&= ~0x00000004;

	/* Configure the Interrupt1 clear mode */
	g_sga_system_context.p_sga_registers->sga_gcr	&= ~0x00000008;

	main_fw_addr = (uint32_t)memalign(SGA_BUFFER_ALIGNMENT,
					  SGE_MAIN_FIRMWARE_SIZE);
	if (!main_fw_addr) {
		TRACE_ERR("%s: No memory to allocate SGA main fw\n", __func__);
		return SGA_GENERIC_FAILURE;
	}
	g_sga_system_context.p_main_fw_addr = (uint32_t *)main_fw_addr;

	/*
	 * Build the main FW Context
	 */
	TRACE_INFO("SGA Firmware : 0x%08x-{%d}\n",
		   main_fw_addr,
		   SGE_MAIN_FIRMWARE_SIZE);

	if ((sga_build_main_batch_firmware(
		(uint32_t *)(main_fw_addr + SGE_FIRMWARE_OFFSET),
		(uint32_t *)(main_fw_addr + SGE_FIRMWARE_OFFSET +
		SGE_DEFAULT_FIRMWARE_OFFSET),
		&nb_of_cmds) != SGA_OK) ||
	    (nb_of_cmds * sizeof(uint32_t) >  SGE_MAIN_FIRMWARE_SIZE)) {
		TRACE_ERR("%s: Build Main Firmware failed\n", __func__);
		return SGA_MAIN_FIRMWARE_NOT_BUILD;
	}

	/*
	 * Run the main FW
	 */
	if (sga_run_main_batch_firmware() != SGA_OK) {
		TRACE_ERR("%s: Run batch firmware failed\n", __func__);
		return SGA_GENERIC_FAILURE;
	}

	/* Enable SGA interrupt at nVIC level */
	nvic_init_struct.id		= EXT2_IRQChannel;
	nvic_init_struct.sub_prio	= 0;
	nvic_init_struct.preempt_prio	= IRQ_LOW_PRIO;
	nvic_init_struct.enabled	= true;

	nvic_chnl_init(&nvic_init_struct);

	return SGA_OK;
}

