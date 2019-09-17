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

#include "tunerdriver.h"

/*****************************************************************
| defines and macros (scope: module-local)
|----------------------------------------------------------------*/
#define CMOST_TUNER_DEVICE_TYPE       	BOOT_TUNER_STAR_S
#define CMOST_TUNER_ID_0				0
#define CMOST_TUNER_ID_1				1
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
#define BOOTFMDEMO_DELAY_T2_SEC       2

#define USLEEP_ONE_MSEC            1000
#define USLEEP_ONE_SEC          1000000
#define USEC_MAX                1000000

#define SEEK_STATUS_RESPONSE_LEN        15
#define SEEK_STATUS_FULL_CYCLE        0x40
#define SEEK_STATUS_GOOD_STATION      0x80
#define SEEK_TIMEOUT_MS               1000
#define SEEK_WAIT_MS                    40

/*
 * Hardware configuration
 */
#define CMOST_TUNER_ID_0                0
#define CMOST_TUNER_ID_1                1
#define CMOST_TUNER_DEVICE_TYPE         BOOT_TUNER_STAR_T
//#define CMOST_TUNER_DEVICE_TYPE       BOOT_TUNER_STAR_S

#ifdef CONFIG_COMM_HDRADIO_SPI
    /*********************/
    /* SPI configuration */
    /*********************/
    #ifdef CONFIG_HOST_OS_TKERNEL
        /***************/
        /* OS TKERNEL  */
        /***************/
        #define HDRADIO_ACCORD2_SPI_SPEED                   2000000

        #ifdef CONFIG_COMM_HDRADIO_SPI_CPHA0_CPOL0
            #define HDRADIO_ACCORDO2_SPI_MODE                   SPI_CPHA0_CPOL0
        #elif defined (CONFIG_COMM_HDRADIO_SPI_CPHA1_CPOL1)
            #define HDRADIO_ACCORDO2_SPI_MODE                   SPI_CPHA1_CPOL1
        #endif // CONFIG_COMM_HDRADIO_SPI_CPHA0_CPOL0

        #define HDRADIO_ACCORDO2_SPI_MODE                   SPI_CPHA0_CPOL0

        #ifdef CONFIG_BOARD_ACCORDO2_CUSTOM2
            #define HDRADIO_ACCORDO2_RESET_GPIO             95
        #else //!CONFIG_BOARD_ACCORDO2_CUSTOM2
            #define HDRADIO_ACCORDO2_RESET_GPIO             20
        #endif //CONFIG_BOARD_ACCORDO2_CUSTOM2

        #ifdef CONFIG_BOARD_ACCORDO2_CUSTOM2
            #define HDRADIO_ACCORDO2_SPI_DEVICE             "SSPb"
            #define HDRADIO_ACCORDO2_CS_GPIO                30
        #else //!CONFIG_BOARD_ACCORDO2_CUSTOM2
            #ifdef __SETTING_TEB__
                #define HDRADIO_ACCORDO2_SPI_DEVICE         "SSPb"
                #define HDRADIO_ACCORDO2_CS_GPIO            30
            #else //!__SETTING_TEB__
                #define HDRADIO_ACCORDO2_SPI_DEVICE         "SSPa"
                #define HDRADIO_ACCORDO2_CS_GPIO            13
             #endif //__SETTING_TEB__
        #endif //CONFIG_BOARD_ACCORDO2_CUSTOM2
    #else // !CONFIG_HOST_OS_TKERNEL
        /*************/
        /* OS LINUX  */
        /*************/
        #define HDRADIO_ACCORD2_SPI_SPEED                   2000000
        #ifdef CONFIG_COMM_HDRADIO_SPI_CPHA0_CPOL0
            #define HDRADIO_ACCORDO2_SPI_MODE                   SPI_CPHA0_CPOL0
        #elif defined (CONFIG_COMM_HDRADIO_SPI_CPHA1_CPOL1)
            #define HDRADIO_ACCORDO2_SPI_MODE                   SPI_CPHA1_CPOL1
        #endif // CONFIG_COMM_HDRADIO_SPI_CPHA0_CPOL0

        #ifdef CONFIG_BOARD_ACCORDO5
             #define HDRADIO_ACCORDO2_RESET_GPIO            10
        #else
             #define HDRADIO_ACCORDO2_RESET_GPIO            20
        #endif

        #ifdef CONFIG_BOARD_ACCORDO5
             #define HDRADIO_ACCORDO2_SPI_DEVICE            "spidev32766.0"
             #define HDRADIO_ACCORDO2_CS_GPIO               26
        #else
             #ifdef CONFIG_BOARD_ACCORDO2_SPI0_IS_0
                 #define HDRADIO_ACCORDO2_SPI_DEVICE        "spidev0.0"
             #else
                 #define HDRADIO_ACCORDO2_SPI_DEVICE        "spidev32766.0"
             #endif
             #define HDRADIO_ACCORDO2_CS_GPIO               13
        #endif
    #endif //CONFIG_HOST_OS_TKERNEL

