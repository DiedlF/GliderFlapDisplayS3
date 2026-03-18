#ifndef FLAP_SENSOR_H
#define FLAP_SENSOR_H

#include "globals.h"

// Declare functions
void initFlapSensor(); // Placeholder if initialization needed
void updateFlapSensor();
uint16_t GetSmoothSensorValue(); // Keep internal or expose if needed elsewhere
void calculateAndSaveThresholds(uint16_t* recordedValues);

#endif // FLAP_SENSOR_H