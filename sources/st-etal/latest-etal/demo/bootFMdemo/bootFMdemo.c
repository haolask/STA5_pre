//!
//!  \file 		bootFMdemo.c
//!  \brief 	<i><b> TUNER DRIVER library demo </b></i>
//!  \details   Demo TUNER DRIVER library application
//!  \author 	Raffaele Belardi
//!

/*
 * This demo application shows how to use the TUNER DRIVER library to 
 * download the Firmware or Patches to the CMOST tuner and
 * how to use the CMOST library to issue some simple commands
 * to the CMOST tuner.
 *
 * Tested on Accordo2/Linux board with CMOST STAR-S module
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>  // usleep
#include <sys/time.h> // gettimeofday

#include "stm_types.h"
#include "tunerdriver.h"
#include <windows.h>

/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/
/*
 * OS configuration
 */
#define CONFIG_HOST_OS_WIN32 1
#undef CONFIG_HOST_OS_LINUX
#undef CONFIG_HOST_OS_TKERNEL

/*
 * Hardware configuration
 */
#define CMOST_TUNER_ID_0				0
#define CMOST_TUNER_ID_1				1
#define CMOST_TUNER_DEVICE_TYPE       	BOOT_TUNER_STAR_T

/*
 * USE_MTD : Special case, verify the ability to load two devices
 */
#define USE_MTD

/*****************/
/*   HD RADIO    */
/*****************/
	/*********************/
	/* SPI configuration */
	/*********************/


	#ifdef CONFIG_COMM_HDRADIO_SPI_CPHA0_CPOL0
		#define HDRADIO_ACCORDO2_SPI_MODE		  			(SPI_CPHA0_CPOL0)
	#elif defined (CONFIG_COMM_HDRADIO_SPI_CPHA1_CPOL1)
		#define HDRADIO_ACCORDO2_SPI_MODE		  			(SPI_CPHA1_CPOL1)
	#endif // CONFIG_COMM_HDRADIO_SPI_CPHA0_CPOL0

	#ifdef CONFIG_HOST_OS_TKERNEL
		/***************/
		/* OS TKERNEL  */
		/***************/
		#define HDRADIO_ACCORDO2_SPI_SPEED					3000000
		#define HDRADIO_ACCORDO2_SPI_SPEED_LOAD_PHASE1      2000000
		#define HDRADIO_ACCORDO2_SPI_SPEED_FLASH  			4000000
		#define HDRADIO_ACCORDO2_SPI_SPEED_LOAD_SDRAM       3000000

		#ifdef CONFIG_BOARD_ACCORDO2_CUSTOM1
		 	// no HD configuration for that customer

		#elif defined(CONFIG_BOARD_ACCORDO2_CUSTOM2) //!CONFIG_BOARD_ACCORDO2_CUSTOM1
		 	#define HDRADIO_ACCORDO2_RESET_GPIO      		95

			// SPI defines if SPI configuration
			#define HDRADIO_ACCORDO2_SPI_DEVICE 	 		"SSPb"
		 	#define HDRADIO_ACCORDO2_CS_GPIO         		30

			// I2C defines if I2C configuration
			// for now not a defined configuration for that customer
			//#define HDRADIO_ACCORDO2_I2C_DEVICE           	"IICa"
			//#define HDRADIO_ACCORDO2_I2C_ADDRESS     		(tU8)0x2E

		#else //!CONFIG_BOARD_ACCORDO2_CUSTOM1 && !CONFIG_BOARD_ACCORDO2_CUSTOM2

			// SPI defines if SPI configuration
			#if defined(CONFIG_BOARD_ACCORDO5)
				#define HDRADIO_ACCORDO2_RESET_GPIO 		   10
				#define HDRADIO_ACCORDO2_SPI_DEVICE 	 		"SSPc"
				#define HDRADIO_ACCORDO2_CS_GPIO         		26
				// I2C defines if I2C configuration
				#define HDRADIO_ACCORDO2_I2C_DEVICE           	"IICa"
				#define HDRADIO_ACCORDO2_I2C_ADDRESS     		((tU8)0x2E)

			#else // ACCORDO 2
				#define HDRADIO_ACCORDO2_RESET_GPIO      		20

				#ifdef __SETTING_TEB__
				#define HDRADIO_ACCORDO2_SPI_DEVICE 	 		"SSPb"
				#define HDRADIO_ACCORDO2_CS_GPIO         		30
				#else //!__SETTING_TEB__
					#define HDRADIO_ACCORDO2_SPI_DEVICE 	 	"SSPa"
					#define HDRADIO_ACCORDO2_CS_GPIO         	13
			 	 #endif //__SETTING_TEB__

				// I2C defines if I2C configuration
				#define HDRADIO_ACCORDO2_I2C_DEVICE           	"IICa"
				#define HDRADIO_ACCORDO2_I2C_ADDRESS     		((tU8)0x2E)
			#endif // ACCORDO 5 / 2
		#endif //CONFIG_BOARD_ACCORDO2_CUSTOM1
    #elif defined(CONFIG_HOST_OS_WIN32)
		/***************/
		/* OS WINDOWS  */
		/***************/
		#define HDRADIO_ACCORDO2_SPI_SPEED     				0
		#define HDRADIO_ACCORDO2_SPI_SPEED_LOAD_PHASE1		0
		#define HDRADIO_ACCORDO2_SPI_SPEED_FLASH			0
		#define HDRADIO_ACCORDO2_SPI_SPEED_LOAD_SDRAM		0
		#define HDRADIO_ACCORDO2_SPI_DEVICE		        	"NA"
		#define HDRADIO_ACCORDO2_RESET_GPIO					0
		#define HDRADIO_ACCORDO2_CS_GPIO					0
		#define HDRADIO_ACCORDO2_I2C_DEVICE		        	"NA"
		#define HDRADIO_ACCORDO2_I2C_ADDRESS				(tU8)0x2E
	#else
		/*************/
		/* OS LINUX  */
		/*************/
		#define HDRADIO_ACCORDO2_SPI_SPEED					2000000
		#define HDRADIO_ACCORDO2_SPI_SPEED_LOAD_PHASE1      2000000
		#define HDRADIO_ACCORDO2_SPI_SPEED_FLASH  			3500000
		#define HDRADIO_ACCORDO2_SPI_SPEED_LOAD_SDRAM       2000000

		#ifdef CONFIG_BOARD_ACCORDO5
			#define HDRADIO_ACCORDO2_RESET_GPIO 		   10

			// SPI defines if SPI configuration
		 	#define HDRADIO_ACCORDO2_SPI_DEVICE 	   		"spidev32766.0"
		 	#define HDRADIO_ACCORDO2_CS_GPIO		  		26

			// I2C defines if I2C configuration
			#define HDRADIO_ACCORDO2_I2C_DEVICE 			"i2c-0"
			#define HDRADIO_ACCORDO2_I2C_ADDRESS			(tU8)0x2E
		#else //!CONFIG_BOARD_ACCORDO5
			 #define HDRADIO_ACCORDO2_RESET_GPIO      		20

			 // SPI defines if SPI configuration
		 	 #define HDRADIO_ACCORDO2_SPI_DEVICE        	"spidev32766.0"
		 	 #define HDRADIO_ACCORDO2_CS_GPIO           	13

		 	 // I2C defines if I2C configuration
			#define HDRADIO_ACCORDO2_I2C_DEVICE 			"i2c-0"
			#define HDRADIO_ACCORDO2_I2C_ADDRESS			(tU8)0x2E
		#endif //CONFIG_BOARD_ACCORDO5

		// no HD defined for CUSTOMER 1
		// no Linux defined for CUSTOMER 2
	#endif //CONFIG_HOST_OS_TKERNEL


