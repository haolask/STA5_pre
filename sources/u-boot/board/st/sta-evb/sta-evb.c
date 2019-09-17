/*
 * Copyright (C) 2016 STMicrolelctronics, <www.st.com>
 * Philippe Langlais, <philippe.langlais@st.com>
 */

/* Declare variables shared between M3 and AP processors */
#define DECLARE_SHARED_DATA

#include <common.h>
#include <dm.h>
#include <console.h>
#include <malloc.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/hsem.h>
#include <linux/mtd/fsmc_nand.h>
#include <usb.h>
#include <net.h>
#include <usb/dwc2_udc.h>
#include <sdhci.h>

#ifdef CONFIG_OF_LIBFDT_OVERLAY
#include "sta-dtbo.h"
#endif

#define EMMC_BUS_WIDTH_MODE MMC_MODE_8BIT /* or MMC_MODE_4BIT */


#if defined(CONFIG_OF_LIBFDT)
#include <libfdt.h>
#include <fdt_support.h>
#endif

#ifdef CONFIG_MMC
#include "../../../drivers/mmc/arm_pl180_mmci.h"
#endif

#define NELEMS(X)	(sizeof(X) / sizeof(X[0]))

#if defined(CONFIG_SOC_STA1195) || defined(CONFIG_SOC_STA1295) || defined(CONFIG_SOC_STA1275)

/*
 * The STA1XXX SoC has 6 GPIO banks each with 32 GPIOs max
 */
static const struct sta_gpio_platdata sta_gpio[] = {
	{STA_GPIO0_BASE, "gpio0 "},  /* 00..31 */
	{STA_GPIO1_BASE, "gpio1 "},  /* 32..63 */
	{STA_GPIO2_BASE, "gpio2 "},  /* 64..95 */
	{STA_GPIO3_BASE, "gpio3 "},  /* 96..127 */
	{STA_GPIOS_BASE, "gpios "},  /* 128..143 */
};

#define GPIO_MAX_NUMBER		144
#define S_GPIO_OFFSET		128

U_BOOT_DEVICES(sta_gpio) = {
	{ "gpio_sta", &sta_gpio[0] },
	{ "gpio_sta", &sta_gpio[1] },
	{ "gpio_sta", &sta_gpio[2] },
	{ "gpio_sta", &sta_gpio[3] },
	{ "gpio_sta", &sta_gpio[4] },
	{ "gpio_sta", &sta_gpio[5] },
};
#elif defined(CONFIG_SOC_STA1385)
static const struct sta_gpio_platdata sta_gpio[] = {
	{STA_GPIO0_BASE, "gpio0 "},  /* 00..31 */
	{STA_GPIO1_BASE, "gpio1 "},  /* 32..63 */
	{STA_GPIO2_BASE, "gpio2 "},  /* 64..95 */
	{STA_GPIOS_BASE, "gpios "},  /* 96..111 */
};

#define GPIO_MAX_NUMBER		112
#define S_GPIO_OFFSET		96

U_BOOT_DEVICES(sta_gpio) = {
	{ "gpio_sta", &sta_gpio[0] },
	{ "gpio_sta", &sta_gpio[1] },
	{ "gpio_sta", &sta_gpio[2] },
	{ "gpio_sta", &sta_gpio[3] },
	{ "gpio_sta", &sta_gpio[4] },
};
#else
#error "No SoC configuration defined !"
#endif

#ifdef CONFIG_SHOW_BOOT_PROGRESS
void show_boot_progress(int progress)
{
	printf("%i\n", progress);
}
#endif

struct gpio_mux_t {
	uint8_t start;
	uint8_t end;
	uint8_t mux_type;
};

/*
 * Request gpio pin muxing configuration
 */
static void request_gpio_mux(const struct gpio_mux_t *p_mux,
		int num_mux, const char *label)
{
	int i, gpio;

	for (i = 0; i < num_mux; i++, p_mux++) {
		for (gpio = p_mux->start; gpio <= p_mux->end; gpio++) {
			assert(gpio < GPIO_MAX_NUMBER);
			gpio_request(gpio, label);
			gpio_set_af(gpio, p_mux->mux_type);
		}
	}
}

/*
 * Miscellaneous platform dependent initialisations
 */

int board_early_init_f(void)
{
	gd->arch.p_shared_data = &shared_data;
	return 0;
}

