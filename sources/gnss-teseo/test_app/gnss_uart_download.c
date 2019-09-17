/*
 * gnss_uart_download.c
 *
 * Linux application for Accordo GNSS subsystem.
 *
 * Author: Fabrice Pointeau <fabrice.pointeau@st.com> for ST-Microelectronics.
 *
 ********************************************************************************
 * Copyright (c) 2017 STMicroelectronics - All Rights Reserved
 * Reproduction and Communication of this document is strictly prohibited
 * unless specifically authorized in writing by STMicroelectronics.
 * FOR MORE INFORMATION PLEASE READ CAREFULLY THE LICENSE AGREEMENT LOCATED
 * IN THE ROOT DIRECTORY OF THIS SOFTWARE PACKAGE.
 ********************************************************************************
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <pthread.h>
#include <termios.h>
#include <signal.h>

#define APP_VERSION "v1.1"

/*******************************
 * Modem and socket declarations
 *******************************/
#define DEBUG_ON 0  /* 1 for basic debug : 2 for complete debug */
#define FORCE_PROCESS_WITH_UART_KILL 0

#if (DEBUG_ON > 0)
  #define APP_DEBUG_MSG(macro_string) printf macro_string
#else
  #define APP_DEBUG_MSG(macro_string)
#endif

#define CHUNKSIZE 1024
#define MAX_MSG_LENGTH 10

#define TESEO3_HOST_READY       	0x5AU
#define TESEO3_FLASHER_READY    	0x4AU
#define TESEO3_CHANGE_BAUD_RATE 	0x71U

#define TESEO3_DEFAULT_BAUDRATE     115200

#define BINARY_IMAGE_STEPS      	3
#define CHANGE_BAUD_RATE_STEPS  	3
#define BINARY_LOAD_DISPLAY_DELAY	((6*100000/Download_baudrate)<1?1:(6*100000/Download_baudrate)) //seconds
#define BINARY_LOAD_COUNT_DELAY	    1 //seconds
#define BINARY_LOAD_FINAL_DELAY     1 //seconds

typedef enum {
  STATE_RUNNING,
  STATE_QUIT
} state;

typedef struct {
char model[15];
int  gpio_reset;
int  host_uart_port;
int  output_uart_port;
} board_params_t;

#define BOARD_NUMBERS	(sizeof(board_params)/(sizeof(board_params[0].model) + sizeof(board_params[0].gpio_reset) + sizeof(board_params[0].host_uart_port) + sizeof(board_params[0].output_uart_port)))

board_params_t board_params[] = {
  {"STA1078",  19, 3, 1},
  {"STA1085", 206, 1, 2},
  {"STA1095", 206, 1, 2},
  {"STA1195", 101, 1, 0},
  {"STA1295", 101, 1, 0},
  {"STA1385",  62, 2, 0},
  {"STA1385 MTP",  62, 2, 0},
};

typedef struct uart_baud_rates_s {
  unsigned int rate;
  unsigned int cflag;
} uart_baud_rates_t;

static uart_baud_rates_t uart_baud_rates[] = {
	{  921600, B921600  },
	{  460800, B460800  },
    {  230400, B230400  },
	{  115200, B115200  },
    {   57600, B57600   }
};

int  cur_state = STATE_RUNNING;
int  Download_Binary_started = 0;
int  Download_Binary_image_step = -1;
unsigned int  Download_len_total = 0;
unsigned int  Download_baudrate = TESEO3_DEFAULT_BAUDRATE;
int  Change_baud_rate_step = -1;
int  GpioTeseoReset = -1;
int  Host_uart_port = -1;
int  Output_uart_port = -1;
int  TeseoReset_flag = 1;
char AppliName[40];
char Uart2Host[20]="/dev/ttyAMA";
char OutputUart[20]="/dev/ttyAMA";
static int  UartToTeseo_fd;
static int  UartToOuptput_fd;
struct timespec start_time={0,0},diff_time={0,0},end_time={0,0};
pthread_t stdin_tid, Teseo2Output_tid, Output2Teseo_tid, Tracing_tid;

/*******************************
 * Functions
 *******************************/
void get_appli_name(char *input)
{
  if (strrchr(input, '/') != NULL)
	strcpy(AppliName, strrchr(input, '/') + 1);
  else
	strcpy(AppliName, input);
}

void stop_teseo(void)
{
  char CmdLine[50];

  cur_state = STATE_QUIT;

  printf("\r\n\n***** Quit UART Download *****\r\n");
  if (TeseoReset_flag == 1) {
    sprintf(CmdLine, "echo 0 > /sys/class/gpio/gpio%d/value", GpioTeseoReset); system(CmdLine);
  }
  sprintf(CmdLine, "killall -9 %s", AppliName); system(CmdLine);
}

