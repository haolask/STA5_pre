/****************************************************************************/
/*                                                                          */
/* Project:              ADR3 Control application                           */
/* Filename:             UART_Comm.c                                        */
/* Programmer:           Alessandro Vaghi                                   */
/* Date:                 Jun 18th 2010                                      */
/* CPU:                  PC                                                 */
/* Type:                 C code                                             */
/* Scope:                Serial communication functions                     */
/*                                                                          */
/****************************************************************************/
#include "target_config.h"

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
#include <windows.h>
#include <winbase.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#if defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>
#endif

#include "types.h"
#include "utility.h"


#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW) || defined (CONFIG_HOST_OS_LINUX_DESKTOP)
#include "common_helpers.h"
#elif defined(CONFIG_HOST_OS_LINUX_EMBEDDED) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
#include "osal.h"
#include "common_trace.h"
#endif

#include "uart_mngm.h"

/*****************************************************************
| defines and macros
|----------------------------------------------------------------*/
#define LF_CHAR                     0x0A
#define CR_CHAR                     0x0D
#define ESC_CHAR                    0x1B
#define SPACE_CHAR                  0x20

#define XON_CHAR                    0x13
#define XOFF_CHAR                   0x11

#define COMM_TX_BUFFER_SIZE         16384
#define COMM_RX_BUFFER_SIZE         16384

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW) || defined (CONFIG_HOST_OS_LINUX_DESKTOP)
#define UartLogString(a, b)  LogString(a, b)
#elif defined(CONFIG_HOST_OS_LINUX_EMBEDDED) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
#define UartLogString(a, b)  COMMON_tracePrint(TR_LEVEL_ERRORS, b, a)
#endif

/*****************************************************************
| Local types
|----------------------------------------------------------------*/
#if defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
typedef struct 
{
	int fd;
	struct termios oldtio;
	struct termios newtio;
} com_serial_port;
#endif

/*****************************************************************
| variable definition
|----------------------------------------------------------------*/


/*****************************************************************
| function prototypes
|----------------------------------------------------------------*/
static tVoid * CommPortOpen (tU32 comId, tU32 BaudRate);

tVoid* CommParameterSetup (tU32 ComId, tU32 BaudRateBps)
{
	tChar ComName[10];
	tVoid *devHandle;

	// Build COM string
	sprintf (ComName, "com%d", (tS32)ComId);

	// Open COM port
	devHandle = CommPortOpen (ComId, BaudRateBps);

	// Check if we get a valid handle in return
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
	if (INVALID_HANDLE_VALUE == devHandle)
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
	if (NULL == devHandle)
#endif
	{
		// Error opening COM port
		UartLogString("Comm Open fail", LOG_MASK_UART);

		return (tVoid *)NULL;
	}

	return devHandle;
} 

tVoid CommPortClose (tVoid *deviceHandle)
{
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
	if (INVALID_HANDLE_VALUE != deviceHandle)
	{
		CloseHandle (deviceHandle);
	} 
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
	com_serial_port *serial_port = (com_serial_port *)deviceHandle;

	/* restore the old port settings */
	tcsetattr(serial_port->fd, TCSANOW, &serial_port->oldtio);

	close(serial_port->fd);

	free(serial_port);
#endif
} 

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
tU32 CommTransmitData (tVoid *deviceHandle, tUChar *WriteBuffer, tU16 BytesToSend)
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
tS32 CommTransmitData (tVoid *deviceHandle, tUChar *WriteBuffer, tU16 BytesToSend)
#endif
{
	tU32 SentBytes;

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
	WriteFile (deviceHandle, WriteBuffer, BytesToSend, (DWORD *)&SentBytes, NULL);
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
	com_serial_port *serial_port = (com_serial_port *)deviceHandle;

	SentBytes = write(serial_port->fd, WriteBuffer, BytesToSend);
#endif

	return SentBytes;
}

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
tU32 CommReceiveData (tVoid *deviceHandle, tUChar *ReadBuffer, tU32 BytesToRead, tU32 RxTimeoutMs)
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
tS32 CommReceiveData (tVoid *deviceHandle, tUChar *ReadBuffer, tS32 BytesToRead, tS32 RxTimeoutMs)
#endif
{
#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)
	if (0 == ReadFile (deviceHandle, ReadBuffer, BytesToRead, (tULong *)&BytesToRead, NULL))
	{
		return 0;
	}

	return BytesToRead;
#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)
	struct pollfd input_pollfd;
	com_serial_port *serial_port = (com_serial_port *)deviceHandle;
	tSInt events;
	tS32 numOfBytesRead = 0;

	input_pollfd.fd      = serial_port->fd;
	input_pollfd.events  = POLLIN;
	input_pollfd.revents = 0;

	/* wait for com port events */
	events = poll(&input_pollfd, 1, RxTimeoutMs);

	if (events < 0)
	{
		UartLogString("Error on poll(). It should never happen!!!", LOG_MASK_UART);
		return 0;
	}

	if (input_pollfd.revents && POLLIN)
	{
		numOfBytesRead = read(serial_port->fd, ReadBuffer, BytesToRead);
	}

	return numOfBytesRead;
#endif
}

