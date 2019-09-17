#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "audiolib.h"

/* AIF */
#define DMABUS_CR		0x48d04000
#define DMABUS_TCM		0x48d06000
#define DMABUS_TCM_SIZE	512
#define SAI1			0x48d08C00
#define SAI2			0x48d08C04
#define SAI3			0x48d08c08
#define SAI4			0x48d08C0C
#define SRC0			0x48d08000
#define SRC1			0x48d08004
#define SRC2			0x48d08008
#define SRC3			0x48d0800c
#define SRC4			0x48d08010
#define SRC5			0x48d08014
#define LPF			0x48d08400
#define AIMUX			0x48d08800
#define AOMUX			0x48d08804
#define ADCAUX			0x48d10000
/* AUSS */
#define ADCMIC			0x48060004
#define DAC_CR			0x48060010
#define SSY_CR			0x48060100
#define MUX_CR			0x48061000

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
typedef int (*line_parser)(char *line, void *prec, int *psz);

static int help(char *name)
{
	fprintf(stderr, "Usage: %s -s|-t filein fileout\n", name);
	fprintf(stderr, "\noptions:\n");
	fprintf(stderr, "  -h,--help       		this help\n");
	fprintf(stderr, "  -s,--source       	init audio source\n");
	fprintf(stderr, "  -t,--trace          trace of audio API\n");
	fprintf(stderr, "  -m,--modules             modules infos\n");
	return 1;
}

int make_early(char **argv, line_parser parse, void *prec, unsigned int signature)
{
	char *line = NULL;
	size_t size;
	int ret;
	api_header h;
	FILE *fin, *fout;
	char *name;
	int recsz;

	fin = fopen(argv[0], "r");
	if (fin == NULL) {
		perror(argv[0]);
		return -1;
	}

	fout = stdout;
	name = argv[0];
	h.id = signature;
	h.hsize = (offsetof(api_header, name) + strnlen(name, API_NAME_MAX) + 3)&~3;
	strncpy(h.name, name, API_NAME_MAX);
	/* First dry run to calculate the size */
	h.size = 0;
	while(-1 != getline(&line, &size, fin)) {
		if (parse(line, prec, &recsz))
			h.size += recsz;
	}
	ret = fwrite(&h, h.hsize, 1, fout);
	fprintf(stderr, "%d, [%s]: %d\n", h.hsize, h.name, h.size);

	rewind(fin);
	while(-1 != getline(&line, &size, fin)) {
		if (parse(line, prec, &recsz)){
			ret = fwrite(prec, recsz, 1, fout);
			if (ret <= 0) {
				perror("fwrite");
				return -1;
			}
		}
	}
	free(line);
	fclose(fin);
	return 0;
}

unsigned int tok2addr(char *p)
{
	if (!strcmp(p, "DAC_CR"))
		return DAC_CR;
	else if (!strcmp(p, "SSY_CR"))
		return SSY_CR;
	else if (!strcmp(p, "MUX_CR"))
		return MUX_CR;
	else if (!strcmp(p, "DMABUS_CR"))
		return DMABUS_CR;
	else if (!strcmp(p, "DMABUS_TCM"))
		return DMABUS_TCM;
	else if (!strcmp(p, "SAI1"))
		return SAI1;
	else if (!strcmp(p, "SAI2"))
		return SAI2;
	else if (!strcmp(p, "SAI3"))
		return SAI3;
	else if (!strcmp(p, "SAI4"))
		return SAI4;
	else if (!strcmp(p, "AIMUX"))
		return AIMUX;
	else if (!strcmp(p, "AOMUX"))
		return AOMUX;
	else if (!strcmp(p, "LPF"))
		return LPF;
	else if (!strcmp(p, "SRC0"))
		return SRC0;
	else if (!strcmp(p, "SRC1"))
		return SRC1;
	else if (!strcmp(p, "SRC2"))
		return SRC2;
	else if (!strcmp(p, "SRC3"))
		return SRC3;
	else if (!strcmp(p, "SRC4"))
		return SRC4;
	else if (!strcmp(p, "SRC5"))
		return SRC5;
	else if (!strcmp(p, "ADCAUX"))
		return ADCAUX;
	else if (!strcmp(p, "ADCMIC_CR"))
		return ADCMIC;
	else
		return 0;
}

int parse_source(char *line, void *prec, int *psz)
{
	char *p;
	static unsigned int base = 0;
	setreg *s = (setreg *)prec;

	p = strtok(line, " =:");
	if (!p) return 0;
	s->addr = tok2addr(p);
	if (s->addr == DMABUS_CR) {
		return 0;
	} else if (s->addr == DAC_CR) {
		return 0;
	} else if (s->addr == DMABUS_TCM) {
		base = DMABUS_TCM;
		return 0;
	} else if (s->addr) {
		base = 0;
	} else if (base) {
		s->addr = base + strtoul(p, NULL, 16);
	} else {
		fprintf(stderr, "%s unknown\n", p);
		return 0;
	}
	p = strtok(NULL, " \n");
	if (!p) return 0;
	s->val = strtoul(p, NULL, 16);
	*psz = sizeof(setreg);
	return 1;
}

int parse_trace(char *line, void *prec, int *psz)
{
	char *p;
	setreg *s = (setreg *)prec;

	p = strtok(line, " :");
	if (!p) return 0;
	s->addr = strtoul(p, NULL, 16);
	p = strtok(NULL, " \n");
	if (!p) return 0;
	s->val = strtoul(p, NULL, 16);
	*psz = sizeof(setreg);
	return 1;
}

int parse_modules(char *line, void *prec, int *psz)
{
	char *p;
	modinfo *m = (modinfo *)prec;

	p = strtok(line, " :");
	if (!p) return 0;
	m->size = (offsetof(modinfo, name) + sizeof(m->type) + strnlen(p, API_NAME_MAX) + 3)&~3;
	strncpy(m->name, p, API_NAME_MAX);
	p = strtok(NULL, " \n");
	if (!p) return 0;
	m->xaddr = strtoul(p, NULL, 16);
	p = strtok(NULL, " \n");
	if (!p) return 0;
	m->yaddr = strtoul(p, NULL, 16);
	p = strtok(NULL, " \n");
	if (!p) return 0;
	m->type = strtoul(p, NULL, 16);
	*psz = m->size;
	return 1;
}

int main(int argc, char* argv[])
{
	int choice = 0;
	line_parser parser = NULL;
	unsigned int signature;
	setreg s;
	modinfo m;
	void *prec;

	static const struct option long_option[] =
	{
		{"help", 0, NULL, 'h'},
		{"source", 0, NULL, 's'},
		{"trace", 0, NULL, 't'},
		{"modules", 0, NULL, 'm'},
		{NULL, 0, NULL, 0},
	};
	while (1) {
		int c;

		if ((c = getopt_long(argc, argv, "hstm", long_option, NULL)) < 0)
			break;
		switch (c) {
		case 's':
			signature = API_SIGNATURE;
			parser = &parse_source;
			prec = &s;
			break;
		case 't':
			signature = API_SIGNATURE;
			parser = &parse_trace;
			prec = &s;
			break;
		case 'm':
			signature = MOD_SIGNATURE;
			parser = &parse_modules;
			prec = &m;
			break;
		default:
			return help(argv[0]);
		}
		if (choice) 
			return help(argv[0]);
		choice = c;
	}

	if (argc != optind + 1)
		return help(argv[0]);

	argv += optind;

	make_early(argv, parser, prec, signature);

	return 1;
}
