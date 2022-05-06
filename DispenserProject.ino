#include <Arduino.h>
#include <Stepper.h>
#include "DispenserProject.h"
#include "src/serial/SerialRead.h"
#include "src/led/DispenserLED.h"
#include "src/steppers/Rail.h"
#include "src/SharedConsts.h"
#include "src/steppers/Actuator.h"
#include "src/joystick/Joystick.h"

#define DISPENSERS 3 // Number of snus containers. Odd number only.

bool shouldHome = false; // If the joystick is used then home the carriage before taking next order.

Rail rail = Rail();
Actuator actuator = Actuator();
DispenserLED dispenserLed = DispenserLED();
Joystick joystick = Joystick(rail, actuator);

unsigned long lastRefreshTime = millis();

// For benchmarking
bool benchMarking = false;
unsigned long moveTime = 0;
unsigned long actuateTime = 0;

bool debugging = false;
bool error = false;

void setup() {
    dispenserLed.ledStrip.begin();
    dispenserLed.ledStrip.setBrightness(5);
    dispenserLed.setColor(orange);
    Serial.begin(115200);
    for (auto &stepperPin: STEPPER_PINS) {
        for (int j: stepperPin) {
            pinMode(j, OUTPUT);
        }
    }
    pinMode(joystick.getJSZ(), INPUT_PULLUP);  // Z axis is a button (Homes).
    pinMode(LIMIT_SWITCH_PIN, INPUT_PULLUP);
    home();
    rail.takeInitialRailCNYReading();
}

void loop() {
    String serialString = getSerialString();
    // Serial functionality
    if (serialString.length() != 0 || benchMarking) {
        long dispenserIndex = !benchMarking ? serialString.toInt() : random(1, DISPENSERS + 1);
        if (serialString == "benchmark") {
            benchMarking = !benchMarking;
        } else if (dispenserIndex > 0 && dispenserIndex <= DISPENSERS) {
            unsigned long now = millis();
            int timesToMove = rail.getCurrentDispenserPosition() == -1 ? dispenserIndex : dispenserIndex - abs(rail.getCurrentDispenserPosition());
            if (!triggerDispenser(dispenserIndex)) {
                dispenserLed.setColor(red);
            } else if (benchMarking) {
                Serial.printf("moves=%ld, moveTime=%ld, actuateTime=%ld, totalTime=%ld\n", abs(timesToMove), moveTime, actuateTime, millis() - now);
            }
        } else if (serialString == "info") {
            printInfo();
        } else if (serialString == "debug" || serialString == "debugging") {
            debugging = !debugging;
        } else if (serialString == "next") {
            if (!rail.moveToNextDispenser()) {
                dispenserLed.setColor(red);
            }
        } else if (serialString == "prev") {
            if (!rail.moveToPreviousDispenser()) {
                dispenserLed.setColor(red);
            }
        } else {
            // Can be used for debugging or testing new features via Serial commands.
            Serial.println(serialString);
        }
    }
    joystick.checkJoystick();
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
    Serial.println("CNY70 Rail Average: " + String(rail.readRailCNYAveraged()));
    Serial.println("CNY70 Rail Initial: " + String(rail.getInitialRailCNYReading()));
    Serial.println("CNY70 Actuator: " + String(analogRead(ACTUATOR_CNY_PIN)));
    Serial.println("Limit Switch: " + String(digitalRead(LIMIT_SWITCH_PIN)));
    Serial.println("Rail Position: " + String(rail.getRailPosition()));
    Serial.println("Dispenser Position: " + String(rail.getCurrentDispenserPosition()));
    Serial.println("Error: " + String(error));
    Serial.println("ShouldHome: " + String(shouldHome));
    Serial.println("JoyStick:");
    Serial.println("    x=" + String(analogRead(joystick.getJSX())));
    Serial.println("    y=" + String(analogRead(joystick.getJSY())));
    Serial.println("    z=" + String(digitalRead(joystick.getJSZ())));
}

bool triggerDispenser(int index) {
    if (error) return false;
    if (shouldHome) {
        dispenserLed.setColor(orange);
        if (!rail.home()) {
            dispenserLed.setColor(red);
            return false;
        } else dispenserLed.setColor(green);
    }
    shouldHome = false;

    unsigned long now = millis();
    bool movedCorrectly = rail.moveToDispenser(index);
    if (movedCorrectly && !error) {
        moveTime = millis() - now;
        now = millis();
        movedCorrectly = actuator.dispense();
        if (movedCorrectly && !error) {
            actuateTime = millis() - now;
            rail.setCurrentDispenserPosition(index);
        }
    }
    return movedCorrectly;
}

bool home() {
    dispenserLed.setColor(orange);

    if (!actuator.home() || !rail.home()) {
        dispenserLed.setColor(red);
        return false;
    }

    error = false;
    dispenserLed.setColor(green);
    return true;
}
