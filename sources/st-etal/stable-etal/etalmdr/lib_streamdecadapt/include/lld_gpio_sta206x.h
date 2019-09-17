// _____________________________________________________________________________
//| FILE:         lld_gpio.h
//| PROJECT:      ADR3 - STA660
//|_____________________________________________________________________________
//| DESCRIPTION:  GPIO low level driver header file
//|_____________________________________________________________________________
//| COPYRIGHT:    (c) 2009 STMicroelectronics, Agrate Brianza (MI) (ITALY)
//|
//| HISTORY:
//| Date        | Modification               | Author
//|_____________________________________________________________________________
//| 2009.06.26  | Porting from Newcastle     | A^_^L
//|               project
//| 2009.06.26  | Initial revision           | A^_^L
//|_____________________________________________________________________________

//------------------------------------------------------------------------------
//!  \file lld_gpio.h
//!  \brief <i><b>GPIO low level driver header file</b></i>
//!  \author Alberto Saviotti
//!  \author (original version) Luigi Cotignano
//!  \version 1.0
//!  \date 2009.06.26
//!  \bug Unknown
//!  \warning None
//------------------------------------------------------------------------------

#ifndef _LLD_GPIO_H_
#define _LLD_GPIO_H_

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------
// defines
//----------------------------------------------------------------------
//! \def LLD_GPIO0_ADDRESS
//!      Specify the GPIO 0 mapping address.
#define LLD_GPIO0_ADDRESS           GPIO0_REG_START_ADDR

//! \def LLD_GPIO1_ADDRESS
//!      Specify the GPIO 1 mapping address.
#define LLD_GPIO1_ADDRESS           GPIO1_REG_START_ADDR

//! \def LLD_GPIO2_ADDRESS
//!      Specify the GPIO 2 mapping address.
#define LLD_GPIO2_ADDRESS           GPIO2_REG_START_ADDR

//! \def LLD_GPIO3_ADDRESS
//!      Specify the GPIO 3 mapping address.
#define LLD_GPIO3_ADDRESS           GPIO3_REG_START_ADDR

#if defined (PLATFORM_IS_CARTESIOPLUS)
//! \def LLD_GPIO4_ADDRESS
//!      Specify the GPIO 4 mapping address.
#define LLD_GPIO4_ADDRESS           GPIO4_REG_START_ADDR

//! \def NUMBERS_OF_PINS
//!      Numbers of available PINs in a linear mapping.
#define NUMBERS_OF_PINS 			139
#else
//! \def NUMBERS_OF_PINS
//!      Numbers of available PINs in a linear mapping.
#define NUMBERS_OF_PINS 			127
#endif

//! \def PINS_FOR_GROUP
//!      Number of Pins for group.
#define	PINS_FOR_GROUP 			32


/*#define AF_MODE_GPIO				((tU32)0)
#define AF_MODE_A					((tU32)1)
#define AF_MODE_B					((tU32)2)
#define AF_MODE_C					((tU32)3)
*/
#define AF_MODE_SHIFT				((tU32)30)
#define AF_SHIFT_SHIFT				((tU32)22)
#define AF_BITS_SHIFT				((tU32)0)

#define AF_MODE_MASK				((tU32)0x00000003)
#define AF_SHIFT_MASK				((tU32)0x0000007F)
#define AF_BITS_MASK				((tU32)0x007FFFFF)

#define AF_UART_BITS				((tU32)0x00000003)  //uart no modem AF
#define AF_UART0_MODEM_BITS			((tU32)0x000000FF)
#define AF_UART1_MODEM_BITS			((tU32)0x00100207)
#define AF_UART2_MODEM_BITS			((tU32)0x0000003F)
#define AF_UART3_MODEM_BITS			((tU32)0x0000000F)

#define AF_I2C_BITS					((tU32)0x00000003)  //I2C generic AF
#define AF_I2C0_A_BITS				((tU32)0x000000021) // I2C0 AF

