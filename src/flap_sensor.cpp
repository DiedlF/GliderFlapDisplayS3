#include "flap_sensor.h"
#include "globals.h" // Include globals for the enum
#include <Adafruit_ADS1X15.h> // Include the ADS1X15 library header
//#include <Wire.h> // Include Wire for I2C

// Extern global variables needed/updated by this module
extern uint16_t wbkValue;
extern bool wbkChanged;
uint16_t SensorValue;

// ADS1115 instance
Adafruit_ADS1115 ads;

uint16_t flapThresholds[7];
uint16_t flapHysteresis[7];

const uint16_t defaultThresholds[7] = {458, 495, 545, 590, 810, 1150, 1800};
const uint16_t defaultHyst[7] = {4, 5, 10, 20, 30, 50, 70};

// Offsets remain internal
static int Offset1, Offset2, Offset3, Offset4, Offset5, Offset6, Offset7;

void initFlapSensor() {
    Serial.print("Initializing ADS1115... ");
    // Initialize the ADS1115
    // Default address is 0x48
    if (!ads.begin()) { // Use the correct I2C address   
        Serial.println("failed!");
        // Handle initialization failure, perhaps loop forever or set an error flag
        while (1);
    }
    Serial.println("done!");
    ads.setGain(GAIN_ONE); // Set gain to GAIN_ONE (+/-4.096V)
    //ads.setDataRate(RATE_ADS1115_128SPS); // Set data rate to 128 SPS

    Offset1 = Offset2 = Offset3 = Offset4 = Offset5 = Offset6 = Offset7 = 0; // Initialize offsets

    size_t tLen = preferences.getBytes("thresh", flapThresholds, sizeof(flapThresholds));
    size_t hLen = preferences.getBytes("hyst", flapHysteresis, sizeof(flapHysteresis));
    
    if (tLen != sizeof(flapThresholds) || hLen != sizeof(flapHysteresis)) {
        for(int i=0; i<7; i++) {
            flapThresholds[i] = defaultThresholds[i];
            flapHysteresis[i] = defaultHyst[i];
        }
    }
}

void calculateAndSaveThresholds(uint16_t* recordedValues) {
    for (int i = 0; i < 7; i++) {
        flapThresholds[i] = (recordedValues[i] + recordedValues[i+1]) / 2;
        
        int gap = abs((int)recordedValues[i+1] - (int)recordedValues[i]);
        int H = round(gap * 0.10);
        if (H < 5) H = 5;
        flapHysteresis[i] = (uint16_t)H;
    }
    
    preferences.putBytes("thresh", flapThresholds, sizeof(flapThresholds));
    preferences.putBytes("hyst", flapHysteresis, sizeof(flapHysteresis));
    Serial.println("New Flap Thresholds calculated and saved!");
}

// Reads one sample and returns the raw (clamped) value.
uint16_t GetSmoothSensorValue() {
    // static float smoothedValue = -1.0f;
    // const float alpha = 0.1f;
    // const int numReadings = 21;
    // unsigned long Sum = 0;

    // for (int i = 0; i < numReadings; i++) {
    //     Sum += analogRead(WBK_AnalogIn);
    //     delayMicroseconds(50);
    // }

    // float currentValue = (float)Sum / numReadings;

    // if (smoothedValue < 0.0f) {
    //     smoothedValue = currentValue;
    // } else {
    //     smoothedValue = alpha * currentValue + (1.0f - alpha) * smoothedValue;
    // }

    // return (uint16_t)(smoothedValue);

    // Read one sample from ADS1115 channel 0 (AIN0)
    // This call blocks for the duration of one conversion.
    int16_t currentValue = ads.readADC_SingleEnded(0);

    // Return the raw value as uint16_t. Clamp if necessary.
    return (currentValue < 0) ? 0 : ((uint16_t)currentValue>>3);
}

void updateFlapSensor() {
    // Get the latest raw value
    SensorValue = GetSmoothSensorValue();
    uint16_t oldWbkValue = wbkValue;
    uint16_t newWbkValue; // Use a temporary variable

    // Apply hysteresis using offsets
        if (SensorValue < (flapThresholds[0] + flapHysteresis[0] * Offset1)) { newWbkValue = (uint16_t)FlapPosition::S1; }
    else if (SensorValue < (flapThresholds[1] + flapHysteresis[1] * Offset2)) { newWbkValue = (uint16_t)FlapPosition::S; }
    else if (SensorValue < (flapThresholds[2] + flapHysteresis[2] * Offset3)) { newWbkValue = (uint16_t)FlapPosition::M2; }
    else if (SensorValue < (flapThresholds[3] + flapHysteresis[3] * Offset4)) { newWbkValue = (uint16_t)FlapPosition::M1; }
    else if (SensorValue < (flapThresholds[4] + flapHysteresis[4] * Offset5)) { newWbkValue = (uint16_t)FlapPosition::ZERO; }
    else if (SensorValue < (flapThresholds[5] + flapHysteresis[5] * Offset6)) { newWbkValue = (uint16_t)FlapPosition::P1; }
    else if (SensorValue < (flapThresholds[6] + flapHysteresis[6] * Offset7)) { newWbkValue = (uint16_t)FlapPosition::P2; }
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