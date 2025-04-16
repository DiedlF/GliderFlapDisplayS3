#ifndef DISPLAY_AHI_H
#define DISPLAY_AHI_H

#include "globals.h" // Includes Arduino, TFT_eSPI etc.

// Define AHI specific constants
#define AHRS_Height 300
#define AHRS_Width 300
#define AHRS_SkyColor TFT_SKYBLUE
#define AHRS_GroundColor brown1 // TFT_DARKGREEN

// Declare functions
void initAHI();
void drawHorizon(int roll, int pitch);
void drawAhiOverlay();
void updateAHI(float roll, float pitch);

#endif // DISPLAY_AHI_H