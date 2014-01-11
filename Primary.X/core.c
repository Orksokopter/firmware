#include "headers.h"

#define RC_CHANNEL_COUNT 7

typedef uint24_t time_in_ms;

#pragma udata access isrvars // für schnellen Zugriff in den Access-RAM legen (haben die UAVX-Leute auch so gemacht)
near uint16_t current_edge_time;
near int24_t last_edge_time;
near uint16_t pulse_width; // muss eigentlich nicht global sein
near uint8_t current_rc_channel;
near uint16_t raw_rc_data[RC_CHANNEL_COUNT+1]; // So passen die Indexe zu den Zahlen auf dem Empfänger :)
near bool current_rc_frame_ok;
near bool new_rc_values_available;
near bool new_gyro_values_available;
near bool motors_need_updating;
near bool motor_debug_needs_processing;
near bool signal;
bool lifesaver_timeout = false;
time_in_ms ms_clock = 0;
#pragma udata

time_in_ms last_valid_rc_frame = 0;

bool tmp_bool;

// Achtung, damit das Initialisieren so klappt, müssen die IDs fortlaufend sein!
int32_t parameters[] = {
	0, // YAW_KP
	0, // YAW_KI
	42, // YAW_KD
	42, // YAW_ILIMIT
	42, // YAW_RESOLUTIONFILTER
	42, // YAW_AVERAGINGFILTER
	0, // ROLL_KP
	0, // ROLL_KI
	42, // ROLL_KD
	42, // ROLL_ILIMIT
	42, // ROLL_RESOLUTIONFILTER
	42, // ROLL_AVERAGINGFILTER
	0, // PITCH_KP
	0, // PITCH_KI
	42, // PITCH_KD
	42, // PITCH_ILIMIT
	42, // PITCH_RESOLUTIONFILTER
	42, // PITCH_AVERAGINGFILTER
	0, // MISC_ACC_HORIZ_KI
	0, // MISC_ACC_VERT_KI
	42, // MISC_COMPASS_KI
	2,  // MISC_IDLE_SPEED
	0, // MISC_START_THRESHOLD
	0, // MISC_STOP_THRESHOLD
	42, // MISC_SKIP_CONTROL_CYCLES
	42  // MISC_ACC_RANGE
};

struct rc_data_t rc_data = {0,0,0,0,0,0,0,0,0};
// struct rc_offsets_t rc_offsets = {0,625,625,625,0,626};

void init_core(void)
{
	// === Timer und Capture/Compare-Einheit ===

	// 10 MHz Oszillatortakt -> 40 MHz Systemtakt -> 10 MHz Instruktionstakt
	
	// 10 MHz Instruktionstakt / 16 Prescaler -> 1,6 µs Timertaktlänge, daher theoretisch alle 65536
	// Timertakte = 104,8ms ein Timerüberlauf
	OpenTimer0(TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_16);
	
	// 10 MHz Instruktionstakt / 8 Prescaler -> 0,8 µs Timertaktlänge
	OpenTimer1(T1_16BIT_RW & TIMER_INT_OFF & T1_PS_1_8 & T1_SYNC_EXT_ON & T1_SOURCE_CCP & T1_SOURCE_INT);
	OpenCapture1(CAPTURE_INT_ON & C1_EVERY_FALL_EDGE);
	
//	TxQ.Head = TxQ.Tail = RxCheckSum = 0;
	
	INTCONbits.PEIE = true;	// Wofür ist das?
	// INTCONbits.TMR0IE = true;

	// === Fernbedienungsdecoder ===

	raw_rc_data[0] = 0xDEADBEEF;
	current_rc_frame_ok = false;
	new_rc_values_available = false;

}

