//
// Created by bemma on 19-04-2022.
//

#include "DispenserLED.h"

void DispenserLED::setColor(color color) {
    int colors[3] = {0, 0, 0};
    switch (color) {
        case green:
            colors[1] = 255;
            break;
        case orange:
            colors[0] = 255;
            colors[1] = 140;
            break;
        case red:
            colors[0] = 255;
            break;
    }
    for (int i = 0; i < LEDS_COUNT; i++) {
        ledStrip.setLedColorData(i, colors[0], colors[1], colors[2]);
    }
    ledStrip.show();
}