#define AF_MSP_BITS					((tU32)0x0000000F)
#define AF_MSP_0_BITS				((tU32)0x00007E01)
#define AF_MSP_2_BITS				((tU32)0x0000000F)
#define AF_MSP_3_BITS				((tU32)0x00000063)

//#define AF_AUDIOOUT_BITS			((tU32)0x00000107)
//#define AF_AUDIOIN_BITS				((tU32)0x00000063)

#define AF_SSP_BITS					((tU32)0x0000000F)
#define AF_SSP_0_B_BITS				((tU32)0x00000063)

//! \def AF_UART0
//!      This define must be passed to function LLD_GPIO_EnAltFunction() in order to
//!      set the proper pin in the correct alternate mode. Refer to the datasheet for
//!      the mapping table between the GPIOs and the alternate functionalities available.
#define AF_UART0					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA 			 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)0 					 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_UART_BITS 		 << (tU32)AF_BITS_SHIFT)))

#define AF_UART0_MODEM				((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA  << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)0 					 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_UART0_MODEM_BITS << (tU32)AF_BITS_SHIFT)))

#define AF_UART1					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA  << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)14 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_UART_BITS 		 << (tU32)AF_BITS_SHIFT)))

#define AF_UART1_MODEM				((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA  << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)14 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_UART1_MODEM_BITS << (tU32)AF_BITS_SHIFT)))

#define AF_UART2					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA  << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)38 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_UART_BITS 		 << (tU32)AF_BITS_SHIFT)))

#define AF_UART2_MODEM				((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA  << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)38 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_UART2_MODEM_BITS << (tU32)AF_BITS_SHIFT)))

#define AF_UART3					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTB  << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)3 					 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_UART_BITS 		 << (tU32)AF_BITS_SHIFT)))

#define AF_UART3_MODEM				((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTB  << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)3 					 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_UART3_MODEM_BITS << (tU32)AF_BITS_SHIFT)))

#define AF_I2C_0_A					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTC  << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)2 					 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_I2C0_A_BITS 			 << (tU32)AF_BITS_SHIFT)))

#define AF_I2C_0_B					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA	 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)62 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_I2C_BITS 			 << (tU32)AF_BITS_SHIFT)))

#define AF_I2C_1_A					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTC  << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)0 					 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_I2C_BITS 			 << (tU32)AF_BITS_SHIFT)))

#define AF_I2C_1_B					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTC	 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)3 					 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_I2C_BITS 			 << (tU32)AF_BITS_SHIFT)))

#define AF_I2C_1_C					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA	 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)53 					 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_I2C_BITS 			 << (tU32)AF_BITS_SHIFT)))


#define AF_I2C_2_A					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTB 			 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)28 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_I2C_BITS 			 << (tU32)AF_BITS_SHIFT)))

#define AF_I2C_2_B					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTB 			 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)36 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_I2C_BITS 			 << (tU32)AF_BITS_SHIFT)))

#define AF_I2C_2_C					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTB 			 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)76 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_I2C_BITS 			 << (tU32)AF_BITS_SHIFT)))

#define AF_SSP_0_A					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA 			 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)58 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_SSP_BITS 			 << (tU32)AF_BITS_SHIFT)))

#define AF_SSP_0_B					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA 			 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)51 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_SSP_0_B_BITS 			 << (tU32)AF_BITS_SHIFT)))

#define AF_SSP_1_A					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA 			 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)44 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_SSP_BITS 			 << (tU32)AF_BITS_SHIFT)))
#define AF_MSP_0_A					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA 			 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)8 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_MSP_0_BITS 			 << (tU32)AF_BITS_SHIFT)))

#define AF_MSP_3_A					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTC 			 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)51 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_MSP_3_BITS 			 << (tU32)AF_BITS_SHIFT)))

//#define AF_AUDIOIN					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTC 			 << (tU32)AF_MODE_SHIFT)  | \
//										    ((tU32)51 				 << (tU32)AF_SHIFT_SHIFT) | \
//										    ((tU32)AF_AUDIOIN_BITS	 << (tU32)AF_BITS_SHIFT)))

