extern struct motor_data_t {
	uint8_t front;
	uint8_t left;
	uint8_t right;
	uint8_t rear;
} motor_data;

extern struct gyro_offsets_t {
	uint16_t roll_zero;
	uint16_t yaw_zero;
	uint16_t pitch_zero;
} gyro_offsets;

void do_control(void);
void do_control2(void);

extern byte debug1;
extern byte debug2;
extern byte debug3;
extern byte debug4;
