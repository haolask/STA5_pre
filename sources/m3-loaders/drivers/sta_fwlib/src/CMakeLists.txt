set(sta_fwlib_SRCS
	flash_cache.c
	mmc_transfer.s
	nand_fast.s
	sdmmc.c
	sta_acc.c
	sta_crc.c
	sta_dsp.c
	sta_gpio.c
	sta_hsem.c
	sta_i2c.c
	sta_lcd.c
	sta_lib.c
	sta_m3_irq.c
	sta_mbox.c
	sta_mtu.c
	sta_nand.c
	sta_nvic.c
	sta_pmu.c
	sta_pm.c
	sta_rtc.c
	sta_a7.c
	sta_src.c
	sta_src_a7.c
	sta_mmci.c
	sta_sdhc.c
	sta_ahbapb.c
	sta_nic_security.c
	sta_qos.c
	sta_sqi.c
	sta_systick.c
	sta_uart.c
	stmpe1600.c
	tca9535.c
	sta_ltdc.c
	sta_tvdec.c
	sta_sga.c
	sta_cam.c
	sta_vip.c
	sta_adc.c
	sta_usb.c
	sta_stm.c
	sta_wdt.c
	sta_wdt_a7.c
	sta_ddr.c
	sta_math.c
	sta_ampli.c
	sta_spi.c
	sta_cot.c
	sta_m3ss_otp.c
	sta_dma.c
	sta_regmap.c
	sta_regmap_firewall.c
        sta_eft.c
)

# DDR3 support for sta1195, sta1295 and sta1275
set(sta1x95_ddr3_SRCS
	sta1x95_ddr3_pub.c
	sta1x95_ddr3_ctl.c
	sta1x95_ddr3_setting.c
)

# DDR3 support for sta1385
set(sta1385_ddr3_SRCS
	sta1385_ddr3_publ.c
	sta1385_ddr3_ctl.c
	sta1385_ddr3_setting.c
)

# LPDDR2 support for sta1385
set(sta1385_lpddr2_SRCS
	sta1385_lpddr2_publ.c
	sta1385_lpddr2_ctl.c
	sta1385_lpddr2_setting.c
)

set_property(SOURCE mmc_transfer.s PROPERTY LANGUAGE C)
set_property(SOURCE nand_fast.s PROPERTY LANGUAGE C)

if (${SOC_ID} STREQUAL "SOCID_STA1385")
	if (LPDDR2)
		list(APPEND sta_fwlib_SRCS ${sta1385_lpddr2_SRCS})
	else()
		list(APPEND sta_fwlib_SRCS ${sta1385_ddr3_SRCS})
	endif()
else()
	list(APPEND sta_fwlib_SRCS ${sta1x95_ddr3_SRCS})
endif()

add_library(sta_fwlib OBJECT ${sta_fwlib_SRCS})
add_library(sta_fwlib_noscheduler OBJECT ${sta_fwlib_SRCS})
add_library(sta_fwlib_noscheduler_verbose OBJECT ${sta_fwlib_SRCS})
include_directories(${PROJECT_SOURCE_DIR}/services/inc/)
include_directories(${PROJECT_SOURCE_DIR}/drivers/vpu/source/inc)

target_compile_definitions(sta_fwlib_noscheduler PRIVATE
	NO_SCHEDULER
)

target_compile_definitions(sta_fwlib_noscheduler_verbose PRIVATE
	VERBOSE_DDR
	NO_SCHEDULER
)
