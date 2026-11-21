#include <stdio.h>
#include <assert.h>
#include <stdint.h>

#include "ring_buffer.h"

#define BUFFER_SIZE   8
static ring_buffer_t rb;
static uint8_t buffer[BUFFER_SIZE];

static void reset_buffer(void)
{
    assert(ring_buffer_clear(&rb) == RINGBUFFER_OK);
}

static void test_init(void)
{
    assert(ring_buffer_init(&rb, buffer, BUFFER_SIZE) == RINGBUFFER_OK);
    assert(ring_buffer_is_empty(&rb));
    assert(!ring_buffer_is_full(&rb));
    assert(ring_buffer_count(&rb) == 0U);

    assert(ring_buffer_init(NULL, buffer, BUFFER_SIZE) == RINGBUFFER_ERR_INV_ARG);
    assert(ring_buffer_init(&rb, NULL, BUFFER_SIZE) == RINGBUFFER_ERR_INV_ARG);
    assert(ring_buffer_init(&rb, buffer, 7U) == RINGBUFFER_ERR_INV_ARG);

    printf("test_init: PASS\n");
}

static void test_put_get(void)
{
    uint8_t value;

    assert(ring_buffer_put(&rb, 0x00U) == RINGBUFFER_OK);
    assert(ring_buffer_put(&rb, 0xFFU) == RINGBUFFER_OK);

    assert(ring_buffer_get(&rb, &value) == RINGBUFFER_OK);
    assert(value == 0x00U);

    assert(ring_buffer_get(&rb, &value) == RINGBUFFER_OK);
    assert(value == 0xFFU);

    assert(ring_buffer_is_empty(&rb));

    printf("test_put_get: PASS\n");
}

static void test_full_empty(void)
{
    uint8_t value;

    assert(ring_buffer_is_empty(&rb));

    for (uint8_t i = 0U; i < BUFFER_SIZE - 1U; i++)
    {
        assert(ring_buffer_put(&rb, i) == RINGBUFFER_OK);
    }

    assert(ring_buffer_is_full(&rb));
    assert(ring_buffer_count(&rb) == BUFFER_SIZE - 1U);
    assert(ring_buffer_put(&rb, 0xAAU) == RINGBUFFER_ERR_FULL);

    for (uint8_t i = 0U; i < BUFFER_SIZE - 1U; i++)
    {
        assert(ring_buffer_get(&rb, &value) == RINGBUFFER_OK);
        assert(value == i);
    }

    assert(ring_buffer_is_empty(&rb));
    assert(ring_buffer_get(&rb, &value) == RINGBUFFER_ERR_EMPTY);

    printf("test_full_empty: PASS\n");
}

static void test_wrap_around(void)
{
    uint8_t value;

    for (uint8_t i = 0U; i < BUFFER_SIZE - 1U; i++)
    {
        assert(ring_buffer_put(&rb, i) == RINGBUFFER_OK);
    }

    for (uint8_t i = 0U; i < 4U; i++)
    {
        assert(ring_buffer_get(&rb, &value) == RINGBUFFER_OK);
        assert(value == i);
    }

    for (uint8_t i = 10U; i < 14U; i++)
    {
        assert(ring_buffer_put(&rb, i) == RINGBUFFER_OK);
    }

    /* Remaining sequence must be 4, 5, 6, 10, 11, 12, 13 */
    for (uint8_t i = 4U; i < 7U; i++)
    {
        assert(ring_buffer_get(&rb, &value) == RINGBUFFER_OK);
        assert(value == i);
    }

    for (uint8_t i = 10U; i < 14U; i++)
    {
        assert(ring_buffer_get(&rb, &value) == RINGBUFFER_OK);
        assert(value == i);
    }

    assert(ring_buffer_is_empty(&rb));

    printf("test_wrap_around: PASS\n");
}

static void test_write_read(void)
{
    uint8_t tx[] = { 0x00U, 0x11U, 0x22U, 0xFFU };
    uint8_t rx[sizeof(tx)] = { 0U };

    uint32_t written = 0U;
    uint32_t received = 0U;

    assert(rb_write(&rb, tx, sizeof(tx), &written) == RINGBUFFER_OK);
    assert(written == sizeof(tx));

    assert(rb_read(&rb, rx, sizeof(rx), &received) == RINGBUFFER_OK);
    assert(received == sizeof(rx));

    for (uint32_t i = 0U; i < sizeof(tx); i++)
    {
        assert(rx[i] == tx[i]);
    }

    printf("test_write_read: PASS\n");
}

static void test_partial_write(void)
{
    uint8_t tx[] = { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U };
    uint32_t written = 0U;

    assert(rb_write(&rb, tx, sizeof(tx), &written) == RINGBUFFER_ERR_FULL);
    assert(written == BUFFER_SIZE - 1U);

    printf("test_partial_write: PASS\n");
}

static void test_partial_read(void)
{
    reset_buffer();

    uint8_t tx[] = { 10U, 20U, 30U };
    uint8_t rx[5] = { 0U };
    uint32_t written = 0U;
    uint32_t received = 0U;

    assert(rb_write(&rb, tx, sizeof(tx), &written) == RINGBUFFER_OK);
    assert(written == sizeof(tx));

    assert(rb_read(&rb, rx, 5U, &received) == RINGBUFFER_ERR_EMPTY);
    assert(received == sizeof(tx));

    assert(rx[0] == 10U);
    assert(rx[1] == 20U);
    assert(rx[2] == 30U);

    printf("test_partial_read: PASS\n");
}

static void test_clear(void)
{

    assert(ring_buffer_put(&rb, 0x55U) == RINGBUFFER_OK);
    assert(!ring_buffer_is_empty(&rb));

    assert(ring_buffer_clear(&rb) == RINGBUFFER_OK);

    assert(ring_buffer_is_empty(&rb));
    assert(ring_buffer_count(&rb) == 0U);

    printf("test_clear: PASS\n");
}

int main(void)
{
    test_init();

    test_put_get();
    test_full_empty();
    test_wrap_around();
    test_write_read();
    test_partial_write();
    test_partial_read();
    test_clear();

    printf("\nAll Ring Buffer tests passed!\n");

    return 0;
}