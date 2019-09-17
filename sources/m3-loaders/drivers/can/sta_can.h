/******************** (C) COPYRIGHT 2013 STMicroelectronics ********************
* File Name          : sta_can.h
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
*******************************************************************************/

#ifndef _CAN_H_
#define _CAN_H_

#include "sta_map.h"
#include "sta_can_p.h"

#define CLOCK_MHz 51.2

#define CAN_E_TIMEOUT		(uint8_t)0x10		// REMOVE IT AFTER INTEGRATION WITH CAN_INTERFACE !!!

#define CAN_DEV_ERROR_DETECT		// Development error detection

#define CAN_E_PARAM_CONFIG	    (uint8_t)0x01
#define CAN_E_PARAM_HANDLE	    (uint8_t)0x02
#define CAN_E_PARAM_PDUINFO	    (uint8_t)0x03
#define CAN_E_PARAM_CONTROLLER	(uint8_t)0x04
#define CAN_E_UNINIT		    (uint8_t)0x05
#define CAN_E_TRANSITION	    (uint8_t)0x06

// Sergio 30/08/2005
// added the codes to identify the APIs (see Autosar CAN driver spec)
#define CAN_MODULE_ID       0  // to search the real value in the spec "defined in Wp1.1.2 Basic Software Module List"
#define CAN_MAJOR_VERSION   0
#define CAN_MINOR_VERSION   0
#define	CAN_PATCH_VERSION	0
#define CAN_VENDOR_ID		0
#define	CAN_DEVICE_ID		0

// Each API is identifie with a service nymber (see Specification of Module CAN Driver 1.0.0)
#define CAN_INIT_ID	        			    (uint8_t)0x00
#define CAN_MAIN_FUNCTION_ID			    (uint8_t)0x01
#define CAN_INITCONTROLLER_ID			    (uint8_t)0x02
#define CAN_SETCONTROLLERMODE_ID		    (uint8_t)0x03
#define CAN_DISABLECONTROLLERINTERRUPTS_ID	(uint8_t)0x04
#define CAN_ENABLECONTROLLERINTERRUPTS_ID	(uint8_t)0x05
#define CAN_WRITE_ID					    (uint8_t)0x06
#define CAN_RXINDICATION_ID				    (uint8_t)0x07
#define CAN_TXCNFOK_ID					    (uint8_t)0x08
#define CAN_TXCNFABORT_ID				    (uint8_t)0x09
#define CAN_CONTROLLERWAKEUP_ID			    (uint8_t)0x0A
#define CAN_CONTROLLERBUSOFF_ID			    (uint8_t)0x0B

typedef	uint8_t 	    PduIdType;	    	// Type PDU
typedef uint8_t   	Can_HwHandleType;	// Identifies a HTH
typedef uint32_t     Can_IdType;	    	// Frame ID value.

// CallBack pointer function definition
typedef void (*RxIndication)	(Can_HwHandleType, Can_IdType, uint8_t, uint8_t *);
typedef void (*TxCnf)	    	(PduIdType);
typedef void (*ControllerState) (uint8_t);

typedef struct
{
	Can_IdType	 Id;   	 // Frame ID value.
	uint8_t			 Lenght; // CAN data length of frame.
	uint8_t			 *Sdu; 	 // Pointer to SDU
} Can_PduType;

typedef enum
{
  STANDARD = 0,
  EXTENDED
} Can_Id_Format;

// Controller Config Type
// -------------------------------
typedef	struct
{
	uint8_t		    Controller_Number;
	volatile    CCan *	Controller_Symbolic_Name;
	uint16_t		    Baudrate;
	uint16_t		    Allowed_Tollerance;
	uint16_t		    Propagation_delay;
	uint16_t		    Tseg1;
	uint16_t		    Tseg2;
} Can_ControllerConfigType;

// private config data passed from conmmon configuration