void sig_inthandler()
{
  APP_DEBUG_MSG(("Uart Download : SIG INT HANDLER called\r\n"));
  stop_teseo();
  exit(0);
}

void* check_input_cmd(void* arg)
{
  char input[MAX_MSG_LENGTH];
  char *buffer = input;

  /* Start reception of command on STDIN */
  do
  {
    bzero(input, sizeof(input));
    fgets(input, sizeof(input), stdin);

    buffer=strchr(input,'\n');
    input[buffer-input] = '\0'; // Remove the '\n' for display
    printf("\r\nCommand %s received\r\n", input);  
    
  } while (!((input[0] == 'q') && (input[1] == '\0'))); // Exit the Socket client with character "q"

  /* Quit application */
  stop_teseo();
}

static int get_board_model(char *model)
{
  FILE *fp;

  fp = fopen("/proc/device-tree/model", "r");
  if (!fp)
    return -ENODEV;

  fgets(model, CHUNKSIZE, fp);
  fclose(fp);

  return 0;
}

int open_uart_port(char *device, unsigned int baudrate)
{
  int i;
  int uartfd;
  struct termios newtio;

  uartfd = open(device, O_RDWR | O_NOCTTY);

  if (uartfd == -1) {
    printf("Uart download - %s: Unable to open UART %s, error %d\r\n", __func__, device, errno);
    return errno;
  }

  for (i = 0; uart_baud_rates[i].rate; i++)
    if (uart_baud_rates[i].rate <= baudrate)
      break;

  APP_DEBUG_MSG(("Uart download : open UART %s with fd %d at rate %d bps\r\n", device, uartfd, baudrate));

  tcgetattr(uartfd, &newtio);

  newtio.c_cflag = uart_baud_rates[i].cflag;
  newtio.c_cflag |= CLOCAL | CREAD;
  newtio.c_cflag |= CS8;
  newtio.c_iflag &= ~(IXON|ICRNL|INLCR);
  newtio.c_oflag = 0;
  newtio.c_lflag = 0;
  newtio.c_cc[VMIN] = 1;
  newtio.c_cc[VTIME] = 0;

  if (tcflush(uartfd, TCIOFLUSH)) {
    printf("Uart download - %s ERROR: tcflush\r\n", __func__);
    stop_teseo();
  }

  if (tcsetattr(uartfd,TCSANOW,&newtio)) {
    printf("Uart download - %s ERROR: tcsetattr\r\n", __func__);
    stop_teseo();
  }

  return uartfd;
}


void change_uart_baudrate(int uartfd, unsigned int baudrate)
{
  struct termios newtio;
  int i;

  for (i = 0; uart_baud_rates[i].rate; i++)
    if (uart_baud_rates[i].rate <= baudrate)
      break;

  APP_DEBUG_MSG(("Uart download : Set baud rate 0x%x (Id %d) on UART with fd %d\r\n", uart_baud_rates[i].cflag, i, uartfd));

  tcgetattr(uartfd, &newtio);

  APP_DEBUG_MSG(("Uart download : Old CFlag 0x%x\r\n", newtio.c_cflag));
  newtio.c_cflag &= ~CBAUD;
  newtio.c_cflag |= uart_baud_rates[i].cflag;
  APP_DEBUG_MSG(("Uart download : New CFlag 0x%x\r\n", newtio.c_cflag));

  if (tcflush(uartfd, TCIOFLUSH)) {
    printf("Uart download - %s ERROR: tcflush\r\n", __func__);
    stop_teseo();
  }

  if (tcsetattr(uartfd,TCSANOW,&newtio)) {
    printf("Uart download - %s ERROR: tcsetattr\r\n", __func__);
    stop_teseo();
  }  
}

