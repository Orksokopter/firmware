#include "headers.h"

// Hier kommen die Rohdaten vom seriellen Port rein
unsigned char ser_rec_buffer_data[128]; buffer ser_rec_buffer = { ser_rec_buffer_data, sizeof(ser_rec_buffer_data) / sizeof(char), 0, 0, 0};
unsigned char ser_send_buffer_data[128]; buffer ser_send_buffer = { ser_send_buffer_data, sizeof(ser_send_buffer_data) / sizeof(char), 0, 0 , 0};

// Über der Framing- und Escaping-Schicht kommt hier dann jeweils eine Nachricht rein
unsigned char in_msg_buffer_data[128]; buffer in_msg_buffer = { in_msg_buffer_data, sizeof(in_msg_buffer_data) / sizeof(char), 0, 0, 0};
unsigned char out_msg_buffer_data[128]; buffer out_msg_buffer = { out_msg_buffer_data, sizeof(out_msg_buffer_data) / sizeof(char), 0, 0, 0};

// Hier liegt eine Nachricht drin, durch Implementierung eines einfachen Flow-Control brauchen wir da keinen Rohdatenpuffer
unsigned char spi_in_msg_buffer_data[128]; buffer spi_in_msg_buffer = { spi_in_msg_buffer_data, sizeof(spi_in_msg_buffer_data) / sizeof(char), 0, 0, 0};
unsigned char spi_out_msg_buffer_data[128]; buffer spi_out_msg_buffer = { spi_out_msg_buffer_data, sizeof(spi_out_msg_buffer_data) / sizeof(char), 0, 0, 0};

// Hier liegt der Inhalt einer Proxy-Message drin, denn der kann auch schonmal größer werden als die normalerweise zur Verfügung stehenden 14 Bytes
unsigned char proxy_message_contents[255];

struct blah out_messages = {0, 0, false, 0};

// Da in dem Puffer immer nur eine Nachricht liegen kann, braucht auch nur eine zerlegte Nachricht vorgehalten zu werden.
// Hier kommt sie:
message in_msg;

bool spi_locked = true;
bool spi_message_locked = true;
bool spi_receiving = false;
bool last_irq_state = false;
int pending_cts_number = 0;
int previous_cts_number = 0;
bool clear_to_send_to_primary = true;

void process_message_from_primary(void);
void build_messages(void);

// === Hardware-Zeug ===================================================================================================

void init_communications()
{
	// --- UART ---

	// Laut Tabelle: Fosc = 40 MHz, BRGH, BRG16, 57k6 -> SPBRG = 172
	SPBRGH = 0;
	SPBRG = 172;

	// Nur TXEN (!TX9, !SYNC, BRGH)
	TXSTA = 0x24;

	// SPEN + CREN (!RX9, !ADDEN)
	RCSTA = 0x90;

	// (!RXDTP, !TXCKP, BRG16, !WUE, !ABDEN)
	BAUDCON = 0x08;

	PIE1bits.TXIE = 0;
	PIE1bits.RCIE = 1;

  // --- SPI ---

	// Der Secondary ist SPI-Master

  // (!SMP, CKE, !BF)
  SSPSTAT = 0x40;

  // (!WCOL, !SSPOV, SSPEN, !CKP, SSPM = 0b0000)
  SSPCON1 = 0x20;

	PIE1bits.SSPIE = 0;

	// Wir laden den Puffer mit Daten, damit kommt das MSSP-Modul sozusagen in seinen Idle-Zustand
	SSPBUF = 0;

	// --- Timer (für SPI Retransmission) ---

	// 8-Bit-Timer mit Prescaler 1:2 von Instruction Clock -> 204,8 µs
	// (TMR0ON, T08BIT, !T0CS, !PSA, T0PS = 0b000
	T0CON = 0xC0;

	INTCONbits.TMR0IE = 1;

	// Timer deaktiviert:
	// T0CON = 0;
	// INTCONbits.TMR0IE = 0;
}

// === SPI-Funktionen (Nachrichtenpuffer <-> Hardware) =================================================================

