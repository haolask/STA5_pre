/**
 * @file sta_pmu.h
 * @brief Provide all the sta Power Management definitions.
 *
 * Copyright (C) ST-Microelectronics SA 2017
 * @author: Jean-Nicolas GRAUX <jean-nicolas.graux@st.com>
 */

#ifndef _STA_PM_H_
#define _STA_PM_H_

/**
 * enum pm_msg_types - various message types currently supported
 */
enum pm_msg_types {
	PM_MSG_GET_WKUP_STAT, /* front-end driver -> PM driver */
	PM_MSG_WKUP_STAT, /* PM driver -> front-end driver */
	PM_MSG_REGISTER_SBY, /* front-end driver -> PM driver */
	PM_MSG_SBY, /* PM driver -> front-end driver */
	PM_MSG_SBY_READY, /* front-end driver -> PM driver */
	PM_MSG_SW_GOTO_SBY, /* front-end driver -> PM driver */
	PM_MSG_UPD_WKUP_SRC, /* front-end driver -> PM driver */
	PM_MSG_UPD_SBY_SRC, /* front-end driver -> PM driver */
	PM_MSG_REGISTER_REBOOT, /* front-end driver -> PM driver */
	PM_MSG_REBOOT, /* PM driver -> front-end driver */
	PM_MSG_REBOOT_READY, /* front-end driver -> PM driver */
	PM_MSG_DO_REBOOT, /* front-end driver -> PM driver */
};

#define PM_STR_RESUME_NOT_VALID 0xFFFFFFFF

struct pm_msg_hdr {
	uint32_t type;
	uint32_t len;
	char data[0];
} __attribute__ ((packed));

/**
 * struct pm_msg_sby - PM_MSG_SBY notification message
 *
 * Used to notify host about pending STAND-BY.
 * message direction: from PM back-end driver to front-end PM driver (host).
 *
 * @sby_status:	current value of PMU ON to STAND-BY status register.
 */
struct pm_msg_sby {
	uint32_t sby_status;
} __attribute__ ((packed));

/**
 * struct pm_msg_wkup_stat - PM_MSG_WKUP_STAT notification message
 *
 * Sent by back-end driver to answer to PM_MSG_GET_WKUP_STAT.
 * message direction: from PM back-end driver to front-end PM driver (host).
 *
 * @wkup_status: content of STANDBY to ON status register (red at boot time).
 */
struct pm_msg_wkup_stat {
	uint32_t wkup_status;
} __attribute__ ((packed));

/**
 * struct pm_msg_upd_wkup_src - PM_MSG_UPD_WKUP_SRC request message
 *
 * Sent by host to update wake-up sources.
 * message direction: from front-end PM driver (host) to PM back-end driver.
 *
 * @enable_src: wake-up sources to enable.
 * @disable_src: wake-up sources to disable.
 * @rising_edge: to select active edges as rising edges.
 * @falling_edge: to select active edges as falling edges.

 */
struct pm_msg_upd_wkup_src {
	uint32_t enable_src;
	uint32_t disable_src;
	uint32_t rising_edge;
	uint32_t falling_edge;
} __attribute__ ((packed));

/**
 * struct pm_msg_upd_sby_src - PM_MSG_UPD_SBY_SRC request message
 *
 * Sent by host to update STAND-BY sources.
 * message direction: from front-end PM driver (host) to PM back-end driver.
 *
 * @enable_src: STAND-BY sources to enable.
 * @disable_src: STAND-BY sources to disable.
 */
struct pm_msg_upd_sby_src {
	uint32_t enable_src;
	uint32_t disable_src;
	uint32_t rising_edge;
	uint32_t falling_edge;
	uint32_t enable_irq;
	uint32_t disable_irq;
} __attribute__ ((packed));

/**
 * Below are the definitions that are ONLY specific to back-end driver.
 */

struct pm_notifier {
	xListItem list_item;
	void (*call_back)(void);
};

/* should NOT be called by any task except by WDT task */
void __pm_reboot(char *reboot_info, void *cookie);

/* PM API */

void pm_task(void *cookie);
/* to register a user on Go To STAND-BY event */
void pm_register_standby(struct pm_notifier *);
/* to ask to move STAND-BY state */
int pm_goto_sby(unsigned char bypass_host);
/* to register a user on software reboot event */
void pm_register_reboot(struct pm_notifier *);
/* to trigger a software reboot */
int pm_reboot(unsigned char bypass_host);
/* to initialize PM context */
int pm_init(void *context);
/* to check whether we were suspended to ram and we need to resume from */
unsigned char pm_str_check_resume(void);
unsigned char pm_std_check_resume(void);
void pm_suspend_get_boot_mode(void);
void pm_suspend_restore_secure_mem(void);
uint32_t pm_get_ap_resume_entry(void);
#endif

