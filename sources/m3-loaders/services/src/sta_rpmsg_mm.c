/**
 * @file sta_rpmsg_mm.c
 * @brief RPMsg communication between CortexM and CortexA for multimedia tasks
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "trace.h"

#include "utils.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "sta_rpmsg_mm.h"

#define DEFAULT_LOCAL_EPT   0xFFFF

/**
 * @struct hw_resource_info
 * @brief Used to described a registered endpoint
 *
 * RPMSG_MM users have to register themselves. At this registration, an
 * hw_resource_info element is created to be able to communicate with it
 * (using callback pCB).
 */
struct hw_resource_info {
	char name[MAX_NAME_LENGTH]; /*! Name of the resource */
	uint32_t ept; /*!< Endpoint identifier from RPMSG framework */
	t_pfct_rpmsg_mm_cb pCB;
	/*!< Callback to address this resource on message reception */
	void *priv; /*!< private data transferred with the callback */
	uint16_t owner; /*!< refer to ::rpmsg_mm_owner */
	uint16_t instance_type; /*!< refer to #rpmsg_mm_msg_type */
	bool is_valid; /*!< false : resource isn't valid */
};

/**
 * @struct s_rpmsg_mm_service_user
 * @brief Used to store a callback to one or several services
 *
 * This callback will be called by rpmsg_mm when a service is triggered.
 */
struct s_rpmsg_mm_service_user {
	t_pfct_rpmsg_mm_service_cb pCB; /*!< callback */
	void *priv; /*!< private data transferred in the callback */
	uint8_t services; /*!< services, user is interested in */
};

#define MAX_NB_RESOURCES 10

/**
 * @struct rpmsg_mm_context
 * @brief Used to store rpmsg_mm context
 */
struct rpmsg_mm_context {
	SemaphoreHandle_t xSemResourceLock;
	/*!< Semaphore used to protext acces to resources_table */
	struct hw_resource_info resources_table[MAX_NB_RESOURCES];
	/*!< table of registered resources (local and remote) */
	SemaphoreHandle_t xSemServiceLock;
	/*!< Semaphore used to protext acces to services_users table */
	struct s_rpmsg_mm_service_user services_users[MAX_NB_RESOURCES];
	/*!< table of registered users of services*/
	uint16_t latestUserId;
	/*!< Last free index in the service_users table */
	void *MMhandle;
	/*!< Handle on auto-registered "Mx_MM" instance */
	uint32_t local_ept;
	/*!< RPMSG endpoint identifier*/
};

/*! @internal */

#define LOCK_RESOURCES() \
{\
	if (!rpmsg_mm_ctx.xSemResourceLock)\
		vSemaphoreCreateBinary(rpmsg_mm_ctx.xSemResourceLock);\
	xSemaphoreTake(rpmsg_mm_ctx.xSemResourceLock, portMAX_DELAY);\
}

#define UNLOCK_RESOURCES() \
{\
	xSemaphoreGive(rpmsg_mm_ctx.xSemResourceLock);\
}

#define LOCK_SERVICES() \
{\
	if (!rpmsg_mm_ctx.xSemServiceLock)\
		vSemaphoreCreateBinary(rpmsg_mm_ctx.xSemServiceLock);\
	xSemaphoreTake(rpmsg_mm_ctx.xSemServiceLock, portMAX_DELAY);\
}

#define UNLOCK_SERVICES() \
{\
	xSemaphoreGive(rpmsg_mm_ctx.xSemServiceLock);\
}

char *info2str[] = {
	"UNUSED",
	"REGISTRATION_REQ",
	"REGISTRATION_ACK",
	"UNREGISTRATION_REQ",
	"UNREGISTRATION_ACK",
	"SHARED_RES_STATUS_REQ",
	"SHARED_RES_STATUS_ACK",
	"SHARED_RES_LOCKED",
	"SHARED_RES_LOCKED_ACK",
	"SHARED_RES_UNLOCKED",
	"COMM_UNAVAILABLE",
	"COMM_AVAILABLE",
	"USER_SERVICE",
	"REARGEAR_STATUS_REQ",
	"REARGEAR_STATUS_ACK",
	"PRIVATE_MESSAGE"
};

struct rpmsg_ack_info {
	uint32_t status;
	SemaphoreHandle_t sem;
};

/* Structure MUST be inline with the one used on remote side*/
struct rpmsg_mm_payload {
	char emitter[MAX_NAME_LENGTH];
	/*!< String used to described the message emitter */
	char receiver[MAX_NAME_LENGTH];
	/*!< String used to described the message receiver */
	void *reserved;
	/*!< Reserved data used by rpmsg_mm */
	struct s_rpmsg_mm_data user_data;
	/* Nothing after...*/
};

