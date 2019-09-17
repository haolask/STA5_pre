set(hsm_key_storage_SRCS
	cmd_mailbox.c
	ks_proxy.c
	loadstore.c
	rpmx_common.c
)

add_library(hsm_ks_driver OBJECT ${hsm_key_storage_SRCS})