void do_spi_comm(void)
{
	uint16_t n;

	// Ggfs. die nächste Nachricht im Puffer vorbereiten
	build_messages();

	// Wenn der andere was zu sagen hat, lassen wir ihn reden
	if (PORTBbits.RB1)
	{
		// Puffer leeren
		spi_in_msg_buffer.read_cursor = 0;
		spi_in_msg_buffer.write_cursor = 0;

		LATDbits.LATD7 = 0; // /CS aktivieren
		
		while (spi_in_msg_buffer.write_cursor < spi_in_msg_buffer.size)
		{

			// Critical Section weil er sonst evtl. den Puls auf der IRQ-Leitung verpassen könnte
			DisableInterrupts;
			LATAbits.LATA1 = 1;
			// Ein Byte abholen
			SSPBUF = 0;

			// Wenn der Primary bereit ist, weitere Bytes zu senden, gibt er einen kurzen Low-Puls auf der IRQ-Leitung
			while(PORTBbits.RB1);
			LATAbits.LATA1 = 0;
			EnableInterrupts;
			

			// Das hier dauert einige Zyklen, währenddessen hat der Primary Zeit, die IRQ-Leitung wieder high zu setzen, falls
			// er noch weiter schicken will:
			spi_in_msg_buffer.data[spi_in_msg_buffer.write_cursor] = SSPBUF;

			// Wenn danach die IRQ-Leitung wieder high ist, will er noch weitere Bytes schicken
			if (PORTBbits.RB1)
				spi_in_msg_buffer.write_cursor++;
			else
				break; // ansonsten hören wir auf
		}

		LATDbits.LATD7 = 1; // /CS wieder aus

		process_message_from_primary();
	}
	else if (!spi_message_locked)
	{
		LATDbits.LATD7 = 0; // /CS aktivieren

		while (spi_out_msg_buffer.write_cursor > spi_out_msg_buffer.read_cursor)
		{
			// Critical Section weil er sonst evtl. den Puls auf der IRQ-Leitung verpassen könnte
			DisableInterrupts;
			SSPBUF = spi_out_msg_buffer.data[spi_out_msg_buffer.read_cursor];
			// Wenn der Primary bereit ist, weitere Bytes zu empfangen, gibt er einen kurzen High-Puls auf der IRQ-Leitung
			while(!PORTBbits.RB1);
				EnableInterrupts;
			spi_out_msg_buffer.read_cursor++;
		}

		LATDbits.LATD7 = 1; // /CS wieder aus

		spi_message_locked = true;

		if (spi_out_msg_buffer.data[0] != 0 || spi_out_msg_buffer.data[1] != 0 || spi_out_msg_buffer.data[2] != 1)
		{
			// Wenns keine ACK-Nachricht war, war es wohl eine normale, wir warten also auf ein ACK

			// Und zwar 0.68 Sekunden lang, laut Data ist das "für einen Androiden fast eine Ewigkeit":
			out_messages.waiting_for_ack_timer = 13281;
		}
	}
}

void add_message_to_primary(message *out_msg)
{
	out_messages.messages[out_messages.write_cursor] = *out_msg;
	out_messages.write_cursor++;
	clear_to_send_to_primary = false;
}

// Baut die nächste Nachricht im spi_out_msg_buffer zusammen
void build_messages(void)
{
	uint8_t i;
	message *out_msg;

	// Wenn die Nachricht im Puffer zum Senden freigegeben ist, machen wir nichts
	if (!spi_message_locked)
		return;

	if (out_messages.read_cursor == out_messages.write_cursor && !out_messages.ack_pending)
		return;

	if (out_messages.waiting_for_ack_timer)
		return;

	// Nachrichtenpuffer leeren

	spi_out_msg_buffer.read_cursor = 0;
	spi_out_msg_buffer.write_cursor = 0;

	if (out_messages.ack_pending)
	{
		spi_out_msg_buffer.data[0] = 0;
		spi_out_msg_buffer.data[1] = 0;
		spi_out_msg_buffer.data[2] = 1;
		spi_out_msg_buffer.data[3] = 1;
		spi_out_msg_buffer.data[4] = 0x12; // Prüfsumme hardcoded
		spi_out_msg_buffer.write_cursor = 5;

		out_messages.ack_pending = false;
	}
	else
	{
		out_msg = &out_messages.messages[out_messages.read_cursor];

		if (out_msg->type == MSG_PROXY_MESSAGE)
		{
			for (i = 0 ; i < out_msg->length ; i++)
				spi_out_msg_buffer.data[i] = out_msg->contents.proxy_message.data[i];

			spi_out_msg_buffer.data[out_msg->length] = crc(spi_out_msg_buffer.data, out_msg->length);

			spi_out_msg_buffer.write_cursor = out_msg->length + 1;
		}
		else
		{
			spi_out_msg_buffer.data[0] = ((uint24_t)out_msg->type >> 16) & 0xFF;
			spi_out_msg_buffer.data[1] = ((uint24_t)out_msg->type >> 8)  & 0xFF;
			spi_out_msg_buffer.data[2] = ((uint24_t)out_msg->type)       & 0xFF;

			spi_out_msg_buffer.data[3] = (out_msg->contents.pong.sequence_number >> 8) & 0xFF;
			spi_out_msg_buffer.data[4] = (out_msg->contents.pong.sequence_number)      & 0xFF;

			spi_out_msg_buffer.data[out_msg->length + 3] = crc(spi_out_msg_buffer.data, out_msg->length+3);

			spi_out_msg_buffer.write_cursor = out_msg->length + 4;
		}
	}

	// Nachricht zum Versand freigeben
	spi_message_locked = false;
}

