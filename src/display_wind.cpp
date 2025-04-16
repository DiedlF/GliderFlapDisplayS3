#include "display_wind.h"
#include "globals.h"
#include <math.h> // For sinf, cosf, sqrtf, atan2f, round()

// Extern global objects needed
extern TFT_eSprite WindSprite;
extern TFT_eSprite Arrow;
extern LilyGo_Class amoled;

// Global LUTs defined here (declared extern in globals.h)
float sin_lut[LUT_SIZE];
float cos_lut[LUT_SIZE];

void precomputeTrigLUT() {
    Serial.print("Precomputing Trig LUT...");
    for (int i = 0; i < LUT_SIZE; ++i) {
        float angle_rad = i * DEG_TO_RAD;
        sin_lut[i] = sinf(angle_rad);
        cos_lut[i] = cosf(angle_rad);
    }
    Serial.println(" done!");
}

void vektordifferenz_polar(uint16_t r_a, uint16_t phi_a_deg, uint16_t r_b, uint16_t phi_b_deg, uint16_t *r_d, uint16_t *phi_d_deg) {
    phi_a_deg %= 360;
    phi_b_deg %= 360;

    float A_x = r_a * cos_lut[phi_a_deg];
    float A_y = r_a * sin_lut[phi_a_deg];
    float B_x = r_b * cos_lut[phi_b_deg];
    float B_y = r_b * sin_lut[phi_b_deg];

    float D_x = A_x - B_x;
    float D_y = A_y - B_y;

    float r_d_float = sqrtf(D_x * D_x + D_y * D_y);
    float phi_d_rad = atan2f(D_y, D_x);
    float phi_d_deg_float = phi_d_rad * RAD_TO_DEG;

    if (phi_d_deg_float < 0.0f) {
        phi_d_deg_float += 360.0f;
    }

    *r_d = (uint16_t)r_d_float;
    *phi_d_deg = ((uint16_t)phi_d_deg_float) % 360;
}

// Helper function to draw the indentation on the arrow base
void drawIndentedArrowBase(TFT_eSprite &sprite, int tipX, int tipY, int base1X, int base1Y, int base2X, int base2Y, int indentDepth, uint16_t bgColor) {

    // 1. Calculate the midpoint of the base
    int midBaseX = (base1X + base2X) / 2;
    int midBaseY = (base1Y + base2Y) / 2;

    // 2. Calculate the vector from the tip to the midpoint of the base
    float vecX = midBaseX - tipX;
    float vecY = midBaseY - tipY;

    // 3. Normalize this vector (make it length 1)
    float mag = sqrt(vecX * vecX + vecY * vecY);
    if (mag == 0) return; // Avoid division by zero if points coincide
    float normVecX = vecX / mag;
    float normVecY = vecY / mag;

    // 4. Calculate the indentation point by moving from the midpoint towards the tip
    int indentX = midBaseX - round(normVecX * indentDepth);
    int indentY = midBaseY - round(normVecY * indentDepth);

    // 5. Draw the cutout triangle using the base corners and the indent point
    sprite.fillTriangle(base1X, base1Y, base2X, base2Y, indentX, indentY, bgColor);
}

