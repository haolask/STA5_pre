/**
 * @file sta_pm.c
 * @brief Provide all the Power Management functions and tasks
 * for the STA SoC.
 *
 * Copyright (C) ST-Microelectronics SA 2017
 * @author: Jean-Nicolas GRAUX <jean-nicolas.graux@st.com>
 */
#include <errno.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "sta_common.h"
#include "sta_type.h"
#include "sta_ddr.h"
#include "sta_pm.h"
#include "sta_pmu.h"
#include "sdmmc.h"
#include "sta_src.h"
#include "sta_src_a7.h"
#include "sta_mbox.h"
#include "sta_rpmsg.h"
#include "sta_m3_irq.h"
#include "sta_nvic.h"
#include "sta_mtu.h"
#include "sta_rtc.h"
#include "sta_mtu.h"
#include "sta_image.h"
#include "sta_a7.h"

#include "MessageQCopy.h"
#include "NameMap.h"

#include "sta_mem_map.h"
#include "utils.h"

/*
 * Update PM definitions with extrem care.
 * Since this must stay aligned with what is defined in the application
 * processor side which can the ARM Trusted Firmware if secure boot is enabled,
 * or U-boot and Linux if not.
 */
#define PM_MSG_APXL_REBOOT	0xFFFF0001
#define PM_MSG_SUSPEND		0xFFFF0002
#define PM_MSG_SYSTEM_OFF	0xFFFF0003
#define PM_MSG_SYSTEM_RST	0xFFFF0004
#define PM_MSG_HIBERNATE	0xFFFF0005

#define PM_SUSPEND_MAGIC_ADDR	BACKUP_RAM_STR_TAG_BASE
#define PM_STR_MAGIC		0x53545200
#define PM_STD_MAGIC		0x53544400
#define PM_RESUME_MAGIC_ADDR	BACKUP_RAM_STR_RESUME_ADDR_BASE

/**
 * struct pm_host_status - Front-end PM driver status (running in AP)
 * @goto_sby:		to know wheter Go To STAND-BY request came from front-end PM driver.
 * @reboot:		to know wheter reboot request came from front-end PM driver.
 * @sby_notify:		to remember whether host asked to be notified whenever a Go To
 *			STAND-BY event occur.
 * @sby_ready:		to know when host is ready for a move to STAND-BY state.
 * @reboot_notify:	to remember whether host asked to be notified whenever a software
 *			reboot event occur on Cortex-M3 side.
 * @reboot_ready:	to know when host is ready for reboot.
 */
struct pm_host_status {
	bool goto_sby;
	bool reboot;
	bool sby_notify;
	bool sby_ready;
	bool reboot_notify;
	bool reboot_ready;
};

/**
 * struct pm_status - Power and Reset Manager status
 * @goto_sby:			To know wheter there a pending move to STAND-BY state.
 * @reboot:			To know whether there a pending reboot request occur.
 * @pmu_stat			PMU status.
 * @host_stat:			Front-end host status.
 * @sby_notifier_list:		To store callbacks of users to notify whenever a Go To
 *				STAND-BY event/request occur.
 * @reboot_notifier_list:	To store callbacks of users to notify whenever a reboot
 *				request occur.
 * @remote_endpoint:		RPMSG remote endpoint.
 * @endpoint:			RPMSG local endpoint.
 * @buffer:			buffer to store RMPSG data content.
 */
struct pm_status {
	bool goto_sby;
	bool reboot;
	struct pm_host_status host_stat;
	xList sby_notifier_list;
	xList reboot_notifier_list;
	uint32_t remote_endpoint;
	uint32_t endpoint;
	char buffer[RP_MSG_PAYLOAD_SIZE];
	uint8_t uart_console_no;
};

static struct pm_status pm_stat;

static inline void pm_str_write_magic(uint32_t resume_addr)
{
	write_reg(PM_STR_MAGIC, PM_SUSPEND_MAGIC_ADDR);
	write_reg(resume_addr, PM_RESUME_MAGIC_ADDR);
}

