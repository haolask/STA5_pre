/**
 * @file rpmx_common.h
 * @brief RPMX common interface
 *
 * Copyright (C) ST-Microelectronics SA 2018
 * @author: ADG-MID team
 */

#ifndef _RPMX_COMMON_H_
#define _RPMX_COMMON_H_

#include "sdmmc.h"
#include "sta_sqi.h"

/* Key storage supported NVM memory types */
#define NVM_NONE		-1
#define NVM_NAND		0 /* Not yet supported */
#define NVM_MMC			1
#define NVM_SQI			2

#if KS_MEMTYPE == NVM_SQI
#define t_nvm_ctx struct t_sqi_ctx
#elif KS_MEMTYPE == NVM_MMC
#define t_nvm_ctx struct t_mmc_ctx
#else
#define t_nvm_ctx void
#endif

/**
 * @brief   RPMX device descriptor ID
 *          It represents the ID of the family the secure device belongs to
 */
enum rpmx_family
{
	RPMX_DEV_FAMILY_1,      /* Macronix secure RPMC NOR */
	RPMX_DEV_FAMILY_2,      /* Micron secure RPMC NOR */
	RPMX_DEV_FAMILY_3,      /* eMMC secure RPMB device (JEDEC compliant) */
	RPMX_DEV_FAMILY_4,
	RPMX_DEV_FAMILY_5,
	RPMX_DEV_FAMILY_6,
	RPMX_DEV_FAMILY_7,
	RPMX_DEV_FAMILY_8,
	RPMX_DEV_FAMILY_UNKNOWN
};

/**
 * @brief   Type of the status of the external secure device
 *          (for monotonic counter)
 */
enum mc_rpmx_status_t {
	MC_RPMX_DEVICE_INIT	=  0,	/*!< Device init success */
	MC_RPMX_DEVICE_ERR	=  1,	/*!< Device init failure */
	MC_RPMX_ROOTKEY_INIT	=  2,	/*!< Root key has been initialized */
	MC_RPMX_ROOTKEY_AVAIL	=  3,	/*!< Root key has already been initialized */
	MC_RPMX_ROOTKEY_ERR	=  4,	/*!< Root key error */
	MC_RPMX_SESSKEY_INIT	=  5,	/*!< Session key has been initialized */
	MC_RPMX_SESSKEY_ERR	=  6,	/*!< Session key error */
	MC_RPMX_VALID_VALUE	=  7,	/*!< Valid value is retrieved */
	MC_RPMX_NO_VALUE	=  8	/*!< Invalid value is retrieved */
};

/**
 * @brief	Type of a the message exchanged during the secure device
 *		initialization phase
 */
struct mc_rpmx_msg_t {
	enum mc_rpmx_status_t status;		/*!< RPMX status */
	enum rpmx_family device;		/*!< RPMX device descriptor */
};

/**
 * @brief   Type of counter value.
 */
typedef uint32_t mcvalue_t;

/*
 * RPMx error codes enum
 */
enum RPMx_ErrTy {
	RPMx_OK                     =   0, /* RPMx successful completion */
	RPMx_KO                     =  -1, /* RPMx generic failure */
	RPMx_BUSY_ERROR             =  -2, /* RPMx card busy error */
	RPMx_SIG_ERROR              =  -3, /* RPMx signature error */
	RPMx_PAYLOAD_ERROR          =  -4, /* RPMx payload error */
	RPMx_INIT_ERROR             =  -5, /* RPMx counter not initialized */
	RPMx_ROOT_KEY_AVAILABLE     =  -6, /* RPMx ROOT KEY already there */
	RPMx_LS_DEVICE_INIT         =  -7, /* LS device init success */
	RPMx_LS_DEVICE_ERR          =  -8, /* LS device init failure */
	RPMx_LS_DEVICE_WARNING      =  -9, /* LS device erase was necessary */
	RPMx_LS_NO_DEVICE           =  -10, /* LS device not found at all */
	RPMx_LS_DEVICE_FAILURE      =  -11, /* LS device not reliable */
	RPMx_LS_DEVICE_FULL	    =  -12  /* LS device full */
};

/**
 * @brief       rpmx_init
 *              This routine writes the root key to the remote RPMB storage
 * @param	ctx: NVM context
 * @param	descriptor: pointer to the descriptor of the family.
 * @param	payload: Write Root key payload
 * @return      RPMx_OK if ok else error code
 */
enum RPMx_ErrTy rpmx_init(t_nvm_ctx *ctx,
			  enum rpmx_family *descriptor, uint8_t *payload);

/**
 * @brief       rpmx_discover
 *              This routine discovers the monotonic counter remote device
 *              family.
 * @param	ctx: NVM context
 * @param       descriptor: pointer to the descriptor of the family.
 * @return      RPMX_LS_DEVICE_FAILURE if the drive is not functional
 *              RPMX_LS_DEVICE_ERR in case it cannot be erased
 *              RPMx_OK if ok
 */
enum RPMx_ErrTy rpmx_discover(t_nvm_ctx *ctx, enum rpmx_family *descriptor);

#endif // _RPMX_COMMON_H_
