/*
 * gnss_socket_redirect.c
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
#include <limits.h>

#define APP_VERSION "v1.1"

/*******************************
 * Modem and socket declarations
 *******************************/
#define EXTERNAL_PORT_NUMBER 8000
#define HOSTNAME "127.0.0.1"
#define SZ 255
#define MAX_NMEA_MSG_LENGTH 512

typedef enum {
  STATE_RUNNING,
  STATE_QUIT
} state;

typedef struct {
char model[15];
int  gpio_reset;
int  uart_port;
} board_params_t;

#define BOARD_NUMBERS	(sizeof(board_params)/(sizeof(board_params[0].model) + sizeof(board_params[0].gpio_reset) + sizeof(board_params[0].uart_port)))

board_params_t board_params[] = {
  {"STA1078",  19, 3},
  {"STA1085", 206, 1},
  {"STA1095", 206, 1},
  {"STA1195", 101, 1},
  {"STA1295", 101, 1},
  {"STA1385",  62, 2},
  {"STA1385 MTP",  62, 2},
};

int  cur_state = STATE_RUNNING;
int  GpioTeseoReset = -1;
int  TeseoReset_flag = 1;
char uart2host[20]="/dev/ttyAMA";
char SocketClientName[25]="gnss_socket_client.bin";
char BinaryPath[PATH_MAX]="/usr/bin/";
char ClientAppliExec[100];
char AppliName[40];
int  uartfd;

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

void get_appli_path(char *pathname)
{
  char path_save[PATH_MAX];
  char *p;

  strcpy(path_save, pathname);
  p = strrchr(path_save, '/');
  if( p != NULL )
  {
    path_save[p-path_save+1] = '\0';
    strcpy(BinaryPath, path_save);
  }

  printf("Absolute path to executable is: %s\n", BinaryPath);
}

void stop_teseo(void)
{
  char CmdLine[50];

  printf("\r\n***** Quit Socket redirection *****\r\n");
  if (TeseoReset_flag == 1) {
    sprintf(CmdLine, "echo 0 > /sys/class/gpio/gpio%d/value", GpioTeseoReset); system(CmdLine);
  }
  sprintf(CmdLine, "killall -9 %s", SocketClientName); system(CmdLine);
  sprintf(CmdLine, "killall -9 %s", AppliName); system(CmdLine);
}

void sig_inthandler()
{
  printf("Socket redirect : SIG INT HANDLER called\r\n");
  stop_teseo();
  exit(0);
}

void send_nmea_cmd(char* input, int length)
{
  int n;

  if (strstr(input,"$") != NULL)
  {
     n = write(uartfd, input, length);

     if (n != length) {
       printf("Socket redirect - %s: ERROR sending NMEA command %s\r\n", __func__, input);
       printf("Socket redirect - %s: sent %d characters instead of %d\r\n", __func__, n, length);
     }
  }
}