#ifdef CONFIG_STA_QSPI
int board_qspi_init(void)
{
	if (get_soc_id() == SOCID_STA1385) {
		/* Temporary set right SQIx pin mux configuration here */
		static const struct gpio_mux_t sqi0_mux[] = {
			/* FB Clock, Clock, CS */
			{ 73, 75, GPIOF_FUNC_B },
			/* DAT0-3 */
			{ 78, 81, GPIOF_FUNC_B },
		};

		request_gpio_mux(sqi0_mux, NELEMS(sqi0_mux), "SQI0");
	} else {
		/* Temporary set right SQIx pin mux configuration here */
		static const struct gpio_mux_t sqi0_mux[] = {
			/* FB Clock */
			{ 6, 6, GPIOF_FUNC_A },
			/* DAT0-3, Clock, CS */
			{ 108, 113, GPIOF_FUNC_A },
		};

		request_gpio_mux(sqi0_mux, NELEMS(sqi0_mux), "SQI0");
	}

#ifdef USE_SQI1
	if (get_soc_id() == SOCID_STA1385) {
		static const struct gpio_mux_t sqi1_mux[] = {
			/* FB Clock, Clock, CS */
			{ 73, 75, GPIOF_FUNC_B },
			/* DAT0-3 */
			{ 78, 81, GPIOF_FUNC_B },
		};

		request_gpio_mux(sqi1_mux, NELEMS(sqi1_mux), "SQI1");
	} else {
		static const struct gpio_mux_t sqi1_mux[] = {
			/* CS */
			{ 101, 101, GPIOF_FUNC_A },
			/* DAT3-0, Clock */
			{ 108, 112, GPIOF_FUNC_A },
		};

		request_gpio_mux(sqi1_mux, NELEMS(sqi1_mux), "SQI1");
	}
#endif

	return 0;
}
#endif

#ifdef CONFIG_STM_GMAC4
void eth_pin_init(void)
{
	if (get_soc_id() == SOCID_STA1385) {
		static const struct gpio_mux_t eth0_mux[] = {
			{ 0, 13, GPIOF_FUNC_A },
		};
		request_gpio_mux(eth0_mux, NELEMS(eth0_mux), "ETH0");
	} else {
		static const struct gpio_mux_t eth0_mux[] = {
			/* ETH MDIO MDC */
			{ 38, 39, GPIOF_FUNC_A },
			/* ETH EN TX RX CLK CRS */
			{ 116, 121, GPIOF_FUNC_A },
			{ 122, 122, GPIOF_FUNC_B },
			{ 123, 127, GPIOF_FUNC_A },
		};
		request_gpio_mux(eth0_mux, NELEMS(eth0_mux), "ETH0");
	}
}
#endif

int board_init(void)
{
	int ret = 0;

	if (get_soc_id() == SOCID_STA1385) {
		static const struct gpio_mux_t uart2_mux[] = {
			{ 14, 15, GPIOF_FUNC_A },
		};
		request_gpio_mux(uart2_mux, NELEMS(uart2_mux), "UART2");
	} else {
		static const struct gpio_mux_t uart3_mux[] = {
			{ 44, 45, GPIOF_FUNC_A },
		};

		/*
		 * Set UART3 mux config for console (A5 EVBs default)
		 * (FIXME to put in DT when pinmux will be implemented)
		 */
		request_gpio_mux(uart3_mux, NELEMS(uart3_mux), "UART3");
	}

	gd->bd->bi_boot_params = CONFIG_SYS_SDRAM_BASE + 0x100;

#ifdef CONFIG_USB_GADGET_DWC2_OTG
	ret = board_usb_init(0, USB_INIT_DEVICE);
	if (ret)
		printf("%s: board_usb_init: %d\n", __func__, ret);
#endif

#ifdef CONFIG_STA_QSPI
	board_qspi_init();
#endif

#ifdef CONFIG_STM_GMAC4
	eth_pin_init();
#endif
	/* Enable console recording for Fastboot ST OEM commands */
	console_record_reset_enable();

	return ret;
}

int board_late_init(void)
{
#ifdef CONFIG_OF_LIBFDT_OVERLAY
	/* Init DTBO env variable */
	board_sta_DT_overlay_init();
#endif

	/* Display Management if needed */
	if (get_display_cfg() == DUAL_WVGA) {
		/* activate kernel argument "st_drm.dual_display=1" */
		char *args;

		args = getenv("commonargs");
		if (args) {
			#define DUAL_ARG "\"st_drm.dual_display=y\""
			char *args_modified =
				malloc(strlen(args) + strlen(DUAL_ARG) + 2);
			snprintf(args_modified, strlen(args) +
				 strlen(DUAL_ARG) + 2, "%s %s", DUAL_ARG, args);
			setenv("commonargs", args_modified);
			free(args_modified);
			printf("dual_display added to commonargs\n");
		}
	}

	return 0;
}

#if !defined(CONFIG_DISPLAY_CPUINFO)
#define print_cpuinfo sta_print_cpuinfo
#endif
int print_cpuinfo(void)
{
	const char *soc_id;

	/* SoC & ROM version */
	switch (get_soc_id()) {
	case SOCID_STA1295:
		soc_id = "295";
		break;
	case SOCID_STA1275:
		soc_id = "275";
		break;
	case SOCID_STA1195:
		soc_id = "195";
		break;
	case SOCID_STA1385:
		soc_id = "385";
		break;
	default:
		soc_id = "??";
		break;
	}
	printf("SoC ID      = STA1%s cut%X\n", soc_id, get_cut_rev());

	return 0;
}

