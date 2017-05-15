#include "ledstrip.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define __enable_irq sei
#define __disable_irq cli

#if !(F_CPU == 16000000)
#error "Only devices with a 16 MHz clock are supported."
#endif

void writeStrip (unsigned int pin, rgb_color *colors, unsigned int count)
{
	uint8_t  port = digitalPinToPort (pin);
	uint8_t  mask = digitalPinToBitMask (pin);
    volatile uint8_t* mem  = portOutputRegister (port);   // the memory address for the port
    uint8_t reg_hi = *mem | mask;
    uint8_t reg_lo = *mem & ~mask;

    unsigned char *bytes = (unsigned char *) colors;
    count *= 3;              // Count of colors => count of bytes

    __disable_irq ();        // Because of the tight timing
    while (count--) {
        unsigned char color = *bytes++;
        uint8_t bits = 8;
        do {
            // For the WS2812B the complete cycle length is 1.25 µs.
            // A 1 bit is sent by 0.4µs  hi + 0.85µs lo.
            // A 0 bit is sent by 0.85µs hi + 0.4µs  lo.
            // Terminate transmission by 50µs of lo.
            // Bits are sent hi order first.
            // Colors are sent in order green, red, blue.
            // See: https://cdn-shop.adafruit.com/datasheets/WS2812B.pdf

            // Arduino nano clock is 16 MHz. Cycle length is 0.063µs
            // 6.35 cycles for 0.4µs
            // 13.5 cycles for 0.85µs
            // 19.8 cycles for 1.25µs
            //
            // 30µs / LED
            // 3ms  / 100 LEDs

            asm volatile (
                  "st %a0, %[hi]\n"                // 1 clk output hi state

                  "nop\n"                          // 1 clk
                  "nop\n"                          // 1 clk
                  "nop\n"                          // 1 clk

                  "rol %[color]\n"                 // 1 clk Rotate left through carry
                  "brcs long\n"                    // 1 or 2 clk
                  "st %a0, %[lo]\n"                // 1 clk output lo state

                  "long:\n"
                  "nop\n"                          // 1 clk
                  "nop\n"                          // 1 clk
                  "nop\n"                          // 1 clk
                  "nop\n"                          // 1 clk
                  "nop\n"                          // 1 clk
                  "nop\n"                          // 1 clk

                  "st %a0, %[lo]\n"                // 1 clk output lo state

                  "nop\n"                          // 1 clk
                  "nop\n"                          // 1 clk
                  "nop\n"                          // 1 clk

                  // subi  1 clk       (generatedby compiler for do while loop)
                  // brne  1 or 2 clk   "

                  // output operands
                  :

                  // input operands
                  :
                  [mem]   "e" (mem),
                  [color] "r" (color),
                  [hi]    "r" (reg_hi),
                  [lo]    "r" (reg_lo)
           );
        } while (--bits);
    }
    __enable_irq ();

    delayMicroseconds (50);  // This sends a reset signal to the leds
}