/*****************/
/*   DAB RADIO    */
/*****************/
    /*********************/
    /* SPI configuration */
    /*********************/
    #ifdef CONFIG_HOST_OS_TKERNEL
        /***************/
        /* OS TKERNEL  */
        /***************/
        #define DAB_ACCORDO2_SPI_SPEED_NORMAL_MODE          2560000
        #define DAB_ACCORDO2_SPI_SPEED_FLASH_MODE           2000000
        #define DAB_ACCORDO2_SPI_MODE                       SPI_CPHA1_CPOL1
        #define DAB_ACCORDO2_IS_BOOT_MODE                   FALSE

		#ifdef CONFIG_BOARD_ACCORDO5
		 	#define DAB_ACCORDO2_SPI_DEVICE				 		"SSPc"

			#define  DAB_ACCORDO2_RESET_GPIO                	10
	        #define  DAB_ACCORDO2_CS_GPIO                   	26
	        #define  DAB_ACCORDO2_REQ_GPIO                  	33
	        #define  DAB_ACCORDO2_BOOT_GPIO                 	14
		#else // ACCORDO 2
	        #ifdef __SETTING_TEB__
	            #define DAB_ACCORDO2_SPI_DEVICE                 "SSPb"
	            #define DAB_ACCORDO2_CS_GPIO                    30
	        #else
	            #define DAB_ACCORDO2_SPI_DEVICE                 "SSPa"
	            #define DAB_ACCORDO2_CS_GPIO                    13
	        #endif

	        #define DAB_ACCORDO2_RESET_GPIO                     20
	        #define DAB_ACCORDO2_REQ_GPIO                       21
	        #define DAB_ACCORDO2_BOOT_GPIO                      10
		#endif // ACCORDO 5 / ACCORDO 2

    #elif defined(CONFIG_HOST_OS_FREERTOS)

        #define DAB_ACCORDO2_SPI_SPEED_NORMAL_MODE          0
        #define DAB_ACCORDO2_SPI_SPEED_FLASH_MODE           0
        #define DAB_ACCORDO2_SPI_MODE                       SPI_CPHA1_CPOL1
        #define DAB_ACCORDO2_IS_BOOT_MODE                   FALSE

        #ifdef CONFIG_BOARD_ACCORDO2_SPI0_IS_0
            #define DAB_ACCORDO2_SPI_DEVICE                 ""
        #else
            #define DAB_ACCORDO2_SPI_DEVICE                 ""
        #endif

        #ifdef CONFIG_BOARD_ACCORDO5
            #define  DAB_ACCORDO2_RESET_GPIO                10
            #define  DAB_ACCORDO2_CS_GPIO                   26
            #define  DAB_ACCORDO2_REQ_GPIO                  33
            #define  DAB_ACCORDO2_BOOT_GPIO                 14
        #else //!CONFIG_BOARD_ACCORDO5
            #ifdef CONFIG_BOARD_ACCORDO2_CUSTOM1
                #define  DAB_ACCORDO2_RESET_GPIO            4
                #define  DAB_ACCORDO2_CS_GPIO               28
                #define  DAB_ACCORDO2_REQ_GPIO              3
                #define  DAB_ACCORDO2_BOOT_GPIO             14
            #else //!CONFIG_BOARD_ACCORDO2_CUSTOM1
                #define  DAB_ACCORDO2_RESET_GPIO            20
                #define  DAB_ACCORDO2_CS_GPIO               13
                #define  DAB_ACCORDO2_REQ_GPIO              21
                #define  DAB_ACCORDO2_BOOT_GPIO             10
            #endif //CONFIG_BOARD_ACCORDO2_CUSTOM1
        #endif //CONFIG_BOARD_ACCORDO5
    #elif defined(CONFIG_HOST_OS_WIN32)
		/***************/
		/* OS WINDOWS  */
		/***************/
		#define DAB_ACCORDO2_SPI_SPEED_NORMAL_MODE     		0
		#define DAB_ACCORDO2_SPI_SPEED_FLASH_MODE			0
		#define DAB_ACCORDO2_SPI_MODE		  				0
		#define DAB_ACCORDO2_IS_BOOT_MODE     				0
		#define DAB_ACCORDO2_SPI_DEVICE			        	"NA"
		#define DAB_ACCORDO2_RESET_GPIO						0
		#define DAB_ACCORDO2_CS_GPIO						0
		#define DAB_ACCORDO2_REQ_GPIO						0
     	#define DAB_ACCORDO2_BOOT_GPIO  	 				0
    #else
        /*************/
        /* OS LINUX  */
        /*************/
        #define DAB_ACCORDO2_SPI_SPEED_NORMAL_MODE          8533333
        #define DAB_ACCORDO2_SPI_SPEED_FLASH_MODE           2000000
        #define DAB_ACCORDO2_SPI_MODE                       SPI_CPHA1_CPOL1
        #define DAB_ACCORDO2_IS_BOOT_MODE                   FALSE

        #ifdef CONFIG_BOARD_ACCORDO2_SPI0_IS_0
            #define DAB_ACCORDO2_SPI_DEVICE                 "spidev0.0"
        #else
            #define DAB_ACCORDO2_SPI_DEVICE                 "spidev32766.0"
        #endif

        #ifdef CONFIG_BOARD_ACCORDO5
            #define  DAB_ACCORDO2_RESET_GPIO                10
            #define  DAB_ACCORDO2_CS_GPIO                   26
            #define  DAB_ACCORDO2_REQ_GPIO                  33
            #define  DAB_ACCORDO2_BOOT_GPIO                 14
        #else //!CONFIG_BOARD_ACCORDO5
            #ifdef CONFIG_BOARD_ACCORDO2_CUSTOM1
                #define  DAB_ACCORDO2_RESET_GPIO            4
                #define  DAB_ACCORDO2_CS_GPIO               28
                #define  DAB_ACCORDO2_REQ_GPIO              3
                #define  DAB_ACCORDO2_BOOT_GPIO             14
            #else //!CONFIG_BOARD_ACCORDO2_CUSTOM1
                #define  DAB_ACCORDO2_RESET_GPIO            20
                #define  DAB_ACCORDO2_CS_GPIO               13
                #define  DAB_ACCORDO2_REQ_GPIO              21
                #define  DAB_ACCORDO2_BOOT_GPIO             10
            #endif //CONFIG_BOARD_ACCORDO2_CUSTOM1
        #endif //CONFIG_BOARD_ACCORDO5

        // no HD defined for CUSTOMER 1
        // no Linux defined for CUSTOMER 2
    #endif //CONFIG_HOST_OS_TKERNEL