/*
 * This function is called by bdinfo to print detail board information.
 * As an exmaple for future board, we organize the messages into
 * several sections. If applicable, the message is in the format of
 * <name>      = <value>
 * It should aligned with normal output of bdinfo command.
 *
 * Board Id    : Board identifier
 * SoC         : STA SoC identifier
 * Clocks      : TODO
 */
void board_detail(void)
{
	int i;
	const char *board_name = "A5 EVB";

	/* Board ID (get info dynamically) */
	printf("Board ID    = ");
	switch (get_board_id()) {
	case BOARD_TC3P_CARRIER:
		board_name = "TC3P CARRIER";
		break;
	case BOARD_TC3P_MTP:
		board_name = "TC3P MTP";
		break;
	case BOARD_TC3_EVB:
		board_name = "TC3 EVB";
		break;
	default:
		break;
	}
	printf("%s rev %c\n", board_name, 'A' + get_board_rev_id());
	print_cpuinfo();
	printf("ROM Version = %X\n", get_erom_version());
	/* Display OTP registers */
	for (i = 0; i < NELEMS(shared_data.bi.otp_regs); i++)
		printf("OTP VR%d = 0x%08X\n", i, get_otp_regs()[i]);
}

int dram_init(void)
{
	gd->ram_size = PHYS_SDRAM_1_SIZE;

	return 0;
}

void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	int off, off2;
	const char *old_mod_name;
	uint8_t enetaddr[6], display_cfg;
	const char *dtb_mmc_path;
	const char *dtb_nand_path;

#ifdef CONFIG_OF_LIBFDT_OVERLAY
	/* apply DTBOs first */
	run_command("dtbo_apply", 0);
#endif

	if (get_board_id() == BOARD_TC3_EVB) {
		char mod_name[64] = {"ST TELEMACO3 STA1195 EValuation Board"};
		fdt_setprop(blob, 0, "model", mod_name, strlen(mod_name) + 1);
	}

	old_mod_name = fdt_getprop(blob, 0, "model", NULL);
	if (old_mod_name) {
		char mod_name[64] = {0};
		snprintf(mod_name, sizeof(mod_name), "%s rev%c", old_mod_name,
			 get_board_rev_id()+'A');
		fdt_setprop(blob, 0, "model", mod_name, strlen(mod_name) + 1);
	}

#ifdef EARLY_TUNER_FTR
	off = fdt_path_offset(blob, "/soc/i2c@50040000");
	if (off >= 0) {
		fdt_status_disabled(blob, off);
		puts("i2c1 disabled in dtb for early tuner usage\n");
	} else {
		puts("failed to disable i2c1\n");
	}

#endif

#ifdef CONFIG_MMC
	if (get_boot_dev_type() == BOOT_MMC) {
		char node_path[20] = {0};
		/* Set mmc boot device as shared */
		snprintf(node_path,  sizeof(node_path),
			 "/soc/sdi@%8X/", get_boot_dev_base());
		/* Find and create "st,shared" property. */
		if ((off = fdt_find_and_setprop(blob, node_path,
					       "st,shared", NULL, 0, 1)))
			printf("Err %d: Can't add st,shared prop\n", off);

	}
#endif /* CONFIG_MMC */

#ifdef CONFIG_ARMV7_NONSEC
	{
		const void *pen_addr = cpu_to_fdt32(CONFIG_SMP_PEN_ADDR);
		const void *resume_addr = cpu_to_fdt32(BACKUP_RAM_STR_KERNEL_RESUME_ADDR_BASE);

		/* Set secondary pen release address for Linux Kernel */
		fdt_find_and_setprop(blob, "/cpus/cpu@1", "cpu-release-addr",
				     &pen_addr, sizeof(u32), 1);

		/* Set PM machine resume address for Linux Kernel */
		fdt_find_and_setprop(blob, "/soc/suspend", "st,cpu-resume-addr",
				     &resume_addr, sizeof(u32), 1);
	}
#endif /* CONFIG_ARMV7_NONSEC */

#ifdef DDRAM_RAMOOPS_RAMOOPS_MEM_BASE
	/* RAMOOPS feature */
	off = fdt_path_offset(blob, "/reserved-memory/ramoops");
	if (off >= 0) {
		struct fdt_property *prop;
		u32 *reg;
		int len;

		prop = fdt_get_property_w(blob, off, "reg", &len);
		if (prop) {
			reg = (u32 *)&prop->data[0];
			reg[0] = cpu_to_fdt32(DDRAM_RAMOOPS_RAMOOPS_MEM_BASE);
			reg[1] = cpu_to_fdt32(DDRAM_RAMOOPS_RAMOOPS_MEM_SIZE);
		}
	}
