#include "headers.h"

int hurz = 0;

void interrupt isr(void)
{
	volatile char dummy;

	if (PIR1bits.RCIF)
	{
		// ringbuffer_push inline wegen Contextsaveinterruptlatenzscheiﬂe

		if (ser_rec_buffer.fill_level == ser_rec_buffer.size)
			dummy = RCREG;
		else
		{
			ser_rec_buffer.data[ser_rec_buffer.write_cursor] = RCREG;

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
