#
# Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

BL32_REGMAP_SMC		?=	yes

BL32_SOURCES		+=	plat/st/sta/sta_helper.S \
				plat/common/aarch32/platform_mp_stack.S \
				plat/st/sta/sp_min/sp_min_setup.c \
				plat/common/plat_psci_common.c \
				plat/st/sta/sta_topology.c \
				plat/st/sta/sta_pm.c \
				plat/st/sta/sta_mbox.c \
				drivers/delay_timer/generic_delay_timer.c \
				drivers/delay_timer/delay_timer.c
# GIC v2
BL32_SOURCES		+=	plat/st/sta/sta_gic.c \
				plat/common/plat_gicv2.c \
				drivers/arm/gic/v2/gicv2_main.c \
				drivers/arm/gic/v2/gicv2_helpers.c \
				drivers/arm/gic/common/gic_common.c

ifeq (${BL32_REGMAP_SMC},yes)
BL32_SOURCES		+=	plat/st/sta/sta_svc_setup.c
endif
