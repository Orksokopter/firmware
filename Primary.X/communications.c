#include "headers.h"

#pragma udata spi_in_buffers
// Hier liegt eine Nachricht drin, durch Implementierung eines einfachen Flow-Control brauchen wir da keinen
// Rohdatenpuffer. Dennoch brauchen wir ein Doublebuffering, damit wir nicht während des Zerlegens der Nachricht
// dauernd prüfen müssen, ob der Puffer verworfen werden muß, weil er für die nächste Nachricht gebraucht wird.
char spi_rec_buffer_data[128]; buffer spi_rec_buffer = { spi_rec_buffer_data, sizeof(spi_rec_buffer_data) / sizeof(char), 0, 0 };
char spi_in_msg_buffer_data[128]; buffer spi_in_msg_buffer = { spi_in_msg_buffer_data, sizeof(spi_in_msg_buffer_data) / sizeof(char), 0, 0 };

// Beim ausgenhenden Puffer brauchen wir das nicht, denn hier haben wir (über spi_sending_state) selbst in der Hand,
// ob vom Puffer gelesen wird oder nicht
#pragma udata spi_out_buffers
char spi_out_msg_buffer_data[128]; buffer spi_out_msg_buffer = { spi_out_msg_buffer_data, sizeof(spi_out_msg_buffer_data) / sizeof(char), 0, 0 };
#pragma udata

#pragma udata out_message_buffer
struct {
	uint8_t write_cursor;
	uint8_t read_cursor;
	uint8_t size;
	bool ack_pending;
	uint16_t waiting_for_ack_timer;
	message messages[10];
} out_messages = {0, 0, 10, false, 0};
#pragma udata

bool spi_message_received = false;
enum {SPI_SENDING_IDLE, SPI_SENDING_PENDING, SPI_SENDING_PROGRESS} spi_sending_state = SPI_SENDING_IDLE;

// Da in dem Puffer immer nur eine Nachricht liegen kann, braucht auch nur eine zerlegte Nachricht vorgehalten zu werden.
// Hier kommt sie:
message in_msg;

uint8_t buffer_overflows = 0;
bool buffer_report_pending = false;

bool cts_when_empty = false;

// === Hardware-Zeug ===================================================================================================

void init_communications()
{
  // --- SPI ---

	// Der Primary ist SPI-Slave

	// (!SMP, CKE, !BF)
  SSPSTAT = 0x40;

  // (!WCOL, !SSPOV, SSPEN, !CKP, SSPM = 0b0100)
  SSPCON1 = 0x24;

	PIE1bits.SSPIE = 1;
}

// Kopiert die Nachricht aus dem ersten in den zweiten Puffer
void do_message_processing(void)
{
	char b;

	if (spi_message_received)
	{
		led_status |= LED5RED;
		write_out_leds();

		spi_message_received = false;
		ringbuffer_clear(&spi_in_msg_buffer);

		while(1) // ...bis zum bitteren Break
		{
			// Falls zwischendurch eine neue Nachricht begonnen wurde, brechen wir die Verarbeitung sofort ab
			if (spi_message_received)
				return;

			DisableInterrupts; // Der Ringpuffer darf nicht zwischendurch gelesen oder verändert werden
			if (!ringbuffer_empty(&spi_rec_buffer))
			{
				b = ringbuffer_shift(&spi_rec_buffer);
				EnableInterrupts;

				ringbuffer_push(&spi_in_msg_buffer, b); // Eigentlich isses kein Ringpuffer
			}
			else
			{
				EnableInterrupts;
				break;
			}
		}

		if (spi_message_received)
			return;

		// Wenn wir es geschafft haben, ohne Unterbrechung eine neue Nachricht in den spi_in_msg_buffer zu kopieren, müsste
		// sie konsistent sein und wir können sie verarbeiten:
		process_message();
	}

	build_messages();

	if (spi_sending_state == SPI_SENDING_PENDING && PORTAbits.NOT_SS) // auch einen Interrupt fordern wir nur an, wenn gerade nicht gesprochen wird
	{
		spi_sending_state = SPI_SENDING_PROGRESS;
		SSPBUF = ringbuffer_shift(&spi_out_msg_buffer); // Erstes Byte in den Puffer laden...
		LATAbits.LATA4 = 1; // ... und Abholung anfordern
	}

	if (led_status & LED5RED)
	{
		led_status &= ~LED5RED;
		write_out_leds();
	}

}

