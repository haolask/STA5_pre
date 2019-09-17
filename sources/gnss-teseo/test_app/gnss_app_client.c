/*
 * gnss_app_client.c
 *
 * Linux application for Accordo GNSS subsystem.
 *
 * Author: Fabrice Deruy <fabrice.deruy@st.com> for ST-Microelectronics.
 *
 ********************************************************************************
 * Copyright (c) 2015 STMicroelectronics - All Rights Reserved
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

/*******************************
 * Modem and socket declarations
 *******************************/
#define PORT_NUMBER 5001
#define HOSTNAME "127.0.0.1"

#define SZ 255
#define NB_MAX_SAT 32
#define GPS_SIZE 512
#define MAX_TIME 60

typedef enum {
  STATE_IDLE,
  STATE_NULL
} state;

int socketfd;
int cur_state = STATE_IDLE;
char TeseoBinaryName[30];

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

/********************************************//**
 * \brief
 * \Function to get the GPS position
 * \param void
 * \return void
 *
 ***********************************************/
static int teseo_get_sat_data()
{
  int n, char_nb, cnt, err, len,idx, sat_count;
  char buf[SZ];
  char buf2[SZ];
  char *p;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char out[4];
  char *array[NB_MAX_SAT];

  /* Wait init GNSS lib */
  sleep (2);

  printf("GPS Client - %s: Create a socket point\r\n", __func__);

  /* Create a socket point */
  socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if (socketfd < 0) {
     printf("GPS Client - %s: ERROR opening socket\r\n", __func__);
    goto end;
  }
  server = gethostbyname(HOSTNAME);
  if (server == NULL) {
     printf("GPS Client - %s: ERROR, no such host\r\n", __func__);
     goto end;
  }

  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(PORT_NUMBER);

  printf("GPS Client - %s: Connect to server\r\n", __func__);

  /* Now connect to the server */
  if (connect(socketfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
  {
     printf("GPS Client - %s: ERROR connecting\r\n", __func__);
     goto end;
  }

  printf("GPS Client - %s: connected\r\n", __func__);

  cnt = 0;

  /* #######################  POSITION ACQUISITION  ####################### */
  do {
    printf("GPS Client - %s: Position acquisition state\r\n", __func__);
    fflush(stdout);

    sleep(1);

    if (cnt++ > MAX_TIME)
       break;

    /* Send message to the server to find position */
    n = write(socketfd, "p", 1);
    if (n != 1) {
       printf("GPS Client - %s: ERROR writing to socket to get position\r\n", __func__);
       goto end;
    }

    /* Now read integer to get number of chars to receive */
    n = read(socketfd, &char_nb, sizeof(char_nb));
    if (n < 0) {
       printf("GPS Client - %s: ERROR reading number of positioning chars\r\n", __func__);
       goto end;
    }

    char_nb = ntohl(char_nb);

    if (char_nb <= 0 || char_nb > SZ)
       continue;

    n = read(socketfd, buf, char_nb);

    if (n == char_nb)
       break;

  } while (1);

  if (n != char_nb) {
     printf("GPS Client - %s: Error no position received\r\n",__func__);
     err = -ENODATA;
     goto end;
  }

  buf[char_nb] = '\0';

  {
    char * lat_ptr;
    char * long_ptr;

    lat_ptr = buf;
    long_ptr = strstr(buf,",");
    *long_ptr = '\0'; /* replace comma character with a NULL character to split the buffer */
    long_ptr++;

    printf("GPS Client - %s: Latitude=%s  Longitude=%s\r\n",__func__, lat_ptr, long_ptr);
  }

  memset(buf, 0, sizeof(buf));

  /* #######################  SPEED ACQUISITION  ####################### */
  n = write(socketfd, "s", 1);

  if (n != 1) {
     printf("Client - %s: ERROR writing to speed socket\r\n", __func__);
     goto end;
  }

  /* Now read integer to get number of chars to receive */
  n = read(socketfd, &char_nb, sizeof(char_nb));

  if (n < 0) {
     printf("GPS Client - %s: ERROR reading number of speed chars\r\n", __func__);
     goto end;
  }

  char_nb = ntohl(char_nb);

  n = read(socketfd, buf, char_nb);

  if (n != char_nb) {
     printf("GPS Client - %s: ERROR reading socket to get speed\r\n", __func__);
     goto end;
  }

  buf[char_nb] = '\0';

  printf("GPS Client - %s: speed = %s km/h\r\n",__func__, buf);

  memset(buf, 0, sizeof(buf));


  /* ####################### LIST OF VISIBLE SATELLITES  ####################### */
  n = write(socketfd, "l", 1);

  if (n != 1) {
     printf("Client - %s: ERROR writing to socket to get list of sat\r\n", __func__);
     goto end;
  }

  /* Now read integer to get number of chars to receive */
  n = read(socketfd, &char_nb, sizeof(char_nb));

  if (n < 0) {
     printf("GPS Client - %s: ERROR reading 1\r\n", __func__);
     goto end;
  }

  char_nb = ntohl(char_nb);

  n = read(socketfd, buf, char_nb);

  if (n != char_nb) {
     printf("GPS Client - %s: ERROR reading 2\r\n", __func__);
     goto end;
  }

  buf[char_nb] = '\0';

  printf("GPS Client - %s: List of visible sats = %s\r\n",__func__, buf);

  memset(buf, 0, sizeof(buf));

end:
  close(socketfd);

  if (errno)
     err = errno;

  return err;
}


void stop_teseo(void)
{
  char CmdLine[40];
  int ret;

  do
  {
    sprintf(CmdLine, "pidof %s > /dev/null", TeseoBinaryName);
    ret = system(CmdLine);
    if (ret == 0)
    {
      printf("\r\nGPS Client : Kill %s\r\n\n", TeseoBinaryName);
      sprintf(CmdLine, "killall -9 %s", TeseoBinaryName);
      ret = system(CmdLine);
    }
  } while (ret != 256);
}

static int start_teseo(char *model)
{
  char TeseoAppliExec[50];

  /* Enable loopback */
  if (system("ifconfig lo up"))
     return -ENODEV;

  /* Check Teseo type to be used */
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

  #ifdef DR_CODE_LINKED
  strcat(TeseoBinaryName, "_DR");
  #endif
  strcat(TeseoBinaryName, ".bin");

  /* Activate Teseo GNSS lib */
  strcpy(TeseoAppliExec, "/usr/bin/");
  strcat(TeseoAppliExec, TeseoBinaryName);

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

  printf("GPS Client - %s %s\r\n",__func__,model);

  teseo_get_sat_data();

  return 0;
}

/********************************************//**
 * \brief
 *
 * \param void
 * \return void
 *
 ***********************************************/
int main()
{
  int ret;
  char model[SZ];

  ret = get_board_model(model);

  if (ret) {
     printf("GPS Client - Error: Failed to retrieve board model\r\n");
     goto end;
  }

  ret = start_teseo(model);
  if (ret)
     printf("GPS Client - Error: Failed to start Teseo thread\r\n");

  stop_teseo();

end:
  return ret;
}