void process_message_from_primary(void)
{
	uint8_t i;
	message tmp_msg;

	in_msg.type = ((uint24_t)spi_in_msg_buffer.data[0] << 16) | ((uint16_t)spi_in_msg_buffer.data[1] << 8) | (spi_in_msg_buffer.data[2]);

	// ACK-Nachricht verarbeiten
	if (in_msg.type == MSG_ACK && spi_in_msg_buffer.data[3] && spi_in_msg_buffer.data[4] == 0x12)
	{
		// Lesecursor weiterschieben, aber nie hinter den Schreibcursor
		if (out_messages.read_cursor != out_messages.write_cursor)
			out_messages.read_cursor++;

		// Wenn der Puffer danach leer ist, zurücksetzen, denn es ist aktuell kein Ringpuffer
		if (out_messages.read_cursor == out_messages.write_cursor)
		{
			out_messages.read_cursor = 0;
			out_messages.write_cursor = 0;
		}

		// Dann kann die nächste Nachricht geschickt werden
		out_messages.waiting_for_ack_timer = 0;

		return;
	}

	// Normale Nachricht verarbeiten

	// Zunächst müssen wir die Länge des Nachrichteninhalts ermitteln, die ist von Typ zu Typ unterschiedlich
	if (in_msg.type == MSG_PROXY_MESSAGE)
		in_msg.length = 2 + (((uint16_t)spi_in_msg_buffer.data[3] << 8) | (spi_in_msg_buffer.data[4]));

	if (in_msg.type == MSG_PONG)
		in_msg.length = 2;

	if (in_msg.type == MSG_CLEAR_TO_SEND)
		in_msg.length = 0;

	if (spi_in_msg_buffer.data[in_msg.length + 3] != crc(spi_in_msg_buffer.data, in_msg.length + 3)) // Prüfsumme über Nachrichtentyp und Inhalt
		return;

	out_messages.ack_pending = true;

	if (in_msg.type == MSG_PROXY_MESSAGE)
	{
		tmp_msg.type = MSG_PROXY_MESSAGE; // Beim Zusammenbau wird das beachtet und weggelassen

		for (i = 0 ; i < in_msg.length - 2 ; i++)
			proxy_message_contents[i] = spi_in_msg_buffer.data[5+i];

		tmp_msg.length = i;

		build_message_to_groundstation(&tmp_msg);
	}

	if (in_msg.type == MSG_CLEAR_TO_SEND)
		clear_to_send_to_primary = true;

}

// === Low-Level-Funktionen (Serielle Puffer <-> Hardware) =============================================================

// Nimmt den Datenstrom aus dem ser_send_buffer und schickt ihn auf die serielle Funkstrecke raus
void do_uart_sending(void)
{
	message cts_message;

	while (!PIR1bits.TXIF); // Blockt, kommt aber hoffentlich nie vor

	// Wenn wir mal schneller als mit 2k4 senden, sollten wir hier auf CTS prüfen

	// Ich schick erstmal immer nur ein Zeichen. Der Vorteil ist, wenn die Übertragung lange dauert (müßte man mal messen),
	// kann der µC derweil was anderes machen
	if (!ringbuffer_empty(&ser_send_buffer))
		TXREG = ringbuffer_shift(&ser_send_buffer);
	else if (pending_cts_number)
	{
		// Dadurch, das wir das hier machen, verhindern wir, dass der Ausgangspuffer volläuft mit CTS-Nachrichten
		cts_message.type = MSG_CLEAR_TO_SEND;
		cts_message.length = 6;
		cts_message.contents.clear_to_send.number = pending_cts_number;
		cts_message.contents.clear_to_send.previous_number = previous_cts_number;

		build_message_to_groundstation(&cts_message);

		previous_cts_number = pending_cts_number;
		pending_cts_number = 0;
	}

	Nop(); // Damit das Interrupt-Flag ganz sicher false werden kann
}

