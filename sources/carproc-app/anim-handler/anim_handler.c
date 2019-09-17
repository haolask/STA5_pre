#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <poll.h>

#define MAX_NAME_LENGTH 15
#define RPMSG_MM_DEVICE_NAME "/dev/rpmsg_mm0"
#define RPMSG_MM_IOC_MAGIC      'L'

#define RPMSG_MM_IOC_GET_RG_STATUS      _IOR(RPMSG_MM_IOC_MAGIC, 1, int)
#define RPMSG_MM_IOC_STOP_ANIM	  	_IOW(RPMSG_MM_IOC_MAGIC, 2, int)
#define RPMSG_MM_IOC_STOP_RVC		_IOW(RPMSG_MM_IOC_MAGIC, 3, int)
#define RPMSG_MM_IOC_START_RVC		_IOW(RPMSG_MM_IOC_MAGIC, 4, int)
#define RPMSG_MM_IOC_RVC_DEBUG_MODE	_IOW(RPMSG_MM_IOC_MAGIC, 5, int)

char *info2str[] = {
	"UNUSED",
	"REGISTRATION_REQ",
	"REGISTRATION_ACK",
	"UNREGISTRATION_REQ",
	"UNREGISTRATION_ACK",
	"SHARED_RES_STATUS_REQ",
	"SHARED_RES_STATUS_ACK",
	"SHARED_RES_LOCKED",
	"SHARED_RES_LOCKED_ACK",
	"SHARED_RES_UNLOCKED",
	"COMM_UNAVAILABLE",
	"COMM_AVAILABLE",
	"USER_SERVICE",
	/* RVC dedicated messages */
	"REARGEAR_STATUS_REQ",
	"REARGEAR_STATUS_ACK",
	"PRIVATE_MESSAGE"
};

enum user_event {
	STOP_ANIM,
	START_RVC,
	STOP_RVC,
	RVC_NORMAL,
	RVC_MANUAL_TOGGLING,
	RVC_AUTO_TOGGLING,
};

/**
 * @enum rpmsg_mm_rvc_debug_mode
 * @brief sets the RVC debug mode
 */
enum rpmsg_mm_rvc_debug_mode {
	RVC_DEBUG_MODE_NORMAL, /*!< rear gear controlled by GPIO input */
	RVC_DEBUG_MODE_MANUAL_TOGGLING, /*!< rear gear toggling on demand */
	RVC_DEBUG_MODE_AUTO_TOGGLING, /*!< rear gear toggling periodically */
};

void send_user_event(int event)
{
	int fd, retval;
	int ret = 0;
	enum rpmsg_mm_rvc_debug_mode mode;

	fd = open(RPMSG_MM_DEVICE_NAME, O_RDWR);
	if (fd < 0) {
	printf ("Can't open device file: %s\n",
			RPMSG_MM_DEVICE_NAME);
	exit(-1);
	}
	switch (event) {
	case STOP_ANIM:
		ret = ioctl(fd, RPMSG_MM_IOC_STOP_ANIM, &retval);
		break;
	case START_RVC:
		ret = ioctl(fd, RPMSG_MM_IOC_START_RVC, &retval);
		break;
	case STOP_RVC:
		ret = ioctl(fd, RPMSG_MM_IOC_STOP_RVC, &retval);
		break;
	case RVC_NORMAL:
		mode = RVC_DEBUG_MODE_NORMAL;
		ret = ioctl(fd, RPMSG_MM_IOC_RVC_DEBUG_MODE, &mode);
		break;
	case RVC_MANUAL_TOGGLING:
		mode = RVC_DEBUG_MODE_MANUAL_TOGGLING;
		ret = ioctl(fd, RPMSG_MM_IOC_RVC_DEBUG_MODE, &mode);
		break;
	case RVC_AUTO_TOGGLING:
		mode = RVC_DEBUG_MODE_AUTO_TOGGLING;
		ret = ioctl(fd, RPMSG_MM_IOC_RVC_DEBUG_MODE, &mode);
		break;
	default:;
	}

	if (ret < 0)
		printf("Command failed ret=%d\n", ret);

	close(fd);
}

void get_rear_gear_status(void)
{
	int fd, retval;

	fd = open(RPMSG_MM_DEVICE_NAME, O_RDWR);
	if (fd < 0) {
	printf ("Can't open device file: %s\n",
			RPMSG_MM_DEVICE_NAME);
	exit(-1);
	}
	if (ioctl(fd, RPMSG_MM_IOC_GET_RG_STATUS, &retval) >= 0) {
	printf(" RearGear status = %d\n", retval);
	}	
	close(fd);
}

void usage(void) {
	printf("Usage:\n");
	printf("anim_handler -[sednmag]:\n");
	printf("* -s : 'S'top the animation on remote side\n");
	printf("* -e : 'E'ngaged the rear view camera use-case\n");
	printf("* -d : 'D'isengaged the rear view camera use-case\n");
	printf("* -n : 'N'ormal RVC debug mode\n");
	printf("* -m : 'M'anual RVC debug mode\n");
	printf("* -a : 'A'utomatic RVC debug mode\n");
	printf("* -g : 'G'pio state of rear gear status\n");
	printf("* -h : this 'H'elp\n");
}

int main(int argc, char * argv[])
{
	int fd, ret_val;
	int opt;

	while ((opt = getopt(argc, argv, "hsgednma")) != -1) {
		switch (opt) {
		case 's': /* Stop animation */
			send_user_event(STOP_ANIM);
			break;
		case 'e': /* Engaged RVC use-case */
			send_user_event(START_RVC);
			break;
		case 'd': /* Disengaged RVC use-case */
			send_user_event(STOP_RVC);
			break;
		case 'n': /* Normal RVC debug mode */
			send_user_event(RVC_NORMAL);
			break;
		case 'm': /* Manual RVC debug mode */
			send_user_event(RVC_MANUAL_TOGGLING);
			break;
		case 'a': /* Automatic RVC debug mode */
			send_user_event(RVC_AUTO_TOGGLING);
			break;
		case 'g': /* get rear gear status */
			get_rear_gear_status();
			break;
		case 'h':
		default:
			usage();
			break;
		}
	}
}