//#define AF_MSP_1					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTA 			 << (tU32)AF_MODE_SHIFT)  | \
//										    ((tU32)8 				 << (tU32)AF_SHIFT_SHIFT) | \
//										    ((tU32)AF_MSP_BITS 			 << (tU32)AF_BITS_SHIFT)))

#define AF_MSP_2_A					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTC 			 << (tU32)AF_MODE_SHIFT)  | \
										    ((tU32)64 				 << (tU32)AF_SHIFT_SHIFT) | \
										    ((tU32)AF_MSP_2_BITS 			 << (tU32)AF_BITS_SHIFT)))

#define AF_MSP_3_A					((AltFunc_Ty)(((tU32)LLD_GPIO_MODE_ALTC 			 << (tU32)AF_MODE_SHIFT)  | \
									((tU32)51 				 << (tU32)AF_SHIFT_SHIFT) | \
									((tU32)AF_MSP_3_BITS 			 << (tU32)AF_BITS_SHIFT)))



//----------------------------------------------------------------------
// macros
//----------------------------------------------------------------------
//----------------------------------------------------------------------
//! \def GPIOid(x)
//!      This macro is used to access the specified GPIO registers.
//----------------------------------------------------------------------
#define GPIOid(x) ((GpioMap*)x)

//----------------------------------------------------------------------
// typedefs
//----------------------------------------------------------------------
//!
//! \typedef typedef tU32 LLD_GPIO_idTy
//!          Define as tU32 the LLD_GPIO_idTy type.
//!
typedef tU32 LLD_GPIO_idTy; // GPIO Id

//!
//! \typedef typedef tU32 AltFunc_Ty
//!          Define an union to divide the tU32 alt func in all field componing it .
//!
typedef  union AltFunc_Ty
	  {
	     struct
	     {
	        tU32 func:22;
	        tU32 pin:8;
	        tU32 AF:2;
	     }field;
	     tU32 peripheral;
	  } AltFunc_Ty;

//!
//! \enum  LLD_GPIO_StateTy
//! This enumerator indicates GPIO state:
//! - LLD_GPIO_LOW means that the GPIO state is ground
//! - LLD_GPIO_HIGH means that the GPIO state is Vcc.
//!
typedef enum                // GPIO pin states
{
    LLD_GPIO_LOW	= 0,
    LLD_GPIO_HIGH	= 1
} LLD_GPIO_StateTy;

//!
//! \enum LLD_GPIO_PinTy
//! This enumerator is used to address particular GPIO that belong to a port.
//! Each port has 32 GPIOs. If a function accept more than one GPIO a OR-ed
//! of more than one GPIO can be passed. To pass all 32 GPIO use
//! LLD_GPIO_ALL_PINS.
//!
typedef enum                // GPIO pin masks
{
    LLD_GPIO_PIN0   = BIT_0,
    LLD_GPIO_PIN1   = BIT_1,
    LLD_GPIO_PIN2   = BIT_2,
    LLD_GPIO_PIN3   = BIT_3,
    LLD_GPIO_PIN4   = BIT_4,
    LLD_GPIO_PIN5   = BIT_5,
    LLD_GPIO_PIN6   = BIT_6,
    LLD_GPIO_PIN7   = BIT_7,
    LLD_GPIO_PIN8   = BIT_8,
    LLD_GPIO_PIN9   = BIT_9,
    LLD_GPIO_PIN10  = BIT_10,
    LLD_GPIO_PIN11  = BIT_11,
    LLD_GPIO_PIN12  = BIT_12,
    LLD_GPIO_PIN13  = BIT_13,
    LLD_GPIO_PIN14  = BIT_14,
    LLD_GPIO_PIN15  = BIT_15,
    LLD_GPIO_PIN16  = BIT_16,
    LLD_GPIO_PIN17  = BIT_17,
    LLD_GPIO_PIN18  = BIT_18,
    LLD_GPIO_PIN19  = BIT_19,
    LLD_GPIO_PIN20  = BIT_20,
    LLD_GPIO_PIN21  = BIT_21,
    LLD_GPIO_PIN22  = BIT_22,
    LLD_GPIO_PIN23  = BIT_23,
    LLD_GPIO_PIN24  = BIT_24,
    LLD_GPIO_PIN25  = BIT_25,
    LLD_GPIO_PIN26  = BIT_26,
    LLD_GPIO_PIN27  = BIT_27,
    LLD_GPIO_PIN28  = BIT_28,
    LLD_GPIO_PIN29  = BIT_29,
    LLD_GPIO_PIN30  = BIT_30,
    LLD_GPIO_PIN31  = BIT_31
} LLD_GPIO_PinTy;

