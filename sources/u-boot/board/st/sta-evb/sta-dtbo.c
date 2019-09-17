/*
 * Copyright (C) 2018 STMicrolelctronics, <www.st.com>
 * Nicolas Guion, <nicolas.guion@st.com>
 */

/* Declare variables shared between M3 and AP processors */
#include <common.h>
#include <malloc.h>

#include <libfdt.h>
#include <fdt_support.h>

/* DTB overlays */
#define DTBO_LIST_VAR_NAME "dtbo"
#define DTBO_STORAGE_DIRECTORY "dtbo"
#define DTBO_LIST_OFF "none"
#define DTBO_MAX_FILENAME 50

void board_sta_DT_overlay_init(void)
{
	const char *soc_id;
	const char *board_type;
	const char *memory_boot;

	char dtbo_str[512];

	/* if env variable describing the dtbo files is already defined,
	 * skip this procedure else define it based on HW information
	 */
	if (getenv(DTBO_LIST_VAR_NAME))
		return;

	switch (get_soc_id()) {
	case SOCID_STA1295:
		soc_id = "295";
		break;
	case SOCID_STA1195:
		soc_id = "195";
		break;
	case SOCID_STA1385:
		soc_id = "385";
		break;
	default:
		soc_id = "?";
		break;
	}

	switch (get_board_id()) {
	case BOARD_TC3P_CARRIER:
		board_type = "carrier";
		break;
	case BOARD_TC3P_MTP:
		board_type = "mtp";
		break;
	case BOARD_TC3_EVB:
		board_type = "evb";
		break;
	default:
		board_type = "?";
		break;
	}

	switch (get_boot_dev_type()) {
	case BOOT_NAND:
		memory_boot = "nand";
		break;
	case BOOT_MMC:
		memory_boot = "mmc";
		break;
	case BOOT_SQI:
		memory_boot = "sqi";
		break;
	default:
		memory_boot = "?";
		break;
	}


	snprintf(dtbo_str, sizeof(dtbo_str),
		 "sta1%s-cut%x.dtbo",
		 soc_id, get_cut_rev());
	snprintf(&dtbo_str[strlen(dtbo_str)],
		 sizeof(dtbo_str) - strlen(dtbo_str),
		 " sta1%s-cut%x-%s.dtbo",
		 soc_id, get_cut_rev(), board_type);
	snprintf(&dtbo_str[strlen(dtbo_str)],
		 sizeof(dtbo_str) - strlen(dtbo_str),
		 " sta1%s-%s.dtbo",
		 soc_id,  board_type);
	snprintf(&dtbo_str[strlen(dtbo_str)],
		 sizeof(dtbo_str) - strlen(dtbo_str),
		 " sta1%s-%s-rev%c.dtbo",
		 soc_id,  board_type, 'A' + get_board_rev_id());
	snprintf(&dtbo_str[strlen(dtbo_str)],
		 sizeof(dtbo_str) - strlen(dtbo_str),
		 " sta1%s-%s.dtbo",
		 soc_id,  memory_boot);
	setenv(DTBO_LIST_VAR_NAME, dtbo_str);
}

