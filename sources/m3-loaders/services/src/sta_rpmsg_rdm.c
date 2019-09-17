/**
 * @file sta_rpmsg_rdm.c
 * @brief Provides Remote Device Manager (RPMSG-based) service
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: ADG-MID team
 */

/* Standard includes. */
#include <stdio.h>
#include <string.h>
#include <errno.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Platform includes */
#include "MessageQCopy.h"
#include "NameMap.h"
#include "trace.h"
#include "sta_common.h"
#include "sta_rpmsg.h"
#include "sta_rpmsg_rdm.h"

/* Start of common definitions */

#define RDM_MAX_DEV_NAME_SZ		25
#define RDM_RPMSG_BUF_SIZE		200

enum rpmsg_rdm_types {
	RDM_CONNECT = 0,
	RDM_REQUEST_DEV,
	RDM_RELEASE_DEV,
	RDM_ERROR,
};

struct rpmsg_rdm_hdr {
	uint32_t type;
	uint32_t len;
	int data[0];
} __packed;

enum rpmsg_rdm_cn_status {
	RDM_CN_REQ = 0,
	RDM_CN_ACK,
	RDM_CN_NACK,
};

struct rpmsg_rdm_cn {
	uint32_t status;
	int data[0];
} __packed;

struct rpmsg_rdm_release {
	char dev_name[RDM_MAX_DEV_NAME_SZ];
} __packed;

/* End of common definitions */

enum rpmsg_rdm_service_state {
	RDM_STATE_INIT = 0,
	RDM_STATE_WAIT_CN,
	RDM_STATE_ONLINE,
	RDM_STATE_DEV_RELEASE_PENDING,
	RDM_STATE_DEV_REQUEST_PENDING,
	RDM_STATE_ERROR,
};

struct rpmsg_rdm {
	MessageQCopy_Handle mqcpy;
	uint32_t myept;
	uint32_t remoteept;
	uint32_t state;
};

static struct rpmsg_rdm rpmsg_rdm;
static char msgBuf[RDM_RPMSG_BUF_SIZE];

/**
 * @brief Remote Device Manager - Send message operation
 * @param	buf: pointer to data to receive
 * @param	len : max length to receive, and actual length received
 * @return	0 on success, negative value on failure
 */
static int rdm_send(struct rpmsg_rdm *rdm, char *buf, uint16_t len)
{
	int ret;

	if ((!rdm) || (len > RDM_RPMSG_BUF_SIZE))
		return -EINVAL;

	ret = MessageQCopy_send(rdm->remoteept, rdm->myept, (void *)buf, len,
				portMAX_DELAY);

	return (ret == MessageQCopy_S_SUCCESS) ? 0 : -EIO;
}

/**
 * @brief Remote Device Manager - Message receive operation
 * @param	rdm : RDM private struct
 * @param	buf : pointer to data to receive
 * @param	len : max length to receive, and actual length received
 * @return	0 on success, negative value on failure
 */
static int rdm_recv(struct rpmsg_rdm *rdm, char *buf, uint16_t *len)
{
	int ret;
	uint32_t rept;

	if (!rdm)
		return -EINVAL;

	ret = MessageQCopy_recv(rdm->mqcpy, (void *)buf, len, &rept,
				portMAX_DELAY);

	/* Save host's endpoint at 1st rx message */
	if (rdm->remoteept == MessageQCopy_ASSIGN_ANY)
		rdm->remoteept = rept;

	/* Multiple clients not supported */
	if (rept != rdm->remoteept) {
		TRACE_ERR("%s: drop msg from unknown Ept (%u)\n",  __func__,
			  rept);
		return -ENXIO;
	}

	return (ret == MessageQCopy_S_SUCCESS) ? 0 : -EIO;
}

/**
 * @brief initialize remoteproc communication for "rpmg-rdm" service
 * @param	rdm : RDM private struct
 * @return	0 on success, negative value on failure
 */
int rdm_init(struct rpmsg_rdm *rdm)
{
	uint32_t requestEndpoint = MessageQCopy_ASSIGN_ANY;
	uint32_t myendpoint = 0;

	if (rdm->state != RDM_STATE_INIT)
		return -EINVAL;

	if (RPMSG_Init() != 0)
		return -EIO;

	rdm->mqcpy = MessageQCopy_create(requestEndpoint, &myendpoint);
	if (!rdm->mqcpy)
		return -ENOMEM;

	NameMap_register("rpmsg-rdm", myendpoint);

	rdm->myept = myendpoint;
	rdm->remoteept = MessageQCopy_ASSIGN_ANY;

	return 0;
}

/**
 * @brief release a device for the host
 * @param	name: name of the device that is released
 * @return	0 on success, negative value on failure
 */
int rdm_relase_device(char *name)
{
	struct rpmsg_rdm_hdr *hdr;
	struct rpmsg_rdm_release *rdm_rel;
	char *buf;
	uint16_t len;
	int ret = 0;

	if (rpmsg_rdm.state != RDM_STATE_ONLINE)
		return -EAGAIN;

	/* TODO compare device name to a list
	 * of effective available device on Host side
	 */

	len = MIN(strlen(name), RDM_MAX_DEV_NAME_SZ);

	buf = pvPortMalloc(len + sizeof(*hdr));
	if (!buf)
		return -ENOMEM;

	hdr = (struct rpmsg_rdm_hdr *)buf;
	rdm_rel = (struct rpmsg_rdm_release *)hdr->data;

	strncpy(rdm_rel->dev_name, name, len);

	hdr->type = RDM_RELEASE_DEV;
	hdr->len = len;

	ret = rdm_send(&rpmsg_rdm, buf, len + sizeof(*hdr));

	vPortFree(buf);

	return ret;
}

/**
 * @brief Remote Device Manager task
 * @return	void
 */
void rdm_task(void *p)
{
	struct rpmsg_rdm_hdr *hdr;
	struct rpmsg_rdm_cn *rdm_cn;
	uint16_t len;
	int ret;

	rpmsg_rdm.state = RDM_STATE_INIT;

	ret = rdm_init(&rpmsg_rdm);
	if (ret) {
		TRACE_ERR("%s init failure\n", __func__);
		TRACE_INFO("returned %d\n", ret);
		goto end;
	}

	rpmsg_rdm.state = RDM_STATE_WAIT_CN;

	while (1) {
		len = RDM_RPMSG_BUF_SIZE;
		ret = rdm_recv(&rpmsg_rdm, msgBuf, &len);

		if (ret || len > RP_MSG_PAYLOAD_SIZE)
			continue;

		hdr = (struct rpmsg_rdm_hdr *)msgBuf;
		switch (hdr->type) {
		case RDM_CONNECT:
			rdm_cn = (struct rpmsg_rdm_cn *)hdr->data;

			if (rdm_cn->status == RDM_CN_REQ) {
				TRACE_INFO("%s RDM CN REQ: %s\n", __func__,
					   (char *)rdm_cn->data);

				/* Format response (ACK) */
				rdm_cn->status = RDM_CN_ACK;

				hdr->type = RDM_CONNECT;
				hdr->len = sizeof(*rdm_cn);
				len = sizeof(*hdr) + hdr->len;

				if (!rdm_send(&rpmsg_rdm, msgBuf, len))
					rpmsg_rdm.state = RDM_STATE_ONLINE;
			}
			break;
		case RDM_RELEASE_DEV:
		case RDM_REQUEST_DEV:
		default:
			TRACE_ERR("%s: Cmd not supp: %u\n", __func__,
				  hdr->type);
			break;
		}
	}

end:
	vTaskDelete(NULL);
}
