# Lab-day test sheet (10 minutes)

**Need:** Arduino Uno or Nano with its USB cable, the Arduino IDE on the laptop.

1. Open `arduino/uart_ring_buffer/uart_ring_buffer.ino`, select the board and port, upload.
2. Serial Monitor: 115200 baud, line ending "Newline". Expect: `uart_ring_buffer ready: ...`
3. Type `hello` → expect `> HELLO`.
4. Type `stats` → expect `received ..., dropped 0, max fill ... of 64`.
5. Overflow test: type `slow 20`, then paste a long text (about 500 characters, one line) and send it. Expect: LED 13 lights up, and `stats` now shows dropped > 0 and max fill 64 of 64.
6. Type `reset`, then `slow 0`, paste the same text again → `stats` shows dropped 0.

**Record:** a screenshot of the Serial Monitor after steps 4 and 5, and a photo of the board with LED 13 on. Write down the received, dropped and max-fill numbers.

Afterwards the README's lab-day box is ticked and the CV line changes from "simulated" to "tested on an Arduino".
