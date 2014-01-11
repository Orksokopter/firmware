#include "headers.h"
#include "accel.h"

accel_values_t raw_accel_values = {0,0,0};
accel_values_t accel_values = {0,0,0};
accel_values_t accel_offsets = {0,0,0};

#define ACCEL_FILTER_LENGTH 30

struct {
	accel_values_t data[ACCEL_FILTER_LENGTH];
	uint8_t pos;
	struct {
		int32_t x;
		int32_t y;
		int32_t z;
	} sum;
} accel_fir = {
	{ {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}, {0,0,0} },
	0,
	{0,0,0}
};

#define ACCEL_I2C_DELAY Delay25Cycles(); Delay10Cycles();
#define ACCEL_START_CONDITION TRISBbits.TRISB4 = 1; ACCEL_I2C_DELAY; TRISBbits.TRISB0 = 0; ACCEL_I2C_DELAY; TRISBbits.TRISB4 = 0; ACCEL_I2C_DELAY
#define ACCEL_REPEATED_START_CONDITION TRISBbits.TRISB4 = 1; ACCEL_I2C_DELAY; TRISBbits.TRISB0 = 0; ACCEL_I2C_DELAY; TRISBbits.TRISB4 = 0; ACCEL_I2C_DELAY
#define ACCEL_STOP_CONDITION TRISBbits.TRISB0 = 0; ACCEL_I2C_DELAY; TRISBbits.TRISB4 = 1; while(!PORTBbits.RB4); ACCEL_I2C_DELAY; TRISBbits.TRISB0 = 1; ACCEL_I2C_DELAY
#define ACCEL_DATA_HIGH TRISBbits.TRISB0 = 1
#define ACCEL_DATA_LOW TRISBbits.TRISB0 = 0
#define ACCEL_CLOCK_PULSE TRISBbits.TRISB4 = 1; while(!PORTBbits.RB4); ACCEL_I2C_DELAY; TRISBbits.TRISB4 = 0; ACCEL_I2C_DELAY
#define ACCEL_SEND_BYTE(the_byte) temp_byte = (the_byte); for (i = 0; i < 8; i++) { if (temp_byte & 128) ACCEL_DATA_HIGH; else ACCEL_DATA_LOW; ACCEL_CLOCK_PULSE; temp_byte <<= 1; }
#define ACCEL_READ_ACK TRISBbits.TRISB0 = 1; ACCEL_I2C_DELAY; TRISBbits.TRISB4 = 1; ACCEL_I2C_DELAY; /* Hier könnte ich ACK lesen */ ACCEL_I2C_DELAY; TRISBbits.TRISB4 = 0; ACCEL_I2C_DELAY
#define ACCEL_READ_BYTE(the_byte) for (i = 0; i < 8; i++) { temp_byte <<= 1; TRISBbits.TRISB4 = 1; while(!PORTBbits.RB4); if (PORTBbits.RB0) temp_byte |= 1; else temp_byte &= ~1; ACCEL_I2C_DELAY;	TRISBbits.TRISB4 = 0; ACCEL_I2C_DELAY; } the_byte = temp_byte
#define ACCEL_SEND_ACK TRISBbits.TRISB0 = 0; ACCEL_I2C_DELAY; TRISBbits.TRISB4 = 1; while(!PORTBbits.RB4); ACCEL_I2C_DELAY; TRISBbits.TRISB4 = 0; ACCEL_I2C_DELAY; TRISBbits.TRISB0 = 1
#define ACCEL_SEND_NACK TRISBbits.TRISB0 = 1; ACCEL_I2C_DELAY; TRISBbits.TRISB4 = 1; while(!PORTBbits.RB4); ACCEL_I2C_DELAY; TRISBbits.TRISB4 = 0; ACCEL_I2C_DELAY

void read_accel_data(uint8_t address, uint8_t length, uint8_t *data)
{
	uint8_t i, j;

	uint8_t temp_byte;

	ACCEL_START_CONDITION;
	ACCEL_SEND_BYTE(0x80); // Schreiben auf I²C-Adresse 0x40
	ACCEL_READ_ACK;
	ACCEL_SEND_BYTE(address); // Die zu lesende Registeradresse
	ACCEL_READ_ACK;
	ACCEL_REPEATED_START_CONDITION;
	ACCEL_SEND_BYTE(0x81); // Lesen von I²C-Adresse 0x40
	ACCEL_READ_ACK;
	for (j = 0 ; j < length ; j++)
	{
		ACCEL_READ_BYTE(*data);
		if (j == length-1)
		{
			ACCEL_SEND_NACK;
		}
		else
		{
			ACCEL_SEND_ACK;
		}
		data++;
	}
	ACCEL_STOP_CONDITION;
}

void acquire_accel_values()
{
	uint8_t data[8];

	read_accel_data(0x00, 8, data);

	raw_accel_values.x = ((uint16_t)data[3] << 8) + (data[2] & ~1);
	raw_accel_values.y = ((uint16_t)data[5] << 8) + (data[4] & ~1);
	raw_accel_values.z = ((uint16_t)data[7] << 8) + (data[6] & ~1);

	accel_fir.sum.x -= accel_fir.data[accel_fir.pos].x;
	accel_fir.sum.y -= accel_fir.data[accel_fir.pos].y;
	accel_fir.sum.z -= accel_fir.data[accel_fir.pos].z;
	accel_fir.sum.x += raw_accel_values.x;
	accel_fir.sum.y += raw_accel_values.y;
	accel_fir.sum.z += raw_accel_values.z;
	accel_fir.data[accel_fir.pos].x = raw_accel_values.x;
	accel_fir.data[accel_fir.pos].y = raw_accel_values.y;
	accel_fir.data[accel_fir.pos].z = raw_accel_values.z;

	accel_values.x = accel_fir.sum.x / ACCEL_FILTER_LENGTH;
	accel_values.y = accel_fir.sum.y / ACCEL_FILTER_LENGTH;
	accel_values.z = accel_fir.sum.z / ACCEL_FILTER_LENGTH;

	accel_fir.pos = (accel_fir.pos + 1) % ACCEL_FILTER_LENGTH;
}