typedef	struct
{
	uint8_t			        Vendor_Id;	//	Changed by ST
	uint8_t			        Device_Id;	//	Changed by ST
	uint8_t			        Module_Id;	// (it was taken from SPAL spec.)
	uint8_t			        Major_Version;
	uint8_t 			        Minor_Version;
	uint8_t			        Driver_Patch_Version;
	uint8_t			        Objects_num;	//	Number of CAN Objects for each CAN controller
	Can_Id_Format		Id_format;	//	Supported frame	ID format (Standard/Extended)
	RxIndication        Can_RxIndication;
	TxCnf               Can_TxCnfOk;
	TxCnf               Can_TxCnfAbort;
	ControllerState     Can_ControllerWakeup;
	ControllerState     Can_ControllerBusOff;
} Can_GlobalConfigData;

typedef struct
{
	volatile CCan * CAN_ptr; // pointer to HW CAN
	uint8_t MailBox;			// internal numbering of MO inside of CAN controller
	uint8_t can_controller;	// internal numbering of CAN controllers
}HandlerMO_private;		// Hardware object handle for each controller
												// it is used for optimized access to data in ISR
// Operative Mode
// -------------------------------
typedef	enum
{
	RX_Basic = 0,
    RX_Full,
	TX_Basic,
	TX_Full
}Can_Operative_mode;

typedef	struct Can_HW_Object   // AUTOSAR specs.
{
	uint8_t			        Associated_Mask_number;
	uint8_t			        Handle_number;
	Can_Operative_mode	Type;
	Can_Id_Format		Id_Type;		// !!! changed by LC from Can_IdType !!!
	Can_IdType		    Id_Value;		// this should to be 16 bits or 32 bits depends on the Id_Type
	uint8_t			        Dlc_Value;
}Can_HW_Object;
											// it contains no. of 'MailBox' = MO
// Filter Mask
// -------------------------------
typedef	struct
{
	uint8_t	Mask_number;
	uint8_t	Associated_Controller;
	uint32_t	Filter_Mask_Value;
}Can_Filter_Mask;

typedef	struct
{
         // there is necessary to put 33 (MAX_HW_OBJECTS + 1), because 0 is not used
	PduIdType	SwPduHandle[MAX_HW_OBJECTS + 1];	// Handler of upper layer
	Can_HW_Object * HW_Object_ptr[MAX_HW_OBJECTS + 1]; // pointer to HW_Object in config 'FLASH' area
	Can_Filter_Mask * Filter_Mask_ptr[MAX_HW_OBJECTS + 1]; // pointer to HW_Object in config 'FLASH' area
	uint16_t state;			// this byte is used for preventing nested handling of some states like as BusOff
}ControllerMO_private;


typedef enum
{
	CAN_T_STOP  = 0,
	CAN_T_START,
	CAN_T_SLEEP,
	CAN_T_WAKEUP
} Can_StateTransitionType;

typedef enum
{
	CAN_OK  = 0,
	CAN_BUSY,
	CAN_WAKEUP
} Can_ReturnType;


typedef struct
{
	uint8_t			        Controller_num; // Number of CAN controllers
	uint8_t			        Objects_num;	//	Number of CAN Objects for each CAN controller
	Can_Id_Format		Id_format;	//	Supported frame ID format (Standard/Extended)
	uint8_t			        Vendor_Id;	//	Changed by ST
	uint8_t			        Device_Id;	//	Changed by ST
	uint8_t			        Module_Id;	// 	it was taken from SPAL spec.
	uint8_t			        Major_Version;	//
	uint8_t			        Minor_Version;	//
	uint8_t			        Driver_Patch_Version;
	Can_ControllerConfigType *Controllers_ptr; 	// List of configuration records for all CAN controllers.
  	Can_Filter_Mask	    *Object_Filter_Mask;
 	Can_HW_Object	    *HW_Object;
	RxIndication  	    Can_RxIndication; // Function pointer to callback CanIf_RxIndication defined in Can Interface
	TxCnf 		        Can_TxCnfOk; 	  // Function pointer to callback CanIf_TxCnfOk defined in Can Interface
	TxCnf 		        Can_TxCnfAbort;   // Function pointer to callback CanIf_TxCnfAbort defined in Can Interface
	ControllerState     Can_ControllerWakeup; // Function pointer to callback CanIf_ControllerWakeup defined in Can Interface
	ControllerState     Can_ControllerBusOff; // Function pointer to callback CanIf_ControllerBusOff
	uint32_t input_clock;
} Can_ConfigType;

