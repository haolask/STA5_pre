/*
 * Copyright (C) 2002 Roman Zippel <zippel@linux-m68k.org>
 * Released under the terms of the GNU GPL v2.0.
 */

#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//#include <unistd.h>
#define PATH_MAX 256

#define LKC_DIRECT_LINK
#include "lkc.h"

extern char* defname;
extern char* makname;
extern char* hname;

/*
static void conf_warning(const char *fmt, ...)
	__attribute__ ((format (printf, 1, 2)));
*/

static void conf_warning(const char *fmt, ...);

static const char *conf_filename;
static int conf_lineno, conf_warnings, conf_unsaved;

char conf_def_filename[1024] = ".config";

char conf_defname[1024] = "config.mak";

char *conf_confnames[3] = {
  ".config",
  conf_defname,
  NULL,
};

#include <gtk/gtk.h>

extern GtkWidget *window_main;

static void conf_warning(const char *fmt, ...)
{
#if 1
	va_list ap;
	GtkTextIter iter;
	GtkTextBuffer* buffer;
	char str1[2028*16];
	char str2[2028*16];
	buffer = gtk_text_view_get_buffer(lookup_widget(window_main,"textview_log"));
	va_start(ap, fmt);
	sprintf(str1, "%s:%d:warning: ", conf_filename, conf_lineno);
	sprintf(str2, fmt, ap);
	strcat(str1,str2);
	strcat(str1,"\n");
	gtk_text_buffer_get_iter_at_offset (buffer, &iter, -1);
	gtk_text_buffer_insert (buffer, &iter, str1, -1);
	va_end(ap);	
#endif
#if 0
	va_list ap;
	va_start(ap, fmt);
	g_fprintf(stderr, "%s:%d:warning: ", conf_filename, conf_lineno);
	vfprintf(stderr, fmt, ap);
	g_fprintf(stderr, "\n");
	va_end(ap);
	conf_warnings++;
#endif
}

static char *conf_expand_value(const char *in)
{
	struct symbol *sym;
	const char *src;
	static char res_value[SYMBOL_MAXLENGTH];
	char *dst, name[SYMBOL_MAXLENGTH];

	res_value[0] = 0;
	dst = name;
	while ((src = strchr(in, '$'))) {
		strncat(res_value, in, src - in);
		src++;
		dst = name;
		while (isalnum(*src) || *src == '_')
			*dst++ = *src++;
		*dst = 0;
		sym = sym_lookup(name, 0);
		sym_calc_value(sym);
		strcat(res_value, sym_get_string_value(sym));
		in = src;
	}
	strcat(res_value, in);

	return res_value;
}

char *conf_get_default_confname(void)
{
	struct stat buf;
	static char fullname[PATH_MAX+1];
	char *env, *name;

	name = conf_expand_value(conf_defname);
	env = getenv(SRCTREE);
	if (env) {
		sprintf(fullname, "%s/%s", env, name);
		if (!stat(fullname, &buf))
			return fullname;
	}
	return name;
}