//!
//! \def LLD_GPIO_ALL_PINS	(LLD_GPIO_PinTy)0xFFFFFFFF
//!      Use this define to indicate all pins of a GPIO port.
//!
#define LLD_GPIO_ALL_PINS	(LLD_GPIO_PinTy)0xFFFFFFFF

typedef enum
{
    LLD_GPIOCH0,
    LLD_GPIOCH1,
    LLD_GPIOCH2,
    LLD_GPIOCH3,
    LLD_GPIOCH4,
    LLD_GPIOCH5,
    LLD_GPIOCH6,
    LLD_GPIOCH7,
    LLD_GPIOCH8,
    LLD_GPIOCH9,
    LLD_GPIOCH10,
    LLD_GPIOCH11,
    LLD_GPIOCH12,
    LLD_GPIOCH13,
    LLD_GPIOCH14,
    LLD_GPIOCH15,
    LLD_GPIOCH16,
    LLD_GPIOCH17,
    LLD_GPIOCH18,
    LLD_GPIOCH19,
    LLD_GPIOCH20,
    LLD_GPIOCH21,
    LLD_GPIOCH22,
    LLD_GPIOCH23,
    LLD_GPIOCH24,
    LLD_GPIOCH25,
    LLD_GPIOCH26,
    LLD_GPIOCH27,
    LLD_GPIOCH28,
    LLD_GPIOCH29,
    LLD_GPIOCH30,
    LLD_GPIOCH31,
    LLD_GPIOCH32,
    LLD_GPIOCH33,
    LLD_GPIOCH34,
    LLD_GPIOCH35,
    LLD_GPIOCH36,
    LLD_GPIOCH37,
    LLD_GPIOCH38,
    LLD_GPIOCH39,
    LLD_GPIOCH40,
    LLD_GPIOCH41,
    LLD_GPIOCH42,
    LLD_GPIOCH43,
    LLD_GPIOCH44,
    LLD_GPIOCH45,
    LLD_GPIOCH46,
    LLD_GPIOCH47,
    LLD_GPIOCH48,
    LLD_GPIOCH49,
    LLD_GPIOCH50,
    LLD_GPIOCH51,
    LLD_GPIOCH52,
    LLD_GPIOCH53,
    LLD_GPIOCH54,
    LLD_GPIOCH55,
    LLD_GPIOCH56,
    LLD_GPIOCH57,
    LLD_GPIOCH58,
    LLD_GPIOCH59,
    LLD_GPIOCH60,
    LLD_GPIOCH61,
    LLD_GPIOCH62,
    LLD_GPIOCH63,
    LLD_GPIOCH64,
    LLD_GPIOCH65,
    LLD_GPIOCH66,
    LLD_GPIOCH67,
    LLD_GPIOCH68,
    LLD_GPIOCH69,
    LLD_GPIOCH70,
    LLD_GPIOCH71,
    LLD_GPIOCH72,
    LLD_GPIOCH73,
    LLD_GPIOCH74,
    LLD_GPIOCH75,
    LLD_GPIOCH76,
    LLD_GPIOCH77,
    LLD_GPIOCH78,
    LLD_GPIOCH79,
    LLD_GPIOCH80,
    LLD_GPIOCH81,
    LLD_GPIOCH82,
    LLD_GPIOCH83,
    LLD_GPIOCH84,
    LLD_GPIOCH85,
    LLD_GPIOCH86,
    LLD_GPIOCH87,
    LLD_GPIOCH88,
    LLD_GPIOCH89,
    LLD_GPIOCH90,
    LLD_GPIOCH91,
    LLD_GPIOCH92,
    LLD_GPIOCH93,
    LLD_GPIOCH94,
    LLD_GPIOCH95,
    LLD_GPIOCH96,
    LLD_GPIOCH97,
    LLD_GPIOCH98,
    LLD_GPIOCH99,
    LLD_GPIOCH100,
    LLD_GPIOCH101,
    LLD_GPIOCH102,
    LLD_GPIOCH103,
    LLD_GPIOCH104,
    LLD_GPIOCH105,
    LLD_GPIOCH106,
    LLD_GPIOCH107,
    LLD_GPIOCH108,
    LLD_GPIOCH109,
    LLD_GPIOCH110,
    LLD_GPIOCH111,
    LLD_GPIOCH112,
    LLD_GPIOCH113,
    LLD_GPIOCH114,
    LLD_GPIOCH115,
    LLD_GPIOCH116,
    LLD_GPIOCH117,
    LLD_GPIOCH118,
    LLD_GPIOCH119,
    LLD_GPIOCH120,
    LLD_GPIOCH121,
    LLD_GPIOCH122,
    LLD_GPIOCH123,
    LLD_GPIOCH124,
    LLD_GPIOCH125,
    LLD_GPIOCH126,
    LLD_GPIOCH127,
#ifdef PLATFORM_IS_CARTESIOPLUS
    LLD_GPIOCH128,
    LLD_GPIOCH129,
    LLD_GPIOCH130,
    LLD_GPIOCH131,
    LLD_GPIOCH132,
    LLD_GPIOCH133,
    LLD_GPIOCH134,
    LLD_GPIOCH135,
    LLD_GPIOCH136,
    LLD_GPIOCH137,
    LLD_GPIOCH138,
    LLD_GPIOCH139,
    LLD_GPIOCH140,
    LLD_GPIOCH141,
    LLD_GPIOCH142,
    LLD_GPIOCH143,
    LLD_GPIOCH144,
    LLD_GPIOCH145,
    LLD_GPIOCH146,
    LLD_GPIOCH147,
    LLD_GPIOCH148,
    LLD_GPIOCH149,
    LLD_GPIOCH150,
    LLD_GPIOCH151,
    LLD_GPIOCH152,
    LLD_GPIOCH153,
    LLD_GPIOCH154,
    LLD_GPIOCH155,
    LLD_GPIOCH156,
    LLD_GPIOCH157,
    LLD_GPIOCH158,
    LLD_GPIOCH159,
#endif
    LLD_GPIOCHUNDEF
} LLD_GPIO_ChanTy;