static struct rpmsg_mm_context rpmsg_mm_ctx = {0};

struct hw_resource_info *rpmsg_mm_find_res_by_name(char *name)
{
	bool found = false;
	uint16_t idres = 0;
	struct hw_resource_info *res = NULL;

	LOCK_RESOURCES();
	for (idres = 0; idres < MAX_NB_RESOURCES; idres++) {
		res = &rpmsg_mm_ctx.resources_table[idres];
		if (!res->is_valid)
			continue;
		if (strcmp(res->name, name) == 0) {
			found = true;
			break;
		}
	}
	UNLOCK_RESOURCES();
	if (found)
		return res;
	return NULL;
}

struct hw_resource_info *rpmsg_mm_book_free_res(void)
{
	bool found = false;
	uint16_t idres = 0;
	struct hw_resource_info *res = NULL;

	LOCK_RESOURCES();
	for (idres = 0; idres < MAX_NB_RESOURCES; idres++) {
		res = &rpmsg_mm_ctx.resources_table[idres];
		if (res->is_valid)
			continue;
		/* Book it */
		found = true;
		res->is_valid = true;
		break;
	}
	UNLOCK_RESOURCES();
	if (found)
		return res;
	return NULL;
}

void rpmsg_mm_release_res_by_name(char *name)
{
	uint16_t idres = 0;
	struct hw_resource_info *res = NULL;

	LOCK_RESOURCES();
	for (idres = 0; idres < MAX_NB_RESOURCES; idres++) {
		res = &rpmsg_mm_ctx.resources_table[idres];
		if (!res->is_valid)
			continue;
		if (strcmp(res->name, name) == 0) {
			memset(res, 0, sizeof(struct hw_resource_info));
			res->is_valid = false;
			break;
		}
	}
	UNLOCK_RESOURCES();
}

void rpmsg_mm_release_res(struct hw_resource_info *res)
{
	if (!res)
		return;
	LOCK_RESOURCES();
	memset(res, 0, sizeof(struct hw_resource_info));
	res->is_valid = false;
	UNLOCK_RESOURCES();
}

/**
 * @fn int rpmsg_mm_lock_resource_ack(struct s_rpmsg_mm_data *pdata)
 * @brief internal service called when ::RPMSG_MM_SHARED_RES_LOCKED_ACK message
 * is received. It's used to unblock ::rpmsg_mm_lock_resource function
 * @param pdata message
 * @return -1 in case of error, else return 0
 *
 * This function gets back semaphore handle from data and unblocks the
 * ::rpmsg_mm_lock_resource function to free the caller.
 */
int rpmsg_mm_lock_resource_ack(struct rpmsg_mm_payload *payload_data)
{
	struct rpmsg_ack_info *ack;

	ack = (struct rpmsg_ack_info *)payload_data->reserved;
	xSemaphoreGive(ack->sem);
	return 0;
}

/**
 * @fn int rpmsg_mm_parse_services(struct s_rpmsg_mm_data *data)
 * @brief internal service called when ::RPMSG_MM_USER_SERVICE message is
 * received. It's used to inform local instances about service info.
 * @param data message (including the service name)
 * @return -1 in case of error, else return 0
 *
 * This function loops on registered service callbacks, then calls successively
 * every callbacks.
 */
int rpmsg_mm_parse_services(struct s_rpmsg_mm_data *data)
{
	uint8_t idres = 0, max_id;
	uint8_t services = 0;
	uint8_t mix = 0;
	uint8_t user_len = 0;
	void *user_data = NULL;

	/* TODO: as service is a bitfield (several services can be indicated in
	 *       one byte), how to handle multiple services with data
	 */

	if (!data)
		return -1;

	if (RPMSG_MM_GET_INFO(data->info) != RPMSG_MM_USER_SERVICE ||
	    data->len <= 0)
		return 0;

	services = data->data[0];
	TRACE_INFO("0x%02x service updated, look for registered users\n",
		   services);

	/* remove 1 because first data contains the service itself */
	user_len = data->len - 1;
	if (user_len > 0)
		user_data = &data->data[1];

	LOCK_SERVICES();
	max_id = rpmsg_mm_ctx.latestUserId;
	for (idres = 0; idres <= max_id; idres++) {
		mix = (rpmsg_mm_ctx.services_users[idres].services & services);
		if (mix) {
			TRACE_INFO("registered user found\n");
			t_pfct_rpmsg_mm_service_cb pCB;

			pCB = rpmsg_mm_ctx.services_users[idres].pCB;
			if (pCB)
				pCB(mix,
				    rpmsg_mm_ctx.services_users[idres].priv,
				    user_len,
				    user_data);
		}
	}
	UNLOCK_SERVICES();
	return 0;
}

