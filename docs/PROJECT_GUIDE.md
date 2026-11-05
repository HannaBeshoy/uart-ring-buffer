# Project guide

How the ring buffer works, why each decision was made, and how it maps to the Data Structures and Algorithms course (page numbers of *DataStructures-2026.pdf*).

## 1. The problem

At 115200 baud with 8 data bits, a start bit and a stop bit, a byte arrives every 10 / 115200 s = 86.8 µs. The ATmega328P's USART holds at most two received bytes in hardware, plus the one being shifted in: if the program does not read them in time, the next byte is lost (a data overrun). So the receive interrupt copies each byte into a buffer, and the main loop reads the buffer when it has time. The buffer is a **queue** (p. 69): first in, first out, every byte read exactly once.

## 2. Why an array, and why "cyclic"

The course gives three representations of a queue:

| Representation (pages) | Enqueue | Dequeue | Problem for a UART |
| --- | --- | --- | --- |
| linked list (p. 70) | O(1) | O(1) | `malloc` in an interrupt; 5 bytes of RAM per byte on the AVR |
| walking array queue (p. 76) | O(n) worst case | O(1) | shifting the data takes too long in an interrupt |
| cyclic array queue (p. 79) | O(1) | O(1) | none: this is the ring buffer |

The cyclic queue needs to tell "full" from "empty", because in both cases the read and write positions are equal. The lecture version keeps a counter (also in `demos/array_queue.c`). A counter would be written by both the interrupt and the main loop, which needs a lock. The ring buffer instead lets the indices run freely from 0 to 255 and wrap: `head - tail` (8-bit) is the number of stored bytes, 0 means empty and `capacity` means full. With a capacity that is a power of two, `index & mask` is the slot, and the wrap of the index at 256 lines up with the wrap of the slot.

## 3. Two writers would need a lock, one writer each does not

| Variable | Written by | Read by |
| --- | --- | --- |
| `head`, `data[head & mask]` | interrupt | main loop |
| `tail` | main loop | interrupt |
| `dropped` | interrupt | main loop (with interrupts off: it is 16-bit) |

The interrupt writes the byte, then publishes it by moving head. The main loop reads the byte, then releases the slot by moving tail. Each side reads the other's index once per call. The tests check this with two threads moving 2 million bytes (`two_threads_no_loss_no_reorder`) and with the AVR self-test, where Timer1 plays the UART.

## 4. Time and space complexity (pp. 23–43)

Every operation is O(1): a comparison, an array access, an increment. On the PC it takes 2.8 ns, against 12.6 ns for the linked list, which calls `malloc` and `free` for every byte. Space is O(capacity), allocated once. That is the space-time trade-off of p. 43: a fixed amount of memory buys constant, predictable time, which is what an interrupt needs.

## 5. Sizing it: a simulation

`bench/uart_burst.c` answers how big the buffer must be. Bytes arrive at 11.52 per millisecond; during a stall of *t* ms, 11.52·*t* bytes arrive with nobody reading. The buffer survives if it holds them all. The simulation runs the real code and confirms the rule: 64 bytes survive 5 ms (58 bytes peak) and lose 4.5 % at 10 ms.

## 6. The tests

- **Boundaries**: capacity 0, 1, 3 and 100 are refused; empty, full, one-before-full; the drop counter.
- **Wrap-around**: 10,000 bytes through a 4-byte buffer, and a full buffer when head has already wrapped past 255 but tail has not.
- **Model-based test**: 200,000 random puts and gets compared, step by step, with the linked-list queue capped at the same size.
- **Concurrency**: one producer thread, one consumer thread, 2 million bytes, every byte checked.
- **Sanitizers**: the demos run under AddressSanitizer and UndefinedBehaviorSanitizer, so a leak or an out-of-bounds access fails the build.

## 7. The lecture demos

Each topic of the course has a small implementation and tests that check the textbook claims, not only the output: the tuned bubble sort needs n − 1 comparisons on sorted data, insertion sort n(n − 1)/2 on reversed data, merge sort at most n·log₂ n, binary search at most log₂ n + 1 steps, an AVL tree with 1,023 increasing keys stays at height 10 while a plain search tree with only 200 becomes a 200-long chain.

The hash-table test reproduces the worked example of p. 107 (n = 10, β = 1.618, keys 5 10 7 21 13 8 2, linear probing, then deleting 7). The slide gives ĥ(13) = 3, but 13 · 1.618 = 21.034, whose fractional part gives ĥ(13) = 0. The test checks both tables, and that the key 8 is still found after 7 is deleted, which works only because a deleted slot is marked "deleted" and not "empty" (p. 106).

## Notes for the three tracks

- **Embedded systems and edge AI.** Every UART, SPI or ADC driver uses this structure; DMA controllers fill a ring buffer in hardware. An edge-AI sensor node feeds its model from one.
- **FPGA to semiconductors.** In hardware the same idea is an asynchronous FIFO: two pointers in two clock domains, compared through Gray code. The free-running-index trick is the same one: one extra bit tells full from empty.
- **Product security.** A receive buffer is where untrusted bytes first land. This one cannot overflow (the full check comes before the write) and it counts every dropped byte, so a flood of data shows up as a number instead of corrupting memory. A linked-list queue, by contrast, lets a sender exhaust the heap.
