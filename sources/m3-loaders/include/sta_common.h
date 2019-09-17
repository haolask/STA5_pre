/**
 * @file sta_common.h
 * @brief main header file
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#ifndef __STA_COMMON_H__
#define __STA_COMMON_H__

#include "sta_type.h"    /* For uint*_t and bool types */
#include "sta_map.h"     /* SoC register map definitions */

#ifndef _SHARED_DATA
extern struct shared_data_bi_t *p_shared_data_bi;
#define _SHARED_DATA        p_shared_data_bi
#define _SHARED_DATA_RW     &shared_data.rw
#endif
#include "shared_data.h" /* Variables shared between M3 and AP processors */

#include "sta_mem_map.h" /* Memory mapping definitions */
#include "sta_audio.h"
#include "utils.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"

/* DRAM Trusted area specified as valid external memory range */
#ifdef DDRAM_ATF_TRUSTED_ZONE_BASE
#define EXT_MEM_RANGE_INOUT_START	DDRAM_ATF_TRUSTED_ZONE_BASE
#define EXT_MEM_RANGE_INOUT_END		(DDRAM_ATF_TRUSTED_ZONE_BASE +\
					DDRAM_ATF_TRUSTED_ZONE_SIZE - 1)
#else
/* No trusted external DRAM defined so only internal eSRAM is specified */
#define EXT_MEM_RANGE_INOUT_START	ESRAM_A7_BASE
#define EXT_MEM_RANGE_INOUT_END		(ESRAM_A7_BASE + ESRAM_A7_SIZE - 1)
#endif

/* Shared REE-TEE memory area specified as valid external memory range */
#ifdef DDRAM_TEE_SHMEM_BASE
#define EXT_MEM_RANGE_INPUT_START	DDRAM_TEE_SHMEM_BASE
#define EXT_MEM_RANGE_INPUT_END		(DDRAM_TEE_SHMEM_BASE +\
					DDRAM_TEE_SHMEM_SIZE - 1)
#else
/* No Shared REE-TEE memory area specified then reuse trusted area */
#define EXT_MEM_RANGE_INPUT_START	EXT_MEM_RANGE_INOUT_START
#define EXT_MEM_RANGE_INPUT_END		EXT_MEM_RANGE_INOUT_END
#endif

/*  A7 SHE registers memory mapping */
#ifdef ESRAM_A7_ATF_HSM_MAILBOXES_BASE
#define A7_SHE_REG_MEM_START		ESRAM_A7_ATF_HSM_MAILBOXES_BASE
#else
/* A7 SHE registers memory mapping. Keep default base address */
#endif

#ifndef SOC_ID
#define SOC_ID SOCID_STA1295
#endif

#ifndef BOARD_ID
#define BOARD_ID BOARD_A5_EVB
#endif

#define PLATFORM_NAME	"Accordo5/Telemaco3-M3"
#define BIN_NAME_XL	"xloader"
#define BIN_NAME_OS	"app os"
#define BIN_NAME_XLU	"xl_uflashloader"
#define BIN_NAME_M3MIN	"m3 mini"

#define wait_forever for(;;)

/**
 * enum task_priorities - Here is the place to start FreeRTOS tasks.
 *
 * The Task dedicated to Power and Reset Management should always have
 * the highest priority.
 */
enum task_priorities {
	TASK_PRIO_IDLE = tskIDLE_PRIORITY,
	TASK_PRIO_WDT_M3,
#ifdef CLI_ENABLED
	TASK_PRIO_CLI,
#endif
	/* do NOT add any new task priorities before this line */
	TASK_PRIO_TEST_HSM,
	TASK_PRIO_TEST_MM,
	TASK_PRIO_TEST_SPLASH_ANIM = TASK_PRIO_TEST_MM,
	TASK_PRIO_BOOTFLOW,
	/* ... */
	TASK_PRIO_TEST_RPMSG,
	TASK_PRIO_TEST_RPMSG_MM = TASK_PRIO_TEST_RPMSG,
	TASK_PRIO_RPMSG_RDM = TASK_PRIO_TEST_RPMSG,
	TASK_PRIO_KS,
	TASK_PRIO_TEST_CCC,
	TASK_PRIO_TEST_ACC,
	TASK_PRIO_TEST_RVC_CONTROL = TASK_PRIO_TEST_ACC,
	TASK_PRIO_TEST_RVC,
	TASK_PRIO_TEST_MAILBOX,
#ifdef MULTIPLE_SQINOR_TEST_TASKS
	TASK_PRIO_TEST_SQI_2,
	TASK_PRIO_TEST_SQI_1,
#endif
	TASK_PRIO_BRD_CFG,
	TASK_PRIO_TEST_SQI,
	TASK_PRIO_TEST_I2C,
	TASK_PRIO_TEST_CAN,
	/* ... */
	/* do NOT add any new task priorities after this line */
	TASK_PRIO_WDT_A7,
	TASK_PRIO_PM,
	TASK_PRIO_MAX = TASK_PRIO_PM,
};

struct feature_set {
	uint8_t no_gfx;
	uint8_t can0; /* if enabled, device must be disabled in Linux side */
	uint8_t can0_hello; /* if enabled, send a hello frame at startup */
	uint8_t accelerometer;
	uint8_t rpmsg_css;
	uint8_t rpmsg_mm;
	uint8_t rpmsg_rdm;
	uint8_t mbox_test;
	uint8_t ccc_test;
	uint8_t hsm_test;
	uint8_t ks;
	uint8_t uart_console_no;
	uint8_t modem;
	uint8_t splash_screen;
	uint8_t splash_animation;
	uint8_t rear_camera;
	uint8_t aux_detect; /* gpio associated to aux detect. Set to -1 if not supported */
	uint8_t wdt; /* To enable watchdog */
	uint8_t hdmi; /* hdmi support on front panel */
	uint8_t early_audio;
	uint8_t remote_gpio;
#ifdef EARLY_TUNER_FTR
	uint8_t early_tuner;
#endif
};

/* Timeout used for synchronization of tasks for shared resource management */
#define TASK_SYNC_TIMEOUT_MS 5000

/* Define here the bit to be used for each task making use of shared resources
 * with A7 at boot time.
 *
 * In order to avoid conflicts with A7, each task should:
 *  - define a unique bit hereafter
 *  - set this bit in "event_bitmask" when task has been created successfully
 *  example:
 *      err = xTaskCreate(...);
 *      if (err == pdPASS)
 *          ctxt->event_bitmask |= RVC_TASK_EVENT_BIT;
 *  - call "xEventGroupSetBits" after making use of shared resources
 */
#define RVC_TASK_EVENT_BIT BIT(0)

struct sta {
	const char *bin;
	const char *boot_dev_name;
	int trace_port;
	struct feature_set features;
	struct audio_context audio_context;
	struct t_sqi_ctx *sqi_ctx;
	void *mmc_ctx;
	EventBits_t event_bitmask;
	EventGroupHandle_t event_group;
};

int m3_lowlevel_core_setup(void);

int m3_core_setup(struct sta *context);

void m3_get_mmc_boot_dev(struct sta *context, int mmc_forced);

int m3_boot_dev_setup(struct sta *context);

void m3_init_trace(struct sta *context);

int m3_ipc_setup(struct sta *context);

uint32_t m3_get_soc_temperature(uint32_t timeout);

void m3_set_ap_sync_point(uint8_t sync);

#endif /* __STA_COMMON_H__ */
