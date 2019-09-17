set(ccc_SRCS
	sta_ccc.c
	sta_ccc_helpers.c
	sta_ccc_osal.c
	sta_ccc_plat.c
	sta_hash.c
	sta_move.c
	sta_mpaes.c
	sta_pka.c
	sta_pka_ecc.c
	sta_pka_ecdsa.c
	sta_pka_rsa.c
	sta_trng.c
)

add_library(cccdriver OBJECT ${ccc_SRCS})

add_library(cccdriver_noscheduler OBJECT ${ccc_SRCS})

target_compile_definitions(cccdriver_noscheduler PRIVATE
	NO_SCHEDULER
)
