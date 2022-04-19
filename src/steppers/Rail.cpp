#include "Rail.h"

#define STEPS_PER_CM 313 // It takes 313 steps for the stepper to move the carriage 1 CM.
#define DISTANCE_DISPENSERS_CM 10 // Approx millimeters between dispensers.
#define RAIL_LEEWAY_CM 1 // Expect the carriage to reach the destination within DISTANCE_DISPENSERS_CM+RAIL_LEEWAY_CM. Allows for some skipping even though it shouldn't during normal operation.
#define RAIL_LENGTH_IN_STEPS 7200 // The rail stepper can make 7200 before derailing.
#define RAIL_CNY_PIN 35
Stepper railStepper(STEPS_PER_REVOLUTION,
                    STEPPER_PINS[RAIL_PINS_INDEX][0],
                    STEPPER_PINS[RAIL_PINS_INDEX][1],
                    STEPPER_PINS[RAIL_PINS_INDEX][2],
                    STEPPER_PINS[RAIL_PINS_INDEX][3]
);

Rail::Rail() {
    railStepper.setSpeed(9); // Should be 3-20ms between steps.
}

extern bool error; // Declared in DispenserProject.ino.

bool Rail::moveToDispenser(int dispenserIndex) {
    bool movedCorrectly = true;
    if (dispenserIndex != currentDispenserPosition) { // Move the dispenser only if it is not already at the desired dispenser.
        int timesToMove = currentDispenserPosition == -1 ? dispenserIndex : dispenserIndex - currentDispenserPosition;
        for (int i = 0; i < abs(timesToMove); ++i) {
            if (timesToMove > 0) {
                movedCorrectly = moveToNextDispenser();
            } else movedCorrectly = moveToPreviousDispenser();
        }
    }
    return movedCorrectly;
}

bool Rail::moveToNextDispenser() {
    return moveTillThreshold(-STEP_GRANULARITY);
}

bool Rail::moveToPreviousDispenser() {
    return moveTillThreshold(STEP_GRANULARITY);
}

bool Rail::moveTillThreshold(int stepsBetweenCheck) {
    if (error) {
        return false;
    }
    bool startDetection = false;
    int detectionThreshold = 400; // Based on initialRailCNYReading. The value is an offset.
    int cnyValue;
    int expectedSteps = STEPS_PER_CM * (RAIL_LEEWAY_CM + DISTANCE_DISPENSERS_CM);
    int executedSteps = 0;
    do {
        if (stepsBetweenCheck > 0 && digitalRead(LIMIT_SWITCH_PIN) == HIGH || // Moving right. Stop at limit switch.
            stepsBetweenCheck < 0 && abs(railPosition) > RAIL_LENGTH_IN_STEPS) { // Moving left stop at end of rail.
            return false;
        }
        bool didStep = step(stepsBetweenCheck);
        if (!didStep || abs(executedSteps) > expectedSteps) {
            error = true;
            return false;
        }
        executedSteps += stepsBetweenCheck;
        cnyValue = readRailCNYAveraged();
        if (cnyValue > initialRailCNYReading - detectionThreshold) { // Start detection after having left the current marked area.
            startDetection = true;
        }
    } while (!startDetection || cnyValue > initialRailCNYReading - detectionThreshold);
    return true;
}

int Rail::readRailCNYAveraged(int millis) {
    int samples = 50;
    int readings = 0;
    for (int i = 0; i < samples; ++i) {
        readings += analogRead(RAIL_CNY_PIN);
        if (millis > 0) {
            delay(millis);
        }
    }
    return readings / 50;
}

int Rail::readRailCNYAveraged() {
    return readRailCNYAveraged(0);
}

bool Rail::step(int steps) {
    bool canStep = (steps < 0 || digitalRead(LIMIT_SWITCH_PIN) == LOW) && // Moving right. Stop at limit switch.
                   (steps > 0 || abs(railPosition) < RAIL_LENGTH_IN_STEPS); // Moving left stop at end of rail.
    if (canStep) { // Not at limit switch and not derailing.
        railPosition += steps;
        railStepper.step(steps);
    }
    return canStep;
}

void Rail::takeInitialRailCNYReading() {
    initialRailCNYReading = readRailCNYAveraged(1);
}

bool Rail::home() {
    int lsVal = digitalRead(LIMIT_SWITCH_PIN);
    if (lsVal == HIGH) {
        step(-STEP_GRANULARITY * 2);
    }
    int executedSteps = 0;
    while (lsVal == LOW) {
        step(STEP_GRANULARITY);
        lsVal = digitalRead(LIMIT_SWITCH_PIN);
        if (abs(executedSteps) > RAIL_LENGTH_IN_STEPS + RAIL_LEEWAY_CM * STEPS_PER_CM) {
            return false;
        }
        executedSteps += STEP_GRANULARITY;
    }
    step(-STEP_GRANULARITY * 2);
    railPosition = 0;
    currentDispenserPosition = -1;
    return true;
}