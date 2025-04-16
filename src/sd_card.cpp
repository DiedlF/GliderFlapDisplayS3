#include "sd_card.h"

// Extern global variables/objects needed/updated
extern LilyGo_Class amoled; // Needed for amoled.installSD()
extern bool sdAvail;
extern File sdFile;

// Internal function to open the next available log file
bool openSDLogFile() {
    for (unsigned int index = 0; index < 65535; index++) {
        char filename[16];
        sprintf(filename, "/GPS%05d.TXT", index);
        if (!SD.exists(filename)) {
            sdFile = SD.open(filename, FILE_WRITE);
            if (sdFile) {
                Serial.printf("LogFile opened: %s\n", filename);
                return true;
            } else {
                Serial.printf("Failed to open %s\n", filename);
                return false; // Stop trying if open fails
            }
        }
    }
    Serial.println("Could not find an available log file name.");
    return false; // No available file name found
}

bool initSDCard() {
    Serial.print("Initializing SD card...");
    if (amoled.installSD()) { // Attempt to mount SD
        Serial.print(" Mount OK. ");
        if (openSDLogFile()) { // Try to open log file
            sdAvail = true;
            Serial.println("Log file OK.");
            return true;
        } else {
            Serial.println("Log file ERROR.");
            // Optional: Unmount SD if log file failed?
            // SD.end();
            return false;
        }
    } else {
        Serial.println(" Mount FAILED.");
        return false;
    }
}