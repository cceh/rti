#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define LED_MATRIX

#ifdef LED_STRIP
    #include "ledstrip.h"
#endif
#ifdef IR_LED
    #include "canon.h"
#endif

// Macros to allow use of Atmel pin names instead of Arduino's.

#define __P__(port,pin)    P##port##pin
#define __BV__(port,pin)   _BV(pin)

#define __PORT__(port,pin) PORT##port
#define __DD__(port,pin)   DDR##port
#define __PIN__(port,pin)  PIN##port

#define P(signal)    __P__(signal)
#define BV(signal)   __BV__(signal)

#define PORT(signal) __PORT__(signal)
#define DD(signal)   __DD__(signal)
#define PIN(signal)  __PIN__(signal)

#define INPUT_PULLUP(signal) __PORT__(signal) |= __BV__(signal)
#define OUTPUT(signal)       __DD__(signal)   |= __BV__(signal)

#define SBI(signal)          __PORT__(signal) |= __BV__(signal)
#define CBI(signal)          __PORT__(signal) &= ~(__BV__(signal))

#define READ(signal)         (__PORT__(signal) & __BV__(signal))

// Define port and pin of our signals

#define HAVE_FLASH_PIN     B,0  // From ring of the 3,5mm jack (jack presence detection)
#define TRIGGER_PIN        B,1  // To tip of the 2,5mm jack  (trigger the camera)
#define WAKEUP_PIN         B,2  // To ring of the 2,5mm jack (wakeup the camera)
#define READY_PIN          B,5  // To ready status LED (Arduino green LED / SCK)

#define START_PIN          D,2  // Start button (the user wants to start)
#define FLASH_PIN          D,3  // From tip of the 3,5mm jack  (the camera wants us to flash)

#define START_INT          INT0
#define FLASH_INT          INT1

#ifdef LED_MATRIX
    // The led matrix 595 driver
    // Consisting of one or more 595 shift registers.

    // How many LEDs do we have in how many columns?
    #define LED_COUNT   16
    #define LED_COLS    4

    // Where do the rows and column bits start in the 595 shift registers?
    // Depends on the number of 595's and the wiring of the board (prototype).
    #define MASK_595_COL 0x0100
    #define MASK_595_ROW 0x0001

    #define SER_IN_PIN  D,4
    #define SRCK_PIN    D,5
    #define RCK_PIN     D,6
    #define G_PIN       D,7
#endif

#ifdef LED_STRIP
    #define LED_STRIP_PIN       4
    #define LED_COUNT          10
    rgb_color colors[LED_COUNT];
#endif

volatile uint16_t index = 0;
volatile uint8_t  index_changed = 0;
volatile uint8_t  ready = 0;
volatile uint8_t  ready_changed = 0;
volatile unsigned long lastFlashTime = 0;
volatile unsigned long lastStartTime = 0;

volatile unsigned long _millis = 0;

uint16_t exposure = 20;  // 1s / 60

// We assume no camera comes close to 20 pics / sec.
// N.B. Flash signal on Canon EOS30D is low for ~33ms.
const unsigned long debounceDelay = 50;

unsigned long millis () {
	unsigned long m;
	uint8_t oldSREG = SREG;

	cli ();
	m = _millis;
	SREG = oldSREG;

	return m;
}

void start_camera () {
    SBI(WAKEUP_PIN);
    _delay_ms (100);
    SBI(TRIGGER_PIN);
    ready = 0;
    ready_changed = 1;
}

void stop_camera () {
    CBI(WAKEUP_PIN);
    CBI(TRIGGER_PIN);
    ready = 1;
    ready_changed = 1;
}

void initMs () {
    // Init the timer to interrupt once every 1ms.

    // Normal mode: WGM02, WGM01, WGM00 = 0, 0, 0
    TCCR0A = 0;

    // Enable timer overflow interrupt
	TIMSK0 |= _BV(TOIE0);

    // Set CS02, CS01, CS00 = 0, 1, 1 for a prescaler of 64
    // See section 15.9.2 of the ATmega328p Datasheet
    // Setting the clock source starts the timer
    TCCR0B = _BV(CS01) | _BV(CS00);
}