/*****************/
/*     CMOST     */
/*****************/

	/*********************/
	/* SPI configuration */
	/*********************/
	#ifdef CONFIG_HOST_OS_TKERNEL
		/***************/
		/* OS TKERNEL  */
		/***************/
		#define CMOST_ACCORDO2_SPI_SPEED					3000000
		#define CMOST_ACCORDO2_SPI_MODE		  				SPI_CPHA1_CPOL1

		#ifdef CONFIG_BOARD_ACCORDO2_CUSTOM1
			// customer is not in TKERNEL

		#elif defined (CONFIG_BOARD_ACCORDO2_CUSTOM2)
					// T1

			// RESET & IRQ PINs
			#define CMOST_T1_ACCORDO2_RESET_GPIO			96
			#define CMOST_T1_ACCORDO2_IRQ_GPIO				4

			// SPI defines if SPI configuration
			#define CMOST_T1_ACCORDO2_SPI_DEVICE      		"SSPa"

			#ifdef	CONFIG_BOARD_ACCORDO2_CMOST_1_TRUE_SPI
			#define CMOST_T1_ACCORDO2_SPI_CS_GPIO      		ETAL_CS_TRUE_SPI
			#else
			#define CMOST_T1_ACCORDO2_SPI_CS_GPIO	 		13
			#endif

			// I2C defines if I2C configuration
			// not tested for that customer
			//
			//#define CMOST_ACCORDO2_I2C_DEVICE 			"IICb"
			//#define CMOST_T1_ACCORDO2_I2C_ADDRESS 			0xC2

					// T2
			#define CMOST_T2_ACCORDO2_RESET_GPIO			97
			#define CMOST_T2_ACCORDO2_IRQ_GPIO				47

			// SPI defines if SPI configuration
			#define CMOST_T2_ACCORDO2_SPI_DEVICE      		"SSPb"
			#define CMOST_T2_ACCORDO2_SPI_CS_GPIO	 		30

			// I2C defines if I2C configuration
			// not tested for that customer
			//
			//#define CMOST_T2_ACCORDO2_I2C_ADDRESS			0xC8

		#else
			#ifdef CONFIG_BOARD_ACCORDO5
				// T1
				// RESET & IRQ PINs
				#define CMOST_T1_ACCORDO2_RESET_GPIO				8
				#define CMOST_T1_ACCORDO2_IRQ_GPIO					7

				// I2C defines if I2C configuration
				// device common for T1 & T2
				// note EPR : j'ai un doute : il se peut que ce soit IICa
				#define CMOST_ACCORDO2_I2C_DEVICE				"IICb"
				#define CMOST_T1_ACCORDO2_I2C_ADDRESS			0xC2

				// T2
				// This is the MTD case : reset PIN shared only 1 IRQ
				// RESET & IRQ PINs
				#define CMOST_T2_ACCORDO2_RESET_GPIO			8
				#define CMOST_T2_ACCORDO2_IRQ_GPIO				7

				// I2C defines if I2C configuration
				// common for T1 & T2
				//#define CMOST_ACCORDO2_I2C_DEVICE				"iIICb"
				#define CMOST_T2_ACCORDO2_I2C_ADDRESS  			0xC8

			#else // CONFIG_BOARD_ACCORDO2
						// T1

				// RESET & IRQ PINs
				#define CMOST_T1_ACCORDO2_RESET_GPIO			198
				#define CMOST_T1_ACCORDO2_IRQ_GPIO				199

				// SPI defines if SPI configuration
				#define CMOST_T1_ACCORDO2_SPI_DEVICE      		"SSPb"
				#define CMOST_T1_ACCORDO2_SPI_CS_GPIO	 		30

				// I2C defines if I2C configuration
				// device common for T1 & T2
				#define CMOST_ACCORDO2_I2C_DEVICE				"IICb"
				#define CMOST_T1_ACCORDO2_I2C_ADDRESS			0xC2

						// T2
				// This is the MTD case : reset PIN shared only 1 IRQ
				// RESET & IRQ PINs
				#define CMOST_T2_ACCORDO2_RESET_GPIO			198
				#define CMOST_T2_ACCORDO2_IRQ_GPIO				199

				// SPI defines if SPI configuration
				#define CMOST_T2_ACCORDO2_SPI_DEVICE      		"SSPa"
				#define CMOST_T2_ACCORDO2_SPI_CS_GPIO	 		13

				// I2C defines if I2C configuration
				// device common for T1 & T2
				//#define CMOST_ACCORDO2_I2C_DEVICE				"IICb"
				#define CMOST_T2_ACCORDO2_I2C_ADDRESS			0xC8
			#endif // CONFIG_BOARD_ACCORDO5 // CONFIG_BOARD_ACCORDO2
		#endif // CONFIG_BOARD_ACCORDO2_CUSTOM1

	#elif defined(CONFIG_HOST_OS_FREERTOS)
		/****************/
		/* OS FREERTOS  */
		/****************/
		#define CMOST_ACCORDO2_SPI_SPEED				0
		#define CMOST_ACCORDO2_SPI_MODE		  			SPI_CPHA1_CPOL1

		#ifdef CONFIG_BOARD_ACCORDO2_CUSTOM1
			// customer single tuner configuration & I2C
			// RESET & IRQ PINs
			#define CMOST_T1_ACCORDO2_RESET_GPIO        		7
			#define CMOST_T1_ACCORDO2_IRQ_GPIO			5

			#define CMOST_ACCORDO2_I2C_DEVICE   			"i2c-0"

			#define CMOST_T1_ACCORDO2_I2C_ADDRESS  			0xC8

			// SPI not defined
			// T2 not defined

		#elif defined (CONFIG_BOARD_ACCORDO2_CUSTOM2)
			// customer is not in Linux

		#else
			#ifdef CONFIG_BOARD_ACCORDO5
				// for now SPI not defined in A5 :
				// ie T1 + T2 in I2C only
				//

					// T1
				// RESET & IRQ PINs
				#define CMOST_T1_ACCORDO2_RESET_GPIO        	8
				#define CMOST_T1_ACCORDO2_IRQ_GPIO		7

					// SPI defines if SPI configuration
					// not tested in Linux yet
	//			#define CMOST_T1_ACCORDO2_SPI_DEVICE      	"spidev32766.0"
	//			#define CMOST_T1_ACCORDO2_SPI_CS_GPIO	 	30

				// I2C defines if I2C configuration
				// common for T1 & T2
				#define CMOST_ACCORDO2_I2C_DEVICE     		"i2c-1"
				#define CMOST_T1_ACCORDO2_I2C_ADDRESS  		0xC2

					// T2
				// This is the MTD case : reset PIN shared only 1 IRQ
				// RESET & IRQ PINs
				#define CMOST_T2_ACCORDO2_RESET_GPIO		8
				#define CMOST_T2_ACCORDO2_IRQ_GPIO		7

				// SPI defines if SPI configuration
				// not tested in Linux yet
	//			#define CMOST_T2_ACCORDO2_SPI_DEVICE      	"spidev32766.0"
	//			#define CMOST_T2_ACCORDO2_SPI_CS_GPIO	 	13


				// I2C defines if I2C configuration
				// common for T1 & T2
				//#define CMOST_ACCORDO2_I2C_DEVICE		"i2c-1"
				#define CMOST_T2_ACCORDO2_I2C_ADDRESS  		0xC8

			#else // !CONFIG_BOARD_ACCORDO5
				// for now SPI not defined in A2 Linux :
				// ie T1 + T2 in I2C only
				//
								// T1
				#define CMOST_T1_ACCORDO2_IRQ_GPIO		167
				#define CMOST_T1_ACCORDO2_RESET_GPIO        	166

				// SPI not defines if SPI configuration
				// #define CMOST_T1_ACCORDO2_SPI_DEVICE      	"spidev32766.0"
				// #define CMOST_T1_ACCORDO2_SPI_CS_GPIO	30

				// I2C defines if I2C configuration
				// common for T1 & T2
				#define CMOST_ACCORDO2_I2C_DEVICE 		"i2c-1"
				#define CMOST_T1_ACCORDO2_I2C_ADDRESS		0xC2

						// T2
				// This is the MTD case : reset PIN shared only 1 IRQ
				#define CMOST_T2_ACCORDO2_IRQ_GPIO		167
				#define CMOST_T2_ACCORDO2_RESET_GPIO		166

				// SPI defines if SPI configuration
				//#define CMOST_T2_ACCORDO2_SPI_DEVICE      	"spidev32766.0"
				//#define CMOST_T2_ACCORDO2_SPI_CS_GPIO	 	13

				// I2C defines if I2C configuration
				// common for T1 & T2
				//#define CMOST_ACCORDO2_I2C_DEVICE		"i2c-1"
				#define CMOST_T2_ACCORDO2_I2C_ADDRESS  		0xC8


			#endif // CONFIG_BOARD_ACCORDO2_CUSTOM1
		#endif // CONFIG_BOARD_ACCORDO5

	#elif defined(CONFIG_HOST_OS_WIN32)
		/***************/
		/* OS WINDOWS  */
		/***************/
		#define CMOST_ACCORDO2_I2C_DEVICE     			"NA"
		#define CMOST_ACCORDO2_SPI_SPEED				0
		#define CMOST_ACCORDO2_SPI_MODE		  			0

		#define CMOST_ACCORDO5_I2C_DEVICE     			"NA"
		#define CMOST_ACCORDO5_I2C_DEVICE_BASE_ADDRESS	0

		#define CMOST_T1_ACCORDO2_I2C_ADDRESS  			0xC2
		#define CMOST_T1_ACCORDO2_RESET_GPIO        	0
		#define CMOST_T1_ACCORDO2_IRQ_GPIO				0
		#define CMOST_T1_ACCORDO2_SPI_CS_GPIO			0
        #define CMOST_T1_ACCORDO2_MISO_GPIO 			0
		#define CMOST_T1_ACCORDO2_SPI_DEVICE			"NA"

		#define CMOST_T2_ACCORDO2_I2C_ADDRESS  			0xC8
		#define CMOST_T2_ACCORDO2_RESET_GPIO			0
		#define CMOST_T2_ACCORDO2_IRQ_GPIO				0
		#define CMOST_T2_ACCORDO2_SPI_CS_GPIO			0
     	#define CMOST_T2_ACCORDO2_MISO_GPIO 			0
		#define CMOST_T2_ACCORDO2_SPI_DEVICE			"NA"
	#else
		/*************/
		/* OS LINUX  */
		/*************/
		#define CMOST_ACCORDO2_SPI_SPEED					3000000
		#define CMOST_ACCORDO2_SPI_MODE		  				SPI_CPHA1_CPOL1

		#ifdef CONFIG_BOARD_ACCORDO2_CUSTOM1
			// customer single tuner configuration & I2C
			// RESET & IRQ PINs
			#define CMOST_T1_ACCORDO2_RESET_GPIO        	7
			#define CMOST_T1_ACCORDO2_IRQ_GPIO				5

			#define CMOST_ACCORDO2_I2C_DEVICE   			"i2c-0"

			#define CMOST_T1_ACCORDO2_I2C_ADDRESS  			0xC8

			// SPI not defined
			// T2 not defined

		#elif defined (CONFIG_BOARD_ACCORDO2_CUSTOM2)
			// customer is not in Linux

		#else
			#ifdef CONFIG_BOARD_ACCORDO5
				// for now SPI not defined in A5 :
				// ie T1 + T2 in I2C only
				//



					// T1
				// RESET & IRQ PINs
				#define CMOST_T1_ACCORDO2_RESET_GPIO        	8
				#define CMOST_T1_ACCORDO2_IRQ_GPIO				7

					// SPI defines if SPI configuration
					// not tested in Linux yet
					// T1
				// SPI defines if SPI configuration
				// it should be : "spidev32765.0" if SPI1 is used
				// #define CMOST_T1_ACCORDO2_SPI_DEVICE			"spidev32765.0"
				// if DCOP SPI is used / SPI2 :
				#define CMOST_T1_ACCORDO2_SPI_DEVICE			"spidev32766.0"