// === Mid-Level-Funktionen (Nachrichten <-> Serielle Puffer) ==========================================================

// Kopiert ROM-Daten in den ser_send_buffer
void send_literal_uart_data(const char* data, size_t length)
{
	// Hundertprozentig identisch zu send_uart_data bis auf die Signatur

	int i;

	for (i = 0; i < length; i++)
		ringbuffer_push(&ser_send_buffer, data[i]);

}

// Kopiert RAM-Daten in den ser_send_buffer
void send_uart_data(const char* data, size_t length)
{
	// Hundertprozentig identisch zu send_literal_uart_data bis auf die Signatur

	int i;

	for (i = 0; i < length; i++)
		ringbuffer_push(&ser_send_buffer, data[i]);
}

// Gibt true zurück, wenn noch Daten im ser_send_buffer liegen
bool uart_is_sending(void)
{
	return (!ringbuffer_empty(&ser_send_buffer));
}

// Nimmt den Datenstrom aus dem ser_rec_buffer, kopiert eine vollständige Nachricht in den in_msg_buffer und ruft dann
// process_message() auf

enum message_processing_state { INACTIVE, IN_MESSAGE, AFTER_ESCAPE, AFTER_MESSAGE } message_processing_state;

void do_message_processing(void)
{
	char b;

	// Wir verarbeiten erst die nächste Nachricht, wenn alle anderen Puffer leer sind, denn es könnten ja Nachrichten
	// dabei entstehen und der serielle Datenstrom wartet bereitwillig
	if (!ringbuffer_empty(&ser_send_buffer) || !(out_messages.read_cursor == out_messages.write_cursor && !out_messages.ack_pending) || !clear_to_send_to_primary)
		return;

	while(1) // ...bis zum bitteren Break
	{

		DisableInterrupts; // Der Ringpuffer darf nicht zwischendurch gelesen oder verändert werden
		if (!ringbuffer_empty(&ser_rec_buffer))
		{
			b = ringbuffer_shift(&ser_rec_buffer);
			EnableInterrupts;
		}
		else
		{
			EnableInterrupts;
			// Wenn kein neues Zeichen gelesen wurde, hat sich auch gegenüber dem letzten Durchlauf nichts geändert, brauchen
			// also nix machen
			break;
		}

		if (message_processing_state != AFTER_ESCAPE && b == STX)
		{
			ringbuffer_clear(&in_msg_buffer);
			message_processing_state = IN_MESSAGE;
		}
		else if (message_processing_state == IN_MESSAGE && b == ESC) // 0x1B
			message_processing_state = AFTER_ESCAPE;
		else if (message_processing_state == IN_MESSAGE && b == ETB) // 0x17
			message_processing_state = AFTER_MESSAGE;
		else if (message_processing_state == AFTER_ESCAPE || message_processing_state == IN_MESSAGE)
		{
			ringbuffer_push(&in_msg_buffer, b); // Eigentlich isses kein Ringpuffer

			if (message_processing_state == AFTER_ESCAPE)
				message_processing_state = IN_MESSAGE;
		}

		if (message_processing_state == AFTER_MESSAGE)
		{
			process_message();
			message_processing_state = INACTIVE;

			// Nach jeder Nachricht lassen wir ihn erstmal anderes Zeug machen (denn den eingehenden seriellen Datenstrom
			// können wir anhalten
			break;
		}
	}
}

// Überträgt eine Nachricht vom Nachrichtenpuffer in den Sendepuffer
// TODO: Ich sollte für die Länge die Cursor benutzen statt der zerlegten Nachricht
void send_message(int length)
{
	int i;
	char c;

	send_literal_uart_data("\x01\x01", 2);

	for (i = 0; i < length; i++)
	{
		c = out_msg_buffer.data[i];
		if (c == 0x01 || c == 0x1B || c == 0x17)
			send_literal_uart_data("\x1B", 1);
		send_uart_data(&c, 1);
	}

	send_literal_uart_data("\x17", 1);
}



// === High-Level-Funktionen (Anwendung <-> Nachrichtenpuffer) =========================================================