typedef enum                // GPIO pin states
{
	LLD_GPIO_PULL_ENABLE,
	LLD_GPIO_PULL_DISABLE
} LLD_GPIO_PullNet_Ty;

//!
//! \enum LLD_GPIO_DirectionTy
//! This enumerator indicates GPIO direction:
//! - LLD_GPIO_INPUT
//! - LLD_GPIO_OUTPUT.
//!
typedef enum		        // GPIO direction types
{
    LLD_GPIO_INPUT,
    LLD_GPIO_OUTPUT
} LLD_GPIO_DirectionTy;

//!
//! \enum LLD_GPIO_IntTy
//! This enumerator indicates GPIO interrupt type:
//! - LLD_GPIO_DISABLED_INT, interrupt disabled
//! - LLD_GPIO_BOTH_EDGES_INT, interrupt on both edges
//! - LLD_GPIO_RISE_EDGE_INT, interrupt on the rising edge
//! - LLD_GPIO_FALL_EDGE_INT, interrupt on the falling edge.
//!
typedef enum		        // GPIO pin interrupt event types
{
	LLD_GPIO_DISABLED_INT,
    LLD_GPIO_BOTH_EDGES_INT,
    LLD_GPIO_RISE_EDGE_INT,
    LLD_GPIO_FALL_EDGE_INT
} LLD_GPIO_IntTy;


