/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <qspi_sf.h>
#include <mmci.h>
#include <sdhci.h>
#include <qspi_sta.h>
#include <sta_nand.h>
#include <errno.h>
#include <firmware_image_package.h>
#include <io_block.h>
#include <io_driver.h>
#include <io_memmap.h>
#include <io_fip.h>
#include <io_storage.h>
#include <mmio.h>
#include <platform.h>
#include <platform_def.h>
#include <xlat_tables_v2.h>
#include <sta_private.h>
#include <string.h>

/* IO devices */
static const io_dev_connector_t *io_dev_con;
static uintptr_t io_dev_handle;
static const io_dev_connector_t *fip_dev_con;
static uintptr_t fip_dev_handle;

static int open_io(const uintptr_t spec);
static int open_fip(const uintptr_t spec);

#if (MEMORY_BOOT_DEVICE == MMC)
#define FIP_DEVICE	"EMMC"

static const io_block_spec_t io_fip_spec = {
	.offset		= FIP_EMMC_BASE,
	.length		= FIP_MAX_SIZE,
};

static io_block_dev_spec_t io_dev_spec = {
	/* It's used as temp buffer in block driver. */
	.buffer		= {
		.offset	= BLOCKDEV_DATA_BASE,
		.length	= BLOCKDEV_DATA_SIZE,
	},
	.ops		= {
		.read	= (size_t (*)(int , uintptr_t , size_t))mmc_read_blocks,
		.write	= (size_t (*)(int , const uintptr_t , size_t))mmc_write_blocks,
	},
	.block_size	= MMC_BLOCK_SIZE,
};

#elif (MEMORY_BOOT_DEVICE == SQI)
#define FIP_DEVICE	"Serial Flash"

static const io_block_spec_t io_fip_spec = {
	.offset		= FIP_QSPI_BASE,
	.length		= FIP_MAX_SIZE,
};

static io_block_dev_spec_t io_dev_spec = {
	/* It's used as temp buffer in block driver. */
	.buffer		= {
		.offset	= BLOCKDEV_DATA_BASE,
		.length	= BLOCKDEV_DATA_SIZE,
	},
	.ops		= {
		.read	= qspi_read_blocks,
		.write	= qspi_write_blocks,
	},
	.block_size	= 1,
};
#elif (MEMORY_BOOT_DEVICE == NAND)
#define FIP_DEVICE	"NAND Flash"

static const io_block_spec_t io_fip_spec = {
	.offset		= FIP_NAND_BASE,
	.length		= FIP_MAX_SIZE,
};

static io_block_dev_spec_t io_dev_spec = {
	/* It's used as temp buffer in block driver. */
	.buffer		= {
		.offset	= BLOCKDEV_DATA_BASE,
		.length	= BLOCKDEV_DATA_SIZE,
	},
	.ops		= {
		.read	= nand_flash_read_blocks,
		.write	= NULL,
	},
	.block_size	= 4096, /* Nand page size dependent */
};
#else
#error Boot device not yet supported
#endif

static const io_uuid_spec_t bl2_uuid_spec = {
	.uuid = UUID_TRUSTED_BOOT_FIRMWARE_BL2,
};


#ifdef SCP_BL2_BASE
static const io_uuid_spec_t m3os_uuid_spec = {
	.uuid = UUID_SCP_FIRMWARE_SCP_BL2,
};
#ifdef SCP_FILE2_BL2_BASE
static const io_uuid_spec_t m3os_file2_uuid_spec = {
	.uuid = UUID_SCP_FIRMWARE_SCP_FILE2_BL2,
};
#endif
#endif

static const io_uuid_spec_t bl32_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32,
};

static const io_uuid_spec_t bl32_extra1_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA1,
};

static const io_uuid_spec_t bl32_extra2_uuid_spec = {
	.uuid = UUID_SECURE_PAYLOAD_BL32_EXTRA2,
};

static const io_uuid_spec_t bl33_uuid_spec = {
	.uuid = UUID_NON_TRUSTED_FIRMWARE_BL33,
};

#if STA_DIRECT_LINUX_BOOT
static const io_uuid_spec_t nt_fw_config_uuid_spec = {
	.uuid = UUID_NT_FW_CONFIG,
};
#endif

#if TRUSTED_BOARD_BOOT
static const io_uuid_spec_t sta_trusted_key_cert_spec = {
	.uuid = UUID_TRUSTED_KEY_CERT,
};