#ifdef	CONFIG_BOARD_ACCORDO2_CMOST_1_TRUE_SPI
				#define CMOST_T1_ACCORDO2_SPI_CS_GPIO			ETAL_CS_TRUE_SPI
#else
				// it should be : "62" if SPI1 is used
				// #define CMOST_T1_ACCORDO2_SPI_CS_GPIO			62
				//  if DCOP SPI is used / SPI2 :
				#define CMOST_T1_ACCORDO2_SPI_CS_GPIO			26
#endif

				// it should be : "36" if SPI1 is used
				//#define CMOST_T1_ACCORDO2_MISO_GPIO 			36
				//	if DCOP SPI is used / SPI2 :
				#define CMOST_T1_ACCORDO2_MISO_GPIO 			20

				// I2C defines if I2C configuration
				// common for T1 & T2
				// TMP CHANGE : dynamic I2C configuration
#if 1
				#define CMOST_ACCORDO5_I2C_DEVICE     			"i2c-2"
				#define CMOST_ACCORDO5_I2C_DEVICE_BASE_ADDRESS	0x50040000
#else
				#define CMOST_ACCORDO2_I2C_DEVICE     			"i2c-1"
#endif
				#define CMOST_T1_ACCORDO2_I2C_ADDRESS  			0xC2

					// T2
				// This is the MTD case : reset PIN shared only 1 IRQ
				// RESET & IRQ PINs
				#define CMOST_T2_ACCORDO2_RESET_GPIO			8
				#define CMOST_T2_ACCORDO2_IRQ_GPIO				7

				// SPI defines if SPI configuration
				// not tested in Linux yet
				#define CMOST_T2_ACCORDO2_SPI_DEVICE			"spidev32766.0"

