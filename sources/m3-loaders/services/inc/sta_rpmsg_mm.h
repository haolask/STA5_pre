/**
 * @file sta_rpmsg_mm.h
 * @brief RPMsg communication between CortexM and CortexA for multimedia tasks
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */
#if !defined STA_RPMSG_MM_H
#define STA_RPMSG_MM_H

#include "sta_rpmsg.h"
#include "MessageQCopy.h"
#include "NameMap.h"

#define MAX_NAME_LENGTH 15
#define RPSMG_MM_EPT_CORTEXM_MM "Mx_MM"
#define RPSMG_MM_EPT_CORTEXM_G1 "Mx_G1"
#define RPSMG_MM_EPT_CORTEXM_DISPLAY "Mx_LCD"
#define RPSMG_MM_EPT_CORTEXM_RVC "Mx_RVC"
#define RPSMG_MM_EPT_CORTEXM_TVDEC "Mx_TVDEC"

#define RPSMG_MM_EPT_CORTEXA_MM "Ax_MM"
#define RPSMG_MM_EPT_CORTEXA_G1 "Ax_G1"
#define RPSMG_MM_EPT_CORTEXA_DISPLAY "Ax_LCD"
#define RPSMG_MM_EPT_CORTEXA_RVC "Ax_RVC"
#define RPSMG_MM_EPT_CORTEXA_TVDEC "Ax_TVDEC"

#define MAX_PAYLOAD_SIZE 128

/**
 * @enum rpmsg_mm_owner
 * @brief Used to indicate if the instance comes from local core or remote core
 */
enum rpmsg_mm_owner {
	RPMSG_MM_OWNER_LOCAL = 1,/*!< Instance registered locally to the core */
	RPMSG_MM_OWNER_REMOTE /*!< Instance registered from a remote core */
};

/**
 * @enum rpmsg_mm_hw_res
 * @brief identifier of hardware resources (bitfield usage)
 */
enum rpmsg_mm_hw_res {
	RPMSG_MM_HW_RES_G1 = 1 << 0, /*!< bit0 :G1 video decoder IP */
	RPMSG_MM_HW_RES_RVC = 1 << 1, /*!< bit1: includes ADV/VIP and SGA IPs */
	/* OVERLAY MUST BE THE LAST DECLARATION */
	RPMSG_MM_HW_RES_OVERLAY_BASE = 1 << 2, /*!< bit2: Layer 1 of the LTDC */
};

#define RPMSG_MM_NB_OVERLAY  4

/**
 * @enum rpmsg_mm_msg_type
 * @brief Identify linux type of instance (created from userland or from kernel)
 *
 * On CortexM side, it's mainly used to manage the name of the user instance :
 * "Ax_MM" is used by rpmsg_mm registered users
 * but inside rpmsg_mm, "Ax_MM" indicates every instances identified by name
 * starting by "Ax_MM" : "Ax_MM_xxx"
 */
enum rpmsg_mm_msg_type {
	RPMSG_MM_APPLI, /*!< Remote instance has been created from userland */
	RPMSG_MM_DRIVER, /*!< Remote instance created from kernel linux */
};

/**
 * @enum rpmsg_mm_msg_info
 * @brief Key information to identify the kind of message transferred between
 * both cores
 */
enum rpmsg_mm_msg_info {
	RPMSG_MM_RESERVED = 0, /*!< Not used - Reserved */

	RPMSG_MM_REGISTRATION_REQ = 1,
	/*!< Registration request.\n It's systematically followed by an answer
	 * ::RPMSG_MM_REGISTRATION_ACK info
	 */

	RPMSG_MM_REGISTRATION_ACK = 2,
	/*!< Registration acknowledge used to indicate registration has been
	 * taken into account. \n data[0] includes the status:
	 * - 0: registration failed
	 * - 1: successful registration
	 */

	RPMSG_MM_UNREGISTRATION_REQ = 3,
	/*!< Unregistration request.\n It's systematically followed by an answer
	 * ::RPMSG_MM_UNREGISTRATION_ACK info
	 */

	RPMSG_MM_UNREGISTRATION_ACK = 4,
	/*!< Unregistration acknowledge used to indicate unregistration has been
	 * taken into account. \n data[0] includes the status:
	 * - 0: registration failed
	 * - 1: successful registration
	 */

	RPMSG_MM_SHARED_RES_STATUS_REQ = 5,
	/*!< Used to request the status of hardware resources usage. \n
	 * It's systematically followed by an answer
	 * ::RPMSG_MM_SHARED_RES_STATUS_ACK.
	 */

	RPMSG_MM_SHARED_RES_STATUS_ACK = 6,
	/*!< Message transferred to return the usage of shared hardware
	 * resources.\n data[0] includes the result. It's used as a bitfield in
	 * which :
	 * - bit = 1 indicates that the resource is currently locked (used)
	 * - bit = 0 indicates that the resource is unlocked (not used)
	 *
	 * \note bit usage is defined in ::rpmsg_mm_hw_res
	 */

