/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_can_p.h
* Author             : APG-MID Application Team
* Date First Issued  : 12/09/2013
* Description        : This file provides CAN driver routines definitions
********************************************************************************
* History:
* 12/09/2013: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************
*/
#ifndef _CAN_P_H_
#define _CAN_P_H_

#define CAN_HCL_VERSION_ID  1
#define CAN_HCL_MAJOR_ID    0
#define CAN_HCL_MINOR_ID    0

#define  __nop() //0x01 << 1

#ifndef NULL_PTR
 #define NULL_PTR  ((void *)0)
#endif

#ifndef CAN_CFG_HEADER
#define CAN_CFG_HEADER

#define CAN_DEV_ERROR_DETECT

#define MAX_HANDLES 		(uint8_t) 0xFF

// Define common parameters
#define CAN0BASE    0x40010000          // CAN Base Address
#define CAN1BASE    0x50130000          // CAN Base Address

// address of CAN controllers
#define CCAN_CONTROLLER_0	((volatile CCan *)(CAN0BASE))
#define CCAN_CONTROLLER_1	((volatile CCan *)(CAN1BASE))
#define CAN0    (*(volatile CCan *)(CAN0BASE))          // CAN Interface
#define CAN1    (*(volatile CCan *)(CAN1BASE))          // CAN Interface

// communication speed
#define CAN_BAUDRATE1MB			(uint16_t) 1000
#define CAN_BAUDRATE500kB		(uint16_t) 500
#define CAN_BAUDRATE250kB		(uint16_t) 250
#define CAN_BAUDRATE125kB		(uint16_t) 125
#define CAN_BAUDRATE100kB		(uint16_t) 100

// definition of maximum controllers handled by this DRIVER
#define MAX_CONFIG_CAN_CONTROLLERS	(uint8_t) 1

// this definitions belong to CAN Controller (HW)
#define MAX_TX_OBJECTS	(uint8_t) 4
#define MAX_RX_OBJECTS	(uint8_t) 4
#define MAX_HW_OBJECTS	(MAX_TX_OBJECTS + MAX_RX_OBJECTS + 24)


#define CAN0_IRQ_PRIORITY	(uint8_t)4
#define CAN1_IRQ_PRIORITY	(uint8_t)4
#define CAN2_IRQ_PRIORITY	(uint8_t)4

// LEC CODES Erros in Status Register
#define CANC_LEC_STUFF_ERR			   (uint16_t)0x0001	// more than 5 equal bits in sequence have
								//occured in a part of received message where this is not allowed
#define CANC_LEC_FORM_ERR			   (uint16_t)0x0002	// fixed format a part of received
													//frame has the wrong format
#define CANC_LEC_ACK_ERR			   (uint16_t)0x0003	// no acknowlegement
#define CANC_LEC_BIT1_ERR			   (uint16_t)0x0004	// during arbitration happend error
#define CANC_LEC_BIT0_ERR			   (uint16_t)0x0005	// during tx happend error
#define CANC_LEC_CRC_ERR			   (uint16_t)0x0006	// crc error
#define CANC_LEC_ALL_ERR			   (uint16_t)(CANC_LEC_STUFF_ERR | CANC_LEC_FORM_ERR | CANC_LEC_ACK_ERR | CANC_LEC_BIT1_ERR | CANC_LEC_BIT0_ERR |CANC_LEC_CRC_ERR)

 //Control/Status registers values used by can_init()
//Control/Status registers values used by can_init()
#define CAN_INITIALIZATION_EN         (uint16_t)  0x0001           //INIT bit in CAN's Control reg
#define CAN_CONFIG_CHANGE_EN          (uint16_t)  0x0040           //allows access to bit timing register
#define CAN_NOIE                      (uint16_t)0x0000    //no interrupts will be used
#define CAN_IE                        (uint16_t)0x0002    //CAN global interrupt enable
#define CAN_SIE                       (uint16_t)0x0004    //Status change int. enable
#define CAN_EIE                       (uint16_t)0x0008    //Error interrupt enable
#define CAN_DAR                       (uint16_t)0x0020    //Disable automatic retransmission

#define CCAN_RECEIVE_ERROR_PASSIVE		(uint16_t)0x8000	// Receive error counter has reached the error passive level

#endif