#ifdef	CONFIG_BOARD_ACCORDO2_CMOST_1_TRUE_SPI
				#define CMOST_T2_ACCORDO2_SPI_CS_GPIO			ETAL_CS_TRUE_SPI
#else
				#define CMOST_T2_ACCORDO2_SPI_CS_GPIO			61
#endif


				// I2C defines if I2C configuration
				// common for T1 & T2
				//#define CMOST_ACCORDO2_I2C_DEVICE				"i2c-1"
				#define CMOST_T2_ACCORDO2_I2C_ADDRESS  			0xC8

			#else // !CONFIG_BOARD_ACCORDO5
				// for now SPI not defined in A2 Linux :
				// ie T1 + T2 in I2C only
				//
								// T1
				#define CMOST_T1_ACCORDO2_IRQ_GPIO				167
				#define CMOST_T1_ACCORDO2_RESET_GPIO        	166

				// SPI not defines if SPI configuration
				// #define CMOST_T1_ACCORDO2_SPI_DEVICE      		"spidev32766.0"
				// #define CMOST_T1_ACCORDO2_SPI_CS_GPIO	 		30

				// I2C defines if I2C configuration
				// common for T1 & T2
				#define CMOST_ACCORDO2_I2C_DEVICE 				"i2c-1"
				#define CMOST_T1_ACCORDO2_I2C_ADDRESS			0xC2

						// T2
				// This is the MTD case : reset PIN shared only 1 IRQ
				#define CMOST_T2_ACCORDO2_IRQ_GPIO				167
				#define CMOST_T2_ACCORDO2_RESET_GPIO			166

				// SPI defines if SPI configuration
				//#define CMOST_T2_ACCORDO2_SPI_DEVICE      		"spidev32766.0"
				//#define CMOST_T2_ACCORDO2_SPI_CS_GPIO	 		13

				// I2C defines if I2C configuration
				// common for T1 & T2
				//#define CMOST_ACCORDO2_I2C_DEVICE				"i2c-1"
				#define CMOST_T2_ACCORDO2_I2C_ADDRESS  			0xC8


			#endif // CONFIG_BOARD_ACCORDO2_CUSTOM1
		#endif // CONFIG_BOARD_ACCORDO5
	#endif //CONFIG_HOST_OS_TKERNEL