static int do_dtbo_apply(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	char *dtbo_files_list;
	char *p;
	char dtbo_file_name[DTBO_MAX_FILENAME];
	char cmdbuf[100];
	void *dtbo_addr = NULL;
	char dtbo_addr_str[9];

#ifdef CONFIG_MMC
	ulong mmcdev;
	ulong mmcbootpart;

#endif

	dtbo_files_list = NULL;
	dtbo_files_list = getenv(DTBO_LIST_VAR_NAME);

	if (!dtbo_files_list ||
	    !strcmp(dtbo_files_list, DTBO_LIST_OFF)) {
		printf("None DTBO to apply\n");
		return 0;
	}
	printf("DTBO to apply %s\n", dtbo_files_list);

	snprintf(cmdbuf, sizeof(cmdbuf),
		 "fdt resize 1024");
	if (run_command(cmdbuf, 0)) {
		printf("unable to resize dtb memory of 1K\n");
		return -1;
	}

	dtbo_addr = malloc(0x1000);
	if (!dtbo_addr) {
		printf("unable to allocate 4KB for dtbo processing\n");
		return -1;
	}
	snprintf(dtbo_addr_str, sizeof(dtbo_addr_str), "%p", dtbo_addr);

	do {
		strcpy(dtbo_file_name, DTBO_STORAGE_DIRECTORY "/");
		p = strchr(dtbo_files_list, ' ');
		if (p) {
			if ((int)(p - dtbo_files_list) > sizeof(dtbo_file_name)
			    - sizeof(DTBO_STORAGE_DIRECTORY) - 1) {
				printf("dtbo name too long %s\n",
				       dtbo_files_list);
				continue;
			}
			strncat(dtbo_file_name, dtbo_files_list,
				(int)p - (int)dtbo_files_list);
			dtbo_files_list = p + 1;
		} else {
			if (strlen(dtbo_files_list) > sizeof(dtbo_file_name)
			    - sizeof(DTBO_STORAGE_DIRECTORY)) {
				printf("dtbo name too long %s\n",
				       dtbo_files_list);
				continue;
			}
			strcat(dtbo_file_name, dtbo_files_list);
		}
		/*  Move console in silent mode
		 *  as not find any dtbo files is normal
		 *  avoid to polluate console with FS layers error messages
		 */
		gd->flags |= GD_FLG_SILENT;
#ifdef CONFIG_MMC
		mmcdev = getenv_ulong("mmcdev", 10, 0);
		mmcbootpart = getenv_ulong("mmcbootpart", 10, 1);

		snprintf(cmdbuf, sizeof(cmdbuf),
			 "ext4load  mmc  %lu:%lu %s %s",
			 mmcdev, mmcbootpart,
			 dtbo_addr_str,
			 dtbo_file_name);
		if (run_command(cmdbuf, 0)) {
			continue;
		}
#endif

#if defined(CONFIG_NAND_FSMC)
		if (!ubifs_is_mounted()) {
			snprintf(cmdbuf, sizeof(cmdbuf),
				 "run  nandbootfs");
			if (run_command(cmdbuf, 0)) {
				printf("unable to mount bootfs volume\n");
				continue;
			}
		}

		snprintf(cmdbuf, sizeof(cmdbuf),
			 "ubifsload %s %s",
			 dtbo_addr_str,
			 dtbo_file_name);

		if (run_command(cmdbuf, 0)) {
			continue;
		}
#endif
		gd->flags &= ~GD_FLG_SILENT;

		snprintf(cmdbuf, sizeof(cmdbuf),
			 "fdt apply %s",
			 dtbo_addr_str);

		if (run_command(cmdbuf, 0)) {
			printf("unable to apply file:%s\n", dtbo_file_name);
			continue;
		} else{
			printf("Update DTB with %s\n", dtbo_file_name);
		}

	} while (p);

	if (dtbo_addr)
		free(dtbo_addr);
	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char dtbo_apply_help_text[] =
	"none argument\n"
	"\t-The dtbo file list is provided thank to env variable dtbo,\n"
	"\twhich should be initialized\n"
	"\t-Furthermore the main kernel dtb has to be already loaded\n"
	"\twith command fdt addr ${fdtaddr}\n"
	"\tif sucessful this command save env variable in NVstorage\n";
#else
#define dtbo_apply_help_text ""
#endif

U_BOOT_CMD(
	dtbo_apply,   1,   0,     do_dtbo_apply,
	"apply the dtbo overlay file(s) to the main kernel dtb",
	dtbo_apply_help_text
);

static int do_dtbo_list(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	char cmdbuf[100];

#ifdef CONFIG_MMC
	ulong mmcdev;
	ulong mmcbootpart;

	mmcdev = getenv_ulong("mmcdev", 10, 0);
	mmcbootpart = getenv_ulong("mmcbootpart", 10, 1);

	snprintf(cmdbuf, sizeof(cmdbuf),
		 "ext4ls  mmc %lu:%lu %s",
		 mmcdev, mmcbootpart,
		 DTBO_STORAGE_DIRECTORY);

	printf("List of DTBO files available:\n");
	if (run_command(cmdbuf, 0))
		printf("unable to list dtbo files\n");
#endif

#if defined(CONFIG_NAND_FSMC)
	if (!ubifs_is_mounted()) {
		snprintf(cmdbuf, sizeof(cmdbuf),
			 "run  nandbootfs");
		if (run_command(cmdbuf, 0))
			printf("unable to mount bootfs volume\n");
	}

	snprintf(cmdbuf, sizeof(cmdbuf),
		 "ubifsls  %s",
		 DTBO_STORAGE_DIRECTORY);

	printf("List of DTBO files available:\n");
	if (run_command(cmdbuf, 0))
		printf("unable to list dtbo files\n");
#endif

	return 0;
}

#ifdef CONFIG_SYS_LONGHELP
static char dtbo_list_help_text[] =
	"none argument\n"
	"\t-list dtbo files in bootfs partition";
#else
#define dtbo_list_help_text ""
#endif

U_BOOT_CMD(
	dtbo_list,   1,   0,     do_dtbo_list,
	"list dtbo files available in bootfs partition",
	dtbo_list_help_text
);

static int do_dtbo_reset(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	setenv(DTBO_LIST_VAR_NAME, NULL);
	board_sta_DT_overlay_init();
	saveenv();
	return 0;
}

U_BOOT_CMD(
	dtbo_reset,   1,   0,	 do_dtbo_reset,
	"reset the dtbo files customised to default dtbo",
	""
);

static int do_dtbo_off(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	setenv(DTBO_LIST_VAR_NAME, DTBO_LIST_OFF);
	saveenv();
	return 0;
}

U_BOOT_CMD(
	dtbo_off,   1,   0,	 do_dtbo_off,
	"reset the dtbo files customised to default dtbo",
	""
);

static int do_dtbo_add(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	char *dtbo_files_list;
	char *p;
	char dtbo_file_name[DTBO_MAX_FILENAME];
	char cmdbuf[100];

	dtbo_files_list = NULL;
	dtbo_files_list = getenv(DTBO_LIST_VAR_NAME);
	if (!dtbo_files_list) {
		printf("%s env variable not defined", DTBO_LIST_VAR_NAME);
		return -1;
	}

	if (argc != 2) {
		printf("dtbo_add usage\n\t dtbo_add filename.dtbo\n");
		return -1;
	}

	if (strlen(argv[1]) + sizeof(DTBO_STORAGE_DIRECTORY) + 1 >
	    sizeof(dtbo_file_name)) {
		printf("dtbo filename too long, limited to %dB\n",
		       sizeof(dtbo_file_name));
		return -1;
	}

	sprintf(dtbo_file_name, "%s/%s", DTBO_STORAGE_DIRECTORY, argv[1]);

#ifdef CONFIG_MMC
	ulong mmcdev;
	ulong mmcbootpart;

	mmcdev = getenv_ulong("mmcdev", 10, 0);
	mmcbootpart = getenv_ulong("mmcbootpart", 10, 1);
	snprintf(cmdbuf, sizeof(cmdbuf),
		 "ext4size  mmc %lu:%lu %s",
		 mmcdev, mmcbootpart,
		 dtbo_file_name);

	if (run_command(cmdbuf, 0) ||
	    !getenv_hex("filesize", 0)) {
		printf("file %s unknown or NULL size\n", dtbo_file_name);
		return -1;
	}
#endif

#if defined(CONFIG_NAND_FSMC)
	if (!ubifs_is_mounted()) {
		snprintf(cmdbuf, sizeof(cmdbuf),
			 "run  nandbootfs");
		if (run_command(cmdbuf, 0))
			printf("unable to mount bootfs volume\n");
	}

	snprintf(cmdbuf, sizeof(cmdbuf),
		 "ubifsexist  %s",
		 dtbo_file_name);

	if (run_command(cmdbuf, 0))
		printf("file %s unknown\n", dtbo_file_name);
#endif

	p = malloc(strlen(dtbo_files_list) + 1 + strlen(argv[1]) + 1);
	if (!p) {
		printf("unable to allocate memory %dB\n",
		       strlen(dtbo_files_list) + 1 + sizeof(argv[1]));
		return -1;
	}

	sprintf(p, "%s %s", dtbo_files_list, argv[1]);

	setenv(DTBO_LIST_VAR_NAME, p);
	free(p);
	saveenv();
	return 0;
}

U_BOOT_CMD(
	dtbo_add,   2,   0,	 do_dtbo_add,
	"add a dtbo file in dtbo files list",
	"\tdtbo_add filename.dtbo"
);
