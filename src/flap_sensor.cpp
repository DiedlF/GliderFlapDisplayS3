#include "flap_sensor.h"

// Extern global variables needed/updated by this module
extern uint16_t SensorValue;
extern uint16_t wbkValue;
extern bool wbkChanged;
extern int Offset1, Offset2, Offset3, Offset4, Offset5, Offset6, Offset7;

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

    // Apply hysteresis using offsets
    if (SensorValue < (450 + 4 * Offset1)) { wbkValue = 1; }
    else if (SensorValue < 485 + 5 * Offset2) { wbkValue = 2; }
    else if (SensorValue < 535 + 10 * Offset3) { wbkValue = 3; }
    else if (SensorValue < 650 + 20 * Offset4) { wbkValue = 4; }
    else if (SensorValue < 910 + 30 * Offset5) { wbkValue = 5; }
    else if (SensorValue < 1380 + 50 * Offset6) { wbkValue = 6; }
    else if (SensorValue < 2300 + 70 * Offset7) { wbkValue = 7; }
    else { wbkValue = 8; }

    if (wbkValue != oldWbkValue) {
        wbkChanged = true;

        // Update offsets based on the new state
        Offset1 = Offset2 = Offset3 = Offset4 = Offset5 = Offset6 = Offset7 = 0; // Reset all
        switch (wbkValue) {
            case 1: Offset1 = 1; Offset2 = -1; break; // Adjust thresholds for next transitions
            case 2: Offset1 = -1; Offset2 = 1; Offset3 = -1; break;
            case 3: Offset2 = -1; Offset3 = 1; Offset4 = -1; break;
            case 4: Offset3 = -1; Offset4 = 1; Offset5 = -1; break;
            case 5: Offset4 = -1; Offset5 = 1; Offset6 = -1; break;
            case 6: Offset5 = -1; Offset6 = 1; Offset7 = -1; break;
            case 7: Offset6 = -1; Offset7 = 1; break;
            case 8: Offset7 = -1; break;
        }
         // Serial.printf("New WBK: %d, Sensor: %d\n", wbkValue, SensorValue);
         // Serial.printf("Offsets: %d,%d,%d,%d,%d,%d,%d\n", Offset1, Offset2, Offset3, Offset4, Offset5, Offset6, Offset7);
    }
}