#else // !CONFIG_COMM_HDRADIO_SPI
    /*********************/
    /* I2C configuration */
    /*********************/
    #ifdef CONFIG_HOST_OS_TKERNEL
        /***************/
        /* OS TKERNEL  */
        /***************/
        #ifdef CONFIG_BOARD_ACCORDO2_CUSTOM2
             #define HDRADIO_ACCORDO2_RESET_GPIO            95
        #else //!CONFIG_BOARD_ACCORDO2_CUSTOM2
             #define HDRADIO_ACCORDO2_RESET_GPIO            20
        #endif //CONFIG_BOARD_ACCORDO2_CUSTOM2

        #define HDRADIO_ACCORDO2_I2C_DEVICE                 "IICa"
        #define HDRADIO_ACCORDO2_I2C_ADDRESS                (tU8)0x2E

    #else // !CONFIG_HOST_OS_TKERNEL
        /*************/
        /* OS LINUX  */
        /*************/
        #ifdef CONFIG_BOARD_ACCORDO5
             #define HDRADIO_ACCORDO2_RESET_GPIO            10
        #else
             #define HDRADIO_ACCORDO2_RESET_GPIO            20
        #endif

        #define HDRADIO_ACCORDO2_I2C_DEVICE                 "i2c-0"
        #define HDRADIO_ACCORDO2_I2C_ADDRESS                (tU8)0x2E

    #endif //CONFIG_HOST_OS_TKERNEL

#endif //CONFIG_COMM_HDRADIO_SPI


