#include "ring_buffer.h"

// Helper functions
static bool ring_buffer_is_power_of_two(uint32_t value){
    return ((value >= 2U) && ((value & (value -1U)) == 0U));
}

static bool ring_buffer_is_valid(const ring_buffer_t *rb){
    return ((rb != NULL) && (rb->data != NULL) && 
           (ring_buffer_is_power_of_two(rb->size)) && 
           (rb->mask == (rb->size -1))
    );
}


ring_buffer_status_t  ring_buffer_init(ring_buffer_t *rb, uint8_t *storage, uint32_t size)
{
    if ((rb == NULL) || (storage == NULL)) {
        return RINGBUFFER_ERR_INV_ARG;
    }

    if (!ring_buffer_is_power_of_two(size)) {
        return RINGBUFFER_ERR_INV_ARG;
    }

    rb->data = storage;
    rb->size = size;
    rb->mask = size - 1;
    rb->head = 0;
    rb->tail = 0;

    return RINGBUFFER_OK;
}

ring_buffer_status_t ring_buffer_put(ring_buffer_t *rb, uint8_t src)
{
    if (!ring_buffer_is_valid(rb)) {
        return RINGBUFFER_ERR_INV_ARG;
    }

    uint32_t next = (rb->head + 1) & rb->mask;

    if (next == rb->tail) {
        return RINGBUFFER_ERR_FULL;
    }

    rb->data[rb->head] = src;
    rb->head = next;

    return RINGBUFFER_OK;
}

ring_buffer_status_t ring_buffer_get(ring_buffer_t *rb, uint8_t *dst)
{
    if (!ring_buffer_is_valid(rb) || !dst) {
        return RINGBUFFER_ERR_INV_ARG;
    }
    
    uint32_t next = (rb->tail + 1) & rb->mask;

    if (rb->head == rb->tail) {
        return RINGBUFFER_ERR_EMPTY;
    }
    
    *dst = rb->data[rb->tail];
    rb->tail = next;

    return RINGBUFFER_OK;
}

ring_buffer_status_t rb_write(ring_buffer_t *rb, const uint8_t *src, uint32_t length, uint32_t *written)
{
    if (!rb || !src || !written) {
        return RINGBUFFER_ERR_INV_ARG;
    }
    *written = 0;

    if (!ring_buffer_is_valid(rb)) {
        return RINGBUFFER_ERR_INV_ARG;
    }

    while (*written < length) {
        ring_buffer_status_t res = ring_buffer_put(rb, src[*written]);
        if (res != RINGBUFFER_OK) {
            return res;
        }
        (*written)++;
    }
    return RINGBUFFER_OK;
}

ring_buffer_status_t rb_read(ring_buffer_t *rb, uint8_t *dst, uint32_t length, uint32_t *received)
{
    if (!rb || !dst || !received) {
        return RINGBUFFER_ERR_INV_ARG;
    }
    *received = 0;

    if (!ring_buffer_is_valid(rb)) {
        return RINGBUFFER_ERR_INV_ARG;
    }

    while (*received < length) {
        ring_buffer_status_t res = ring_buffer_get(rb, &dst[*received]);
        if (res != RINGBUFFER_OK) {
            return res;
        }
        (*received)++;
    }
    return RINGBUFFER_OK;
}

ring_buffer_status_t ring_buffer_clear(ring_buffer_t *rb)
{
    if (!ring_buffer_is_valid(rb)) {
        return RINGBUFFER_ERR_INV_ARG;
    }
    rb->head = 0;
    rb->tail = 0;
    return RINGBUFFER_OK;
}

bool ring_buffer_is_empty(const ring_buffer_t *rb)
{
    if (!ring_buffer_is_valid(rb)) {
        return true;
    }
    return (rb->head == rb->tail);
}

bool ring_buffer_is_full(const ring_buffer_t *rb)
{
    if (!ring_buffer_is_valid(rb)) {
        return false;
    }
    return (((rb->head +1) & rb->mask) == rb->tail);
}

uint32_t ring_buffer_free(const ring_buffer_t *rb)
{
    if (!ring_buffer_is_valid(rb)) {
        return 0;
    }
    return (rb->size - 1) - ring_buffer_count(rb);
}

uint32_t ring_buffer_count(const ring_buffer_t *rb)
{
    if (!ring_buffer_is_valid(rb)) {
        return 0;
    }
    return ((rb->head - rb->tail) & rb->mask);
}

