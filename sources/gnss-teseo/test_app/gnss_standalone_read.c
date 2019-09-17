/*
 * gnss_standalone_read.c
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
#define SZ 255
#define MAX_NMEA_MSG_LENGTH 512
#define TIME_DIFFERENCE_CHECK (60*60*24)   //1 day in seconds
#define FILENAME_FORMAT "/gnss_%d%02d%02d-%02d_%02d_%02d.log"

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

int  cur_state = STATE_RUNNING;
int  GpioTeseoReset = -1;
int  Host_uart_port = -1;
int  Output_uart_port = -1;
int  TeseoReset_flag = 1;
char AppliName[40];
char Uart2Host[20]="/dev/ttyAMA";
char OutputUart[20]="/dev/ttyAMA";
char SavedPIDs[30]="/tmp/gnss_saved_pid";
static int  UartToTeseo_fd;
static int  UartToOuptput_fd;
int  StdoutDisplayEnabled = 0;
int  LogOutputEnabled = 0;

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

  printf("\r\n\n***** Quit standalone reading *****\r\n");
  sprintf(CmdLine, "kill -9 `cat %s`", SavedPIDs); system(CmdLine);
  sprintf(CmdLine, "rm %s", SavedPIDs); system(CmdLine);
  if (TeseoReset_flag == 1) {
    sprintf(CmdLine, "echo 0 > /sys/class/gpio/gpio%d/value", GpioTeseoReset); system(CmdLine);
  }
  sprintf(CmdLine, "killall -9 %s", AppliName); system(CmdLine);
}

void sig_inthandler()
{
  printf("Standalone read : SIG INT HANDLER called\r\n");
  stop_teseo();
  exit(0);
}

void send_nmea_cmd(char* input, int length)
{
  int n;

  if (strstr(input,"$") != NULL) {
     n = write(UartToTeseo_fd, input, length);

     if (n != length) {
       printf("Standalone read - %s: ERROR sending NMEA command %s\r\n", __func__, input);
       printf("Standalone read - %s: sent %d characters instead of %d\r\n", __func__, n, length);
     }
     else {
       input[length-1] = '\0'; // Remove the '\n' for display
       printf("Standalone read : Send NMEA command - %s\r\n", input);
     }
  }
}

void* check_input_cmd(void* arg)
{
  char input[MAX_NMEA_MSG_LENGTH];
  char *buffer = input;

  /* Start reception of NMEA command on Output UART */
  sprintf(input, "cat < %s > %s & echo $! >%s", OutputUart, Uart2Host, SavedPIDs); system(input);

  /* Start reception of command on STDIN */
  do
  {
    bzero(input, sizeof(input));
    fgets(input, sizeof(input), stdin);
    /*
    buffer=strchr(input,'\n');
    input[buffer-input] = '\0'; // Remove the '\n' for display
    printf("Standalone read : %s received input - %s\r\n", __func__, input);
    input[buffer-input] = '\n'; // Set back the '\n'
    */
    if (input[0] == '$') {
      send_nmea_cmd(input, strlen(input));
    }

  } while (!((input[0] == 'q') && (input[1] == '\n'))); // Exit the Socket client with character "q"

  /* Quit application */
  cur_state = STATE_QUIT;

  /* #######################  QUIT  ####################### */
  pthread_exit (0);
}

static int get_board_model(char *model)
{
  FILE *fp;

  fp = fopen("/proc/device-tree/model", "r");
  if (!fp)
    return -ENODEV;

  fgets(model, SZ, fp);
  fclose(fp);

  return 0;
}

