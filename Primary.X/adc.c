#include "headers.h"

unsigned char * curr_chan;
unsigned char channel_sequence[] = {ADC_CH0, ADC_CH1, ADC_CH2, ADC_CH5};
int chan_count = sizeof(channel_sequence) / sizeof channel_sequence[0];
adc_values_t adc_values;

void init_adc()
{
	curr_chan = &channel_sequence[0];
	
	// Der C18-Weg um den ADC zu konfigurieren...
	OpenADC(
		ADC_FOSC_32 &         // Kann laut Datenblatt bei 40 MHz auch ADC_FOSC_64 sein
		ADC_RIGHT_JUST &      // Wir lesen eh 16 Bit
		ADC_20_TAD,           // 12 TAD?
		channel_sequence[0] &
		ADC_INT_ON &
		ADC_VREFPLUS_VDD &
		ADC_VREFMINUS_VSS,
		11                    // Damit sind AN0-AN3 analoge Eing‰nge, AN5 muﬂ leider auf digital bleiben, damit AN4 digital
		                      // sein kann. *kotz* Hoffen wir, daﬂ der Input Buffer es aush‰lt.
	);

}	