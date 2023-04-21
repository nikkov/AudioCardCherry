#include "ring_buf.h"

ring_buf* ring_buf_init(ring_buf *buff, uint8_t *staticbuff, uint32_t len) {
	buff->buff = staticbuff;
	buff->size = len;
	ring_buf_reset(buff);
	return buff;
}

void ring_buf_reset(ring_buf *buff) {
	buff->start = 0;
	buff->end = 0;
	buff->count = 0;
	buff->remain = buff->size;
}

uint32_t ring_buf_write(ring_buf *buff, const uint8_t *src, uint32_t len) {
	// Assume everything will be queued
	uint32_t ret = 0;
	uint32_t end = buff->end;
	uint32_t size = buff->size;

	// No space
	if (ring_buf_full(buff)) 
        return len;

	// Not enough space in the buffer
	if (buff->remain < len) {
		ret = len - buff->remain;
		len = buff->remain;
	}

	// Update counters before len is modified in the for-loop
	buff->count += len;
	buff->remain -= len;

	// Loop to end of data or end of buffer, whichever was decided above
	for (; len > 0; len--) {
		// Copy byte
		buff->buff[end++] = *(src++);
		if (end >= size)
			end = 0;
	}

	// Update buffer with new end value
	buff->end = end;
	return ret;
}

uint32_t ring_buf_read(ring_buf *buff, uint8_t *dst, uint32_t len) {
	uint32_t ret;
	uint32_t start = buff->start;
	uint32_t size = buff->size;

	// Nothing to read
	if (ring_buf_empty(buff)) 
        return 0;

	// If buffer does not contain len bytes then truncate
	// the amount dequeued to that of what's in the buffer
	if (len > buff->count) 
        len = buff->count;

	// Number of bytes dequeues into dest
	ret = len;

	// Update counters before len is modified in the for-loop
	buff->count -= len;
	buff->remain += len;

	// copy bytes
	for (; len > 0; len--) {
		*(dst++) = buff->buff[start++];
		if (start >= size)
			start = 0;
	}

	// Update buffer with new start value
	buff->start = start;
	return ret;
}

void dbl_buffer_reset(dbl_buffer* buf) {
    buf->buffer_pos = 0;
    buf->count = 0;
}

void dbl_buffer_init(dbl_buffer* buf, uint8_t *staticbuff, uint32_t halflen) {
    buf->buffer = staticbuff;
    buf->halfsize = halflen;
    dbl_buffer_reset(buf);
}
