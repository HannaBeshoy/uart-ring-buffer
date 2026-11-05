/*
 * uart_ring_buffer.ino - the ring buffer as the real UART receive buffer of an
 * Arduino Uno or Nano (ATmega328P). Open the Serial Monitor at 115200 baud,
 * line ending "Newline".
 *
 * The sketch does not use Serial: it drives USART0 itself, and its receive
 * interrupt puts every byte into ring_buffer.c (Arduino's own Serial is not
 * linked when the sketch never names it, so the interrupt is free).
 *
 * Type a line and it comes back in capitals. Commands:
 *   stats      bytes received, bytes dropped, the highest fill level
 *   slow <ms>  make the main loop busy for <ms> after every line, to see the
 *              buffer overflow when you paste a long text (LED 13 lights up
 *              on the first dropped byte)
 *   reset      clear the counters
 */
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>
#include "ring_buffer.h"

static uint8_t rx_storage[64];
static ring_buffer_t rx;
static volatile uint32_t rx_total;
static volatile uint8_t max_fill;

ISR(USART_RX_vect)
{
    uint8_t b = UDR0;                      /* reading UDR0 clears the interrupt */
    if (rb_put(&rx, b)) rx_total++;
    else PORTB |= _BV(PB5);                /* LED on: a byte was lost */
    uint8_t fill = rb_count(&rx);
    if (fill > max_fill) max_fill = fill;
}

static void tx(char c)
{
    while (!(UCSR0A & _BV(UDRE0))) { }
    UDR0 = c;
}

static void print(const char *s) { while (*s) tx(*s++); }

static void print_num(uint32_t v)
{
    char buf[11];
    ultoa(v, buf, 10);
    print(buf);
}

static char line[80];
static uint8_t len;
static uint16_t slow_ms;

static void busy_ms(uint16_t ms) { while (ms--) _delay_ms(1); }

static void handle_line(void)
{
    line[len] = '\0';
    if (strcmp(line, "stats") == 0) {
        uint32_t total; uint16_t dropped; uint8_t fill;
        cli(); total = rx_total; dropped = rx.dropped; fill = max_fill; sei();
        print("received "); print_num(total);
        print(", dropped "); print_num(dropped);
        print(", max fill "); print_num(fill);
        print(" of "); print_num(rb_capacity(&rx)); print("\r\n");
    } else if (strncmp(line, "slow ", 5) == 0) {
        slow_ms = (uint16_t)atoi(line + 5);
        print("main loop now busy "); print_num(slow_ms); print(" ms per line\r\n");
    } else if (strcmp(line, "reset") == 0) {
        cli(); rx_total = 0; rx.dropped = 0; max_fill = 0; sei();
        PORTB &= ~_BV(PB5);
        print("counters cleared\r\n");
    } else {
        print("> ");
        for (uint8_t i = 0; i < len; i++) tx(line[i] >= 'a' && line[i] <= 'z' ? line[i] - 32 : line[i]);
        print("\r\n");
        busy_ms(slow_ms);
    }
    len = 0;
}

void setup(void)
{
    DDRB |= _BV(PB5);                      /* LED 13 */
    rb_init(&rx, rx_storage, sizeof rx_storage);
    UBRR0 = 16;                            /* 115200 baud at 16 MHz with double speed */
    UCSR0A = _BV(U2X0);
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);    /* 8N1 */
    UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
    sei();
    print("uart_ring_buffer ready: type a line, or stats / slow <ms> / reset\r\n");
}

void loop(void)
{
    uint8_t b;
    while (rb_get(&rx, &b)) {
        if (b == '\r') continue;
        if (b == '\n') { handle_line(); continue; }
        if (len < sizeof line - 1) line[len++] = (char)b;
    }
}