static inline void pm_std_write_magic(uint32_t resume_addr)
{
	write_reg(PM_STD_MAGIC, PM_SUSPEND_MAGIC_ADDR);
	write_reg(resume_addr, PM_RESUME_MAGIC_ADDR);
}

static inline unsigned char pm_str_check_magic(void)
{
	uint32_t val = read_reg(PM_SUSPEND_MAGIC_ADDR);

	return (val == PM_STR_MAGIC ? true : false);
}

#ifdef NO_SCHEDULER
#define pm_ramoops_backup(x) do {} while (0)
#define pm_ramdump_backup(x) do {} while (0)
#else
static inline void pm_ramoops_backup(struct sta *context)
{
#if !defined(DDRAM_RAMDUMP_MAGIC_BASE) && defined(DDRAM_RAMOOPS_RAMOOPS_MEM_BASE)

	uint32_t ramoops_addr;
	uint32_t ramoops_size;

	/* Check if rammoops production condition
	 * Reboot asked by A7
	 * Area DDRAM_RAMOOPS_RAMOOPS_MEM_BASE fill with payload
	 */
	if (!context)
		return;

	if (!read_reg(DDRAM_RAMOOPS_RAMOOPS_MEM_BASE + 4))
		return;

	if (get_boot_dev_type() == BOOT_MMC) {
		/* release the current mmc session in order
		 * to force the mmc ctrl initialisation
		 */
		sdmmc_remove(context->mmc_ctx);
		if (!sdmmc_init(get_mmc_boot_dev(), true)) {
			TRACE_ERR("Can't Init SDMMC\n");
			return;
		}
	}

	if (!get_partition_info(AP_RAMDUMP_ID, &ramoops_addr, &ramoops_size)) {
		ramoops_addr *= MMC_BLOCK_SIZE;
		write_raw_binary(context, ramoops_addr,
				 ramoops_size, DDRAM_RAMOOPS_RAMOOPS_MEM_BASE,
				 DDRAM_RAMOOPS_RAMOOPS_MEM_SIZE);
	} else
		TRACE_ERR("%s: missing AP_RAMDUMP partition information\n",
			  __func__);
#endif
}

#define RAMDUMP_MAGIC 0x444D4152
static inline void pm_ramdump_backup(struct sta *context)
{
#if defined(DDRAM_RAMDUMP_MAGIC_BASE)
	uint32_t ramdump_addr;
	uint32_t ramdump_size;

	/* Check if rammoops production condition
	 * Reboot asked by A7
	 * Area DDRAM_RAMOOPS_RAMOOPS_MEM_BASE fill with payload
	 */
	if (!context)
		return;

	if (read_reg(DDRAM_RAMDUMP_MAGIC_BASE) != RAMDUMP_MAGIC)
		return;

	if (get_boot_dev_type() == BOOT_MMC) {
		/* release the current mmc session in order
		 * to force the mmc ctrl initialisation
		 */
		sdmmc_remove(context->mmc_ctx);
		if (!sdmmc_init(get_mmc_boot_dev(), true)) {
			TRACE_ERR("Can't Init SDMMC\n");
			return;
		}
	}

	if (!get_partition_info(AP_RAMDUMP_ID, &ramdump_addr, &ramdump_size)) {
		ramdump_addr *= MMC_BLOCK_SIZE;
		write_raw_binary(context, ramdump_addr,
				 ramdump_size, DDRAM_APP_OS_SYSTEM_MEM_BASE,
				 DDRAM_APP_OS_SYSTEM_MEM_SIZE);
	} else
		TRACE_ERR("%s: missing AP_RAMDUMP partition information\n",
			  __func__);
#endif
}
#endif

