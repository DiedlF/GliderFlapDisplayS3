#ifndef DISPLAY_MAIN_H
#define DISPLAY_MAIN_H

#include "globals.h"
#include "imagedata.h" // Include bitmap data

// Declare functions
void updateWbkDisplay(uint16_t flapValue, bool isWarning = false);
void updateInfoDisplay();
void updateCalDisplay(CalState state, uint16_t currentSensorValue);

#endif // DISPLAY_MAIN_H