// ***************************** End of CAN_Cfg.h *************************


    //error codes
    #define FAIL_REGWRITE_C0CSR           FAIL_REGWRITE+0x1D0
    #define FAIL_REGWRITE_C1CSR           FAIL_REGWRITE+0x1D1
    #define FAIL_REGWRITE_C2CSR           FAIL_REGWRITE+0x1D2
    #define FAIL_REGWRITE_C0BTR           FAIL_REGWRITE+0x1D3
    #define FAIL_REGWRITE_C1BTR           FAIL_REGWRITE+0x1D4
    #define FAIL_REGWRITE_C2BTR           FAIL_REGWRITE+0x1D5

    #define FAIL_CAN_FRAME_LOSS           FAIL_REGWRITE+0x1DA
    #define FAIL_CAN_NO_DATA              FAIL_REGWRITE+0x1DB

    //CAN channels
    #define CAN_CHANNEL0                  0x00
    #define CAN_CHANNEL1                  0x01
    #define CAN_CHANNEL2                  0x02


    //test modes
    #define CAN_TSTMODE_BASIC             0x0040    //CCAN Basic Mode
    #define CAN_TSTMODE_SILENT            0x0080    //CCAN Silent Mode
    #define CAN_TSTMODE_LOOPBACK          0x0100    //CCAN Loopback Mode

    #define CAN_TSTMODE_TXCTRL_DEFAULT    0x0000    //CAN core controls CAN_TX pin (Reset value)
    #define CAN_TSTMODE_TXCTRL_MONITOR    0x0200    //Sample point monitored at CAN_TX
    #define CAN_TSTMODE_TXCTRL_DOMINANT   0x0400    //CAN_TX drives dominat (=0) value
    #define CAN_TSTMODE_TXCTRL_RECESIVE   0x0600    //CAN_TX drives recessive value

    #define CAN_TSTMODE_RXCTRL_RECESIVE   0x0800    //CAN bus is recessive (CAN_RX = 1)
    #define CAN_TSTMODE_RXCTRL_DOMINANT   0x0000    //CAN bus is dominant (CAN_RX = 0)

    //Message Control and Msg. Config. registers values used by can_mobject_setup()
    #define CAN_MOBJECT_TX                0x01      //indicates Transmit Message object
    #define CAN_MOBJECT_RX                0x00      //indicates Receive Message object
    #define CAN_MOBJECT_LONGID            0x02      //indicates length of MO's identifier (29 bits)
    #define CAN_MOBJECT_SHORTID           0x00      //indicates length of MO's identifier (11 bits)
    #define CAN_MOBJECT_TX_IE             0x04      //msg. object can generate TX interrupts
    #define CAN_MOBJECT_RX_IE             0x08      //msg. object can generate RX interrupts
    #define CAN_MOBJECT_EOB               0x10      //indicates End of Buffer
    #define CAN_MOBJECT_UMASK             0x20      //indicates Use Acceptance Mask
    #define CAN_MOBJECT_MXTD              0x40      //indicates Mask Extended Identifier
    #define CAN_MOBJECT_MDIR              0x80      //indicates Mask Message Direction

    //following two constants are used by can_mobject_updatevalid()
    #define CAN_MOBJECT_VALID             0x10      //indicates object is valid and is
                                                    //used by CAN controller
    #define CAN_MOBJECT_INVALID           0x00      //indicates object is invalid and
                                                    //can not by used by CAN controller

    //CAN MOs (Message Objects) indexes
    #define CAN_MO_1                      0x01
    #define CAN_MO_2                      0x02
    #define CAN_MO_3                      0x03
    #define CAN_MO_4                      0x04
    #define CAN_MO_5                      0x05
    #define CAN_MO_6                      0x06
    #define CAN_MO_7                      0x07
    #define CAN_MO_8                      0x08
    #define CAN_MO_9                      0x09
    #define CAN_MO_10                     0x0A
    #define CAN_MO_11                     0x0B
    #define CAN_MO_12                     0x0C
    #define CAN_MO_13                     0x0D
    #define CAN_MO_14                     0x0E
    #define CAN_MO_15                     0x0F
    #define CAN_MO_16                     0x10
    #define CAN_MO_17                     0x11
    #define CAN_MO_18                     0x12
    #define CAN_MO_19                     0x13
    #define CAN_MO_20                     0x14
    #define CAN_MO_21                     0x15
    #define CAN_MO_22                     0x16
    #define CAN_MO_23                     0x17
    #define CAN_MO_24                     0x18
    #define CAN_MO_25                     0x19
    #define CAN_MO_26                     0x1A
    #define CAN_MO_27                     0x1B
    #define CAN_MO_28                     0x1C
    #define CAN_MO_29                     0x1D
    #define CAN_MO_30                     0x1E
    #define CAN_MO_31                     0x1F
    #define CAN_MO_32                     0x20

    //shortcut to access CAN's Message Objects
    //use this whenever you use can_mobject_xxx() functions
    #define CAN0_MO(mo_id) (mo_id)        // use parameter mo_id = CAN_MO_1..CAN_MO_32
    #define CAN1_MO(mo_id) (0x40|mo_id)   // use parameter mo_id = CAN_MO_1..CAN_MO_32
    #define CAN2_MO(mo_id) (0x80|mo_id)   // use parameter mo_id = CAN_MO_1..CAN_MO_32