void setTimeout (uint16_t nth_of_second) {
    // CTC (Clear Timer on Compare Match) mode: WGM13, WGM12, WGM11, WGM10 = 0, 1, 0, 0
    TCCR1A = 0;
    TCNT1  = 0; // initialize counter value to 0
    // Set compare match register
    OCR1A = 15625 / nth_of_second; // = 16MHz / 1024 (prescaler) (must be <65536)
    // Enable output compare match interrupt
    TIMSK1 |= (1 << OCIE1A);
    // Set CS12, CS11, CS10 = 1, 0, 1 for a prescaler of 1024
    // See section 16.11.2 of the ATmega328p Datasheet
    // Setting the clock source starts the timer
    TCCR1B = _BV(WGM12) | _BV(CS12) | _BV(CS10);
}

#ifdef LED_MATRIX

void write595 (uint16_t value) {
    cli ();
    for (uint8_t i = 0; i < 16; ++i)  {
        CBI(SRCK_PIN);   // clk -> low
        if (value & 0x8000) {                 // data out
            SBI(SER_IN_PIN);
        } else {
            CBI(SER_IN_PIN);
        }
        SBI(SRCK_PIN); // clk -> high
        value <<= 1;
    }
    sei ();
    CBI(SER_IN_PIN);   // ser_in -> low
    CBI(SRCK_PIN);     // clk -> low
}

#endif

ISR (INT0_vect) { // Start
    if ((millis () - lastStartTime) > debounceDelay) {
        index = 0;
        index_changed = 1;
        start_camera ();
    }
    lastStartTime = millis ();
}

ISR (INT1_vect) { // Flash
    if ((millis () - lastFlashTime) > debounceDelay) {
        // turn programmed led on
        SBI(RCK_PIN);
        CBI(RCK_PIN);
        CBI(G_PIN);
        setTimeout (exposure);
        ++index;
        index_changed = 1;
        // Stop the camera one shot early
        if (index >= LED_COUNT) {
            stop_camera ();
        }
    }
    lastFlashTime = millis ();
}

ISR (TIMER0_OVF_vect) {
    _millis++;
}

ISR (TIMER1_COMPA_vect) {
    // Only turn the LEDs off in this function. If the camera flashes again
    // before the timeout, on_flash () will reset the timer and we will miss
    // this function altogether.

    // Stop the timer
    TCCR1B = 0;
    // Turns LEDs off
    SBI(G_PIN);
}

int main () {
    INPUT_PULLUP(FLASH_PIN);
    INPUT_PULLUP(START_PIN);
    INPUT_PULLUP(HAVE_FLASH_PIN);

    OUTPUT(TRIGGER_PIN);
    OUTPUT(WAKEUP_PIN);
    OUTPUT(READY_PIN);

    stop_camera ();

    SBI(READY_PIN);

#ifdef LED_MATRIX
    OUTPUT(SER_IN_PIN);
    OUTPUT(SRCK_PIN);
    OUTPUT(RCK_PIN);
    OUTPUT(G_PIN);

    SBI(G_PIN);
    CBI(SER_IN_PIN);
    CBI(SRCK_PIN);
    CBI(RCK_PIN);
#endif

#ifdef LED_STRIP
    pinMode (LED_STRIP_PIN, OUTPUT);

    memset (colors, 0, sizeof colors);
    memset (colors, 0x20, 3 * sizeof (rgb_color));
    writeStrip (LED_STRIP_PIN, colors, LED_COUNT);
    _delay_ms (1000);
    memset (colors, 0, sizeof colors);
    writeStrip (LED_STRIP_PIN, colors, LED_COUNT);
#endif

    EIMSK |= (1 << START_INT);   // enable interrupt
    EICRA |= (2 << ISC00);       // 2 == on FALLING edge

    EIMSK |= (1 << FLASH_INT) ;
    EICRA |= (2 << ISC10); // FIXME use macro

    initMs ();

    sei ();

    while (1) {
        if (index_changed) {
            index_changed = 0;

#ifdef LED_MATRIX
            if (index < LED_COUNT) {
                // prepare the 595s to fire the next LED
                uint8_t col = index % LED_COLS;
                uint8_t row = index / LED_COLS;
                write595 ((MASK_595_COL << col) | (MASK_595_ROW << row));
                // write595 (0x0011);
            }
#endif

#ifdef LED_STRIP
            memset (colors, 0, sizeof colors);
            if (index > 0 && (index - 1) < LED_COUNT)
                memset (colors + index - 1, 0x20, sizeof (rgb_color));
            writeStrip (LED_STRIP_PIN, colors, LED_COUNT);
#endif

        }
        if (ready_changed) {
            ready ? SBI(READY_PIN) : CBI(READY_PIN);
            ready_changed = 0;
        }
        sleep_mode ();
    }
}
