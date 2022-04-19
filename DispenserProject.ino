#include <Arduino.h>
#include <Stepper.h> // TODO Maybe use accelstepper library
#include "SerialRead.h"
#include "DispenserLED.h"

#define DISPENSERS 3 // Number of snus containers. Odd number only.
#define STEP_GRANULARITY 100 // Steps per Stepper.step() invocation.

// Limit Switch
#define LIMIT_SWITCH_PIN 32
#define LIMIT_SWITCH_TO_FIRST_DISPENSER_CM 3

// Stepper motors and rail
#define STEPS_PER_REVOLUTION 2048 // 32 * 64
#define STEPS_PER_CM 313 // It takes 313 steps for the stepper to move the carriage 1 CM.
#define DISTANCE_DISPENSERS_CM 10 // Approx millimeters between dispensers.
#define RAIL_LEEWAY_CM 1 // Expect the carriage to reach the destination within DISTANCE_DISPENSERS_CM+RAIL_LEEWAY_CM. Allows for some skipping even though it shouldn't during normal operation.
#define RAIL_PINS_INDEX 0
#define ACTUATOR_PINS_INDEX 1
#define RAIL_LENGTH_IN_STEPS 7200 // The rail stepper can make 7200 before derailing.
int stepperPins[2][4] = {{27, 25, 26, 33},
                         {18, 21, 19, 22}};
Stepper railStepper(STEPS_PER_REVOLUTION, stepperPins[RAIL_PINS_INDEX][0], stepperPins[RAIL_PINS_INDEX][1],
                    stepperPins[RAIL_PINS_INDEX][2], stepperPins[RAIL_PINS_INDEX][3]); // Pins

Stepper actuatorStepper(STEPS_PER_REVOLUTION, stepperPins[ACTUATOR_PINS_INDEX][0], stepperPins[ACTUATOR_PINS_INDEX][1],
                        stepperPins[ACTUATOR_PINS_INDEX][2], stepperPins[ACTUATOR_PINS_INDEX][3]); // Pins
bool shouldHome = false; // If the joystick is used then home the carriage before taking next order.

// CNY70s
#define ACTUATOR_CNY_PIN 34
#define RAIL_CNY_PIN 35

int currentDispenserPosition = -1; // -1 == is not at dispenser.
int initialRailCNYReading = 4096; // Is set in setup().

// Actuator
#define EXPECTED_ACTUATOR_STEPS 3500
#define ACTUATOR_STEPS_LEEWAY 700
#define ACTUATOR_DETECTION_THRESHOLD 1500

// LED
DispenserLED dispenserLed = DispenserLED();

// Joystick
int xyzJoystickPins[] = {13, 14, 15};   //x,y,z pins

unsigned long lastRefreshTime = millis();

int railPosition;

bool debugging = false;
bool error = false;

void setup() {
    dispenserLed.ledStrip.begin();
    dispenserLed.ledStrip.setBrightness(5);
    dispenserLed.setColor(orange);
    Serial.begin(115200);
    for (auto &stepperPin: stepperPins) {
        for (int j: stepperPin) {
            pinMode(j, OUTPUT);
        }
    }
    railStepper.setSpeed(9); // Should be 3-20ms between steps.
    actuatorStepper.setSpeed(9); // Should be 3-20ms between steps.
    pinMode(xyzJoystickPins[2], INPUT_PULLUP);  // Z axis is a button (Homes).
    pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
    //preferences.begin("dispenser-app", false);
    home();
    initialRailCNYReading = readRailCNYAveraged(1);
}

