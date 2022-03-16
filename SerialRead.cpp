#include "SerialRead.h"

String getSerialString() {
    static String inputString;
    static bool stringComplete;
    String toReturn = "";
    if (Serial.available()) {
        char inChar = Serial.read();
        inputString += inChar;
        if (inChar == '\n') {
            stringComplete = true;
        }
    }
    if (stringComplete) {
        inputString.trim();
        toReturn = inputString;
        inputString = "";
        stringComplete = false;
    }
    return toReturn;
}