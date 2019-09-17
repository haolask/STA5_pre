/**
 * @file sta_tuner.c
 * @early tuner setup
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"
#include "utils.h"
#include "queue.h"
#include <errno.h>

#include "sta_common.h"
#include "sta_image.h"
#include "sta_mbox.h"

#include "etal_api.h"
#include "tunerdriver.h"
#include "sta_tuner.h"
#include "sta_rpmsg_rdm.h"

#include "sta_mem_map.h"

/* Define for backup RAM save/restore feature */
#define TUNER_MAGIC_ADDR	BACKUP_RAM_TUNER_CONTEXT_BASE
#define TUNER_MAGIC_WORD	0x19283746
#define TUNER_FREQ_BASE_ADDR	(BACKUP_RAM_TUNER_CONTEXT_BASE + 4)
#define TUNER_BAND_BASE_ADDR	(BACKUP_RAM_TUNER_CONTEXT_BASE + 8)

/* Define for MBOX use for I2C release feature */
#define MBOX_BUFFER_SIZE    0x10
#define TUNER_CTX_Q_SIZE	5

/* Local variable declaration */
static ETAL_HANDLE v_handle_fe;
static xQueueHandle x_queue_tuner_ctx;
static struct tuner_fwm_ty tuner_FWM;

/**************************************
 * Backup Radio context for early audio
 **************************************/
void tuner_ctx_isr(void *context, struct mbox_msg *msg)
{
	BaseType_t x_higher_priority_task_woken;
	char buf[MBOX_BUFFER_SIZE];
	int i;
	int len = msg->dsize;

	if (len > MBOX_BUFFER_SIZE)
		len = MBOX_BUFFER_SIZE;

	for (i = 0; i < len; i++)
		buf[i] = msg->pdata[i];

	xQueueSendToBackFromISR(x_queue_tuner_ctx,
				(void *)buf, &x_higher_priority_task_woken);
	/* Now the buffer is empty we can switch context if necessary. */
	if (x_higher_priority_task_woken)
		taskYIELD();
}

static int tuner_ctx_init(void)
{
	struct mbox_chan_req mboxreq;
	int channel;

	x_queue_tuner_ctx = xQueueCreate(TUNER_CTX_Q_SIZE,
					 sizeof(char) * MBOX_BUFFER_SIZE);
	if (!x_queue_tuner_ctx) {
		/* Queue was not created and must not be used. */
		TRACE_ERR("%s: out of memory\n", __func__);
		return -ENOMEM;
	}

	/* Set requested Mailbox channel feature */
	mboxreq.chan_name = "tuner-backup";
	mboxreq.user_data = NULL;
	mboxreq.rx_cb = tuner_ctx_isr;

	channel = mbox_request_channel(&mboxreq);
	if (channel == MBOX_ERROR) {
		TRACE_ERR("%s: requested chan %s not found\n",
			  __func__, mboxreq.chan_name);
		return -EIO;
	}

	TRACE_INFO("%s: requested chan %s [id:%d] allocated\n",
		   __func__, mboxreq.chan_name, channel);

	return 0;
}

static int tuner_get_msg(etalCtxBackupEarlyAudioTy *buf)
{

	/* Queue not initialized yet */
	if (!x_queue_tuner_ctx)
		return -EAGAIN;

	if (xQueueReceive(x_queue_tuner_ctx, buf, portMAX_DELAY)) {
		TRACE_INFO("%s: received buf: %s\n", __func__, buf);
	} else {
		TRACE_ERR("%s: dit not receive anything\n", __func__);
		return -EIO;
	}
	return 0;
}

/********************************
 *  BackupRAM function management
 ********************************/
static void tuner_backup_write_magic(void)
{
	write_reg(TUNER_MAGIC_WORD, TUNER_MAGIC_ADDR);
}

static uint32_t tuner_backup_magic_request(void)
{
	uint32_t val = 0;
	uint32_t ret = 0;

	val = read_reg(TUNER_MAGIC_ADDR);
	if (val == TUNER_MAGIC_WORD)
		ret = 1;

	return ret;
}

