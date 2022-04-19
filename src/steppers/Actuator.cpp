#include "Actuator.h"

#define EXPECTED_ACTUATOR_STEPS 3500
#define ACTUATOR_STEPS_LEEWAY 700
#define ACTUATOR_DETECTION_THRESHOLD 1500

Stepper actuatorStepper(STEPS_PER_REVOLUTION,
                STEPPER_PINS[ACTUATOR_PINS_INDEX][0],
                STEPPER_PINS[ACTUATOR_PINS_INDEX][1],
                STEPPER_PINS[ACTUATOR_PINS_INDEX][2],
                STEPPER_PINS[ACTUATOR_PINS_INDEX][3]
);

Actuator::Actuator() {
    actuatorStepper.setSpeed(9); // Should be 3-20ms between steps.
}

extern bool error; // Declared in DispenserProject.ino.

bool Actuator::actuateTillThreshold(int initialValue, int steps, bool force = false) {
    if (error && !force) {
        return false;
    }
    bool startDetection = false;
    int executedSteps = 0;
    while (!startDetection || initialValue > ACTUATOR_DETECTION_THRESHOLD) {
        if (initialValue > ACTUATOR_DETECTION_THRESHOLD) { // Start detection after having left the current marked area.
            startDetection = true;
        }
        actuatorStepper.step(steps);
        if (abs(executedSteps) > ACTUATOR_STEPS_LEEWAY + EXPECTED_ACTUATOR_STEPS) {
            error = true;
            return false;
        }
        executedSteps += steps;
        initialValue = analogRead(ACTUATOR_CNY_PIN);
    }
    return true;
}

bool Actuator::home() {
    // Home actuator
    // 3150 white, 850 - 1000 black.
    int cnyValue = analogRead(ACTUATOR_CNY_PIN);

    if (cnyValue > ACTUATOR_DETECTION_THRESHOLD) { // Current at white.
        if (!actuateTillThreshold(cnyValue, STEP_GRANULARITY, true)) {
            return false;
        }
    } else { // Current at black.
        // Check for white in each direction incrementally. Based on where white is found we can establish if the actuator is retracted or advanced.
        int n = 1;
        do {
            actuatorStepper.step(-(STEP_GRANULARITY * n + STEP_GRANULARITY * (n - 1))); // Move steps + undo steps from previous iteration.
            cnyValue = analogRead(ACTUATOR_CNY_PIN);
            if (cnyValue > ACTUATOR_DETECTION_THRESHOLD) { // Retracted.
                break;
            }
            actuatorStepper.step(STEP_GRANULARITY * n * 2); // Move the other way. (*2 to undo steps from above)
            cnyValue = analogRead(ACTUATOR_CNY_PIN);
            if (cnyValue > ACTUATOR_DETECTION_THRESHOLD) { // Advanced.
                if (!actuateTillThreshold(cnyValue, STEP_GRANULARITY, true)) {
                    return false;
                }
                break;
            }
            n++;
        } while (cnyValue < ACTUATOR_DETECTION_THRESHOLD && n < 5);
        if (n >= 6) { // Couldn't find white within reasonable range.
            error = true;
            return false;
        }
    }
    return true;
}

void Actuator::actuate(int steps) {
    actuatorStepper.step(steps);
}

bool Actuator::dispense() {
    bool movedCorrectly = actuateTillThreshold(analogRead(ACTUATOR_CNY_PIN), -STEP_GRANULARITY);
    if (movedCorrectly && !error) {
        movedCorrectly = actuateTillThreshold(analogRead(ACTUATOR_CNY_PIN), STEP_GRANULARITY);
    }
    return movedCorrectly;
}
