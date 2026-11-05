// Stand-in for the Arduino core's main(), so `make sketch` can compile the sketch with plain avr-g++.
void setup(void);
void loop(void);
int main(void) { setup(); for (;;) loop(); }