void pm_shutdown_prepare(void *context)
{
	/* we do not want to be interrupted */
	taskENTER_CRITICAL();

	/* Move A7 sub-system in reset */
	a7_stop();

	/*
	 * Since SRC peripheral itself is not reset on a SoC
	 * reset, be safe and re-enable clock gates before issuing
	 * low level reboot sequence.
	 * This is to prevent Rom-code or anybody else
	 * accessing a peripheral and assuming that corresponding clock is ON.
	 */
	srcm3_pclk_enable_all(src_m3_regs);
	srca7_pclk_enable_all(src_a7_regs);

	/* work-around for cut3 rom issue */
	if (get_boot_dev_type() == BOOT_MMC)
		sdmmc_reset(get_mmc_boot_dev());

	pm_ramoops_backup((struct sta *)context);
	pm_ramdump_backup((struct sta *)context);

	/* work-around for cut3 rom issue */
	if (get_boot_dev_type() == BOOT_MMC)
		sdmmc_reset(get_mmc_boot_dev());

	if (pm_str_check_magic()) {
		sta_ddr_suspend();
	}

	sta_ddr_core_reset();
	sta_ddr_axi_reset();

	trace_flush();
}

void __pm_reboot(char *reboot_info, void *context)
{
	if (!reboot_info)
		TRACE_NOTICE("%s: No reboot information\n", __func__);
	else
		TRACE_NOTICE("%s: %s\n", __func__, reboot_info);

	pm_shutdown_prepare(context);

#if SRC_RESET_METHOD
	/**
	 * The src-m3 sw reset method that requires some software
	 * work-around to be stable.
	 */
	srcm3_reset(src_m3_regs);
#else
	/**
	 * The hardware reset method that consists in relying on
	 * PMU goto SBY and wake-up from RTC.
	 */
	rtc_init();
	udelay(1000);
	rtc_set_timeout(2);
	udelay(1000);
	pmu_goto_sby();
#endif
}

static void __pm_sby(void)
{
	pm_shutdown_prepare(NULL);
	pmu_goto_sby();
}

/**
 * pm_handle_goto_sby() function is periocally called by pm_task
 * to check whether we need to move to STAND-BY state.
 * On a hardware/software Go To STAND-BY event, we may also enter
 * this function several times, until SoC effectively move
 * to STAND-BY state.
 */
static void pm_handle_goto_sby(void)
{
	uint16_t len;
	static bool host_notified = false;
	static bool notifier_list_parsed = false;

	if (!pm_stat.goto_sby)
		return;

	/**
	 * optionally notify the host (Linux front-end driver
	 * running in AP side) in the case:
	 * - it asked to be notified on a Go To STAND-BY event and
	 *   it has not been notified yet
	 * and:
	 * - it is not the one who initiated the Go To STAND-BY event.
	 * - it did not ask for reboot yet.
	 *
	 */
	if (pm_stat.host_stat.sby_notify && !host_notified &&
			!pm_stat.host_stat.goto_sby &&
			!pm_stat.host_stat.reboot) {

		struct pm_msg_hdr *hdr = (struct pm_msg_hdr *)pm_stat.buffer;
		struct pm_msg_sby * msg = (struct pm_msg_sby *)hdr->data;

		/* Format SBY message */
		hdr->type = PM_MSG_SBY;
		hdr->len = sizeof(*msg);
		len = sizeof(*hdr) + hdr->len;

		msg->sby_status = pmu_get_sby_stat();

		MessageQCopy_send(pm_stat.remote_endpoint, pm_stat.endpoint,
				(void *)pm_stat.buffer, len, portMAX_DELAY);

		host_notified = true;
	}

	/* parse notifier list to advise any user Cortex-M3 side */
	if (!listLIST_IS_EMPTY(&pm_stat.sby_notifier_list) &&
			!notifier_list_parsed) {

		struct pm_notifier *notifier;
		while(listLIST_IS_EMPTY(&(pm_stat.sby_notifier_list)) == false) {
			notifier = listGET_OWNER_OF_HEAD_ENTRY(
					&(pm_stat.sby_notifier_list));
			uxListRemove(&(notifier->list_item));
			notifier->call_back();
		}
		notifier_list_parsed = true;
	}

	/**
	 * Now, let's see if we can move SoC to standby state.
	 * If:
	 * - host asked to be notified and answered that it is ready,
	 * - or host is the initiator of Go To STAND-BY event,
	 * - or host do not bother with Go To STAND-BY event,
	 * then let's move to STAND-BY state.
	 */
	if ((pm_stat.host_stat.sby_notify && pm_stat.host_stat.sby_ready) ||
		pm_stat.host_stat.goto_sby ||
		!pm_stat.host_stat.sby_notify) {

		__pm_sby();
	}
}