#endif /* DDRAM_RAMOOPS_RAMOOPS_MEM_BASE */

#ifdef DDRAM_RAMDUMP_MAGIC_BASE
	/* RAMDUMP feature */
	{
		const void *magic_addr = cpu_to_fdt32(DDRAM_RAMDUMP_MAGIC_BASE);
		fdt_find_and_setprop(blob, "/reserved-memory",
				     "ramdump-magic-addr",
				     &magic_addr, sizeof(u32), 1);
	}
#endif /* DDRAM_RAMDUMP_MAGIC_BASE */

	/* Update IPC mailbox mapping */
	off = fdt_path_offset(blob, "/soc/mailbox@481a0000");
	if (off >= 0) {
		struct fdt_property *prop;
		int len;
		int index;
		u32 *reg, *reg2;

		index = fdt_stringlist_search(blob, off, "reg-names", "mbox-shm");
		if (index >= 0) {
			prop = fdt_get_property_w(blob, off, "reg", &len);
			if (prop) {
				int idx;
				int tuple_size = 2 * sizeof(reg);

				reg = reg2 = (u32 *)&prop->data[0];
				for (idx = 0; idx < (len / tuple_size); idx++) {
					/* Update address and size in reg property */
					if (idx == index) {
						reg[0] = cpu_to_fdt32(DDRAM_APP_OS_MAILBOXES_BASE);
						reg[1] = cpu_to_fdt32(DDRAM_APP_OS_MAILBOXES_SIZE);
					}

					/* Point to next reg tuple */
					reg += 2;
				}

				fdt_setprop(blob, off, "reg", reg2, len);
			}
		}
	}

#ifdef DDRAM_APP_OS_GCNANO_POOL_BASE
	/* Update GCNANO pool mapping */
	off = fdt_path_offset(blob, "/soc/gcnano@48F00000");
	if (off >= 0) {
		struct fdt_property *prop;
		int len;
		int index;
		u32 *reg, *reg2;

		index = fdt_stringlist_search(blob, off, "reg-names", "contig_baseaddr");
		if (index >= 0) {
			prop = fdt_get_property_w(blob, off, "reg", &len);
			if (prop) {
				int idx;
				int tuple_size = 2 * sizeof(reg);

				reg = reg2 = (u32 *)&prop->data[0];
				for (idx = 0; idx < (len / tuple_size); idx++) {
					/* Update address and size in reg property */
					if (idx == index) {
						reg[0] = cpu_to_fdt32(DDRAM_APP_OS_GCNANO_POOL_BASE);
						reg[1] = cpu_to_fdt32(DDRAM_APP_OS_GCNANO_POOL_SIZE);
					}

					/* Point to next reg tuple */
					reg += 2;
				}

				fdt_setprop(blob, off, "reg", reg2, len);
			}
		}
	}
#endif /* DDRAM_APP_OS_GCNANO_POOL_BASE */
	dtb_nand_path = "/soc/flash@80000000";
	switch (get_soc_id()) {
		case SOCID_STA1385:
			dtb_mmc_path = "/soc/sdi@50070000";
			break;
		default:
			dtb_mmc_path = "/soc/sdi@500B0000";
			break;
	}
#if defined(CONFIG_NAND_FSMC)
	/* Nand is enabled and eMMC disabled */
	off = fdt_path_offset(blob, dtb_nand_path);
	if (off >= 0) {
		fdt_status_okay(blob, off);
		puts("dtb: FSMC NAND enabled\n");
	}
	off = fdt_path_offset(blob, dtb_mmc_path);
	if (off >= 0) {
		fdt_status_disabled(blob, off);
	}

#else
	/* eMMC is enabled and FSMC NAND disabled */
	off = fdt_path_offset(blob, dtb_mmc_path);
	if (off >= 0) {
		fdt_status_okay(blob, off);
		puts("dtb: eMMC enabled\n");
	}
	off = fdt_path_offset(blob, dtb_nand_path);
	if (off >= 0) {
		fdt_status_disabled(blob, off);
	}

#endif

	if (eth_getenv_enetaddr("eth_mac_addr", enetaddr)) {
		fdt_find_and_setprop(blob, "/soc/dwmac@500a0000", "mac-address",
				     enetaddr, sizeof(enetaddr), 1);
	}

#if defined(CONFIG_SUPPORT_EMMC_BOOT)
	if (get_soc_id() == SOCID_STA1385) {
		/* ethernet1 is conflicting with eMMC on sta1385-mtp-mmc machine */
		off = fdt_path_offset(blob, "/soc/ethernet1");
		fdt_status_disabled(blob, off);
	}