// HW Object
// -------------------------------

#define STATE_WRITE_REENTRANT	(uint16_t)0x8000

/* Define the maximum number of Time Quantum that compose a Bit in the CAN message */
#define MAX_NUM_TIME_QUANTUM_PER_BIT   20

// 2 highest bits indicates CAN channel (0,1,2,3)
#define GET_CAN_CHANNEL(object)  ((object & 0xC0) >> 6)
#define CAN_CHANNEL_MASK 0x3F
#define FAIL_REGWRITE   0xF000

#define CPU_CLOCK       (CLOCK_MHz * 1000000)

#ifndef MIN
#define MIN(a,b) (((a) > (b))?(b):(a))
#endif

#ifndef MAX
#define MAX(a,b) (((a) > (b))?(a):(b))
#endif

#define CAN_CFGPRM_NONBLOCKING_IFWAIT

/***************************************************************************************************************/
/* Prototypes */
/***************************************************************************************************************/
void    Can_Init(Can_ConfigType* Config);
void    CCanLLD_StartController(volatile CCan * CAN_ptr);
void    CCanLLD_Init		  (Can_ControllerConfigType * Controller_ptr);
void    CCanLLD_InitController   (uint8_t Controller, Can_ControllerConfigType *Config);
void    CCanLLD_EnableControllerInterrupts (uint8_t Controller);
void    CCanLLD_DisableControllerInterrupts (uint8_t Controller);
Can_ReturnType CCanLLD_Write (uint8_t Controller, Can_HwHandleType Hth, Can_PduType *PduInfo,PduIdType SwPduHandle );
Can_ReturnType CCanLLD_SetControllerMode (uint8_t Controller, Can_StateTransitionType  Transition);

/******************** Low Layer Private API *******************************************************************************************/
uint16_t     CCanLLD_InitializeControllerDefaultState(volatile CCan * CAN_ptr, uint8_t Controller, uint16_t baudrate, uint16_t mode);
void    CCanLLD_SetRxMsgObj(volatile CCan * CAN_ptr, uint8_t Object, Can_Id_Format idType, uint32_t range_start, uint32_t range_end, uint8_t EndOfBlock);
void    CCanLLD_SetTxMsgObj(volatile CCan * CAN_ptr, uint8_t Object, Can_Id_Format idType, uint32_t range_start, uint8_t DLC_Val);
void    CCanLLD_MObject_UpdateValid(volatile CCan * CAN_ptr, uint8_t object, uint8_t state);
void    CCanLLD_InitControllerPins(uint8_t Controller);
uint8_t      CCanLLD_CheckController(uint8_t Controller);
void    CCanLLD_StopController(volatile CCan * CAN_ptr);
void    CCanLLD_StartController(volatile CCan * CAN_ptr);
void    CCanLLD_EnableWakeUp(uint8_t Controller);
void    CCanLLD_DisableWakeUp(uint8_t Controller);
void    CCanLLD_DisableCLK(uint8_t Controller);
void    CCanLLD_EnableCLK(uint8_t Controller);

#define	Can_InitController CCanLLD_InitController
#define Can_MainFunction CCanLLD_MainFunction
#define Can_SetControllerMode CCanLLD_SetControllerMode
#define Can_DisableControllerInterrupts CCanLLD_DisableControllerInterrupts
#define Can_EnableControllerInterrupts CCanLLD_EnableControllerInterrupts
#define Can_Write CCanLLD_Write

