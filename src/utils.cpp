#include "utils.h"

// Extern global objects needed
extern TFT_eSprite img; // Sprite used by numberBox
extern LilyGo_Class amoled; // Needed by numberBox to push sprite

// Pass the sprite to draw on as an argument
void DegreesToDegMinSec(TFT_eSprite& sprite, double coordinate, bool isLat) {
    int deg = (int)coordinate;
    double minutesRemainder = abs(coordinate - deg) * 60.0;
    int arcMinutes = (int)minutesRemainder;
    double arcSeconds = (minutesRemainder - arcMinutes) * 60.0;
    char buffer[20]; // Buffer for formatted string

    if (isLat) {
        snprintf(buffer, sizeof(buffer), " %02d*%02d'%05.2f''%c", abs(deg), arcMinutes, arcSeconds, (deg < 0 ? 'S' : 'N'));
    } else {
        snprintf(buffer, sizeof(buffer), "%03d*%02d'%05.2f''%c", abs(deg), arcMinutes, arcSeconds, (deg < 0 ? 'W' : 'E'));
    }
    sprite.print(buffer); // Print to the passed sprite
}


void numberBox(int x, int y, float num) {
    // Size of sprite
    #define IWIDTH 80
    #define IHEIGHT 35

    img.fillSprite(TFT_BLACK); // Transparent background
    img.fillRoundRect(0, 0, IWIDTH, IHEIGHT, 15, TFT_RED);
    img.drawRoundRect(0, 0, IWIDTH, IHEIGHT, 15, TFT_WHITE);
    img.setTextSize(1);
    img.setTextColor(TFT_WHITE);
    img.setTextDatum(MR_DATUM);
    img.drawFloat(num, 1, 70, 20, 4); // Font 4

    amoled.pushColors(x, y, IWIDTH, IHEIGHT, (uint16_t *)img.getPointer());
}
