set(can_SRCS
	sta_can.c
	sta_canif.c
)

add_library(candriver OBJECT ${can_SRCS})