	RPMSG_MM_SHARED_RES_LOCKED = 7,
	/*!< Message transferred to indicate a resource is going to be locked.\n
	 * Systematically followed by an answer ::RPMSG_MM_SHARED_RES_LOCKED_ACK
	 * \n data[0] includes the resource (Only one ::rpmsg_mm_hw_res) which
	 * is requested to be locked.
	 * \warning
	 * - user MUST not used the resource before reception of the acknowledge
	 * - receiver of this message MUST ensure resource is freed before
	 * sending back the acknowledge
	 */

	RPMSG_MM_SHARED_RES_LOCKED_ACK = 8,
	/*!< Acknowledge a lock or unlock request.
	 * \warning
	 * - Lock of Unlock message sender MUST not used the resource before
	 * reception of this acknowledge.
	 */

	RPMSG_MM_SHARED_RES_UNLOCKED = 9,
	/*!< Message transferred to indicate a resource is going to be
	 * unlocked.\n Systematically followed by an answer
	 * ::RPMSG_MM_SHARED_RES_LOCKED_ACK.\n
	 * data[0] includes the resource (Only one ::rpmsg_mm_hw_res) which is
	 * requested to be unlocked.
	 * \warning
	 * - user MUST release the resource before sending this message
	 */

	RPMSG_MM_COMM_UNAVAILABLE = 10,
	/*!< Message used to indicate that rpmsg link between both cores is
	 * nomore available
	 */

	RPMSG_MM_COMM_AVAILABLE = 11,
	/*!< Message used to indicate that rpmsg link between both cores is
	 * available
	 */

	RPMSG_MM_USER_SERVICE = 12,
	/*!< Message used to transfer specific 'services'
	 * from Linux to CortexM.\n
	 * data[0] includes the transfer service \n
	 * List of these services is described in ::rpmsg_mm_service
	 */

	/* RVC dedicated messages */
	RPMSG_MM_REARGEAR_STATUS_REQ = 13,
	/*!< RVC message transferred to get the current status of rear gear
	 * (engaged or not).\n
	 * Systematically followed by an answer ::RPMSG_MM_REARGEAR_STATUS_ACK
	 */

	RPMSG_MM_REARGEAR_STATUS_ACK = 14,
	/*!< RVC message transferred to return the current status of rear gear
	 * (engaged or not).\n
	 * data[0] includes the status:
	 * - 0 : rear gear is disengaged
	 * - 1 : rear gear is engaged
	 */

	RPMSG_MM_PRIVATE_MESSAGE = 15,
	/*!< message transferred to send a custom information between two cores.
	 * in this case, there is no treatment inside RPMSG_MM.
	 * Additional data are used to describe the information
	 * only understandable by message's emitter and receiver.
	 */
};

#define RPMSG_MM_ACK_REQUESTED 0x8000000
#define RPMSG_MM_SET_ACK_INFO(info) ((info) | RPMSG_MM_ACK_REQUESTED)
#define RPMSG_MM_RESET_ACK_INFO(info) ((info) & ~RPMSG_MM_ACK_REQUESTED)
#define RPMSG_MM_IS_ACK_REQUESTED(info) (((info) & RPMSG_MM_ACK_REQUESTED) != 0)
#define RPMSG_MM_GET_INFO(info) ((info) & ~RPMSG_MM_ACK_REQUESTED)

/**
 * @enum rpmsg_mm_service
 * @brief List of user-land services to interact with use-cases
 * handled by CortexM.
 *
 * \note it's used as data associated to ::RPMSG_MM_USER_SERVICE message
 */
enum rpmsg_mm_service {
	RPMSG_MM_SERV_STOP_ANIM = 1 << 0,
	/*!< Service used to indicate that the Linux boot (cold or warm boot)
	 * is ended so that we can stop the splash screen animation, the audio
	 * 'welcome' sound... automatically started at CortexM boot.\n
	 * Typically this info is transferred in two following cases :
	 * - At cold boot, via a dedicated systemd service
	 * - At warm boot (suspend/resume), via a pm-utils hook
	 * (/etc/pm/sleep.d/...)
	 */

	RPMSG_MM_SERV_START_RVC = 1 << 1,
	/*!< Service used mainly for test purpose. Start the RVC use-case
	 * independently on GPIO (RearGear detection) status.
	 */

	RPMSG_MM_SERV_STOP_RVC = 1 << 2,
	/*!< Service used mainly for test purpose. Stop the RVC use-case
	 * independently on GPIO (Rear gear detection) status.
	 */

	RPMSG_MM_SERV_RVC_DEBUG_MODE = 1 << 3,
	/*!< Service used mainly for test purpose. Sets the debug mode for RVC.
	 * data to be sent must be an enum rpmsg_mm_rvc_debug_mode.
	 */

	RPMSG_MM_SERV_RVC_AX_TAKEOVER = 1 << 4,
	/*!< Service used to request full takeover of RVC resources to A7.
	 */
};

/**
 * @enum rpmsg_mm_rvc_debug_mode
 * @brief sets the RVC debug mode
 */
