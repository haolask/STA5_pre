/*
 * (C) Copyright 2017 ST-Microlectronics ADG
 *
 */

#ifndef __STA_HSM_PROVIDER_H
#define __STA_HSM_PROVIDER_H

#define C_MAX_SIG_SIZE			68
#define C_MAX_PUB_KEY_SIZE		C_MAX_SIG_SIZE

#if defined(CFG_CRYPTO_ECC) && !defined(CFG_CRYPTO_ECC_FROM_CRYPTOLIB)

int ecc_hsm_init(void);

TEE_Result ecc_key_mapping_init(void);
TEE_Result ecc_get_pub_key(uint32_t key_id, void *pub_key);
TEE_Result ecc_handle_hsm_curve(uint32_t curve);
TEE_Result ecc_load_hsm_ram_key(void *pubk_x, void *pubk_y, void *privk_d,
				uint32_t key_size);
TEE_Result ecc_get_hsm_key_id(void *kpair, void *kpub, uint32_t *key_id);

#endif /* CFG_CRYPTO_ECC && !CFG_CRYPTO_ECC_FROM_CRYPTOLIB */

#endif /* __STA_HSM_PROVIDER_H */
