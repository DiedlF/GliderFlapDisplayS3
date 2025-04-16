#include "display_main.h"
#include "utils.h" // For DegreesToDegMinSec

// Extern global objects/variables needed
extern TFT_eSprite WbkSprite;
extern TFT_eSprite TftSprite; // Used for Info Display
extern LilyGo_Class amoled;
extern TinyGPSPlus Larus;
extern TinyGPSPlus Butterfly;
// Include other GPS custom objects if needed for info display
extern TinyGPSCustom lxwp0Speed;
extern TinyGPSCustom lxwp0Alt;
extern TinyGPSCustom lxwp0Heading;
extern TinyGPSCustom lxwp0WindDirection;
extern TinyGPSCustom lxwp0WindSpeed;
extern TinyGPSCustom LarusWindAngle; // Already in globals.h
extern TinyGPSCustom LarusWindSpeed; // Already in globals.h
extern TinyGPSCustom LarusRoll;
extern TinyGPSCustom LarusPitch;
extern TinyGPSCustom LarusYaw;
extern TinyGPSCustom LarusVario;
extern TinyGPSCustom LarusVarioAvg;
extern TinyGPSCustom LarusAltitude;
extern TinyGPSCustom LarusSpeed; // Larus speed from PLARV
extern TinyGPSCustom LarusBatVoltage;
extern TinyGPSCustom LarusAirDensity;

extern uint16_t LarusWindDirInst, LarusWindSpeedInst, LarusWindDirAvg, LarusWindSpeedAvg;
extern unsigned long recDataKB;
extern uint16_t TftBrightness;
extern uint16_t SensorValue;


void updateWbkDisplay(uint16_t flapValue) {
    String SpeedString;
    WbkSprite.fillSprite(TFT_BLACK);
    const uint8_t* bitmapData = nullptr;

    switch (flapValue) {
        case 1: bitmapData = S1_DATA; SpeedString = "210 - 280"; break;
        case 2: bitmapData = S_DATA; SpeedString = "190 - 210"; break;
        case 3: bitmapData = M2_DATA; SpeedString = "170 - 190"; break;
        case 4: bitmapData = M1_DATA; SpeedString = "135 - 170"; break;
        case 5: bitmapData = Zero_DATA; SpeedString = "100 - 135"; break;
        case 6: bitmapData = P1_DATA; SpeedString = "90 - 100"; break;
        case 7: bitmapData = P2_DATA; SpeedString = "85 - 90"; break;
        case 8: bitmapData = L_DATA; SpeedString = "   < 85"; break;
        default: SpeedString = "---"; break; // Handle unexpected value
    }

    if (bitmapData) {
        WbkSprite.drawBitmap(0, 0, bitmapData, 240, 170, TFT_BLACK, TFT_WHITE);
    }
    WbkSprite.setCursor(30, 220); // Position from original code
    WbkSprite.print(SpeedString);
    amoled.pushColors(0, 0, WbkSprite.width(), WbkSprite.height(), (uint16_t *)WbkSprite.getPointer());
}

