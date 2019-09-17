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
#ifndef _MPAES_ABLK_H_
#define _MPAES_ABLK_H_

extern struct mpaes_alg mpaes_ablk_alg;

int mpaes_ablk_set_key(struct crypto_ablkcipher *, const u8 *, unsigned int);
int mpaes_ablk_encrypt(struct ablkcipher_request *);
int mpaes_ablk_decrypt(struct ablkcipher_request *);

#endif /* _MPAES_ABLK_H_ */
