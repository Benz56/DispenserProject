#include <Arduino.h>
#include <Stepper.h> // TODO Maybe use accelstepper library

#define DISPENSERS 3 // Number of snus containers. Odd number only.
#define STEPS_PER_REVOLUTION 2048 // 32 * 64

// Stepper motors
#define RAIL_PINS_INDEX 0
#define ACTUATOR_PINS_INDEX 1
#define STEPS_PER_REVOLUTION 2048 // 32 * 64
#define STEPS_PER_CM 313 // It takes 313 steps for the stepper to move the carriage 1 CM.
#define DISTANCE_DISPENSERS_MM 86 // Centimeters between dispensers.
int stepperPins[2][4] = {{27, 25, 26, 33},
                         {18, 21, 19, 22}};
Stepper railStepper(STEPS_PER_REVOLUTION, stepperPins[RAIL_PINS_INDEX][0], stepperPins[RAIL_PINS_INDEX][1],
                    stepperPins[RAIL_PINS_INDEX][2], stepperPins[RAIL_PINS_INDEX][3]); // Pins
Stepper actuatorStepper(STEPS_PER_REVOLUTION, stepperPins[ACTUATOR_PINS_INDEX][0], stepperPins[ACTUATOR_PINS_INDEX][1],
                        stepperPins[ACTUATOR_PINS_INDEX][2], stepperPins[ACTUATOR_PINS_INDEX][3]); // Pins

int currentDispenserPosition = DISPENSERS / 2 + 1; // e.g. 3/2=1.5=1+1=2 or 5/2=2.5=2+1=3. 1-indexed. Starts at

// Actuator
int actuatorSteps = 3000;

// Joystick
int xyzPins[] = {13, 12, 14};   //x,y,z pins

String inputString;

bool stringComplete;

void checkJoystick();

void setup() {
    Serial.begin(115200);
    for (auto &stepperPin: stepperPins) {
        for (int j: stepperPin) {
            pinMode(j, OUTPUT);
        }
    }
    railStepper.setSpeed(9); // Should be 3-20ms between steps.
    actuatorStepper.setSpeed(9); // Should be 3-20ms between steps.
    pinMode(xyzPins[2], INPUT_PULLUP);  // Z axis is a button (No use for now).
}

void loop() {
    if (Serial.available() > 0) {
        long dispenserIndex = Serial.parseInt(); // Read the integer i.e. dispenser to go to.
        if (dispenserIndex > 0 && dispenserIndex <= DISPENSERS) {
            Serial.printf("Moving to %ld!\n", dispenserIndex);
            triggerDispenser(dispenserIndex);
        }
    }

    checkJoystick();
}

void checkJoystick() {
    int xVal = analogRead(xyzPins[0]);
    int yVal = analogRead(xyzPins[1]);

    // x == 0 == forward, x == 4095 == back, y == 0 == left, y == 4095 == right.
    bool joystickInUse = xVal == 0 || xVal == 4095 || yVal == 0 || yVal == 4095;
    if (joystickInUse) {
        Stepper stepper = xVal == 0 || xVal == 4095 ? actuatorStepper : railStepper;
        int steps = xVal == 0 || yVal == 0 ? -96 : 96;
        stepper.step(steps);
    }
}

void triggerDispenser(long index) {
    if (index != currentDispenserPosition) { // Move the dispenser only if it is not already at the desired dispenser.
        int stepsToMove = abs(currentDispenserPosition - index) * (DISTANCE_DISPENSERS_MM * STEPS_PER_CM / 10);
        stepsToMove = currentDispenserPosition > index ? -stepsToMove : stepsToMove; // Determine left or right.
        railStepper.step(stepsToMove);
        currentDispenserPosition = index;
    }
    actuatorStepper.step(-actuatorSteps);
    actuatorStepper.step(actuatorSteps);
}