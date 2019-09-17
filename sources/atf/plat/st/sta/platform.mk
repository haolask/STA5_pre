#
# Copyright (c) 2013-2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

ARM_CORTEX_A7		:=	yes
ARM_WITH_NEON		:=	yes
ARM_ARCH_MAJOR		:= 	7
ARM_ARCH_MINOR		:= 	3
BL2_AT_EL3			:=	1
USE_COHERENT_MEM	:=	0
#MULTI_CONSOLE_API	:=	1
SP_MIN_WITH_SECURE_FIQ	:=	1
#DEBUG 			:= 	0

# Not needed for Cortex-A7
WORKAROUND_CVE_2017_5715:=	0

PLAT_PATH 		:=	plat/st/sta

ifeq (${MEMORY_BOOT_DEVICE},) # EMMC by default
MEMORY_BOOT_DEVICE      := 	MMC # MMC, SQI or NAND
endif

# CONSOLE_BASE is board dependant (Default UART3 for A5 boards)
CONSOLE_BASE		:=	UART3_BASE
# In case of security and safety isolation some peripherals can only be
# accessed by a secure bus master, so the related register access requests
# from REE go through secure monitor.
# If REMOTEPROC_CTRL is set, then read, write and update register requests
# are forwarded to the remote processor, relying on "regmap" mailbox channel.
# Otherwise, register requests are directly handled by the secure monitor.
# REMOTEPROC_CTRL has to be defined for safety isolation compliancy (Default
# setting for TC3P SoC)
REMOTEPROC_CTRL 	:= 	0
ifeq (${TARGET_SOC_ID},SOCID_STA1385) # TC3P SOC
	CONSOLE_BASE	:=	UART2_BASE
	REMOTEPROC_CTRL := 	1
endif

# If DTB then BL33 is Linux
ifneq ($(DTB),)
include lib/libfdt/libfdt.mk
STA_DIRECT_LINUX_BOOT := 1
NEED_FDT := yes
# Add it in FIP image as NT_FW_CONFIG image
$(eval $(call TOOL_ADD_PAYLOAD,${DTB},--nt-fw-config))
BL2_SOURCES += ${PLAT_PATH}/sta_fdt.c
else
STA_DIRECT_LINUX_BOOT := 0
endif

define assert_defined
$(if $(value $(1)),,$(error $(1) must be set))
endef

define warn_undefined
$(if $(value $(1)),,$(warning $(1) should be set))
endef

# Process flags
$(eval $(call add_define,CONSOLE_BASE))
$(eval $(call add_define,MEMORY_BOOT_DEVICE))
$(eval $(call add_define,REMOTEPROC_CTRL))
$(eval $(call add_define,STA_DIRECT_LINUX_BOOT))

# Add the build options to pack Trusted OS Extra1 and Trusted OS Extra2 images
# in the FIP if the platform requires.
ifneq ($(BL32_EXTRA1),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA1,--tos-fw-extra1))
endif
ifneq ($(BL32_EXTRA2),)
$(eval $(call TOOL_ADD_IMG,BL32_EXTRA2,--tos-fw-extra2))
endif

# Use an implementation of SHA-256 with a smaller memory footprint but reduced
# speed.
$(eval $(call add_define,MBEDTLS_SHA256_SMALLER))

PLAT_GENERATED_INCLUDES :=	${BUILD_PLAT}/include

PLAT_INCLUDES		:=	-I${PLAT_PATH}/include
PLAT_INCLUDES		+=	-Iinclude/drivers/st
PLAT_INCLUDES		+=	-I${PLAT_GENERATED_INCLUDES}
PLAT_INCLUDES		+=	${PLAT_EXTRA_INCLUDES}

PLAT_BL_COMMON_SOURCES	:=	${PLAT_PATH}/sta_common.c \
				drivers/arm/pl011/aarch32/pl011_console.S

include lib/xlat_tables_v2/xlat_tables.mk
PLAT_BL_COMMON_SOURCES	+=	${XLAT_TABLES_LIB_SRCS}
PLAT_BL_COMMON_SOURCES	+=	lib/cpus/aarch32/cortex_a7.S
PLAT_BL_COMMON_SOURCES	+=	${PLAT_PATH}/sta_hsem.c

