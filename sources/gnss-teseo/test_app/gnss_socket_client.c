/*
 * gnss_socket_client.c
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
#include <signal.h>

#define APP_VERSION "v1.1"

/*******************************
 * Modem and socket declarations
 *******************************/
#define NMEA_PORT_NUMBER     6000
#define DEBUG_PORT_NUMBER    7000
#define EXTERNAL_PORT_NUMBER 8000
#define HOSTNAME "127.0.0.1"

#define SZ 255
#define MAX_NMEA_MSG_LENGTH 512
#define TIME_DIFFERENCE_CHECK (60*60*24)   //1 day in seconds
#define FILENAME_FORMAT "/gnss_%s_%d%02d%02d-%02d_%02d_%02d.log"

typedef enum {
STATE_RUNNING,
STATE_QUIT
} state_t;

typedef enum {
NONE_FLOW		= 0x0,
DEBUG_FLOW		= 0x1,
NMEA_FLOW		= 0x2,
EXTERNAL_FLOW	= 0x4
}FlowType_t;

typedef enum {
TESEO_NONE,
TESEO2,
TESEO3
}ChipType_t;

typedef struct
{
  FlowType_t FlowID;
  int PortNumber;
  char FlowName[10];
} task_params_t;

typedef void* (*task_function_t)(void*);

typedef struct
{
  pthread_t tid;
  task_function_t function_ptr;
  task_params_t *param_ptr;
} task_init_t;

state_t    cur_state = STATE_RUNNING;
ChipType_t ChipType  = TESEO_NONE;
int FlowList_Active  = NONE_FLOW;
int FlowList_Enabled = NONE_FLOW;

int  socketfd;
int  StdoutDisplayEnabled = 0;
int  LogOutputEnabled = 1;
char TeseoBinaryName[30];

#define FLOW_NUMBERS 3
task_params_t Flow_params[FLOW_NUMBERS]={{DEBUG_FLOW   , DEBUG_PORT_NUMBER   , "debug"},
                                         {NMEA_FLOW    , NMEA_PORT_NUMBER    , "nmea" },
                                         {EXTERNAL_FLOW, EXTERNAL_PORT_NUMBER, "ext"  }};


/*******************************
 * Functions
 *******************************/

void stop_teseo(void)
{
  char CmdPID[100];
  char CmdLine[40];
  int ret;

  if ( (FlowList_Active & EXTERNAL_FLOW) == 0 )
  {
   do
   {
     sprintf(CmdLine, "pidof %s > /dev/null", TeseoBinaryName);
     ret = system(CmdLine);
     if (ret == 0)
     {
       printf("Socket Client : Kill %s\r\n", TeseoBinaryName);
       sprintf(CmdLine, "killall -9 %s", TeseoBinaryName);
       ret = system(CmdLine);
     }
   } while (ret != 256);
  }
}

