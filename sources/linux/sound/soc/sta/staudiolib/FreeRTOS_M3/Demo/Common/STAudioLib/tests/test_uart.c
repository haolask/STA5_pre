/*
 * STAudioLib - test_uart.c
 *
 * Created on 2013/10/23 by Christophe Quarre
 * Copyright: STMicroelectronics
 *
 * ACCORDO2 STAudio test application
 */

/*
	Testing


 */

#if 0
#include "common.h"

#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"

static char g_cmd[64];

//------------------------------------------------------------------------
//test UART_GetCommand()
//------------------------------------------------------------------------

/*
static void test_uart0_GetCommand(void)
{
	PRINTF("> ");

	while (1){

		if (!(UART0->FR & UARTFRRXFE)) {	//UART RX fifo not empty
			UART_GetCommand(g_cmd);
			break;
		}
	}

	PRINTF("end\n");
}
*/

static void test_uart_GetCommand(UART_TypeDef* uart)
{
	u32 UARTport;

//	UART_Init(uart, BR115200BAUD, NOPARITY_BIT, ONE_STOPBIT, DATABITS_8); //ok
	UART_Init(uart, BR19200BAUD,  NOPARITY_BIT, ONE_STOPBIT, DATABITS_8);
//	UART_Init(uart, BR230400BAUD, NOPARITY_BIT, ONE_STOPBIT, DATABITS_8); //ok
//	UART_Init(uart, BR460800BAUD, NOPARITY_BIT, ONE_STOPBIT, DATABITS_8); //
//	UART_Init(uart, BR921600BAUD, NOPARITY_BIT, ONE_STOPBIT, DATABITS_8); //

	switch ((u32)uart) {
	case UART0_BASE: UARTport = 0; uprintf(UARTport, "uart0>"); break;
	case UART1_BASE: UARTport = 1; uprintf(UARTport, "uart1>"); break;
	case UART3_BASE: UARTport = 3; uprintf(UARTport, "uart3>"); break;
	}

 	while (1) {
		UART_GetCommand(uart, g_cmd);
	}

}

//------------------------------------------------------------------------
//test UART IRQ
//------------------------------------------------------------------------
static int g_cmd_received       = 0;
//static int g_uart_rx_is_timeout = 0;

static void uart_isr(UART_TypeDef* uart)
{
	u32 MIS = uart->MIS.REG; //for debug

	//RX irq
	if (uart->MIS.BIT.RXI)
	{
		uart->ICR.BIT.RXI = 1;		//clear RX irq

		g_cmd_received = 1;
	}
	//RX timeout
	//(when rxfifo not empty, while not receiving new data for a while)
	else if (uart->MIS.BIT.RTI)
	{
		uart->ICR.BIT.RTI = 1;		//clear RX timeout irq

		//g_uart_rx_is_timeout = 1;
	}
	//errors
	else
	{
		uart->ICR.BIT.FEI = 1;		// clear FramingError
		uart->ICR.BIT.PEI = 1;		// clear ParityError
		uart->ICR.BIT.BEI = 1;		// clear BreakError
		uart->ICR.BIT.OEI = 1;		// clear OverrunError

		//TODO: process error
		int error = 1;
	}
}

/*
static void get_cmd(UART_TypeDef* uart)
{
	char header[16]; //in fact 13bytes with little headroom
	u32 count = 0;

	//assuming rxfifo filled with at leat 8 bytes.

	//get the header (13 bytes)
	while (count < 13)
	{
		//get burst of 8 bytes
		FORi(8) header[count++] = uart->DATA.BIT.DATA;

		while (uart->FR.BIT.RXFE); //wait till fifo not empty


	}


	g_cmd_received = 0; //incoming cmd processed
}
*/

//return 1 if valid header
static int get_cmd(UART_TypeDef* uart)
{
	/*
	UART Frame format:
	--- header ---
	SOF   :
	uC    :
	ID    : module recipient. using 0xF
	r/w   :	0=read, 1=write
	device: 1=Accordo2
	Len   : data size just after it (excl CS)
	--- data ---
	LibId : used by Ark
	ParamId:
	Size  :
	...
	CS    : checksum
	---- end ---
						<-------- header ------------------------------><-- data -->
						  SOF   SOF    uC    ID   r/w  device LenL LenH LibId  */
//	const char href[] = {0xAA, 0xAA, 0x00, 0x0F, 0x01,  0x01,   0,   0,   0};

	char h[16]; //header. in fact 13bytes with little headroom
	u32 i, count = 0;

	//get the 8-byte header
	//assuming rxfifo filled with at leat 8 bytes.
	FORi(8) h[count++] = uart->DATA.BIT.DATA;

	//check headera
	if (h[0] != 0xAA || h[1] != 0xAA)
//	if (h[0] != 'a' || h[1] != 'a')  //TMP for debug in console
		goto wrong_header;

	//write
	if (h[4] == 1)
//	if (h[4] == '1')   //TMP for debug in console
	{
		int a = 0;
	}
	//read
	else
	{
		assert(0); //TODO
	}



	UART_FlushRx(uart);
	g_cmd_received = 0; //incoming cmd processed
	return 1;

wrong_header:
	UART_FlushRx(uart);
	g_cmd_received = 0; //incoming cmd processed
	return 0;
}

static void test_uart_rx_irq(UART_TypeDef* uart)
{
	u32 UARTport;

//	UART_Init(uart, BR115200BAUD, NOPARITY_BIT, ONE_STOPBIT, DATABITS_8);
	UART_Init(uart, BR19200BAUD,  NOPARITY_BIT, ONE_STOPBIT, DATABITS_8);

	switch ((u32)uart) {
	case UART0_BASE: UARTport = 0; uprintf(UARTport, "uart0>"); break;
	case UART1_BASE: UARTport = 1; uprintf(UARTport, "uart1>"); break;
	case UART3_BASE: UARTport = 3; uprintf(UARTport, "uart3>"); break;
	}

	//additionnal init...

	//RX irq
	uart->IFLS.BIT.RXIFLSEL   = UART_IFLS_1BYTE;	//fifo trigger
	uart->IMSC.BIT.RXI        = 1;					//enable RX irq

#if 0
	//RX timeout irq
	//TODO: set timeout
//	uart->RXTIMEOUT =
	uart->IMSC.BIT.RTI        = 1;					//enable RX timeout irq
#endif

	//register the irq handler
	UART_SetIRQHandler(uart, uart_isr, 0, 0);

 	while (1)
 	{
		if (g_cmd_received)
			get_cmd(uart);

		SLEEP(1);
	}
}

//------------------------------------------------------------------------
//test UART DMA
//------------------------------------------------------------------------
//On cut 0, only UART1 can work with DMA
static void test_uart_to_DMA(void)
{
}

//------------------------------------------------------------------------

void TestUART(void)
{
	PRINTF("\nTestUART...\n");

//	test_uart_GetCommand(UART3);
	test_uart_rx_irq(UART3);

}

#endif //0