static void pm_handle_reboot(void *cookie)
{
	uint16_t len;
	static bool host_notified = false;
	static bool notifier_list_parsed = false;

	if (!pm_stat.reboot)
		return;

	/**
	 * optionally notify the host (Linux front-end driver
	 * running in AP side) in the case:
	 * - it asked to be notified on a reboot event and
	 *   it has not been notified yet
	 * and:
	 * - it is not the one who initiated reboot event.
	 * - it did not ask for going to STAND-BY yet.
	 *
	 */
	if (pm_stat.host_stat.reboot_notify && !host_notified &&
			!pm_stat.host_stat.reboot &&
			!pm_stat.host_stat.goto_sby) {

		struct pm_msg_hdr *hdr = (struct pm_msg_hdr *)pm_stat.buffer;

		/* Format REBOOT message */
		hdr->type = PM_MSG_REBOOT;
		hdr->len = 0;
		len = sizeof(*hdr);

		MessageQCopy_send(pm_stat.remote_endpoint, pm_stat.endpoint,
				(void *)pm_stat.buffer, len, portMAX_DELAY);

		host_notified = true;
	}

	/* parse notifier list to advise any user Cortex-M3 side */
	if (!listLIST_IS_EMPTY(&pm_stat.reboot_notifier_list) &&
			!notifier_list_parsed) {

		struct pm_notifier *notifier;
		while(listLIST_IS_EMPTY(&(pm_stat.reboot_notifier_list)) == false) {
			notifier = listGET_OWNER_OF_HEAD_ENTRY(
					&(pm_stat.reboot_notifier_list));
			uxListRemove(&(notifier->list_item));
			notifier->call_back();
		}
		notifier_list_parsed = true;
	}

	/**
	 * Now, let's see if we can reboot the SoC.
	 * If:
	 * - host asked to be notified and answered that it is ready,
	 * - or host is the initiator of reboot event,
	 * - or host do not bother with reboot event,
	 * then let's reboot.
	 */
	if ((pm_stat.host_stat.reboot_notify &&
		pm_stat.host_stat.reboot_ready) ||
		pm_stat.host_stat.reboot ||
		!pm_stat.host_stat.reboot_notify) {
		__pm_reboot("A7 request", cookie);
	}
}

static void pm_send_wkup_status(void)
{
	uint16_t len;
	struct pm_msg_hdr *hdr = (struct pm_msg_hdr *)pm_stat.buffer;
	struct pm_msg_wkup_stat *msg = (struct pm_msg_wkup_stat *)hdr->data;

	/* Format SBY message */
	hdr->type = PM_MSG_WKUP_STAT;
	hdr->len = sizeof(*msg);
	len = sizeof(*hdr) + hdr->len;

	msg->wkup_status = pmu_get_wkup_stat();
	MessageQCopy_send(pm_stat.remote_endpoint, pm_stat.endpoint,
			(void *)pm_stat.buffer, len, portMAX_DELAY);
}

static void pm_update_wkup_src(void)
{
	struct pm_msg_hdr *hdr = (struct pm_msg_hdr *)pm_stat.buffer;
	struct pm_msg_upd_wkup_src *msg = (struct pm_msg_upd_wkup_src *)hdr->data;

	if (msg->enable_src & msg->disable_src) {
		/* should not occur */
		TRACE_ERR("%s: warning, same source to enable/disable\n", __func__);
		return;
	}
	if (msg->rising_edge & msg->falling_edge) {
		/* should not occur */
		TRACE_ERR("%s: warning, same active rising/falling edge to select\n", __func__);
		return;
	}

	pmu_enable_wkup_src(msg->enable_src);
	pmu_disable_wkup_src(msg->disable_src);
	pmu_enable_wkup_edge(msg->rising_edge);
	pmu_disable_wkup_edge(msg->falling_edge);
}