static void tuner_backup_read_data(etalCtxBackupEarlyAudioTy *tnr_ctx)
{
	tnr_ctx->Freq = read_reg(TUNER_FREQ_BASE_ADDR);
	tnr_ctx->Band = read_reg(TUNER_BAND_BASE_ADDR);
}

static void tuner_backup_write_data(const etalCtxBackupEarlyAudioTy *tnr_ctx)
{
	write_reg(tnr_ctx->Freq, TUNER_FREQ_BASE_ADDR);
	write_reg(tnr_ctx->Band, TUNER_BAND_BASE_ADDR);
	TRACE_INFO("Save Freq= %d, Band= %x\n", tnr_ctx->Freq, tnr_ctx->Band);
}

void tuner_FWM_update(void *shadow_addr, uint32_t size)
{
	tuner_FWM.fwm_address = shadow_addr;
	tuner_FWM.fwm_size = size;
}

static void tuner_user_notif_handler(void *ctx, ETAL_EVENTS dw_event, void *pstatus)
{
	if (dw_event != ETAL_INFO_TUNE)
		TRACE_ERR("Unexpected event %d\n", dw_event);
}

/********************************
 *  Tuner function management
 ********************************/
static int tuner_amfm_tune(uint32_t v_amfm_freq, uint32_t v_duration)
{
	EtalReceiverAttr attr;
	ETAL_HANDLE handle_amfm;
	ETAL_STATUS ret;

	/*
	 * Create a receiver configuration
	 */
	TRACE_INFO("************* FM FG RADIO TUNING *************\n");

	handle_amfm = ETAL_INVALID_HANDLE;
	memset(&attr, 0x00, sizeof(EtalReceiverAttr));

	attr.m_Standard = ETAL_BCAST_STD_FM;
	attr.m_FrontEnds[0] = v_handle_fe;
	attr.m_FrontEndsSize = 1;

	ret = etal_config_receiver(&handle_amfm, &attr);
	if (ret != ETAL_RET_SUCCESS)
		return -1;

	/* Select audio source */
	ret = etal_audio_select(handle_amfm, ETAL_AUDIO_SOURCE_STAR_AMFM);
	if (ret != ETAL_RET_SUCCESS)
		return -1;

	/*
	 * Tune to an FM station
	 */

	TRACE_INFO("Tune to fm freq %d\n", v_amfm_freq);
	ret = etal_tune_receiver(handle_amfm, v_amfm_freq);
	if (ret != ETAL_RET_SUCCESS)
		return -1;

	TRACE_INFO("************* TUNE FM FG DONE *************\n");

	return 0;
}

/**
 * @brief	Early tuner task
 * @return	None
 */