void Can_TxCnfOk(PduIdType	SwPduHandle);
void Can_TxCnfAbort(PduIdType	SwPduHandle);
void Can_RxIndication(uint8_t Handle_number, uint32_t Identifier, uint8_t CanDlc, uint8_t *CanSduPtr);
void Can_ControllerBusOff(uint8_t Controller);
void Can_ControllerWakeup(uint8_t Controller);

void CAN_Engine_Start(RxIndication rxcb, uint32_t input_clock);

// ***************************** Start of can_cbk.h *************************
#ifndef CAN_CBK_HEADER
#define CAN_CBK_HEADER

// Interface Driver Prototypes for callback functions
// --------------------------------------------------
/*extern void CanIf_RxIndication (Can_HwHandleType Hrh,Can_IdType Identifier,uint8_t CanDlc,uint8_t *CanSduPtr);
extern void CanIf_TxCnfOk(PduIdType SwPduHandle);
extern void CanIf_TxCnfAbort ( PduIdType SwPduHandle );
extern void CanIf_ControllerWakeup (uint8_t Controller);
extern void CanIf_TxConfirmation( PduIdType CanTxPduId);
extern void CanIf_ControllerBusOff(uint8_t Controller);*/
#endif

// ***************************** End of can_cbk.h *************************


// ***************************** Start of can_c_consts.h *************************


#ifndef _CAN_C_CONSTS_
#define _CAN_C_CONSTS_


// ****************************************************************************
// ************************** Configuration Constants *************************
// ****************************************************************************



// ****************************************************************************
// ********************** External Constants and Variables ********************
// ****************************************************************************


//Control register's constants *******************************
//INIT bit in CAN's Control reg
#define CANC_INITIALIZATION_EN         0x0001           //software initialization of the CAN controller
//IE bit in CAN's Control reg
#define CANC_INTERRUPT_EN              0x0002           //global interrupt enable
//SIE bit in CAN's Control reg
#define CANC_STATUS_INTERRUPT_EN       0x0004           //Interrupt on message receive/send or bus error
//EIE bit in CAN's Control reg
#define CANC_ERROR_INTERRUPT_EN        0x0008           //Interrupt on change of BOFF or EWARN bit
//DAR bit in CAN's Control reg
#define CANC_AUTORETRANS_DISABLE       0x0020           //disable automatic retransmission
//CCE bit in CAN's Control reg
#define CANC_CONFIG_CHANGE_EN          0x0040           //allows access to bit timing register
//TST bit in CAN's Control reg
#define CANC_TEST_MODE_EN              0x0080           //allows write access to test register

//Status register's constants *******************************
//LEC bits in CAN's Status reg
#define CANC_LEC_MASK                  0x0007           //mask to access LEC bits
#define CANC_LEC_NO_ERROR              0x0000           //
#define CANC_LEC_STUFF_ERROR           0x0001           //
#define CANC_LEC_FORM_ERROR            0x0002           //
#define CANC_LEC_ACK_ERROR             0x0003           //
#define CANC_LEC_BIT1_ERROR            0x0004           //
#define CANC_LEC_BIT0_ERROR            0x0005           //
#define CANC_LEC_CRC_ERROR             0x0006           //

//TXOK bit in CAN's Status reg
#define CANC_TRANSMIT_OK               (uint16_t)0x0008   //message was send successfully and acknowledged
                                                        //Has to be reset by software!
//RXOK bit in CAN's Status reg
#define CANC_RECEIVE_OK                (uint16_t)0x0010   //message was received successfully

//EPASS bit in CAN's Status reg
#define CANC_ERROR_PASSIVE             (uint16_t)0x0020   //indicates error passive or active status

//EWRN bit in CAN's Status reg
#define CANC_ERROR_WARNING             (uint16_t)0x0040   //indicates that error counter
                                                        //has reached warning limit