static void pm_update_sby_src(void)
{
	struct pm_msg_hdr *hdr = (struct pm_msg_hdr *)pm_stat.buffer;
	struct pm_msg_upd_sby_src *msg = (struct pm_msg_upd_sby_src *)hdr->data;

	if (msg->enable_src & msg->disable_src) {
		/* should not occur */
		TRACE_ERR("%s: warning, same source to enable/disable\n", __func__);
		return;
	}
	if (msg->rising_edge & msg->falling_edge) {
		/* should not occur */
		TRACE_ERR("%s: warning, same active rising/falling edge to select\n", __func__);
		return;
	}

	pmu_enable_sby_src(msg->enable_src);
	pmu_disable_sby_src(msg->disable_src);
	pmu_enable_sby_edge(msg->rising_edge);
	pmu_disable_sby_edge(msg->falling_edge);
	pmu_enable_sby_irq(msg->enable_irq);
	pmu_disable_sby_irq(msg->disable_irq);
}

/* Get magic tag in backup RAM */
void pm_suspend_get_boot_mode(void)
{
	uint32_t val = read_reg(PM_SUSPEND_MAGIC_ADDR);

	switch (val) {
	case PM_STR_MAGIC:
		set_boot_mode(STR_BOOT);
		break;

	case PM_STD_MAGIC:
		set_boot_mode(STD_BOOT);
		break;

	default:
		set_boot_mode(COLD_BOOT);
		break;
	}
	/* Clear magic tag in backup RAM */
	write_reg(0xDEADBEEF, PM_SUSPEND_MAGIC_ADDR);
}

unsigned char pm_str_check_resume(void)
{
	return (get_boot_mode() == STR_BOOT ? true : false);
}

unsigned char pm_std_check_resume(void)
{
	return (get_boot_mode() == STD_BOOT ? true : false);
}

#ifdef ATF
static inline void pm_suspend_save_secure_mem(void)
{
#if defined(ESRAM_A7_ATF_TRUSTED_ZONE_BASE)
	memcpy((void *)DDRAM_ATF_BL32_SAVED_BASE,
	       (void *)ESRAM_A7_ATF_TRUSTED_ZONE_BASE,
	       ESRAM_A7_ATF_TRUSTED_ZONE_SIZE);
#endif
	write_reg(PM_STR_MAGIC, DDRAM_ATF_BL32_SAVED_BASE +
		  DDRAM_ATF_BL32_SAVED_SIZE - BACKUP_RAM_STR_TAG_SIZE);
	write_reg(pm_get_ap_resume_entry(), DDRAM_ATF_BL32_SAVED_BASE +
		 DDRAM_ATF_BL32_SAVED_SIZE - BACKUP_RAM_STR_TAG_SIZE -
		 sizeof(void *));
}

void pm_suspend_restore_secure_mem(void)
{
#if defined(ESRAM_A7_ATF_TRUSTED_ZONE_BASE) && defined(DDRAM_ATF_BL32_SAVED_BASE)
	/* FIXME: maybe use dma memcpy to improve things ? */
	memcpy((void *)ESRAM_A7_ATF_TRUSTED_ZONE_BASE,
	       (void *)DDRAM_ATF_BL32_SAVED_BASE,
	       ESRAM_A7_ATF_TRUSTED_ZONE_SIZE);
#endif
}

