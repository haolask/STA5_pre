#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

static int count = 0;

int hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev,
                     libusb_hotplug_event event, void *user_data) {
  static libusb_device_handle *handle = NULL;
  struct libusb_device_descriptor desc;
  int rc;
  (void)libusb_get_device_descriptor(dev, &desc);
  if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
    rc = libusb_open(dev, &handle);
    if (LIBUSB_SUCCESS != rc) {
      printf("Could not open USB device\n");
    }
    else {
		libusb_control_transfer(handle, 0x40, 0x51, 1, 0, NULL, 0, 200);
	}
  } else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
    if (handle) {
      libusb_close(handle);
      handle = NULL;
    }
  } else {
    printf("Unhandled event %d\n", event);
  }
  count++;
  return 0;
}

#if 0
libusb_device_handle *h;
uint32_t resp;
libusb_open(dev, &h);
// check carplay support
ret = libusb_control_transfer(h, 0xc0, 0x53, 0, 0,&resp, 4, 1500);
int support=(ret == 4 && response) ? 1 : 0;
// custom role switch
if(support) ret = libusb_control_transfer(h, 0x40, 0x51, 1, 0, NULL, 0, 200);
libusb_close(dev, &h);
#endif



int main (void) {
  libusb_hotplug_callback_handle handle;
  int rc;
  libusb_init(NULL);
  rc = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
                                        LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, 0, 0x05ac, 0x12a8,
                                        LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, NULL,
                                        &handle);
  if (LIBUSB_SUCCESS != rc) {
    printf("Error creating a hotplug callback\n");
    libusb_exit(NULL);
    return EXIT_FAILURE;
  }
  while (count < 2) {
    libusb_handle_events_completed(NULL, NULL);
    usleep(10000);
  }
  libusb_hotplug_deregister_callback(NULL, handle);
  libusb_exit(NULL);
  return 0;
}

