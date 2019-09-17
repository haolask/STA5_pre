/*
 * Copyright (C) STMicroelectronics SA 2015
 * Authors: Hugues Fruchet <hugues.fruchet@st.com>
 *          Fabrice Lecoultre <fabrice.lecoultre@st.com>
 *          for STMicroelectronics.
 * License terms:  GNU General Public License (GPL), version 2
 */

#ifndef G1_DEBUG_H
#define G1_DEBUG_H

/*
 * struct mem_info
 *
 * @size:		size of memory allocated
 * @nb_of_allocations:	counter to know number of times this type of
 *			allocations have been done
 * @name:		name of the allocation
 */
struct mem_info {
	struct list_head list;
	int size;
	int nb_of_allocations;
	unsigned char name[50];
	const char *type;
};

char *g1_streaminfo_str(struct g1_streaminfo *s, char *str, unsigned int len);
char *g1_frameinfo_str(struct g1_frameinfo *s, char *str, unsigned int len);
char *g1_summary_str(struct g1_ctx *ctx, char *str, unsigned int len);

void g1_debugfs_create(struct g1_dev *g1);
void g1_debugfs_remove(struct g1_dev *g1);
void g1_debugfs_open(struct g1_ctx *ctx);
void g1_debugfs_close(struct g1_ctx *ctx);

int g1_debugfs_track_mem(struct mem_info *buf, struct g1_ctx *ctx,
			 const unsigned char *type);
unsigned char *mem_status(struct g1_ctx *ctx);

void g1_perf_begin(struct g1_ctx *ctx, struct g1_au *au);
void g1_perf_end(struct g1_ctx *ctx);
void g1_perf_output(struct g1_ctx *ctx);

#endif /* G1_DEBUG_H */
