/*
 * selftest.c - the ring buffer on a real ATmega328P core, run in simavr.
 *
 * Timer1 interrupts every 87 us and pushes one byte, exactly like a UART
 * receiving at 115200 baud would. The main loop reads the bytes and checks
 * that they arrive in order. Phase 1: the main loop is always free, so no byte
 * may be lost. Phase 2: every 100 ms it is busy for 10 ms (like an SD-card
 * write), so a 64-byte buffer must overflow by about 115 - 64 = 51 bytes per
 * stall, and the bytes that do arrive must still be in order.
 * Results are printed on the real UART (simavr shows them on the console).
 *
 *   make avr
 */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdio.h>
#include <util/delay.h>
#include "ring_buffer.h"

static uint8_t storage[64];
static ring_buffer_t rx;
static volatile uint16_t sent;         /* bytes produced by the "UART" */

ISR(TIMER1_COMPA_vect)
{
    rb_put(&rx, (uint8_t)sent);
    sent++;
}

static int uart_putchar(char c, FILE *f)
{
    (void)f;
    if (c == '\n') uart_putchar('\r', f);
    while (!(UCSR0A & _BV(UDRE0))) { }
    UDR0 = c;
    return 0;
}
static FILE uart_out = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

static uint16_t sent_now(void)
{
    uint16_t s;
    cli(); s = sent; sei();            /* 16-bit read: protect it from the ISR */
    return s;
}

/* read everything that is there. Bytes may be missing (dropped while the
   buffer was full) but never swapped or repeated: each byte must be a little
   ahead of the previous one, and all the gaps together must equal the drops. */
static uint8_t expected;
static uint32_t received;
static uint16_t gaps, order_errors;

static void drain(void)
{
    uint8_t b;
    while (rb_get(&rx, &b)) {
        uint8_t gap = (uint8_t)(b - expected);
        if (gap > 128) order_errors++;     /* would mean a byte came back or out of order */
        gaps += gap;
        expected = (uint8_t)(b + 1);
        received++;
    }
}

/* drops after the last byte that did arrive (the buffer was full at the end) */
static uint16_t trailing(void) { return (uint8_t)((uint8_t)sent - expected); }

int main(void)
{
    UBRR0 = 16;                        /* 115200 baud at 16 MHz with U2X */
    UCSR0A = _BV(U2X0);
    UCSR0B = _BV(TXEN0);
    stdout = &uart_out;

    rb_init(&rx, storage, sizeof storage);
    TCCR1B = _BV(WGM12) | _BV(CS11);   /* CTC, clock / 8 = 2 MHz */
    OCR1A = 173;                       /* 174 counts = 87 us */
    TIMSK1 = _BV(OCIE1A);

    printf("ring buffer self-test on ATmega328P, 64-byte buffer, a byte every 87 us\n");
    sei();

    /* phase 1: 20,000 bytes, the main loop always free */
    while (sent_now() < 20000) drain();
    cli();
    drain();
    printf("phase 1: sent %u, received %lu, dropped %u, order errors %u\n",
           sent, (unsigned long)received, rx.dropped, order_errors);
    uint16_t p1_sent = sent, p1_drop = rx.dropped;
    uint32_t p1_recv = received;
    gaps = 0;
    sei();

    /* phase 2: 1 s with a 10 ms stall every 100 ms */
    received = 0;
    for (uint8_t cycle = 0; cycle < 10; cycle++) {
        _delay_ms(10);                 /* busy: nobody reads the buffer */
        uint16_t until = sent_now() + 1035;   /* about 90 ms of bytes */
        while ((int16_t)(sent_now() - until) < 0) drain();
    }
    cli();
    drain();
    uint16_t p2_sent = sent - p1_sent, p2_drop = rx.dropped - p1_drop;
    printf("phase 2: sent %u, received %lu, dropped %u, gaps %u + %u at the end, order errors %u\n",
           p2_sent, (unsigned long)received, p2_drop, gaps, trailing(), order_errors);

    int pass = p1_drop == 0 && p1_recv == p1_sent && order_errors == 0
               && received + p2_drop == p2_sent && p2_drop > 0
               && gaps + trailing() == p2_drop;
    printf("%s\n", pass ? "SELF-TEST PASS" : "SELF-TEST FAIL");
    while (!(UCSR0A & _BV(TXC0)) && !(UCSR0A & _BV(UDRE0))) { }
    _delay_ms(2);
    sleep_cpu();                       /* interrupts off + sleep: simavr stops here */
    for (;;) { }
}
