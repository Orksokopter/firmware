#include "headers.h"

#pragma code isr=0x08
#pragma interrupt isr

int hurz = 0;

void isr(void)
{
	char dummy;

	if (PIR1bits.RCIF)
	{
		// ringbuffer_push inline wegen Contextsaveinterruptlatenzscheiﬂe

		if (ser_rec_buffer.fill_level == ser_rec_buffer.size)
			dummy = RCREG;
		else
		{
			ser_rec_buffer.data[ser_rec_buffer.write_cursor] = RCREG;

			if (ser_rec_buffer.data[ser_rec_buffer.write_cursor] == 0x01)
				hurz = 1;
			else if (hurz == 1 && ser_rec_buffer.data[ser_rec_buffer.write_cursor] == 0x00)
				hurz = 2;
			else if (hurz == 2 && ser_rec_buffer.data[ser_rec_buffer.write_cursor] == 0x00)
				hurz = 3;
			else if (hurz == 3 && ser_rec_buffer.data[ser_rec_buffer.write_cursor] == 0x13)
			{
				LATAbits.LATA2 = 1;
				LATAbits.LATA2 = 0;
			}

			ser_rec_buffer.write_cursor++;
			if (ser_rec_buffer.write_cursor == ser_rec_buffer.size)
				ser_rec_buffer.write_cursor = 0;
			ser_rec_buffer.fill_level++;
		}
	}

	if (INTCONbits.TMR0IF)
	{
		if (out_messages.waiting_for_ack_timer)
			out_messages.waiting_for_ack_timer--;

		INTCONbits.TMR0IF = 0;
	}
}

#pragma code
