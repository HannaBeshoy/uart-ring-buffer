#define _POSIX_C_SOURCE 199309L
/*
 * bench.c - ring buffer vs linked-list queue on this PC.
 * Prints CSV: structure,operations,ns_per_op,bytes_per_stored_byte
 */
#include <stdio.h>
#include <time.h>
#include "../src/list_queue.h"
#include "../src/ring_buffer.h"

static double now_ns(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec * 1e9 + t.tv_nsec;
}

int main(void)
{
    enum { OPS = 20000000, BATCH = 32 };
    static uint8_t mem[64];
    ring_buffer_t rb;
    list_queue_t q;
    volatile uint8_t sink = 0;
    uint8_t b;

    rb_init(&rb, mem, 64);
    double t0 = now_ns();
    for (long i = 0; i < OPS / BATCH; i++) {
        for (int k = 0; k < BATCH; k++) rb_put(&rb, (uint8_t)k);
        for (int k = 0; k < BATCH; k++) { rb_get(&rb, &b); sink ^= b; }
    }
    double ring_ns = (now_ns() - t0) / (2.0 * OPS / BATCH * BATCH);

    lq_init(&q);
    t0 = now_ns();
    for (long i = 0; i < OPS / BATCH; i++) {
        for (int k = 0; k < BATCH; k++) lq_put(&q, (uint8_t)k);
        for (int k = 0; k < BATCH; k++) { lq_get(&q, &b); sink ^= b; }
    }
    double list_ns = (now_ns() - t0) / (2.0 * OPS / BATCH * BATCH);

    printf("structure,operations,ns_per_op,bytes_per_stored_byte\n");
    printf("ring_buffer,%d,%.2f,%.2f\n", OPS, ring_ns, 1.0);
    /* a node holds 1 byte of data and an 8-byte pointer, padded to 16; malloc adds about 16 more */
    printf("linked_list_queue,%d,%.2f,%zu\n", OPS, list_ns, sizeof(lq_node_t) + 16);
    /* on the ATmega328P (2-byte pointers, 2-byte malloc header): 1 + 2 + 2 = 5 bytes per byte */
    printf("linked_list_queue_avr,0,,%d\n", 5);
    return sink == 255 ? 1 : 0;
}