#if defined(CONFIG_HOST_OS_WIN_VISUALSTUDIO) || defined(CONFIG_HOST_OS_WIN32_MINGW)

static tVoid * CommPortOpen (tU32 comId, tU32 BaudRate)
{
	DCB Dcb;
	static COMMTIMEOUTS CommTimeouts = {0, 1, 0, MAXDWORD, MAXDWORD};
	HANDLE comHandle;

#if (defined CONFIG_HOST_OS_WIN_VISUALSTUDIO)
	wchar_t ComName[16];
#elif (defined CONFIG_HOST_OS_WIN32_MINGW) || (defined CONFIG_HOST_OS_LINUX)
	char ComName[16];
#endif

	char debugString[25];

#if (defined CONFIG_HOST_OS_WIN_VISUALSTUDIO)
	wsprintf (ComName, L"\\\\.\\COM%u", comId);
#elif (defined CONFIG_HOST_OS_WIN32_MINGW) || (defined CONFIG_HOST_OS_LINUX)
	sprintf (ComName, "\\\\.\\COM%d", comId);
#endif

	//CommPortClose(); // Close Line

	// Create handle
	comHandle = CreateFile (ComName,			// 'wText'
		(DWORD)(GENERIC_READ | GENERIC_WRITE),
		(DWORD)0,								// No shared mode
		(LPSECURITY_ATTRIBUTES)NULL,			// No security attributes
		(DWORD)CREATE_NEW,						// OPEN_EXISTING  // OPEN_ALWAYS
		(DWORD)FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	if (comHandle == INVALID_HANDLE_VALUE)
	{
		UartLogString ("Invalid COM handler value", LOG_MASK_UART);

		return (tVoid *)comHandle;
	}

	if (GetCommState (comHandle, &Dcb) != TRUE)
	{
		comHandle = INVALID_HANDLE_VALUE;

		UartLogString ("Invalid COM handler value", LOG_MASK_UART);

		return (tVoid *)comHandle;
	}

	Dcb.BaudRate = BaudRate;
	Dcb.ByteSize = 8;
	Dcb.Parity = NOPARITY;
	Dcb.StopBits = ONESTOPBIT;
	Dcb.fBinary = TRUE;								// Binary mode
	Dcb.fParity = FALSE;							// No parity check
	Dcb.fOutxCtsFlow = FALSE;						// No CTS flow control
	Dcb.fOutxDsrFlow = FALSE;						// No DSR flow control
	Dcb.fOutX = FALSE;								// No XON XOFF flow control
	Dcb.fInX = FALSE;								// No XON XOFF flow control
	Dcb.fNull = FALSE;								// No discard null character
	Dcb.fDtrControl = DTR_CONTROL_DISABLE;			// DTR flow control type
	Dcb.fDsrSensitivity = FALSE;					// DSR sensitivity
	//Dcb.EvtChar           = CH_ETX;				// Event character
	Dcb.EvtChar = 0;
	Dcb.fTXContinueOnXoff = TRUE;					// XOFF continues Tx
	Dcb.fErrorChar = FALSE;							// Enable error replacement
	Dcb.fRtsControl = RTS_CONTROL_DISABLE;			// RTS flow control
	Dcb.fAbortOnError = FALSE;						// No abort reads/writes on error
	Dcb.XonLim = (COMM_TX_BUFFER_SIZE * 8) / 10;	// Transmit XON threshold
	Dcb.XoffLim = (COMM_TX_BUFFER_SIZE * 8) / 10;	// Transmit XOFF threshold
	Dcb.EofChar = SPACE_CHAR;						// End of input character
	Dcb.XonChar = XON_CHAR;							// Tx and Rx XON character
	Dcb.XoffChar = XOFF_CHAR;						// Tx and Rx XOFF character
	Dcb.ErrorChar = SPACE_CHAR;						// Error replacement character
	Dcb.wReserved = 0;

	// Get buffers
	if (SetupComm (comHandle, COMM_TX_BUFFER_SIZE, COMM_TX_BUFFER_SIZE) != TRUE)
	{
		comHandle = INVALID_HANDLE_VALUE;

		UartLogString ("Invalid COM handler value", LOG_MASK_UART);

		return (tVoid *)comHandle;
	}

	// Initialize the COM hardware
	if (SetCommState (comHandle, &Dcb) != TRUE)
	{
		comHandle = INVALID_HANDLE_VALUE;

		UartLogString ("Invalid COM handler value", LOG_MASK_UART);

		return (tVoid *)comHandle;
	}

	EscapeCommFunction (comHandle, SETDTR);

	CommTimeouts.ReadIntervalTimeout = MAXDWORD;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.WriteTotalTimeoutMultiplier = 0;
	CommTimeouts.WriteTotalTimeoutConstant = 0;

	SetCommTimeouts (comHandle, &CommTimeouts);

	// Message that everything went fine
	sprintf (debugString, "COM %u handle created", comId);
	UartLogString (debugString, LOG_MASK_UART);

	return (tVoid *)comHandle;
}

#elif defined(CONFIG_HOST_OS_LINUX) || defined(CONFIG_HOST_OS_TKERNEL) || defined(CONFIG_HOST_OS_WIN32)

static tVoid * CommPortOpen (tU32 comId, tU32 BaudRate)
{
	com_serial_port *serial_port;
	char ComName[16];

	serial_port = (com_serial_port *)calloc(1, sizeof(com_serial_port));
	if (!serial_port)
		return NULL;

#if defined(CONFIG_BOARD_ACCORDO2)
	sprintf (ComName, "/dev/ttyUSB%d", (comId % 10));
#elif defined(CONFIG_BOARD_ACCORDO5)
	sprintf (ComName, "/dev/ttyAMA%d", (comId % 10));
#elif defined(CONFIG_BOARD_CMOST_MAIN)
	sprintf (ComName, "/dev/ttyS%d", (comId % 10));
#else
#error tty port not supported
#endif
printf("ComName: %s\n", ComName);
	serial_port->fd = open(ComName, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (serial_port->fd < 0) {
		UartLogString("Failed to open device\n", LOG_MASK_UART);
		return NULL;
	}

	/* save current serial port settings */
	tcgetattr(serial_port->fd, &serial_port->oldtio);

	OSAL_pvMemorySet(&serial_port->newtio, 0, sizeof(serial_port->newtio));

	/*
	 * B115200 : Set 1152000 bps rate.
	 * CS8     : 8bit, no parity, 1 stopbit
	 * CLOCAL  : local connection, no modem control
	 * CREAD   : enable receiving characters
	 */
	serial_port->newtio.c_cflag = BaudRate/*B115200*/ | CS8 | CLOCAL | CREAD;

	/*
	 * IGNPAR  : ignore bytes with parity errors
	 * ICRNL   : map CR to NL (a CR input on the remote peer will terminate input)
	 */
	serial_port->newtio.c_iflag = IGNPAR;

	/*
	 * Raw output and input
	 */
	serial_port->newtio.c_oflag = 0;
	serial_port->newtio.c_lflag = 0;

	/*
	 * Blocking read: read(2) function blocks until MIN bytes are
	 * available, and returns up to the number of bytes requested.
	 */
	serial_port->newtio.c_cc[VTIME] = 0; /* Inter-character timeout */
	serial_port->newtio.c_cc[VMIN]  = 1; /* Blocking read until 1 character arrives */

	/*
	 * Clean the modem line and activate the settings for the port
	 */
	tcflush(serial_port->fd, TCIFLUSH);
	tcsetattr(serial_port->fd, TCSANOW, &serial_port->newtio);

	return serial_port;

}

#endif

// End of file
