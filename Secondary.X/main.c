#include "config.h"
#include "headers.h"


void init_ports(void)
{
	TRISDbits.TRISD2 = 1; // LED_DATA inaktiv
	TRISDbits.TRISD3 = 1; // LED_CLOCK inaktiv
	TRISEbits.TRISE1 = 1; // LED_LATCH inaktiv
	TRISBbits.TRISB6 = 1; // PGC inaktiv
	TRISBbits.TRISB7 = 1; // PGC inaktiv
	
	// RX einschalten:
	TRISEbits.TRISE2 = 0;
	// LATEbits.LATE2 = 1;
	LAT(E,2) = 1;
	
	// Heartbeat auf Pin 1 von Breakout 2
	TRISBbits.TRISB0 = 0;

	// Für Debug-Ausgaben
	TRISBbits.TRISB3 = 0;
	TRISAbits.TRISA0 = 0;
	TRISAbits.TRISA1 = 0;
	TRISAbits.TRISA2 = 0;
	
	TRISAbits.TRISA5 = 0; // SER_DETACH
	LATAbits.LATA5 = 0;   //     "
	TRISDbits.TRISD4 = 0; // GPS_DETACH_AUX
	LATDbits.LATD4 = 1;   //     "
	TRISDbits.TRISD5 = 0; // GPS_DETACH_MAIN
	LATDbits.LATD5 = 0;   //     "
	
	TRISEbits.TRISE0 = 0; // SER_COMMAND
	LATEbits.LATE0 = 1;   // SER_COMMAND high (= Normalbetrieb)

	TRISCbits.TRISC3 = 0; // SPI SCK
	TRISCbits.TRISC4 = 1; // SPI MISO
	TRISCbits.TRISC5 = 0; // SPI MOSI
	TRISCbits.TRISC5 = 0; // SPI MOSI
	TRISDbits.TRISD7 = 0; // CHIPSELECT PRIMARY
	LATDbits.LATD7 = 1;   //     "         "    high (= nicht angesprochen)
	TRISBbits.TRISB1 = 1; // IRQ from Primary
	TRISDbits.TRISD6 = 0; // CHIPSELECT SD
	LATDbits.LATD6 = 1;   //     "       "      high (= nicht angesprochen)
	
}

void heartbeat(void)
{
	LATBbits.LATB0 ^= 1;
}

void main()
{
	volatile uint32_t penis;

	init_ports();
	init_communications();

	INTCONbits.PEIE=1; // Könnte mal in ne init_allgmeines_zeug_oder_so()
	EnableInterrupts;

	send_literal_uart_data("Ready", sizeof("Ready")-1);
LATBbits.LATB0 = 0;
	while(1)
	{
		//heartbeat();

		do_uart_sending();
		do_spi_comm();
		do_message_processing();

//		if (PIR1bits.RCIF)
//			TXREG = RCREG;

	}

}