/**
 * @fn uint8_t rpmsg_mm_transfer_msg_to_local_ept(
 *		struct rpmsg_mm_payload *payload_data)
 * @brief internal service used to forward message to targeted endpoint
 * @param data message (including the receiver name)
 * @return -1 in case of error, else return 0
 *
 * Receiver name is used to retrieve the instance. Once found, rpmsg_mm called
 * the associated callback to transfer the data.
 */
uint8_t rpmsg_mm_transfer_msg_to_local_ept(
			struct rpmsg_mm_payload *payload_data)
{
	struct hw_resource_info *pData = NULL;
	struct s_rpmsg_mm_data *data = &payload_data->user_data;

	data->info = RPMSG_MM_RESET_ACK_INFO(data->info);
	/* Look for if endpoint is known as a local endoint */
	pData = rpmsg_mm_find_res_by_name(payload_data->receiver);
	if (!pData) {
		TRACE_INFO("Unknown %s endpoint\n", payload_data->receiver);
		return 0;
	}
	if (!pData->pCB) {
		TRACE_INFO("No callback associated to the receiver %s\n",
			   payload_data->receiver);
		return 0;
	}
	return pData->pCB(data, pData->priv);
}

static int rpmsg_mm_send_to_remote(struct hw_resource_info *handle,
				   struct hw_resource_info *receiver,
				   struct s_rpmsg_mm_data *user_data,
				   void *reserved,
				   bool ack_requested)
{
	struct hw_resource_info *mm;
	char buffer[MAX_PAYLOAD_SIZE];
	struct rpmsg_mm_payload *payload_data = NULL;
	struct s_rpmsg_mm_data *payload_user = NULL;
	struct rpmsg_ack_info ack;
	SemaphoreHandle_t xSemWaitAnswer;
	int paysize = 0;

	mm = (struct hw_resource_info *)handle;
	if (!mm)
		return -1;

	if (!user_data)
		return -1;

	memset(buffer, 0, sizeof(buffer));
	paysize = sizeof(struct rpmsg_mm_payload) + user_data->len;
	if (paysize > MAX_PAYLOAD_SIZE) {
		TRACE_INFO("Invalid payload size\n");
		return -1;
	}

	payload_data = (struct rpmsg_mm_payload *)buffer;
	payload_user = &payload_data->user_data;

	strncpy(payload_data->emitter, mm->name, MAX_NAME_LENGTH);
	strncpy(payload_data->receiver, receiver->name, MAX_NAME_LENGTH);
	payload_user->info = RPMSG_MM_RESET_ACK_INFO(user_data->info);

	if (user_data->len) {
		/* User wants to transfer additional data */
		memcpy(&payload_user->data[0],
		       &user_data->data[0],
		       user_data->len);
		payload_user->len = user_data->len;
	}

	if (ack_requested) {
		ack.status = 0;
		xSemWaitAnswer = xSemaphoreCreateBinary();
		ack.sem = xSemWaitAnswer;
		payload_user->info = RPMSG_MM_SET_ACK_INFO(payload_user->info);
		payload_data->reserved = (void *)&ack;
	} else {
		payload_data->reserved = reserved;
	}

#if defined RPMSG_MM_DEBUG_MSG
	{
		unsigned int i = 0;

		TRACE_INFO("***** Message sent by CortexM ****\n");
		TRACE_INFO("* Sent From %s - Receiver : %s\n",
			   payload_data->emitter,
			   payload_data->receiver);
		TRACE_INFO("* Info : %s\n",
			   info2str[
				RPMSG_MM_RESET_ACK_INFO(payload_user->info)]);
		TRACE_INFO("* Ack requested : %s\n",
			   (RPMSG_MM_IS_ACK_REQUESTED(payload_user->info) ?
			   "Yes" : "No"));
		TRACE_INFO("* Reserved : 0x%x\n", payload_data->reserved);
		TRACE_INFO("* Length : %d\n", user_data->len);
		i = 0;
		while (i < payload_user->len) {
			TRACE_INFO("* data [%d] : 0x%02x\n", i,
				   payload_user->data[i]);
			i++;
		}
	}
#endif

	MessageQCopy_send(receiver->ept, rpmsg_mm_ctx.local_ept,
			  (void *)payload_data, paysize, portMAX_DELAY);

	if (!ack_requested)
		return 0;

	if (xSemaphoreTake(xSemWaitAnswer, pdMS_TO_TICKS(2000)) == pdFAIL) {
		TRACE_INFO("No answer from remote ept\n");
		ack.status = -1;
	}
	vSemaphoreDelete(xSemWaitAnswer);
	return ack.status;
}