enum rpmsg_mm_rvc_debug_mode {
	RVC_DEBUG_MODE_NORMAL, /*!< rear gear controlled by 12V input */
	RVC_DEBUG_MODE_MANUAL_TOGGLING, /*!< rear gear toggling on demand */
	RVC_DEBUG_MODE_AUTO_TOGGLING, /*!< rear gear toggling periodically */
};

/**
 * @struct s_rpmsg_mm_data
 * @brief common header for all MM messages
 *
 * All MM messages will start with this common header (which will begin
 * right after the standard rpmsg header ends).
 */
struct s_rpmsg_mm_data {
	uint32_t info;
	/*!< Kind of message (refer to ::rpmsg_mm_msg_info) */
	uint32_t len;
	/*!< Size in bytes of additional data associated to this message. */
	char data[0]; /*!< pointer on additional data */
};

/**
 * @typedef t_pfct_rpmsg_mm_cb
 * @brief common function pointer definition used to interact with registered
 * instance.\n
 * Called when the owner of this callback receives a message.
 *
 * @param data pointer of payload data (received message)
 * @param priv private data previously send at registration
 */
typedef int (*t_pfct_rpmsg_mm_cb)(struct s_rpmsg_mm_data *data, void *priv);

/**
 * @typedef t_pfct_rpmsg_mm_service_cb
 * @brief common function pointer definition used to be inform about service
 * reception.
 *
 * @param services services trigged by remote users
 * (bitfield ::rpmsg_mm_service)
 * @param priv private data previously send at registration
 * @param len length of data
 * @param data pointer of payload data
 */
typedef int (*t_pfct_rpmsg_mm_service_cb)(uint8_t services, void *priv, int len,
					  void *data);

/**
 * @fn void *rpmsg_mm_register_local_endpoint (char *endpoint_name,
 *				       t_pfct_rpmsg_mm_cb pCB,
 *				       void *priv)
 * @brief API used to register local instance
 * @param endpoint_name string identifying the local instance ("Mx_xxx")
 * @param pCB callback used by rpmsg_mm to inform this instance about message
 * reception
 * @param priv private data
 * @return null in case of error or an handle to keep in mind to use other
 * services
 */
void *rpmsg_mm_register_local_endpoint(char *endpoint_name,
				       t_pfct_rpmsg_mm_cb pCB, void *priv);

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
				t_pfct_rpmsg_mm_service_cb pCB, void *priv);

/**
 * @fn int rpmsg_send_private_message(void *handle, char *receiver,
 *			       void *data, int len)
 * @brief API used to send a message on Linux side to a specific receiver
 * @param handle identifier of the resource (from ::rpmsg_mm_register_endpoint)
 * @param receiver string identifying the remote instance ("A7_xxx")
 * @param data message to send
 * @param len size of data to transfer
 * @return -1 in case of error, else return 0
 *
 * Generic service used to transfer a message to an identified remote user.
 *
 * \note if remote user is not yet registered, the message is aborted
 */
int rpmsg_send_private_message(void *handle, char *remote_endpoint,
			       void *data, int len);

/**
 * @fn int rpmsg_mm_send_reargear_status(int status)
 * @brief API used to send transfer the rear gear status on remote side
 * @param status rear gear status ( 1: engaged; 0: disengaged )
 * @return -1 in case of error, else return 0
 *
 * Service called by entity able to detect the rear gear status when it changes.
 */
int rpmsg_mm_send_reargear_status(int status);

/**
 * @fn int rpmsg_mm_lock_resource(void *handle, int status,
 *				  int resource, char *receiver)
 * @brief API used to send a message on Linux side to inform remote endpoint
 * about user's wish to lock/unlock a resource.
 * @param handle identifier of the resource
 * (from ::rpmsg_mm_register_local_endpoint)
 * @param status ::RPMSG_MM_SHARED_RES_LOCKED or ::RPMSG_MM_SHARED_RES_UNLOCKED
 * @param resource hdw resource involved in this request (::rpmsg_mm_hw_res)
 * @param receiver string identifying the remote instance ("Ax_xxx")
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
int rpmsg_mm_lock_resource(void *handle, int status, int resource,
			   char *receiver);

/**
 * @fn int rpmsg_mm_unlock_resource(void *handle, int resource, char *receiver)
 * @brief API used to send a message on Linux side to inform remote endpoint
 * about user's wish to unlock a resource.
 * @param handle identifier of the resource
 * (from ::rpmsg_mm_register_local_endpoint)
 * @param resource hdw resource involved in this request (::rpmsg_mm_hw_res)
 * @param receiver string identifying the remote instance ("Ax_xxx")
 * @return -1 in case of error, else return 0
 *
 * This function transfers ::RPMSG_MM_SHARED_RES_UNLOCKED message on Linux side.
 *
 * \note this function is not blocking. Unlike to ::rpmsg_mm_lock_resource
 * service, this call doesn't wait for ::RPMSG_MM_SHARED_RES_LOCKED_ACK
 * message reception.
 */
int rpmsg_mm_unlock_resource(void *handle, int resource, char *receiver);
#endif
