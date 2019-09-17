//!
//!  \file 		tunerdriver_types.h
//!  \brief 	<i><b> CMOST driver API layer </b></i>
//!  \details   Type definitions for the ETAL user application
//!  $Author$
//!  \author 	(original version) Jean-Hugues Perrin
//!  $Revision$
//!  $Date$
//!

#ifndef TUNERDRIVER_TYPES_H
#define TUNERDRIVER_TYPES_H

// Maximum size of bus name
#define MAX_SIZE_BUS_NAME					64

/**************************************
 * Types
 *************************************/
/*
 * This section is provided for application builds that normally
 * do not use ETAL's OSAL (Operating System Abstraction Layer).
 * It defines the minimal set of OSAL types needed to build
 * TUNER DRIVER.
 */
#ifndef OSAL_PLAIN_TYPES_DEFINED
#define OSAL_PLAIN_TYPES_DEFINED
// TKernel exception leading to warning :
// put tVoid as void define
#ifdef CONFIG_HOST_OS_TKERNEL
#define tVoid					void
#else
typedef void                    tVoid;
#endif

typedef unsigned char           tBool;
typedef unsigned char           tU8;
typedef unsigned short          tU16;
typedef short                   tS16;
typedef unsigned int            tU32;
typedef int                     tS32;
typedef int                     tSInt;
typedef char                    tChar;
#define OSAL_OK                 0
#define OSAL_ERROR              -1
#endif

#define IP_ADDRESS_CHARLEN         20
#define DEFAULT_IP_ADDRESS         "127.0.0.1"

/*
 * The GPIO CS value when the TRUE SPI is used.
 */
#define ETAL_CS_TRUE_SPI			0xFFFFFFFF

/*!
 * \enum	tyCommunicationBusType
 * 			Describes the type of communication bus used by a hardware device.
 */
typedef enum
{
	BusI2C,
	BusSPI
} tyCommunicationBusType;

/*!
 * \enum	tySPIMode
 * 			Describes the SPI mode
 */
typedef enum
{
	SPI_CPHA0_CPOL0 = 0,
	SPI_CPHA1_CPOL1 = 3,
} tySPIMode;

/*!
 * \struct	tySpiCommunicationBus
 * 			Describes the bus communication type
 */
typedef struct
{
	/*! Name of the SPI bus */
	tChar 			busName[MAX_SIZE_BUS_NAME];
	/*! GPIO used to control the SPI Chip Select signal */
	/*! Use ETAL_CS_TRUE_SPI as value for a TRUE SPI configuration */
	tU32			GPIO_CS;
	/*! mode of the SPI bus */
	tySPIMode		mode;
	/*! speed of the SPI bus */
	tU32			speed;
} tySpiCommunicationBus;

/*!
 * \struct	tyI2cCommunicationBus
 * 			Describes the bus communication type
 */
typedef struct
{
	/*! Name of the I2C bus */
	tChar 			busName[MAX_SIZE_BUS_NAME];
	/*! Address of the device */
	tU32			deviceAddress;
} tyI2cCommunicationBus;

/*!
 * \struct	tyCMOSTDeviceConfiguration
 * 			Describes the CMOST device configuration
 */
typedef struct
{
	/*! Communication bus type (SPI or I2C) */
	tyCommunicationBusType 		communicationBusType;
	/*! SPI or I2C bus configuration */
	union
	{
		/*! SPI bus configuration */
		tySpiCommunicationBus	spi;
		/*! I2C bus configuration */
		tyI2cCommunicationBus	i2c;
	} communicationBus;
	/*! GPIO where the CMOST DCOP's RSTN line is connected */
	tU32 						GPIO_RESET;
	/*! GPIO where the CMOST DCOP's IRQ line is connected */
	tU32 						GPIO_IRQ;
	/*! Callback function called when an IRQ occurs */
	tVoid						(*IRQCallbackFunction)(tVoid);
} tyCMOSTDeviceConfiguration;

#endif // TUNERDRIVER_TYPES_H
