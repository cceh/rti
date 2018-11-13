#include <Arduino.h>
#include "canon.h"

// See: http://www.doc-diy.net/photo/rc-1_hacked/index.php
#define FREQ 32768
#define TIME_16_PULSES 488   // 488µs = 16 / 32768
#define DELAY_TRIGGER         7333
#define DELAY_DELAYED_TRIGGER 5360

void canon_ir_trigger (unsigned int pin) {
    tone (pin, FREQ);
    delayMicroseconds (TIME_16_PULSES);
    noTone (pin);
    digitalWrite (pin, LOW);
    delayMicroseconds (DELAY_TRIGGER);
    tone (pin, FREQ);
    delayMicroseconds (TIME_16_PULSES);
    noTone (pin);
    digitalWrite (pin, LOW);
}
