/**
  ******************************************************************************
  * @file    sta_acc.h
  * @author  jean-nicolas.graux@st.com
  * @version V1.0.0
  * @date    27-March-2015
  * @brief   This file contains definitions for sta_acc.c driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
  
#ifndef __STA_ACC_H
#define __STA_ACC_H

#include <stdint.h> 
#include "sta_type.h"
/** 
* @brief  Accelerometer driver data
*/ 
struct acc_driver_data {
	int tap_threshold; /* tag threadhold in mG */
	bool tap; /* tag detection flag */
	uint8_t outs; /* outs register value following tap */
	uint16_t *xyz_buf1; /* filled in stream mode */
	uint16_t *xyz_buf2; /* filled only one time after tap */
	uint16_t *xyz_buf1_next; /* pointer to next sample to fill in xyz_buf1 */
	uint16_t *xyz_buf2_next; /* pointer to next sample to fill in xyz_buf2 */
	int buf1_xyz_size; /* size of xyz_buf1, xyz_buf2 in number of uint16_t */
	int buf2_xyz_size; /* size of xyz_buf1, xyz_buf2 in number of uint16_t */
	bool xyz_buf1_full; /* buf1 full flag */ 
	bool xyz_buf2_full; /* buf2 full flag */
	uint8_t odr_pre; /* Output Data Rate used for pre-crash history */
	uint8_t odr; /* Output Data Rate used for post-crash history */
	uint8_t axes_enable; /* Axes enable into CTRL_REG4 register */
	uint8_t self_test; /* Self test into CTRL_REG5 register */
	uint8_t full_scale; /* Full scale into CTRL_REG5 register */
	uint8_t filter_bw; /* Filter BW into CTRL_REG5 register */
	uint8_t wtm; /* nb of samples that must be exceeded in fifo to raise a interrupt */
};

/**
* @brief  Accelerometer driver ops definition
*/
struct acc_driver_ops {
	int (*init)(uint16_t);
	int (*read_id)(uint8_t *);
	int (*reset)(void);
	int (*config_irq)(struct acc_driver_data *);
	int (*handle_irq)(struct acc_driver_data *);
	int (*enable_irq)(void);
	int (*disable_irq)(void);
	int (*calc_xyz)(uint16_t *data_out, uint16_t *data_in, int size, uint8_t fs);
	int (*set_rate)(uint8_t);
	int (*set_scale)(uint8_t);
	int (*sby_prepare)(void);
};

enum acc_error_types {
	ACC_OK = 0,
	ACC_ERROR = -1,
	ACC_TIMEOUT = -2
};

int acc_init_io(void);
int acc_config_io_irq(void);
int acc_io_write(uint8_t* buf, uint8_t addr);
int acc_io_read(uint8_t* buf, uint8_t addr);
int acc_io_read_multiple(uint8_t* buf, uint8_t addr, int length);
int acc_on_tap_get_xyz_datas(uint16_t *buf);
void acc_task(void);

#endif /* __STA_ACC_H */
