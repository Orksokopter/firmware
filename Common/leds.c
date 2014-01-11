#include <p18f4620.h>
#include "leds.h"

int led_status = 0;

void init_leds()
{
	TRISDbits.TRISD2 = 0;
	TRISDbits.TRISD3 = 0;
	TRISEbits.TRISE1 = 0;
	LATDbits.LATD2 = 0;
	LATDbits.LATD3 = 0;
	LATEbits.LATE1 = 0;
}

void write_out_leds()
{
	int pattern;
	int i;

	pattern = led_status;
	for (i = 0 ; i < 8 ; i++)
	{
		if (pattern & 1)
			LATDbits.LATD2 = 1;
		else
			LATDbits.LATD2 = 0;

		LATDbits.LATD3 = 1;
		LATDbits.LATD3 = 0;
		pattern >>= 1;
	}
	LATEbits.LATE1 = 1;
	LATEbits.LATE1 = 0;
}