// ****************************************************************************
// *********************** Function Prototypes ********************************
// ****************************************************************************

    // Function for baudrate computation
    // Note that TSEG1 is preset to 4, TSEG2 to 1 (i.e. sample point is at 75% of bittime)
    // and SJW to 0 (i.e. 12.5% of bittime)
    // This calculation is valid for CPU freqv (CPU_CLOCK) = 64, 40, 32, 24, 20, 16, 12, 8 or 4 MHz
    // and standard CAN bitrates: 500, 250, 125 or 100 Kbit/s. Other values should be checked!
    // note: CAN bitrates 500 and 100 Kbit/s are not valid for CPU frekv. 20 and 4 MHZ
    // note: CAN bitrate 1 Mbit/s is valid only for CPU freqv. 32 and 16 MHz.
    #ifdef CAN_CFGPRM_DISABLE_DIVIDER
	#define CAN_GET_RELOAD_VALUE(bitrate) (0x1400 | (0x3F & ((CPU_CLOCK/(8*bitrate)) - 1)))
    #else
	#define CAN_GET_RELOAD_VALUE(bitrate) (0x1400 | (0x3F & ((CPU_CLOCK/(8*bitrate)) - 1)))
    #endif

    //tells if CAN channel 'channel' is in the Bus Off state
    #define CAN_IS_BUSOFF(channel) (CAN##channel##.SR.reg & (uint16_t)0x80)