int rpmsg_mm_send_to_receivers(void *handle, char *remote_endpoint,
			       struct s_rpmsg_mm_data *user_data,
			       void *reserved,
			       bool ack_requested)
{
	uint16_t idres = 0;
	struct hw_resource_info *hwres = NULL;
	struct hw_resource_info *emitter = (struct hw_resource_info *)handle;

	if (!user_data || !remote_endpoint || !emitter) {
		TRACE_INFO("%s (ERROR)\n", __func__);
		return -1;
	}

	/* When remote endpoint is RPSMG_MM_EPT_CORTEXA_MM, it indicates
	 * that we want to broadcast the message to every registered MM users
	 * on Ax processor side.
	 */
	if (strcmp(remote_endpoint, RPSMG_MM_EPT_CORTEXA_MM) == 0) {
		/* Broadcast is requested, we need to parse the resources_table.
		 */
		LOCK_RESOURCES();
		for (idres = 0; idres < MAX_NB_RESOURCES; idres++) {
			hwres = &rpmsg_mm_ctx.resources_table[idres];
			if (!hwres)
				continue;
			if (!hwres->is_valid)
				continue;
			if (hwres->instance_type == RPMSG_MM_APPLI) {
				rpmsg_mm_send_to_remote(emitter, hwres,
							user_data, reserved,
							ack_requested);
			}
		}
		UNLOCK_RESOURCES();
		return 0;
	}

	/* Pair to Pair message exchange */
	hwres = rpmsg_mm_find_res_by_name(remote_endpoint);
	if (!hwres) {
		TRACE_INFO("%s not registered : message aborted\n",
			   remote_endpoint);
		return 0;
	}
	return rpmsg_mm_send_to_remote(emitter, hwres, user_data,
				       reserved, ack_requested);
}

/**
 * @fn uint8_t rpmsg_mm_get_status(struct s_rpmsg_mm_data *data)
 * @brief internal service called when ::RPMSG_MM_SHARED_RES_STATUS_REQ message
 * is received. It's used to retrieve usage of hardware resources.
 * @param data message (including the receiver name)
 * @return -1 in case of error, else return 0
 *
 * This function parses each local instance and forward the
 * ::RPMSG_MM_SHARED_RES_STATUS_REQ message to them to get the status of the
 * hardware resource they use.\n
 * This function concatenate the result then transfer the status to the caller.
 */
uint8_t rpmsg_mm_get_status(struct rpmsg_mm_payload *payload_data)
{
	uint32_t status = 0;
	uint8_t idres = 0;
	struct hw_resource_info *res = NULL;
	struct s_rpmsg_mm_data *user_data = NULL;
	uint32_t *pdata = NULL;

	user_data = &payload_data->user_data;

	if (!RPMSG_MM_IS_ACK_REQUESTED(user_data->info))
		return status;

	user_data->info = RPMSG_MM_RESET_ACK_INFO(user_data->info);

	LOCK_RESOURCES();
	for (idres = 0; idres < MAX_NB_RESOURCES; idres++) {
		res = &rpmsg_mm_ctx.resources_table[idres];
		if (!res->is_valid)
			continue;
		if (res->owner != RPMSG_MM_OWNER_LOCAL)
			continue;
		if (!res->pCB)
			continue;
		if (res->pCB(user_data, res->priv) >= 0) {
			if (user_data->info == RPMSG_MM_SHARED_RES_STATUS_ACK &&
			    user_data->len == 1) {
				/* Update the generic status */
				status |= (uint32_t)user_data->data[0];
				/* restore data for next resource */
				user_data->info =
					RPMSG_MM_SHARED_RES_STATUS_REQ;
				user_data->len = 0;
				user_data->data[0] = 0;
			}
		}
	}
	UNLOCK_RESOURCES();

	/* Return the status to the emitter */
	user_data->info = RPMSG_MM_SHARED_RES_STATUS_ACK;
	user_data->len = sizeof(status);
	pdata = (uint32_t *)&user_data->data[0];
	*pdata = status;
	/* receiver becomes emitter and vice-versa */
	res = rpmsg_mm_find_res_by_name(payload_data->receiver);
	return rpmsg_mm_send_to_receivers((void *)res,
					  payload_data->emitter, user_data,
					  payload_data->reserved, false);
}