int conf_read_simple(const char *name)
{
	FILE *in = NULL;
	char line[1024];
	char *p, *p2;
	struct symbol *sym;
	int i;

	if (name) {
		in = zconf_fopen(name);
	} else {
		const char **names = conf_confnames;
		while ((name = *names++)) {
			name = conf_expand_value(name);
			in = zconf_fopen(name);
			if (in) {
				printf(_("#\n"
				         "# using defaults found in %s\n"
				         "#\n"), name);
				break;
			}
		}
	}
	if (!in)
		return 1;

	conf_filename = name;
	conf_lineno = 0;
	conf_warnings = 0;
	conf_unsaved = 0;

	for_all_symbols(i, sym) {
		sym->flags |= SYMBOL_NEW | SYMBOL_CHANGED;
		if (sym_is_choice(sym))
			sym->flags &= ~SYMBOL_NEW;
		sym->flags &= ~SYMBOL_VALID;
		switch (sym->type) {
		case S_INT:
		case S_HEX:
		case S_STRING:
			if (sym->user.val)
				free(sym->user.val);
		default:
			sym->user.val = NULL;
			sym->user.tri = no;
		}
	}

	while (fgets(line, sizeof(line), in)) {
		conf_lineno++;
		sym = NULL;
		switch (line[0]) {
		case '#':
			if (memcmp(line + 2, "CONFIG_", 7))
				continue;
			p = strchr(line + 9, ' ');
			if (!p)
				continue;
			*p++ = 0;
			if (strncmp(p, "is not set", 10))
				continue;
			sym = sym_find(line + 9);
			if (!sym) {
				conf_warning("trying to assign nonexistent symbol %s", line + 9);
				break;
			} else if (!(sym->flags & SYMBOL_NEW)) {
				conf_warning("trying to reassign symbol %s", sym->name);
				break;
			}
			switch (sym->type) {
			case S_BOOLEAN:
			case S_TRISTATE:
				sym->user.tri = no;
				sym->flags &= ~SYMBOL_NEW;
				break;
			default:
				;
			}
			break;
		case 'C':
			if (memcmp(line, "CONFIG_", 7)) {
				conf_warning("unexpected data");
				continue;
			}
			p = strchr(line + 7, '=');
			if (!p)
				continue;
			*p++ = 0;
			p2 = strchr(p, '\n');
			if (p2)
				*p2 = 0;
			sym = sym_find(line + 7);
			if (!sym) {
				conf_warning("trying to assign nonexistent symbol %s", line + 7);
				break;
			} else if (!(sym->flags & SYMBOL_NEW)) {
				conf_warning("trying to reassign symbol %s", sym->name);
				break;
			}
			switch (sym->type) {
			case S_TRISTATE:
				if (p[0] == 'm') {
					sym->user.tri = mod;
					sym->flags &= ~SYMBOL_NEW;
					break;
				}
			case S_BOOLEAN:
				if (p[0] == 'y') {
					sym->user.tri = yes;
					sym->flags &= ~SYMBOL_NEW;
					break;
				}
				if (p[0] == 'n') {
					sym->user.tri = no;
					sym->flags &= ~SYMBOL_NEW;
					break;
				}
				conf_warning("symbol value '%s' invalid for %s", p, sym->name);
				break;
			case S_STRING:
				if (*p++ != '"')
					break;
				for (p2 = p; (p2 = strpbrk(p2, "\"\\")); p2++) {
					if (*p2 == '"') {
						*p2 = 0;
						break;
					}
					memmove(p2, p2 + 1, strlen(p2));
				}
				if (!p2) {
					conf_warning("invalid string found");
					continue;
				}
			case S_INT:
			case S_HEX:
				if (sym_string_valid(sym, p)) {
					sym->user.val = strdup(p);
					sym->flags &= ~SYMBOL_NEW;
				} else {
					conf_warning("symbol value '%s' invalid for %s", p, sym->name);
					continue;
				}
				break;
			default:
				;
			}
			break;
		case '\n':
			break;
		default:
			conf_warning("unexpected data");
			continue;
		}
		if (sym && sym_is_choice_value(sym)) {
			struct symbol *cs = prop_get_symbol(sym_get_choice_prop(sym));
			switch (sym->user.tri) {
			case no:
				break;
			case mod:
				if (cs->user.tri == yes) {
					conf_warning("%s creates inconsistent choice state", sym->name);
					cs->flags |= SYMBOL_NEW;
				}
				break;
			case yes:
				if (cs->user.tri != no) {
					conf_warning("%s creates inconsistent choice state", sym->name);
					cs->flags |= SYMBOL_NEW;
				} else
					cs->user.val = sym;
				break;
			}
			cs->user.tri = E_OR(cs->user.tri, sym->user.tri);
		}
	}
	fclose(in);

	if (modules_sym)
		sym_calc_value(modules_sym);
	return 0;
}

