#define FRAME_SIZE 8

struct can_frame {
	uint32_t id;
	uint32_t length;
	uint8_t data[FRAME_SIZE];
};

int can_get_frame(struct can_frame *f);
void can_send_frame(struct can_frame *f);
void can_set_loopback(bool enabled);