// Zerlegt die Nachricht im in_msg_buffer und bearbeitet sie
void process_message(void)
{
	uint8_t i;
	message tmp_msg;
	message tmp_msg2;

	in_msg.number = ((uint24_t)in_msg_buffer.data[0] << 16) | ((uint16_t)in_msg_buffer.data[1] << 8) | (in_msg_buffer.data[2]);

	in_msg.type = ((uint24_t)in_msg_buffer.data[3] << 16) | ((uint16_t)in_msg_buffer.data[4] << 8) | (in_msg_buffer.data[5]);

	// Zunächst müssen wir die Länge des Nachrichteninhalts ermitteln, die ist von Typ zu Typ unterschiedlich
	if (in_msg.type == MSG_PROXY_MESSAGE)
		in_msg.length = 2 + ((uint16_t)in_msg_buffer.data[6] << 8) | (in_msg_buffer.data[7]);

	if (in_msg.type == MSG_PING)
		in_msg.length = 2;

	if (in_msg.type == MSG_NOP)
		in_msg.length = 0;

	if (in_msg_buffer.data[in_msg.length + 6] != crc(in_msg_buffer.data, in_msg.length + 6)) // Prüfsumme über Nachrichtennummer, Nachrichtentyp und Inhalt
		return;

	pending_cts_number = in_msg.number;

	if (in_msg.type == MSG_PROXY_MESSAGE)
	{
		tmp_msg.type = MSG_PROXY_MESSAGE; // Beim Zusammenbau wird das beachtet und weggelassen

		for (i = 0 ; i < in_msg.length - 2 && i < 16 ; i++)
			tmp_msg.contents.proxy_message.data[i] = in_msg_buffer.data[8+i];

		tmp_msg.length = i;

		add_message_to_primary(&tmp_msg);
	}

	if (in_msg.type == MSG_PING)
	{
		in_msg.contents.ping.sequence_number = ((uint16_t)in_msg_buffer.data[6] << 8) | (in_msg_buffer.data[7]);

		// Antwort erstellen
		tmp_msg.type = MSG_PONG;
		tmp_msg.length = 2;
		tmp_msg.contents.pong.sequence_number = in_msg.contents.ping.sequence_number;

		build_message_to_groundstation(&tmp_msg);
	}
}

// Baut eine Nachricht im out_msg_buffer zusammen
void build_message_to_groundstation(message *out_msg)
{
	uint8_t i;

	// Nachrichtenpuffer leeren
	out_msg_buffer.read_cursor = 0;
	out_msg_buffer.write_cursor = 0;

	if (out_msg->type == MSG_PROXY_MESSAGE)
	{
		for (i = 0 ; i < out_msg->length ; i++)
			out_msg_buffer.data[i] = proxy_message_contents[i];

		out_msg_buffer.data[out_msg->length] = crc(out_msg_buffer.data, out_msg->length);

		send_message(out_msg->length+1);
	}
	else
	{
		out_msg_buffer.data[0] = ((uint24_t)out_msg->type >> 16) & 0xFF;
		out_msg_buffer.data[1] = ((uint24_t)out_msg->type >> 8)  & 0xFF;
		out_msg_buffer.data[2] = ((uint24_t)out_msg->type)       & 0xFF;

		if (out_msg->type == MSG_PONG)
		{
			out_msg_buffer.data[3] = (out_msg->contents.pong.sequence_number >> 8) & 0xFF;
			out_msg_buffer.data[4] = (out_msg->contents.pong.sequence_number)      & 0xFF;
		}
		if (out_msg->type == MSG_CLEAR_TO_SEND)
		{
			out_msg_buffer.data[3] = (out_msg->contents.clear_to_send.number >> 16) & 0xFF;
			out_msg_buffer.data[4] = (out_msg->contents.clear_to_send.number >> 8)  & 0xFF;
			out_msg_buffer.data[5] = (out_msg->contents.clear_to_send.number)       & 0xFF;

			out_msg_buffer.data[6] = (out_msg->contents.clear_to_send.previous_number >> 16) & 0xFF;
			out_msg_buffer.data[7] = (out_msg->contents.clear_to_send.previous_number >> 8)  & 0xFF;
			out_msg_buffer.data[8] = (out_msg->contents.clear_to_send.previous_number)       & 0xFF;
		}

		out_msg_buffer.data[out_msg->length + 3] = crc(out_msg_buffer.data, out_msg->length+3);

		send_message(out_msg->length+4);
	}
}