//!
//! \enum LLD_GPIO_PinModeTy
//! This enumerator indicates GPIO mode:
//! - LLD_GPIO_MODE_SOFTWARE, sw mode
//! - LLD_GPIO_MODE_ALTA, alternate mode A
//! - LLD_GPIO_MODE_ALTB, alternate mode B
//! - LLD_GPIO_MODE_ALTC, alternate mode C.
//!
typedef enum
{
	LLD_GPIO_MODE_SOFTWARE 	=0x0,
	LLD_GPIO_MODE_ALTA		=0x1,
	LLD_GPIO_MODE_ALTB		=0x2,
	LLD_GPIO_MODE_ALTC		=0x3
} LLD_GPIO_PinModeTy;

//!
//! \enum LLD_GPIO_SleepModeTy
//! This enumerator indicates GPIO mode on IC set into SLEEP mode:
//! - LLD_GPIO_DEFAULT_SLEEP, GPIO mode set to input
//! - LLD_GPIO_DRIVEN_SLEEP, GPIO mode unchanged.
//!
typedef enum				// GPIO set sleep mode behavior
{
    LLD_GPIO_DEFAULT_SLEEP,
    LLD_GPIO_DRIVEN_SLEEP
} LLD_GPIO_SleepModeTy;

//!
//! \struct LLD_GPIO_IdentifierTy
//! This structure reports the GPIO port identifier.
//!
typedef struct
{
	//! The cell part number lower part (8 bits)
	tU8 partNumber0;
	//! The cell part number higher part (8 bits)
	tU8 partNumber1;
	//! The cell designer number lower part (8 bits)
	tU8 designer0;
	//! The cell designer number higher part (8 bits)
	tU8 designer1;
	//! Cell revision
	tU8 revision;
	//! Cell configuration
	tU8 configuration;
	//! Cell ID field 0
	tU8 GPIOPCellID0;
	//! Cell ID field 1
	tU8 GPIOPCellID1;
	//! Cell ID field 2
	tU8 GPIOPCellID2;
	//! Cell ID field 3
	tU8 GPIOPCellID3;
} LLD_GPIO_IdentifierTy;

//----------------------------------------------------------------------
// function prototypes
//----------------------------------------------------------------------
// Following function group addresses the GPIO as bank, so:
// - BANK: 0 -> 3
// - GPIO: 0 -> 31
extern tVoid 	        LLD_GPIO_SetControlMode 	(LLD_GPIO_idTy id, LLD_GPIO_PinTy pins, LLD_GPIO_PinModeTy mode);
extern LLD_GPIO_PinModeTy	LLD_GPIO_GetControlMode	(LLD_GPIO_idTy id, LLD_GPIO_PinTy pin);

extern tVoid            LLD_GPIO_SetState      	 	(LLD_GPIO_idTy id, LLD_GPIO_PinTy pins, LLD_GPIO_StateTy state);
extern tVoid 	        LLD_GPIO_SetStateHigh  	 	(LLD_GPIO_idTy id, LLD_GPIO_PinTy pins);
extern tVoid 	        LLD_GPIO_SetStateLow	  	(LLD_GPIO_idTy id, LLD_GPIO_PinTy pins);
extern LLD_GPIO_StateTy LLD_GPIO_GetPinState		(LLD_GPIO_idTy id, LLD_GPIO_PinTy pin);
extern tU32 			LLD_GPIO_GetPortState		(LLD_GPIO_idTy id);

extern tVoid 	        LLD_GPIO_SetDirection  	 	(LLD_GPIO_idTy id, LLD_GPIO_PinTy pins, LLD_GPIO_DirectionTy direction);
extern tVoid 	        LLD_GPIO_SetDirectionInput  (LLD_GPIO_idTy id, LLD_GPIO_PinTy pins);
extern tVoid 	        LLD_GPIO_SetDirectionOutput (LLD_GPIO_idTy id, LLD_GPIO_PinTy pins);
extern tU32             LLD_GPIO_GetDirection  	 	(LLD_GPIO_idTy id, LLD_GPIO_PinTy pins);