int readRailCNYAveraged(int millis) {
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

int readRailCNYAveraged() {
    return readRailCNYAveraged(0);
}

void loop() {
    String serialString = getSerialString();
    // Serial functionality
    if (serialString.length() != 0) {
        // Serial is dispenser goto command?
        long dispenserIndex = serialString.toInt(); // Read the integer i.e. dispenser to go to.
        if (dispenserIndex > 0 && dispenserIndex <= DISPENSERS) {
            Serial.printf("Moving to %ld!\n", dispenserIndex);
            triggerDispenser(dispenserIndex);
        } else if (serialString == "info") {
            printInfo();
        } else if (serialString == "debug" || serialString == "debugging") {
            debugging = !debugging;
        } else if (serialString == "next") {
            moveToNextDispenser();
        } else if (serialString == "prev") {
            moveToPreviousDispenser();
        } else {
            // Can be used for debugging or testing new features via Serial commands.
            Serial.println(serialString);
        }
    }
    checkJoystick();
    if (millis() - lastRefreshTime >= 1000) {
        lastRefreshTime += 1000;
        loopSecond();
    }
}

void loopSecond() {
    if (debugging) {
        printInfo();
    }
}

void printInfo() {
    Serial.println("CNY70 Rail: " + String(analogRead(RAIL_CNY_PIN)));
    Serial.println("CNY70 Rail Average: " + String(readRailCNYAveraged()));
    Serial.println("CNY70 Rail Initial: " + String(initialRailCNYReading));
    Serial.println("CNY70 Actuator: " + String(analogRead(ACTUATOR_CNY_PIN)));
    Serial.println("Limit Switch: " + String(digitalRead(LIMIT_SWITCH_PIN)));
    Serial.println("Rail Position: " + String(railPosition));
    Serial.println("Dispenser Position: " + String(currentDispenserPosition));
    Serial.println("Error: " + String(error));
    Serial.println("ShouldHome: " + String(shouldHome));
    Serial.println("JoyStick:");
    Serial.println("    x=" + String(analogRead(xyzJoystickPins[0])));
    Serial.println("    y=" + String(analogRead(xyzJoystickPins[1])));
    Serial.println("    z=" + String(digitalRead(xyzJoystickPins[2])));
}

void checkJoystick() {
    int xVal = analogRead(xyzJoystickPins[0]);
    int yVal = analogRead(xyzJoystickPins[1]);
    // x == 0 == forward, x == 4095 == back, y == 0 == left, y == 4095 == right.
    int tolerance = 1000;
    bool joystickInUse = xVal <= tolerance || xVal >= 4095 - tolerance || yVal <= tolerance || yVal >= 4095 - tolerance;
    if (joystickInUse) {
        shouldHome = true;
        bool actuator = xVal <= tolerance || xVal >= 4095 - tolerance;
        int steps = xVal <= tolerance || yVal <= tolerance ? STEP_GRANULARITY : -STEP_GRANULARITY;
        if (actuator) {
            actuatorStepper.step(steps);
        } else stepRail(steps);
    } else if (digitalRead(xyzJoystickPins[2]) == LOW) {
        home(); // Home if joystick is pressed down.
    }
}

bool triggerDispenser(int index) {
    if (error) {
        return false;
    }
    if (shouldHome) {
        home();
        shouldHome = false;
    }
    bool movedCorrectly = true;
    if (index != currentDispenserPosition) { // Move the dispenser only if it is not already at the desired dispenser.
        int timesToMove = currentDispenserPosition == -1 ? index : index - currentDispenserPosition;
        for (int i = 0; i < abs(timesToMove); ++i) {
            if (timesToMove > 0) {
                movedCorrectly = moveToNextDispenser();
            } else movedCorrectly = moveToPreviousDispenser();
        }
    }
    if (movedCorrectly && !error) {
        movedCorrectly = actuateTillThreshold(analogRead(ACTUATOR_CNY_PIN), -STEP_GRANULARITY);
        if (movedCorrectly) {
            movedCorrectly = actuateTillThreshold(analogRead(ACTUATOR_CNY_PIN), STEP_GRANULARITY);
        }
    }
    if (movedCorrectly) {
        currentDispenserPosition = index;
    }
    return movedCorrectly;
}

bool moveToNextDispenser() {
    return moveToDispenser(-STEP_GRANULARITY);
}

bool moveToPreviousDispenser() {
    return moveToDispenser(STEP_GRANULARITY);
}

bool moveToDispenser(int stepsBetweenCheck) {
    if (error) {
        return false;
    }
    bool startDetection = false;
    int detectionThreshold = 400; // Based on initialRailCNYReading. The value is an offset.
    int cnyValue;
    int expectedSteps = STEPS_PER_CM * (RAIL_LEEWAY_CM + (currentDispenserPosition < 0 ? LIMIT_SWITCH_TO_FIRST_DISPENSER_CM : DISTANCE_DISPENSERS_CM));
    int executedSteps = 0;
    do {
        if (stepsBetweenCheck > 0 && digitalRead(LIMIT_SWITCH_PIN) == HIGH || // Moving right. Stop at limit switch.
            stepsBetweenCheck < 0 && abs(railPosition) > RAIL_LENGTH_IN_STEPS) { // Moving left stop at end of rail.
            return false;
        }
        bool didStep = stepRail(stepsBetweenCheck);
        if (!didStep || abs(executedSteps) > expectedSteps) {
            dispenserLed.setColor(red);
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

void home() {
    dispenserLed.setColor(orange);
    // Home actuator
    // 3150 white, 850 - 1000 black.
    int cnyValue = analogRead(ACTUATOR_CNY_PIN);

    if (cnyValue > ACTUATOR_DETECTION_THRESHOLD) { // Current at white.
        if (!actuateTillThreshold(cnyValue, STEP_GRANULARITY)) {
            return;
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
                if (!actuateTillThreshold(cnyValue, STEP_GRANULARITY)) {
                    return;
                }
                break;
            }
            n++;
        } while (cnyValue < ACTUATOR_DETECTION_THRESHOLD && n < 5);
        if (n >= 5) { // Couldn't find white within reasonable range.
            dispenserLed.setColor(red);
            return;
        }
    }

    // Home rail
    int lsVal = digitalRead(LIMIT_SWITCH_PIN);
    if (lsVal == HIGH) {
        stepRail(-STEP_GRANULARITY * 2);
    }
    int executedSteps = 0;
    while (lsVal == LOW) {
        stepRail(STEP_GRANULARITY);
        lsVal = digitalRead(LIMIT_SWITCH_PIN);
        if (abs(executedSteps) > RAIL_LENGTH_IN_STEPS + RAIL_LEEWAY_CM * STEPS_PER_CM) {
            dispenserLed.setColor(red);
            return;
        }
        executedSteps += STEP_GRANULARITY;
    }
    stepRail(-STEP_GRANULARITY * 2);
    railPosition = 0;
    currentDispenserPosition = -1;
    dispenserLed.setColor(green);
    error = false;
}

bool actuateTillThreshold(int initialValue, int steps) {
    if (error) {
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
            dispenserLed.setColor(red);
            error = true;
            return false;
        }
        executedSteps += steps;
        initialValue = analogRead(ACTUATOR_CNY_PIN);
    }
    return true;
}

bool stepRail(int steps) {
    bool canStep = (steps < 0 || digitalRead(LIMIT_SWITCH_PIN) == LOW) && // Moving right. Stop at limit switch.
                   (steps > 0 || abs(railPosition) < RAIL_LENGTH_IN_STEPS); // Moving left stop at end of rail.
    if (canStep) { // Not at limit switch and not derailing.
        railPosition += steps;
        railStepper.step(steps);
    }
    return canStep;
}

//void setColor(color color) {
//    int colors[3] = {0, 0, 0};
//    switch (color) {
//        case green:
//            colors[1] = 255;
//            break;
//        case orange:
//            colors[0] = 255;
//            colors[1] = 140;
//            break;
//        case red:
//            colors[0] = 255;
//            break;
//    }
//    for (int i = 0; i < LEDS_COUNT; i++) {
//        ledStrip.setLedColorData(i, colors[0], colors[1], colors[2]);
//    }
//    ledStrip.show();
//}