int rpmsg_mm_send_reargear_status_int(int status, char *endpoint_name,
				      void *reserved)
{
	char buffer[MAX_PAYLOAD_SIZE];
	struct s_rpmsg_mm_data *user_data = (struct s_rpmsg_mm_data *)buffer;
	uint32_t res = status;
	uint32_t *pdata = (uint32_t *)&user_data->data[0];

	memset(buffer, 0, sizeof(buffer));
	/* Return the status to the emitter */
	user_data->info = RPMSG_MM_REARGEAR_STATUS_ACK;
	user_data->len = sizeof(res);
	*pdata = res;
	return rpmsg_mm_send_to_receivers(
		(void *)rpmsg_mm_find_res_by_name(RPSMG_MM_EPT_CORTEXM_MM),
		endpoint_name, user_data,
		reserved, false);
}

int rpmsg_mm_get_reargear_status(struct rpmsg_mm_payload *payload_data)
{
	int status = 0;
	void *reserved = payload_data->reserved;

	status = rpmsg_mm_transfer_msg_to_local_ept(payload_data);
	TRACE_INFO("%s status=%d\n", __func__, status);
	return rpmsg_mm_send_reargear_status_int (status,
						  payload_data->emitter,
						  reserved);
}

int rpmsg_mm_send_reargear_status(int status)
{
	return rpmsg_mm_send_reargear_status_int(status,
						 RPSMG_MM_EPT_CORTEXA_MM,
						 NULL);
}

/**
 * @fn void *rpmsg_mm_register_endpoint (char *endpoint_name, uint32_t ept,
 *				  t_pfct_rpmsg_mm_cb pCB, void *priv,
 *				  uint16_t owner)
 * @brief internal service used to register local or remote instance
 * @param endpoint_name instance name
 * @param ept endpoint identifier
 * @param pCB callback used by rpmsg_mm to inform this instance about message
 * reception
 * @param priv private data
 * @param owner identify if it's a remote or a local instance.
 * @return null in case of error or an handle to keep in mind to use other
 * services
 */

void *rpmsg_mm_register_endpoint (char *endpoint_name, uint32_t ept,
				  t_pfct_rpmsg_mm_cb pCB, void *priv,
				  uint16_t owner)
{
	struct hw_resource_info *pData = NULL;

	if (!endpoint_name)
		return NULL;

	pData = rpmsg_mm_find_res_by_name(endpoint_name);
	if (pData) {
		TRACE_INFO("%s already registered\n", endpoint_name);
		return (void *)pData;
	}

	pData = rpmsg_mm_book_free_res();
	if (!pData) {
		TRACE_INFO("No way to add a new hardware resource\n");
		return NULL;
	}

	strncpy(pData->name, endpoint_name, MAX_NAME_LENGTH);
	pData->instance_type = RPMSG_MM_DRIVER;

	if (strncmp(pData->name, RPSMG_MM_EPT_CORTEXA_MM,
		    strlen(RPSMG_MM_EPT_CORTEXA_MM)) == 0)
		pData->instance_type = RPMSG_MM_APPLI;

	pData->ept = ept;
	pData->pCB = pCB;
	pData->priv = priv;
	pData->owner = owner;
	return (void *)pData;
}

/**
 * @fn int rpmsg_mm_generic_cb(struct s_rpmsg_mm_data *data, void *priv)
 * @brief Callback associated to the "CortexM_MM" instance
 * @param data data received from remote core
 * @param priv private data
 *
 * Generic callback used to handle generic information from CortexA UserLand.\n
 */
int rpmsg_mm_generic_cb(struct s_rpmsg_mm_data *data, void *priv)
{
	rpmsg_mm_parse_services(data);
	return 0;
}

/**
 * @fn void rpmsg_mm_init(void)
 * @brief internal service used to initialize rpmsg_mm
 *
 * Actually, it just registers itself as "CortexM_MM" instance to be able
 * to handle generic messages sent from Linux to CortexM
 */
void rpmsg_mm_init(void)
{
	/* Systematically 'auto'declare a "MM" resource
	 * to manage generic requests
	 */
	rpmsg_mm_ctx.MMhandle = rpmsg_mm_register_local_endpoint(
					RPSMG_MM_EPT_CORTEXM_MM,
					rpmsg_mm_generic_cb, NULL);
	TRACE_INFO("RPMSG_MM Init done\n");
}

/**
 * @fn int rpmsg_mm_remote_registration(struct s_rpmsg_mm_data *pindata,
 *					uint32_t remote_endpoint)
 * @brief internal service used to handle ::RPMSG_MM_REGISTRATION_REQ and
 * ::RPMSG_MM_UNREGISTRATION_REQ messages
 *
 * @param pindata data received from remote core
 * @param remote_endpoint endpoint identifier used to communicate
 *
 * Function in charge of registering an new instance in case of registration or
 * deleting a known instance in case of unregistration \n
 * When done, it systematically sends back an acknowledge with a the status.
 */