extern tVoid			LLD_GPIO_SetSleepModeReg	(LLD_GPIO_idTy id, LLD_GPIO_PinTy pins, LLD_GPIO_SleepModeTy mode);
extern tU32			 	LLD_GPIO_GetSleepModeReg	(LLD_GPIO_idTy id);

extern tVoid            LLD_GPIO_SetInterruptType   (LLD_GPIO_idTy id, LLD_GPIO_PinTy pins, LLD_GPIO_IntTy type);
extern LLD_GPIO_IntTy	LLD_GPIO_GetInterruptType   (LLD_GPIO_idTy id, LLD_GPIO_PinTy pin);
extern tVoid            LLD_GPIO_InterruptDisable	(LLD_GPIO_idTy id, LLD_GPIO_PinTy pins);

extern tVoid			LLD_GPIO_DisablePull		(LLD_GPIO_idTy id, LLD_GPIO_PinTy pins);
extern tVoid			LLD_GPIO_EnablePull		 	(LLD_GPIO_idTy id, LLD_GPIO_PinTy pins);
extern tU32			 	LLD_GPIO_GetPullState		(LLD_GPIO_idTy id);

extern tU32             LLD_GPIO_GetInterruptStatus (LLD_GPIO_idTy id);
extern tVoid            LLD_GPIO_ClearInterrupt     (LLD_GPIO_idTy id, LLD_GPIO_PinTy pins);

// Following function addresses GPIO linearly:
// - PIN: 0 -> 127
extern tVoid 			LLD_GPIO_ReadPin			(LLD_GPIO_ChanTy pin, LLD_GPIO_StateTy *p_value);
extern tVoid 			LLD_GPIO_SetPinHigh			(LLD_GPIO_ChanTy pin);
extern tVoid 			LLD_GPIO_SetPinLow			(LLD_GPIO_ChanTy pin);

extern tVoid            LLD_GPIO_SetPinMode         (LLD_GPIO_ChanTy pin, LLD_GPIO_PinModeTy mode);

extern tVoid 			LLD_GPIO_SetPinPullNet		(LLD_GPIO_ChanTy pin, LLD_GPIO_PullNet_Ty pullNet);
extern tVoid 			LLD_GPIO_GetPinPullNet		(LLD_GPIO_ChanTy pin, LLD_GPIO_PullNet_Ty *pPullNet);

extern tVoid 			LLD_GPIO_SetPinDirection	(LLD_GPIO_ChanTy pin, LLD_GPIO_DirectionTy direction);
extern tVoid 			LLD_GPIO_GetPinDirection	(LLD_GPIO_ChanTy pin, LLD_GPIO_DirectionTy *pDirection);

extern tVoid 			LLD_GPIO_SetSleepMode		(LLD_GPIO_ChanTy pin, LLD_GPIO_SleepModeTy mode);
extern tVoid 			LLD_GPIO_GetSleepMode		(LLD_GPIO_ChanTy pin, LLD_GPIO_SleepModeTy *pmode);

extern tVoid 			LLD_GPIO_PinIRQEnable		(LLD_GPIO_ChanTy pin, LLD_GPIO_IntTy irq);
extern tVoid 			LLD_GPIO_PinIRQDisable		(LLD_GPIO_ChanTy pin);
extern tVoid 			LLD_GPIO_PinIRQGetConfig	(LLD_GPIO_ChanTy pin, LLD_GPIO_IntTy *irq);
extern tVoid 			LLD_GPIO_PinIRQClear		(LLD_GPIO_ChanTy pin);

extern tBool 			LLD_GPIO_EnAltFunction		(AltFunc_Ty per);
extern tVoid 			LLD_GPIO_DisAltFunction		(AltFunc_Ty per);

// Cell ID (test only)
extern tBool			LLD_GPIO_CellID			 	(LLD_GPIO_idTy id, LLD_GPIO_IdentifierTy *idPtr);

#ifdef __cplusplus
}
#endif

#endif  // _LLD_GPIO_H_

// End of file
