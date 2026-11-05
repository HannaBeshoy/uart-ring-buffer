/* Unit tests for ring_buffer.c: run with `make test`. */
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "check.h"
#include "../src/list_queue.h"
#include "../src/ring_buffer.h"

static uint8_t mem[128];

TEST(init_rejects_bad_capacity)
{
    ring_buffer_t rb;
    CHECK(!rb_init(&rb, mem, 0));
    CHECK(!rb_init(&rb, mem, 1));
    CHECK(!rb_init(&rb, mem, 3));
    CHECK(!rb_init(&rb, mem, 100));
    CHECK(!rb_init(&rb, NULL, 16));
    CHECK(!rb_init(NULL, mem, 16));
    CHECK(rb_init(&rb, mem, 2));
    CHECK(rb_init(&rb, mem, 128));
}

TEST(new_buffer_is_empty)
{
    ring_buffer_t rb;
    uint8_t b = 0xAA;
    rb_init(&rb, mem, 16);
    CHECK(rb_is_empty(&rb));
    CHECK(!rb_is_full(&rb));
    CHECK(rb_count(&rb) == 0);
    CHECK(rb_free(&rb) == 16);
    CHECK(!rb_get(&rb, &b));
    CHECK(!rb_peek(&rb, &b));
    CHECK(b == 0xAA);                      /* untouched on failure */
}

TEST(fifo_order)
{
    ring_buffer_t rb;
    uint8_t b;
    rb_init(&rb, mem, 8);
    for (uint8_t i = 1; i <= 5; i++) CHECK(rb_put(&rb, i));
    CHECK(rb_count(&rb) == 5);
    CHECK(rb_peek(&rb, &b) && b == 1);
    CHECK(rb_count(&rb) == 5);             /* peek does not remove */
    for (uint8_t i = 1; i <= 5; i++) CHECK(rb_get(&rb, &b) && b == i);
    CHECK(rb_is_empty(&rb));
}

TEST(full_uses_every_slot_and_drops_newest)
{
    ring_buffer_t rb;
    uint8_t b;
    rb_init(&rb, mem, 8);
    for (uint8_t i = 0; i < 8; i++) CHECK(rb_put(&rb, i));
    CHECK(rb_is_full(&rb));
    CHECK(rb_free(&rb) == 0);
    CHECK(!rb_put(&rb, 99));
    CHECK(!rb_put(&rb, 100));
    CHECK(rb.dropped == 2);
    for (uint8_t i = 0; i < 8; i++) CHECK(rb_get(&rb, &b) && b == i);   /* old data intact */
    CHECK(!rb_get(&rb, &b));
}

TEST(wrap_around_many_times)
{
    /* 10,000 bytes through a 4-byte buffer: the slot index wraps 2,500
       times and the 8-bit head and tail wrap past 255 about 39 times */
    ring_buffer_t rb;
    uint8_t b;
    int ok = 1;
    rb_init(&rb, mem, 4);
    for (int i = 0; i < 10000; i++) {
        ok &= rb_put(&rb, (uint8_t)i);
        if (i % 3 == 2) {                   /* keep 1 to 3 bytes in flight */
            ok &= rb_get(&rb, &b) && b == (uint8_t)(i - 2);
            ok &= rb_get(&rb, &b) && b == (uint8_t)(i - 1);
            ok &= rb_get(&rb, &b) && b == (uint8_t)i;
        }
    }
    CHECK(ok);
    CHECK(rb.dropped == 0);
}

TEST(full_across_index_wrap)
{
    /* fill completely when head is near 255, where head < tail numerically */
    ring_buffer_t rb;
    uint8_t b;
    rb_init(&rb, mem, 128);
    rb.head = rb.tail = 250;
    for (int i = 0; i < 128; i++) CHECK(rb_put(&rb, (uint8_t)i));
    CHECK(rb.head == (uint8_t)(250 + 128));
    CHECK(rb_is_full(&rb) && rb_count(&rb) == 128);
    CHECK(!rb_put(&rb, 0));
    for (int i = 0; i < 128; i++) CHECK(rb_get(&rb, &b) && b == (uint8_t)i);
    CHECK(rb_is_empty(&rb));
}

