//!
//!  \file		radio_hmi.c
//!  \brief		<i><b> Human-machine interface for Accordo 2 radio tuners</b></i>
//!  \details
//!  \author	Giuseppe Gorgoglione <giuseppe.gorgoglione@st.com>
//!

#include "target_config.h"

#include "etal_api.h"
#include "etaltml_api.h"


#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <poll.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <linux/fb.h>

#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <libevdev-1.0/libevdev/libevdev.h>

enum hmi_etal_command_id {
	/*
	 * System initialization and configuration
	 */
	ETAL_INITIALIZE,
	ETAL_GET_CAPABILITIES,
	ETAL_CONFIG_RECEIVER,
	ETAL_DESTROY_RECEIVER,
	ETAL_CONFIG_DATAPATH,
	ETAL_DESTROY_DATAPATH,
	ETAL_DEINITIALIZE,
	ETAL_CONFIG_AUDIO_PATH,
	ETAL_AUDIO_SELECT,
	ETAL_CHANGE_BAND_RECEIVER,

	/*
	 * Tune/Seek/Quality
	 */
	ETAL_TUNE_RECEIVER,
	ETAL_TUNE_RECEIVER_SYNC,
	ETAL_SEEK_START,
	ETAL_SEEK_STOP,
	ETAL_GET_RECEPTION_QUALITY,
	ETAL_GET_RECEPTION_QUALITY_AD,
	ETAL_CONFIG_RECEPTION_QUALITY_MONITOR,
	ETAL_DESTROY_RECEPTION_QUALITY_MONITOR,
	ETAL_GET_RECEIVER_FREQUENCY,

	/*
	 * Radio text
	 */
	ETAL_GET_RADIOTEXT,
	ETAL_START_RADIOTEXT,
	ETAL_STOP_RADIOTEXT,
	ETAL_GET_DECODED_RDS,
	ETAL_START_RDS,
	ETAL_STOP_RDS,

	/*
	 * Advanced Tuning
	 */
	ETAL_GET_CURRENT_ENSEMBLE,
	ETAL_SERVICE_SELECT_AUDIO,
	ETAL_SCAN_START,
	ETAL_SCAN_STOP,
	ETAL_LEARN_START,
	ETAL_LEARN_STOP,

	/*
	 * System Data
	 */
	ETAL_GET_ENSEMBLE_LIST,
	ETAL_GET_ENSEMBLE_DATA,
	ETAL_GET_SERVICE_LIST,
	ETAL_GET_SPECIFIC_SERVICE_DATA_DAB,

	/*
	 * Digital Radio services
	 */
#if 0
	ETAL_GET_PAD_DLS,
	ETAL_EVENT_PAD_DLS_START,
	ETAL_EVENT_PAD_DLS_STOP,
#endif

	/*
	 * Enhanced services
	 */
#if 0
	ETAL_START_TPEG_RAW,
	ETAL_STOP_TPEG_RAW,
#endif

	/*
	 * Seamless estimation and seamless switching
	 */
	ETAL_SEAMLESS_ESTIMATION_START,
	ETAL_SEAMLESS_ESTIMATION_STOP,
	ETAL_SEAMLESS_SWITCHING,

	/*
	 * Read / write parameter
	 */
	ETAL_READ_PARAMETER,
	ETAL_WRITE_PARAMETER,

	/*
	 * Invalid ETAL command
	 */
	ETAL_COMMAND_INVALID,
};

#define HMI_MAX_ETAL_RECEIVERS 2
#define HMI_MAX_ETAL_DATAPATHS 2	// TODO: to be verified

struct hmi_etal_status {
	ETAL_HANDLE recv_handle[HMI_MAX_ETAL_RECEIVERS];
	EtalReceiverAttr recv_attr[HMI_MAX_ETAL_RECEIVERS];

	ETAL_HANDLE dpth_handle[HMI_MAX_ETAL_DATAPATHS];
	EtalReceiverAttr dpth_attr[HMI_MAX_ETAL_DATAPATHS];
};

struct hmi_serial_port
{
	int fd;
	struct termios oldtio;
	struct termios newtio;
};

/*
 * UGLY HACK: we need a global context here to store some info because
 *            hmi_etal_initialize() does not allow us to register a pointer
 *            to a local context whose value is provided back by
 *            hmi_etal_notification_handler()
 */
static struct hmi_global_context {
	struct hmi_serial_port *serial_port;
	struct hmi_etal_status *etal_status;
	FILE *log_file;
} hmi_global_context;

/* freqList used for learn start */
tU32 freqList[ETAL_LEARN_MAX_NB_FREQ];

const unsigned char tux [] = {
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x03,0x07,0x0F,0x0F,0x0F,0x1F,0x1F,0x1F,0x1F,0x0F,0x0F,0x07,0x07,
		0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0xFF,0xFF,0xF7,0xF7,0xF0,0xFE,0xFF,0xFE,0xF1,0xE7,0xE7,0xF0,0xF8,0xFF,
		0xFF,0xFF,0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x01,0xFF,0xFF,0x58,0x8C,0x8C,0x06,0x0E,0x06,0x0C,0x08,0x90,0x00,0xF8,0xFC,
		0xFF,0xFF,0xFF,0x1F,0x07,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x07,0x07,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x00,
		0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x07,0x1F,0x3F,
		0xFC,0xF0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x80,0xE0,0xFC,0xFE,0xFF,0xFF,0x7F,0x3F,0x1F,0x07,0x01,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0xFF,0xFF,0x3F,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0x38,0x18,
		0x0C,0x06,0x03,0x01,0x00,0x00,0xFF,0xFF,0x3F,0x00,0x00,0x00,0xFF,0xFF,0x3F,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x3F,0x36,0x00,0x00,0x80,0xC0,0xF1,0x3F,
		0x1E,0x3F,0x77,0xEF,0xBC,0x38,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x0F,0x3F,0xFF,0xFF,0xE0,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0xF8,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,0x06,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0xF0,0xF0,0xFC,0xFC,0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,
		0x06,0x00,0x00,0x00,0xF0,0xF4,0xFC,0x24,0x00,0x00,0x10,0xF0,0xFC,0xFC,0x00,0x00,
		0x00,0x00,0x00,0x80,0xE0,0xF0,0xF8,0xFC,0xFC,0x00,0x00,0x00,0xC0,0xF0,0xF8,0x3C,
		0x34,0x34,0x34,0x34,0x34,0x64,0xEC,0xFC,0xF8,0xF0,0x00,0x10,0x70,0xF4,0x9C,0x3C,
		0x70,0xC0,0xC0,0xC0,0x70,0x3C,0x0C,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,0x80,0x80,0xC0,0xC0,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x01,0x01,0x00,0xFE,0xFC,0xFE,0xFE,0xFC,0xF8,0xF0,0xC0,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

		0x00,0x00,0x00,0x00,0x08,0x0C,0x04,0x04,0x06,0x06,0x06,0x03,0x03,0x01,0x01,0x01,
		0x01,0x03,0x07,0x1F,0x0F,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x0E,0x1E,0x1E,0x3E,0x7E,
		0x7E,0xFF,0xFF,0x27,0x01,0x01,0x03,0x03,0x07,0x04,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,
		0x80,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

static unsigned long int hmi_get_timestamp()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (tv.tv_sec * 1000000 + tv.tv_usec);
}

