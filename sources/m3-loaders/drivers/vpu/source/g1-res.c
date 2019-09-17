#include "FreeRTOS.h"
#include "g1-res.h"
#include "dwl.h"
#include "sta_rpmsg_mm.h"
#include "string.h"
#include "task.h"
#include "semphr.h"

static void *g1_ept;
static uint8_t g1_ip_status = RPMSG_MM_SHARED_RES_UNLOCKED;
static void *g1_ctx;
static int ref_count;
static SemaphoreHandle_t g1_res_sem;
static int g1_res_initialized;

int G1ResCallback(struct s_rpmsg_mm_data *data, void *priv)
{
	if (!data)
		return -1;

	switch (RPMSG_MM_GET_INFO(data->info)) {
	case RPMSG_MM_SHARED_RES_STATUS_REQ:
		data->len = 1;
		data->info = RPMSG_MM_SHARED_RES_STATUS_ACK;
		data->data[0] = (g1_ip_status == RPMSG_MM_SHARED_RES_LOCKED ?
				 RPMSG_MM_HW_RES_G1 : 0);
		TRACE_INFO("return jpeg ip status = %d\n", data->data[0]);
		break;
	default:
		break;
	}
	return 0;
}

int G1ResRpmsgRegistration(void)
{
	if (g1_ept)
		return 0;

	g1_ept = rpmsg_mm_register_local_endpoint(RPSMG_MM_EPT_CORTEXM_G1,
						  G1ResCallback,
						  NULL);

	if (!g1_ept) {
		TRACE_INFO("Unable to book G1 hardware resource !!!\n");
		return -1;
	}
	return 0;
}

void G1ResInit(void)
{
	taskENTER_CRITICAL();
	if (g1_res_initialized) {
		taskEXIT_CRITICAL();
		return;
	}
	vSemaphoreCreateBinary(g1_res_sem);
	if (g1_res_sem)
		g1_res_initialized = true;
	else
		TRACE_ERR("G1ResInit: cannot create Semaphore\n");
	taskEXIT_CRITICAL();
}

void *G1ResBookResource(void)
{
	int ret = 0;

	if (!g1_res_initialized) {
		TRACE_INFO("g1_res NOT initialized <%s\n", __func__);
		return NULL;
	}

	xSemaphoreTake(g1_res_sem, portMAX_DELAY);

	if (!g1_ctx) {
		G1ResRpmsgRegistration();
		ret = rpmsg_mm_lock_resource(g1_ept,
					     RPMSG_MM_SHARED_RES_LOCKED,
					     RPMSG_MM_HW_RES_G1,
					     RPSMG_MM_EPT_CORTEXA_G1);
		if (ret >= 0)
			g1_ip_status = RPMSG_MM_SHARED_RES_LOCKED;

		g1_ctx = g1_hw_init();
	}

	ref_count++;
	TRACE_INFO("G1ResBookResource count=%d\n", ref_count);
	xSemaphoreGive(g1_res_sem);
	return g1_ctx;
}

int G1ResFreeResource(void *ctx)
{
	int ret = 0;

	if (!g1_res_initialized) {
		TRACE_INFO("g1_res NOT initialized <%s\n", __func__);
		return -1;
	}

	xSemaphoreTake(g1_res_sem, portMAX_DELAY);

	if (g1_ip_status == RPMSG_MM_SHARED_RES_UNLOCKED) {
		TRACE_INFO("<%s\n", __func__);
		xSemaphoreGive(g1_res_sem);
		return ret;
	}

	ref_count--;
	TRACE_INFO("G1ResFreeResource count=%d\n", ref_count);

	if (ref_count) {
		xSemaphoreGive(g1_res_sem);
		return 0;
	}

	G1ResRpmsgRegistration();

	if (g1_ctx) {
		g1_hw_release(g1_ctx);
		g1_ctx = NULL;
	}

	TRACE_INFO("%s Unlock G1 resources\n", __func__);
	ret = rpmsg_mm_lock_resource(g1_ept,
				     RPMSG_MM_SHARED_RES_UNLOCKED,
				     RPMSG_MM_HW_RES_G1,
				     RPSMG_MM_EPT_CORTEXA_G1);
	if (ret >= 0)
		g1_ip_status = RPMSG_MM_SHARED_RES_UNLOCKED;

	xSemaphoreGive(g1_res_sem);
	return ret;
}