#pragma code high_vector=0x08
#pragma interrupt isr
void isr(void)
{

	// === Eine PPM-Flanke vom Fernbedienungsempfänger wurde erkannt ===
	if(PIR1bits.CCP1IF)
	{
		current_edge_time = CCPR1;
		if ( current_edge_time < last_edge_time ) // Timer übergelaufen
			last_edge_time -= 0x00ffff;

		pulse_width = current_edge_time - last_edge_time;
		last_edge_time = current_edge_time;

		if (pulse_width > 6250) // 6250 Timertakte * 0,8µs Timertaktlänge = 5 ms
		{
			current_rc_channel = 1;
			current_rc_frame_ok = true;
		}
		else
		{
			if (pulse_width >= 1250 && pulse_width <= 2500) // zwischen 1 und 2 ms
				raw_rc_data[current_rc_channel] = pulse_width - 1250;
			else
				current_rc_frame_ok = false;

			if (current_rc_channel == sizeof(raw_rc_data)/sizeof(raw_rc_data[0]) - 1) // Letzter Kanal
			{
				if (current_rc_frame_ok)
				{
					new_rc_values_available = true;
					last_valid_rc_frame = ms_clock;
					signal = true;
				}
			}

			current_rc_channel++;
		}

		CCP1CONbits.CCP1M0 ^= 1; // Als nächstes auf die umgekehrte Flanke warten

		PIR1bits.CCP1IF = false;
	}

	// === Der ADC hat ne Wandlung fertig ===
	if (PIR1bits.ADIF)
	{
		if (*curr_chan == ADC_CH0)
			adc_values.batt = ReadADC();
		if (*curr_chan == ADC_CH1)
			adc_values.roll = ReadADC();
		if (*curr_chan == ADC_CH2)
			adc_values.pitch = ReadADC();
		if (*curr_chan == ADC_CH5)
			adc_values.yaw = ReadADC();

		curr_chan++;

		if (curr_chan == (channel_sequence + chan_count))
		{
			curr_chan = &channel_sequence[0];
			SetChanADC(*curr_chan);
			new_gyro_values_available = true;
		}
		else
		{
			SetChanADC(*curr_chan);
			ConvertADC();
		}

		PIR1bits.ADIF = false;
	}
	
	// === Interrupt Timer 1 ===
	if (INTCONbits.T0IF)
	{
		i16u temp;
		
		// Timer vorwärtsschieben, somit dauerts bis zum nächsten Überlauf nur 625 Timertakte * 1,6µs Timertaktlänge = 1ms
		temp.b0 = TMR0L;
		temp.b1 = TMR0H;
		temp.u16 += (65536 - 625);
		TMR0H = temp.b1;
		TMR0L = temp.b0;

		if (out_messages.waiting_for_ack_timer)
			out_messages.waiting_for_ack_timer--;

		PIE1bits.ADIE = true; // Das kann hier weg, oder?

		ms_clock++;

		if (signal && ms_clock - last_valid_rc_frame > 200)
			signal = false;

		if (!(ms_clock % 5))
			ConvertADC();

		// if (!(ms_clock % 5)) // erstmal einfach so, hab mir nichts dabei gedacht
		//	motors_need_updating = true;

		// if (!(ms_clock % 5000))
		//	motor_debug_needs_processing = true;
		
		if (lifesaver_timeout)
			lifesaver_timeout--;

		INTCONbits.T0IF = false;
	}

	// === Ein Byte ist per SPI eingegangen ===
	if (PIR1bits.SSPIF)
	{
		// Da wir nur beim ersten Byte frisch in diesen Block kommen und danach hier bleiben, wissen wir, daß dies das
		// erste Byte einer neuen Nachricht ist -> Puffer leeren
		spi_rec_buffer.read_cursor = 0;
		spi_rec_buffer.write_cursor = 0;

		while (!PORTAbits.NOT_SS && spi_rec_buffer.write_cursor < spi_rec_buffer.size) // den Ausgangspuffer brauchen wir nicht zu prüfen, von dem wird nur gelesen
		{
			spi_rec_buffer.data[spi_rec_buffer.write_cursor] = SSPBUF;
			SSPBUF = spi_out_msg_buffer.data[spi_out_msg_buffer.read_cursor];
			PIR1bits.SSPIF = 0;

			if (LATAbits.LATA4)
			{
				if (spi_out_msg_buffer.read_cursor == spi_out_msg_buffer.write_cursor)
				{
					spi_sending_state = SPI_SENDING_IDLE;
					LATAbits.LATA4 = 0; // IRQ-Leitung dauerhaft low, dann wird der Secondary das /CS gleich abschalten und das
					                    // Byte im Puffer nicht mehr abholen

					if (spi_out_msg_buffer.data[0] != 0 || spi_out_msg_buffer.data[1] != 0 || spi_out_msg_buffer.data[2] != 1)
					{
						// Wenns keine ACK-Nachricht war, war es wohl eine normale, wir warten also auf ein ACK

						// Und zwar 0.68 Sekunden lang, laut Data ist das "für einen Androiden fast eine Ewigkeit":
						out_messages.waiting_for_ack_timer = 664;
					}
				}
				else
				{
					// Ansonsten nur ein kurzer Low-Puls, damit er weiß, daß er weitere Bytes abholen kann
					LATAbits.LATA4 = 0;
					LATAbits.LATA4 = 0;
					LATAbits.LATA4 = 0;
					LATAbits.LATA4 = 0; // Um das zu erkennen, braucht er mindestens vier Zyklen
					LATAbits.LATA4 = 0;
					LATAbits.LATA4 = 1;
					spi_out_msg_buffer.read_cursor++;
				}
			}
			else
			{
				// Durch kurzes Wechseln der IRQ-Leitung teilen wir dem Secondary mit, daß er jetzt weitere Bytes schicken kann
				LATAbits.LATA4 = 1;
				LATAbits.LATA4 = 1;
				LATAbits.LATA4 = 1;
				LATAbits.LATA4 = 1; // Um das zu erkennen, braucht er mindestens vier Zyklen
				LATAbits.LATA4 = 1;
				LATAbits.LATA4 = 0;
			}

			spi_rec_buffer.write_cursor++;
			while (!PORTAbits.NOT_SS && !PIR1bits.SSPIF); // Wir warten, daß ein weiteres Byte kommt oder die Nachricht zu Ende ist
		}

		spi_message_received = true;

	}

}