int conf_read(const char *name)
{
	struct symbol *sym;
	struct property *prop;
	struct expr *e;
	int i;

	strcpy(conf_def_filename,makname);
	strcpy(conf_defname,defname);
	conf_confnames[0] = makname;
	conf_confnames[1] = defname;

	if (conf_read_simple(name))
		return 1;

	for_all_symbols(i, sym) {
		sym_calc_value(sym);
		if (sym_is_choice(sym) || (sym->flags & SYMBOL_AUTO))
			goto sym_ok;
		if (sym_has_value(sym) && (sym->flags & SYMBOL_WRITE)) {
			/* check that calculated value agrees with saved value */
			switch (sym->type) {
			case S_BOOLEAN:
			case S_TRISTATE:
				if (sym->user.tri != sym_get_tristate_value(sym))
					break;
				if (!sym_is_choice(sym))
					goto sym_ok;
			default:
				if (!strcmp(sym->curr.val, sym->user.val))
					goto sym_ok;
				break;
			}
		} else if (!sym_has_value(sym) && !(sym->flags & SYMBOL_WRITE))
			/* no previous value and not saved */
			goto sym_ok;
		conf_unsaved++;
		/* maybe print value in verbose mode... */
	sym_ok:
		if (sym_has_value(sym) && !sym_is_choice_value(sym)) {
			if (sym->visible == no)
				sym->flags |= SYMBOL_NEW;
			switch (sym->type) {
			case S_STRING:
			case S_INT:
			case S_HEX:
				if (!sym_string_within_range(sym, sym->user.val)) {
					sym->flags |= SYMBOL_NEW;
					sym->flags &= ~SYMBOL_VALID;
				}
			default:
				break;
			}
		}
		if (!sym_is_choice(sym))
			continue;
		prop = sym_get_choice_prop(sym);
		for (e = prop->expr; e; e = e->left.expr)
			if (e->right.sym->visible != no)
				sym->flags |= e->right.sym->flags & SYMBOL_NEW;
	}

	sym_change_count = conf_warnings && conf_unsaved;

	return 0;
}

#include <stdio.h>

int my_fwrite( char* str, int l, int n, FILE* out)
{
	int j=-1;
	int i;
	for(i=0;i<(l*n);i++)
	{
		g_fprintf(out,"%c",str[i]);
		j++;
	}
	return j;
}

