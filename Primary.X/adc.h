extern void init_adc(void);
extern unsigned char * curr_chan;
extern unsigned char channel_sequence[];
extern int chan_count;
typedef struct {
	uint16_t batt;
	uint16_t roll;
	uint16_t pitch;
	uint16_t yaw;
} adc_values_t;

extern adc_values_t adc_values;
