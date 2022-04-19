//
// Created by bemma on 19-04-2022.
//

#include "Joystick.h"
#include "../../DispenserProject.h"

extern bool shouldHome;

Joystick::Joystick(Rail rail, Actuator actuator) {
    this->rail = rail;
    this->actuator = actuator;
}

void Joystick::checkJoystick() {
    int xVal = analogRead(jsX);
    int yVal = analogRead(jsY);
    // x == 0 == forward, x == 4095 == back, y == 0 == left, y == 4095 == right.
    int tolerance = 1000;
    bool joystickInUse = xVal <= tolerance || xVal >= 4095 - tolerance || yVal <= tolerance || yVal >= 4095 - tolerance;
    if (joystickInUse) {
        shouldHome = true;
        bool isActuator = xVal <= tolerance || xVal >= 4095 - tolerance;
        int steps = xVal <= tolerance || yVal <= tolerance ? STEP_GRANULARITY : -STEP_GRANULARITY;
        if (isActuator) {
            actuator.actuate(steps);
        } else rail.step(steps);
    } else if (digitalRead(jsZ) == LOW) {
        home(); // Home if joystick is pressed down.
    }
}