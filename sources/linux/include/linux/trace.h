#ifndef _LINUX_TRACE_H
#define _LINUX_TRACE_H

#include <linux/ring_buffer.h>
struct trace_array;

#ifdef CONFIG_TRACING
/*
 * The trace export - an export of Ftrace. The trace_export can process
 * traces and export them to a registered destination as an addition to
 * the current only output of Ftrace - i.e. ring buffer.
 *
 * If you want traces to be sent to some other place rather than
 * ring buffer only, just need to register a new trace_export and
 * implement its own .commit() callback or just directly use
 * 'trace_generic_commit()' and hooks up its own .write() function
 * for writing traces to the storage.
 *
 * next		- pointer to the next trace_export
 * write	- copy traces which have been delt with ->commit() to
 *		  the destination
 */
struct trace_export {
	struct trace_export __rcu	*next;
	void (*write)(const char *, unsigned int);
};

int register_ftrace_export(struct trace_export *export);
int unregister_ftrace_export(struct trace_export *export);

#endif	/* CONFIG_TRACING */

#endif	/* _LINUX_TRACE_H */
