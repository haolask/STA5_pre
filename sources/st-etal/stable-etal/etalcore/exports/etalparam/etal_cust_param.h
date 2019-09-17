//!
//!  \file 		etal_cust_param.c
//!  \brief 	<i><b> ETAL API layer </b></i>
//!  \details   Customer setting for device configuration
//!  \author 	Jean-Hugues Perrin
//!

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
			#define HDRADIO_ACCORDO2_RESET_GPIO      		20

			// SPI defines if SPI configuration
			#ifdef __SETTING_TEB__
				#define HDRADIO_ACCORDO2_SPI_DEVICE 	 	"SSPb"
				#define HDRADIO_ACCORDO2_CS_GPIO         	30
			#else //!__SETTING_TEB__
				#define HDRADIO_ACCORDO2_SPI_DEVICE 	 	"SSPa"
				#define HDRADIO_ACCORDO2_CS_GPIO         	13
		 	 #endif //__SETTING_TEB__

			// I2C defines if I2C configuration 
			#define HDRADIO_ACCORDO2_I2C_DEVICE           	"IICa"
			#define HDRADIO_ACCORDO2_I2C_ADDRESS     		((tU8)0x2E)
		#endif //CONFIG_BOARD_ACCORDO2_CUSTOM1
	#else // !CONFIG_HOST_OS_TKERNEL
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
    
    #else // !CONFIG_HOST_OS_TKERNEL
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
		
	#else // !CONFIG_HOST_OS_TKERNEL
		/*************/
		/* OS LINUX  */
		/*************/
		#define CMOST_ACCORDO2_SPI_SPEED					1000000
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
				#define CMOST_T1_ACCORDO2_SPI_DEVICE			"spidev32765.0"
				
#ifdef	CONFIG_BOARD_ACCORDO2_CMOST_1_TRUE_SPI
				#define CMOST_T1_ACCORDO2_SPI_CS_GPIO			ETAL_CS_TRUE_SPI
#else
				#define CMOST_T1_ACCORDO2_SPI_CS_GPIO			62
#endif

				#define CMOST_T1_ACCORDO2_MISO_GPIO 			36


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