int open_uart_port(char *device)
{
  int uartfd;
  struct termios newtio;

  uartfd = open(device, O_RDWR | O_NOCTTY);

  if (uartfd == -1) {
    printf("Standalone read - %s: Unable to open UART %s, error %d\r\n", __func__, device, errno);
    return errno;
  }

  printf("Standalone read : open UART %s with fd %d\r\n", device, uartfd);

  tcgetattr(uartfd, &newtio);

  newtio.c_cflag = B115200;
  newtio.c_cflag |= CLOCAL | CREAD;
  newtio.c_cflag |= CS8;
  newtio.c_iflag &= ~(IXON|ICRNL|INLCR);
  newtio.c_oflag = 0;
  newtio.c_lflag = 0;
  newtio.c_cc[VMIN] = 1;
  newtio.c_cc[VTIME] = 0;

  if (tcflush(uartfd, TCIOFLUSH)) {
    printf("Standalone read - %s ERROR: tcflush\r\n", __func__);
    stop_teseo();
  }

  if (tcsetattr(uartfd,TCSANOW,&newtio)) {
    printf("Standalone read - %s ERROR: tcsetattr\r\n", __func__);
    stop_teseo();
  }

  return uartfd;
}

static int teseo_trace_read()
{
  int len_in, len_out;
  char buf[SZ+1];
  struct termios newtio;
  char CmdLine[50];
  FILE *fp;
  char StartFile[40];
  char NewFile[40];
  char GpioPathName[40];
  time_t rawtime, Start_rawtime;
  struct tm * timeinfo;

  /* #######################  Open output log file  ####################### */
  if (LogOutputEnabled == 1) {
    time ( &Start_rawtime );
    timeinfo = localtime ( &Start_rawtime );

    sprintf(StartFile, FILENAME_FORMAT, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    fp = fopen(StartFile, "w");
    if (!fp)
      return errno;
    printf("Standalone read : Create file %s\r\n", StartFile);
  }

  /* ################ Start Teseo in Standalone mode  #################### */
  if (TeseoReset_flag == 1)
  {
    sprintf(GpioPathName,"/sys/class/gpio/gpio%d",GpioTeseoReset);
    if( access( GpioPathName, F_OK ) == -1 ) {
      printf("Standalone read : Configure new GPIO %s\r\n",GpioPathName);
      sprintf(CmdLine, "echo %d > /sys/class/gpio/export", GpioTeseoReset);  system(CmdLine);
      sprintf(CmdLine, "echo out > %s/direction", GpioPathName); system(CmdLine);
      sleep(1);
    }

    printf("Standalone read : Reset Teseo with %s\r\n",GpioPathName);
    sprintf(CmdLine, "echo 0 > %s/value", GpioPathName); system(CmdLine);
    sprintf(CmdLine, "echo 1 > %s/value", GpioPathName); system(CmdLine);
  }

  /* #####################  Open the UART port  ##################### */
  UartToTeseo_fd   = open_uart_port(Uart2Host );
  UartToOuptput_fd = open_uart_port(OutputUart);

  /* #######################  Display Exit message  ####################### */
  printf("\r\n\n");
  printf("*********************************************************\r\n\n");
  printf("  Press q<return> to Quit the application  \r\n\n");
  printf("*********************************************************\r\n\n");

  signal(SIGINT, sig_inthandler);

  /* #######################  Redirect Trace Flow  ####################### */
  while (cur_state != STATE_QUIT)
  {
    bzero(buf,sizeof(buf));
    len_in = read(UartToTeseo_fd, buf, SZ*sizeof(char));
    if (len_in < 0)
      printf("Standalone read - ERROR: UART read failed!\r\n");

    len_in = SZ*sizeof(char);
    len_out = write(UartToOuptput_fd, buf, len_in);
    if (len_in != len_out) {
      printf("Standalone read - ERROR: redirect %d characters instead of %d\r\n", len_out, len_in);
    }

    if (StdoutDisplayEnabled == 1)
	  printf("%s", buf);

    if (LogOutputEnabled == 1) {
      /* Fill output File */
      fprintf(fp,"%s", buf);

      /* Check if the date has been updated to a real date */
      /* or the log was started for more than 1 day        */
      /* Change the name of the output File                */
      time ( &rawtime );
      timeinfo = localtime ( &rawtime );

      if (abs(rawtime - Start_rawtime) > TIME_DIFFERENCE_CHECK) {
         sprintf(NewFile, FILENAME_FORMAT, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
         rename(StartFile, NewFile);
         printf("Standalone read : Rename file %s into %s\r\n", StartFile, NewFile);
         strcpy(StartFile, NewFile);
         Start_rawtime = rawtime;
      }
    }
  }

  /* #######################  QUIT  ####################### */
end:
  stop_teseo();

  return errno;
}


int main(int argc, char *argv[])
{
  int ret;
  pthread_t stdin_tid;
  char model[SZ];
  int id = 1;
  long int value;
  int Gpio_user = 0, Output_uart_user = 0, Host_uart_user = 0;

  printf("\r\n");
  get_appli_name(argv[0]);

  /* #######################  Check application parameters  ####################### */
  while (id < argc)
  {
    if (strcmp("-f" , argv[id]) == 0) {
       LogOutputEnabled = 1;
       printf("Standalone read : enable trace saving in LOG files\r\n");
    }

    if (strcmp("-d", argv[id]) == 0) {
       StdoutDisplayEnabled = 1;
       printf("Standalone read : display in STDOUT is enabled\r\n");
    }

    if (strcmp("-no_reset" , argv[id]) == 0) {
       TeseoReset_flag = 0;
       printf("Standalone read : Teseo reset canceled\r\n");
    }

    if (strcmp("IN", argv[id]) == 0) {
       id++;
       errno = 0;
       value = strtol(argv[id], (char **)NULL, 10);
       if ((errno != EINVAL) && (errno != ERANGE))
       {
         Host_uart_port = (int)value;
         Host_uart_user = 1;
       }
       printf("Standalone read : Host<->Teseo UART set to port %d\r\n", Host_uart_port);
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
       printf("Standalone read : Output UART set to port %d\r\n", Output_uart_port);
    }

    if (strcmp("GPIO", argv[id]) == 0) {
       id++;
       errno = 0;
       value = strtol(argv[id], (char **)NULL, 10);
       GpioTeseoReset = (int)value;
       Gpio_user = 1;
       printf("Standalone read : Teseo reset GPIO set to %d\r\n", GpioTeseoReset);
    }

    if (strcmp("-h" , argv[id]) == 0) {
       printf("Usage [%s]: %s [-h] [IN x] [OUT y] [GPIO z] [-d] [-f] [-no_reset]\r\n", APP_VERSION, argv[0]);
       printf("-h  : Display the \"Usage\" message\r\n");
       printf("IN   x : User can specify the Host<->Teseo UART port x of the host\r\n");
       printf("OUT  y : User can specify the output UART port y of the host\r\n");
       printf("GPIO z : User can specify the GPIO z to reset the Teseo\r\n");
       printf("-d  : Enable the trace on the STDOUT\r\n");
       printf("-f  : Enable trace saving in \"log\" files /gnss_ext_<date&time>.log\r\n");
       printf("-no_reset : No Teseo reset performed at start/stop\r\n");
       printf("\r\n");
       goto end;
    }

    id++;
  }

  /* #######################  Set the Board parameters  ####################### */
  ret = get_board_model(model);
  if (ret) {
    printf("Standalone read : Failed to retrieve board model!!\r\n");
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

  /* #######################  Start Input handler ####################### */
  ret = pthread_create(&stdin_tid, NULL, &check_input_cmd, NULL);
  if (ret != 0)
    printf("Standalone read : can't create thread for STDIN :[%s]\r\n", strerror(ret));
  else
    printf("Standalone read : Input Thread for STDIN created successfully\r\n");

  /* #######################  Start Trace redirection ####################### */
  ret = teseo_trace_read();

  /* #######################  QUIT  ####################### */
end:
  return ret;
}