int conf_write(const char *name)
{
	FILE *out, *out_h;
	struct symbol *sym;
	struct menu *menu;
	const char *basename;
	//char dirname[128], tmpname[128], newname[128];
	int type, l;
	const char *str;
	time_t now;
	int use_timestamp = 1;
	char *env;

#if 0
	dirname[0] = 0;
	if (name && name[0]) {
		struct stat st;
		char *slash;

		if (!stat(name, &st) && S_ISDIR(st.st_mode)) {
			strcpy(dirname, name);
			strcat(dirname, "/");
			basename = conf_def_filename;
		} else 
		if ((slash = strrchr(name, '/'))) {
			int size = slash - name + 1;
			memcpy(dirname, name, size);
			dirname[size] = 0;
			if (slash[1])
				basename = slash + 1;
			else
				basename = conf_def_filename;
		} else
			basename = name;
	} else
#endif
	basename = conf_def_filename;

	//sprintf(newname, "%s.tmpconfig.%d", dirname, (int)getpid());
	//sprintf(newname, "tmpconfig.%d", (int)getpid());
	//sprintf(newname,"%s.tmp",basename);
	out = g_fopen(basename, "w");

	if (!out)
		return 1;
	out_h = NULL;
	if (!name) {
		out_h = g_fopen(hname, "w+");
		if (!out_h)
			return 1;
	}
	sym = sym_lookup("KERNELVERSION", 0);
	sym_calc_value(sym);
	time(&now);
	env = getenv("KCONFIG_NOTIMESTAMP");
	if (env && *env)
		use_timestamp = 0;

	g_fprintf(out, _("#\n"
		       "# Automatically generated make config: don't edit\n"
		       "# Linux kernel version: %s\n"
		       "%s%s"
		       "#\n"),
		     sym_get_string_value(sym),
		     use_timestamp ? "# " : "",
		     use_timestamp ? ctime(&now) : "");
	if (out_h)
		g_fprintf(out_h, "/*\n"
			       " * Automatically generated C config: don't edit\n"
			       " * Linux kernel version: %s\n"
			       "%s%s"
			       " */\n"
			       "#define AUTOCONF_INCLUDED\n",
			       sym_get_string_value(sym),
			       use_timestamp ? " * " : "",
			       use_timestamp ? ctime(&now) : "");

	if (!sym_change_count)
		sym_clear_all_valid();

	menu = rootmenu.list;
	while (menu) {
		sym = menu->sym;
		if (!sym) {
			if (!menu_is_visible(menu))
				goto next;
			str = menu_get_prompt(menu);
			g_fprintf(out, "\n"
				     "#\n"
				     "# %s\n"
				     "#\n", str);
			if (out_h)
				g_fprintf(out_h, "\n"
					       "/*\n"
					       " * %s\n"
					       " */\n", str);
		} else if (!(sym->flags & SYMBOL_CHOICE)) {
			sym_calc_value(sym);
			if (!(sym->flags & SYMBOL_WRITE))
				goto next;
			sym->flags &= ~SYMBOL_WRITE;
			type = sym->type;
			if (type == S_TRISTATE) {
				sym_calc_value(modules_sym);
				if (modules_sym->curr.tri == no)
					type = S_BOOLEAN;
			}
			switch (type) {
			case S_BOOLEAN:
			case S_TRISTATE:
				switch (sym_get_tristate_value(sym)) {
				case no:
					g_fprintf(out, "# CONFIG_%s is not set\n", sym->name);
					if (out_h)
						g_fprintf(out_h, "#undef CONFIG_%s\n", sym->name);
					break;
				case mod:
					g_fprintf(out, "CONFIG_%s=m\n", sym->name);
					if (out_h)
						g_fprintf(out_h, "#define CONFIG_%s_MODULE 1\n", sym->name);
					break;
				case yes:
					g_fprintf(out, "CONFIG_%s=y\n", sym->name);
					if (out_h)
						g_fprintf(out_h, "#define CONFIG_%s 1\n", sym->name);
					break;
				}
				break;
			case S_STRING:
				// fix me
				str = sym_get_string_value(sym);
				g_fprintf(out, "CONFIG_%s=\"", sym->name);
				if (out_h)
					g_fprintf(out_h, "#define CONFIG_%s \"", sym->name);
				do {
					l = strcspn(str, "\"\\");
					if (l) {						
						my_fwrite(str, l, 1, out);
						if (out_h)
							my_fwrite(str, l, 1, out_h);
					}
					str += l;
					while (*str == '\\' || *str == '"') {
						g_fprintf(out, "\\%c", *str);
						if (out_h)
							g_fprintf(out_h, "\\%c", *str);
						str++;
					}
				} while (*str);
				g_fprintf(out,"\"\n");
				if (out_h)
					g_fprintf(out_h,"\"\n");
				break;
			case S_HEX:
				str = sym_get_string_value(sym);
				if (str[0] != '0' || (str[1] != 'x' && str[1] != 'X')) {
					g_fprintf(out, "CONFIG_%s=%s\n", sym->name, str);
					if (out_h)
						g_fprintf(out_h, "#define CONFIG_%s 0x%s\n", sym->name, str);
					break;
				}
			case S_INT:
				str = sym_get_string_value(sym);
				g_fprintf(out, "CONFIG_%s=%s\n", sym->name, str);
				if (out_h)
					g_fprintf(out_h, "#define CONFIG_%s %s\n", sym->name, str);
				break;
			}
		}

	next:
		if (menu->list) {
			menu = menu->list;
			continue;
		}
		if (menu->next)
			menu = menu->next;
		else while ((menu = menu->parent)) {
			if (menu->next) {
				menu = menu->next;
				break;
			}
		}
	}
	//close(out);
	if (out_h) {
		//fclose(out_h);
/* 		rename(, "include/linux/autoconf.h"); */
		file_write_dep(NULL);
	}
#if 0
	if (!name || basename != conf_def_filename) {
		if (!name)
			name = conf_def_filename;
		sprintf(tmpname, "%s.old", name);
		rename(name, tmpname);
	}
#endif
//	_unlink(basename);
//	if (rename(newname, basename)!=0)
//		return 1;

	sym_change_count = 0;

	return 0;
}