static int teseo_start()
{
  char CmdLine[50];
  char GpioPathName[40];

  /* ################ Start Teseo in Standalone mode  #################### */
  if (TeseoReset_flag == 1)
  {
    sprintf(GpioPathName,"/sys/class/gpio/gpio%d",GpioTeseoReset);
    if( access( GpioPathName, F_OK ) == -1 ) {
      APP_DEBUG_MSG(("Uart download : Configure new GPIO %s\r\n",GpioPathName));
      sprintf(CmdLine, "echo %d > /sys/class/gpio/export", GpioTeseoReset);  system(CmdLine);
      sprintf(CmdLine, "echo out > %s/direction", GpioPathName); system(CmdLine);
      sleep(1);
    }
    printf("Teseo Reset with gpio %d\r\n",GpioTeseoReset);
    sprintf(CmdLine, "echo 0 > %s/value", GpioPathName); system(CmdLine);
    sprintf(CmdLine, "echo 1 > %s/value", GpioPathName); system(CmdLine);
  }

  /* #####################  Open the UART port  ##################### */
  UartToTeseo_fd   = open_uart_port(Uart2Host , Download_baudrate);
  UartToOuptput_fd = open_uart_port(OutputUart, Download_baudrate);

  /* #######################  Display Exit message  ####################### */
  printf("\r\n\n");
  printf("*********************************************************\r\n\n");
  printf("  Press q<return> to Quit the application  \r\n\n");
  printf("*********************************************************\r\n\n");

  signal(SIGINT, sig_inthandler);

  return errno;
}

void* Tracing_Download(void* arg)
{
  static unsigned int Last_Download_len = 0;
  static unsigned int display_count = 0;

  while (cur_state != STATE_QUIT)
  { 
    if (Download_len_total != Last_Download_len)
    {
      if (display_count == 0)
      {
        printf("\tDownload on going : %8d bytes loaded\r\n", Download_len_total);
        display_count = BINARY_LOAD_DISPLAY_DELAY;
      }
      else
      {
        display_count--;
      }

      Last_Download_len = Download_len_total;
      sleep(BINARY_LOAD_COUNT_DELAY);
    }
    else {
      printf("\tBinary file download over : %8d bytes loaded\r\n", Download_len_total);
      sleep(BINARY_LOAD_FINAL_DELAY);
      break;
    }
  }

  printf("\r\nDownload is over\r\n", Download_len_total);

  clock_gettime(CLOCK_REALTIME, &end_time);
  diff_time.tv_sec=end_time.tv_sec - start_time.tv_sec;
  diff_time.tv_nsec=end_time.tv_nsec - start_time.tv_nsec;
  if (diff_time.tv_nsec >= 1000000000L) {
     diff_time.tv_sec++;
  }
  printf("\tDownload time: %2d min %2d sec\r\n", diff_time.tv_sec/60, diff_time.tv_sec%60);

  close(UartToTeseo_fd);
  UartToTeseo_fd   = open_uart_port(Uart2Host , TESEO3_DEFAULT_BAUDRATE);
  close(UartToOuptput_fd);
  UartToOuptput_fd = open_uart_port(OutputUart, TESEO3_DEFAULT_BAUDRATE);

  /* Quit application */
  //stop_teseo();
}

void Check_download_steps(int len_in, char *data)
{
  if (start_time.tv_sec == 0) clock_gettime(CLOCK_REALTIME, &start_time);

  /* CHECK STEP 1.1 : Xloader request baud rate change */
  if ((len_in == 1) && (TESEO3_CHANGE_BAUD_RATE == data[0])) 
  {
    Change_baud_rate_step = CHANGE_BAUD_RATE_STEPS;
  }

  /* CHECK STEP 1.2 : Get new baud rate */
  if (Change_baud_rate_step == (CHANGE_BAUD_RATE_STEPS-1)) 
  {
    memcpy(&Download_baudrate, data, 4);

    sleep(1);   // Let time for teseo to send the ACK message

    change_uart_baudrate(UartToTeseo_fd  , Download_baudrate);
    change_uart_baudrate(UartToOuptput_fd, Download_baudrate);

    printf("\r\nChanged Baud rate to %d bps\r\n",Download_baudrate);
    Download_len_total = 4; /* Start byte counting for Boot Loader and other */
                            /* Initialised with previously received 
                               (TESEO3_FLASHER_IDENTIFIER size = 4 bytes) */
  }

  /* CHECK STEP 1.3 : New baud rate steps counting */
  if (Change_baud_rate_step > 0)
  {
    Change_baud_rate_step--;
  }

  /* CHECK STEP 2 : Xloader sends TESEO3_HOST_READY to Teseo */
  if ((len_in == 1) && (TESEO3_HOST_READY == data[0])) 
  {
    printf("\r\nBoot Loader download started\r\n");
    Download_len_total = 4; /* Start byte counting for Boot Loader and other */
                            /* Initialised with previously received 
                               (TESEO3_FLASHER_IDENTIFIER size = 4 bytes) */
  }

  /* CHECK STEP 3 : Xloader sends TESEO3_FLASHER_READY to Teseo */
  if ((len_in == 1) && (TESEO3_FLASHER_READY == data[0])) 
  {
    Download_Binary_image_step = BINARY_IMAGE_STEPS;
  }

  /* CHECK STEP 4 : Xloader sends Binary image information to Teseo */
  if (Download_Binary_image_step > 0)
  {
    Download_Binary_image_step--;
  }

  /* CHECK STEP 5 : Xloader sends Binary image to Teseo */
  if (Download_Binary_image_step == 0) 
  {
    printf("\tBoot Loader download over : %d bytes loaded\r\n", Download_len_total);
    printf("\r\nBinary file download started\r\n");
    Download_len_total = 0;  /* Start byte counting for Teseo3 Binary image */
    Download_Binary_image_step = 0;
    Download_Binary_started = 1;
    pthread_create(&Tracing_tid, NULL, &Tracing_Download, NULL);
  }
}

