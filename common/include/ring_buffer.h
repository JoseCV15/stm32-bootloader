#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    uint8_t *data;
    uint32_t size;
    uint32_t mask;
    volatile uint32_t head; // next write
    volatile uint32_t tail; // next read
} ring_buffer_t;

typedef enum {
    RINGBUFFER_OK = 0,
    RINGBUFFER_ERR_FULL,
    RINGBUFFER_ERR_EMPTY,
    RINGBUFFER_ERR_INV_ARG,
    RINGBUFFER_ERR_CAPACITY
} ring_buffer_status_t;

ring_buffer_status_t ring_buffer_init(ring_buffer_t *rb, uint8_t *storage, uint32_t size);
ring_buffer_status_t ring_buffer_put(ring_buffer_t *rb, uint8_t src);
ring_buffer_status_t ring_buffer_get(ring_buffer_t *rb, uint8_t *dst);
ring_buffer_status_t rb_write(ring_buffer_t *rb, const uint8_t *src, uint32_t length, uint32_t *written);
ring_buffer_status_t rb_read(ring_buffer_t *rb, uint8_t *dst, uint32_t length, uint32_t *received);

ring_buffer_status_t ring_buffer_clear(ring_buffer_t *rb);
bool ring_buffer_is_empty(const ring_buffer_t *rb);
bool ring_buffer_is_full(const ring_buffer_t *rb);
uint32_t ring_buffer_free(const ring_buffer_t *rb);
uint32_t ring_buffer_count(const ring_buffer_t *rb);



#endif  /* RING_BUFFER_H*/

