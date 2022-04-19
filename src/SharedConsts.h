//
// Created by bemma on 19-04-2022.
//

#ifndef DISPENSERPROJECT_CONSTS_H
#define DISPENSERPROJECT_CONSTS_H

#define LIMIT_SWITCH_PIN 32
#define STEP_GRANULARITY 100 // Steps per Stepper.step() invocation.

// Stepper motors
#define STEPS_PER_REVOLUTION 2048 // 32 * 64
#define RAIL_PINS_INDEX 0
#define ACTUATOR_PINS_INDEX 1
const int STEPPER_PINS[2][4] = {{27, 25, 26, 33},
                         {18, 21, 19, 22}};

// CNY70s
#define ACTUATOR_CNY_PIN 34
#define RAIL_CNY_PIN 35

#endif //DISPENSERPROJECT_CONSTS_H
