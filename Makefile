# make test      unit tests for the ring buffer and every demo (with address and UB sanitizers)
# make bench     ring buffer vs linked-list queue, and the UART burst simulation -> results/
# make avr       build the ATmega328P self-test and run it in simavr (needs gcc-avr, simavr)
CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Wpedantic -O2 -g
SAN     := -fsanitize=address,undefined -fno-omit-frame-pointer

DEMOS := $(wildcard demos/*.c)

.PHONY: all test bench avr sketch clean sync-check
all: test

build:
	mkdir -p build

build/test_ring_buffer: tests/test_ring_buffer.c src/ring_buffer.c src/list_queue.c | build
	$(CC) $(CFLAGS) -pthread $^ -o $@

build/test_demos: tests/test_demos.c $(DEMOS) | build
	$(CC) $(CFLAGS) $(SAN) -pthread -Idemos $^ -o $@ -lm

build/bench: bench/bench.c src/ring_buffer.c src/list_queue.c | build
	$(CC) -std=c11 -O2 -Wall $^ -o $@

build/uart_burst: bench/uart_burst.c src/ring_buffer.c | build
	$(CC) -std=c11 -O2 -Wall $^ -o $@

test: build/test_ring_buffer build/test_demos sync-check
	./build/test_ring_buffer
	./build/test_demos

bench: build/bench build/uart_burst
	./build/bench > results/bench.csv && cat results/bench.csv
	./build/uart_burst > results/uart_burst.csv && head -5 results/uart_burst.csv

# the Arduino sketch carries its own copy of the ring buffer (the IDE only compiles the sketch folder)
sync-check:
	cmp src/ring_buffer.c arduino/uart_ring_buffer/ring_buffer.c
	cmp src/ring_buffer.h arduino/uart_ring_buffer/ring_buffer.h

avr:
	avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -Isrc avr/selftest.c src/ring_buffer.c -o build/selftest.elf
	avr-size build/selftest.elf
	simavr -m atmega328p -f 16000000 build/selftest.elf 2>&1 | sed -e "s/\x1b\[[0-9;]*m//g" -e "s/\.\.$$//" | grep -v "^Loaded" | tee results/avr_selftest.txt

clean:
	rm -rf build

# compile the Arduino sketch with avr-g++ (no Arduino core needed: the sketch only uses avr-libc)
sketch: | build
	avr-g++ -mmcu=atmega328p -DF_CPU=16000000UL -Os -Wall -x c++ arduino/uart_ring_buffer/uart_ring_buffer.ino -x none avr/arduino_main.cpp -x c arduino/uart_ring_buffer/ring_buffer.c -Iarduino/uart_ring_buffer -o build/sketch.elf
	avr-size build/sketch.elf