#ifdef CONFIG_COMM_CMOST_SPI
    /*********************/
    /* SPI configuration */
    /*********************/
    #ifdef CONFIG_HOST_OS_TKERNEL
        /***************/
        /* OS TKERNEL  */
        /***************/
        #define CMOST_ACCORDO2_SPI_SPEED                    2000000
        #define CMOST_ACCORDO2_SPI_MODE                     SPI_CPHA1_CPOL1

        #ifdef CONFIG_BOARD_ACCORDO2_CUSTOM2
            #define CMOST_T1_ACCORDO2_SPI_DEVICE            "SSPa"
            #define CMOST_T1_ACCORDO2_SPI_CS_GPIO           13
            #define CMOST_T1_ACCORDO2_IRQ_GPIO              4
            #define CMOST_T1_ACCORDO2_RESET_GPIO            96

            #define CMOST_T2_ACCORDO2_SPI_DEVICE            "SSPb"
            #define CMOST_T2_ACCORDO2_SPI_CS_GPIO           30
            #define CMOST_T2_ACCORDO2_IRQ_GPIO              47
            #define CMOST_T2_ACCORDO2_RESET_GPIO            97
        #else
            #define CMOST_T1_ACCORDO2_SPI_DEVICE            "SSPb"
            #define CMOST_T1_ACCORDO2_SPI_CS_GPIO           30
            #define CMOST_T1_ACCORDO2_IRQ_GPIO              199
            #define CMOST_T1_ACCORDO2_RESET_GPIO            198

            #define CMOST_T2_ACCORDO2_SPI_DEVICE            "SSPa"
            #define CMOST_T2_ACCORDO2_SPI_CS_GPIO           13
            #define CMOST_T2_ACCORDO2_IRQ_GPIO              199
            #define CMOST_T2_ACCORDO2_RESET_GPIO            198
        #endif // CONFIG_BOARD_ACCORDO2_CUSTOM2
    #else // !CONFIG_HOST_OS_TKERNEL
        /*************/
        /* OS LINUX  */
        /*************/
        #define CMOST_ACCORDO2_SPI_SPEED                    2000000
        #define CMOST_ACCORDO2_SPI_MODE                     SPI_CPHA1_CPOL1

        #ifdef CONFIG_BOARD_ACCORDO5
            #define CMOST_ACCORDO2_RESET_GPIO               8
            #define CMOST_ACCORDO2_IRQ_GPIO                 7
        #else
            #ifdef CONFIG_BOARD_ACCORDO2_CUSTOM2
                #define CMOST_T1_ACCORDO2_SPI_ID                1
                #define CMOST_T1_ACCORDO2_SPI_DEVICE            "spidev32766.0"
                #define CMOST_T1_ACCORDO2_SPI_CS_GPIO           13
                #define CMOST_T1_ACCORDO2_IRQ_GPIO              5
                #define CMOST_T1_ACCORDO2_RESET_GPIO            7

                #define CMOST_T2_ACCORDO2_SPI_ID                2
                #define CMOST_T2_ACCORDO2_SPI_DEVICE            "spidev32766.0"
                #define CMOST_T2_ACCORDO2_SPI_CS_GPIO           30
                #define CMOST_T2_ACCORDO2_IRQ_GPIO              5
                #define CMOST_T2_ACCORDO2_RESET_GPIO            7
            #else
                #define CMOST_T1_ACCORDO2_SPI_ID                1
                #define CMOST_T1_ACCORDO2_SPI_DEVICE            "spidev32766.0"
                #define CMOST_T1_ACCORDO2_SPI_CS_GPIO           30
                #define CMOST_T1_ACCORDO2_IRQ_GPIO              167
                #define CMOST_T1_ACCORDO2_RESET_GPIO            166

                #define CMOST_T2_ACCORDO2_SPI_ID                2
                #define CMOST_T2_ACCORDO2_SPI_DEVICE            "spidev32766.0"
                #define CMOST_T2_ACCORDO2_SPI_CS_GPIO           13
                #define CMOST_T2_ACCORDO2_IRQ_GPIO              167
                #define CMOST_T2_ACCORDO2_RESET_GPIO            166
            #endif // CONFIG_BOARD_ACCORDO2_CUSTOM2
        #endif
    #endif //CONFIG_HOST_OS_TKERNEL

#else // !CONFIG_COMM_HDRADIO_SPI
    /*********************/
    /* I2C configuration */
    /*********************/
    #ifdef CONFIG_HOST_OS_TKERNEL
        /***************/
        /* OS TKERNEL  */
        /***************/
        #ifdef CONFIG_BOARD_ACCORDO2_CUSTOM2
            #define CMOST_T1_ACCORDO2_IRQ_GPIO              4
            #define CMOST_T1_ACCORDO2_RESET_GPIO            96

            #define CMOST_T2_ACCORDO2_IRQ_GPIO              47
            #define CMOST_T2_ACCORDO2_RESET_GPIO            97
        #else
            #define CMOST_T1_ACCORDO2_IRQ_GPIO              199
            #define CMOST_T1_ACCORDO2_RESET_GPIO            198

            #define CMOST_T2_ACCORDO2_IRQ_GPIO              199
            #define CMOST_T2_ACCORDO2_RESET_GPIO            198
        #endif // CONFIG_BOARD_ACCORDO2_CUSTOM2

        #define CMOST_ACCORDO2_I2C_DEVICE                   "IICb"

        #ifdef CONFIG_BOARD_ACCORDO2_CUSTOM1
            #define CMOST_T1_ACCORDO2_I2C_ADDRESS           0xC8
        #else
            #define CMOST_T1_ACCORDO2_I2C_ADDRESS           0xC2
            #define CMOST_T2_ACCORDO2_I2C_ADDRESS           0xC8
        #endif


    #else // !CONFIG_HOST_OS_TKERNEL
        /*************/
        /* OS LINUX  */
        /*************/
        #ifdef CONFIG_BOARD_ACCORDO5
            #define CMOST_ACCORDO2_RESET_GPIO               8
            #define CMOST_ACCORDO2_IRQ_GPIO                 7
        #else
            #ifdef CONFIG_BOARD_ACCORDO2_CUSTOM1
                #define CMOST_T1_ACCORDO2_RESET_GPIO            7
                #define CMOST_T1_ACCORDO2_IRQ_GPIO              5

                #define CMOST_T2_ACCORDO2_RESET_GPIO            7
                #define CMOST_T2_ACCORDO2_IRQ_GPIO              5
            #else
                #define CMOST_T1_ACCORDO2_RESET_GPIO            166
                #define CMOST_T1_ACCORDO2_IRQ_GPIO              167

                #define CMOST_T2_ACCORDO2_RESET_GPIO            166
                #define CMOST_T2_ACCORDO2_IRQ_GPIO              167
            #endif
        #endif

        #ifdef CONFIG_BOARD_ACCORDO2_CUSTOM1
            #define CMOST_T1_ACCORDO2_I2C_ADDRESS           0xC8
            #define CMOST_ACCORDO2_I2C_DEVICE               "i2c-0"
        #else
            #define CMOST_T1_ACCORDO2_I2C_ADDRESS           0xC2
            #define CMOST_T2_ACCORDO2_I2C_ADDRESS           0xC8
            #define CMOST_ACCORDO2_I2C_DEVICE               "i2c-1"
        #endif
    #endif //CONFIG_HOST_OS_TKERNEL
