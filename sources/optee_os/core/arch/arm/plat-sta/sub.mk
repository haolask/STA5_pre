global-incdirs-y += .
global-incdirs-y += ./hsm/inc

srcs-y += a7_plat_init.S
srcs-y += sta_helper_a32.S
srcs-$(CFG_PSCI_ARM32) += sta_psci.c
srcs-$(CFG_SM_PLATFORM_HANDLER) += sta_svc_setup.c
srcs-y += sta_mbox.c
srcs-y += main.c
ifeq ($(PLATFORM_FLAVOR),1385)
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_AES_API_sc.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_AES_HW_Modes.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_cmd_param.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_ext_AES256.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_ext_ECC_ECDH.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_ext_ECC_ECIES.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_ext_ECC.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_ext_hash.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_ext_HMAC.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_ext_manager.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_ext_OTP.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_ext_RSA.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_ext_TLSv12_PRF.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_HAL.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_Key.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_Manager.c
srcs-$(CFG_CRYPTO_WITH_HSM) += hsm/src/CSE_RNG.c
srcs-$(CFG_CRYPTO_WITH_HSM) += sta_hsm_provider.c
srcs-$(CFG_CRYPTO_WITH_HSM) += sta_hsm_pta.c
endif