uint32_t pm_get_ap_resume_entry(void)
{
	if (get_boot_mode() == STD_BOOT) {
		if (read_reg(DDRAM_ATF_BL32_SAVED_BASE +
				DDRAM_ATF_BL32_SAVED_SIZE -
				BACKUP_RAM_STR_TAG_SIZE) != PM_STR_MAGIC)
			return PM_STR_RESUME_NOT_VALID;
		else
			return read_reg(DDRAM_ATF_BL32_SAVED_BASE +
					DDRAM_ATF_BL32_SAVED_SIZE -
					BACKUP_RAM_STR_TAG_SIZE -
					sizeof(void *));
	} else
		return read_reg(PM_RESUME_MAGIC_ADDR);
}

static inline void pm_std_snapshot_secure_mem(void *context)
{
	uint32_t addr;
	uint32_t size;

	sta_ddr_std_check(false);

	srcm3_pclk_enable_all(src_m3_regs);
	srca7_pclk_enable_all(src_a7_regs);

	get_std_info(STD_PART1, &addr, &size);
	if (size)
		write_raw_binary((struct sta *)context, addr, size,
				 DDRAM_ATF_BL32_SAVED_BASE,
				 DDRAM_ATF_BL32_SAVED_SIZE);
	else
		TRACE_ERR("STD snaphot failure, partition:%d info not defined",
			  AP_BL32_SNAPSHOT_ID);

	get_std_info(STD_PART2, &addr, &size);
	if (size)
		write_raw_binary((struct sta *)context, addr, size,
				 DDRAM_ATF_TEE_BASE,
				 DDRAM_ATF_TEE_SIZE);
	else
		TRACE_ERR("STD snaphot failure, partition:%d info not defined",
			  AP_TEE_SNAPSHOT_ID);
}

#endif

static void pm_cb(void *context, struct mbox_msg *msg)
{
	static uint8_t buf[8];
	uint32_t *request = (uint32_t *)buf;
	uint32_t *payload = (uint32_t *)&buf[4];
	uint32_t i;

	/* Get Rx message from Mailbox IPC */
	for (i = 0; i < msg->dsize && i < sizeof(buf); i++) {
		buf[i] = *(msg->pdata + i);
	}

	switch (*request)
	{
	case PM_MSG_APXL_REBOOT:
		/* AP xl flashloader low level reboot request */
		__pm_reboot("Flashloader reboot", NULL);
		break;
	case PM_MSG_HIBERNATE:
	case PM_MSG_SUSPEND:
		trace_init(pm_stat.uart_console_no, NULL, 0);
		/*
		 * Write MAGIC ID and the received address of platform low
		 * level resume entry function in backup-ram to tell bootloader
		 * to switch to pm-resume on next wake-up from stanby and
		 * figure out entry code where C-A7 should resume.
		 */
		pm_str_write_magic(*payload);

#ifdef ATF
		/*
		 * Save ATF/SP_MIN memory context in DDR so that it can
		 * be restored by the M3 xloader at next resume time.
		 */
		pm_suspend_save_secure_mem();

		if (*request == PM_MSG_HIBERNATE) {
			pm_std_write_magic(*payload);
			pm_std_snapshot_secure_mem(context);
		}
#endif
		/* fall through ... */
	case PM_MSG_SYSTEM_OFF:
		trace_init(pm_stat.uart_console_no, NULL, 0);
		if (pm_stat.goto_sby || pm_stat.reboot) {
			/*
			 * A goto standby or a reboot request from M3 OS is in
			 * progress. Let's store the host AP OS acknowlegde
			 * which shall be taken into account once pm_task will
			 * be scheduled again.
			 */
			pm_stat.host_stat.sby_ready = true;
			pm_stat.host_stat.reboot_ready = true;
		} else {
			/* trigger the high level goto standby request */
			pm_goto_sby(true);
		}
		break;
	case PM_MSG_SYSTEM_RST:
		trace_init(pm_stat.uart_console_no, NULL, 0);
		if (pm_stat.goto_sby || pm_stat.reboot) {
			/*
			 * A goto standby or a reboot request from M3 OS is in
			 * progress. Let's store the host AP OS acknowlegde
			 * which shall be taken into account once pm_task will
			 * be scheduled again.
			 */
			pm_stat.host_stat.sby_ready = true;
			pm_stat.host_stat.reboot_ready = true;
		} else {
			/* trigger the high level reboot request */
			pm_reboot(true);
		}
		break;
	default:
		TRACE_ERR("%s: unexpected AP_XL request: %d\n", __func__,
			  *request);
		break;
	}
}

