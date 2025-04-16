#include "display_ahi.h"

// Define colors used only here
#define blue1 0x0559
#define brown1 0x7800
#define blue2 0x0370
#define brown2 0x0187

// Static variables for AHI module
static unsigned short grays[13];
static double rad = 0.01745;
static int ax, ay;
static int gng_size_x, gng_size_y;
static int gng_center_x, gng_center_y;
static int linesAngles[6] = {0, 180, 210, 240, 300, 330};
static int dotsAngles[6] = {225, 250, 260, 280, 290, 315};
static int linesX[6], linesY[6], linesX2[6], linesY2[6], dotX[6], dotY[6], dotX2[6], dotY2[6];

// Extern global objects needed
extern TFT_eSPI tft;
extern TFT_eSprite AhrsSprite;
extern LilyGo_Class amoled;

void initAHI() {
    ax = AHRS_Width / 2;
    ay = AHRS_Height / 2;
    gng_size_x = AHRS_Width * 1.7;
    gng_size_y = AHRS_Height * 1.7;
    gng_center_x = gng_size_x / 2;
    gng_center_y = gng_size_y / 2;

    int co = 220;
    for (int i = 0; i < 13; i++) {
        grays[i] = tft.color565(co, co, co);
        co = co - 20;
    }

    for (int i = 0; i < 6; i++) {
        linesX[i] = (ax * cos(rad * (linesAngles[i]))) + ax;
        linesY[i] = (ax * sin(rad * (linesAngles[i]))) + ay;
        linesX2[i] = ((ax - 30) * cos(rad * (linesAngles[i]))) + ax;
        linesY2[i] = ((ax - 30) * sin(rad * (linesAngles[i]))) + ay;

        dotX[i] = ((ax - 16) * cos(rad * (dotsAngles[i]))) + ax;
        dotY[i] = ((ax - 16) * sin(rad * (dotsAngles[i]))) + ay;
        dotX2[i] = ((ax - 30) * cos(rad * (dotsAngles[i]))) + ax;
        dotY2[i] = ((ax - 30) * sin(rad * (dotsAngles[i]))) + ay;
    }
}

void drawHorizon(int roll, int pitch) {
    if (roll > 180)
        roll = roll - 360;

    float RollRad = -roll * rad;
    int a = tan(RollRad) * AHRS_Width / 2;
    int pitch_offset = -pitch * 2;
    if (abs(roll) > 90)
        pitch_offset = pitch * 2;

    float slope = -2.0 * a / AHRS_Width;
    int intercept = AHRS_Height / 2 + a + pitch_offset;

    if (roll == 90) {
        AhrsSprite.fillRect(0, 0, AHRS_Width / 2, AHRS_Height, AHRS_GroundColor);
    } else if (roll == -90) {
        AhrsSprite.fillRect(AHRS_Width / 2, 0, AHRS_Width / 2, AHRS_Height, AHRS_GroundColor);
    } else {
        AhrsSprite.drawWedgeLine(0, intercept, AHRS_Width, AHRS_Width * slope + intercept, 1, 1, AHRS_GroundColor);
        if (abs(roll) < 90) {
            for (int k = 0; k < AHRS_Width; k++) {
                if (k * slope + intercept < 0)
                    AhrsSprite.drawFastVLine(k, 0, AHRS_Height, AHRS_GroundColor);
                else if (k * slope + intercept < AHRS_Height)
                    AhrsSprite.drawFastVLine(k, k * slope + intercept, AHRS_Height, AHRS_GroundColor);
            }
        } else if (abs(roll) > 90) {
            for (int k = 0; k < AHRS_Width; k++) {
                if (k * slope + intercept > AHRS_Height)
                    AhrsSprite.drawFastVLine(k, 0, AHRS_Height, AHRS_GroundColor);
                else if (k * slope + intercept > 0)
                    AhrsSprite.drawFastVLine(k, 0, k * slope + intercept, AHRS_GroundColor);
            }
        }
    }

    float sinRoll = sin(RollRad);
    float cosRoll = cos(RollRad);
    AhrsSprite.drawWedgeLine(ax - sinRoll * (AHRS_Width / 2 - 46), ay - cosRoll * (AHRS_Width / 2 - 46), ax - sinRoll * (AHRS_Width / 2 - 31), ay - cosRoll * (AHRS_Width / 2 - 31), 7, 0, TFT_WHITE);
}

void drawAhiOverlay() {
    AhrsSprite.drawArc(ax, ay, 2 * ax, ax - 30, 0, 360, TFT_BLACK, blue2, false);
    AhrsSprite.drawArc(ax, ay, ax, ax - 30, 90, 270, blue2, TFT_BLACK, false);
    AhrsSprite.drawArc(ax, ay, ax, ax - 2, 0, 360, grays[5], TFT_BLACK, false);

    for (int i = 0; i < 6; i++) {
        AhrsSprite.drawWedgeLine(linesX[i], linesY[i], linesX2[i], linesY2[i], 1.5, 1.5, TFT_WHITE);
        if (i == 0 || i == 5)
            AhrsSprite.fillCircle(dotX[i], dotY[i], 4, TFT_WHITE);
        else
            AhrsSprite.drawWedgeLine(dotX[i], dotY[i], dotX2[i], dotY2[i], 1, 1, TFT_WHITE);
    }
    AhrsSprite.fillTriangle(ax - 7, 1, ax + 7, 1, ax, 20, TFT_WHITE);

    AhrsSprite.fillRect(ax - 80, ay - 1, 45, 3, TFT_YELLOW);
    AhrsSprite.fillRect(ax + 35, ay - 1, 45, 3, TFT_YELLOW);
    AhrsSprite.fillCircle(ax, ay, 4, TFT_YELLOW);
    //AhrsSprite.fillRect(ax - 30, ay - 121, 61, 3, TFT_YELLOW);
    AhrsSprite.fillRect(ax - 10, ay - 91, 21, 3, TFT_YELLOW);
    AhrsSprite.fillRect(ax - 20, ay - 61, 41, 3, TFT_YELLOW);
    AhrsSprite.fillRect(ax - 10, ay - 31, 21, 3, TFT_YELLOW);
    //AhrsSprite.fillRect(ax - 30, ay + 121, 61, 3, TFT_YELLOW);
    AhrsSprite.fillRect(ax - 10, ay + 91, 21, 3, TFT_YELLOW);
    AhrsSprite.fillRect(ax - 20, ay + 61, 41, 3, TFT_YELLOW);
    AhrsSprite.fillRect(ax - 10, ay + 31, 21, 3, TFT_YELLOW);
}

void updateAHI(float roll, float pitch) {
    AhrsSprite.fillSprite(AHRS_SkyColor);
    drawHorizon(roll, pitch * 3.0); // Apply 3x scaling
    drawAhiOverlay();
    amoled.pushColors(amoled.width() - AhrsSprite.width(), 0, AhrsSprite.width(), AhrsSprite.height(), (uint16_t *)AhrsSprite.getPointer());
}