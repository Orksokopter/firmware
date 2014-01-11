#include "config.h"
#include "headers.h"

void calibrate_gyros(void) {
	uint8_t i;

	uint32_t temp_pitch = 0;
	uint32_t temp_roll = 0;
	uint32_t temp_yaw = 0;

	uint8_t sample_count = 10;

	for (i = 0; i < 10; i++)
	{
		while (!new_gyro_values_available); // Warten, dass neue Werte vom ADC kommen
		new_gyro_values_available = false;

		temp_pitch += adc_values.pitch;
		temp_roll += adc_values.roll;
		temp_yaw += adc_values.yaw;
	}

	gyro_offsets.pitch_zero = temp_pitch / sample_count;
	gyro_offsets.roll_zero = temp_roll / sample_count;
	gyro_offsets.yaw_zero = temp_yaw / sample_count;
}

void calibrate_accel(void) {
	uint8_t i;

	int32_t temp_x = 0;
	int32_t temp_y = 0;
	int32_t temp_z = 0;

	uint8_t sample_count = 10;

	for (i = 0; i < 10; i++)
	{
		acquire_accel_values();

		temp_x += raw_accel_values.x;
		temp_y += raw_accel_values.y;
		temp_z += raw_accel_values.z;

		Delay_ms(5);
	}

	accel_offsets.x = temp_x / sample_count;
	accel_offsets.y = temp_y / sample_count;
	accel_offsets.z = temp_z / sample_count;
}

void main()
{
	unsigned int i, j;
	message tmp_msg;
	char a = 'a';

	signed int roll_pos = 0;

	init_ports();
	init_leds();
	init_core();
	init_adc();
	init_communications();

	led_status = 0x0F;
	write_out_leds();
	Delay_ms(200);
	led_status = 0;
	write_out_leds();
	Delay_ms(200);
	led_status = 0xF0;
	write_out_leds();
	Delay_ms(200);
	led_status = 0;
	write_out_leds();
	Delay_ms(200);

	EnableInterrupts;

	calibrate_gyros();
	calibrate_accel();

	// acquire_accel_values(); // Codezeile nur hier geparkt

	while(1)
	{
		/*
		if (!(i++ % 10))
			roll_pos += (adc_values.roll - 513) / 5;


		led_status = 0;
		if (roll_pos < -1000)
			led_status += 1;
		if (roll_pos < -2000)
			led_status += 2;
		if (roll_pos < -3000)
			led_status += 4;
		if (roll_pos < -4000)
			led_status += 8;
		if (roll_pos > 1000)
			led_status += 16;
		if (roll_pos > 2000)
			led_status += 32;
		if (roll_pos > 3000)
			led_status += 64;
		if (roll_pos > 4000)
			led_status += 128;

		write_out_leds();

		heartbeat();

		write_out_motors(3, (roll_pos > 0 ? roll_pos/100 : -roll_pos/100) );

		*/

		if (rc_data.sw1)
		{
			calibrate_gyros();
//			calibrate_accel();
		}

		decode_rc_signals();
		do_control2();
		write_out_motors();

		if (signal)
			led_status |= LED3GREEN;
		else
			led_status &= ~LED3GREEN;

		write_out_leds();

		do_message_processing();

	}
}

