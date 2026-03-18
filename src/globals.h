#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include "TFT_eSPI.h"
#include <LilyGo_AMOLED.h>
#include <AceButton.h>
#include <TinyGPS++.h>
#include <Preferences.h>
#include <SD.h> // Include SD library header

// Define Enums for Modes and Flap Positions
enum class Mode : uint16_t {
    STANDARD = 0,
    INFO = 1,
    CAL = 2
};

enum class CalState : uint8_t { INIT, WAIT_S1, WAIT_S, WAIT_M2, WAIT_M1, WAIT_ZERO, WAIT_P1, WAIT_P2, WAIT_L, DONE };

enum class FlapPosition : uint16_t {
    UNKNOWN = 0, // Or a default/error state
    S1 = 1,      // Speed 1
    S = 2,       // Speed
    M2 = 3,      // Minus 2
    M1 = 4,      // Minus 1
    ZERO = 5,    // Zero
    P1 = 6,      // Plus 1
    P2 = 7,      // Plus 2
    L = 8        // Landing
};

// Declare global objects as extern
extern LilyGo_Class amoled;
extern TFT_eSPI tft;
extern TFT_eSprite TftSprite;
extern TFT_eSprite WbkSprite;
extern TFT_eSprite AhrsSprite;
extern TFT_eSprite WindSprite;
extern TFT_eSprite ArrowSprite;
extern TFT_eSprite img; // For numberBox

extern Preferences preferences;
extern ace_button::AceButton button;

// GPS Objects
extern TinyGPSPlus Butterfly;
extern TinyGPSCustom lxwp0Speed;
extern TinyGPSCustom lxwp0Alt;
extern TinyGPSCustom lxwp0Heading;
extern TinyGPSCustom lxwp0WindDirection;
extern TinyGPSCustom lxwp0WindSpeed;
extern TinyGPSPlus Larus;
extern TinyGPSCustom LarusWindAngle;
extern TinyGPSCustom LarusWindSpeed;
extern TinyGPSCustom LarusWindType;
extern TinyGPSCustom LarusWindStatus;
extern TinyGPSCustom LarusRoll;
extern TinyGPSCustom LarusPitch;
extern TinyGPSCustom LarusYaw;
extern TinyGPSCustom LarusAirDensity;
extern TinyGPSCustom LarusAirDensityStatus;
extern TinyGPSCustom LarusBatVoltage;
extern TinyGPSCustom LarusVario;
extern TinyGPSCustom LarusVarioAvg;
extern TinyGPSCustom LarusAltitude;
extern TinyGPSCustom LarusSpeed;

// Declare global variables as extern
extern volatile bool ModeChanged;
extern uint16_t mode; // 0: standard, 1: info, 2: setup
extern uint16_t wbkValue; //S1:1 S:2 -2:3 -1:4 0:5 +1:6 +2:7 L:8
extern bool wbkChanged;
extern uint16_t TftBrightness;
extern bool BrightnessChanged;
extern bool LarusWindChanged;
extern uint16_t LarusWindSpeedInst, LarusWindDirInst, LarusWindSpeedAvg, LarusWindDirAvg;

// Calibration tracking
extern CalState calState;
extern volatile bool calConfirmPressed;
extern uint16_t flapThresholds[7];
extern uint16_t flapHysteresis[7];

// SD Card related
extern bool sdAvail;
extern File sdFile;
extern unsigned long recDataKB;

// Flap Sensor related
extern uint16_t SensorValue;

// Wind display related (LUTs)
#define LUT_SIZE 360
extern float sin_lut[LUT_SIZE];
extern float cos_lut[LUT_SIZE];

#endif // GLOBALS_H