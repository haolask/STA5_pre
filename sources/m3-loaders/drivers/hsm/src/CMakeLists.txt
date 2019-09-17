set(hsm_SRCS
	CSE_AES_API_sc.c
	CSE_AES_HW_Modes.c
	CSE_cmd_param.c
	CSE_ext_AES256.c
	CSE_ext_ECC.c
	CSE_ext_ECC_ECDH.c
	CSE_ext_ECC_ECIES.c
	CSE_ext_hash.c
	CSE_ext_HMAC.c
	CSE_ext_manager.c
	CSE_ext_OTP.c
	CSE_ext_RSA.c
	CSE_ext_TLSv12_PRF.c
	CSE_HAL.c
	CSE_Key.c
	CSE_Manager.c
	CSE_RNG.c
)


add_definitions(-DCSE_HSM)
add_library(hsmdriver OBJECT ${hsm_SRCS})
