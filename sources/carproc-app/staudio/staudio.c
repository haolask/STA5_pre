/*
 * Copyright (C) ST Microelectronics 2015
 *
 * Author:	Gian Antonio Sampietro <gianantonio.sampietro@st.com>
 *		for STMicroelectronics.
 *
 * License terms:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sound/asound.h>

struct staudio {
    int fd;
	int card;
	struct snd_ctl_elem_list elist;
	struct snd_ctl_elem_value ev;
	struct snd_ctl_elem_info info;
};

enum staudio_cmd {
    GET,
    SET,
    LIST,
};

static struct staudio *staudio_open(int card)
{
	char fn[256];
	struct snd_ctl_elem_id *eid = NULL;
	static struct staudio staudio;
	int fd;

	snprintf(fn, sizeof(fn), "/dev/snd/controlC%u", card);
	fd = open(fn, O_RDWR);
	if (fd < 0){
		fprintf(stderr, "Cannot open card %d\n", card);
		return NULL;
	}

	staudio.fd = fd;
	staudio.card = card;
	memset(&staudio.elist, 0, sizeof(struct snd_ctl_elem_list));
	if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_LIST, &staudio.elist) < 0) {
		fprintf(stderr, "Cannot get elem list of card %d\n", card);
		return NULL;
	}

	eid = calloc(staudio.elist.count, sizeof(struct snd_ctl_elem_id));
	if (!eid) {
		fprintf(stderr, "Cannot allocate %d elist space for card %d\n", staudio.elist.count, card);
		return NULL;
	}

	staudio.elist.space = staudio.elist.count;
	staudio.elist.pids = eid;
	if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_LIST, &staudio.elist) < 0) {
		fprintf(stderr, "Cannot get elem list of card %d\n", card);
		return NULL;
	}

	return &staudio;
}

static int staudio_close(struct staudio *staudio)
{
	if (!staudio)
		return 0;

	if (staudio->elist.pids)
		free(staudio->elist.pids);

	close(staudio->fd);
	return 0;
}

int staudio_get_ctl(struct staudio *staudio, char *name)
{
	int i;
	struct snd_ctl_elem_info *info = &staudio->info;
	struct snd_ctl_elem_list *elist = &staudio->elist;
	struct snd_ctl_elem_value *ev = &staudio->ev;

	if (!name)
		return -EINVAL;
	for (i = 0; i < elist->count; i++){
		if (!strcmp(name, (char *) elist->pids[i].name)){
			info->id.numid = elist->pids[i].numid;
			if (ioctl(staudio->fd, SNDRV_CTL_IOCTL_ELEM_INFO, info) < 0){
				fprintf(stderr, "Cannot get info of the control %s\n", name);
				return -EINVAL;
			}
			break;
		}
	}

	if (i == elist->count) {
		fprintf(stderr, "Cannot find the control %s in card %d\n", name, staudio->card);
		return -EINVAL;
	}
	ev->id.numid = info->id.numid;
	return 0;
}

static int staudio_ctl_elem_list(struct staudio *staudio)
{
	int i;
	struct snd_ctl_elem_list *elist = &staudio->elist;

	for (i = 0; i < elist->count; i++)
		    printf("%s\n",(char *) elist->pids[i].name);
}

static int staudio_ctl_elem_write(struct staudio *staudio)
{
	struct snd_ctl_elem_value *ev = &staudio->ev;
	return ioctl(staudio->fd, SNDRV_CTL_IOCTL_ELEM_WRITE, ev);
}

static int staudio_ctl_elem_read(struct staudio *staudio)
{
	struct snd_ctl_elem_value *ev = &staudio->ev;
	return ioctl(staudio->fd, SNDRV_CTL_IOCTL_ELEM_READ, ev);
}

static int staudio_get_enum_index(struct staudio *staudio,
								  const char *string)
{
	struct snd_ctl_elem_info *info = &staudio->info;
    unsigned int i, num_enums;

    num_enums = info->value.enumerated.items;
    for (i = 0; i < num_enums; i++) {
		info->value.enumerated.item = i;
		if (ioctl(staudio->fd, SNDRV_CTL_IOCTL_ELEM_INFO, info) < 0){
			return -EINVAL;
		}
        if (!strcmp(string, info->value.enumerated.name))
            return i;
    }

    return -EINVAL;
}

static int staudio_ascii_value_parse(struct staudio *staudio,
			      const char *strval)
{
	struct snd_ctl_elem_info *info = &staudio->info;
	struct snd_ctl_elem_value *ev = &staudio->ev;
	char *ptr = (char *)strval;
	unsigned int idx;
	long val;

	switch (info->type) {
	case SNDRV_CTL_ELEM_TYPE_BYTES:
		for (idx = 0; idx < 511 && ptr && *ptr; idx++) {
			ev->value.bytes.data[idx] = ptr[idx];
		}
		ev->value.bytes.data[idx] = 0;
		break;
	case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
		val = staudio_get_enum_index(staudio, ptr);
		if (val < 0)
			return val;
		ev->value.enumerated.item[0] = val;
		break;
	case SNDRV_CTL_ELEM_TYPE_INTEGER:
		for (idx = 0; idx < 128 && ptr && *ptr; idx++) {
			val = strtol(ptr, &ptr, 0);
			ev->value.integer.value[idx] = val;
			if (*ptr == ',')
				ptr++;
		}
		break;
	}

	return 0;
}

static int staudio_value_print(struct staudio *staudio)
{
	struct snd_ctl_elem_info *info = &staudio->info;
	struct snd_ctl_elem_value *ev = &staudio->ev;
	int idx;
	if (ioctl(staudio->fd, SNDRV_CTL_IOCTL_ELEM_INFO, info) < 0){
		fprintf(stderr, "Cannot get info of the control %s\n", info->id.name);
		return -EINVAL;
	}

	for (idx = 0; idx < info->count; idx++) {
		if (idx > 0)
			printf(",");
		switch (info->type) {
		case SNDRV_CTL_ELEM_TYPE_INTEGER:
			printf("%lx", ev->value.integer.value[idx]);
			break;
		case SNDRV_CTL_ELEM_TYPE_BOOLEAN:
			printf("%s", !!ev->value.integer.value[idx] ? "on" : "off");
			break;
		case SNDRV_CTL_ELEM_TYPE_ENUMERATED:
			info->value.enumerated.item =  ev->value.enumerated.item[idx];
			if (ioctl(staudio->fd, SNDRV_CTL_IOCTL_ELEM_INFO, info) < 0){
				return -EINVAL;
			}
			printf("%s", info->value.enumerated.name);
			break;
		default:
			printf("?");
			break;
		}
	}
	printf("\n");

	return 0;
}

static int help(char *name)
{
	printf("Usage: %s <options> [command]\n", name);
	printf("\noptions:\n");
	printf("  -h,--help       		this help\n");
	printf("  -c,--card N			select the card\n");
	printf("\ncommands:\n");
	printf(" simple\n");
	printf(" get id\n");
	printf(" set id val1,val2,..\n");
	return 0;
}

int main(int argc, char *argv[])
{
	int err, ret;
	enum staudio_cmd cmd;
	struct staudio *handle;
/* default parameters */
	char *ctl = NULL;
	int card = 3;


	static const struct option long_option[] =
	{
		{"help", 0, NULL, 'h'},
		{"card", 1, NULL, 'c'},
		{"name", 1, NULL, 'n'},
		{"iface", 1, NULL, 'i'},
		{NULL, 0, NULL, 0},
	};

	while (1) {
		int c;

		if ((c = getopt_long(argc, argv, "hc:n:i:", long_option, NULL)) < 0)
			break;
		switch (c) {
		case 'h':
			return help(argv[0]);
		case 'c':
			card = atoi(optarg);
			if (card <= 0 || card >= 32){
				fprintf(stderr, "Invalid card number.\n");
				return 1;
			}
			break;
		case 'n':
			ctl = optarg;
			break;
		case 'i':
			break;
		default:
			help(argv[0]);
			return 1;
		}
	}

	if (argc < optind + 1) {
		fprintf(stderr, "Missing command get/set\n");
		return -EINVAL;
	}

	if (!strcmp(argv[optind], "set")) {
		cmd = SET;
	} else if (!strcmp(argv[optind], "get")) {
		cmd = GET;
	} else if (!strcmp(argv[optind], "simple")) {
		cmd = LIST;
	} else {
		fprintf(stderr, "%s: Unknown command '%s'...\n", argv[0], argv[optind]);
		return -EINVAL;
	}
	optind++;

	/* if ctl name not specified with the -n option, it must be the next argv */
	if (cmd != LIST && !ctl && argc >= optind + 1) {
		ctl = argv[optind];
		optind++;
	}

	handle = staudio_open(card);
	if (!handle)
		return -EINVAL;

	if (cmd == LIST)
		return staudio_ctl_elem_list(handle);

	if (staudio_get_ctl(handle, ctl))
		return -EINVAL;

	if (argc > optind) {
		err = staudio_ascii_value_parse(handle, argv[optind]);
		if (err < 0) {
			fprintf(stderr, "Control %s parse error: %s\n", ctl, strerror(err));
			return err;
		}
	} else if (cmd == SET) {
		fprintf(stderr, "Missing values\n");
		return -EINVAL;
	}

	if (cmd == SET) {
		ret = staudio_ctl_elem_write(handle);
		if (ret < 0){
			fprintf(stderr, "Control %s element write error: %s\n", ctl, strerror(errno));
			return ret;
		}
	} else if (cmd == GET) {
		ret = staudio_ctl_elem_read(handle);
		if (ret < 0){
			fprintf(stderr, "Control %s element read error: %s\n", ctl, strerror(errno));
			return ret;
		}
		/* after the read of Tune controls, the number of returned values is in info->count */
		staudio_value_print(handle);
	}

	staudio_close(handle);
	return 0;
}