void* ReadFromTeseo_WriteToOutput(void* arg)
{
  int len_in, len_out;
  char buf[CHUNKSIZE];

  /* #######################  Redirect Trace Flow  ####################### */
  while (cur_state != STATE_QUIT)
  {
    bzero(buf,sizeof(buf));
    len_in = read(UartToTeseo_fd, buf, CHUNKSIZE*sizeof(char));
    if (len_in < 0)
    { 
      printf("Uart download - ERROR: Teseo UART read failed!\r\n");
    }
    else
    {
      #if (DEBUG_ON == 2)
      {
        unsigned int i;      
        printf("Teseo sends : %d bytes ", len_in);
        for (i=0; i<len_in; i++)
          printf("Tx%x ", buf[i]);
        printf("\r\n");
      }
      #endif

      len_out = write(UartToOuptput_fd, buf, len_in);
      if (len_in != len_out) {
        printf("Uart download - ERROR: Teseo --> Output redirect %d characters instead of %d\r\n", len_out, len_in);
      }
    }
  }

  /* #######################  QUIT  ####################### */
  pthread_exit (0);
}

void* ReadFromOutput_WriteToTeseo(void* arg)
{
  int len_in, len_out;
  char buf[CHUNKSIZE];

  /* #######################  Redirect Trace Flow  ####################### */
  while (cur_state != STATE_QUIT)
  {
    bzero(buf,sizeof(buf));
    len_in = read(UartToOuptput_fd, buf, CHUNKSIZE*sizeof(char));
    if (len_in < 0)
    {
      printf("Uart download - ERROR: Output UART read failed!\r\n");
    }
    else
    {
      #if (DEBUG_ON == 2)
      if (Download_Binary_started == 0)
      {
        unsigned int i;
        printf("Output sends : %d bytes ", len_in);
        for (i=0; i<len_in; i++)
          printf("Ox%x ", buf[i]);
        printf("\r\n");
      }
      #endif

      len_out = write(UartToTeseo_fd, buf, len_in);
      if (len_in != len_out) {
        printf("Uart download - ERROR: Teseo <-- Output redirect %d characters instead of %d\r\n", len_out, len_in);
      }
      else {
        if (Download_Binary_started == 0)
        {
          Check_download_steps(len_in, buf);         
        }
        /* Increment data counter */
        Download_len_total += len_in;
      }
    }
  }

  /* #######################  QUIT  ####################### */
  pthread_exit (0);
}

void check_uart_usage(char* UARTport)
{
  char uartchk[256];

  sprintf(uartchk, "ls -l /proc/[0-9]*/fd/* 2> /dev/null | grep -c %s > /dev/null 2>&1", UARTport);
  if (system(uartchk) == 0)
  {
      sprintf(uartchk, "ls -l /proc/[0-9]*/fd/* 2> /dev/null | grep %s 2> /dev/null", UARTport);
	  printf("WARNING : Output UART port %s is already used by process\r\n", UARTport);
      system(uartchk);
      #if (FORCE_PROCESS_WITH_UART_KILL == 1)
      sprintf(uartchk, "kill -9 `ls -l /proc/[0-9]*/fd/* 2> /dev/null | grep %s 2> /dev/null | cut -s -f 3 -d /`", UARTport);
      system(uartchk);
	  printf("\r\nProcess killed\r\n\n");
      #else
	  printf("\r\n !!! Kill used process first  !!!\r\n\n");
      stop_teseo();
      #endif
  }
}