int pm_goto_sby(unsigned char bypass_host)
{
	if (pm_stat.reboot) {
		TRACE_INFO("%s: warning, pending reboot!\n", __func__);
		return -1;
	}
	if (pm_stat.goto_sby) {
		TRACE_INFO("%s: warning, pending goto_sby!\n", __func__);
		return -1;
	}
	pm_stat.goto_sby = true;
	pm_stat.host_stat.goto_sby = bypass_host;
	return 0;
}

int pm_reboot(unsigned char bypass_host)
{
	if (pm_stat.reboot) {
		TRACE_INFO("%s: warning, pending reboot!\n", __func__);
		return -1;
	}
	if (pm_stat.goto_sby) {
		TRACE_INFO("%s: warning, pending goto_sby!\n", __func__);
		return -1;
	}
	pm_stat.reboot = true;
	pm_stat.host_stat.reboot = bypass_host;
	return 0;
}

void pm_register_standby(struct pm_notifier *notifier)
{
	/**
	 * do not allow registering in case pending STAND-BY
	 * or reboot is pending.
	 */
	if (pm_stat.reboot || pm_stat.goto_sby)
		return;

	taskENTER_CRITICAL();

	vListInitialiseItem(&(notifier->list_item));

	listSET_LIST_ITEM_OWNER(&(notifier->list_item),
			notifier);
	vListInsertEnd(&(pm_stat.sby_notifier_list),
			&(notifier->list_item));

	taskEXIT_CRITICAL();
}

void pm_register_reboot(struct pm_notifier *notifier)
{
	/**
	 * do not allow registering in case pending STAND-BY
	 * or reboot is pending.
	 */
	if (pm_stat.reboot || pm_stat.goto_sby)
		return;

	taskENTER_CRITICAL();

	vListInitialiseItem(&(notifier->list_item));

	listSET_LIST_ITEM_OWNER(&(notifier->list_item),
			notifier);
	vListInsertEnd(&(pm_stat.reboot_notifier_list),
			&(notifier->list_item));

	taskEXIT_CRITICAL();
}

