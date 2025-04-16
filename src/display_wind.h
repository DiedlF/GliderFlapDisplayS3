#ifndef DISPLAY_WIND_H
#define DISPLAY_WIND_H

#include "globals.h"

// Declare functions
void precomputeTrigLUT();
void vektordifferenz_polar(uint16_t r_a, uint16_t phi_a_deg, uint16_t r_b, uint16_t phi_b_deg, uint16_t *r_d, uint16_t *phi_d_deg);
void drawWind(int InstDir, int InstSpeed, int AvgDir, int AvgSpeed, int Yaw);
void drawBestEnergy(int BestDir, int BestSpeed, int Yaw);
void updateWindDisplay(int InstDir, int InstSpeed, int AvgDir, int AvgSpeed, int Yaw);
void updateBestEnergyDisplay(int AvgSpeed, int AvgDir, int InstSpeed, int InstDir, int Yaw); // Note: Params reordered based on usage

#endif // DISPLAY_WIND_H