/*
 * After a change band it is necessary to allow some time
 * to the CMOST to stabilize before issuing a Tune command
 * This is normally done in ETAL but the CMOST driver knows
 * nothing of commands
 *
 * Units: msec
 */
#define CMOST_CHANGE_BAND_DELAY_MSEC 30

#define FM_LOW_FREQ               87500
#define FM_MID_FREQ               97800
#define FM_HIGH_FREQ             108000
#define BOOTFMDEMO_DELAY_T1_SEC       2
#define BOOTFMDEMO_DELAY_T2_SEC      10

#define USLEEP_ONE_MSEC            1000
#define USLEEP_ONE_SEC          1000000
#define USEC_MAX                1000000

#define SEEK_STATUS_RESPONSE_LEN        15
#define SEEK_STATUS_FULL_CYCLE        0x40
#define SEEK_STATUS_GOOD_STATION      0x80
#define SEEK_TIMEOUT_MS               1000
#define SEEK_WAIT_MS                    40


/*****************************************************************
| variables
|----------------------------------------------------------------*/

struct timeval startup_tv;

/***************************
 *
 * initLibrary
 *
 **************************/
static int initLibrary(void)
{
	tyCMOSTDeviceConfiguration CMOSTDeviceConfiguration[2];

#ifdef CONFIG_COMM_CMOST_SPI
	CMOSTDeviceConfiguration[0].communicationBusType = BusSPI;
	strncpy(CMOSTDeviceConfiguration[0].communicationBus.spi.busName, (tChar *)CMOST_T1_ACCORDO2_SPI_DEVICE, MAX_SIZE_BUS_NAME);
	CMOSTDeviceConfiguration[0].communicationBus.spi.GPIO_CS = CMOST_T1_ACCORDO2_CS_GPIO;
	CMOSTDeviceConfiguration[0].communicationBus.spi.mode = CMOST_ACCORDO2_SPI_MODE;
	CMOSTDeviceConfiguration[0].communicationBus.spi.speed = CMOST_ACCORD2_SPI_SPEED;
	CMOSTDeviceConfiguration[0].GPIO_RESET = CMOST_T1_ACCORDO2_RESET_GPIO;
	CMOSTDeviceConfiguration[0].GPIO_IRQ = CMOST_T1_ACCORDO2_IRQ_GPIO;
	CMOSTDeviceConfiguration[0].IRQCallbackFunction = NULL;

	CMOSTDeviceConfiguration[1].communicationBusType = BusSPI;
	strncpy(CMOSTDeviceConfiguration[1].communicationBus.spi.busName, (tChar *)CMOST_T2_ACCORDO2_SPI_DEVICE, MAX_SIZE_BUS_NAME);
	CMOSTDeviceConfiguration[1].communicationBus.spi.GPIO_CS = CMOST_T2_ACCORDO2_CS_GPIO;
	CMOSTDeviceConfiguration[1].communicationBus.spi.mode = CMOST_ACCORDO2_SPI_MODE;
	CMOSTDeviceConfiguration[1].communicationBus.spi.speed = CMOST_ACCORD2_SPI_SPEED;
	CMOSTDeviceConfiguration[1].GPIO_RESET = CMOST_T2_ACCORDO2_RESET_GPIO;
	CMOSTDeviceConfiguration[1].GPIO_IRQ = CMOST_T2_ACCORDO2_IRQ_GPIO;
	CMOSTDeviceConfiguration[1].IRQCallbackFunction = NULL;
#else
	CMOSTDeviceConfiguration[0].communicationBusType = BusI2C;
	strncpy(CMOSTDeviceConfiguration[0].communicationBus.i2c.busName, (tChar *)CMOST_ACCORDO2_I2C_DEVICE, MAX_SIZE_BUS_NAME);
	CMOSTDeviceConfiguration[0].communicationBus.i2c.deviceAddress = CMOST_T1_ACCORDO2_I2C_ADDRESS;
	CMOSTDeviceConfiguration[0].GPIO_RESET = CMOST_T1_ACCORDO2_RESET_GPIO;
	CMOSTDeviceConfiguration[0].GPIO_IRQ = CMOST_T1_ACCORDO2_IRQ_GPIO;
	#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
		CMOSTDeviceConfiguration[0].IRQCallbackFunction = IRQCallbackFunction;
	#else
		CMOSTDeviceConfiguration[0].IRQCallbackFunction = NULL;
	#endif

	CMOSTDeviceConfiguration[1].communicationBusType = BusI2C;
	strncpy(CMOSTDeviceConfiguration[1].communicationBus.i2c.busName, (tChar *)CMOST_ACCORDO2_I2C_DEVICE, MAX_SIZE_BUS_NAME);
	CMOSTDeviceConfiguration[1].communicationBus.i2c.deviceAddress = CMOST_T2_ACCORDO2_I2C_ADDRESS;
	CMOSTDeviceConfiguration[1].GPIO_RESET = CMOST_T2_ACCORDO2_RESET_GPIO;
	CMOSTDeviceConfiguration[1].GPIO_IRQ = CMOST_T2_ACCORDO2_IRQ_GPIO;
	#ifdef CONFIG_COMM_ENABLE_RDS_IRQ
		CMOSTDeviceConfiguration[1].IRQCallbackFunction = IRQCallbackFunction;
	#else
		CMOSTDeviceConfiguration[1].IRQCallbackFunction = NULL;
	#endif
#endif

	if (TUNERDRIVER_system_init() != 0)
	{
		printf("Error TUNERDRIVER_system_init\n");
		return 1;
	}

	/*
	 * Performs some library initialization.
	 */
	if (TUNERDRIVER_init(CMOST_TUNER_ID_0, &CMOSTDeviceConfiguration[CMOST_TUNER_ID_0]) != 0)
	{
		printf("Error TUNERDRIVER_init CMOST_TUNER_ID_0\n");
		return 1;
	}

#ifdef USE_MTD
	if (TUNERDRIVER_init(CMOST_TUNER_ID_1, &CMOSTDeviceConfiguration[CMOST_TUNER_ID_1]) != 0)
	{
		printf("Error TUNERDRIVER_init CMOST_TUNER_ID_1\n");
		return 1;
	}
#endif
	return 0;
}


