//
// Created by bemma on 19-04-2022.
//

#ifndef DISPENSERPROJECT_ACTUATOR_H
#define DISPENSERPROJECT_ACTUATOR_H

#include "Arduino.h"
#include "Stepper.h"
#include "../SharedConsts.h"

class Actuator {
public:
    Actuator();
    bool actuateTillThreshold(int initialValue, int steps, bool force);
    bool home();
    void actuate(int steps);
    bool dispense();
};


#endif //DISPENSERPROJECT_ACTUATOR_H
