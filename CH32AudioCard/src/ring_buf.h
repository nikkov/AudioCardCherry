#ifndef __RING_BUF_H_
#define __RING_BUF_H_
#include <stdint.h>

typedef struct
{
	// Buffer pointer
	uint8_t *buff;
	// Size of the buffer
	uint32_t size;
	// Start and end indices of valid data in the buffer
	// Read from start
	uint32_t start;
    // Write to end
	uint32_t end;
	// Number of bytes in the buffer
	uint32_t count;
	// Number of bytes remaining in the buffer
	uint32_t remain;
} ring_buf;

// Set buffer ptr and size
ring_buf* ring_buf_init(ring_buf *buff, uint8_t *staticbuff, uint32_t len);

// ring_buf_reset -- Reset the buffer to an empty buffer starting at the begin.
void ring_buf_reset(ring_buf *buff);

// True if the buffer is full
static inline uint32_t ring_buf_full(ring_buf *buff) {
	return buff->remain == 0;
}

// True if the buffer is empty
static inline uint32_t ring_buf_empty(ring_buf *buff) {
	return buff->count == 0;
}

// True if the buffer has space for size bytes (or less)
static inline uint32_t ring_buf_hasspace(ring_buf *buff, uint32_t size) {
	return buff->remain >= size;
}

/*
    ring_buf_write -- Write data to the circular buffer
    Since this implementation is not aware of how it is being used, then it must
     return if not all of the data can be written to the buffer and the caller must
     then deal with waiting for space to be made in the buffer.
    Returns the number of remaining bytes in dat that was NOT enqueued (this means
     that a return of zero is complete copy of dat into buff).
 */
uint32_t ring_buf_write(ring_buf *buff, const uint8_t *src, uint32_t len);

/*
    ring_buf_read -- Read an number of bytes into dest.
     It's your job to ensure dest has len bytes (or less) available to it.
    Return the number of bytes copied from buff to dest
 */
uint32_t ring_buf_read(ring_buf *buff, uint8_t *dst, uint32_t len);


// simple double buffer
typedef struct {
    // ptr to buffer size of 2 * halfsize
    uint8_t* buffer;
    // halfsize of common size
    uint32_t halfsize;
    // bit indicating which half of the buffer is in use
    volatile uint8_t buffer_pos;
    // usable buffer size in bytes
    volatile uint32_t count;
} dbl_buffer;

void dbl_buffer_reset(dbl_buffer* buf);

void dbl_buffer_init(dbl_buffer* buf, uint8_t *staticbuff, uint32_t halflen);


#endif // __RING_BUF_H_