#ifndef DISPENSERPROJECT_DISPENSERLED_H
#define DISPENSERPROJECT_DISPENSERLED_H

#include "Freenove_WS2812_Lib_for_ESP32.h"

#define LEDS_COUNT 8 // The number of leds
#define LEDS_PIN 2 // The pin connected to the Freenove 8 led ledStrip
#define CHANNEL 0 // RMT channel

enum color {
    red, green, orange
};

class DispenserLED {
public:
    Freenove_ESP32_WS2812 ledStrip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);
    void setColor(color color);
};


#endif //DISPENSERPROJECT_DISPENSERLED_H
