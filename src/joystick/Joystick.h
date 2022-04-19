//
// Created by bemma on 19-04-2022.
//

#ifndef DISPENSERPROJECT_JOYSTICK_H
#define DISPENSERPROJECT_JOYSTICK_H

#include "../steppers/Rail.h"
#include "../steppers/Actuator.h"

class Joystick {
private:
    Rail rail;
    Actuator actuator;
    int jsX = 13;
    int jsY = 14;
    int jsZ = 15;
public:
    Joystick(Rail rail, Actuator actuator);
    void checkJoystick();
    int getJSX() {
        return jsX;
    }
    int getJSY() {
        return jsY;
    }
    int getJSZ() {
        return jsZ;
    }
};


#endif //DISPENSERPROJECT_JOYSTICK_H
