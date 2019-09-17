/**
 * @file    CSE_ext_OTP.h
 * @brief   CSE OTP management module for telemaco3p module - header
 * @details Set of functions used to manage access OTP area via the HSM.
 *
 *
 * @addtogroup TC3P_driver
 * @{
 * @addtogroup API Functions
 * @{
 */

#include "cse_typedefs.h"

typedef uint32_t t_uint32;
typedef uint8_t t_uint8;
typedef uint32_t t_bool;

/*!
 * @brief	boot security levels
 *		It describes all the possible boot security levels
 */
enum t_boot_security_level {
	BOOT_SECURITY_LEVEL_0 = 0,	/*!< NO BOOT SECURITY (LEVEL 0) */
	BOOT_SECURITY_LEVEL_1 = 1,	/*!< SECURITY LEVEL 1 */
	BOOT_SECURITY_LEVEL_2 = 2,	/*!< SECURITY LEVEL 2 */
	BOOT_SECURITY_LEVEL_3 = 3,	/*!< SECURITY LEVEL 3 */
	BOOT_SECURITY_LEVEL_4 = 4,	/*!< SECURITY LEVEL 4 */
	BOOT_SECURITY_LEVEL_5 = 5,	/*!< SECURITY LEVEL 5 */
	BOOT_SECURITY_LEVEL_6 = 6,	/*!< SECURITY LEVEL 6 */
};

/*!
 * @brief	user OTP boot area layout (32 bit).
 *		It describes all the field burned by the user in order to
 *		implement remap option "C"
 *		Boot is not performed thorugh a sequence of attempts,
 *		but setting a precise boot sequence through
 *		guidelines described in this structure.
 */
struct  t_safemem_boot_cfg {
	union {
		struct {
			/*
			 * LSB half word - boot parameters
			 */

			/* 8 bits */

			/*
			 *!< To go back to remap boot flow in case Remap Bypass
			 * is set to 1:
			 * 0 - don't bypass SAFEMEM,
			 * 1 - bypass SAFEMEM
			 */
			t_uint32  BOOT_SAFEMEM_BYPASS : 1;
			 /*
			  *!< SAFEMEM bypass sequence:
			  * 0 - REMAP "A", 1 - REMAP "B",
			  * 2 - REMAP "C", 3 -  REMAP "D”,
			  * 4 - REMAP "E", 5 - REMAP "F"
			  */
			t_uint32  BOOT_BYPASS_SEQUENCE : 3;
			/*!< Candidate boot source: allowed values 0 - 9 */
			t_uint32  BOOT_PERIPHERAL : 4;

			/* 8 bits */
			union {
				struct {
					/*!< NAND bus width: 0 - 8x, 1 - 16x */
					t_uint8 NAND_BUS_WIDTH : 1;
					/*
					 *!< NAND page type:
					 * 0 - LARGE PAGE,
					 * 1 - VERY LARGE PAGE
					 */
					t_uint8 NAND_PAGE_TYPE : 1;
					/*
					 *!< NAND GPIO config:
					 * 0 - GPIO_Settings_FSMC_x8,
					 * 1 - GPIO_Settings_FSMC_x16,
					 * other - notallowed
					 */
					t_uint8 NAND_GPIO_CONFIG : 2;
					/*!< NAND unused field */
					t_uint8 NAND_unused : 4;
				} NAND_BOOT_OPTION;

				struct {
					/*
					 *!< SDMMC boot partition:
					 * 0 - user partition,
					 * 1 - boot partition
					 */
					t_uint8 SDMMC_BOOT_PARTITION : 1;
					/*
					 *!< SDMMC bus width:
					 * 0 - 1 bit, 1 - 4 bit, 2 - 8 bit
					 */
					t_uint8 SDMMC_BUS_WIDTH : 2;
					/*
					 *!< SDMMC SPEED:
					 * 0 - Normal Speed, 1 - High Speed
					 */
					t_uint8 SDMMC_SPEED : 1;
					/*
					 *!< SDMMC GPIO config SDMMC1:
					 * 0 - GPIO_Settings_SDMMC1_1_set,
					 * 1 - GPIO_Settings_SDMMC1_2_set,
					 * other - not allowed
					 * SDMMC2:
					 * 0 - GPIO_Settings_SDMMC2_1_set,
					 * other - not allowed
					 */
					t_uint8 SDMMC_GPIO_CONFIG : 2;
					/*!< SDMMC unused field */
					t_uint8 SDMMC_unused : 2;
				} SDMMC_BOOT_OPTION;

				struct {
					/*
					 *!< SERIAL NOR: mode:
					 * GPIO configuration
					 */
					t_uint8 SERIAL_NOR_GPIO_CONFIG : 2;
					/*!< SERIAL NOR: unused field */
					t_uint8 SERIAL_NOR_unused : 6;
				} SERIAL_NOR_BOOT_OPTION;

			} BOOT_PERIPHERAL_OPTIONS;

			/*
			 * MSB half word - security level
			 */
			/* 16 bits */

