#ifndef FLAP_SENSOR_H
#define FLAP_SENSOR_H

#include "globals.h"

#define WBK_AnalogIn 18 // Define pin here

// Declare functions
void initFlapSensor(); // Placeholder if initialization needed
void updateFlapSensor();
uint16_t GetSmoothSensorValue(); // Keep internal or expose if needed elsewhere

#endif // FLAP_SENSOR_H