#pragma code

void init_ports(void)
{
	// Eingänge (hochohmig):
	TRISDbits.TRISD2 = 1; // LED_CLOCK
	TRISDbits.TRISD3 = 1; // LED_DATA
	TRISEbits.TRISE1 = 1; // LED_LATCH

	TRISBbits.TRISB6 = 1; // PGC
	TRISBbits.TRISB7 = 1; // PGD

	// Heartbeat auf Pin 1 von Breakout 1
	TRISBbits.TRISB1 = 0;

	TRISAbits.TRISA0 = 1; // AN0
	TRISAbits.TRISA1 = 1; // AN1
	TRISAbits.TRISA2 = 1; // AN2
	TRISEbits.TRISE0 = 1; // AN5
	
	LATDbits.LATD4 = 0; // BL_CTRL_DATA fest low (I2C)
	LATDbits.LATD5 = 0; // BL_CTRL_CLOCK fest low (I2C)
	TRISDbits.TRISD4 = 1; // BL_CTRL_DATA high (idle)
	TRISDbits.TRISD5 = 1; // BL_CTRL_CLOCK high (I2C)

	TRISCbits.TRISC3 = 1; // SPI SCK
	TRISCbits.TRISC4 = 1; // SPI MISO
	TRISCbits.TRISC5 = 0; // SPI MOSI
	TRISAbits.TRISA5 = 1; // CHIPSELECT
	TRISAbits.TRISA4 = 0; // IRQ to Secondary
	LATAbits.LATA4 = 0; //  "  "      "     (low = nicht ausgelöst)

}

void heartbeat(void) {
//	LATBbits.LATB1 ^= 1;
}	


