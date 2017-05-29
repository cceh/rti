#include <Arduino.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>

#define LED_MATRIX

#ifdef LED_STRIP
    #include "ledstrip.h"
#endif
#ifdef IR_LED
    #include "canon.h"
#endif

#define LED_PIN            13  // Arduino green led

#define TRIGGER_PIN        11  // Tip of the 2,5mm jack  (trigger the camera)
#define WAKEUP_PIN         10  // Ring of the 2,5mm jack (wakeup the camera)
#define FLASH_PIN           2  // Tip of the 3,5mm jack  (the camera wants us to flash)
#define START_PIN           3  // Start button           (the user wants to start)
#define FLASH_INSERTED_PIN  4  // Ring of the 3,5mm jack (jack presence detection)

#ifdef LED_MATRIX
    // the led matrix 595 driver
    #define LED_COUNT  16
    #define LED_COLS    4
    #define SER_IN_PIN  5
    #define SRCK_PIN    6
    #define RCK_PIN     7
    #define G_PIN       8
    uint8_t ser_in_port;
    uint8_t ser_in_mask;
    uint8_t srck_port;
    uint8_t srck_mask;
    volatile uint8_t* ser_in_ptr;
    volatile uint8_t* srck_ptr;
#endif

#ifdef LED_STRIP
    #define LED_STRIP_PIN      12
    #define LED_COUNT          10
    rgb_color colors[LED_COUNT];
#endif

volatile uint16_t index   = 0;
volatile uint8_t  index_changed = 0;
volatile uint8_t  blink   = 0;
volatile uint8_t  blink_changed = 0;
volatile unsigned long lastFlashTime = 0;
volatile unsigned long lastStartTime = 0;

uint16_t exposure = 10;  // 1s / 60

// We assume no camera comes close to 20 pics / sec.
// N.B. Flash signal on Canon EOS30D is low for ~33ms.
const unsigned long debounceDelay = 50;

void start_camera () {
    digitalWrite (WAKEUP_PIN, HIGH);
    delay (250);
    digitalWrite (TRIGGER_PIN, HIGH);
}

void stop_camera () {
    digitalWrite (WAKEUP_PIN, LOW);
    digitalWrite (TRIGGER_PIN, LOW);
}

void setTimeout (uint16_t nth_of_second) {
    // Don't drive any of the pins directly, WGM11 = WGM10 = 0
    TCCR1A = 0;
    TCNT1  = 0; // initialize counter value to 0
    // Set compare match register
    OCR1A = 15625 / nth_of_second; // = 16MHz / 1024 (prescaler) (must be <65536)
    // Enable output compare match interrupt
    TIMSK1 |= (1 << OCIE1A);
    // Set WGM12 for Clear Timer on Compare Match (CTC) Mode
    // Set CS12 and CS10 for a prescaler of 1024
    // See section 16.11.2 of the ATmega328p Datasheet
    // Setting the clock source starts the timer
    TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
}

#ifdef LED_MATRIX
void write595 (uint16_t value) {
    cli ();
    for (uint8_t i = 0; i < 16; ++i)  {
        *srck_ptr = *srck_ptr & ~srck_mask;   // clk -> low
        if (value & 0x8000) {                 // data out
            *ser_in_ptr = *ser_in_ptr | ser_in_mask;
        } else {
            *ser_in_ptr = *ser_in_ptr & ~ser_in_mask;
        }
        *srck_ptr = *srck_ptr | srck_mask;    // clk -> high
        value <<= 1;
    }
    *ser_in_ptr = *ser_in_ptr & ~ser_in_mask; // ser_in -> low
    *srck_ptr = *srck_ptr & ~srck_mask;       // clk -> low
    sei ();
}
#endif

void on_start () {
    if ((millis () - lastStartTime) > debounceDelay) {
        index = 0;
        index_changed = 1;
        start_camera ();
    }
    lastStartTime = millis ();
}

void on_flash () {
    if ((millis () - lastFlashTime) > debounceDelay) {
        // turn programmed led on
        digitalWrite (G_PIN, LOW);
        setTimeout (exposure);
        ++index;
        index_changed = 1;
        if (index >= LED_COUNT) {
            stop_camera ();
        }
    }
    lastFlashTime = millis ();
}

ISR (TIMER1_COMPA_vect) {
    // Only turn the LEDs off in this function. If the camera flashes again
    // before the timeout, we will miss this altogether.

    // Stop the timer
    TCCR1B = 0;
    // Turns LEDs off
    digitalWrite (G_PIN, HIGH);
}

void setup () {
    pinMode (FLASH_PIN,          INPUT_PULLUP);
    pinMode (START_PIN,          INPUT_PULLUP);
    pinMode (FLASH_INSERTED_PIN, INPUT_PULLUP);
    pinMode (TRIGGER_PIN,        OUTPUT);
    pinMode (WAKEUP_PIN,         OUTPUT);

    stop_camera ();

#ifdef LED_MATRIX
    pinMode (SER_IN_PIN, OUTPUT);
    pinMode (SRCK_PIN,   OUTPUT);
    pinMode (RCK_PIN,    OUTPUT);
    pinMode (G_PIN,      OUTPUT);

    digitalWrite (G_PIN, HIGH);
    digitalWrite (SER_IN_PIN, LOW);
    digitalWrite (SRCK_PIN, LOW);
    digitalWrite (RCK_PIN, LOW);

    ser_in_port = digitalPinToPort    (SER_IN_PIN);
    ser_in_mask = digitalPinToBitMask (SER_IN_PIN);
    srck_port   = digitalPinToPort    (SRCK_PIN);
    srck_mask   = digitalPinToBitMask (SRCK_PIN);
    ser_in_ptr  = portOutputRegister (ser_in_port); // the memory address for the port
    srck_ptr    = portOutputRegister (srck_port);   // the memory address for the port
#endif

#ifdef LED_STRIP
    pinMode (LED_STRIP_PIN, OUTPUT);

    memset (colors, 0, sizeof colors);
    memset (colors, 0x20, 3 * sizeof (rgb_color));
    writeStrip (LED_STRIP_PIN, colors, LED_COUNT);
    delay (1000);
    memset (colors, 0, sizeof colors);
    writeStrip (LED_STRIP_PIN, colors, LED_COUNT);
#endif

    attachInterrupt (0, on_flash, FALLING);
    attachInterrupt (1, on_start, FALLING);
}

void loop () {
    if (index_changed) {
        index_changed = 0;

#ifdef LED_MATRIX
        if (index < LED_COUNT) {
            // prepare the 595s to fire the next LED
            uint8_t col = index % LED_COLS;
            uint8_t row = index / LED_COLS;
            write595 ((0x10 << col) | (0x01 << row));
            digitalWrite (RCK_PIN, HIGH);
            digitalWrite (RCK_PIN, LOW);
        }
#endif

#ifdef LED_STRIP
        memset (colors, 0, sizeof colors);
        if (index > 0 && (index - 1) < LED_COUNT)
            memset (colors + index - 1, 0x20, sizeof (rgb_color));
        writeStrip (LED_STRIP_PIN, colors, LED_COUNT);
#endif

    }
    if (blink_changed) {
        digitalWrite (LED_PIN, (blink & 1) ? HIGH : LOW);
        blink_changed = 0;
    }
    sleep_mode ();
}