int rpmsg_mm_remote_registration(struct rpmsg_mm_payload *payin,
				 uint32_t remote_endpoint)
{
	uint8_t buffer[MAX_PAYLOAD_SIZE];
	uint32_t linfo;
	bool ackreq;
	struct s_rpmsg_mm_data *poutdata = (struct s_rpmsg_mm_data *)buffer;
	struct s_rpmsg_mm_data *pindata = &payin->user_data;
	struct hw_resource_info *hwres = NULL;
	struct hw_resource_info *hwres_to_release = NULL;
	uint32_t status = 1;
	uint32_t *pdata = NULL;

	memset(buffer, 0, sizeof(buffer));
	linfo = RPMSG_MM_GET_INFO(pindata->info);
	ackreq = RPMSG_MM_IS_ACK_REQUESTED(pindata->info);

	if (linfo == RPMSG_MM_REGISTRATION_REQ) {
		hwres = (struct hw_resource_info *)rpmsg_mm_register_endpoint(
							payin->emitter,
							remote_endpoint,
							NULL, NULL,
							RPMSG_MM_OWNER_REMOTE);
		poutdata->info = RPMSG_MM_REGISTRATION_ACK;
	} else if (linfo == RPMSG_MM_UNREGISTRATION_REQ) {
		/**/
		hwres_to_release = rpmsg_mm_find_res_by_name(payin->emitter);
		hwres = hwres_to_release;
		poutdata->info = RPMSG_MM_UNREGISTRATION_ACK;
	}
	if (!hwres)
		status = 0;

	/* receiver becomes emitter and vice-versa */
	if (ackreq) {
		poutdata->len = sizeof(status);
		pdata = (uint32_t *)&poutdata->data[0];
		*pdata = status;
		hwres = rpmsg_mm_find_res_by_name(payin->receiver);
		rpmsg_mm_send_to_receivers((void *)hwres,
					   payin->emitter, poutdata,
					   payin->reserved, false);
	}
	if (hwres_to_release)
		rpmsg_mm_release_res(hwres_to_release);
	return 0;
}

/**
 * @fn void rpmsg_mm_task(void *p)
 * @brief Main function associated to the rpmsg_mm task
 * @param p pointer on the feature_set (Not used)
 *
 * Main purpose of this task is to wait for reception of message coming from
 * Linux.
 */
void rpmsg_mm_task(void *p)
{
	MessageQCopy_Handle handle;
	uint16_t len;
	int ret;
	uint32_t request_endpoint = MessageQCopy_ASSIGN_ANY;
	uint32_t endpoint, remote_endpoint = 0;
	uint32_t linfo;
	uint8_t buffer[MAX_PAYLOAD_SIZE];

	TRACE_INFO("RPMSG_MM Task starting ...\n");

	/* init RPMSG framework */
	if (RPMSG_Init() != 0)
		goto end;

	rpmsg_mm_init();
	/*
	 * 1st argument must be less than
	 * MessageQCopy_MAX_RESERVED_ENDPOINT
	 * or MessageQCopy_ASSIGN_ANY.
	 */
	handle = MessageQCopy_create(request_endpoint, &endpoint);
	if (!handle) {
		TRACE_ERR("%s MessageQCopy_create failure\n", __func__);
		goto end;
	}
	NameMap_register("service_mm", endpoint);
	rpmsg_mm_ctx.local_ept = endpoint;

	memset(buffer, 0, sizeof(buffer));

	while (1) {
		/* Set maximum data size that I can receive. */
		len = (uint16_t)sizeof(buffer);

		ret = MessageQCopy_recv(handle, (void *)buffer, &len,
					(uint32_t *)&remote_endpoint,
					MessageQCopy_FOREVER);

		if (ret == MessageQCopy_E_TIMEOUT)
			continue;

		/* MessageQCopy_recv succeeded ? */
		if (ret == MessageQCopy_S_SUCCESS &&
		    len <= RP_MSG_PAYLOAD_SIZE) {
			struct rpmsg_mm_payload *payload_data;
			struct s_rpmsg_mm_data *user_data;

			payload_data = (struct rpmsg_mm_payload *)buffer;
			user_data = &payload_data->user_data;
			linfo = RPMSG_MM_GET_INFO(user_data->info);

#if defined RPMSG_MM_DEBUG_MSG
			{
			unsigned int i = 0;

			TRACE_INFO("***** Message received ****\n");
			TRACE_INFO("* Sent From %s - Receiver : %s\n",
				   payload_data->emitter,
				   payload_data->receiver);
			TRACE_INFO("* Info : %s\n", info2str[linfo]);
			TRACE_INFO("* Ack requested : %s\n",
				   (RPMSG_MM_IS_ACK_REQUESTED(user_data->info) ?
				   "Yes" : "No"));
			TRACE_INFO("* Reserved : 0x%x\n",
				   payload_data->reserved);
			TRACE_INFO("* Length : %d\n", user_data->len);
			i = 0;
			while (i < user_data->len) {
				TRACE_INFO("* data [%d] : 0x%02x\n",
					   i, user_data->data[i]);
				i++;
			}
			}
#endif

			switch (linfo) {
			case RPMSG_MM_SHARED_RES_STATUS_REQ:
				/* GET Status message is transferred
				 * to every local endpoints
				 */
				rpmsg_mm_get_status(payload_data);
				break;
			case RPMSG_MM_SHARED_RES_LOCKED_ACK:
				rpmsg_mm_lock_resource_ack(payload_data);
				break;
			case RPMSG_MM_REARGEAR_STATUS_REQ:
				rpmsg_mm_get_reargear_status(payload_data);
				break;
			case RPMSG_MM_REGISTRATION_REQ:
			case RPMSG_MM_UNREGISTRATION_REQ:
				/* Used to align known remote endpoints */
				rpmsg_mm_remote_registration(payload_data,
							     remote_endpoint);
				break;
			default:
				/* Transfer the message to the
				 * targeted receiver
				 */
				rpmsg_mm_transfer_msg_to_local_ept(
								payload_data);
				break;
			}
		}
	}
end:
	TRACE_INFO("************ rpmsg_mm_task DELETED\n");
	vTaskDelete(NULL);
}