void* check_input(void* arg)
{
  char input[MAX_NMEA_MSG_LENGTH];
  char *eof;

  do
  {
    input[0] = '\0'; /* Ensure empty line if no input delivered */
    input[sizeof(input)-1] = ~'\0';  /* Ensure no false-null at end of buffer */

    eof = fgets(input, sizeof(input), stdin);
    printf("Socket redirect read input - %s\r\n", input);

    send_nmea_cmd(input, strlen(input));

  } while (input[0] != 0x71); /* Exit the Socket client with character "q" */

  cur_state = STATE_QUIT;
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


static int teseo_trace_redirect()
{
  int socketfd, connectedSocketfd;
  int c;
  int option = 1;
  struct sockaddr_in server, client;
  int len;
  char buf[SZ+1];
  struct termios newtio;
  char CmdLine[50];
  char GpioPathName[40];

  /* #####################  Open the UART port  ##################### */
  printf("Socket redirect : open UART %s\r\n", uart2host);
  uartfd = open(uart2host, O_RDWR | O_NOCTTY);
  if (uartfd == -1)
  {
    printf("Socket redirect - %s: Unable to open %s\r\n", __func__, uart2host);
    return errno;
  }

  tcgetattr(uartfd, &newtio);

  newtio.c_cflag = B115200;
  newtio.c_cflag |= CLOCAL | CREAD;
  newtio.c_cflag |= CS8;
  newtio.c_iflag &= ~(IXON|ICRNL|INLCR);
  newtio.c_oflag = 0;
  newtio.c_lflag = 0;
  newtio.c_cc[VMIN] = 1;
  newtio.c_cc[VTIME] = 0;

  if (tcflush(uartfd, TCIFLUSH)) {
    printf("Socket redirect - %s ERROR: tcflush\r\n", __func__);
    goto end;
  }

  if (tcsetattr(uartfd,TCSANOW,&newtio)) {
    printf("Socket redirect - %s ERROR: tcsetattr\r\n", __func__);
    goto end;
  }

  /* #####################  Create a socket point  ##################### */
  printf("Socket redirect : Create a socket point\n");
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
     printf("Socket redirect - %s: ERROR opening socket\r\n", __func__);
     goto end;
  }

  bzero((char *) &server, sizeof(server));

  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(EXTERNAL_PORT_NUMBER);

  printf("Socket redirect : binding... %d, port %d\r\n", socketfd, EXTERNAL_PORT_NUMBER);

  if(setsockopt(socketfd,SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(option)) < 0)
  {
    perror("Socket redirect -Error setsockopt");
    close(socketfd);
    return errno;
  }

  if (bind(socketfd, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    perror("Socket redirect -Error bind");
    close(socketfd);
    return errno;
  }

  printf("Socket redirect : listening ...\r\n");
  listen(socketfd, 3);

  c = sizeof(struct sockaddr_in);
  connectedSocketfd = accept(socketfd, (struct sockaddr *)&client, (socklen_t *) &c);
  printf("Socket redirect : New connection\r\n");

  /* ################ Start Teseo in Standalone mode  #################### */
  if (TeseoReset_flag == 1) {
    sprintf(GpioPathName,"/sys/class/gpio/gpio%d",GpioTeseoReset);
    if( access( GpioPathName, F_OK ) == -1 ) {
      printf("Socket redirect : Configure new GPIO %s\r\n",GpioPathName);
      sprintf(CmdLine, "echo %d > /sys/class/gpio/export", GpioTeseoReset);  system(CmdLine);
      sprintf(CmdLine, "echo out > %s/direction", GpioPathName); system(CmdLine);
      sleep(1);
    }
    printf("Socket redirect : Reset Teseo with %s\r\n",GpioPathName);
    sprintf(CmdLine, "echo 0 > %s/value", GpioPathName); system(CmdLine);
    sprintf(CmdLine, "echo 1 > %s/value", GpioPathName); system(CmdLine);
  }

  /* #######################  Display Exit message  ####################### */
  printf("\r\n\n");
  printf("*******************************************\r\n\n");
  printf("  Press q<return> to Quit the application  \r\n\n");
  printf("*******************************************\r\n\n");

  signal(SIGINT, sig_inthandler);

  /* #######################  Redirect Trace Flow  ####################### */
  while (cur_state != STATE_QUIT)
  {
    bzero(buf,sizeof(buf));

    len = read(uartfd, buf, SZ*sizeof(char));
    if (len < 0)
      printf("Socket redirect - UART read failed!\r\n");

    if(write(connectedSocketfd, buf, len) < 0)
      printf("Socket redirect - ERROR writing on socket!\r\n");
  }

  /* #######################  QUIT  ####################### */
end:
  stop_teseo();

  return errno;
}


int main(int argc, char *argv[])
{
  int ret;
  pthread_t tid;
  char model[SZ];
  char ClientOptions[20]="\0";
  int id = 1;
  long int value;
  int Host_uart_port = -1;
  int Gpio_user = 0, Host_uart_user = 0;

  printf("\r\n");
  get_appli_path(argv[0]);
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
       printf("Socket redirect : Host<->Teseo UART set to port %d\r\n", Host_uart_port);
    }

    if (strcmp("GPIO", argv[id]) == 0) {
       id++;
       errno = 0;
       value = strtol(argv[id], (char **)NULL, 10);
       GpioTeseoReset = (int)value;
       Gpio_user = 1;
       printf("Socket redirect : Teseo reset GPIO set to %d\r\n", GpioTeseoReset);
    }

    if (strcmp("-d" , argv[id]) == 0) {
       strcat(ClientOptions, " -d");
       printf("Socket redirect : display in STDOUT is enabled\r\n");
    }

    if (strcmp("-no_log", argv[id]) == 0) {
       strcat(ClientOptions, " -no_log");
       printf("Socket redirect : disable trace saving in LOG files\r\n");
    }

    if (strcmp("-no_reset" , argv[id]) == 0) {
       TeseoReset_flag = 0;
       printf("Socket redirect : Teseo reset canceled\r\n");
    }

    if (strcmp("-h" , argv[id]) == 0) {
       printf("Usage [%s]: %s [-h] [IN x] [GPIO z] [-d] [-no_log] [-no_reset]\r\n", APP_VERSION, argv[0]);
       printf("-h  : Display the \"Usage\" message\r\n");
       printf("IN   x : User can specify the Host<->Teseo UART port x of the host\r\n");
       printf("GPIO z : User can specify the GPIO z to reset the Teseo\r\n");
       printf("-d  : Enable the trace on the STDOUT\r\n");
       printf("-no_log : Disable trace saving in \"log\" files\r\n");
       printf("-no_reset : No Teseo reset performed at start/stop\r\n");
       printf("\r\n");
       goto end;
    }

    id++;
  }

  /* #######################  Set the Board parameters  ####################### */
  ret = get_board_model(model);
  if (ret) {
    printf("Socket redirect : Failed to retrieve board model!!\r\n");
    goto end;
  }

  for (id = 0; id<BOARD_NUMBERS ; id++)
  {
    if (strstr(model, board_params[id].model)) {
       if (Gpio_user == 0)
         GpioTeseoReset = board_params[id].gpio_reset;
       if (Host_uart_user == 0)
         Host_uart_port = board_params[id].uart_port;
    }
  }

  sprintf(uart2host,"%s%d", uart2host, Host_uart_port);

  /* #######################  Start Input handler ####################### */
  ret = pthread_create(&tid, NULL, &check_input, NULL);
  if (ret != 0)
    printf("Socket redirect : can't create thread :[%s]\r\n", strerror(ret));
  else
    printf("Socket redirect : Input Thread created successfully\r\n");

  /* #######################  Start socket client application  ####################### */
  strcpy(ClientAppliExec, BinaryPath);
  strcat(ClientAppliExec, SocketClientName);
  if( access( ClientAppliExec, X_OK ) == -1 ) {
    printf("\r\nSocket redirect : !! ERROR !! File %s is missing or not executable\r\n\n", ClientAppliExec);
    return 0;
  }

  strcat(ClientOptions, " ext &");
  strcat(ClientAppliExec, ClientOptions);
  system(ClientAppliExec);
  printf("\r\nSocket redirect : Start \"%s\"\r\n\n", ClientAppliExec);

  /* #######################  Start Trace redirection ####################### */
  ret = teseo_trace_redirect();

  /* #######################  QUIT  ####################### */
end:
  return ret;
}

