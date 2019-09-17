/*
 * ST C3 Channel Controller v3.0
 *
 * Author: Gerald Lejeune <gerald.lejeune@st.com>
 *
 * Copyright (C) 2017 STMicroelectronics Limited
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#ifndef _MPAES_AEAD_H_
#define _MPAES_AEAD_H_

extern struct mpaes_alg mpaes_aead_alg;

int mpaes_aead_set_key(struct crypto_aead *, const u8 *, unsigned int);
int mpaes_aead_setauthsize(struct crypto_aead *, unsigned int);
int mpaes_aead_encrypt(struct aead_request *);
int mpaes_aead_decrypt(struct aead_request *);

#endif /* _MPAES_AEAD_H_ */
