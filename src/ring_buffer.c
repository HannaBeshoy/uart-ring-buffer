#include "ring_buffer.h"

bool rb_init(ring_buffer_t *rb, uint8_t *storage, uint8_t capacity)
{
    if (rb == 0 || storage == 0) return false;
    if (capacity < 2 || capacity > 128 || (capacity & (capacity - 1)) != 0) return false;
    rb->data = storage;
    rb->mask = (uint8_t)(capacity - 1);
    rb->head = 0;
    rb->tail = 0;
    rb->dropped = 0;
    return true;
}

uint8_t rb_capacity(const ring_buffer_t *rb) { return (uint8_t)(rb->mask + 1); }

uint8_t rb_count(const ring_buffer_t *rb)
{
    /* both indices wrap at 256; the difference is right even across the wrap */
    return (uint8_t)(rb->head - rb->tail);
}

uint8_t rb_free(const ring_buffer_t *rb) { return (uint8_t)(rb_capacity(rb) - rb_count(rb)); }
bool rb_is_empty(const ring_buffer_t *rb) { return rb->head == rb->tail; }
bool rb_is_full(const ring_buffer_t *rb) { return rb_count(rb) == rb_capacity(rb); }

bool rb_put(ring_buffer_t *rb, uint8_t byte)
{
    uint8_t head = rb->head;                 /* own index: read once           */
    if ((uint8_t)(head - rb->tail) == (uint8_t)(rb->mask + 1)) {
        rb->dropped++;                       /* full: newest byte is lost      */
        return false;
    }
    rb->data[head & rb->mask] = byte;        /* store the byte first ...       */
    rb->head = (uint8_t)(head + 1);          /* ... then publish it            */
    return true;
}

bool rb_peek(const ring_buffer_t *rb, uint8_t *byte)
{
    uint8_t tail = rb->tail;
    if (rb->head == tail) return false;
    *byte = rb->data[tail & rb->mask];
    return true;
}

bool rb_get(ring_buffer_t *rb, uint8_t *byte)
{
    uint8_t tail = rb->tail;
    if (rb->head == tail) return false;
    *byte = rb->data[tail & rb->mask];       /* take the byte first ...        */
    rb->tail = (uint8_t)(tail + 1);          /* ... then free the slot         */
    return true;
}

uint8_t rb_read(ring_buffer_t *rb, uint8_t *dst, uint8_t max)
{
    uint8_t n = 0;
    while (n < max && rb_get(rb, &dst[n])) n++;
    return n;
}

void rb_clear(ring_buffer_t *rb) { rb->tail = rb->head; }
