/**
 * @file sta_rpmsg_mm.h
 * @brief RPMsg communication between CortexM and CortexA for multimedia tasks
 *
 * Copyright (C) ST-Microelectronics SA 2015
 * @author: APG-MID team
 */
#if !defined STA_RPMSG_MM_H
#define STA_RPMSG_MM_H

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

#include <linux/ioctl.h>

#define RPMSG_MM_IOC_MAGIC      'L'

#define RPMSG_MM_IOC_GET_RG_STATUS	_IOR(RPMSG_MM_IOC_MAGIC, 1, int)
/*!< IOCTL used to get the rear gear status */
#define RPMSG_MM_IOC_STOP_ANIM	_IOW(RPMSG_MM_IOC_MAGIC, 2, int)
/*!< IOCTL used to stop the splash screen animation */
#define RPMSG_MM_IOC_STOP_RVC	_IOW(RPMSG_MM_IOC_MAGIC, 3, int)
/*!< IOCTL used to stop the rear view usecase */
#define RPMSG_MM_IOC_START_RVC	_IOW(RPMSG_MM_IOC_MAGIC, 4, int)
/*!< IOCTL used to start the rear view usecase */
#define RPMSG_MM_IOC_RVC_DEBUG_MODE	_IOW(RPMSG_MM_IOC_MAGIC, 5, int)
/*!< IOCTL used to change RVC debug mode */
#define RPMSG_MM_IOC_RVC_AX_TAKEOVER_REQ	_IOW(RPMSG_MM_IOC_MAGIC, 6, int)
/*!< IOCTL used to request A7 takeover of RVC resources */

#define RPMSG_MM_IOC_MAXNR      (6)

/**
 * @enum rpmsg_mm_msg_type
 * @brief Identify linux type of instance (created from userland or from kernel)
 */
enum rpmsg_mm_msg_type {
	RPMSG_MM_APPLI, /*!< instance has been created from userland */
	RPMSG_MM_DRIVER, /*!< instance created from kernel linux */
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
	/*!< Message used to transfer specific 'services' from Linux
	 * to CortexM.\n
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
 * @enum rpmsg_mm_hw_res
 * @brief identifier of hardware resources (bitfield usage)
 */
enum rpmsg_mm_hw_res {
	RPMSG_MM_HW_RES_G1 = 1 << 0, /*!< bit0 :G1 video decoder IP */
	RPMSG_MM_HW_RES_RVC = 1 << 1, /*!< bit1: includes ADV/VIP and SGA IPs */
	/* OVERLAY MUST BE THE LAST DECLARATION */
	RPMSG_MM_HW_RES_OVERLAY_BASE = 1 << 2, /*!< bit2: Layer 1 of the LTDC */
};

/**
 * @enum rpmsg_mm_rvc_debug_mode
 * @brief sets the RVC debug mode
 */
enum rpmsg_mm_rvc_debug_mode {
	RVC_DEBUG_MODE_NORMAL, /*!< rear gear controlled by GPIO input */
	RVC_DEBUG_MODE_MANUAL_TOGGLING, /*!< rear gear toggling on demand */
	RVC_DEBUG_MODE_AUTO_TOGGLING, /*!< rear gear toggling periodically */
};

#define RPMSG_MM_NB_OVERLAY  4

/**
 * @struct rpmsg_mm_msg_hdr
 * @brief common header for all MM messages
 *
 * All MM messages will start with this common header (which will begin
 * right after the standard rpmsg header ends).
 */
struct rpmsg_mm_msg_hdr {
	u32 info;
	/*!< Kind of message (refer to ::rpmsg_mm_msg_info) */
	u32 len;
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
typedef int (*t_pfct_rpmsg_mm_cb)(struct rpmsg_mm_msg_hdr *data, void *priv);

/**
 * @fn void *sta_rpmsg_register_endpoint(char *endpoint_name,
 *					 t_pfct_rpmsg_mm_cb callback,
 *					 void *priv)
 * @brief API provided to kernel drivers to register a driver endpoint
 * @param endpoint_name string identifying the local instance ("Ax_xxx")
 * @param callback callback used by rpmsg_mm to inform this instance about
 * message reception
 * @param priv private data
 * @return null in case of error or an handle to keep in mind to use other
 * services
 */
void *sta_rpmsg_register_endpoint(char *endpoint_name,
				  t_pfct_rpmsg_mm_cb callback,
				  void *priv);

/**
 * @fn int sta_rpmsg_unregister_endpoint(void *handle)
 * @brief API provided to kernel drivers to unregister a driver endpoint
 * @param handle  identifier of the endpoint (previously get from
 * ::sta_rpmsg_register_endpoint service
 * @return return -1 in case of error, else return 0
 */
int sta_rpmsg_unregister_endpoint(void *handle);

/**
 * @fn int sta_rpmsg_get_remote_resources_usage(void *handle)
 * @brief API provided to kernel drivers to get current resources usage
 * @param handle  identifier of the endpoint (previously get from
 * ::sta_rpmsg_register_endpoint service
 * @return return -1 in case of error, else value built as a bitfield using
 * ::rpmsg_mm_hw_res definitions
 */
int sta_rpmsg_get_remote_resources_usage(void *handle);

/**
 * @fn int sta_rpmsg_send_private_message(void *handle, char *receiver,
					  void *data, int len)
 * @brief API provided to kernel drivers to send a private message on CortexM
 * side to a specific receiver.
 * @param handle identifier of the resource (from ::sta_rpmsg_register_endpoint)
 * @param receiver string identifying the remote instance ("Mx_xxx")
 * @param data pointer on data to send
 * @param len length of data sent on remote side
 * @return -1 in case of error, else return 0
 */
int sta_rpmsg_send_private_message(void *handle, char *remote_endpoint,
				   void *data, int len);

/**
 * @fn int sta_rpmsg_mm_send(void *handle, char *remote_endpoint,
 *			     struct rpmsg_mm_msg_hdr *data)
 * @brief API provided to kernel drivers to  send a message on CortexM side to a
 * specific receiver
 * @param handle identifier of the resource (from ::sta_rpmsg_register_endpoint)
 * @param remote_endpoint string identifying the remote instance ("Mx_xxx")
 * @param data data to send
 * @return -1 in case of error, else return 0
 *
 * Service used to transfer a Generic message to an identified remote user.
 *
 * \note This call is just sending the data to the remote without waiting for
 * any answer.
 */
int sta_rpmsg_mm_send(void *handle, char *remote_endpoint,
		      struct rpmsg_mm_msg_hdr *data);

#endif