//BOFF bit in CAN's Status reg
#define CANC_BUS_OFF                   (uint16_t)0x0080   //indicates that CAN cell is
                                                        //in BusOff

//CAN Message Interface constants ***************************
//Command request bits

//Busy bit
#define CANC_IF_BUSY                   0x8000           //Busy bit

//Command mask bits (CANxIFxCM)
#define CANC_IF_DATA_B                 0x0001           //Data B
#define CANC_IF_DATA_A                 0x0002           //Data A
#define CANC_IF_TXR_ND                 0x0004           //TxRequest/NewDat
#define CANC_IF_CLR_IP                 0x0008           //Clear Interrupt Pend. Bit
#define CANC_IF_CNTRL                  0x0010           //Control bits of msg. object
#define CANC_IF_ARB                    0x0020           //Arbitration bits
#define CANC_IF_MASK                   0x0040           //Mask bits
#define CANC_IF_WRRD                   0x0080           //Write to RAM
#define CANC_IF_ALL                    0x00FF           //All bits

//Mask Register Bits
#define CANC_MO_IF_MXTD                0x8000           //Xtd
#define CANC_MO_IF_MDIR                0x4000           //Dir

//Arbitration Register Bits

#define CANC_MO_IF_MSGVAL              0x8000           //MsgVal
#define CANC_MO_IF_ID_LONG             0x4000           //this MO uses long (29bit) identificator
#define CANC_MO_IF_ID_SHORT            0x0000           //this MO uses short (11bit) identificator
#define CANC_MO_IF_DIRECTION_TX        0x2000           //set MO as transmit
#define CANC_MO_IF_DIRECTION_RX        0x0000           //set MO as receive

//Message Control Register Bits
#define CANC_M0_IF_NEWDAT              (uint16_t) 0x8000  //NewDat
#define CANC_MO_IF_MSGLST              (uint16_t) 0x4000  //MsgLst
#define CANC_MO_IF_INTPND              (uint16_t) 0x2000  //IntPnd
#define CANC_MO_IF_UMASK               (uint16_t) 0x1000  //UMask
#define CANC_MO_IF_TXIE                (uint16_t) 0x0800  //TxIE
#define CANC_MO_IF_RXIE                (uint16_t) 0x0400  //RxIE
#define CANC_MO_IF_RMTEN               (uint16_t) 0x0200  //RmtEn
#define CANC_MO_IF_TXRQST              (uint16_t) 0x0100  //TxRqst
#define CANC_MO_IF_EOB                 (uint16_t) 0x0080  //EoB

#define CANC_MO_DATALENGTH_0           0x00             //specify length of data bytes in the MO
#define CANC_MO_DATALENGTH_1           0x10
#define CANC_MO_DATALENGTH_2           0x20
#define CANC_MO_DATALENGTH_3           0x30
#define CANC_MO_DATALENGTH_4           0x40
#define CANC_MO_DATALENGTH_5           0x50
#define CANC_MO_DATALENGTH_6           0x60
#define CANC_MO_DATALENGTH_7           0x70
#define CANC_MO_DATALENGTH_8           0x80


//Converts short message ID (11bits) to form needed by IFx Mask register
#define CANC_COMPUTE_MASKSHORT(id)   ((uint16_t)(((uint32_t)id << 2) & 0x1FFC))

//Converts long message ID (29bits) to form needed by IFx Mask register
#define CANC_COMPUTE_MASKLONG_U(id)  (((uint16_t)(id >> 16)) & 0x1FFF)

//Converts long message ID (29bits) to form needed by IFx Mask register
#define CANC_COMPUTE_MASKLONG_L(id)  ((uint16_t)id)

//Macros for data transfer between Message Objects and Interfaces

//just waits for a while interface to be ready again

#ifdef CAN_CFGPRM_NONBLOCKING_IFWAIT
    #define  CANC_IF_WAIT(can,ifc) __nop();__nop();__nop();\