void updateInfoDisplay() {
    TftSprite.fillSprite(TFT_WHITE); // Use the main TftSprite for info
    TftSprite.setTextColor(TFT_BLACK); // Ensure text is visible on white background
    TftSprite.setTextFont(2); // Set font
    TftSprite.setTextSize(1); // Set size

    int col1 = 5;
    int col2 = 100;
    int col3 = 250;
    int col4 = 400;
    int y = 0;
    int yStep = TftSprite.fontHeight(); // Spacing based on font height

    // --- Column Headers ---
    TftSprite.setCursor(col2, y); TftSprite.print("Larus");
    TftSprite.setCursor(col3, y); TftSprite.print("Butterfly");
    y += yStep * 2;

    // --- Row Labels ---
    int startY = y;
    TftSprite.setCursor(col1, y); TftSprite.print("Date"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("Time"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("Latitude"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("Longitude"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("Satellites"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("HDOP"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("Speed"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("Course"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("Altitude"); y += yStep;
    y += yStep; // Spacer
    TftSprite.setCursor(col1, y); TftSprite.print("WindSpeed"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("WindDir"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("Speed"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("Altitude"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("Yaw"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("Roll / Pitch"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("Vario"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("VarioAvg"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("BattVolt"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("AirDensity"); y += yStep;
    y += yStep; // Spacer
    TftSprite.setCursor(col1, y); TftSprite.print("CharsProc"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("SentWFix"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("FailChk"); y += yStep;
    TftSprite.setCursor(col1, y); TftSprite.print("PassChk"); y += yStep;


    // --- Larus Data (Column 2) ---
    y = startY;
    TftSprite.setCursor(col2, y); TftSprite.printf("%02d/%02d/%02d", Larus.date.day(), Larus.date.month(), Larus.date.year() % 100); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.printf("%02d:%02d:%02d", Larus.time.hour(), Larus.time.minute(), Larus.time.second()); y += yStep;
    TftSprite.setCursor(col2 - 15, y); DegreesToDegMinSec(TftSprite, Larus.location.lat(), true); y += yStep;
    TftSprite.setCursor(col2 - 15, y); DegreesToDegMinSec(TftSprite, Larus.location.lng(), false); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(Larus.satellites.value()); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(Larus.hdop.value()); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(Larus.speed.kmph(), 0); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(Larus.course.deg(), 0); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(Larus.altitude.meters(), 0); y += yStep;
    y += yStep; // Spacer
    TftSprite.setCursor(col2, y); TftSprite.printf("%d / %d", LarusWindSpeedInst, LarusWindSpeedAvg); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.printf("%d / %d", LarusWindDirInst, LarusWindDirAvg); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(LarusSpeed.value()); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(LarusAltitude.value()); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(LarusYaw.value()); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.printf("%s / %s", LarusRoll.value(), LarusPitch.value()); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(LarusVario.value()); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(LarusVarioAvg.value()); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(LarusBatVoltage.value()); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(LarusAirDensity.value()); y += yStep;
    y += yStep; // Spacer
    TftSprite.setCursor(col2, y); TftSprite.print(Larus.charsProcessed()); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(Larus.sentencesWithFix()); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(Larus.failedChecksum()); y += yStep;
    TftSprite.setCursor(col2, y); TftSprite.print(Larus.passedChecksum()); y += yStep;


    // --- Butterfly Data (Column 3) ---
    y = startY;
    TftSprite.setCursor(col3, y); TftSprite.printf("%02d/%02d/%02d", Butterfly.date.day(), Butterfly.date.month(), Butterfly.date.year() % 100); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.printf("%02d:%02d:%02d", Butterfly.time.hour(), Butterfly.time.minute(), Butterfly.time.second()); y += yStep;
    TftSprite.setCursor(col3 - 15, y); DegreesToDegMinSec(TftSprite, Butterfly.location.lat(), true); y += yStep;
    TftSprite.setCursor(col3 - 15, y); DegreesToDegMinSec(TftSprite, Butterfly.location.lng(), false); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.print(Butterfly.satellites.value()); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.print(Butterfly.hdop.value()); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.print(Butterfly.speed.kmph(), 0); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.print(Butterfly.course.deg(), 0); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.print(Butterfly.altitude.meters(), 0); y += yStep;
    y += yStep; // Spacer
    TftSprite.setCursor(col3, y); TftSprite.print(lxwp0WindSpeed.value()); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.print(lxwp0WindDirection.value()); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.print(lxwp0Speed.value()); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.print(lxwp0Alt.value()); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.print(lxwp0Heading.value()); y += yStep;
    // Skip Larus specific rows
    y += 6 * yStep; // Skip Roll/Pitch, Vario, VarioAvg, BattVolt, AirDensity, 2 spacers
    TftSprite.setCursor(col3, y); TftSprite.print(Butterfly.charsProcessed()); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.print(Butterfly.sentencesWithFix()); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.print(Butterfly.failedChecksum()); y += yStep;
    TftSprite.setCursor(col3, y); TftSprite.print(Butterfly.passedChecksum()); y += yStep;


    // --- Supplement Data (Column 4) ---
    y = startY;
    TftSprite.setCursor(col4, y); TftSprite.printf("recData: %d kB",recDataKB); y += yStep;
    TftSprite.setCursor(col4, y); TftSprite.printf("TftBrightness: %d", TftBrightness); y += yStep;
    TftSprite.setCursor(col4, y); TftSprite.printf("SensorValue: %d", SensorValue); y += yStep;

    // Push the completed info screen
    amoled.pushColors(0, 0, TftSprite.width(), TftSprite.height(), (uint16_t *)TftSprite.getPointer());
}