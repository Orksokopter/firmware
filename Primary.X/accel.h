typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} accel_values_t;

extern accel_values_t raw_accel_values;
extern accel_values_t accel_values;
extern accel_values_t accel_offsets;

// void do_accel_reading(void);
void acquire_accel_values(void);

