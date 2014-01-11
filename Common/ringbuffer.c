#include "headers.h"

bool ringbuffer_empty(buffer *b)
{
	return (b->read_cursor == b->write_cursor && b->fill_level != b->size);
	// return (b->read_cursor == b->write_cursor);
}

bool ringbuffer_full(buffer *b)
{
	return (b->fill_level == b->size);
}

char ringbuffer_shift(buffer *b)
{
	char r;

	if (b->fill_level == 0)
		return 0;

	r = b->data[b->read_cursor];
	b->data[b->read_cursor] = 0xFF; // Damit man leeren Puffer beim Debuggen sieht

	b->read_cursor++;
	if (b->read_cursor == b->size)
		b->read_cursor = 0;
	b->fill_level--;

	return r;
}

bool ringbuffer_push(buffer *b, char c)
{
	if (b->fill_level == b->size)
		return false;

	b->data[b->write_cursor] = c;
	b->write_cursor++;
	if (b->write_cursor == b->size)
		b->write_cursor = 0;
	b->fill_level++;
	return true;
}

void ringbuffer_clear(buffer *b)
{
	b->read_cursor = 0;
	b->write_cursor = 0;
	b->fill_level = 0;
}