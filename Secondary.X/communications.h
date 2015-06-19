// MPLAB X bietet die mir nicht im Autocomplete an, wenn ich die in der struct message definiere und da sie tatsächlich
// ja ohnehin global sind, definier ich sie hier global.

enum message_type {
	MSG_NOP = 0x000000,
	MSG_ACK = 0x000001,
	MSG_PROXY_MESSAGE = 0x000002,
	MSG_SET_PARAMETER = 0x000003,
	MSG_CUR_PARAMETER = 0x000004,
	MSG_GET_PARAMETER = 0x000005,
	MSG_PING = 0x000006,
	MSG_PONG = 0x000007,
	MSG_SET_LEDS = 0x000008,
	MSG_CLEAR_TO_SEND = 0x000009,
	MSG_BUFFER_REPORT = 0x00000a,
	MSG_OUT_BUFFER_FULL = 0x00000b,
	MSG_DO_ACCEL_CALIBRATION = 0x00000c,
	MSG_ACCEL_CALIBRATION_DONE = 0x00000d,
	MSG_DO_GYRO_CALIBRATION = 0x00000e,
	MSG_GYRO_CALIBRATION_DONE = 0x00000f,
	MSG_DECIMAL_DEBUG_DUMP = 0x000010,
	MSG_REQUEST_CONFIRMATION = 0x000011,
	MSG_CONFIRMATION = 0x000012
};

typedef struct {
	uint24_t number;
	enum message_type type;
	int length;
	union {
		struct {
			char data[16];
		} proxy_message;
		struct {
			uint16_t sequence_number;
		} ping;
		struct {
			uint16_t sequence_number;
		} pong;
		struct {
			uint24_t message_number;
		} request_confirmation;
		struct {
			uint24_t confirmed_message_number;
		} confirmation;
	} contents;
} message;

extern struct blah {
	int write_cursor;
	int read_cursor;
	bool ack_pending;
	uint16_t waiting_for_ack_timer;
	message messages[10];
} out_messages;

extern buffer ser_rec_buffer;
extern buffer ser_send_buffer;
extern buffer spi_rec_buffer;
extern buffer spi_send_buffer;
extern buffer spi_out_msg_buffer;
extern void init_communications(void);
extern void do_uart_sending(void);
extern void send_literal_uart_data(const char* data, size_t length);
extern void send_uart_data(const char* data, size_t length);
extern bool uart_is_sending(void);
extern void on_uart_data_received(char data);
extern void do_message_processing(void);
extern void process_message(void);
extern void clear_out_message_buffer(void);
extern void add_literal_out_message_data(const char* data, size_t length);
extern void add_out_message_data(const char* data, size_t length);
extern bool check_checksum(void);
extern void confirm_message_to_groundstation(uint24_t message_number);
extern void build_message_to_groundstation(message *out_msg);
extern void do_spi_comm(void);
extern void on_timer0_overflow(void);

extern bool spi_locked;
extern bool spi_message_locked;
