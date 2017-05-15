#pragma once

#include <Arduino.h>

typedef struct rgb_color {
    unsigned char green, red, blue;
    rgb_color () : green (0), red (0), blue (0) {};
    rgb_color (uint8_t r, uint8_t g, uint8_t b) : green (g), red (r), blue (b) {};
} rgb_color;

void writeStrip (unsigned int pin, rgb_color *colors, unsigned int count);
