void init_ports(void);
void init_core(void);
void heartbeat(void);
extern int32_t parameters[];

enum parameter_type {
	PARAM_YAW_KP = 0x000000,
	PARAM_YAW_KI = 0x000001,
	PARAM_YAW_KD = 0x000002,
	PARAM_YAW_ILIMIT = 0x000003,
	PARAM_YAW_RESOLUTIONFILTER = 0x000004,
	PARAM_YAW_AVERAGINGFILTER = 0x000005,
	PARAM_ROLL_KP = 0x000006,
	PARAM_ROLL_KI = 0x000007,
	PARAM_ROLL_KD = 0x000008,
	PARAM_ROLL_ILIMIT = 0x000009,
	PARAM_ROLL_RESOLUTIONFILTER = 0x00000a,
	PARAM_ROLL_AVERAGINGFILTER = 0x00000b,
	PARAM_PITCH_KP = 0x00000c,
	PARAM_PITCH_KI = 0x00000d,
	PARAM_PITCH_KD = 0x00000e,
	PARAM_PITCH_ILIMIT = 0x00000f,
	PARAM_PITCH_RESOLUTIONFILTER = 0x000010,
	PARAM_PITCH_AVERAGINGFILTER = 0x000011,
	PARAM_MISC_ACC_HORIZ_KI = 0x000012,
	PARAM_MISC_ACC_VERT_KI = 0x000013,
	PARAM_MISC_COMPASS_KI = 0x000014,
	PARAM_MISC_IDLE_SPEED = 0x000015,
	PARAM_MISC_START_THRESHOLD = 0x000016,
	PARAM_MISC_STOP_THRESHOLD = 0x000017,
	PARAM_MISC_SKIP_CONTROL_CYCLES = 0x000018,
	PARAM_MISC_ACC_RANGE = 0x000019
};

extern struct rc_data_t {
	int16_t left_vertical;
	int16_t left_horizontal;
	int16_t right_vertical;
	int16_t right_horizontal;
	bool sw2;
	int16_t ctrl7;
	int16_t ctrl5;
	bool pb8;
	bool sw1;
} rc_data;

extern struct rc_offsets_t {
    uint16_t left_vertical;
    uint16_t left_horizontal;
    uint16_t right_vertical;
    uint16_t right_horizontal;
    uint16_t ctrl7;
    uint16_t ctrl5;
} rc_offsets;

extern Flags F;

extern near bool new_rc_values_available;
extern near bool new_gyro_values_available;
extern near bool motors_need_updating;
extern near bool motor_debug_needs_processing;
extern near bool signal;
extern near uint16_t raw_rc_data[];

// In Capture/Compare-Timertakten:
#define RC_LONG_PULSE_LENGTH 1139
#define RC_SHORT_PULSE_LENGTH 140
#define RC_PULSE_LENGTH_RANGE (RC_LONG_PULSE_LENGTH-RC_SHORT_PULSE_LENGTH)
#define RC_MIDDLE_PULSE_LENGTH (RC_SHORT_PULSE_LENGTH + RC_PULSE_LENGTH_RANGE / 2)

extern bool lifesaver_timeout;
#define LIFESAVER_START 2;