#endif

	/* Display clock management */
	/* if A5 and < cut 3.0, change lcd clock div */
	if ((get_soc_id() == SOCID_STA1295) && (get_cut_rev() < CUT_30)) {
		const u32 *index;
		off = fdt_path_offset(blob, "/clocks/pll2fvcoby2_by3");
		if (off < 0) {
			puts("Error to get pll2fvcoby2_by3 node\n");
		}

		index = fdt_getprop(blob, off, "phandle", 0);
		if (index == NULL) {
			printf("WARNING: could not get cell-index of ucc\n");
		}

		off = fdt_path_offset(blob, "/clocks/clcdclkdiv");
		if (off < 0) {
			puts("Error to get clcdclkdiv node\n");
		}

		if (fdt_setprop(blob, off, "clocks", index, sizeof(u32)))
			puts("Error to set clcdclkdiv property\n");
		else
			puts("Adapt clcdclkdiv to A5 cut < 3.0 (pll2by2 => pll2by2_by3)\n");
	}

	/* Display Management */
	display_cfg = get_display_cfg();
	off2 = fdt_path_offset(blob, "/soc/panel-rgb1");
	if (off2 < 0) {
		if (get_board_id() == BOARD_A5_EVB)
			puts("Error to get panel-rgb1\n");
	} else {
		switch (display_cfg) {
		case NO_DETECTION:
		case SINGLE_WVGA:
			/* Nothing to do */
			break;
		case DUAL_WVGA:
			/* enable panel_rgb2 */
			off = fdt_path_offset(blob, "/soc/panel-rgb2");
			if (off >= 0) {
				fdt_status_okay(blob, off);
				puts("panel-rgb2 enabled\n");
			} else
				puts("Error to enable panel-rgb2\n");
			break;
		case SINGLE_720P:
		case SINGLE_HYBRID_CLUSTER_720P:
		case SINGLE_720P_10INCHES:
			/* modify panel_rgb1 for 720p panel */
			if (fdt_setprop_string(blob, off2, "compatible", "ampire,am-1280800p2tzqw-t01h"))
				printf("Error to set panel-rgb1 to 720p ampire\n");
			else
				puts("set panel-rgb1 to 720p ampire\n");
			break;
		case SINGLE_CLUSTER:
			/* select panel_rgb1 for cluster chunghwa panel */
			if (fdt_setprop_string(blob, off2, "compatible", "chunghwa,claa103wa02xn"))
				printf("Error to set panel-rgb1 to cluster chunghwa\n");
			else
				puts("set panel-rgb1 to cluster chunghwa\n");
			break;
		case SINGLE_CLUSTER_HD:
			/* select panel_rgb1 for cluster HD chunghwa panel */
			if (fdt_setprop_string(blob, off2, "compatible", "chunghwa,claa123fca1xn"))
				printf("Error to set panel-rgb1 to cluster HD chunghwa\n");
			else
				puts("set panel-rgb1 to cluster HD chunghwa\n");
			break;
		case SINGLE_HYBRID_CLUSTER_WVGA:
			/* select panel_rgb1 for cluster WVGA ampire panel */
			if (fdt_setprop_string(blob, off2, "compatible", "ampire,am-800480aztmqw-54h"))
				printf("Error to set panel-rgb1 to cluster WVGA ampire\n");
			else
				puts("set panel-rgb1 to cluster WVGA ampire\n");
			break;
		default:
			break;
		}
	}

	/* Touchscreen Management */
	/* In case we are not using a WVGA rocktech single/dual display,
	 * disabled ft5336 touchscreen */
	if (!((display_cfg == NO_DETECTION) || (display_cfg == SINGLE_WVGA) ||
	      (display_cfg == DUAL_WVGA))) {
		/* disable ft5336 WVGA touchscreen */
		off = fdt_path_offset(blob, "/soc/i2c/ft5336");
		if (off >= 0) {
			fdt_status_disabled(blob, off);
			puts("ft5336 disabled\n");
		} else
			puts("Error to disable ft5336\n");
	}

	switch (display_cfg) {
	case SINGLE_720P:
		/* enable exc3132 touchscreen */
		off = fdt_path_offset(blob, "/soc/i2c/exc3132");
		if (off >= 0) {
			fdt_status_okay(blob, off);
			puts("exc3132 enabled\n");
		} else
			puts("Error to enable exc3132\n");
		break;
	case SINGLE_720P_10INCHES:
		/* enable ft5x26 touchscreen */
		off = fdt_path_offset(blob, "/soc/i2c/ft5x26");
		if (off >= 0) {
			fdt_status_okay(blob, off);
			puts("ft5x26 enabled\n");
		} else
			puts("Error to enable ft5x26\n");
		break;
	default:
		/* Nothing to do */
		break;
	}

	if ((get_board_id() == BOARD_TC3P_MTP) && (get_board_rev_id() >= 2)) {
		off = fdt_path_offset(blob, "/soc/msp@48010000/i2s");
		if (off >= 0) {
			if (fdt_setprop_u32(blob, off, "rx-data-delay", 1) < 0)
				puts("Error to enable rx-data-delay\n");
		} else {
			puts("Error to find i2s1@48010000\n");
		}
	}

	/*
	 * Wait for the C-M3 micro-ctrl sub-system to set sync point
	 * announcing it reached a state so that it's safe to start the AP OS.
	 * This is to make sure critical M3 init code execute before
	 * start of AP OS.
	 */
	puts("Waiting M3 sync point...\n");
	while (!get_sync_point())
		;

	return 0;
}
#endif

