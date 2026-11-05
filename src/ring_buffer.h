/*
 * ring_buffer.h - fixed-size circular FIFO of bytes for a UART.
 *
 * One writer (the UART receive interrupt) and one reader (the main loop) may
 * use the same buffer at the same time without disabling interrupts:
 *   - only the writer changes head, only the reader changes tail;
 *   - head and tail are 8-bit and free-running (they wrap at 256), so on an
 *     8-bit AVR every read or write of them is a single, atomic instruction;
 *   - the capacity is a power of two (2..128), so the slot is index & mask
 *     and the number of bytes stored is always (uint8_t)(head - tail).
 * Because the indices are free-running, every slot can be used: there is no
 * "one slot always empty" rule as in the textbook array queue.
 *
 * Ordering: the writer stores the byte before it moves head, the reader takes
 * the byte before it moves tail. The storage is volatile too, so the compiler
 * keeps those stores in that order. That is enough on a single-core AVR and on
 * x86; on a multi-core ARM the indices would need C11 release/acquire atomics.
 */
#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    volatile uint8_t *data;   /* storage, capacity bytes (volatile: see below) */
    uint8_t mask;             /* capacity - 1                             */
    volatile uint8_t head;    /* next slot to write (writer only)         */
    volatile uint8_t tail;    /* next slot to read  (reader only)         */
    volatile uint16_t dropped;/* bytes rejected because the buffer was full */
} ring_buffer_t;

/* capacity must be a power of two from 2 to 128; returns false otherwise */
bool rb_init(ring_buffer_t *rb, uint8_t *storage, uint8_t capacity);

bool rb_put(ring_buffer_t *rb, uint8_t byte);      /* false (and dropped++) when full */
bool rb_get(ring_buffer_t *rb, uint8_t *byte);     /* false when empty                */
bool rb_peek(const ring_buffer_t *rb, uint8_t *byte);
uint8_t rb_read(ring_buffer_t *rb, uint8_t *dst, uint8_t max); /* up to max bytes */

uint8_t rb_count(const ring_buffer_t *rb);
uint8_t rb_capacity(const ring_buffer_t *rb);
uint8_t rb_free(const ring_buffer_t *rb);
bool rb_is_empty(const ring_buffer_t *rb);
bool rb_is_full(const ring_buffer_t *rb);
void rb_clear(ring_buffer_t *rb);                  /* reader side: discard all */

#ifdef __cplusplus
}
#endif
#endif
