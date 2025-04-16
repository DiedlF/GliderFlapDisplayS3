#include "gps.h"

// Extern global objects/variables needed/updated
extern TinyGPSPlus Butterfly;
extern TinyGPSPlus Larus;
extern TinyGPSCustom LarusWindAngle;
extern TinyGPSCustom LarusWindSpeed;
extern TinyGPSCustom LarusWindType;
extern TinyGPSCustom LarusWindStatus;
extern bool LarusWindChanged;
extern uint16_t LarusWindSpeedInst, LarusWindDirInst, LarusWindSpeedAvg, LarusWindDirAvg;

extern bool sdAvail;
extern File sdFile;
extern unsigned long recDataKB;

void initGPS() {
    Serial1.begin(38400, SERIAL_8N1, RX_Butterfly, 40); // Connection to Butterfly Vario
    Serial2.begin(38400, SERIAL_8N1, RX_Larus, 40);     // Connection to Larus
    Serial.println("GPS Serial ports initialized.");
}

void processGPS() {
    static int byteCount = 0;

    // Process Butterfly GPS data
    while (Serial1.available() > 0) {
        char c = Serial1.read();
        Butterfly.encode(c);
    }

    // Process Larus GPS data and log to SD
    while (Serial2.available() > 0) {
        char c = Serial2.read();

        // Log raw NMEA data to SD card if available
        if (sdAvail && sdFile) { // Check if sdFile is valid
            sdFile.write(c);
            if (++byteCount >= 1024) { // Flush every 1KB
                sdFile.flush();
                byteCount = 0;
                recDataKB++;
            }
        }
        // Encode data for TinyGPSPlus
        Larus.encode(c);
    }

    // Check for Larus Wind updates
    if (LarusWindAngle.isUpdated() || LarusWindSpeed.isUpdated()) {
        if (!strcmp(LarusWindStatus.value(), "A")) { // Check if status is Active
            LarusWindChanged = true; // Signal that wind data might have changed
            if (!strcmp(LarusWindType.value(), "I")) { // Instantaneous wind
                LarusWindSpeedInst = atoi(LarusWindSpeed.value());
                LarusWindDirInst = atoi(LarusWindAngle.value());
            } else { // Average wind (assuming anything else is average)
                LarusWindSpeedAvg = atoi(LarusWindSpeed.value());
                LarusWindDirAvg = atoi(LarusWindAngle.value());
            }
        } else {
            // Wind data is not active/valid, potentially clear or mark as invalid?
            // For now, just read the values to clear the updated flag
             LarusWindAngle.value();
             LarusWindSpeed.value();
             // Consider setting LarusWindChanged = true here as well if the display
             // should react to invalid wind data (e.g., show dashes).
        }
    }
}