#if defined(CONFIG_NAND_FSMC)

static struct nand_chip nand_chip[CONFIG_SYS_MAX_NAND_DEVICE];

/*
 * board_nand_init - Board specific NAND initialization
 * @nand:	mtd private chip structure
 *
 * Called by nand_init_chip to initialize the board specific functions
 */

void board_nand_init(void)
{
	struct nand_chip *nand = &nand_chip[0];

	if (get_soc_id() == SOCID_STA1385) {
		static const struct gpio_mux_t nand16_mux[] = {
			{ 73, 87, GPIOF_FUNC_A },
		};
		request_gpio_mux(nand16_mux, NELEMS(nand16_mux), "FSMC_NAND");
	} else {
		static const struct gpio_mux_t nand_mux[] = {
			{ 92, 100, GPIOF_FUNC_A },
			{ 102, 107, GPIOF_FUNC_A },
#if defined(CONFIG_SYS_FSMC_NAND_16BIT)
			{ 49, 50, GPIOF_FUNC_C },
			{ 108, 113, GPIOF_FUNC_C },
#endif
		};
		request_gpio_mux(nand_mux, NELEMS(nand_mux), "FSMC_NAND");
	}

	fsmc_nand_init(nand);
	return;
}

/*
 * board_nand_reinit - Board specific NAND re-initialization
 * @a5_cut3_workaround:	Bool to indicate if A5 Cut3 large page workaround must
 *                      be set or not
 *
 */
void board_nand_reinit(int a5_cut3_workaround)
{
	struct nand_chip *nand = &nand_chip[0];

	if (a5_cut3_workaround)
		nand->options = NAND_BROKEN_LP;
	else
		nand->options = 0;
	fsmc_nand_init(nand);
	return;
}

#endif /* CONFIG_NAND_FSMC */

#ifdef CONFIG_USB_GADGET_DWC2_OTG

#define UTMI_PLL_CLK_SEL_OFFSET	0x84
#define UTMIOTG_IDDIG_OFFSET	0x80
#define DRVVBUS_USB0_OFFSET		0x8C
#define DRVVBUS_USB1_OFFSET		0x78
#define DFT_CFG_REG_OFFSET		0x28

#define PLL_POWER_DOWN		(1<<4)
#define PHY_ENABLE			(1<<24)
#define USB0_UTMIOTG_IDDIG	(1<<3)
#define USB0_UTMIOTG_DM_PD	(1<<5)
#define USB0_UTMIOTG_DP_PD	(1<<4)
#define USB1_UTMIOTG_IDDIG	(1<<2)
#define USB1_UTMIOTG_DM_PD	(1<<1)
#define USB1_UTMIOTG_DP_PD	(1<<0)
#define DRVVBUS				(1<<0)

static int sta_phy_control(int on)
{
	u32 val;
	u32 drvvus;
	u32 iddig;
	u32 dm_pd, dp_pd;

	switch (get_soc_id()) {
	case SOCID_STA1385: {
		static const struct gpio_mux_t drv_vbus_mux[] = {
			{ 16, 17, GPIOF_FUNC_B },
		};
		/* Init GPIO_30 to DRVVBUS */
		request_gpio_mux(drv_vbus_mux, NELEMS(drv_vbus_mux), "USB0_DR");

		drvvus = DRVVBUS_USB0_OFFSET;
		iddig = USB0_UTMIOTG_IDDIG;
		dm_pd = USB0_UTMIOTG_DM_PD;
		dp_pd = USB0_UTMIOTG_DP_PD;
		break;
	}
	default: {
		static const struct gpio_mux_t drv_vbus_mux[] = {
			{ 30, 30, GPIOF_FUNC_C },
		};
		/* Init GPIO_30 to DRVVBUS */
		request_gpio_mux(drv_vbus_mux, NELEMS(drv_vbus_mux), "USB1_DR");

		drvvus = DRVVBUS_USB1_OFFSET;
		iddig = USB1_UTMIOTG_IDDIG;
		dm_pd = USB1_UTMIOTG_DM_PD;
		dp_pd = USB1_UTMIOTG_DP_PD;
		break;
	}
	}

	debug("%s: enter\n", __func__);

	/* we need to drive VBUS manually since STA is not OTG capable */
	val = readl(STA_USBPHY_BASE + drvvus);

	if (on)
		val &= ~(DRVVBUS);
	else
		val |= (DRVVBUS);

	writel(val, STA_USBPHY_BASE + drvvus);

	/* disable the PLL power down */
	val = readl(STA_USBPHY_BASE + UTMI_PLL_CLK_SEL_OFFSET);
	if (on)
		val &= ~(PLL_POWER_DOWN);
	else
		val |= (PLL_POWER_DOWN);
	writel(val, STA_USBPHY_BASE + UTMI_PLL_CLK_SEL_OFFSET);

	/* force the ID to device mode, btw disable pulldown resistors on
	 * DP/DM used in host mode */
	val = readl(STA_USBPHY_BASE + UTMIOTG_IDDIG_OFFSET);
	if (on) {
		val |= (iddig);
		val &= ~(dp_pd);
		val &= ~(dm_pd);
	} else {
		val &= ~(iddig);
		val |= (dp_pd);
		val |= (dm_pd);
	}
	writel(val, STA_USBPHY_BASE + UTMIOTG_IDDIG_OFFSET);
	return 0;
}

