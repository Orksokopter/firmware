typedef struct {
	unsigned char *data;
        int size;
	int read_cursor;
	int write_cursor;
        int fill_level;
} buffer;

extern bool ringbuffer_empty(buffer *b);
extern bool ringbuffer_full(buffer *b);
extern char ringbuffer_shift(buffer *b);
extern bool ringbuffer_push(buffer *b, char c);
extern void ringbuffer_clear(buffer *b);