BL2_SOURCES		+=	${PLAT_PATH}/bl2_plat_setup.c		\
				${PLAT_PATH}/sta_io_storage.c		\
				${PLAT_PATH}/sta_security.c		\
				${PLAT_PATH}/sta_helper.S	\
				drivers/gpio/gpio.c			\
				drivers/io/io_storage.c			\
				drivers/io/io_block.c			\
				drivers/io/io_fip.c			\
				drivers/mmc/mmc.c			\
				drivers/st/mmc/mmci.c			\
				drivers/st/mmc/sdhci.c 			\
				drivers/qspi/qspi_sf.c			\
				drivers/st/qspi/sta_qspi.c 		\
				drivers/st/nand/sta_nand.c 		\
				drivers/delay_timer/generic_delay_timer.c \
				drivers/delay_timer/delay_timer.c

# LOAD_IMAGE_V2
BL2_SOURCES		+=	${PLAT_PATH}/plat_bl2_mem_params_desc.c \
				${PLAT_PATH}/sta_image_load.c		\
				common/desc_image_load.c

ifeq (${ARCH}-${AARCH32_SP},aarch32-optee)
BL2_SOURCES		+=	lib/optee/optee_utils.c
endif

PLAT_INCLUDES	+=	-Iinclude/common/tbbr

# Chain of Trust
ifneq (${TRUSTED_BOARD_BOOT},0)

include drivers/auth/mbedtls/mbedtls_crypto.mk
include drivers/auth/mbedtls/mbedtls_x509.mk

# By default, ARM platforms use RSA keys
KEY_ALG		:=	rsa

# Include common TBB sources
AUTH_SOURCES	:=	drivers/auth/auth_mod.c			\
			drivers/auth/crypto_mod.c		\
			drivers/auth/img_parser_mod.c		\
			drivers/auth/tbbr/tbbr_cot.c		\
			plat/common/tbbr/plat_tbbr.c

BL2_SOURCES	+=	$(AUTH_SOURCES)				\
			${PLAT_PATH}/sta_trusted_boot.c

TF_MBEDTLS_KEY_ALG	:=	${KEY_ALG}

# ROTPK hash location
ifeq (${STA_ROTPK_LOCATION}, regs)
STA_ROTPK_LOCATION_ID	= 	STA_ROTPK_REGS_ID
else ifeq (${STA_ROTPK_LOCATION}, devel_rsa)
STA_ROTPK_LOCATION_ID	=	STA_ROTPK_DEVEL_RSA_ID
else
$(error "Unsupported STA_ROTPK_LOCATION value")
endif
$(eval $(call add_define,STA_ROTPK_LOCATION_ID))

endif
# end of Chain of Trust

ASFLAGS			+= 	-march=armv7ve -mtune=cortex-a7 -mfpu=neon-vfpv4 -mthumb -mthumb-interwork
TF_CFLAGS		+= 	-march=armv7ve -mtune=cortex-a7 -mfpu=neon-vfpv4 -mthumb -mthumb-interwork

# Platform configuration
$(eval $(call warn_undefined,MEMORY_MAP_XML))
DEFAULT_MEMORY_MAP_XML	:= ${PLAT_PATH}/include/memory_map.xml
MEMORY_MAP_XML		?= ${DEFAULT_MEMORY_MAP_XML}
MEMORY_MAP_H		:= ${PLAT_GENERATED_INCLUDES}/memory_map.h

plat_config: ${MEMORY_MAP_H}

${MEMORY_MAP_H}: ${MEMORY_MAP_XML} ${PLAT_PATH}/to_memory_map_include.xml
	@echo "  Generate $@"
	@mkdir -p ${PLAT_GENERATED_INCLUDES}
	$(Q)xsltproc -o $@ ${PLAT_PATH}/to_memory_map_include.xml ${MEMORY_MAP_XML}

plat_clean:
	@echo "  Platform clean"
	${Q}rm -f ${MEMORY_MAP_H}
