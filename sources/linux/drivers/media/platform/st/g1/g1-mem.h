/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Chetan Nanda <chetan.nanda@st.com>
 *          Hugues Fruchet <hugues.fruchet@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#ifndef G1_MEM_H
#define G1_MEM_H

int hw_alloc(struct g1_ctx *ctx, __u32 size, const char *name,
	     struct g1_buf *buf);
void hw_free(struct g1_ctx *ctx, struct g1_buf *buf);

#endif /* G1_MEM_H */