// Zerlegt die Nachricht im in_msg_buffer und bearbeitet sie
void process_message(void)
{
	message tmp_msg;

	uint16_u temp16;
	uint32_u temp32;

	in_msg.type = ((uint24_t)spi_in_msg_buffer.data[0] << 16) | ((uint16_t)spi_in_msg_buffer.data[1] << 8) | (spi_in_msg_buffer.data[2]);

	// Die Null-Nachricht, die zum Empfangen gebraucht wird, wäre theoretisch eine gültige Nachricht und würde ein ACK
	// hervorrufen, das wollen wir nicht, daher brechen wir hier ab.
	if (in_msg.type == 0)
		return;

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

			tmp_msg.type = MSG_CLEAR_TO_SEND;
			tmp_msg.length = 0;

			if (cts_when_empty)
				add_message(&tmp_msg);
			cts_when_empty = false; // nach der CTS-Nachricht natürlich nicht :)

		}

		// Dann kann die nächste Nachricht geschickt werden
		out_messages.waiting_for_ack_timer = 0;

		return;
	}

	// Normale Nachricht verarbeiten

	// Zunächst müssen wir die Länge des Nachrichteninhalts ermitteln, die ist von Typ zu Typ unterschiedlich
	// ADD NEW MESSAGE TYPE HERE
	if (in_msg.type == MSG_PING)
		in_msg.length = 2;
	if (in_msg.type == MSG_SET_PARAMETER)
		in_msg.length = 6;
	if (in_msg.type == MSG_GET_PARAMETER)
		in_msg.length = 2;

	if (spi_in_msg_buffer.data[in_msg.length + 3] != crc(spi_in_msg_buffer.data, in_msg.length + 3)) // Prüfsumme über Nachrichtentyp und Inhalt
		return;

	out_messages.ack_pending = true;

	// ADD NEW MESSAGE TYPE HERE
	if (in_msg.type == MSG_PING)
	{
		in_msg.contents.ping.sequence_number = ((uint16_t)spi_in_msg_buffer.data[3] << 8) | (spi_in_msg_buffer.data[4]);

		tmp_msg.type = MSG_PONG;
		tmp_msg.length = 2; // Wieso setze ich eigentlich hier schon die Länge?
		tmp_msg.contents.pong.sequence_number = in_msg.contents.ping.sequence_number;

		add_message(&tmp_msg);
	}
	
	if (in_msg.type == MSG_SET_PARAMETER)
	{
		temp16.b1 = spi_in_msg_buffer.data[3];
		temp16.b0 = spi_in_msg_buffer.data[4];
		in_msg.contents.set_parameter.type = temp16.u16;

		temp32.b3 = spi_in_msg_buffer.data[5];
		temp32.b2 = spi_in_msg_buffer.data[6];
		temp32.b1 = spi_in_msg_buffer.data[7];
		temp32.b0 = spi_in_msg_buffer.data[8];
		in_msg.contents.set_parameter.value = temp32.u32 - 0x80000000;

		parameters[in_msg.contents.set_parameter.type] = in_msg.contents.set_parameter.value;

		tmp_msg.type = MSG_CUR_PARAMETER;
		tmp_msg.length = 6;
		tmp_msg.contents.cur_parameter.type = in_msg.contents.set_parameter.type;
		tmp_msg.contents.cur_parameter.value = parameters[in_msg.contents.set_parameter.type] + 0x80000000;

		add_message(&tmp_msg);
	}
	
	if (in_msg.type == MSG_GET_PARAMETER)
	{
		temp16.b1 = spi_in_msg_buffer.data[3];
		temp16.b0 = spi_in_msg_buffer.data[4];
		in_msg.contents.get_parameter.type = temp16.u16;

		tmp_msg.type = MSG_CUR_PARAMETER;
		tmp_msg.length = 6;
		tmp_msg.contents.cur_parameter.type = in_msg.contents.get_parameter.type;
		tmp_msg.contents.cur_parameter.value = parameters[in_msg.contents.get_parameter.type] + 0x80000000;

		add_message(&tmp_msg);
	}
	
}