int main(int argc, char *argv[])
{
  int ret;
  char model[128];
  int id = 1;
  long int value;
  int Gpio_user = 0, Output_uart_user = 0, Host_uart_user = 0;

  printf("\r\n");
  get_appli_name(argv[0]);

  /* #######################  Check application parameters  ####################### */
  while (id < argc)
  {
    if (strcmp("IN", argv[id]) == 0) {
       id++;
       errno = 0;
       value = strtol(argv[id], (char **)NULL, 10);
       if ((errno != EINVAL) && (errno != ERANGE))
       {
         Host_uart_port = (int)value;
         Host_uart_user = 1;
       }
       printf("Uart download : Host<->Teseo UART set to port %d\r\n", Host_uart_port);
    }

    if (strcmp("OUT", argv[id]) == 0) {
       id++;
       errno = 0;
       value = strtol(argv[id], (char **)NULL, 10);
       if ((errno != EINVAL) && (errno != ERANGE))
       {
         Output_uart_port = (int)value;
         Output_uart_user = 1;
       }
       printf("Uart download : Output UART set to port %d\r\n", Output_uart_port);
    }

    if (strcmp("GPIO", argv[id]) == 0) {
       id++;
       errno = 0;
       value = strtol(argv[id], (char **)NULL, 10);
       GpioTeseoReset = (int)value;
       Gpio_user = 1;
       printf("Uart download : Teseo reset GPIO set to %d\r\n", GpioTeseoReset);
    }

    if (strcmp("-no_reset" , argv[id]) == 0) {
       TeseoReset_flag = 0;
       printf("Uart download : Teseo reset canceled\r\n");
    }

    if (strcmp("-h" , argv[id]) == 0) {
       printf("Usage [%s]: %s [-h] [IN x] [OUT y] [GPIO z] [-no_reset]\r\n", APP_VERSION, argv[0]);
       printf("-h  : Display the \"Usage\" message\r\n");
       printf("IN   x : User can specify the Host<->Teseo UART port x of the host\r\n");
       printf("OUT  y : User can specify the output UART port y of the host\r\n");
       printf("GPIO z : User can specify the GPIO z to reset the Teseo\r\n");
       printf("-no_reset : No Teseo reset performed at start/stop\r\n");
       printf("\r\n");
       goto end;
    }

    id++;
  }

  /* #######################  Set the Board parameters  ####################### */
  ret = get_board_model(model);
  if (ret) {
    printf("Uart download : Failed to retrieve board model!!\r\n");
    goto end;
  }
  else {
    printf("Detected board model : \"%s\"\r\n\n", model);
  }

  for (id = 0; id<BOARD_NUMBERS ; id++)
  {
    if (strstr(model, board_params[id].model)) {
       if (Gpio_user == 0)
         GpioTeseoReset = board_params[id].gpio_reset;
       if (Host_uart_user == 0)
         Host_uart_port = board_params[id].host_uart_port;
       if (Output_uart_user == 0)
         Output_uart_port = board_params[id].output_uart_port;
    }
  }
  sprintf(Uart2Host ,"%s%d", Uart2Host , Host_uart_port);
  sprintf(OutputUart,"%s%d", OutputUart, Output_uart_port);
  printf("UART%d : Host <--> Teseo \r\n", Host_uart_port);
  printf("UART%d : Host <--> Output\r\n", Output_uart_port);

  /* ########  Check if another process is using the serial port  ############# */
  check_uart_usage(OutputUart);
  check_uart_usage(Uart2Host);

  /* #######################  Start Teseo ####################### */
  ret = teseo_start();

  /* #######################  Start Input handler ####################### */
  ret = pthread_create(&stdin_tid, NULL, &check_input_cmd, NULL);
  if (ret != 0)
    printf("Uart download : can't create thread for STDIN :[%s]\r\n", strerror(ret));
  else
    APP_DEBUG_MSG(("Uart download : Input Thread for STDIN created successfully\r\n"));

  /* #######################  Start Trace redirection ####################### */
  ret = pthread_create(&Teseo2Output_tid, NULL, &ReadFromTeseo_WriteToOutput, NULL);
  if (ret != 0)
    printf("Uart download : can't create thread for Teseo2Output :[%s]\r\n", strerror(ret));
  else
    APP_DEBUG_MSG(("Uart download : Thread for Teseo2Output created successfully\r\n"));

  ret = pthread_create(&Output2Teseo_tid, NULL, &ReadFromOutput_WriteToTeseo, NULL);
  if (ret != 0)
    printf("Uart download : can't create thread for Output2Teseo :[%s]\r\n", strerror(ret));
  else
    APP_DEBUG_MSG(("Uart download : Thread for Output2Teseo created successfully\r\n"));

  /* #######################  QUIT  ####################### */
  while (1);

end:  
  return ret;
}