#endif //CONFIG_COMM_CMOST_SPI


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

	/*
	 * Performs some library initialization.
	 *
	 * In particular it initializes the OSAL needed in the EXTERNAL
	 * driver implementation.
	 *
	 * Call only once during the application startup
	 */
	if (TUNERDRIVER_init(CMOST_TUNER_ID_0, &CMOSTDeviceConfiguration[CMOST_TUNER_ID_0]) != OSAL_OK)
	{
		printf("Error initializg \n");
		return 1;
	}
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
	if (TUNERDRIVER_reset_CMOST(CMOST_TUNER_ID_0) != OSAL_OK)
	{
		printf("Error TUNERDRIVER_reset_CMOST\n");
		return 1;
	}

	if (TUNERDRIVER_download_CMOST(CMOST_TUNER_ID_0, CMOST_TUNER_DEVICE_TYPE, NULL, 0, 1) != OSAL_OK)
	{
		printf("Error TUNERDRIVER_download_CMOST\n");
		return 1;
	}

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
	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdSeekStart, sizeof(CMOST_cmdSeekStart), resp, &resp_len) != OSAL_OK)
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
		if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdGetSeekStatus, sizeof(CMOST_cmdGetSeekStatus), resp, &resp_len) != OSAL_OK)
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

	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdSeekStop, sizeof(CMOST_cmdSeekStop), resp, &resp_len) != OSAL_OK)
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
	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdSetBand, sizeof(CMOST_cmdSetBand), resp, &resp_len) != OSAL_OK)
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
	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdTune, sizeof(CMOST_cmdTune), resp, &resp_len) != OSAL_OK)
	{
		printf("Error tuning frequency\n");
		return 1;
	}

	/*
	 * [test] wait T1
	 */
	usleep(BOOTFMDEMO_DELAY_T1_SEC * USLEEP_ONE_SEC);

	/*
	 * [test] signal control interface that FFM.LOW MHz tests can start
	 */
	// ....

	/*
	 * [test] upon reception of "continue test" signal from control interface, set received frequency to FFM.HIGH MHz
	 */
	printf("Tuning to %d\n", FM_HIGH_FREQ);
	setTuneFrequencyParameter(CMOST_cmdTune, FM_HIGH_FREQ);
	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdTune, sizeof(CMOST_cmdTune), resp, &resp_len) != OSAL_OK)
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
	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdTune, sizeof(CMOST_cmdTune), resp, &resp_len) != OSAL_OK)
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
	if (TUNERDRIVER_sendCommand_CMOST(CMOST_TUNER_ID_0, CMOST_cmdSetSeekThresholds, sizeof(CMOST_cmdSetSeekThresholds), resp, &resp_len) != OSAL_OK)
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

