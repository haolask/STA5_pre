/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_i2c_test.c
* Author             : APG-MID Application Team
* Date First Issued  : 08/29/2013
* Description        : This file provides I2C tests in MASTER configuration.
********************************************************************************
* History:
* 08/29/2013: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "trace.h"

#include "sta_i2c_service.h"

#define I2C_CLOCK   51200000

void I2C_test_1(void* param)
{
	struct i2c_com_handler_s      *I2C1_Master = NULL;

	portTickType I2CTimeout = 1000;

	uint8_t		SlaveAddr		= 0x40; // This represents the TVDECODER A2 EVB board is equipped with, write slave address
	uint8_t		SlaveOffset	    = 0x00; // This is the slave offset we want to read write and read
	uint8_t 	    OwnAddress  	= 0x55; // A2 own address assigned to I2C1 peripheral
	uint32_t	    Times			= 20;   // Write/read cycles
 	uint8_t      RX              = 0;    // Test RX buffer (one byte)
 	uint8_t      TX              = 0x2;  // Test TX buffer (one byte)

    /*!
     * I2C service initialization
     */
	i2c_service_init(I2C_CLOCK);

	TRACE_INFO("[I2C Test 1 MASTER]: Creating I2C port\n");

	/*!
    * Open I2C interface 0
    * The port is initialized in SLAVE mode by default.
    */
	if(0 != i2c_open_port(i2c1_regs, 0, OwnAddress, I2C_BUSCTRLMODE_MASTER))
	{
		TRACE_ERR("[I2C Test 1 MASTER]: ERROR: cannot open COM port\n");
		return;
	}

	/*!
	* Set Bus control mode as MASTER, requirement for this test
	*/
	i2c_set_port_mode(i2c1_regs, I2C_BUSCTRLMODE_MASTER);

	/*!
	 * Let's create the unique link between I2C channel and slave device
	 */
	I2C1_Master = i2c_create_com(i2c1_regs, I2C_STANDARD_MODE, SlaveAddr >> 1);

	if(I2C1_Master == NULL)
	{
		TRACE_ERR("[I2C Test 1 MASTER]: ERROR: I2C handler is null for port O\n");
		return;
	}

    /*!
     * Let's start the write -> read cycle
     */
	while(Times--)
	{
		RX	= 0;

        /*!
         * Write the specified offset
         */
        //if(0 != i2c_write (I2C1_Master, SlaveOffset, 1, &TX, 1, 1, &I2CTimeout))
        if(0 != i2c_write (I2C1_Master, SlaveOffset, 1, &TX, 1, 1, &I2CTimeout))
        {
            TRACE_ERR("[I2C Test 1 MASTER]: ERROR: I2C write error\n");
        }
        else
        {
            TRACE_INFO("[I2C Test 1 MASTER]: I2C write OK!\n");
        }

		/*!
		 * Read back the specified offset
		 */
		if(0 != i2c_read(I2C1_Master, SlaveOffset, 1, &RX, 1, &I2CTimeout))
		{
			TRACE_ERR("[I2C Test 1 MASTER]: ERROR: I2C read error\n");
            return;
		}
		else
		{
            TRACE_INFO("[I2C Test 1 MASTER]: I2C read ok\n");
		}

		/*!
		 * Verify that what has been written matches with what we read back
		 */
		if(RX != TX)
		{
			TRACE_ERR("[I2C Test 1 MASTER]: ERROR: Mismatching values %d - %d\n", RX, TX);
			return;
		}
		else
		{
			TRACE_INFO("[I2C Test 1 MASTER]: Matching values %d - %d\n", RX, TX);
		}

		TX += 0x1;

		if(TX == 0xFF)
			TX = 0x1;
	}

	while(1)
		vTaskDelay(pdMS_TO_TICKS(5000));

	return;
}