/*!< @endinternal */

/**
 * @fn void *rpmsg_mm_register_local_endpoint (char *endpoint_name,
 *				       t_pfct_rpmsg_mm_cb pCB,
 *				       void *priv)
 * @brief API used to register local instance
 * @param endpoint_name string identifying the local instance ("CortexM_xxx")
 * @param pCB callback used by rpmsg_mm to inform this instance about message
 * reception
 * @param priv private data
 * @return null in case of error or an handle to keep in mind to use other
 * services
 */
void *rpmsg_mm_register_local_endpoint(char *endpoint_name,
				       t_pfct_rpmsg_mm_cb pCB,
				       void *priv)
{
	return (rpmsg_mm_register_endpoint (endpoint_name, DEFAULT_LOCAL_EPT,
					    pCB, priv, RPMSG_MM_OWNER_LOCAL));
}

/**
 * @fn int rpmsg_mm_send_msg(void *handle, char *receiver,
 *			      struct s_rpmsg_mm_data *data)
 * @brief API used to send a message on Linux side to a specific receiver
 * @param handle identifier of the resource (from ::rpmsg_mm_register_endpoint)
 * @param receiver string identifying the remote instance ("CortexA_xxx")
 * @param data message to send
 * @return -1 in case of error, else return 0
 *
 * Generic service used to transfer a message to an identified remote user.
 *
 * \note if remote user is not yet registered, the message is aborted
 */
int rpmsg_send_private_message(void *handle, char *remote_endpoint,
			       void *data, int len)
{
	uint8_t buffer[MAX_PAYLOAD_SIZE];
	struct s_rpmsg_mm_data *user_data = (struct s_rpmsg_mm_data *)buffer;

	if (!handle || !remote_endpoint || !data)
		return 0;

	if ((len + sizeof(struct s_rpmsg_mm_data *)) > MAX_PAYLOAD_SIZE)
		return 0;

	memset(buffer, 0, sizeof(buffer));
	user_data->len = len;
	memcpy(user_data->data, data, user_data->len);
	user_data->info = RPMSG_MM_PRIVATE_MESSAGE;

	rpmsg_mm_send_to_receivers(handle, remote_endpoint,
				   user_data, NULL, false);

	return 0;
}

