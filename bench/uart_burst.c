/*
 * uart_burst.c - how big must the receive buffer be?
 *
 * A sender streams bytes at 115200 baud (8N1: 10 bits per byte, one byte every
 * 86.8 us). The main loop normally empties the buffer at once, but every
 * 100 ms it is busy for a while (writing to an SD card, updating a display).
 * The simulation runs the real ring_buffer.c in 1 us steps for 10 seconds and
 * counts the bytes lost for each buffer size and busy time.
 * Prints CSV: capacity,busy_ms,bytes_sent,bytes_dropped,max_fill
 */
#include <stdio.h>
#include "../src/ring_buffer.h"

int main(void)
{
    const double byte_us = 1e6 / (115200 / 10.0);           /* 86.8 us */
    const int caps[] = { 8, 16, 32, 64, 128 };
    const double busy_ms[] = { 0.5, 1, 2, 5, 10, 12 };
    static uint8_t mem[128];

    printf("capacity,busy_ms,bytes_sent,bytes_dropped,max_fill\n");
    for (unsigned c = 0; c < sizeof caps / sizeof *caps; c++)
        for (unsigned d = 0; d < sizeof busy_ms / sizeof *busy_ms; d++) {
            ring_buffer_t rb;
            rb_init(&rb, mem, (uint8_t)caps[c]);
            double next_byte = 0;
            long sent = 0;
            int max_fill = 0;
            for (long t = 0; t < 10000000; t++) {               /* 10 s in 1 us steps */
                if (t >= next_byte) { rb_put(&rb, (uint8_t)sent); sent++; next_byte += byte_us; }
                if (rb_count(&rb) > max_fill) max_fill = rb_count(&rb);
                long in_cycle = t % 100000;                     /* 100 ms cycle */
                int busy = in_cycle < (long)(busy_ms[d] * 1000);
                uint8_t b;
                if (!busy) while (rb_get(&rb, &b)) { }          /* main loop drains the buffer */
            }
            printf("%d,%g,%ld,%u,%d\n", caps[c], busy_ms[d], sent, rb.dropped, max_fill);
        }
    return 0;
}
