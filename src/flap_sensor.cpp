#include "flap_sensor.h"
#include "globals.h" // Include globals for the enum

// Extern global variables needed/updated by this module
extern uint16_t wbkValue;
extern bool wbkChanged;
uint16_t SensorValue;

// Offsets remain internal
static int Offset1, Offset2, Offset3, Offset4, Offset5, Offset6, Offset7;

void initFlapSensor() {
    // Add any sensor-specific initialization if required
    // pinMode(WBK_AnalogIn, INPUT); // Already default for analogRead
    Offset1 = Offset2 = Offset3 = Offset4 = Offset5 = Offset6 = Offset7 = 0; // Initialize offsets
}

uint16_t GetSmoothSensorValue() {
    static float smoothedValue = -1.0f;
    const float alpha = 0.1f;
    const int numReadings = 21;
    unsigned long Sum = 0;

    for (int i = 0; i < numReadings; i++) {
        Sum += analogRead(WBK_AnalogIn);
        delayMicroseconds(50);
    }

    float currentValue = (float)Sum / numReadings;

    if (smoothedValue < 0.0f) {
        smoothedValue = currentValue;
    } else {
        smoothedValue = alpha * currentValue + (1.0f - alpha) * smoothedValue;
    }

    return (uint16_t)(smoothedValue);
}

void updateFlapSensor() {
    SensorValue = GetSmoothSensorValue();
    uint16_t oldWbkValue = wbkValue;
    uint16_t newWbkValue; // Use a temporary variable

    // Apply hysteresis using offsets
    if (SensorValue < (450 + 4 * Offset1)) { newWbkValue = (uint16_t)FlapPosition::S1; }
    else if (SensorValue < 485 + 5 * Offset2) { newWbkValue = (uint16_t)FlapPosition::S; }
    else if (SensorValue < 535 + 10 * Offset3) { newWbkValue = (uint16_t)FlapPosition::M2; }
    else if (SensorValue < 650 + 20 * Offset4) { newWbkValue = (uint16_t)FlapPosition::M1; }
    else if (SensorValue < 910 + 30 * Offset5) { newWbkValue = (uint16_t)FlapPosition::ZERO; }
    else if (SensorValue < 1380 + 50 * Offset6) { newWbkValue = (uint16_t)FlapPosition::P1; }
    else if (SensorValue < 2300 + 70 * Offset7) { newWbkValue = (uint16_t)FlapPosition::P2; }
    else { newWbkValue = (uint16_t)FlapPosition::L; }

    // Check if value changed
    if (newWbkValue != oldWbkValue) {
        wbkValue = newWbkValue; // Update the global variable
        wbkChanged = true;

        // Update offsets based on the new state
        Offset1 = Offset2 = Offset3 = Offset4 = Offset5 = Offset6 = Offset7 = 0; // Reset all
        switch ((FlapPosition)wbkValue) { // Cast to enum for switch
            case FlapPosition::S1: Offset1 = 1; Offset2 = -1; break; // Adjust thresholds for next transitions
            case FlapPosition::S: Offset1 = -1; Offset2 = 1; Offset3 = -1; break;
            case FlapPosition::M2: Offset2 = -1; Offset3 = 1; Offset4 = -1; break;
            case FlapPosition::M1: Offset3 = -1; Offset4 = 1; Offset5 = -1; break;
            case FlapPosition::ZERO: Offset4 = -1; Offset5 = 1; Offset6 = -1; break;
            case FlapPosition::P1: Offset5 = -1; Offset6 = 1; Offset7 = -1; break;
            case FlapPosition::P2: Offset6 = -1; Offset7 = 1; break;
            case FlapPosition::L: Offset7 = -1; break;
            case FlapPosition::UNKNOWN: // Fall through or handle default
            default: break; // No offsets needed or handle error
        }
         // Serial.printf("New WBK: %d, Sensor: %d\n", wbkValue, SensorValue);
         // Serial.printf("Offsets: %d,%d,%d,%d,%d,%d,%d\n", Offset1, Offset2, Offset3, Offset4, Offset5, Offset6, Offset7);
    }
}