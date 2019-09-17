/*
 * Copyright (C) ST Microelectronics 2017
 *
 * Author:	Gian Antonio Sampietro <gianantonio.sampietro@st.com>,
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */
#include "internal.h"

#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>

#define TRACE_SIZE 128
static LIST_HEAD(chunk_list);
static DEFINE_RWLOCK(trace_lock);

static bool trace_active;

struct trace {
	u32 addr, value;
};

static struct chunk {
	struct trace buffer[TRACE_SIZE];
	int trace_count;
	struct list_head list;
} *curr_chunk;

void dsp_trace_enable(bool status)
{
	struct chunk *c;
	struct list_head *p, *n;

	if (status == trace_active)
		return;

	read_lock(&trace_lock);
	trace_active = status;
	if (status) {
		list_for_each_safe(p, n, &chunk_list) {
			c = list_entry(p, struct chunk, list);
			list_del(&c->list);
			vfree(c);
		}
		curr_chunk = NULL;
	}
	read_unlock(&trace_lock);
}

void dsp_trace_print(u32 addr, u32 value)
{
	if (!trace_active)
		return;

	if (!curr_chunk || curr_chunk->trace_count >= TRACE_SIZE - 1) {
		curr_chunk = vzalloc(sizeof(struct chunk));
		WARN_ON(!curr_chunk);
		list_add_tail(&curr_chunk->list, &chunk_list);
	}
	curr_chunk->buffer[curr_chunk->trace_count].addr = addr;
	curr_chunk->buffer[curr_chunk->trace_count].value = value;
	curr_chunk->trace_count++;
}

static void *trace_seq_start(struct seq_file *seq, loff_t *pos)
	__acquires(trace_lock)
{
	read_lock(&trace_lock);
	return seq_list_start(&chunk_list, *pos);
}

static void *trace_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	return seq_list_next(v, &chunk_list, pos);
}

static void trace_seq_stop(struct seq_file *seq, void *v)
	__releases(trace_lock)
{
	read_unlock(&trace_lock);
}

static int trace_seq_show(struct seq_file *seq, void *v)
{
	unsigned long pfn;
	struct page *page;
	struct chunk *c = list_entry(v, struct chunk, list);
	u32 addr, value;
	int i;

	for (i = 0; i < c->trace_count; i++) {
		addr = c->buffer[i].addr;
		value = c->buffer[i].value;
		page = vmalloc_to_page((void *)addr);
		pfn = page_to_pfn(page);
		addr = (pfn << PAGE_SHIFT) | (addr & (PAGE_SIZE - 1));
		seq_printf(seq, "%x: %x\n", addr, value);
	}
	return 0;
}

static const struct seq_operations trace_seqops = {
	.start = trace_seq_start,
	.next = trace_seq_next,
	.stop = trace_seq_stop,
	.show = trace_seq_show,
};

static int trace_info_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &trace_seqops);
}

const struct file_operations trace_fops = {
	.owner = THIS_MODULE,
	.open = trace_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release,
};

int dsp_trace_init(void)
{
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *proc_fs_sta;

	proc_fs_sta = proc_mkdir("sta", NULL);
	if (!proc_fs_sta)
		return -EINVAL;

	if (!proc_create("trace", 0444, proc_fs_sta, &trace_fops))
		return -ENOMEM;

	return 0;
#endif
}