struct dwc2_plat_otg_data sta_otg_usb1_data = {
	.phy_control = sta_phy_control,
	.regs_phy = STA_USBPHY_BASE,
	.regs_otg = STA_USB1_OTG_BASE,
	.usb_flags = PHY0_SLEEP,
};

struct dwc2_plat_otg_data sta_otg_usb0_data = {
	.phy_control = sta_phy_control,
	.regs_phy = STA_USBPHY_BASE,
	.regs_otg = STA_USB0_OTG_BASE,
	.usb_flags = PHY0_SLEEP,
};

unsigned int s5p_cpu_id = 0x51A1; /* value for STA1xxx family used in OTG phy */


int board_usb_init(int index, enum usb_init_type init)
{
	int ret = 0;

	debug("USB_udc_probe\n");
	switch (get_soc_id()) {
	case SOCID_STA1385:
		ret = dwc2_udc_probe(&sta_otg_usb0_data);
		break;
	default:
		ret = dwc2_udc_probe(&sta_otg_usb1_data);
		break;
	}

	return ret;
}

#endif /* CONFIG_USB_GADGET_DWC2_OTG */

#ifdef CONFIG_USB_ETHER

int board_eth_init(bd_t *bis)
{
	int ret;

	ret = usb_eth_initialize(bis);
	printf("%s: usb_eth_initialize: %d\n", __func__, ret);

	return ret;
}

#endif /* CONFIG_USB_ETHER */


#ifdef CONFIG_MMC

static unsigned char *const mmc_ports[] = {
	(void *)CONFIG_ARM_PL180_MMCI0_BASE,  /* MMC0 (Arasan SDIO) */
	(void *)CONFIG_ARM_PL180_MMCI1_BASE,  /* MMC1 (SD-MMC card for A5/TC3
					       * EVB or EMMC for Carrier) */
	(void *)CONFIG_ARM_PL180_MMCI2_BASE   /* MMC2 (EMMC for A5/TC3 EVB) */
};

static int sta_mmc_board_init(int mmc_port)
{
	if (get_soc_id() == SOCID_STA1385) {
		static const struct gpio_mux_t sdmmc0_mux[] = {
			{ 22, 29, GPIOF_FUNC_A },   /* CMD, CLK, DAT0-DAT3, FB clock, PWR */
		};
		static const struct gpio_mux_t emmc0_mux[] = {
			{ 73, 74, GPIOF_FUNC_C }, /* CMD CLK */
			{ 76, 84, GPIOF_FUNC_C }, /* DAT0-7 PWR */
			{ 87, 87, GPIOF_FUNC_C }, /* FBCLK */
		};

		static const struct gpio_mux_t mmc1_mux[] = {
			{ 38, 48, GPIOF_FUNC_A },
		};

		switch (mmc_port) {
		case 0:
			/* MMC port 0 is ARASAN IP for SD-MMC or SDIO */
			if (get_mmc_boot_dev() == 0) /* SDHC0 is eMMC ? */
				request_gpio_mux(emmc0_mux,
						 NELEMS(emmc0_mux), "MMC0");
			else
				request_gpio_mux(sdmmc0_mux,
						 NELEMS(sdmmc0_mux), "MMC0");
			return 0;

		case 1:
			request_gpio_mux(mmc1_mux, NELEMS(mmc1_mux), "MMC1");
			return 0;
		}
	} else {
		static const struct gpio_mux_t mmc1_mux[] = {
			{ 0, 2, GPIOF_FUNC_A },   /* CMD, CLK, DAT0 */
			{ 11, 13, GPIOF_FUNC_B }, /* DAT1--DAT3 */
			{ 32, 32, GPIOF_FUNC_C }, /* FB clock */
		};

		/* eMMC: GPIO 92 to 103 in FUNC_B except 101 */
		static const struct gpio_mux_t mmc2_mux[] = {
			{ 92, 100, GPIOF_FUNC_B },
			{ 102, 103, GPIOF_FUNC_B },
		};

		switch (mmc_port) {
		case 0:
			/*
			 * MMC port 0 is ARASAN IP for SDIO/WLAN
			 * No specific mux required for 4 wires
			 */
			return 0;

		case 1: /* External SD/MMC */
			request_gpio_mux(mmc1_mux, NELEMS(mmc1_mux), "MMC1");
			return 0;

		case 2: /* eMMC */
			request_gpio_mux(mmc2_mux, NELEMS(mmc2_mux), "MMC2");
			return 0;
		}
	}
	return -ENODEV;
}

