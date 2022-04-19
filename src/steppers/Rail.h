//
// Created by bemma on 19-04-2022.
//

#ifndef DISPENSERPROJECT_RAIL_H
#define DISPENSERPROJECT_RAIL_H

#include "Arduino.h"
#include "Stepper.h"
#include "../SharedConsts.h"

class Rail {
private:
    int initialRailCNYReading = 4096;
    int currentDispenserPosition = -1;
    int railPosition = 0;
public:
    Rail();
    bool triggerDispenser(int index);
    bool moveToNextDispenser();
    bool moveToPreviousDispenser();
    bool moveTillThreshold(int stepsBetweenChecks);
    bool moveToDispenser(int dispenserIndex);
    bool step(int steps);
    int readRailCNYAveraged(int millis);
    int readRailCNYAveraged();
    void takeInitialRailCNYReading();
    bool home();
    int getRailPosition() {
        return railPosition;
    }
    int getInitialRailCNYReading() {
        return initialRailCNYReading;
    }
    int getCurrentDispenserPosition() {
        return currentDispenserPosition;
    }
    void setCurrentDispenserPosition(int currentDispenserPosition) {
        this->currentDispenserPosition = currentDispenserPosition;
    }
};


#endif //DISPENSERPROJECT_RAIL_H
