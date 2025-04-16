#ifndef UTILS_H
#define UTILS_H

#include "globals.h" // Includes TFT_eSprite for numberBox

// Declare functions
void DegreesToDegMinSec(TFT_eSprite& sprite, double coordinate, bool isLat); // Pass sprite by reference
void numberBox(int x, int y, float num);

#endif // UTILS_H