static const io_uuid_spec_t sta_scp_fw_key_cert_spec = {
	.uuid = UUID_SCP_FW_KEY_CERT,
};

static const io_uuid_spec_t sta_soc_fw_key_cert_spec = {
	.uuid = UUID_SOC_FW_KEY_CERT,
};

static const io_uuid_spec_t sta_tos_fw_key_cert_spec = {
	.uuid = UUID_TRUSTED_OS_FW_KEY_CERT,
};

static const io_uuid_spec_t sta_nt_fw_key_cert_spec = {
	.uuid = UUID_NON_TRUSTED_FW_KEY_CERT,
};

static const io_uuid_spec_t sta_scp_fw_cert_spec = {
	.uuid = UUID_SCP_FW_CONTENT_CERT,
};

static const io_uuid_spec_t sta_soc_fw_cert_spec = {
	.uuid = UUID_SOC_FW_CONTENT_CERT,
};

static const io_uuid_spec_t sta_tos_fw_cert_spec = {
	.uuid = UUID_TRUSTED_OS_FW_CONTENT_CERT,
};

static const io_uuid_spec_t sta_nt_fw_cert_spec = {
	.uuid = UUID_NON_TRUSTED_FW_CONTENT_CERT,
};
#endif /* TRUSTED_BOARD_BOOT */

struct plat_io_policy {
	uintptr_t *dev_handle;
	uintptr_t image_spec;
	int (*check)(const uintptr_t spec);
};

static const struct plat_io_policy policies[] = {
	[FIP_IMAGE_ID] = {
		&io_dev_handle,
		(uintptr_t)&io_fip_spec,
		open_io
	},
	[BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl2_uuid_spec,
		open_fip
	},
#ifdef SCP_BL2_BASE
	[SCP_BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&m3os_uuid_spec,
		open_fip
	},
#ifdef SCP_FILE2_BL2_BASE
	[SCP_FILE2_BL2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&m3os_file2_uuid_spec,
		open_fip
	},
#endif
#endif
	[BL32_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_uuid_spec,
		open_fip
	},
	[BL32_EXTRA1_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra1_uuid_spec,
		open_fip
	},
	[BL32_EXTRA2_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl32_extra2_uuid_spec,
		open_fip
	},
	[BL33_IMAGE_ID] = {
		&fip_dev_handle,
		(uintptr_t)&bl33_uuid_spec,
		open_fip
	},
#if STA_DIRECT_LINUX_BOOT
	/* This image contains the Linux DTB */
	[NT_FW_CONFIG_ID] = {
		&fip_dev_handle,
		(uintptr_t)&nt_fw_config_uuid_spec,
		open_fip
	},
#endif
#if TRUSTED_BOARD_BOOT
	[TRUSTED_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&sta_trusted_key_cert_spec,
		open_fip
	},
	[SCP_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&sta_scp_fw_key_cert_spec,
		open_fip
	},
	[SOC_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&sta_soc_fw_key_cert_spec,
		open_fip
	},
	[TRUSTED_OS_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&sta_tos_fw_key_cert_spec,
		open_fip
	},
	[NON_TRUSTED_FW_KEY_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&sta_nt_fw_key_cert_spec,
		open_fip
	},
	[SCP_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&sta_scp_fw_cert_spec,
		open_fip
	},
	[SOC_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&sta_soc_fw_cert_spec,
		open_fip
	},
	[TRUSTED_OS_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&sta_tos_fw_cert_spec,
		open_fip
	},
	[NON_TRUSTED_FW_CONTENT_CERT_ID] = {
		&fip_dev_handle,
		(uintptr_t)&sta_nt_fw_cert_spec,
		open_fip
	},
#endif /* TRUSTED_BOARD_BOOT */
};

static int open_io(const uintptr_t spec)
{
	int result;
	uintptr_t local_handle;

	result = io_dev_init(io_dev_handle, (uintptr_t)NULL);
	if (result == 0) {
		result = io_open(io_dev_handle, spec, &local_handle);
		if (result == 0) {
			VERBOSE("Using %s\n", FIP_DEVICE);
			io_close(local_handle);
		}
	}
	return result;
}