//                                   if (CAN##can##.IF##ifc##CR.reg & CANC_IF_BUSY){__nop();__nop();__nop();}
#else
    #define  CANC_IF_WAIT(can,ifc) while (CAN##can##.IF##ifc##CR.reg & CANC_IF_BUSY)
#endif

//takes data from message object 'mo' and stores them to interface 'ifc'
#define CANC_IF_FILL(can,ifc,mo,what) CAN##can##.IF##ifc##CMR.reg = what;\
                                      CAN##can##.IF##ifc##CR.reg = mo

//takes data from interface 'ifc' and stores them to message object 'mo'
#define CANC_IF_FLUSH(can,ifc,mo,what) CAN##can##.IF##ifc##CMR.reg = CANC_IF_WRRD | what;\
                                       CAN##can##.IF##ifc##CR.reg = mo

//takes data from message object 'mo' and stores them to interface 'ifc'
//it waits for a while interface to be ready again
#define CANC_IF_FILL_WAIT(can,ifc,mo,what) CAN##can##.IF##ifc##CMR.reg = what;\
                                           CAN##can##.IF##ifc##CR.reg = mo;\
                                           CANC_IF_WAIT(can,ifc)

//takes data from interface 'ifc' and stores them to message object 'mo'
//it waits for a while interface to be ready again
#define CANC_IF_FLUSH_WAIT(can,ifc,mo,what) CAN##can##.IF##ifc##CMR.reg = CANC_IF_WRRD | what;\
                                            CAN##can##.IF##ifc##CR.reg = mo;\
                                            CANC_IF_WAIT(can,ifc)



#ifdef CAN_CFGPRM_NONBLOCKING_IFWAIT
     #define  CAN_IF_WAIT(ifc) __nop();__nop();__nop();\
                                       if (CAN_ptr->IF##ifc##CR.reg & CANC_IF_BUSY){__nop();__nop();__nop();}
#else
     #define  CAN_IF_WAIT(ifc) while (CAN_ptr->IF##ifc##CR.reg & CANC_IF_BUSY)
#endif


    //takes data from message object 'mo' and stores them to interface 'ifc'
#define CAN_IF_FILL(ifc,mo,what) CAN_ptr->.IF##ifc##CMR.reg = what;\
                                          CAN_ptr->IF##ifc##CR.reg = mo

    //takes data from interface 'ifc' and stores them to message object 'mo'
#define CAN_IF_FLUSH(ifc,mo,what) CAN_ptr->IF##ifc##CMR.reg = CANC_IF_WRRD | what;\
                                           CAN_ptr->IF##ifc##CR.reg = mo

    //takes data from message object 'mo' and stores them to interface 'ifc'
    //it waits for a while interface to be ready again
#define CAN_IF_FILL_WAIT(ifc,mo,what) CAN_ptr->IF##ifc##CMR.reg = what;\
                                               CAN_ptr->IF##ifc##CR.reg = mo;\
                                               CAN_IF_WAIT(ifc)

    //takes data from interface 'ifc' and stores them to message object 'mo'
    //it waits for a while interface to be ready again
#define CAN_IF_FLUSH_WAIT(ifc,mo,what) CAN_ptr->IF##ifc##CMR.reg = CANC_IF_WRRD | what;\
                                                CAN_ptr->IF##ifc##CR.reg = mo;\
                                                CAN_IF_WAIT(ifc)


#define	RANGE_ID_MSK(range_start, range_end)	(~((range_end) - (range_start)))
#define	RANGE_ID_ARB(range_start, range_end)	((range_start) & (range_end))

#endif


extern const			Can_ControllerConfigType	Controllers_ptr_Config[];
extern const			Can_Filter_Mask				Object_Filters_Config[];
extern Can_HW_Object	HW_Objects_Config[];

#endif

// ***************************** End of can_c_consts.h *************************