typedef struct /* can_t */
{
    union /* control */           // 0x00
    {
        struct
        {
            unsigned int  init          : 1;        // initialization is started
            unsigned int  ie            : 1;        // module interrupt enable
            unsigned int  sie           : 1;        // status change interrupt enable
            unsigned int  eie           : 1;        // error interrupt enable
            unsigned int  unused        : 1;
            unsigned int  dar           : 1;        // Disable automatic retransmission
            unsigned int  cce           : 1;        // CPU has write access to the bit timing register (while init = 1)
            unsigned int  test          : 1;        // test mode enable
            unsigned int  unused_0      : 24;
        }bit;
        uint16_t reg;
    }CR;

    union /* status */        // 0x04
    {
        struct
        {
            unsigned int  lec           : 3;        // Last error code
            unsigned int  txok          : 1;        // Transmitted msg. OK
            unsigned int  rxok          : 1;        // Received msg. OK
            unsigned int  epas          : 1;        // Error passive
            unsigned int  ewarn         : 1;        // Error warning status
            unsigned int  boff          : 1;        // Bus off status
            unsigned int  unused_1      : 24;
        }bit;
        uint16_t reg;
    }SR;

    union /* error */     // 0x08
    {
        struct
        {
            unsigned int  tec           : 8;        // Transmit error counter
            unsigned int  rec           : 7;        // Rx error counter
            unsigned int  rp            : 1;        // receive error passive
            unsigned int  unused_2      : 16;
        }bit;
        uint16_t reg;
    }ECR;

    union /* bit timing */    // 0x0c
    {
        struct
        {
            unsigned int  brp           : 6;        // baud rate prescaler
            unsigned int  sjw           : 2;        // synch. jump width
            unsigned int  tseg1         : 4;        // the time segment AFTER time the sample point
            unsigned int  tseg2         : 3;        // the time segment BEFORE time the sample point
            unsigned int  unused_3      : 17;
        }bit;
        uint16_t reg;
    }BTR;

    uint16_t IDR;								// 0x10
    uint16_t unused_5;

    union /* test */              // 0x14
    {
        struct
        {
            unsigned int  unused_test0  : 2;
            unsigned int  basic         : 1;        // Basic mode if '1' IF1 reg. used as TX Buffer; IF2 reg. used as RX Buffer
            unsigned int  silent        : 1;        // Silent mode enable
            unsigned int  lback         : 1;        // Loop back mode enable
            unsigned int  tx            : 2;        // Control of CAN_TX pin
            unsigned int  rx            : 1;        // Monitors the actual value of the CAN_RX pin
            unsigned int  unused_test1  : 24;
        }bit;
        uint16_t reg;
    }TEST;

  uint16_t BRP;                                 // 0x18 (only 4 last significant bits are valid ??? )
  uint16_t unused_6;                           // 0x1c
  uint32_t unused_61;                           // 0x1c

/********************* IF 1 registers ***************************/

    union  /* IF 1 command request */  // 0x20
    {
        struct
        {
            unsigned int  msgn          : 6;        // Message number valid only <0x01 - 0x20>
            unsigned int  unused        : 9;
            unsigned int  busy          : 1;        // Set this flag to 1, when writing to the IFx Command Register !
            unsigned int  unused_7      : 16;
        }bit;
        uint16_t reg;
    }IF1CR;

    union  /* IF 1 command mask */    // 0x24
    {
        struct
        {
            unsigned int  datab         : 1;        // access data bytes 4 - 7
            unsigned int  dataa         : 1;
            unsigned int  txreq         : 1;
            unsigned int  clrintpnd     : 1;
            unsigned int  control       : 1;
            unsigned int  arb           : 1;
            unsigned int  mask          : 1;
            unsigned int  wr            : 1;
            unsigned int  unused_8      : 24;
        }bit;
        uint16_t reg;
    }IF1CMR;

    uint16_t IF1MASK1;                // 0x28
    uint16_t unused_9;

    union  /* IF 1 Mask 2 */          // 0x2c
    {
        struct
        {
            unsigned int  msk           : 13;
            unsigned int  unused        : 1;
            unsigned int  mdir          : 1;
            unsigned int  mxtd          : 1;
            unsigned int  unused_10     : 16;
        }bit;
        uint16_t reg;
    }IF1MASK2;

  uint16_t IF1AR1;     // 0x30
  uint16_t unused_11;

    union  /* IF 1 Arbitration 2 */   // 0x34
    {
        struct
        {
            unsigned int  id            : 13;
            unsigned int  dir           : 1;
            unsigned int  xtd           : 1;
            unsigned int  msgval        : 1;
            unsigned int  unused_12     : 16;
        }bit;
        uint16_t reg;
    }IF1AR2;

    union  /* IF 1 message control */     // 0x38
    {
        struct
        {
            unsigned int  dlc           : 4;
            unsigned int  $RESERVED     : 3;
            unsigned int  eob           : 1;
            unsigned int  txrqst        : 1;
            unsigned int  rmten         : 1;
            unsigned int  rxie          : 1;
            unsigned int  txie          : 1;
            unsigned int  umask         : 1;
            unsigned int  intpnd        : 1;
            unsigned int  msglst        : 1;
            unsigned int  newdat        : 1;
            unsigned int  unused_13     : 16;
        }bit;
        uint16_t reg;
    }IF1MCR;

    union  /* IF 1 Data A1 */     // 0x3c
    {
        struct
        {
            unsigned int  data0         : 8;
            unsigned int  data1         : 8;
            unsigned int  unused_14     : 16;
        }bit;
        uint16_t reg;
    }IF1DATAA1;


    union  /* IF 1 Data A2 */     // 0x40
    {
        struct
        {
            unsigned int  data2         : 8;
            unsigned int  data3         : 8;
            unsigned int  unused_15     : 16;
        }bit;
        uint16_t reg;
    }IF1DATAA2;


    union  /* IF 1 Data B1 */     // 0x44
    {
        struct
        {
            unsigned int  data4         : 8;
            unsigned int  data5         : 8;
            unsigned int  unused_16     : 16;
        }bit;
        uint16_t reg;
    }IF1DATAB1;


    union  /* IF 1 Data B2 */     // 0x48
    {
        struct
        {
            unsigned int  data6         : 8;
            unsigned int  data7         : 8;
            unsigned int  unused_17     : 16;
        }bit;
        uint16_t reg;
    }IF1DATAB2;

    uint32_t unused_18[13];      // 0x4B - 0x7F

/********************* IF 2 registers ***************************/
      union  /* IF 2 command request */     // 0x80
    {
        struct
        {
            unsigned int  msgn          : 6;        // Message number valid only <0x01 - 0x20>
            unsigned int  unused_19     : 9;
            unsigned int  busy          : 1;        // Set this flag to 1, when writing to the IFx Command Register !
            unsigned int  unused_20     : 16;
        }bit;
        uint16_t reg;
    }IF2CR;


    union  /* IF 2 command mask */          // 0x84
    {
        struct
        {
            unsigned int  datab         : 1;        // access data bytes 4 - 7
            unsigned int  dataa         : 1;
            unsigned int  txreq         : 1;
            unsigned int  clrintpnd     : 1;
            unsigned int  control       : 1;
            unsigned int  arb           : 1;
            unsigned int  mask          : 1;
            unsigned int  wr            : 1;
            unsigned int  unused_22     : 24;
        }bit;
        uint16_t reg;
    }IF2CMR;


    uint16_t IF2MASK1;                         // 0x88
    uint16_t unused_23;

    union  /* IF 2 Mask 2 */                // 0x8c
    {
        struct
        {
            unsigned int  msk           : 13;
            unsigned int  $RESERVED     : 1;
            unsigned int  mdir          : 1;
            unsigned int  mxtd          : 1;
            unsigned int  unused_24     : 16;
        }bit;
        uint16_t reg;
    }IF2MASK2;


  uint16_t IF2AR1;                             // 0x90
  uint16_t unused_25;

    union  /* IF 2 Arbitration 2 */         // 0x94
    {
        struct
        {
            unsigned int  id            : 13;
            unsigned int  dir           : 1;
            unsigned int  xtd           : 1;
            unsigned int  msgval        : 1;
            unsigned int  unused_26     : 16;
        }bit;
        uint16_t reg;
    }IF2AR2;


    union  /* IF 2 message control */       // 0x98
    {
        struct
        {
            unsigned int  dlc           : 4;
            unsigned int  unused_27     : 3;
            unsigned int  eob           : 1;
            unsigned int  txrqst        : 1;
            unsigned int  rmten         : 1;
            unsigned int  rxie          : 1;
            unsigned int  txie          : 1;
            unsigned int  umask         : 1;
            unsigned int  intpnd        : 1;
            unsigned int  msglst        : 1;
            unsigned int  newdat        : 1;
            unsigned int  unused_28     : 16;
        }bit;
        uint16_t reg;
    }IF2MCR;


    union  /* IF 2 Data A1 */               // 0x9c
    {
        struct
        {
            unsigned int  data0         : 8;
            unsigned int  data1         : 8;
            unsigned int  unused_29     : 16;
        }bit;
        uint16_t reg;
    }IF2DATAA1;


    union  /* IF 2 Data A2 */               // 0xa0
    {
        struct
        {
            unsigned int  data2         : 8;
            unsigned int  data3         : 8;
            unsigned int  unused_30     : 16;
        }bit;
        uint16_t reg;
    }IF2DATAA2;


    union  /* IF 2 Data B1 */               // 0xa4
    {
        struct
        {
            unsigned int  data4         : 8;
            unsigned int  data5         : 8;
            unsigned int  unused_31     : 16;
        }bit;
        uint16_t reg;
    }IF2DATAB1;


    union  /* IF 2 Data B2 */               // 0xa8
    {
        struct
        {
            unsigned int  data6         : 8;
            unsigned int  data7         : 8;
            unsigned int  unused_32     : 16;
        }bit;
        uint16_t reg;
    }IF2DATAB2;

    uint32_t unused_33[21];                    // 0xac - 0xb0
/*************** Status Registers **********************/

    uint16_t TRR1;                             // offset 0x100
	uint16_t unused_34;
    uint16_t TRR2;
	uint16_t unused_35;
    uint32_t unused_36[6];
    uint16_t NEWDATR1;
	uint16_t unused_37;                        // offset 0x120
    uint16_t NEWDATR2;
	uint16_t unused_38;
    uint32_t unused_39[6];

    uint16_t INTPNDR1;                         // offset 0x140
	uint16_t unused_40;
    uint32_t INTPNDR2;
	uint16_t unused_41;
    uint32_t unused_42[6];

    uint16_t MSGVALR1;
	uint16_t unused_43;                        // offset 0x160
    uint16_t MSGVALR2;
	uint16_t unused_44;
}CCan;



#endif

/* End of file - CAN_P.h*/