/***************************
 *
 * initCMOST
 *
 **************************/
static int initCMOST(void)
{
	/*
	 * Issue a harware RESET to the CMOST using the TUNERDRIVER libray function
	 *
	 * Call once per each CMOST tuner device in the system with independent RESET line
	 * Note that on Accordo2 there is only one reset line to the CMOST module
	 * so resetting one of the modules automatically resets also the other(s), if present
	 */
	if (TUNERDRIVER_reset_CMOST(CMOST_TUNER_ID_0) != 0)
	{
		printf("Error TUNERDRIVER_reset_CMOST CMOST_TUNER_ID_0\n");
		return 1;
	}
#ifdef USE_MTD
	if (TUNERDRIVER_reset_CMOST(CMOST_TUNER_ID_1) != 0)
	{
		printf("Error TUNERDRIVER_reset_CMOST CMOST_TUNER_ID_1\n");
		return 1;
	}
#endif


	if (TUNERDRIVER_download_CMOST(CMOST_TUNER_ID_0, CMOST_TUNER_DEVICE_TYPE, NULL, 0, 1) != 0)
	{
        printf("Error TUNERDRIVER_download_CMOST CMOST_TUNER_ID_0\n");
		return 1;
	}
#ifdef USE_MTD
    if (TUNERDRIVER_download_CMOST(CMOST_TUNER_ID_1, CMOST_TUNER_DEVICE_TYPE, NULL, 0, 1) != 0)
    {
        printf("Error TUNERDRIVER_download_CMOST CMOST_TUNER_ID_1\n");
        return 1;
    }
#endif

	printf("CMOST initialization complete\n");
	return 0;
}

/***************************
 *
 * setTuneFrequencyParameter
 *
 **************************/
static void setTuneFrequencyParameter(unsigned char *cmd, tU32 freq)
{
	cmd[6] = (freq & 0xFF0000) >> 16;
	cmd[7] = (freq & 0x00FF00) >>  8;
	cmd[8] = (freq & 0x0000FF) >>  0;
}

/***************************
 *
 * OSAL_ClockGetElapsedTime
 *
 **************************/
/*
 * returns the current time in millisec
 */
unsigned int OSAL_ClockGetElapsedTime(void)
{
	struct timeval tv;
	int sec, usec;

	gettimeofday(&tv, NULL);
	if (startup_tv.tv_usec > tv.tv_usec)
	{
		tv.tv_sec--;
		tv.tv_usec += USEC_MAX;
	}
	sec = tv.tv_sec - startup_tv.tv_sec;
	usec = tv.tv_usec - startup_tv.tv_usec;
    return (sec * 1000) + (usec / 1000);
}

/***************************
 *
 * OSAL_ClockResetTime
 *
 **************************/
void OSAL_ClockResetTime(void)
{
	gettimeofday(&startup_tv, NULL);
}

/***************************
 *
 * seekTest
 *
 **************************/
static int seekTest(int *status)
{
	unsigned char resp[10];
	unsigned int resp_len;

	unsigned char CMOST_cmdSeekStart[] =     {0x00, 0x26, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01};
	unsigned char CMOST_cmdGetSeekStatus[] = {0x00, 0x28, 0x01, 0x00, 0x00, 0x01};
	unsigned char CMOST_cmdSeekStop[] =      {0x00, 0x27, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00};

	int start_time;
	int end_time;
	int seek_timeout_error;
	int stop_freq;

	/*
	 * [test] start FM seek up 
	 */
	printf("Seek start\n");
	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdSeekStart, sizeof(CMOST_cmdSeekStart), resp, &resp_len) != 0)
	{
		printf("Error starting seek\n");
		return 1;
	}

	/*
	 * [test] wait for seek to be finished
	 */
	start_time = OSAL_ClockGetElapsedTime();
	end_time = start_time + SEEK_TIMEOUT_MS;
	seek_timeout_error = 1;
	usleep(SEEK_WAIT_MS * USLEEP_ONE_MSEC);

	while (OSAL_ClockGetElapsedTime() < end_time)
	{
		if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdGetSeekStatus, sizeof(CMOST_cmdGetSeekStatus), resp, &resp_len) != 0)
		{
			printf("Error reading seek status\n");
			return 1;
		}
		if (resp_len < SEEK_STATUS_RESPONSE_LEN)
		{
			printf("Error reading seek status\n");
			return 1;
		}
		*status = resp[3] & (SEEK_STATUS_FULL_CYCLE | SEEK_STATUS_GOOD_STATION);
		if ((*status & (SEEK_STATUS_FULL_CYCLE | SEEK_STATUS_GOOD_STATION)) != 0)
		{
			seek_timeout_error = 0;
			break;
		}
		usleep(SEEK_WAIT_MS * USLEEP_ONE_MSEC);
	}

	stop_freq = (((int)resp[3] & 0x0F) << 16) + ((int)resp[4] << 8) + (int)resp[5];
	printf("Seek complete, status 0x%x, freq %d\n", *status, stop_freq);

	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdSeekStop, sizeof(CMOST_cmdSeekStop), resp, &resp_len) != 0)
	{
		printf("Error stopping seek\n");
		return 1;
	}

	if (seek_timeout_error)
	{
		printf("Error timeout during seek\n");
		return 1;
	}

	return 0;
}

/***************************
 *
 * RunFMTests
 *
 **************************/