void pm_task(void *cookie)
{
	MessageQCopy_Handle handle;
	uint16_t len;
	int ret;
	uint32_t request_endpoint = MessageQCopy_ASSIGN_ANY;
	struct feature_set *fs = &((struct sta *)cookie)->features;

	pm_stat.uart_console_no = fs->uart_console_no;

	pm_stat.endpoint = 0;

	/* init RPMSG framework */
	if (RPMSG_Init() != 0)
		goto end;

	/**
	 * 1st argument must be less than
	 * MessageQCopy_MAX_RESERVED_ENDPOINT
	 * or MessageQCopy_ASSIGN_ANY.
	 */
	handle = MessageQCopy_create(request_endpoint, (uint32_t*)&pm_stat.endpoint);
	if (!handle) {
		TRACE_ERR("%s MessageQCopy_create failure\n", __func__);
		goto end;
	}
	NameMap_register("sta1295-prm", pm_stat.endpoint);

	memset(pm_stat.buffer, 0, sizeof(pm_stat.buffer));

	TRACE_INFO("%s: wkup_status=0x%x, wkup_srcs=0x%x, sby_srcs=0x%x\n",
			__func__,
			pmu_get_wkup_stat(),
			pmu_get_wkup_src(),
			pmu_get_sby_src());

	while(1) {

		/* Set maximum data size that I can receive. */
		len = (uint16_t)sizeof(pm_stat.buffer);

		ret = MessageQCopy_recv(handle, (void *)pm_stat.buffer, &len,
				(uint32_t*)&pm_stat.remote_endpoint, 0);

		/* MessageQCopy_recv succeeded ? */
		if (ret == MessageQCopy_S_SUCCESS && len <= RP_MSG_PAYLOAD_SIZE) {
			struct pm_msg_hdr *hdr = (struct pm_msg_hdr *)pm_stat.buffer;

			TRACE_INFO("%s: msg received, type: %d\n", __func__, hdr->type);

			switch (hdr->type) {
			case PM_MSG_GET_WKUP_STAT:
				/* answer with PM_MSG_WKUP_STAT */
				pm_send_wkup_status();
				break;

			case PM_MSG_REGISTER_SBY:
				pm_stat.host_stat.sby_notify = true;
				break;
#ifndef ATF
			case PM_MSG_SBY_READY:
				if (!pm_stat.goto_sby)
					TRACE_ERR("%s: warning: SBY_READY received but no pending GoToSby!\n", __func__);
				pm_stat.host_stat.sby_ready = true;
				break;

			case PM_MSG_SW_GOTO_SBY:
				pm_goto_sby(true);
				break;
#endif
			case PM_MSG_UPD_WKUP_SRC:
				pm_update_wkup_src();
				break;

			case PM_MSG_UPD_SBY_SRC:
				pm_update_sby_src();
				break;

			case PM_MSG_REGISTER_REBOOT:
				pm_stat.host_stat.reboot_notify = true;
				break;
#ifndef ATF
			case PM_MSG_REBOOT_READY:
				if (!pm_stat.reboot)
					TRACE_ERR("%s: warning: REBOOT_READY received but no pending reboot!\n", __func__);
				pm_stat.host_stat.reboot_ready = true;
				break;
			case PM_MSG_DO_REBOOT:
				/* let's store that host asked to reboot */
				pm_reboot(true);
				break;
#endif
			default:
				TRACE_ERR("%s: wrong msg type: %d\n", __func__, hdr->type);
				break;
			}
		}

		/**
		 * recv timeout, check for pending Go To STAND-BY or
		 * reboot event.
		 */
		pm_handle_goto_sby();
		pm_handle_reboot(cookie);

		/* let's sleep during 10ms */
		vTaskDelay(pdMS_TO_TICKS(10));
	}
end:
	vTaskDelete(NULL);
}

void PMU_IRQHandler(void)
{
	pmu_clear_sby();

	/**
	 * If nobody to notify on Cortex-M3 side and
	 * ( host did not registered the Go To STAND-BY event or
	 * host already initiated a Go To STAND-BY request ),
	 * then we can move directly to STAND-BY state.
	 */
	if (listLIST_IS_EMPTY(&pm_stat.sby_notifier_list) &&
			(!pm_stat.host_stat.sby_notify || pm_stat.host_stat.goto_sby)) {
		/* nobody is registered */
		pmu_goto_sby();
	}

	/**
	 * advise PM task and host that there is pending
	 * Go To STAND-BY event.
	 */
	pm_goto_sby(false);
}

/**
 * @brief	PM init
 * @return	0 if no error, not 0 otherwise
 */
int pm_init(void *context)
{
	struct mbox_chan_req mbreq;
	struct nvic_chnl irq_chnl;

#ifdef ATF
	mbreq.chan_name = "psci";
#else
	mbreq.chan_name = "pm";
#endif
	mbreq.user_data = context;
	mbreq.rx_cb = pm_cb;

	/* Open a mbox channel for remote PM request */
	if (mbox_request_channel(&mbreq) == MBOX_ERROR) {
		TRACE_INFO("%s: failed to allocate mbox channel\n", __func__);
		return -ENODEV;
	}

	/* initialise STAND-BY and reboot notifier lists */
	vListInitialise(&pm_stat.sby_notifier_list);
	vListInitialise(&pm_stat.reboot_notifier_list);

	/* setup NVIC to catch PMU interrupt */
	irq_chnl.id = PMU_IRQChannel;
	irq_chnl.preempt_prio = IRQ_LOW_PRIO;
	irq_chnl.enabled = true;
	nvic_chnl_init(&irq_chnl);

	return 0;
}

