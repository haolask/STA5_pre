add_definitions(-DSOC_ID=${SOC_ID} -DBOARD_ID=${BOARD_ID})

set(platform_SRCS
	sta_platform.c
	sta_pinmux.c
)

add_library(platform STATIC ${platform_SRCS})

install(TARGETS platform DESTINATION lib${LIB_SUFFIX})