static int RunFMTests(void)
{
	unsigned char resp[10];
	unsigned int resp_len;
	int status = 0;

	unsigned char CMOST_cmdSetBand[] =           {0x00, 0x0a, 0x06, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x55, 0xcc, 0x01, 0xa5, 0xe0, 0x00, 0x00, 0x64};
	unsigned char CMOST_cmdTune[] =              {0x00, 0x08, 0x03, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	//unsigned char CMOST_cmdSetSeekThresholds[] = {0x00, 0x29, 0x03, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	/*
	 * [test] set tuner to FM band, with limits set to FFM.LOW MHz and FFM.HIGH MHz, seek step set to FSTEP.FM kHz
	 * in this example:
	 * FFM.LOW =   87500
	 * FFM.HIGH = 108000
	 * FSTEP.FM =    100
	 */
	printf("Changing band to FM\n");
	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdSetBand, sizeof(CMOST_cmdSetBand), resp, &resp_len) != 0)
	{
		printf("Error changing band\n");
		return 1;
	}

	/*
 	* NOTE: After a change band it is necessary to allow some time
 	* to the CMOST to stabilize before issuing a Tune command
 	*/
	usleep(CMOST_CHANGE_BAND_DELAY_MSEC * USLEEP_ONE_MSEC);

	/*
	 * [test] set received frequency to FFM.LOW MHz
	 */
	printf("Tuning to %d\n", FM_LOW_FREQ);
	setTuneFrequencyParameter(CMOST_cmdTune, FM_LOW_FREQ);
	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdTune, sizeof(CMOST_cmdTune), resp, &resp_len) != 0)
	{
		printf("Error tuning frequency\n");
		return 1;
	}

	/*
	 * [test] wait T1
	 */
	Sleep(BOOTFMDEMO_DELAY_T1_SEC);

	/*
	 * [test] signal control interface that FFM.LOW MHz tests can start
	 */
	// ....

	/*
	 * [test] upon reception of "continue test" signal from control interface, set received frequency to FFM.HIGH MHz
	 */
	printf("Tuning to %d\n", FM_HIGH_FREQ);
	setTuneFrequencyParameter(CMOST_cmdTune, FM_HIGH_FREQ);
	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdTune, sizeof(CMOST_cmdTune), resp, &resp_len) != 0)
	{
		printf("Error tuning frequency\n");
		return 1;
	}

	/*
	 * [test] wait T2
	 */
	usleep(BOOTFMDEMO_DELAY_T2_SEC * USLEEP_ONE_SEC);

	/*
	 * [test] signal control interface that FFM.HIGH MHz tests can start
	 */
	// ...

	/*
	 * [test] upon reception of "continue test" signal from control interface, set received frequency to FFM.MID MHz
	 */
	printf("Tuning to %d\n", FM_MID_FREQ);
	setTuneFrequencyParameter(CMOST_cmdTune, FM_MID_FREQ);
	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdTune, sizeof(CMOST_cmdTune), resp, &resp_len) != 0)
	{
		printf("Error tuning frequency\n");
		return 1;
	}

	/*
	 * [test] wait T2
	 */
	usleep(BOOTFMDEMO_DELAY_T2_SEC * USLEEP_ONE_SEC);

	/*
	 * [test] signal control interface that FFM.MID MHz tests can start
	 */
	// ...

	/*
	 * [test] upon reception of "continue test" signal from control interface, set FM seek thresholds to VFM,SEEK,FS and VFM,SEEK,DET
	 * Commented out since I don't have sane defaults for the seek thresholds
	 */
#if 0
	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdSetSeekThresholds, sizeof(CMOST_cmdSetSeekThresholds), resp, &resp_len) != 0)
	{
		printf("Error setting seek thresholds\n");
		return 1;
	}
#endif

	/*
	 * first seek, done with no signal, expect the seek to loop around the whole band
	 */
	if (seekTest(&status))
	{
		return 1;
	}
	if (((status & SEEK_STATUS_FULL_CYCLE) == SEEK_STATUS_FULL_CYCLE) &&
		((status & SEEK_STATUS_GOOD_STATION) == 0))
	{
		/*
		 * the test is successfull, communicate to the control interface
		 */
	}
	else
	{
		/*
		 * the test is failed, communicate to the control interface
		 */
	}

	/*
	 * not required by the test but useful for debuging
	 */
	usleep(BOOTFMDEMO_DELAY_T2_SEC * USLEEP_ONE_SEC);

	/*
	 * [test] signal control interface final status of seek (expected behavior is that a full round is made,
	 * [test] with the tuner coming back to FFM.MID MHz and reporting full scan made with no valid station found)
	 */
	// send status...
	
	/*
	 * second seek, signal is present, expect the seek to stop before looping around the whole band
	 */
	if (seekTest(&status))
	{
		return 1;
	}
	if (((status & SEEK_STATUS_FULL_CYCLE) == 0) &&
		((status & SEEK_STATUS_GOOD_STATION) == SEEK_STATUS_GOOD_STATION))
	{
		/*
		 * the test is successfull, communicate to the control interface
		 */
	}
	else
	{
		/*
		 * the test is failed, communicate to the control interface
		 */
	}

	/*
	 * [test] signal control interface final status of seek (expected behavior is that seek is interrupted before a full round is made,
	 * [test] with the tuner stopping at FFM.SEEK MHz and reporting scan finished with valid station found)
	 */
	// send status...

	return 0;
}

/***************************
 *
 * parseParameters
 *
 **************************/
int parseParameters(int argc, char **argv)
{
	if (argc == 1)
	{
		return 0;
	}

	printf("Usage: %s\n", argv[0]);
	return 1;
}

/***************************
 *
 * main
 *
 **************************/
/*
 * Returns:
 * 1 error(s)
 * 0 success
 */
int main(int argc, char **argv)
{
	if (parseParameters(argc, argv))
	{
		return 1;
	}

	OSAL_ClockResetTime();

	if (initLibrary())
	{
		printf("Error initializing the library\n");
		return 1;
	}
	/*
	 * CONTROL INTERFACE: Start TEST received
	 */
	if (initCMOST())
	{
		printf("Error initializing the CMOST\n");
		return 1;
	}

	RunFMTests();

	return 0;
}

