#/*
# * Copyright (C) 2017 ST Microelectronics Limited
# * Author: Gerald Lejeune <gerald.lejeune@st.com>
# * License terms: GNU General Public License (GPL) version 2  */

ifdef CONFIG_CRYPTO_DEV_CCC_DEBUG
	EXTRA_CFLAGS := -DDEBUG -O0
endif

obj-$(CONFIG_CRYPTO_DEV_CCC_PKA) += ccc_pka.o
ccc_pka-objs :=  pka.o