TEST(bulk_read_and_clear)
{
    ring_buffer_t rb;
    uint8_t out[10];
    rb_init(&rb, mem, 16);
    for (uint8_t i = 0; i < 6; i++) rb_put(&rb, (uint8_t)('a' + i));
    CHECK(rb_read(&rb, out, 4) == 4 && memcmp(out, "abcd", 4) == 0);
    CHECK(rb_read(&rb, out, 10) == 2 && memcmp(out, "ef", 2) == 0);
    for (uint8_t i = 0; i < 6; i++) rb_put(&rb, i);
    rb_clear(&rb);
    CHECK(rb_is_empty(&rb) && rb_free(&rb) == 16);
}

TEST(random_ops_match_linked_list_model)
{
    /* 200,000 random puts and gets; the linked-list queue (capped at the same
       capacity) is the reference model */
    ring_buffer_t rb;
    list_queue_t q;
    uint8_t a, b;
    int ok = 1;
    unsigned dropped = 0;
    srand(12345);
    rb_init(&rb, mem, 32);
    lq_init(&q);
    for (int i = 0; i < 200000; i++) {
        if (rand() % 100 < 55) {
            uint8_t v = (uint8_t)rand();
            bool put = rb_put(&rb, v);
            if (lq_count(&q) < 32) { ok &= put; lq_put(&q, v); }
            else { ok &= !put; dropped++; }
        } else {
            bool got = rb_get(&rb, &a);
            bool want = lq_get(&q, &b);
            ok &= got == want && (!got || a == b);
        }
        ok &= rb_count(&rb) == lq_count(&q);
    }
    CHECK(ok);
    CHECK(rb.dropped == (uint16_t)dropped);
    lq_free(&q);
}

/* one producer thread (plays the UART interrupt), one consumer thread (the main loop) */
#define N_BYTES 2000000
static ring_buffer_t shared;
static uint8_t shared_mem[64];

static void *producer(void *arg)
{
    (void)arg;
    for (long i = 0; i < N_BYTES; i++)
        while (!rb_put(&shared, (uint8_t)(i * 7 + 3))) { }   /* spin while full */
    return NULL;
}

TEST(two_threads_no_loss_no_reorder)
{
    pthread_t t;
    long errors = 0;
    rb_init(&shared, shared_mem, 64);
    pthread_create(&t, NULL, producer, NULL);
    for (long i = 0; i < N_BYTES; i++) {
        uint8_t b;
        while (!rb_get(&shared, &b)) { }
        if (b != (uint8_t)(i * 7 + 3)) errors++;
    }
    pthread_join(t, NULL);
    CHECK(errors == 0);
    CHECK(rb_is_empty(&shared));
}

TEST(list_queue_basics)
{
    list_queue_t q;
    uint8_t v;
    lq_init(&q);
    CHECK(!lq_get(&q, &v));
    for (int i = 0; i < 1000; i++) CHECK(lq_put(&q, (uint8_t)i));
    CHECK(lq_count(&q) == 1000);
    for (int i = 0; i < 1000; i++) CHECK(lq_get(&q, &v) && v == (uint8_t)i);
    CHECK(lq_count(&q) == 0 && q.first == NULL && q.last == NULL);
}

int main(void)
{
    RUN(init_rejects_bad_capacity);
    RUN(new_buffer_is_empty);
    RUN(fifo_order);
    RUN(full_uses_every_slot_and_drops_newest);
    RUN(wrap_around_many_times);
    RUN(full_across_index_wrap);
    RUN(bulk_read_and_clear);
    RUN(random_ops_match_linked_list_model);
    RUN(two_threads_no_loss_no_reorder);
    RUN(list_queue_basics);
    return REPORT();
}