void add_message(message *out_msg)
{
	if (out_messages.write_cursor < out_messages.size)
	{
		out_messages.messages[out_messages.write_cursor] = *out_msg;
		out_messages.write_cursor++;
		cts_when_empty = true; // warum hier?
	}
	else
	{
		buffer_overflows++;
		buffer_report_pending = true;
		led_status |= LED1RED;
	}
}

// Baut eine Nachricht im out_msg_buffer zusammen
void build_messages(void)
{
	bool needs_proxy;
	uint8_t offset;
	message *out_msg;
	uint16_t i16;

	if (spi_sending_state != SPI_SENDING_IDLE)
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

		needs_proxy = false;
		offset = 0;

		// ADD NEW MESSAGE TYPE HERE
		if (out_msg->type == MSG_PONG)
			needs_proxy = true;
		if (out_msg->type == MSG_CUR_PARAMETER)
			needs_proxy = true;
		if (out_msg->type == MSG_DECIMAL_DEBUG_DUMP)
			needs_proxy = true;

		if (needs_proxy)
		{
			spi_out_msg_buffer.data[0] = 0;
			spi_out_msg_buffer.data[1] = 0;
			spi_out_msg_buffer.data[2] = 2;
			spi_out_msg_buffer.data[3] = (((uint24_t)out_msg->length + 3) >> 8) & 0xFF;
			spi_out_msg_buffer.data[4] = (((uint24_t)out_msg->length + 3))      & 0xFF;

			offset = 5;
		}
		
		spi_out_msg_buffer.data[offset + 0] = ((uint24_t)out_msg->type >> 16) & 0xFF;
		spi_out_msg_buffer.data[offset + 1] = ((uint24_t)out_msg->type >> 8)  & 0xFF;
		spi_out_msg_buffer.data[offset + 2] = ((uint24_t)out_msg->type)       & 0xFF;

		// ADD NEW MESSAGE TYPE HERE
		if (out_msg->type == MSG_PONG)
		{
			spi_out_msg_buffer.data[offset + 3] = (out_msg->contents.pong.sequence_number >> 8) & 0xFF;
			spi_out_msg_buffer.data[offset + 4] = (out_msg->contents.pong.sequence_number)      & 0xFF;
		}

		if (out_msg->type == MSG_CUR_PARAMETER)
		{
			spi_out_msg_buffer.data[offset + 3] = (out_msg->contents.cur_parameter.type >> 8) & 0xFF;
			spi_out_msg_buffer.data[offset + 4] = (out_msg->contents.cur_parameter.type)      & 0xFF;

			spi_out_msg_buffer.data[offset + 5] = (out_msg->contents.cur_parameter.value >> 24) & 0xFF;
			spi_out_msg_buffer.data[offset + 6] = (out_msg->contents.cur_parameter.value >> 16) & 0xFF;
			spi_out_msg_buffer.data[offset + 7] = (out_msg->contents.cur_parameter.value >> 8)  & 0xFF;
			spi_out_msg_buffer.data[offset + 8] = (out_msg->contents.cur_parameter.value)       & 0xFF;
		}

		if (out_msg->type == MSG_DECIMAL_DEBUG_DUMP)
		{
			for (i16 = 0 ; i16 < out_msg->length ; i16++)
				spi_out_msg_buffer.data[offset + 3 + i16] = out_msg->contents.decimal_debug_dump.data[i16];
		}

		spi_out_msg_buffer.data[offset + out_msg->length + 3] = crc(spi_out_msg_buffer.data, offset + out_msg->length+3);

		spi_out_msg_buffer.write_cursor = offset + out_msg->length + 4;

		// Falls noch ein Byte vom Secondary abgeholt würde (was nicht passieren sollte) dann ist es auf diese Weise FF, was
		// ich im LA dann sehen kann.
		spi_out_msg_buffer.data[spi_out_msg_buffer.write_cursor] = 0xFF;

	}

	spi_sending_state = SPI_SENDING_PENDING;
}