/**
 * @fn int rpmsg_mm_lock_resource(void *handle, int status,
 *			   int resource, char *receiver)
 * @brief API used to send a message on Linux side to inform remote endpoint
 * about user's wish to lock/unlock a resource.
 * @param handle identifier of the resource
 * (from ::rpmsg_mm_register_local_endpoint)
 * @param status ::RPMSG_MM_SHARED_RES_LOCKED or ::RPMSG_MM_SHARED_RES_UNLOCKED
 * @param resource hdw resource involved in this request (::rpmsg_mm_hw_res)
 * @param receiver string identifying the remote instance ("CortexA_xxx")
 * @return -1 in case of error, else return 0
 *
 * This function transfers ::RPMSG_MM_SHARED_RES_LOCKED or
 * ::RPMSG_MM_SHARED_RES_UNLOCKED message on Linux side.
 *
 * \note this function is blocked up to the ::RPMSG_MM_SHARED_RES_LOCKED_ACK
 * message reception.
 *
 * \warning
 * Usage of the resource is allowed ONLY after return of this function.
 */
int rpmsg_mm_lock_resource(void *handle, int status,
			   int resource, char *receiver)
{
	uint8_t buffer[MAX_PAYLOAD_SIZE];
	struct s_rpmsg_mm_data *user_data = (struct s_rpmsg_mm_data *)buffer;

	if (!handle || !receiver)
		return 0;

	if (!(status == RPMSG_MM_SHARED_RES_LOCKED ||
	      status == RPMSG_MM_SHARED_RES_UNLOCKED))
		return -1;

	memset(buffer, 0, sizeof(buffer));
	user_data->data[0] = resource;
	user_data->len = 1;
	user_data->info = RPMSG_MM_SET_ACK_INFO(status);

	return rpmsg_mm_send_to_receivers(handle, receiver,
					  user_data, NULL, true);
}

/**
 * @fn int rpmsg_mm_unlock_resource(void *handle, int resource, char *receiver)
 * @brief API used to send a message on Linux side to inform remote endpoint
 * about user's wish to unlock a resource.
 * @param handle identifier of the resource
 * (from ::rpmsg_mm_register_local_endpoint)
 * @param resource hdw resource involved in this request (::rpmsg_mm_hw_res)
 * @param receiver string identifying the remote instance ("CortexA_xxx")
 * @return -1 in case of error, else return 0
 *
 * This function transfers ::RPMSG_MM_SHARED_RES_UNLOCKED message on Linux side.
 *
 * \note this function is not blocking. Unlike to ::rpmsg_mm_lock_resource
 * service, this call doesn't wait for ::RPMSG_MM_SHARED_RES_LOCKED_ACK
 * message reception.
 */
int rpmsg_mm_unlock_resource(void *handle, int resource, char *receiver)
{
	uint8_t buffer[MAX_PAYLOAD_SIZE];
	struct s_rpmsg_mm_data *user_data = (struct s_rpmsg_mm_data *)buffer;

	memset(buffer, 0, sizeof(buffer));
	user_data->info = RPMSG_MM_RESET_ACK_INFO(RPMSG_MM_SHARED_RES_UNLOCKED);
	user_data->len = 1;
	user_data->data[0] = resource;

	if (rpmsg_mm_send_to_receivers(handle, receiver,
				       user_data, NULL, false) < 0)
		TRACE_INFO("Unable to send the message, unlock allowed\n");
	return 0;
}

/**
 * @fn void *rpmsg_mm_register_service(uint8_t services,
 *				       t_pfct_rpmsg_mm_service_cb pCB,
 *				       void *priv)
 * @brief API used to register a callback to one or several services
 * @param services list of services (::rpmsg_mm_service) on which caller
 * needs to be inform
 * @param pCB callback of type ::t_pfct_rpmsg_mm_service_cb
 * @param priv private data
 * @return it returns the status
 * - Null in case of error
 * - something else in case of success
 *
 * Generic callback used to handle generic information from CortexA UserLand.\n
 * This callback will be called when rpmsg_mm receives a ::RPMSG_MM_USER_SERVICE
 * message if the associated service (data[0]) is part of those that user
 * registered.\n
 * Treatment just consists in storing callback, private data and associated
 * services.
 */
void *rpmsg_mm_register_service(uint8_t services,
				t_pfct_rpmsg_mm_service_cb pCB, void *priv)
{
	uint16_t index;
	void *status = NULL;

	LOCK_SERVICES();
	index = rpmsg_mm_ctx.latestUserId + 1;
	if (index >= MAX_NB_RESOURCES) {
		TRACE_INFO("No way to register a new services user\n");
		goto go_out;
	}
	TRACE_INFO("New service registration => [ services : %d ]\n",
		   services);
	rpmsg_mm_ctx.services_users[index].pCB = pCB;
	rpmsg_mm_ctx.services_users[index].priv = priv;
	rpmsg_mm_ctx.services_users[index].services = services;
	rpmsg_mm_ctx.latestUserId = index;
	status = (void *)&rpmsg_mm_ctx.services_users[index];
go_out:
	UNLOCK_SERVICES();
	return status;
}