void early_tuner_task(struct sta *context)
{
	bool fwm_in_partition = true;
	bool tuner1_init = true;
	bool tuner2_init = true;
	bool dcop_reset = true;
	int ret;
	EtalHardwareAttr init_params;

	bool read_image_ok = false;
	uint32_t vl_duration = 0xFFFFFFFF;
	etalCtxBackupEarlyAudioTy tnr_ctx;

	v_handle_fe = ETAL_FE_HANDLE_1;

	tuner_ctx_init();

	/*
	 * Step init :
	 * 1st step : init ETAL
	 * Then tuner 1
	 * Tune freq on first Tuner
	 * Then Tuner 2
	 * then DCOP
	 */

	/*
	 * Initialize ETAL
	 */
	TRACE_INFO("ETAL Initializing\n");
	memset(&init_params, 0x0, sizeof(init_params));
	init_params.m_cbNotify = tuner_user_notif_handler;
	init_params.m_tunerAttr[0].m_isDisabled = true;
	init_params.m_tunerAttr[1].m_isDisabled = true;
	init_params.m_DCOPAttr.m_isDisabled = true;

	ret = etal_initialize(&init_params);
	if (ret != ETAL_RET_SUCCESS) {
		TRACE_ERR("etal_initialize failed (%d)\n", ret);
		goto exit;
	}

	/*
	 * Get CMOST FWM in partition
	 */
	if (fwm_in_partition) {
		if (tuner_FWM.fwm_address) {
			TRACE_INFO("FWM partition existing\n");

			init_params.m_tunerAttr[0].m_useDownloadImage = true;
			init_params.m_tunerAttr[0].m_DownloadImage = tuner_FWM.fwm_address;
			init_params.m_tunerAttr[0].m_DownloadImageSize = tuner_FWM.fwm_size;
			read_image_ok = true;

		} else {
			TRACE_ERR("FWM partition not existing\n");
		}
	}

	/*
	 * Initialize Tuner 1
	 */
	if (tuner1_init) {
		TRACE_INFO("Tuner 1 initializing\n");

		init_params.m_tunerAttr[0].m_isDisabled = false;

		ret = etal_tuner_initialize(0, &init_params.m_tunerAttr[0],
					    false);
		if (ret != ETAL_RET_SUCCESS) {
			TRACE_ERR("etal_tuner_initialize (%d)\n", ret);
			goto exit;
		}

		TRACE_INFO("Tuner 1 initialized\n");
	}

	/*
	 * Tune Tuner 1
	 */
	if (tuner_backup_magic_request() == 1) {
		tuner_backup_read_data(&tnr_ctx);
	} else {
		tuner_backup_write_magic();
		tnr_ctx.Freq = 0;
		tnr_ctx.Band = ETAL_BAND_UNDEF;
		tuner_backup_write_data(&tnr_ctx);
	}

	if (tnr_ctx.Band & ETAL_BAND_FM_BIT)
		ret = tuner_amfm_tune(tnr_ctx.Freq, vl_duration);

	/*
	 * Initialize Tuner 2
	 */
	if (tuner2_init) {
		TRACE_INFO("Tuner 2 initializing\n");

		init_params.m_tunerAttr[1].m_isDisabled = false;
		if (read_image_ok) {
			init_params.m_tunerAttr[1].m_useDownloadImage = true;
			init_params.m_tunerAttr[1].m_DownloadImage = tuner_FWM.fwm_address;
			init_params.m_tunerAttr[1].m_DownloadImageSize = tuner_FWM.fwm_size;
		}

		ret = etal_tuner_initialize(1, &init_params.m_tunerAttr[1],
					    false);
		if (ret != ETAL_RET_SUCCESS) {
			TRACE_ERR("etal_tuner_initialize (%d)\n", ret);
			goto exit;
		}

		TRACE_INFO("Tuner 2 initialized\n");
	}

	if (dcop_reset) {
		TRACE_INFO("DCOP resetting\n");

		init_params.m_DCOPAttr.m_isDisabled = false;

		ret = etal_dcop_initialize(&init_params.m_DCOPAttr,
					   ETAL_DCOP_INIT_RESET_ONLY);
		if (ret != ETAL_RET_SUCCESS) {
			TRACE_ERR("etal_dcop_initialize (%d)\n", ret);
			goto exit;
		}

		TRACE_INFO("DCOP reset done\n");
	}

exit:

	TRACE_INFO("Call etal_deinitialize\n");
	(void)etal_deinitialize();

	/* Release i2C for A7 */
	while (rdm_relase_device("tuner_i2c") == -EAGAIN)
		vTaskDelay(pdMS_TO_TICKS(500));

	/* msg treatment of backup information from tuner */
	while (1) {
		ret = tuner_get_msg(&tnr_ctx);
		if (ret == -EAGAIN) {
			/* receive queue not ready */
			vTaskDelay(pdMS_TO_TICKS(100));
		} else if (!ret) {
			/* ret = 0, Message received */
			if (tnr_ctx.Band != ETAL_BAND_UNDEF)
				tuner_backup_write_data(&tnr_ctx);
		} else {
			/* No message - Nothing to do */
		}
	}

	TRACE_INFO("Service ended\n");

	vTaskDelete(NULL);
}