int mmc_get_env_dev(void)
{
#if defined(CONFIG_MMC_SDHCI)
	return get_mmc_boot_dev();
#else
	return get_mmc_boot_dev() - 1; /* Arasan SDI#0 not managed */
#endif
}

/* On STA1xxx SoC Feed-Back clock control bit is inverted */
#define SDI_PWR_FBCLKDIS SDI_PWR_FBCLKEN

int board_mmc_init(bd_t *bd)
{
	struct pl180_mmc_host *host;
	int port;
	int port_max;
	int port_emmc = -1; /* No eMMC by default */
	int err = 0;

	port = 0; /* Arasan IP is managed */
	switch (get_board_id())  {
	case BOARD_TC3P_CARRIER:
	case BOARD_TC3P_MTP:
		port_max = 1; /* Only one MMCI port is available */
		port_emmc = get_mmc_boot_dev();
		break;

	default: /* A5 or TC3 EVB */
#if defined CONFIG_NAND_FSMC
		port_max = 2; /* Only MMC1, MMC2 (eMMC) is not available */
#else
		port_emmc = 2; /* MMC2 is an eMMC */
		port_max = 3; /* 3 MMC ports could be managed */
#endif
		break;
	}

	/* Init Arasan IP if managed? */
	if (port == 0) {
#if defined(CONFIG_MMC_SDHCI)
		struct sdhci_host *host = calloc(1, sizeof(struct sdhci_host));

		if (!host)
			return -ENOMEM;

		err = sta_mmc_board_init(port);

		if (err)
			return err;

		host->name = "SDHC0";
		host->ioaddr = (void *)mmc_ports[port];
		host->bus_width = (port == port_emmc) ? 8 : 4;
		host->max_clk = 100000000;
		host->quirks = SDHCI_QUIRK_BROKEN_VOLTAGE;
		host->voltages = MMC_VDD_32_33 | MMC_VDD_33_34 | MMC_VDD_165_195;
		/* MMC boot device is shared with M3 => hsem */
		if (port == get_mmc_boot_dev())
			host->hsem = HSEM_MMC_ID;

		if (add_sdhci(host, 0, 400000))
			puts("Warning: No SDMMC detected on SDHC0\n");
#endif
		port++;
	}
	/* Always Initialize MMC1 and eventually MMC2 for A5/eMMC */
	for (; port <= port_max; port++) {
		bool is_emmc;

		is_emmc = (port == port_emmc);

		if (sta_mmc_board_init(port))
			return -ENODEV;

		host = calloc(1, sizeof(struct pl180_mmc_host));
		if (!host)
			return -ENOMEM;

		strcpy(host->name, is_emmc ? "EMMC" : "MMC");
		host->base = (struct sdi_registers *)mmc_ports[port];
		host->pwr_init = is_emmc
			? (SDI_PWR_OPD | SDI_PWR_PWRCTRL_ON | SDI_PWR_FBCLKDIS)
			: SDI_PWR_PWRCTRL_ON;
		host->clkdiv_init = SDI_CLKCR_CLKDIV_INIT_V2 | SDI_CLKCR_CLKEN
			| SDI_CLKCR_HWFC_EN;
		host->voltages = is_emmc ? VOLTAGE_WINDOW_MMC : VOLTAGE_WINDOW_SD;
		host->caps = MMC_MODE_HS | MMC_MODE_HS_52MHz
			| (is_emmc ? EMMC_BUS_WIDTH_MODE : MMC_MODE_4BIT);
		host->clock_in = ARM_MCLK;
		host->clock_min = ARM_MCLK / (2 + SDI_CLKCR_CLKDIV_INIT_V2);
		host->clock_max = ARM_MCLK;
		host->version2 = 1;
		/* MMC boot device is shared with M3 => hsem */
		if (port == get_mmc_boot_dev())
			host->hsem = HSEM_MMC_ID;

		err = arm_pl180_mmci_init(host);
	}

	return err;
}

#endif /* CONFIG_MMC */

void board_deinit(void)
{
#if defined CONFIG_STA_QSPI
	extern void sta_qspi_deinit(void);

	sta_qspi_deinit();
#endif
}
