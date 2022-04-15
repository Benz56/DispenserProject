#include <Arduino.h>
#include <Stepper.h> // TODO Maybe use accelstepper library
#include "SerialRead.h"
#include <Preferences.h>

#define DISPENSERS 3 // Number of snus containers. Odd number only.

// Limit Switch
#define LIMIT_SWITCH_PIN 32
int lsVal;

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

long currentDispenserPosition = DISPENSERS / 2 + 1; // e.g. 3/2=1.5=1+1=2 or 5/2=2.5=2+1=3. 1-indexed. Starts at middle or loaded position. // TODO position system should be changed.

// Actuator
int actuatorSteps = 3100;

// Joystick
int xyzPins[] = {13, 14, 15};   //x,y,z pins

// Persistent Storage
Preferences preferences;
String positionKey = "position";


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
    pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
    preferences.begin("dispenser-app", false);  // Used for saving position.
    currentDispenserPosition = preferences.getLong(positionKey.c_str(), currentDispenserPosition); // Get stored position or default.
    Serial.println(currentDispenserPosition);
}

void loop() {
    String serialString = getSerialString();
    Serial.println(serialString);
    // Serial functionality
    if (serialString.length() != 0) {
        // Serial is dispenser goto command?
        long dispenserIndex = serialString.toInt(); // Read the integer i.e. dispenser to go to.
        if (dispenserIndex > 0 && dispenserIndex <= DISPENSERS) {
            Serial.printf("Moving to %ld!\n", dispenserIndex);
            triggerDispenser(dispenserIndex);
        } else {
            // Can be used for debugging or testing new features via Serial commands.
            Serial.println(serialString);
        }
    }
    checkJoystick();
}

void checkJoystick() { // TODO If used then it should home before accepting orders.
    int xVal = analogRead(xyzPins[0]);
    int yVal = analogRead(xyzPins[1]);

    // x == 0 == forward, x == 4095 == back, y == 0 == left, y == 4095 == right.
    int tolerance = 500;
    bool joystickInUse = xVal <= tolerance || xVal >= 4095 - tolerance || yVal <= tolerance || yVal >= 4095 - tolerance;
    if (joystickInUse) {
        Stepper stepper = xVal <= tolerance || xVal >= 4095 - tolerance ? actuatorStepper : railStepper;
        int steps = xVal <= tolerance || yVal <= tolerance ? 96 : -96;
        stepper.step(steps);
    } else if (digitalRead(xyzPins[2]) == LOW) {
        home(); // Home if joystick is pressed down.
    }
}

void triggerDispenser(long index) {
    if (index != currentDispenserPosition) { // Move the dispenser only if it is not already at the desired dispenser.
        int stepsToMove = abs(currentDispenserPosition - index) * (DISTANCE_DISPENSERS_MM * STEPS_PER_CM / 10);
        stepsToMove = currentDispenserPosition > index ? -stepsToMove : stepsToMove; // Determine left or right.
        railStepper.step(stepsToMove);
        currentDispenserPosition = index;
        preferences.putLong(positionKey.c_str(), currentDispenserPosition);
    }
    actuatorStepper.step(-actuatorSteps);
    actuatorStepper.step(actuatorSteps); // TODO Make CNY70 readings and figure out if out.
}

void home() {
  lsVal = digitalRead(LIMIT_SWITCH_PIN);
  if (lsVal != HIGH) {  // Not already homed.
    while (lsVal == LOW) {
        railStepper.step(100);
      lsVal = digitalRead(LIMIT_SWITCH_PIN);
    }
      railStepper.step(-200);
  }
}