void sig_inthandler()
{
  printf("Socket Client : SIG INT HANDLER called\r\n");
  stop_teseo();
  exit(0);
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


static void teseo_get_trace_flow(char *FlowName, int PortNumber)
{
  int n, err;
  char buf[SZ+1];
  struct hostent *server;
  struct sockaddr_in serv_addr;
  FILE *fp;
  char StartFile[40];
  char NewFile[40];
  time_t rawtime, Start_rawtime;
  struct tm * timeinfo;

  /* #######################  Open output log file  ####################### */
  if (LogOutputEnabled == 1) {
    time ( &Start_rawtime );
    timeinfo = localtime ( &Start_rawtime );

    sprintf(StartFile, FILENAME_FORMAT, FlowName, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

    fp = fopen(StartFile, "w");
    if (!fp)
      return;
    printf("Socket Client : Create file %s\r\n", StartFile);
  }

  /* #######################  Wait init GNSS lib  ####################### */
  sleep (2);

  /* #######################  Create a socket point  ####################### */
  printf("Socket Client : Create a socket point\r\n");
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
    printf("Socket Client : ERROR opening socket\r\n");
    goto end;
  }
  server = gethostbyname(HOSTNAME);
  if (server == NULL) {
    printf("Socket Client : ERROR, no such host\r\n");
    goto end;
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(PortNumber);

  /* #######################  Connect to the server  ####################### */
  printf("Socket Client : Connect to server port %d\r\n", PortNumber);

  if (connect(socketfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
  {
    printf("Socket Client : ERROR connecting\r\n");
    goto end;
  }

  printf("Socket Client : port %d connected\r\n\n", PortNumber);


  /* #######################  Get & save Trace Flow  ####################### */
  do {
    bzero(buf,sizeof(buf));
    n = read(socketfd, buf, SZ*sizeof(char));
    if (StdoutDisplayEnabled == 1) printf("%s", buf);
    if (LogOutputEnabled == 1)
    {
      /* Fill output File */
      fprintf(fp,"%s", buf);

      /* Check if the date has been updated to a real date */
      /* or the log was started for more than 1 day        */
      /* Change the name of the output File                */
      time ( &rawtime );
      timeinfo = localtime ( &rawtime );

      if (abs(rawtime - Start_rawtime) > TIME_DIFFERENCE_CHECK)
      {
         sprintf(NewFile, FILENAME_FORMAT, FlowName, timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
         rename(StartFile, NewFile);
         printf("Socket Client : Rename file %s into %s\r\n", StartFile, NewFile);
         strcpy(StartFile, NewFile);
         Start_rawtime = rawtime;
      }
    }

  } while (cur_state == STATE_RUNNING);

  /* #######################  QUIT  ####################### */
end:
  printf("\r\n***** Quit %s Socket Client *****\r\n", FlowName);
  stop_teseo();

  close(socketfd);
  if (errno)
    err = errno;

  if (LogOutputEnabled == 1)
    fclose(fp);

  cur_state = STATE_QUIT;
}


static int start_teseo(char *model)
{
  char TeseoAppliExec[50];
  char CmdLine[40];
  int ret;

  /* #######################  Check Teseo type to be used  ####################### */
  if (ChipType == TESEO2) strcpy(TeseoBinaryName, "T2-gnssapp");
  else if (ChipType == TESEO3) strcpy(TeseoBinaryName, "T3-gnssapp");
  else {
    if (strstr(model,"Telemaco2")) {
      if (strstr(model,"EVB rev2")) {
        strcpy(TeseoBinaryName, "T3-gnssapp");
      }
      else {
        strcpy(TeseoBinaryName, "T2-gnssapp");
      }
    }
    else {
      strcpy(TeseoBinaryName, "T3-gnssapp");
    }
  }

  #ifdef DR_CODE_LINKED
  strcat(TeseoBinaryName, "_DR");
  #endif
  strcat(TeseoBinaryName, ".bin");

  /* #######################  Activate Teseo GNSS lib  ####################### */
  strcpy(TeseoAppliExec, "/usr/bin/");
  strcat(TeseoAppliExec, TeseoBinaryName);

  sprintf(CmdLine, "pidof %s > /dev/null", TeseoBinaryName);
  ret = system(CmdLine);
  if (ret == 0)
  {
    printf("\r\nSocket Client : %d GNSS application %s already started\r\n\n", ret, TeseoAppliExec);
    return 0;
  }

  if( access( TeseoAppliExec, X_OK ) == -1 ) {
    printf("\r\nSocket Client : !! ERROR !! File %s is missing or not executable\r\n\n", TeseoAppliExec);
    return 0;
  }
  else {
    printf("Socket Client : Starting GNSS application \"%s\"\r\n", TeseoAppliExec);
    strcat(TeseoAppliExec, "&");
    if (system(TeseoAppliExec))
       return -ENOSYS;
  }

  printf("Socket Client : Model - %s\r\n",model);
  return 0;
}


void* handle_flow(void* arg)
{
  task_params_t *params = arg;
  printf("Socket Client : Start thread [%s] with port %d\r\n", params->FlowName, params->PortNumber);
  teseo_get_trace_flow(params->FlowName, params->PortNumber);
  FlowList_Active &= ~params->FlowID;
}

void send_nmea_cmd(char* input, int length)
{
  int n;

  if (strstr(input,"$") != NULL)
  {
     n = write(socketfd, input, length);

     if (n != length) {
       printf("Socket Client - %s: ERROR sending NMEA command %s\r\n", __func__, input);
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
    printf("Socket Client : read input %s\r\n", input);

    send_nmea_cmd(input, strlen(input));

  } while (input[0] != 0x71); /* Exit the Socket client with character "q" */

  cur_state = STATE_QUIT;
}


int task_handler_init(task_init_t *task)
{
  int ret = 0;

  ret = pthread_create(&task->tid, NULL, task->function_ptr, (void*)task->param_ptr);
  if (ret != 0)
    printf("Socket Client : can't create thread [%s]\r\n", strerror(ret));
  else
    printf("Socket Client : Thread created successfully\r\n");
}


int main(int argc, char *argv[])
{
  task_init_t *task[FLOW_NUMBERS+1];
  int id = 1;
  int TaskID = 0;
  char model[SZ];

  printf("\r\n");

  /* #######################  Check application parameters  ####################### */
  while (id < argc)
  {
    if (strcmp("-d"   , argv[id]) == 0) {
       StdoutDisplayEnabled = 1;
       printf("Socket Client : display in STDOUT is enabled\r\n");
    }
    if (strcmp("-no_log"   , argv[id]) == 0) {
       LogOutputEnabled = 0;
       printf("Socket Client : disable trace saving in LOG files\r\n");
    }
    if (strcmp("ext"  , argv[id]) == 0) {
       FlowList_Active |= EXTERNAL_FLOW;
    }
    if (strcmp("debug", argv[id]) == 0) {
       FlowList_Active |= DEBUG_FLOW;
    }
    if (strcmp("nmea" , argv[id]) == 0) {
       FlowList_Active |= NMEA_FLOW;
    }
    if (strcmp("T2"   , argv[id]) == 0) { ChipType = TESEO2; }
    if (strcmp("T3"   , argv[id]) == 0) { ChipType = TESEO3; }

    id++;
  }

  /* #######################  Print Usage if needed  ####################### */
  if (FlowList_Active == NONE_FLOW) {
     printf("Usage [%s]: %s debug|nmea [T2|T3] [-d] [-no_log]\r\n", APP_VERSION, argv[0]);
     printf("debug|nmea : Choose the trace to be handled via socket\r\n");
     printf("             (By default traces are saved in \"log\" files)\r\n");
     printf("T2|T3      : Specify the Teseo chip (if needed)\r\n");
     printf("-d         : Enable the trace on the STDOUT\r\n");
     printf("-no_log    : Disable trace saving in \"log\" files\r\n");
     printf("\r\n");
     goto end;
  }

  /* #######################  Start Input handler and Teseo SW  ####################### */
  if ( (FlowList_Active & EXTERNAL_FLOW) == 0 )
  {
    int ret = 0;

    ret = get_board_model(model);
    if (ret) {
      printf("Socket Client : Failed to retrieve board model!!\r\n");
      goto end;
    }

    task[TaskID] = (task_init_t *)malloc(sizeof(task_init_t));
    task[TaskID]->function_ptr = check_input;
    task[TaskID]->param_ptr    = NULL;
    task_handler_init(task[TaskID]);
    TaskID++;

    if ( FlowList_Active != NONE_FLOW ) {
      ret = start_teseo(model);
      if (ret)
        printf("Socket Client - Error: Failed to start Teseo thread\r\n");
    }
  }

  /* #######################  Start flow handlers  ####################### */
  for(id = 0; id < FLOW_NUMBERS; id++) {
    if ((FlowList_Active & Flow_params[id].FlowID) != 0) {
       task[TaskID]=(task_init_t *)malloc(sizeof(task_init_t));
       task[TaskID]->function_ptr = handle_flow;
       task[TaskID]->param_ptr    = &Flow_params[id];
       task_handler_init(task[TaskID]);
       TaskID++;
       FlowList_Enabled |= Flow_params[id].FlowID;
       printf("Socket Client : Log %s is enabled\r\n", Flow_params[id].FlowName);
    }
  }

  /* #######################  Display Exit message  ####################### */
  if (( FlowList_Active & EXTERNAL_FLOW) == 0)
  {
    printf("\r\n\n");
    printf("*******************************************\r\n\n");
    printf("  Press q<return> to Quit the application  \r\n\n");
    printf("*******************************************\r\n\n");
  }

  signal(SIGINT, sig_inthandler);

  /* #######################  Wait User request  ####################### */
  while ((cur_state == STATE_RUNNING) || (FlowList_Active != 0)) {
    sleep(1);
  }

  /* #######################  QUIT  ####################### */
end:
  stop_teseo();
  printf("\r\n\n");
  printf("!! %s application Ended !!", argv[0]);
  printf("\r\n\n");
  return 1;
}