			/*!< Boot security level */
			t_uint32 BOOT_SECURITY_LEVEL : 4;
			/*!< SAFEMEM unused field */
			t_uint32 BOOT_unused2 : 12;
		} BIT;

	/*!< SAFEMEM entire 32 bit word	*/
	t_uint32 WORD;

	} SAFEMEM_BOOT_CONFIG;
};

#define OTP_SUCCESS		0
/*!< OTP generic failure */
#define OTP_FAILURE		-1
/*!< OTP customer certificate revision exceeded */
#define OTP_CUST_CERT_EXCEED	-2
 /*!< OTP security revision revision exceeded */
#define OTP_SECR_REV_EXCEED	-3

#define CUSTOMER_ID             1
#define CUSTOMER_CERT_VER       2
#define CUSTOMER_CERT_HASH_INFO 3
#define CUSTOMER_CERT_HASH      4
#define SECURITY_LEVEL          5
#define SECURITY_VERSION        6
#define BOOT_PARAMETERS         7
#define ECC                     8
#define MONOTONIC_COUNTER       9

/*============================================================================*/
/*!
 * @brief	otp_Read_Word:
 * @details	This routine reads a word from OTP

 * @param[in]	: word_addr:	progressive word address
 * @param[out]	: word_to_read:	pointer to store the read value
 * @return	: OTP error condition
 */
extern int otp_Read_Word(uint32_t word_addr, uint32_t *pword_to_read);

/*!
 * @brief	otp_Write_Word:
 *		This routine writes a word to OTP
 * @param	: word_addr:	progressive word address
 * @param	: word_to_read:	value to be written
 * @return	: OTP error condition
 */
extern int otp_Write_Word(uint32_t word_addr, uint32_t word_to_write);

/*!
 * @brief	otp_Get_Security_Level:
 * @details	This routine returns the Security level value word from OTP
 *		If an error occur in the HSM service request,
 *		the returned security level variable is not updated
 * @param[out]	plevel - pointer to a t_boot_security_level variable where
 *		the value will be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Get_Security_Level(enum t_boot_security_level *plevel);

extern t_bool otp_Is_Empty(void);

/*!
 * @brief	otp_Get_Cust_ID:
 * @details	This routine returns the Customer ID value word from OTP
 *		If an error occur in the HSM service request,
 *		the returned variable is not updated
 * @param[out]	pcid - pointer to a 32b unsigned integer variable where
 *		the value will be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Get_Cust_ID(t_uint32 *pcid);

/*!
 * @brief	otp_Get_Cust_Cert_Ver:
 * @details	This routine returns the Customer Certificate Version word from
 *		OTP
 * @param[out]	pver - pointer to a 32b unsigned integer where the value will
 *		be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Get_Cust_Cert_Ver(t_uint32  *pver);

/*!
 * @brief	otp_Get_Secr_Ver:
 * @details	This routine returns the Security version value word from OTP
 *		If an error occur in the HSM service request, the returned
 *		security level variable is not updated
 * @param[out]	pver - pointer to a 32b unsigned integer variable where the
 *		value will be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Get_Secr_Ver(t_uint32  *pver);

/*!
 * @brief	otp_Get_Boot_Parameters
 * @details	This routine returns the Boot parameters value from OTP
 *		If an error occur in the HSM service request,
 *		the returned security level variable is not updated
 * @param[out]	pver - pointer to a t_safemem_boot_cfg variable where the value
 *		will be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Get_Boot_Parameters(volatile struct t_safemem_boot_cfg *pinfo);

/*!
 * @brief	otp_Get_Customer_Certificate_Hash_Info
 * @details	This routine returns the customer root certificate hash
 *		information from OTP
 *		(ie. the type of hash used and if root customer certificate
 *		was programmed)
 * @param[out]	proot_hash_info - pointer to a 32b unsigned integer where the
 *		value will be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Get_Customer_Certificate_Hash_Info(uint32_t *proot_hash_info);

/*!
 * @brief	otp_Get_Customer_Certificate_Hash
 * @details	This routine first check if the customer root certificate hash
 *		has been written and enabled
 *		If yes, copies the associated 512bits from OTP to the specified
 *		destination
 * @param[out]	phash - pointer to a byte array where the value will be written
 *		- must be at least 64 bytes long (512bits)
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE - if customer hash was not written already, or
 *		errors while reading OTP
 * @retval	: OTP_SUCCESS
 */
extern int otp_Get_Customer_Certificate_Hash(uint8_t *phash);

/*
 * Update/Writing functions
 */