#define HMI_CMD_TRACE(func, ...) \
	do { \
		if (hmi_global_context.log_file) { \
			fprintf(hmi_global_context.log_file, "CMD (%lu): %s()\n", \
					hmi_get_timestamp(), \
					func, ##__VA_ARGS__); \
		} \
	} while (0)

#define HMI_ACK_TRACE(cmd, target) \
	do { \
		if (hmi_global_context.log_file) { \
			fprintf(hmi_global_context.log_file, "ACK (%lu): command %s() on target %d\n", \
					hmi_get_timestamp(), \
					cmd, target); \
		} \
	} while (0)

#define HMI_PAR_TRACE(args...) \
	do { \
		if (hmi_global_context.log_file) { \
			fprintf(hmi_global_context.log_file, "\t\t\t" args); \
		} \
	} while (0)

/******************************************************************************
 *
 * Display device
 *
 *****************************************************************************/

struct hmi_display_device
{
	int fd;
	struct fb_fix_screeninfo fix_info;
	struct fb_var_screeninfo var_info;
	int fb_size;
	unsigned char *fb;
};

static struct hmi_display_device *hmi_display_device_open(char *device_name)
{
	struct hmi_display_device *display_device = calloc(1, sizeof(struct hmi_display_device));
	if (!display_device)
		return NULL;

	display_device->fd = open(device_name, O_RDWR);
	if (display_device->fd < 0) {
		fprintf(stderr, "Failed to open %s device\n", device_name);
		free(display_device);
		return NULL;
	}

	int rc = ioctl(display_device->fd, FBIOGET_FSCREENINFO, &display_device->fix_info);
	if (rc < 0) {
		fprintf(stderr, "Failed to get screen info: %s\n", strerror(-rc));
		free(display_device);
		return NULL;
	}

	rc = ioctl(display_device->fd, FBIOGET_VSCREENINFO, &display_device->var_info);
	if (rc < 0) {
		fprintf(stderr, "failed to get variable screen info: %s\n", strerror(-rc));
		free(display_device);
		return NULL;
	}

	display_device->fb_size = display_device->var_info.xres *
			display_device->var_info.yres *
			display_device->var_info.bits_per_pixel / 8;

	display_device->fb = mmap(NULL, display_device->fb_size, PROT_READ | PROT_WRITE,
			MAP_SHARED, display_device->fd, 0);
	if (!display_device->fb) {
		fprintf(stderr, "Failed to mmap the framebuffer (%s)\n", strerror(errno));
		free(display_device);
		return NULL;
	}

	/*
	 * clear screen to black
	 */
	memset(display_device->fb, 0, display_device->fb_size);

	return display_device;
}

static void hmi_display_device_close(struct hmi_display_device *display_device)
{
	munmap(display_device->fb, display_device->fb_size);

	close(display_device->fd);

	free(display_device);
}

static void hmi_display_device_render(struct hmi_display_device *display_device, const unsigned char *img)
{
	memcpy(display_device->fb, img, display_device->fb_size);

	int rc = ioctl(display_device->fd, FBIOPAN_DISPLAY, &display_device->var_info);
	if (rc < 0) {
		fprintf(stderr, "Failed to render to screen (%s)\n", strerror(-rc));
		exit(EXIT_FAILURE);
	}
}

/******************************************************************************
 *
 * Input devices
 *
 *****************************************************************************/

struct hmi_input_device
{
	int fd;
	struct libevdev *dev;
};

static struct hmi_input_device *hmi_input_device_open(char *device_name)
{
	struct hmi_input_device *input_device = calloc(1, sizeof(struct hmi_input_device));
	if (!input_device)
		return NULL;

	input_device->fd = open(device_name, O_RDONLY | O_NONBLOCK);
	if (input_device->fd < 0) {
		fprintf(stderr, "Failed to open %s device\n", device_name);
		free(input_device);
		return NULL;
	}

	int rc = libevdev_new_from_fd(input_device->fd, &input_device->dev);
	if (rc < 0) {
		fprintf(stderr, "Failed to init %s device (%s)\n", device_name, strerror(-rc));
		free (input_device);
		return NULL;
	}

	printf("Input device name: \"%s\"\n", libevdev_get_name(input_device->dev));
	printf("Input device ID: bus %#x vendor %#x product %#x\n",
			libevdev_get_id_bustype(input_device->dev),
			libevdev_get_id_vendor(input_device->dev),
			libevdev_get_id_product(input_device->dev));

	return input_device;
}

static void hmi_input_device_close(struct hmi_input_device *input_device)
{
	libevdev_free(input_device->dev);

	close(input_device->fd);

	free(input_device);
}

/******************************************************************************
 *
 * Serial interface
 *
 *****************************************************************************/

static struct hmi_serial_port *hmi_serial_port_open(char *device_name)
{
	struct hmi_serial_port *serial_port = calloc(1, sizeof(struct hmi_serial_port));
	if (!serial_port)
		return NULL;

	serial_port->fd = open(device_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (serial_port->fd < 0) {
		fprintf(stderr, "Failed to open %s device\n", device_name);
		free(serial_port);
		return NULL;
	}

	/* save current serial port settings */
	tcgetattr(serial_port->fd, &serial_port->oldtio);

	memset(&serial_port->newtio, 0, sizeof(serial_port->newtio));

	/*
	 * B115200 : Set 1152000 bps rate.
	 * CS8     : 8bit, no parity, 1 stopbit
	 * CLOCAL  : local connection, no modem control
	 * CREAD   : enable receiving characters
	 */
	serial_port->newtio.c_cflag = B115200 | CS8 | CLOCAL | CREAD;

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

static void hmi_serial_port_close(struct hmi_serial_port *serial_port)
{
	/* restore the old port settings */
	tcsetattr(serial_port->fd, TCSANOW, &serial_port->oldtio);

	close(serial_port->fd);

	free(serial_port);
}

static bool hmi_serial_read_8(struct hmi_serial_port *serial_port, tU8 *value)
{
	int num = read(serial_port->fd, value, 1);

	if (num < 1)
		return false;

	return true;
}

/* currently not used
static bool hmi_serial_read_16(struct hmi_serial_port *serial_port, tU16 *value)
{
	char tmp[2];

	int num = read(serial_port->fd, tmp, 2);

	if (num < 2)
		return false;

	*value = (tmp[0] << 8) | tmp[1];

	return true;
} */

static bool hmi_serial_read_32(struct hmi_serial_port *serial_port, tU32 *value)
{
	char tmp[4];

	int num = read(serial_port->fd, tmp, 4);

	if (num < 4)
		return false;

	*value = (tmp[0] << 24) | (tmp[1] << 16) | (tmp[2] << 8) | tmp[3];

	return true;
}

static inline void hmi_write_8(char *buffer, int *pos, tU8 value)
{
	buffer[*pos + 0] = value;

	*pos += 1;
}

static inline void hmi_write_16(char *buffer, int *pos, tU16 value)
{
	buffer[*pos + 0] = (char)((value >> 8) & 0xFF);
	buffer[*pos + 1] = (char)((value >> 0) & 0xFF);

	*pos += 2;
}

static inline void hmi_write_32(char *buffer, int *pos, tU32 value)
{
	buffer[*pos + 0] = (char)((value >> 24) & 0xFF);
	buffer[*pos + 1] = (char)((value >> 16) & 0xFF);
	buffer[*pos + 2] = (char)((value >>  8) & 0xFF);
	buffer[*pos + 3] = (char)((value >>  0) & 0xFF);

	*pos += 4;
}

static void hmi_serial_send_ack(struct hmi_serial_port *serial_port, int command, int target, int ret, char *payload, int size)
{
	const char footer = 0xEF;
	char buffer[7];
	int pos = 0;

	hmi_write_8(buffer, &pos, 0xFE);
	hmi_write_8(buffer, &pos, command);
	hmi_write_8(buffer, &pos, target);

	hmi_write_32(buffer, &pos, ret);

	assert(sizeof(buffer) == pos);

	write(serial_port->fd, buffer, pos);

	if (size > 0)
		write(serial_port->fd, payload, size);

	write(serial_port->fd, &footer, 1);
}

static void hmi_serial_handle_get_capabilities(struct hmi_serial_port *serial_port, int target)
{
	const EtalHwCapabilities *caps;

	ETAL_STATUS rc = etal_get_capabilities(&caps);

	HMI_CMD_TRACE("etal_get_capabilities");

	char buffer[40];
	int pos = 0;

	hmi_write_8(buffer, &pos, caps->m_FeCount);

	int i, j, k;

	for (i = 0; i < 3; i++) {
		hmi_write_32(buffer, &pos, caps->m_BroadcastStandard[i].m_Standard);

		for (j = 0; j < 3; j++) {
			hmi_write_8(buffer, &pos, caps->m_BroadcastStandard[i].m_Diversity[j].m_DiversityMode);

			for (k = 0; k < 2; k++) {
				hmi_write_8(buffer, &pos, caps->m_BroadcastStandard[i].m_Diversity[j].m_FeConfig[k]);
			}
		}
	}

	assert(sizeof(buffer) == pos);

	hmi_serial_send_ack(serial_port, ETAL_GET_CAPABILITIES, target, rc, buffer, pos);

	HMI_ACK_TRACE("etal_get_capabilities", target);
}

static void hmi_serial_handle_config_receiver(struct hmi_serial_port *serial_port, int target)
{
	tU8 frontend, standard;

	if ((target < 0) || (target > HMI_MAX_ETAL_RECEIVERS)) {
		hmi_serial_send_ack(serial_port, ETAL_CONFIG_RECEIVER, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_8(serial_port, &frontend)) {
		hmi_serial_send_ack(serial_port, ETAL_CONFIG_RECEIVER, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_8(serial_port, &standard)) {
		hmi_serial_send_ack(serial_port, ETAL_CONFIG_RECEIVER, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	hmi_global_context.etal_status->recv_attr[target].m_Standard = standard;
	hmi_global_context.etal_status->recv_attr[target].m_FrontEnds[0] = frontend;

	ETAL_STATUS rc = etal_config_receiver(&hmi_global_context.etal_status->recv_handle[target],
										  &hmi_global_context.etal_status->recv_attr[target]);

	HMI_CMD_TRACE("etal_config_receiver");
	HMI_PAR_TRACE("receiver = %d\n", target);
	HMI_PAR_TRACE("frontend = %d\n", frontend);
	HMI_PAR_TRACE("standard = %d\n", standard);

	char buffer[1];
	int pos = 0;

	hmi_write_8(buffer, &pos, hmi_global_context.etal_status->recv_handle[target]);

	assert(sizeof(buffer) == pos);

	hmi_serial_send_ack(serial_port, ETAL_CONFIG_RECEIVER, target, rc, buffer, pos);

	HMI_ACK_TRACE("etal_config_receiver", target);
}

static void hmi_serial_handle_destroy_receiver(struct hmi_serial_port *serial_port, int target)
{
	ETAL_STATUS rc = ETAL_RET_NOT_INITIALIZED;

	int i;
	for (i = 0; i < HMI_MAX_ETAL_RECEIVERS; i++) {
		if (hmi_global_context.etal_status->recv_handle[i] == target) {
			rc = etal_destroy_receiver(&hmi_global_context.etal_status->recv_handle[i]);
		}
	}

	HMI_CMD_TRACE("etal_destroy_receiver");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_DESTROY_RECEIVER, target, rc, NULL, 0);

	HMI_ACK_TRACE("etal_destroy_receiver", target);
}

static void hmi_serial_handle_config_datapath(struct hmi_serial_port *serial_port, int target)
{
	tU8 data_type;

	if (!hmi_serial_read_8(serial_port, &data_type)) {
		hmi_serial_send_ack(serial_port, ETAL_CONFIG_DATAPATH, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	EtalDataPathAttr datapath_attr;
	datapath_attr.m_receiverHandle = target;
	datapath_attr.m_dataType = data_type;

	/*
	 * TODO: prepare some predefined sinks
	 */
	datapath_attr.m_sink.m_context = NULL;
	datapath_attr.m_sink.m_BufferSize = 0;
	datapath_attr.m_sink.m_CbProcessBlock = NULL;

	bool found = false;
	int i;

	for (i = 0; i < HMI_MAX_ETAL_DATAPATHS; i++) {
		if (hmi_global_context.etal_status->dpth_handle[i] == ETAL_INVALID_HANDLE) {
			found = true;
			break;
		}
	}

	if (!found)
		hmi_serial_send_ack(serial_port, ETAL_CONFIG_DATAPATH, target, ETAL_RET_ERROR, NULL, 0);

	ETAL_STATUS rc = etal_config_datapath(&hmi_global_context.etal_status->dpth_handle[i], &datapath_attr);

	HMI_CMD_TRACE("etal_config_datapath");
	HMI_PAR_TRACE("receiver = %d\n", target);

	char buffer[1];
	int pos = 0;

	hmi_write_8(buffer, &pos, hmi_global_context.etal_status->dpth_handle[i]);

	assert(sizeof(buffer) == pos);

	hmi_serial_send_ack(serial_port, ETAL_CONFIG_DATAPATH, target, rc, buffer, pos);

	HMI_ACK_TRACE("etal_config_datapath", target);
}

static void hmi_serial_handle_destroy_datapath(struct hmi_serial_port *serial_port, int target)
{
	ETAL_STATUS rc = ETAL_RET_NOT_INITIALIZED;

	int i;
	for (i = 0; i < HMI_MAX_ETAL_DATAPATHS; i++) {
		if (hmi_global_context.etal_status->dpth_handle[i] == target) {
			rc = etal_destroy_datapath(&hmi_global_context.etal_status->dpth_handle[i]);
		}
	}

	HMI_CMD_TRACE("etal_destroy_datapath");
	HMI_PAR_TRACE("datapath = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_DESTROY_DATAPATH, target, rc, NULL, 0);
}

static void hmi_serial_handle_config_audio_path(struct hmi_serial_port *serial_port, int target)
{
	EtalAudioInterfTy audioIf;

    memset(&audioIf, 0, sizeof(EtalAudioInterfTy));

	if (!hmi_serial_read_8(serial_port, (tU8 *) &audioIf)) {
		hmi_serial_send_ack(serial_port, ETAL_CONFIG_AUDIO_PATH, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	ETAL_STATUS rc = etal_config_audio_path(0, audioIf);

	HMI_CMD_TRACE("etal_config_audio_path");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_CONFIG_AUDIO_PATH, target, rc, NULL, 0);
}

static void hmi_serial_handle_audio_select(struct hmi_serial_port *serial_port, int target)
{
	EtalAudioSourceTy src = 0;

	if (!hmi_serial_read_8(serial_port, (tU8 *) &src)) {
		hmi_serial_send_ack(serial_port, ETAL_AUDIO_SELECT, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	ETAL_STATUS rc = etal_audio_select(src);

	HMI_CMD_TRACE("etal_audio_select");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_AUDIO_SELECT, target, rc, NULL, 0);
}

static void hmi_serial_handle_change_band_receiver(struct hmi_serial_port *serial_port, int target)
{
	tU32 band, fmin, fmax;
    EtalProcessingFeatures proc_features;

	if (!hmi_serial_read_32(serial_port, &band)) {
		hmi_serial_send_ack(serial_port, ETAL_CHANGE_BAND_RECEIVER, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &fmin)) {
		hmi_serial_send_ack(serial_port, ETAL_CHANGE_BAND_RECEIVER, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &fmax)) {
		hmi_serial_send_ack(serial_port, ETAL_CHANGE_BAND_RECEIVER, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_8(serial_port, &proc_features.u.m_processing_features)) {
		hmi_serial_send_ack(serial_port, ETAL_CHANGE_BAND_RECEIVER, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	ETAL_STATUS rc = etal_change_band_receiver(target, band, fmin, fmax, proc_features);

	HMI_CMD_TRACE("etal_change_band_receiver");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_CHANGE_BAND_RECEIVER, target, rc, NULL, 0);
}

static void hmi_serial_handle_tune_receiver(struct hmi_serial_port *serial_port, int target)
{
	tU32 frequency = 0;

	target |= 0x2000;
	if (!hmi_serial_read_32(serial_port, &frequency)) {
		hmi_serial_send_ack(serial_port, ETAL_TUNE_RECEIVER, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	ETAL_STATUS rc = etal_tune_receiver(target, frequency);

	HMI_CMD_TRACE("etal_tune_receiver");
	HMI_PAR_TRACE("receiver = %d\n", target);
	HMI_PAR_TRACE("frequency = %d\n", frequency);

	hmi_serial_send_ack(serial_port, ETAL_TUNE_RECEIVER, target, rc, NULL, 0);
}

static void hmi_serial_handle_seek_start(struct hmi_serial_port *serial_port, int target)
{
	tU8 flags = 0;
	tU32 amfm_step = 0;
	tU32 frequency = 0;

	if (!hmi_serial_read_8(serial_port, &flags)) {
		hmi_serial_send_ack(serial_port, ETAL_SEEK_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &amfm_step)) {
		hmi_serial_send_ack(serial_port, ETAL_SEEK_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &frequency)) {
		hmi_serial_send_ack(serial_port, ETAL_SEEK_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	bool direction = (flags & 0x01) != 0;

	ETAL_STATUS rc = etal_autoseek_start(target, direction, amfm_step, frequency, seekInSPS, TRUE); // TODO frequency is no longer a frequency, it is etalSeekAudioTy

	HMI_CMD_TRACE("etal_autoseek_start");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_SEEK_START, target, rc, NULL, 0);
}

static void hmi_serial_handle_seek_stop(struct hmi_serial_port *serial_port, int target)
{
	tU8 flags = 0;

	if (!hmi_serial_read_8(serial_port, &flags)) {
		hmi_serial_send_ack(serial_port, ETAL_SEEK_STOP, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	EtalSeekTerminationModeTy mode = (EtalSeekTerminationModeTy)(flags && 0x01);
	
	ETAL_STATUS rc = etal_autoseek_stop(target, mode);

	HMI_CMD_TRACE("etal_autoseek_stop");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_SEEK_STOP, target, rc, NULL, 0);
}

static void hmi_serial_handle_get_reception_quality(struct hmi_serial_port *serial_port, int target)
{
	EtalBcastQualityContainer quality;

	ETAL_STATUS rc = etal_get_reception_quality(target, &quality);

	HMI_CMD_TRACE("etal_get_reception_quality");
	HMI_PAR_TRACE("receiver = %d\n", target);

	char buffer[40];
	int pos = 0;

	hmi_write_32(buffer, &pos, quality.m_TimeStamp);
	hmi_write_32(buffer, &pos, quality.m_standard);

	int i;
	char *data = (char *)&quality.EtalQualityEntries;

	for (i = 0; i< sizeof(quality.EtalQualityEntries); i++)
		hmi_write_8(buffer, &pos, data[i]);

	assert(sizeof(buffer) == pos);

	hmi_serial_send_ack(serial_port, ETAL_GET_RECEPTION_QUALITY, target, rc, buffer, pos);
}

static void hmi_serial_handle_get_radio_text(struct hmi_serial_port *serial_port, int target)
{
	EtalTextInfo radio_text;
	memset(&radio_text, 0, sizeof(radio_text));

	ETAL_STATUS rc = etaltml_get_textinfo(target, &radio_text);

	HMI_CMD_TRACE("etaltml_get_textinfo");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_GET_RADIOTEXT, target, rc, NULL, 0);

	char buffer[167];
	int pos = 0;
	int i;

	int service_name_len = strlen(radio_text.m_serviceName);
	int current_info_len = strlen(radio_text.m_currentInfo);

	hmi_write_8(buffer, &pos, radio_text.m_broadcastStandard);
	hmi_write_8(buffer, &pos, radio_text.m_serviceNameIsNew ? 1 : 0);
	hmi_write_8(buffer, &pos, service_name_len);

	for (i = 0; i < service_name_len; i++)
		hmi_write_8(buffer, &pos, radio_text.m_serviceName[i]);

	hmi_write_8(buffer, &pos, radio_text.m_currentInfoIsNew ? 1 : 0);
	hmi_write_8(buffer, &pos, current_info_len);

	for (i = 0; i < current_info_len; i++)
		hmi_write_8(buffer, &pos, radio_text.m_currentInfo[i]);

	assert(sizeof(buffer) >= pos);

	write(serial_port->fd, buffer, pos);
}

static void hmi_serial_handle_start_radio_text(struct hmi_serial_port *serial_port, int target)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
    ETAL_STATUS rc = etaltml_start_textinfo(target);

	HMI_CMD_TRACE("etaltml_start_textinfo");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_START_RADIOTEXT, target, rc, NULL, 0);
#endif
}

static void hmi_serial_handle_stop_radio_text(struct hmi_serial_port *serial_port, int target)
{
#if defined (CONFIG_ETAL_HAVE_ETALTML) && defined (CONFIG_ETALTML_HAVE_RADIOTEXT)
	ETAL_STATUS rc = etaltml_stop_textinfo(target);

	HMI_CMD_TRACE("etaltml_stop_textinfo");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_STOP_RADIOTEXT, target, rc, NULL, 0);
#endif
}

static void hmi_serial_handle_get_decoded_rds(struct hmi_serial_port *serial_port, int target)
{
	EtalRDSData rds_data;

	ETAL_STATUS rc = etaltml_get_decoded_RDS(target, &rds_data);

	HMI_CMD_TRACE("etaltml_get_decoded_RDS");
	HMI_PAR_TRACE("receiver = %d\n", target);

	char buffer[119];
	int pos = 0;
	int i;

	hmi_write_32(buffer, &pos, rds_data.m_validityBitmap);

	for (i = 0; i < ETAL_DEF_MAX_PS_LEN; i++)
		hmi_write_8(buffer, &pos, rds_data.m_PS[i]);

	hmi_write_8(buffer, &pos, rds_data.m_DI);
	hmi_write_16(buffer, &pos, rds_data.m_PI);
	hmi_write_8(buffer, &pos, (rds_data.m_PTY & 0x1F) | (rds_data.m_TP << 5) | (rds_data.m_TP << 6) | (rds_data.m_MS << 7)); /* copy m_PTY, m_TP, m_TA, m_MS */
	hmi_write_8(buffer, &pos, rds_data.m_timeHour);
	hmi_write_8(buffer, &pos, rds_data.m_timeMinutes);
	hmi_write_8(buffer, &pos, rds_data.m_offset);
	hmi_write_8(buffer, &pos, rds_data.m_MJD);

	for (i = 0; i < ETAL_DEF_MAX_RT_LEN; i++)
		hmi_write_8(buffer, &pos, rds_data.m_RT[i]);

	hmi_write_32(buffer, &pos, rds_data.m_AFListLen);
	hmi_write_32(buffer, &pos, rds_data.m_AFListPI);

	for (i = 0; i < ETAL_DEF_MAX_AFLIST; i++)
		hmi_write_8(buffer, &pos, rds_data.m_AFList[i]);

	assert(sizeof(buffer) == pos);

	hmi_serial_send_ack(serial_port, ETAL_GET_DECODED_RDS, target, rc, buffer, pos);
}

static void hmi_serial_handle_start_decoded_rds(struct hmi_serial_port *serial_port, int target)
{
	ETAL_STATUS rc = etal_start_RDS(target, 0, 0);

	HMI_CMD_TRACE("etal_start_RDS");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_START_RDS, target, rc, NULL, 0);
}

static void hmi_serial_handle_stop_decoded_rds(struct hmi_serial_port *serial_port, int target)
{
	ETAL_STATUS rc = etal_stop_RDS(target);

	HMI_CMD_TRACE("etal_stop_RDS");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_STOP_RDS, target, rc, NULL, 0);
}

static void hmi_serial_handle_get_receiver_frequency(struct hmi_serial_port *serial_port, int target)
{
	tU32 frequency = 0;

	ETAL_STATUS rc = etal_get_receiver_frequency(target, &frequency);

	HMI_CMD_TRACE("etal_get_receiver_frequency");
	HMI_PAR_TRACE("receiver = %d\n", target);

	char buffer[4];
	int pos = 0;

	hmi_write_32(buffer, &pos, frequency);

	assert(sizeof(buffer) == pos);

	hmi_serial_send_ack(serial_port, ETAL_GET_RECEIVER_FREQUENCY, target, rc, buffer, pos);
}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

static void hmi_serial_handle_get_current_ensemble(struct hmi_serial_port *serial_port, int target)
{
	unsigned int ensemble;

	ETAL_STATUS rc = etal_get_current_ensemble(target, &ensemble);

	HMI_CMD_TRACE("etal_get_current_ensemble");
	HMI_PAR_TRACE("receiver = %d\n", target);

	char buffer[4];
	int pos = 0;

	hmi_write_32(buffer, &pos, ensemble);

	assert(sizeof(buffer) == pos);

	hmi_serial_send_ack(serial_port, ETAL_GET_CURRENT_ENSEMBLE, target, rc, buffer, pos);
}

#endif // ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

static void hmi_serial_handle_service_select_audio(struct hmi_serial_port *serial_port, int target)
{
	tU32 ueid, service, sc, subch;
	tU8 mode;

	if (!hmi_serial_read_8(serial_port, &mode)) {
		hmi_serial_send_ack(serial_port, ETAL_SERVICE_SELECT_AUDIO, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &ueid)) {
		hmi_serial_send_ack(serial_port, ETAL_SERVICE_SELECT_AUDIO, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &service)) {
		hmi_serial_send_ack(serial_port, ETAL_SERVICE_SELECT_AUDIO, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &sc)) {
		hmi_serial_send_ack(serial_port, ETAL_SERVICE_SELECT_AUDIO, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &subch)) {
		hmi_serial_send_ack(serial_port, ETAL_SERVICE_SELECT_AUDIO, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	ETAL_STATUS rc = etal_service_select_audio(target, mode, ueid, service, sc, subch);

	HMI_CMD_TRACE("etal_service_select_audio");
	HMI_PAR_TRACE("datapath / receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_SERVICE_SELECT_AUDIO, target, rc, NULL, 0);
}

static void hmi_serial_handle_scan_start(struct hmi_serial_port *serial_port, int target)
{
	tU8 flags = 0;
	tU8 playtime = 0;

	if (!hmi_serial_read_8(serial_port, &flags)) {
		hmi_serial_send_ack(serial_port, ETAL_SCAN_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_8(serial_port, &playtime)) {
		hmi_serial_send_ack(serial_port, ETAL_SCAN_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	bool direction_down = (flags & 0x01) != 0;
	bool play_last_bad  = (flags & 0x02) != 0;

	ETAL_STATUS rc = etaltml_scan_start(target, direction_down, playtime, play_last_bad);

	HMI_CMD_TRACE("etaltml_scan_start");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_SCAN_START, target, rc, NULL, 0);
}

static void hmi_serial_handle_scan_stop(struct hmi_serial_port *serial_port, int target)
{
	tU8 flags = 0;

	if (!hmi_serial_read_8(serial_port, &flags)) {
		hmi_serial_send_ack(serial_port, ETAL_SCAN_STOP, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	bool play_last_bad  = (flags & 0x02) != 0;

	ETAL_STATUS rc = etaltml_scan_stop(target, play_last_bad);

	HMI_CMD_TRACE("etaltml_scan_stop");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_SCAN_STOP, target, rc, NULL, 0);
}

static void hmi_serial_handle_learn_start(struct hmi_serial_port *serial_port, int target)
{
	EtalFrequencyBand bandIndex;
	tU32 step, nbOfFreq;
	etalLearnReportingModeStatusTy mode;

	if (!hmi_serial_read_32(serial_port, &bandIndex)) {
		hmi_serial_send_ack(serial_port, ETAL_LEARN_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &step)) {
		hmi_serial_send_ack(serial_port, ETAL_LEARN_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &nbOfFreq)) {
		hmi_serial_send_ack(serial_port, ETAL_LEARN_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &mode)) {
		hmi_serial_send_ack(serial_port, ETAL_LEARN_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	ETAL_STATUS rc = etaltml_learn_start(target, bandIndex, step, nbOfFreq, mode, freqList);

	HMI_CMD_TRACE("etaltml_learn_start");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_LEARN_START, target, rc, NULL, 0);
}

static void hmi_serial_handle_learn_stop(struct hmi_serial_port *serial_port, int target)
{
	tU32 terminationMode;

	if (!hmi_serial_read_32(serial_port, &terminationMode)) {
		hmi_serial_send_ack(serial_port, ETAL_LEARN_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	ETAL_STATUS rc = etaltml_learn_stop(target, terminationMode);

	HMI_CMD_TRACE("etaltml_learn_stop");
	HMI_PAR_TRACE("receiver = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_LEARN_STOP, target, rc, NULL, 0);
}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

static void hmi_serial_handle_get_ensemble_list(struct hmi_serial_port *serial_port, int target)
{
	if (target != 0) {
		hmi_serial_send_ack(serial_port, ETAL_GET_ENSEMBLE_LIST, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	EtalEnsembleList ensemble_list;

	ETAL_STATUS rc = etal_get_ensemble_list(&ensemble_list);

	HMI_CMD_TRACE("etal_get_ensemble_list");

	char buffer[292];
	int pos = 0;
	int i;

	hmi_write_32(buffer, &pos, ensemble_list.m_ensembleCount);

	for (i = 0; i < ensemble_list.m_ensembleCount; i++) {
		hmi_write_8(buffer, &pos, ensemble_list.m_ensemble[i].m_ECC);
		hmi_write_32(buffer, &pos, ensemble_list.m_ensemble[i].m_ensembleId);
		hmi_write_32(buffer, &pos, ensemble_list.m_ensemble[i].m_frequency);
	}

	assert(sizeof(buffer) >= pos);

	hmi_serial_send_ack(serial_port, ETAL_GET_ENSEMBLE_LIST, target, rc, buffer, pos);
}

static void hmi_serial_handle_get_ensemble_data(struct hmi_serial_port *serial_port, int target)
{
	tU32 eid;
	tU8 charset;
	tChar label[ETAL_DEF_MAX_LABEL_LEN];
	tU16 bitmap;

	if (target != 0) {
		hmi_serial_send_ack(serial_port, ETAL_GET_ENSEMBLE_DATA, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &eid)) {
		hmi_serial_send_ack(serial_port, ETAL_LEARN_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	ETAL_STATUS rc = etal_get_ensemble_data(eid, &charset, label, &bitmap);

	HMI_CMD_TRACE("etal_get_ensemble_data");

	char buffer[20];
	int pos = 0;
	int i;

	hmi_write_8(buffer, &pos, charset);

	for (i = 0; i < ETAL_DEF_MAX_LABEL_LEN; i++)
		hmi_write_8(buffer, &pos, label[i]);

	hmi_write_16(buffer, &pos, bitmap);

	assert(sizeof(buffer) == pos);

	hmi_serial_send_ack(serial_port, ETAL_GET_ENSEMBLE_DATA, target, rc, buffer, pos);
}

#endif // ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

static void hmi_serial_handle_get_service_list(struct hmi_serial_port *serial_port, int target)
{
	tU32 eid;
	tU8 flags;

	if (!hmi_serial_read_32(serial_port, &eid)) {
		hmi_serial_send_ack(serial_port, ETAL_GET_SERVICE_LIST, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_8(serial_port, &flags)) {
		hmi_serial_send_ack(serial_port, ETAL_GET_SERVICE_LIST, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	bool get_audio_services = (flags & 0x01) != 0;
	bool get_data_services  = (flags & 0x02) != 0;

	EtalServiceList service_List;

	ETAL_STATUS rc = etal_get_service_list(target, eid, get_audio_services, get_data_services, &service_List);

	HMI_CMD_TRACE("etal_get_service_list");
	HMI_PAR_TRACE("receiver = %d\n", target);

	char buffer[132];
	int pos = 0;
	int i;

	hmi_write_32(buffer, &pos, service_List.m_serviceCount);

	for (i = 0; i < ETAL_DEF_MAX_SERVICE_PER_ENSEMBLE; i++)
		hmi_write_32(buffer, &pos, service_List.m_service[i]);

	assert(sizeof(buffer) == pos);

	hmi_serial_send_ack(serial_port, ETAL_GET_SERVICE_LIST, target, rc, buffer, pos);
}

#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

static void hmi_serial_handle_get_specific_service_data_dab(struct hmi_serial_port *serial_port, int target)
{
	tU32 eid, sid;

	if (target != 0) {
		hmi_serial_send_ack(serial_port, ETAL_GET_SPECIFIC_SERVICE_DATA_DAB, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &eid)) {
		hmi_serial_send_ack(serial_port, ETAL_GET_SPECIFIC_SERVICE_DATA_DAB, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &sid)) {
		hmi_serial_send_ack(serial_port, ETAL_GET_SPECIFIC_SERVICE_DATA_DAB, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	EtalServiceInfo serv_info;
	EtalServiceComponentList sclist;

	ETAL_STATUS rc = etal_get_specific_service_data_DAB(eid, sid, &serv_info, &sclist, NULL);

	HMI_CMD_TRACE("etal_get_specific_service_data_DAB");

	char buffer[799];
	int pos = 0;
	int i, j;

	hmi_write_16(buffer, &pos, serv_info.m_serviceBitrate);
	hmi_write_8(buffer, &pos, serv_info.m_subchId);
	hmi_write_16(buffer, &pos, serv_info.m_packetAddress);
	hmi_write_8(buffer, &pos, serv_info.m_serviceLanguage);
	hmi_write_8(buffer, &pos, serv_info.m_componentType);
	hmi_write_8(buffer, &pos, serv_info.m_streamType);
	hmi_write_8(buffer, &pos, serv_info.m_scCount);
	hmi_write_8(buffer, &pos, serv_info.m_serviceLabelCharset);

	for (i = 0; i < ETAL_DEF_MAX_LABEL_LEN; i++)
		hmi_write_8(buffer, &pos, serv_info.m_serviceLabel[i]);

	hmi_write_16(buffer, &pos, serv_info.m_serviceLabelCharflag);

	hmi_write_16(buffer, &pos, sclist.m_scCount);

	for (i = 0; i < sclist.m_scCount; i++) {
		hmi_write_16(buffer, &pos, sclist.m_scInfo[i].m_scIndex);
		hmi_write_8(buffer, &pos, sclist.m_scInfo[i].m_dataServiceType);
		hmi_write_8(buffer, &pos, sclist.m_scInfo[i].m_scType);
		hmi_write_8(buffer, &pos, sclist.m_scInfo[i].m_scLabelCharset);

		for (j = 0; j < ETAL_DEF_MAX_LABEL_LEN; j++)
			hmi_write_8(buffer, &pos, sclist.m_scInfo[i].m_scLabel[j]);

		hmi_write_16(buffer, &pos, sclist.m_scInfo[i].m_scLabelCharflag);
	}

	assert(sizeof(buffer) >= pos);

	hmi_serial_send_ack(serial_port, ETAL_GET_SPECIFIC_SERVICE_DATA_DAB, target, rc, buffer, pos);
}

#if 0
static void hmi_serial_handle_get_pad_dls(struct hmi_serial_port *serial_port, int target)
{
	etalPADDataTy data;

	ETAL_STATUS rc = etal_get_PAD_DLS(target, &data);

	HMI_CMD_TRACE("etal_get_PAD_DLS");
	HMI_PAR_TRACE("datapath = %d\n", target);

	char buffer[132];
	int pos = 0;
	int i;

	hmi_write_8(buffer, &pos, data.m_dataPath);
	hmi_write_8(buffer, &pos, data.m_PADType);
	hmi_write_8(buffer, &pos, data.m_charset);

	for (i = 0; i < ETAL_DEF_MAX_PAD_STRING; i++)
		hmi_write_8(buffer, &pos, data.m_PAD[i]);

	assert(sizeof(buffer) == pos);

	hmi_serial_send_ack(serial_port, ETAL_GET_PAD_DLS, target, rc, buffer, pos);
}

static void hmi_serial_handle_event_pad_dls_start(struct hmi_serial_port *serial_port, int target)
{
	ETAL_STATUS rc = etal_event_PAD_DLS_start(target);

	HMI_CMD_TRACE("etal_event_PAD_DLS_start");
	HMI_PAR_TRACE("datapath = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_EVENT_PAD_DLS_START, target, rc, NULL, 0);
}

static void hmi_serial_handle_event_pad_dls_stop(struct hmi_serial_port *serial_port, int target)
{
	ETAL_STATUS rc = etal_event_PAD_DLS_stop(target);

	HMI_CMD_TRACE("etal_event_PAD_DLS_stop");
	HMI_PAR_TRACE("datapath = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_EVENT_PAD_DLS_STOP, target, rc, NULL, 0);
}

static void hmi_serial_handle_start_tpeg_raw(struct hmi_serial_port *serial_port, int target)
{
	ETAL_STATUS rc = etal_start_TPEG_raw(target);

	HMI_CMD_TRACE("etal_start_TPEG_raw");
	HMI_PAR_TRACE("datapath = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_START_TPEG_RAW, target, rc, NULL, 0);
}

static void hmi_serial_handle_stop_tpeg_raw(struct hmi_serial_port *serial_port, int target)
{
	ETAL_STATUS rc = etal_stop_TPEG_raw(target);

	HMI_CMD_TRACE("etal_stop_TPEG_raw");
	HMI_PAR_TRACE("datapath = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_STOP_TPEG_RAW, target, rc, NULL, 0);
}
#endif

#endif // ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

#if defined (CONFIG_ETAL_HAVE_ALL_API) && defined (CONFIG_ETAL_HAVE_SEAMLESS)

static void hmi_serial_handle_seamless_estimation_start(struct hmi_serial_port *serial_port, int target)
{
	tU8 receiver_sas;

	if (!hmi_serial_read_8(serial_port, &receiver_sas)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_ESTIMATION_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	etalSeamlessEstimationConfigTy config;

	if (!hmi_serial_read_8(serial_port, &config.mode)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_ESTIMATION_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, (tU32 *)&config.startPosition)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_ESTIMATION_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, (tU32 *)&config.stopPosition)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_ESTIMATION_START, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	ETAL_STATUS rc = etal_seamless_estimation_start(target, receiver_sas, &config);

	HMI_CMD_TRACE("etal_seamless_estimation_start");
	HMI_PAR_TRACE("receiver FAS = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_ESTIMATION_START, target, rc, NULL, 0);
}

static void hmi_serial_handle_seamless_estimation_stop(struct hmi_serial_port *serial_port, int target)
{
	tU8 receiver_sas;

	if (!hmi_serial_read_8(serial_port, &receiver_sas)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_ESTIMATION_STOP, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	ETAL_STATUS rc = etal_seamless_estimation_stop(target, receiver_sas);

	HMI_CMD_TRACE("etal_seamless_estimation_stop");
	HMI_PAR_TRACE("receiver FAS = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_ESTIMATION_STOP, target, rc, NULL, 0);
}

static void hmi_serial_handle_seamless_switching(struct hmi_serial_port *serial_port, int target)
{
	tU8 receiver_sas;

	if (!hmi_serial_read_8(serial_port, &receiver_sas)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_SWITCHING, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	etalSeamlessSwitchingConfigTy config;

	if (!hmi_serial_read_8(serial_port, &config.systemToSwitch)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_SWITCHING, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_8(serial_port, &config.providerType)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_SWITCHING, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, (tU32 *)&config.absoluteDelayEstimate)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_SWITCHING, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, (tU32 *)&config.delayEstimate)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_SWITCHING, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &config.timestampFAS)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_SWITCHING, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &config.timestampSAS)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_SWITCHING, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &config.averageRMS2FAS)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_SWITCHING, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	if (!hmi_serial_read_32(serial_port, &config.averageRMS2SAS)) {
		hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_SWITCHING, target, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	ETAL_STATUS rc = etal_seamless_switching(target, receiver_sas, &config);

	HMI_CMD_TRACE("etal_seamless_switching");
	HMI_PAR_TRACE("receiver FAS = %d\n", target);

	hmi_serial_send_ack(serial_port, ETAL_SEAMLESS_SWITCHING, target, rc, NULL, 0);
}

#endif // if defined (CONFIG_ETAL_HAVE_ALL_API) && defined (CONFIG_ETAL_HAVE_SEAMLESS)

static void hmi_serial_port_read(struct hmi_serial_port *serial_port)
{
	char buffer[2];
	bool found = false;
	bool eof = false;
	int num;

	do {
		num = read(serial_port->fd, buffer, 1);
		if (num <= 0) {
			eof = true;
		} else {
			if (buffer[0] == 0xFE)
				found = true;
		}
	} while (!found && !eof);

	if (!found)
		return;

	num = read(serial_port->fd, buffer, 2);
	if (num < 2) {
		hmi_serial_send_ack(serial_port, ETAL_COMMAND_INVALID, ETAL_INVALID_HANDLE, ETAL_RET_PARAMETER_ERR, NULL, 0);
		return;
	}

	int command_id = buffer[0];
	int target_id  = buffer[1];

	switch (command_id) {
	/*
	 * System initialization and configuration
	 */
	case ETAL_INITIALIZE:
		hmi_serial_send_ack(serial_port, command_id, target_id, ETAL_RET_NOT_IMPLEMENTED, NULL, 0);
		break;
	case ETAL_GET_CAPABILITIES:
		hmi_serial_handle_get_capabilities(serial_port, target_id);
		break;
	case ETAL_CONFIG_RECEIVER:
		hmi_serial_handle_config_receiver(serial_port, target_id);
		break;
	case ETAL_DESTROY_RECEIVER:
		hmi_serial_handle_destroy_receiver(serial_port, target_id);
		break;
	case ETAL_CONFIG_DATAPATH:
		hmi_serial_handle_config_datapath(serial_port, target_id);
		break;
	case ETAL_DESTROY_DATAPATH:
		hmi_serial_handle_destroy_datapath(serial_port, target_id);
		break;
	case ETAL_DEINITIALIZE:
		hmi_serial_send_ack(serial_port, command_id, target_id, ETAL_RET_NOT_IMPLEMENTED, NULL, 0);
		break;
    case ETAL_CONFIG_AUDIO_PATH:
        hmi_serial_handle_config_audio_path(serial_port, target_id);
        break;
	case ETAL_AUDIO_SELECT:
		hmi_serial_handle_audio_select(serial_port, target_id);
		break;
	case ETAL_CHANGE_BAND_RECEIVER:
		hmi_serial_handle_change_band_receiver(serial_port, target_id);
		break;

	/*
	 * Tune/Seek/Quality
	 */
	case ETAL_TUNE_RECEIVER:
		hmi_serial_handle_tune_receiver(serial_port, target_id);
		break;
	case ETAL_TUNE_RECEIVER_SYNC:
		hmi_serial_send_ack(serial_port, command_id, target_id, ETAL_RET_NOT_IMPLEMENTED, NULL, 0);
		break;
	case ETAL_SEEK_START:
		hmi_serial_handle_seek_start(serial_port, target_id);
		break;
	case ETAL_SEEK_STOP:
		hmi_serial_handle_seek_stop(serial_port, target_id);
		break;
	case ETAL_GET_RECEPTION_QUALITY:
		hmi_serial_handle_get_reception_quality(serial_port, target_id);
		break;
	case ETAL_GET_RECEPTION_QUALITY_AD:
		hmi_serial_send_ack(serial_port, command_id, target_id, ETAL_RET_NOT_IMPLEMENTED, NULL, 0);
		break;
	case ETAL_CONFIG_RECEPTION_QUALITY_MONITOR:
		hmi_serial_send_ack(serial_port, command_id, target_id, ETAL_RET_NOT_IMPLEMENTED, NULL, 0);
		break;
	case ETAL_DESTROY_RECEPTION_QUALITY_MONITOR:
		hmi_serial_send_ack(serial_port, command_id, target_id, ETAL_RET_NOT_IMPLEMENTED, NULL, 0);
		break;
	case ETAL_GET_RECEIVER_FREQUENCY:
		hmi_serial_handle_get_receiver_frequency(serial_port, target_id);
		break;

	/*
	 * Radio text
	 */
	case ETAL_GET_RADIOTEXT:
		hmi_serial_handle_get_radio_text(serial_port, target_id);
		break;
	case ETAL_START_RADIOTEXT:
		hmi_serial_handle_start_radio_text(serial_port, target_id);
		break;
	case ETAL_STOP_RADIOTEXT:
		hmi_serial_handle_stop_radio_text(serial_port, target_id);
		break;
	case ETAL_GET_DECODED_RDS:
		hmi_serial_handle_get_decoded_rds(serial_port, target_id);
		break;
	case ETAL_START_RDS:
		hmi_serial_handle_start_decoded_rds(serial_port, target_id);
		break;
	case ETAL_STOP_RDS:
		hmi_serial_handle_stop_decoded_rds(serial_port, target_id);
		break;

	/*
	 * Advanced Tuning
	 */
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	case ETAL_GET_CURRENT_ENSEMBLE:
		hmi_serial_handle_get_current_ensemble(serial_port, target_id);
		break;
#endif // ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	case ETAL_SERVICE_SELECT_AUDIO:
		hmi_serial_handle_service_select_audio(serial_port, target_id);
		break;
	case ETAL_SCAN_START:
		hmi_serial_handle_scan_start(serial_port, target_id);
		break;
	case ETAL_SCAN_STOP:
		hmi_serial_handle_scan_stop(serial_port, target_id);
		break;
	case ETAL_LEARN_START:
		hmi_serial_handle_learn_start(serial_port, target_id);
		break;
	case ETAL_LEARN_STOP:
		hmi_serial_handle_learn_stop(serial_port, target_id);
		break;

	/*
	 * System Data
	 */
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	case ETAL_GET_ENSEMBLE_LIST:
		hmi_serial_handle_get_ensemble_list(serial_port, target_id);
		break;
	case ETAL_GET_ENSEMBLE_DATA:
		hmi_serial_handle_get_ensemble_data(serial_port, target_id);
		break;
#endif // ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	case ETAL_GET_SERVICE_LIST:
		hmi_serial_handle_get_service_list(serial_port, target_id);
		break;
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
	case ETAL_GET_SPECIFIC_SERVICE_DATA_DAB:
		hmi_serial_handle_get_specific_service_data_dab(serial_port, target_id);
		break;
#endif // ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

	/*
	 * Digital Radio services
	 */
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
#if 0
	case ETAL_GET_PAD_DLS:
		hmi_serial_handle_get_pad_dls(serial_port, target_id);
		break;
	case ETAL_EVENT_PAD_DLS_START:
		hmi_serial_handle_event_pad_dls_start(serial_port, target_id);
		break;
	case ETAL_EVENT_PAD_DLS_STOP:
		hmi_serial_handle_event_pad_dls_stop(serial_port, target_id);
		break;
#endif
#endif // ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

	/*
	 * Enhanced services
	 */
#ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR
#if 0
	case ETAL_START_TPEG_RAW:
		hmi_serial_handle_start_tpeg_raw(serial_port, target_id);
		break;
	case ETAL_STOP_TPEG_RAW:
		hmi_serial_handle_stop_tpeg_raw(serial_port, target_id);
		break;
#endif
#endif // ifdef CONFIG_ETAL_SUPPORT_DCOP_MDR

	/*
	 * Seamless estimation and seamless switching
	 */
#if defined (CONFIG_ETAL_HAVE_ALL_API) && defined (CONFIG_ETAL_HAVE_SEAMLESS)
	case ETAL_SEAMLESS_ESTIMATION_START:
		hmi_serial_handle_seamless_estimation_start(serial_port, target_id);
		break;
	case ETAL_SEAMLESS_ESTIMATION_STOP:
		hmi_serial_handle_seamless_estimation_stop(serial_port, target_id);
		break;
	case ETAL_SEAMLESS_SWITCHING:
		hmi_serial_handle_seamless_switching(serial_port, target_id);
		break;
#endif // if defined (CONFIG_ETAL_HAVE_ALL_API) && defined (CONFIG_ETAL_HAVE_SEAMLESS)
	case ETAL_READ_PARAMETER:
		hmi_serial_send_ack(serial_port, command_id, target_id, ETAL_RET_NOT_IMPLEMENTED, NULL, 0);
		break;
	case ETAL_WRITE_PARAMETER:
		hmi_serial_send_ack(serial_port, command_id, target_id, ETAL_RET_NOT_IMPLEMENTED, NULL, 0);
		break;
	default:
		hmi_serial_send_ack(serial_port, ETAL_COMMAND_INVALID, target_id, ETAL_RET_PARAMETER_ERR, NULL, 0);
		break;
	};
}

/******************************************************************************
 *
 * ETAL interface
 *
 *****************************************************************************/

static void hmi_serial_send_tune_response(void *pvContext)
{
	EtalTuneStatus *status = (EtalTuneStatus *)pvContext;

	char buffer[12];
	int pos = 0;

	hmi_write_8(buffer, &pos, 0xFD);
	hmi_write_8(buffer, &pos, ETAL_TUNE_RECEIVER);
	hmi_write_8(buffer, &pos, status->m_receiverHandle);

	hmi_write_32(buffer, &pos, status->m_stopFrequency);
	hmi_write_32(buffer, &pos, status->m_sync);

	hmi_write_8(buffer, &pos, 0xDF);

	assert(sizeof(buffer) == pos);

	write(hmi_global_context.serial_port->fd, buffer, pos);
}

static void hmi_serial_send_learn_end_response(void *pvContext)
{
	EtalLearnStatus *status = (EtalLearnStatus *)pvContext;

	char buffer[8];
	int pos = 0;

	hmi_write_8(buffer, &pos, 0xFD);
	hmi_write_8(buffer, &pos, ETAL_INFO_LEARN);
	hmi_write_8(buffer, &pos, status->m_receiverHandle);

	hmi_write_32(buffer, &pos, status->m_services);

	hmi_write_8(buffer, &pos, 0xDF);

	assert(sizeof(buffer) == pos);

	write(hmi_global_context.serial_port->fd, buffer, pos);
}

static void hmi_serial_send_scan_response(void *pvContext)
{
	EtalTuneStatus *status = (EtalTuneStatus *)pvContext;

	char buffer[12];
	int pos = 0;

	hmi_write_8(buffer, &pos, 0xFD);
	hmi_write_8(buffer, &pos, ETAL_INFO_SCAN);
	hmi_write_8(buffer, &pos, status->m_receiverHandle);

	if (status)	{
		hmi_write_32(buffer, &pos, status->m_stopFrequency);
		hmi_write_32(buffer, &pos, status->m_sync);
	}

	hmi_write_8(buffer, &pos, 0xDF);

	assert(sizeof(buffer) >= pos);

	write(hmi_global_context.serial_port->fd, buffer, pos);
}

static void hmi_serial_send_com_failed_response(void *pvContext)
{
	char buffer[4];
	int pos = 0;

	hmi_write_8(buffer, &pos, 0xFD);
	hmi_write_8(buffer, &pos, ETAL_ERROR_COMM_FAILED);
	hmi_write_8(buffer, &pos, 0x00);	// HACK: the receiver ID is not returned

	hmi_write_8(buffer, &pos, 0xDF);

	assert(sizeof(buffer) == pos);

	write(hmi_global_context.serial_port->fd, buffer, pos);
}

#if 0
static void hmi_serial_send_pad_response(void *pvContext)
{
	etalPADDataTy *status = (etalPADDataTy *)pvContext;

	char buffer[ETAL_DEF_MAX_PAD_STRING + 6];
	int pos = 0;

	hmi_write_8(buffer, &pos, 0xFD);
	hmi_write_8(buffer, &pos, ETAL_INFO_PAD);
	hmi_write_8(buffer, &pos, status->m_dataPath);

	hmi_write_8(buffer, &pos, status->m_PADType);
	hmi_write_8(buffer, &pos, status->m_charset);

	int len = strlen(status->m_PAD);
	strncpy(buffer + pos, status->m_PAD, ETAL_DEF_MAX_PAD_STRING);

	hmi_write_8(buffer, &pos, 0xDF);

	assert(sizeof(buffer) >= (pos + ETAL_DEF_MAX_PAD_STRING));

	write(hmi_global_context.serial_port->fd, buffer, pos + len);
}
#endif

static void hmi_serial_send_seamless_estimation_end_response(void *pvContext)
{
	EtalSeamlessEstimationStatus *status = (EtalSeamlessEstimationStatus *)pvContext;

	char buffer[34];
	int pos = 0;

	hmi_write_8(buffer, &pos, 0xFD);
	hmi_write_8(buffer, &pos, ETAL_INFO_SEAMLESS_ESTIMATION_END);
	hmi_write_8(buffer, &pos, status->m_receiverHandle);

	hmi_write_8(buffer, &pos, status->m_status);
	hmi_write_8(buffer, &pos, status->m_providerType);
	hmi_write_32(buffer, &pos, status->m_absoluteDelayEstimate);
	hmi_write_32(buffer, &pos, status->m_delayEstimate);
	hmi_write_32(buffer, &pos, status->m_timestamp_FAS);
	hmi_write_32(buffer, &pos, status->m_timestamp_SAS);
	hmi_write_32(buffer, &pos, status->m_RMS2_FAS);
	hmi_write_32(buffer, &pos, status->m_RMS2_SAS);
	hmi_write_32(buffer, &pos, status->m_confidenceLevel);

	hmi_write_8(buffer, &pos, 0xDF);

	assert(sizeof(buffer) == pos);

	write(hmi_global_context.serial_port->fd, buffer, pos);
}

static void hmi_serial_send_seamless_switching_end_response(void *pvContext)
{
	EtalSeamlessSwitchingStatus *status = (EtalSeamlessSwitchingStatus *)pvContext;

	char buffer[13];
	int pos = 0;

	hmi_write_8(buffer, &pos, 0xFD);
	hmi_write_8(buffer, &pos, ETAL_INFO_SEAMLESS_SWITCHING_END);
	hmi_write_8(buffer, &pos, status->m_receiverHandle);

	hmi_write_8(buffer, &pos, status->m_status);
	hmi_write_32(buffer, &pos, status->m_absoluteDelayEstimate);

	hmi_write_8(buffer, &pos, 0xDF);

	assert(sizeof(buffer) == pos);

	write(hmi_global_context.serial_port->fd, buffer, pos);
}

static void hmi_etal_notification_handler(void * context, ETAL_EVENTS event, void* status)
{
	switch (event) {
	case ETAL_INFO_TUNE:
		hmi_serial_send_tune_response(status);
		break;
	case ETAL_INFO_LEARN:
		hmi_serial_send_learn_end_response(status);
		break;
	case ETAL_INFO_SCAN:
		hmi_serial_send_scan_response(status);
		break;
	case ETAL_ERROR_COMM_FAILED:
		hmi_serial_send_com_failed_response(status);
		break;
#if 0
	case ETAL_INFO_PAD:
		hmi_serial_send_pad_response(status);
		break;
#endif
	case ETAL_INFO_SEAMLESS_ESTIMATION_END:
		hmi_serial_send_seamless_estimation_end_response(status);
		break;
	case ETAL_INFO_SEAMLESS_SWITCHING_END:
		hmi_serial_send_seamless_switching_end_response(status);
		break;
	default:
		fprintf(stderr, "Unexpected event from ETAL notification handler (%d)", event);
		break;
	}
}

static struct hmi_etal_status *hmi_etal_initialize()
{
	struct hmi_etal_status *etal_status = calloc(1, sizeof(struct hmi_etal_status));
	if (!etal_status)
		return NULL;

	EtalHardwareAttr init_params;
	memset(&init_params, 0, sizeof(EtalHardwareAttr));
	init_params.m_cbNotify = hmi_etal_notification_handler;
	init_params.m_context = &hmi_global_context;

	ETAL_STATUS rc = etal_initialize(&init_params);
	if (rc != ETAL_RET_SUCCESS)
		fprintf(stderr, "etal_initialize() failed (%d)\n", rc);

	int i;
	for (i = 0; i < HMI_MAX_ETAL_RECEIVERS; i++) {
		etal_status->recv_handle[i] = ETAL_INVALID_HANDLE;
		memset(&etal_status->recv_attr[i], 0, sizeof(EtalReceiverAttr));
	}

	for (i = 0; i < HMI_MAX_ETAL_DATAPATHS; i++) {
		etal_status->dpth_handle[i] = ETAL_INVALID_HANDLE;
		memset(&etal_status->dpth_attr[i], 0, sizeof(EtalDataPathAttr));
	}

	return etal_status;
}

static void hmi_etal_deinitialize(struct hmi_etal_status *etal_status)
{
	int i;
	for (i = 0; i < HMI_MAX_ETAL_RECEIVERS; i++) {
		if (etal_status->recv_handle[i] != ETAL_INVALID_HANDLE) {
			etal_destroy_receiver(&etal_status->recv_handle[i]);
		}
	}

	for (i = 0; i < HMI_MAX_ETAL_DATAPATHS; i++) {
		if (etal_status->dpth_handle[i] != ETAL_INVALID_HANDLE) {
			etal_destroy_datapath(&etal_status->dpth_handle[i]);
		}
	}

	etal_deinitialize();

	free(etal_status);
}

/******************************************************************************
 *
 * Signal handler
 *
 *****************************************************************************/

static bool hmi_running = true;

static void hmi_signal_handler(int sig)
{
	switch (sig) {
	case SIGINT:
		hmi_running = false;
		break;
	default:
		/* do nothing */
		break;
	}
}

/******************************************************************************
 *
 * Help
 *
 *****************************************************************************/

static void hmi_help(char *program_name)
{
	printf("%s [OPTIONS...]\n\n"
		   "Radio HMI application. (C) STMicroelectronics\n\n"
		   "  -h --help                 Print this message\n"
		   "     --log[=filename]       Log operations to filename\n"
		   "\n\n",
		   program_name);
}

/******************************************************************************
 *
 * Application
 *
 *****************************************************************************/

int main(int argc, char* argv[])
{
	/*
	 * install signal handler to exit gracefully
	 */
	signal(SIGINT, hmi_signal_handler);

	/*
	 * parse command line options
	 */
	static const struct option long_options[] = {
			{ "log",  optional_argument, NULL, 'l' },
			{ "help", no_argument,       NULL, 'h' },
			{ 0, 0, 0, 0 }
	};

	int c;

	while ((c = getopt_long(argc, argv, "l:h", long_options, NULL)) >= 0) {
		switch (c) {
		case 'l':
			hmi_global_context.log_file = fopen(optarg ? optarg : "radio_hmi_log.txt", "a");
			break;
		case 'h':
			hmi_help(argv[0]);
			return EXIT_SUCCESS;
		default:
			printf("Unknown option: %c\n", c);
			return EXIT_FAILURE;
		}
	}

	struct hmi_display_device *display = hmi_display_device_open("/dev/fb1");
	if (!display)
		return EXIT_FAILURE;

	struct hmi_input_device *keypad = hmi_input_device_open("/dev/input/event0");
	if (!keypad)
		return EXIT_FAILURE;

	struct hmi_input_device *rotary = hmi_input_device_open("/dev/input/event2");
	if (!rotary)
		return EXIT_FAILURE;

	struct hmi_serial_port *serial = hmi_serial_port_open("/dev/ttyUSB0");
	if (!serial)
		return EXIT_FAILURE;

	struct hmi_etal_status *etal_status = hmi_etal_initialize();
	if (!etal_status)
		return EXIT_FAILURE;

	/*
	 * UGLY HACK: we need a global context here to store some info because
	 *            hmi_etal_initialize() does not allow us to register a pointer to
	 *            a local context whose value is provided back by
	 *            hmi_etal_notification_handler()
	 */
	hmi_global_context.serial_port = serial;
	hmi_global_context.etal_status = etal_status;

	/*
	 * define the user input events to wait for
	 */
	struct pollfd input_pollfd[3];
	input_pollfd[0].fd      = keypad->fd;
	input_pollfd[0].events  = POLLIN;
	input_pollfd[0].revents = 0;
	input_pollfd[1].fd      = rotary->fd;
	input_pollfd[1].events  = POLLIN;
	input_pollfd[1].revents = 0;
	input_pollfd[2].fd      = serial->fd;
	input_pollfd[2].events  = POLLIN;
	input_pollfd[2].revents = 0;

	/*
	 * main application loop: wait for user input, execute, display
	 */
	do {
		int rc = LIBEVDEV_READ_STATUS_SUCCESS;
		struct input_event ev;

		hmi_display_device_render(display, tux);

		/*
		 * wait for keypad and rotary events
		 */
		int events = poll(input_pollfd, 3, -1);

		if ((events < 0) && hmi_running) {
			fprintf(stderr, "Error on poll(). It should never happen!!!");
			exit(EXIT_FAILURE);
		}

		if (input_pollfd[0].revents && POLLIN) {
			do {
				rc = libevdev_next_event(keypad->dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
				if (rc == 0)
					printf("Keypad event: %s %s %d\n",
							libevdev_event_type_get_name(ev.type),
							libevdev_event_code_get_name(ev.type, ev.code),
							ev.value);
			} while (rc == LIBEVDEV_READ_STATUS_SYNC || rc == LIBEVDEV_READ_STATUS_SUCCESS);
		}

		if (input_pollfd[1].revents && POLLIN) {
			do {
				rc = libevdev_next_event(rotary->dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
				if (rc == 0)
					printf("Rotary event: %s %s %d\n",
							libevdev_event_type_get_name(ev.type),
							libevdev_event_code_get_name(ev.type, ev.code),
							ev.value);
			} while (rc == LIBEVDEV_READ_STATUS_SYNC || rc == LIBEVDEV_READ_STATUS_SUCCESS);
		}

		if (input_pollfd[2].revents && POLLIN) {
			hmi_serial_port_read(serial);
		}
	} while (hmi_running);

	hmi_input_device_close(keypad);
	hmi_input_device_close(rotary);
	hmi_display_device_close(display);

	hmi_serial_port_close(serial);

	hmi_etal_deinitialize(etal_status);

	if (hmi_global_context.log_file)
		fclose(hmi_global_context.log_file);

	return EXIT_SUCCESS;
}
