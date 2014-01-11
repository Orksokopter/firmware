#include "headers.h"
#include "motors.h"

#define MOTORS_I2C_DELAY Delay5us();

#pragma udata motor_debug
uint8_t motor_debug[100];
#pragma udata
uint8_t motor_debug_pos = 0;

void write_i2c_bytes(byte length, byte *data)
{
	byte i, j;

	TRISDbits.TRISD5 = 1; // Sollte zwar noch high sein, aber wurscht
	MOTORS_I2C_DELAY
	TRISDbits.TRISD4 = 0; // DATA low (Start condition)
	MOTORS_I2C_DELAY
	TRISDbits.TRISD5 = 0; // CLOCK low
	MOTORS_I2C_DELAY

	for (j = 0; j < length; j++)
	{
		for (i = 0; i < 8; i++)
		{
			if (data[j] & 128)
				TRISDbits.TRISD4 = 1; // DATA high
			else
				TRISDbits.TRISD4 = 0; // DATA low

			MOTORS_I2C_DELAY
			TRISDbits.TRISD5 = 1; // CLOCK high
			while(!PORTDbits.RD5); // Clock stretching
			MOTORS_I2C_DELAY
			TRISDbits.TRISD5 = 0; // CLOCK low
			data[j] <<= 1;
		}
		MOTORS_I2C_DELAY
		TRISDbits.TRISD4 = 1; // DATA high
		MOTORS_I2C_DELAY
		TRISDbits.TRISD5 = 1; // CLOCK high
		MOTORS_I2C_DELAY
		// Hier könnte ich ACK lesen
		MOTORS_I2C_DELAY
		TRISDbits.TRISD5 = 0; // CLOCK low
		MOTORS_I2C_DELAY
	}
	TRISDbits.TRISD4 = 0; // DATA low
	MOTORS_I2C_DELAY
	TRISDbits.TRISD5 = 1; // CLOCK high
	while(!PORTDbits.RD5); // Clock stretching
	MOTORS_I2C_DELAY
	TRISDbits.TRISD4 = 1; // DATA high (Stop condition)
	MOTORS_I2C_DELAY
}

void write_out_motor(byte addr, byte value)
{
	byte data[2];

	addr = 0x50 + 2 * addr;
	data[0] = addr;
	data[1] = value;
	write_i2c_bytes(2,data);
	MOTORS_I2C_DELAY
}

void write_out_motors(void)
{
	message tmp_msg;

	if (!motors_need_updating)
		return;
	motors_need_updating = false;

	led_status |= LED8BLUE;
	write_out_leds();

	write_out_motor(1, motor_data.rear);
	write_out_motor(2, motor_data.front);
	write_out_motor(3, motor_data.right);
	write_out_motor(4, motor_data.left);

	write_out_motor(21, debug1);
	write_out_motor(22, debug2);
	write_out_motor(23, debug3);
	write_out_motor(24, debug4);

	if (motor_debug_pos > 96)
	{
		if (out_messages.write_cursor == out_messages.read_cursor && motor_debug_needs_processing)
		{
			motor_debug_needs_processing = false;

			led_status ^= LED6YELLOW;
			write_out_leds();

			tmp_msg.type = MSG_DECIMAL_DEBUG_DUMP;
			tmp_msg.length = 100;
			tmp_msg.contents.decimal_debug_dump.data = motor_debug;
			add_message(&tmp_msg);
		}
 
		motor_debug_pos = 0;
	}
	
	motor_debug[motor_debug_pos++] = motor_data.rear;
	motor_debug[motor_debug_pos++] = motor_data.front;
	motor_debug[motor_debug_pos++] = motor_data.right;
	motor_debug[motor_debug_pos++] = motor_data.left;

	led_status &= ~LED8BLUE;
	write_out_leds();
}