/*!
 * @brief	otp_Set_Security_Level
 * @details	This routine writes the provided Security level value to OTP
 *		If Security level field was already written, the request is
 *		rejected
 * @param[in]	level - value to write
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Set_Security_Level(enum t_boot_security_level level);

/*!
 * @brief	otp_Set_Cust_ID:
 * @details	This routine writes the provided Customer ID value word to OTP
 *		If Security level field was already written, the request is
 *		rejected
 * @param[out]	cid - v lue to write
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Set_Cust_ID(t_uint32 cid);

/*!
 * @brief	otp_Incr_Cust_Cert_Ver
 * @details	This routine increments the Customer Certificate Version word
 *		in OTP
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Incr_Cust_Cert_Ver(void);

/*!
 * @brief	otp_Get_Secr_Ver:
 * @details	This routine increments the Security version value word in OTP
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Incr_Secr_Ver(void);

/*!
 * @brief	otp_Set_Boot_Parameters
 * @details	This routine writes the provided Boot parameters value to OTP
 *		If Boot info field was already written, the request is rejected
 * @param[in]	info - value to write
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Set_Boot_Parameters(volatile struct t_safemem_boot_cfg info);

/*!
 * @brief	otp_Set_Customer_Certificate_Hash_Info
 * @details	This routine write the provided customer root certificate hash
 *		information in OTP
 *		(ie. the type of hash used and if root customer certificate is
 *		programmed)
 * @param[in]	root_hash_info - 32b unsigned integer with the information
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Set_Customer_Certificate_Hash_Info(uint32_t root_hash_info);

/*!
 * @brief	otp_Set_Customer_Certificate_Hash
 * @details	This routine first check if the customer root certificate hash
 *		has been written and enabled
 *		If not, it writes the provided value in the OTP
 * @param[out]	phash - pointer to a byte array containing the value to be
 *		written - must be at least 64 bytes long (512bits)
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE - if customer hash was written already, or errors
 *		while reading OTP
 * @retval	: OTP_SUCCESS
 */
extern int otp_Set_Customer_Certificate_Hash(uint8_t *phash);

/*!
 * @brief	otp_write_encrypted_boot_key
 * @details	This routine writes the Encrypted Boot Key and configuration bits
 *		in OTP (present and size).
 *		In case the key was already written, an error will be returned
 * @param[in]	bitSize - unsigned integer telling the key size in bits
 *		(only 128 and 256 values are supported)
 * @param[in]	pSecBootKey - pointer to a byte array containing the key to
 *		write (could be 128 or 256 bits)
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE - if Encrypted Boot Key was not written already,
 *		or errors while accessing OTP
 * @retval	: OTP_SUCCESS
 */
extern int otp_write_encrypted_boot_key(uint32_t bitSize, uint8_t *pSecBootKey);

/*!
 * @brief	otp_Word_Set_Ecc_Public_Word
 * @details	This routine configure the chunk enclosing the specified word
 *		as ECC protected - only if word_address belongs to the public
 *		range
 * @param	: word_addr: progressive word address
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE - if error while accessing OTP or word_addr not
 *		in the public word range
 * @retval	: OTP_SUCCESS
 */
extern int otp_Word_Set_Ecc_Public_Word(uint32_t word_addr);

/*!
 * @brief	otp_Word_Get_Ecc_Public_Word
 * @details	This routine returns the configuration status of the chunk
 *		enclosing the specified word - only if word_address belongs to
 *		the public range
 * @param	: word_addr: progressive word address
 * @param	: p_ecc_set: pointer to unsigned int (will be set to 1 if word
 *		has ECC, 0 if not )
 *		(if word_addr is not valid, p_ecc_set is not updated)
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE - if error while accessing OTP or word_addr not
 *		in the public word range
 * @retval	: OTP_SUCCESS
 */
extern int otp_Word_Get_Ecc_Public_Word(uint32_t word_addr,
					uint32_t *p_ecc_set);

/*!
 * @brief	otp_Set_monotonic_counter_selected_type:
 * @details	This routine writes the provided selected type for monotonic
 *		counter (0 -> ram, 1 -> otp, 2 -> NOR SQI, 3 -> eMMC )
 * @param[in]	type - value to write
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int  otp_Set_monotonic_counter_selected_type(t_uint32 type);

/*!
 * @brief	otp_Get_monotonic_counter_selected_type:
 * @details	This routine returns the curent selected type for monotonic
 *		counter (0 -> ram, 1 -> otp, 2 -> NOR SQI, 3 -> eMMC )
 *		If an error occur in the HSM service request, the returned
 *		variable is not updated
 * @param[out]	ptype - pointer to a 32b unsigned integer variable where the
 *		value will be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	:  OTP_SUCCESS
 */
extern int otp_Get_monotonic_counter_selected_type(t_uint32 *ptype);

/*!
 * @brief	otp_Get_monotonic_counter_value:
 * @details	This routine returns the current value for otp monotonic counter
 *		if selected
 *		If an error occur in the HSM service request or current selected
 *		monotonic counter is not the otp one, the returned variable is
 *		not updated
 * @param[out]	pcounter - pointer to a 32b unsigned integer variable where the
 *		value will be written
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Get_monotonic_counter_value(t_uint32 *pcounter);

/*!
 * @brief	otp_Incr_monotonic_counter_value:
 * @details	This routine incrments the current value for otp monotonic
 *		counter if selected
 *		If an error occur in the HSM service request or current selected
 *		monotonic counter is not the otp one, the request is rejected
 * @return	: OTP error condition
 * @retval	: OTP_FAILURE
 * @retval	: OTP_SUCCESS
 */
extern int otp_Incr_monotonic_counter_value(void);
/**
 * @}
 * @}
 */