static int open_fip(const uintptr_t spec)
{
	int result;
	uintptr_t local_image_handle;

	/* See if a Firmware Image Package is available */
	result = io_dev_init(fip_dev_handle, (uintptr_t)FIP_IMAGE_ID);
	if (result == 0) {
		result = io_open(fip_dev_handle, spec, &local_image_handle);
		if (result == 0) {
			VERBOSE("Using FIP\n");
			io_close(local_image_handle);
		}
	}
	return result;
}

#if (MEMORY_BOOT_DEVICE == MMC)
mmap_reserve(MAP_MMC0);
mmap_reserve(MAP_MMC1);
mmap_reserve(MAP_MMC2);
#elif (MEMORY_BOOT_DEVICE == SQI)
mmap_reserve(MAP_SQI);
mmap_reserve(MAP_DEVICE3);
#elif (MEMORY_BOOT_DEVICE == NAND)
mmap_reserve(MAP_NAND);
mmap_reserve(MAP_DEVICE2);
#endif
static void sta_io_init(void)
{
#if (MEMORY_BOOT_DEVICE == MMC)
	mmci_params_t mmci_params;
	sdhc_params_t sdhc_params;
	uintptr_t mmc_base;

	switch (get_mmc_boot_dev()) {
	case 0:
		memset(&sdhc_params, 0, sizeof(sdhc_params_t));
		assert(IN_MMAP(MMC0_BASE, MAP_MMC0));
		sdhc_params.reg_base = MMC0_BASE;
		sdhc_params.clk_rate = 100 * 1000 * 1000;
		sdhc_params.bus_width = MMC_BUS_WIDTH_8;
		sdhc_params.io_volt = IO_VOLT_180;
		sta_sdhc_init(&sdhc_params);
		break;
	case 1:
		assert(IN_MMAP(MMC1_BASE, MAP_MMC1));
		mmc_base = MMC1_BASE;
		/* FALLTHROUGH !!!! */
	default:
		if (get_mmc_boot_dev() == 2) {
			assert(IN_MMAP(MMC2_BASE, MAP_MMC2));
			mmc_base = MMC2_BASE;
		}
		memset(&mmci_params, 0, sizeof(mmci_params_t));
		mmci_params.reg_base = mmc_base;
		mmci_params.clk_rate = 100 * 1000 * 1000;
		mmci_params.bus_width = MMC_BUS_WIDTH_8;
		sta_mmci_init(&mmci_params);
		break;
	}

#elif (MEMORY_BOOT_DEVICE == SQI)
	assert(IN_MMAP(QSPI0_BASE, MAP_SQI));
	assert(IN_MMAP(QSPI0_NOR_MEM_MAP_BASE, MAP_DEVICE3));
	sta_qspi_init(QSPI0_BASE, QSPI0_NOR_MEM_MAP_BASE);
#elif (MEMORY_BOOT_DEVICE == NAND)
	assert(sta_nand_init() == 0);
	assert(IN_MMAP(FSMC_BASE, MAP_NAND));
	assert(IN_MMAP(NAND_CS0_BASE, MAP_DEVICE2));
	/* TODO: Pass FSMC_BASE and NAND_CS0_BASE to nand init API */
	io_dev_spec.block_size = nand_page_size();
#else
#error Wrong boot device
#endif
}

void sta_io_setup(void)
{
	int result;

	sta_io_init();

	/* Register the IO devices on this platform */
	result = register_io_dev_block(&io_dev_con);
	assert(result == 0);

	result = register_io_dev_fip(&fip_dev_con);
	assert(result == 0);

	result = io_dev_open(io_dev_con, (uintptr_t)&io_dev_spec,
			     &io_dev_handle);
	assert(result == 0);

	result = io_dev_open(fip_dev_con, (uintptr_t)NULL, &fip_dev_handle);
	assert(result == 0);

	/* Ignore improbable errors in release builds */
	(void)result;
}

/*
 * Return an IO device handle and specification which can be used to access
 * an image. Use this to enforce platform load policy
 */
int plat_get_image_source(unsigned int image_id, uintptr_t *dev_handle,
			  uintptr_t *image_spec)
{
	int rc;
	const struct plat_io_policy *policy;

	assert(image_id < ARRAY_SIZE(policies));

	policy = &policies[image_id];
	rc = policy->check(policy->image_spec);
	if (rc == 0) {
		*image_spec = policy->image_spec;
		*dev_handle = *policy->dev_handle;
	}

	return rc;
}