void drawWind(int InstDir, int InstSpeed, int AvgDir, int AvgSpeed, int Yaw) {
    WindSprite.fillSprite(TFT_BLACK);

    // Clamp speeds for drawing
    if (InstSpeed > 60) InstSpeed = 60;
    if (InstSpeed < 2) InstSpeed = 2;
    if (AvgSpeed > 60) AvgSpeed = 60;
    if (AvgSpeed < 2) AvgSpeed = 2;

    // Define arrow geometry constants relative to ArrowSprite (200x200) center (100,100)
    const int centerX = 100;
    const int centerY = 100;
    const int baseWidth = 20; // Half-width of the arrow base
    const int indentDepth = 5; // Depth of the base indentation

    // Calculate points for Average Wind (Blue)
    int avgTipX = centerX;
    int avgTipY = centerY - round(AvgSpeed * 1.25);
    int avgBase1X = centerX - baseWidth;
    int avgBase1Y = centerY + round(AvgSpeed * 1.25);
    int avgBase2X = centerX + baseWidth;
    int avgBase2Y = avgBase1Y; // Same Y as base1

    // Draw Average Wind (Blue)
    ArrowSprite.fillSprite(TFT_BLACK); // Use TFT_BLACK as transparent color
    ArrowSprite.fillTriangle(avgTipX, avgTipY, avgBase1X, avgBase1Y, avgBase2X, avgBase2Y, TFT_SKYBLUE);
    drawIndentedArrowBase(ArrowSprite, avgTipX, avgTipY, avgBase1X, avgBase1Y, avgBase2X, avgBase2Y, indentDepth, TFT_BLACK); // Draw indentation
    ArrowSprite.pushRotated(&WindSprite, 180 + AvgDir - Yaw, TFT_BLACK); // Push with transparency

    // Calculate points for Instant Wind (White)
    int instTipX = centerX;
    int instTipY = centerY - round(InstSpeed * 1.25);
    int instBase1X = centerX - baseWidth;
    int instBase1Y = centerY + round(InstSpeed * 1.25);
    int instBase2X = centerX + baseWidth;
    int instBase2Y = instBase1Y; // Same Y as base1

    // Draw Instant Wind (White)
    ArrowSprite.fillSprite(TFT_BLACK); // Clear Arrow sprite for next drawing
    ArrowSprite.fillTriangle(instTipX, instTipY, instBase1X, instBase1Y, instBase2X, instBase2Y, TFT_WHITE);
    drawIndentedArrowBase(ArrowSprite, instTipX, instTipY, instBase1X, instBase1Y, instBase2X, instBase2Y, indentDepth, TFT_BLACK); // Draw indentation
    ArrowSprite.pushRotated(&WindSprite, 180 + InstDir - Yaw, TFT_BLACK); // Push with transparency

    // Add text after drawing arrows
    WindSprite.setCursor(200, 125);
    WindSprite.printf("%i / %i", AvgSpeed, AvgDir);
}

void drawBestEnergy(int BestDir, int BestSpeed, int Yaw) {
    WindSprite.fillSprite(TFT_BLACK); // Clears previous wind arrows

    if (BestSpeed > 2) {
        if (BestSpeed > 60) BestSpeed = 60;

    
        // Define arrow geometry constants relative to ArrowSprite (200x200) center (100,100)
        const int centerX = 100;
        const int centerY = 100;
        const int baseWidth = 20; // Use the same base width as drawWind for consistency
        const int indentDepth = 5; // Use the same indent depth

        // Calculate points for Best Energy arrow (Red)
        // Scale speed for length, similar to drawWind
        float scaledLength = BestSpeed * 1.25;
        int tipX = centerX;
        int tipY = centerY - round(scaledLength);
        int base1X = centerX - baseWidth;
        int base1Y = centerY + round(scaledLength);
        int base2X = centerX + baseWidth;
        int base2Y = base1Y; // Same Y as base1

       // Draw the main triangle
        ArrowSprite.fillSprite(TFT_BLACK); // Use TFT_BLACK as transparent color
        ArrowSprite.fillTriangle(tipX, tipY, base1X, base1Y, base2X, base2Y, TFT_RED);

        // Draw the indentation
        drawIndentedArrowBase(ArrowSprite, tipX, tipY, base1X, base1Y, base2X, base2Y, indentDepth, TFT_BLACK);
    
        // Push the arrow onto the WindSprite
        ArrowSprite.pushRotated(&WindSprite, 180 + BestDir - Yaw, TFT_BLACK); // Push with transparency
    }
}

void updateWindDisplay(int InstDir, int InstSpeed, int AvgDir, int AvgSpeed, int Yaw) {
    // Combine drawing and pushing
    drawWind(InstDir, InstSpeed, AvgDir, AvgSpeed, Yaw);
    // Push WindSprite to its location
    amoled.pushColors(0, amoled.height() - WindSprite.height(), WindSprite.width(), WindSprite.height(), (uint16_t *)WindSprite.getPointer());
}

void updateBestEnergyDisplay(int AvgSpeed, int AvgDir, int InstSpeed, int InstDir, int Yaw) {
    uint16_t r_d, phi_d;
    // Calculate difference vector (Best Energy)
    vektordifferenz_polar(InstSpeed, InstDir, AvgSpeed, AvgDir, &r_d, &phi_d);
    // Draw the best energy arrow
    drawBestEnergy(phi_d, 2 * r_d, Yaw);
    amoled.pushColors(400, 300, WindSprite.width(), 150, (uint16_t *)WindSprite.getPointer());
}