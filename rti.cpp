#include <Arduino.h>
#include "ledstrip.h"
#include "canon.h"

#define LED_STRIP_PIN    12
#define LED_STRIP_COUNT 150

#define TRIGGER_PIN              11  // Tip of the 2,5mm jack
#define FOCUS_PIN                10  // Ring of the 2,5mm jack
#define TRIGGER_CABLE_DETECT_PIN  9  // Presence switch on the 2,5mm jack socket.

#define LED_IR_PIN 8

rgb_color colors[LED_STRIP_COUNT];
int index;

void setup () {
    index = 0;
    pinMode (LED_STRIP_PIN, OUTPUT);

    pinMode (TRIGGER_PIN,              OUTPUT);
    pinMode (FOCUS_PIN,                OUTPUT);
    pinMode (TRIGGER_CABLE_DETECT_PIN, INPUT);

    pinMode (LED_IR_PIN, OUTPUT);
}

void loop () {
    memset (colors, 0, sizeof colors);
    memset (colors + index, 0x20, sizeof (rgb_color));

    digitalWrite (TRIGGER_PIN, HIGH);
    writeStrip (LED_STRIP_PIN, colors, LED_STRIP_COUNT);

    delay (100);
    digitalWrite (TRIGGER_PIN, LOW);
    delay (900);

    // I have to get a camera to test this.
    // canon_ir_trigger (LED_IR_PIN);

    index++;
    if (index >= LED_STRIP_COUNT)
        index = 0;
}
