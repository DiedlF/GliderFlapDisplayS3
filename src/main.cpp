// TODOs:
// Kalibrier-Routine
// Larus True Heading HCHDT
// SpeedToFly Pfeil
// Update auf Info-Seite nur für Values
// Config-Management auf SD-Karte (Kalibrierung, Wind-Skala, Demo-Mode,...)
// Named Constants für Pin-Nummern, Colors, ...

const bool DEMO_MODE = false; // <<< Set to true to activate demo mode

#include "globals.h"
#include "display_ahi.h"
#include "display_wind.h"
#include "flap_sensor.h"
#include "gps.h"
#include "button_handler.h"
#include "sd_card.h"
#include "utils.h"
#include "display_main.h"
#include <math.h> // Include for sin()


// --- Define global objects and variables (initialized here, declared extern in globals.h) ---
LilyGo_Class amoled;
TFT_eSPI tft = TFT_eSPI(); // Initialize TFT object
TFT_eSprite TftSprite = TFT_eSprite(&tft); // Initialize sprites using the tft object
TFT_eSprite WbkSprite = TFT_eSprite(&tft);
TFT_eSprite AhrsSprite = TFT_eSprite(&tft);
TFT_eSprite WindSprite = TFT_eSprite(&tft);
TFT_eSprite ArrowSprite = TFT_eSprite(&tft);
TFT_eSprite img = TFT_eSprite(&tft); // For numberBox

Preferences preferences;
ace_button::AceButton button;

// GPS Objects
TinyGPSPlus Butterfly;
TinyGPSCustom lxwp0Speed(Butterfly, "LXWP0", 2);
TinyGPSCustom lxwp0Alt(Butterfly, "LXWP0", 3);
TinyGPSCustom lxwp0Heading(Butterfly, "LXWP0", 10);
TinyGPSCustom lxwp0WindDirection(Butterfly, "LXWP0", 11);
TinyGPSCustom lxwp0WindSpeed(Butterfly, "LXWP0", 12);
TinyGPSPlus Larus;
TinyGPSCustom LarusWindAngle(Larus, "PLARW", 1);
TinyGPSCustom LarusWindSpeed(Larus, "PLARW", 2);
TinyGPSCustom LarusWindType(Larus, "PLARW", 3);
TinyGPSCustom LarusWindStatus(Larus, "PLARW", 4);
TinyGPSCustom LarusRoll(Larus, "PLARA", 1);
TinyGPSCustom LarusPitch(Larus, "PLARA", 2);
TinyGPSCustom LarusYaw(Larus, "PLARA", 3);
TinyGPSCustom LarusAirDensity(Larus, "PLARD", 1);
TinyGPSCustom LarusAirDensityStatus(Larus, "PLARD", 2);
TinyGPSCustom LarusBatVoltage(Larus, "PLARB", 1);
TinyGPSCustom LarusVario(Larus, "PLARV", 1);
TinyGPSCustom LarusVarioAvg(Larus, "PLARV", 2);
TinyGPSCustom LarusAltitude(Larus, "PLARV", 3);
TinyGPSCustom LarusSpeed(Larus, "PLARV", 4);

// Global Variables
volatile bool ModeChanged = false;
uint16_t mode = (uint16_t)Mode::STANDARD; // Use enum for initial mode, cast needed for now
uint16_t wbkValue = (uint16_t)FlapPosition::UNKNOWN; // Use enum for initial state, cast needed
bool wbkChanged = true; // Start true to force initial draw
uint16_t TftBrightness = 255;
bool BrightnessChanged = false;
bool LarusWindChanged = true; // Start true to force initial draw
uint16_t LarusWindSpeedInst = 0, LarusWindDirInst = 0, LarusWindSpeedAvg = 0, LarusWindDirAvg = 0;

// SD Card related
bool sdAvail = false;
File sdFile;


// --- Helper Function for Demo Values ---
void updateDemoValues(float &roll, float &pitch, int &yaw,
                      uint16_t &windSpeedInst, uint16_t &windDirInst,
                      uint16_t &windSpeedAvg, uint16_t &windDirAvg,
                      uint16_t &wbkValue)
{
    unsigned long timeMs = millis();

    // Calculate demo values using sine waves for smooth variation
    roll = 45.0 * sin(timeMs * 0.0005); // Varies between -45 and 45 over ~12.5s
    pitch = 15.0 * sin(timeMs * 0.0008); // Varies between -15 and 15 over ~7.8s
    yaw = (timeMs / 50) % 360; // Sweeps 0-359 degrees every ~18 seconds

    float speedAvgBase = 25.0 + 5.0 * sin(timeMs * 0.0003);
    windSpeedAvg = (speedAvgBase < 0) ? 0 : (uint16_t)speedAvgBase;
    windDirAvg = (180 + (int)(90.0 * sin(timeMs * 0.0002))) % 360;

    float speedInstBase = (float)windSpeedAvg + 2.0 * sin(timeMs * 0.0015);
    windSpeedInst = (speedInstBase < 0) ? 0 : (uint16_t)speedInstBase;
    windDirInst = (windDirAvg + (int)(15.0 * sin(timeMs * 0.001))) % 360;

    int numFlapPositions = 8; // Number of actual flap positions (1-8)
    // Cycle through FlapPosition enum values 1 through 8
    wbkValue = (uint16_t)(((timeMs / 2000) % numFlapPositions) + (uint16_t)FlapPosition::S1);
}

// --- Setup ---
void setup() {
    Serial.begin(115200);
    delay(2000); // Wait for hardware initialization
    Serial.println("Starting Setup...");

    Serial.print("Initializing EEPROM...");
    preferences.begin("GliderFlap", false); // Namespace "GliderFlap"
    TftBrightness = preferences.getUChar("Brightness", 255); // Load brightness
    Serial.println(" done!");

    Serial.print("Initializing Display...");
    if (amoled.beginAMOLED_241(true, true)) { // Initialize AMOLED display
        amoled.setBrightness(TftBrightness); // Set loaded brightness
        Serial.println(" done!");
    } else {
        Serial.println(" failed!");
        // Handle display initialization failure (e.g., halt or retry)
        while(1) delay(100);
    }

    // Initialize Sprites (must be done after TFT init)
    Serial.print("Creating Sprites...");
    TftSprite.createSprite(amoled.width(), amoled.height()); // Main sprite for info mode
    TftSprite.setTextFont(2); // Default font for info sprite
    TftSprite.setTextSize(1);

    AhrsSprite.createSprite(AHRS_Width, AHRS_Height); // AHI sprite

    WbkSprite.createSprite(250, 270); // Flap sprite
    WbkSprite.setTextFont(4);
    WbkSprite.setTextSize(2);
    WbkSprite.setTextColor(TFT_WHITE);

    WindSprite.createSprite(350, 180); // Wind sprite
    WindSprite.setTextFont(4);
    WindSprite.setTextSize(1);
    WindSprite.setTextColor(TFT_WHITE);
    WindSprite.setPivot(90, 100); // Set pivot for rotation

    ArrowSprite.createSprite(200, 200); // Arrow sprite used by wind display
    ArrowSprite.fillSprite(TFT_BLACK); // Set default background (used as transparent color)

    img.createSprite(80, 35); // Sprite for numberBox utility

    Serial.println(" done!");

    // Initialize Modules
    initGPS();          // Initialize GPS serial ports
    initSDCard();       // Initialize SD card and open log file
    initButton();       // Initialize button handler
    initAHI();          // Initialize AHI display elements
    precomputeTrigLUT();// Initialize Wind display LUTs
    initFlapSensor();   // Initialize flap sensor reading

    Serial.println("Setup Complete.");
    // Initial screen clear or splash screen?
    TftSprite.fillSprite(TFT_BLACK);
    amoled.pushColors(0,0, TftSprite.width(), TftSprite.height(), (uint16_t *)TftSprite.getPointer());
}

// --- Main Loop ---
void loop() {
  // 1. Process Inputs (Always run)
  checkButton();
  processGPS();       // Reads sensors/GPS, updates Larus... globals
  updateFlapSensor(); // Reads sensor, updates wbkValue global

  // 2. Handle System State Changes
  if (BrightnessChanged) {
      BrightnessChanged = false; // Reset flag
      preferences.putUChar("Brightness", TftBrightness);
      amoled.setBrightness(TftBrightness);
      Serial.printf("Brightness updated to %d\n", TftBrightness);
  }

  if (ModeChanged) {
      ModeChanged = false; // Reset flag
      // Clear screen before drawing new mode
      TftSprite.fillSprite(TFT_BLACK);
      amoled.pushColors(0, 0, TftSprite.width(), TftSprite.height(), (uint16_t *)TftSprite.getPointer());
      // Force redraw of all elements in the new mode
      wbkChanged = true;
      LarusWindChanged = true;
      Serial.printf("Mode changed to %d. Forcing redraw.\n", mode);
  }

  // --- Prepare values for display update ---
  bool forceAhiUpdate = false;
  bool forceWindUpdate = false;
  bool forceWbkUpdate = false;

  // Local variables for display values
  float currentRoll = 0.0;
  float currentPitch = 0.0;
  int currentYaw = 0;
  uint16_t currentWbkValue = 0;
  bool currentWbkWarning = false;
  uint16_t currentWindSpeedInst = 0, currentWindDirInst = 0, currentWindSpeedAvg = 0, currentWindDirAvg = 0;


  if (DEMO_MODE && mode == (uint16_t)Mode::STANDARD) { // Use enum constant with cast
      // Generate demo values using the helper function
      updateDemoValues(currentRoll, currentPitch, currentYaw,
                       currentWindSpeedInst, currentWindDirInst,
                       currentWindSpeedAvg, currentWindDirAvg,
                       currentWbkValue);

      // Force display updates in demo mode
      forceAhiUpdate = true;
      forceWindUpdate = true;
      forceWbkUpdate = true;

  } else { // Normal Operation (or other modes)
      // Determine if updates are needed based on actual changes
      forceAhiUpdate = LarusRoll.isUpdated() || LarusPitch.isUpdated() || wbkChanged; // wbk change forces AHI redraw
      forceWindUpdate = LarusYaw.isUpdated() || LarusWindChanged;
      forceWbkUpdate = wbkChanged;

      // Assign real values from sensors/GPS
      currentRoll = -atof(LarusRoll.value());
      currentPitch = -atof(LarusPitch.value());
      currentYaw = atoi(LarusYaw.value());
      // Use global wind values updated by processGPS()
      currentWindDirAvg = LarusWindDirAvg;
      currentWindSpeedAvg = LarusWindSpeedAvg;
      currentWindDirInst = LarusWindDirInst;
      currentWindSpeedInst = LarusWindSpeedInst;
      // Use global flap value updated by updateFlapSensor()
      currentWbkValue = wbkValue;

      // --- Flap Speed Warning Logic ---
      static unsigned long warningStartTime = 0;
      static bool warningConditionActive = false;
      static bool lastWbkWarning = false;
      
      bool currentWarningCondition = false;
      if (currentWbkValue >= 1 && currentWbkValue <= 5) {
          int airSpeed = -1;
          if (strlen(LarusSpeed.value()) > 0) {
              float s = atof(LarusSpeed.value());
              if (s > 0) airSpeed = (int)s;
          }
          if (airSpeed == -1 && strlen(lxwp0Speed.value()) > 0) {
              float s = atof(lxwp0Speed.value());
              if (s > 0) airSpeed = (int)s;
          }

          if (airSpeed > 0) {
              switch (currentWbkValue) {
                  case 1: currentWarningCondition = (airSpeed < 210 || airSpeed > 280); break;
                  case 2: currentWarningCondition = (airSpeed < 190 || airSpeed > 210); break;
                  case 3: currentWarningCondition = (airSpeed < 170 || airSpeed > 190); break;
                  case 4: currentWarningCondition = (airSpeed < 135 || airSpeed > 170); break;
                  case 5: currentWarningCondition = (airSpeed < 100 || airSpeed > 135); break;
              }
          }
      }

      if (currentWarningCondition) {
          if (!warningConditionActive) {
              warningConditionActive = true;
              warningStartTime = millis();
          } else if (millis() - warningStartTime > 10000) {
              currentWbkWarning = true;
          }
      } else {
          warningConditionActive = false;
      }
      
      if (currentWbkWarning != lastWbkWarning) {
          forceWbkUpdate = true;
          lastWbkWarning = currentWbkWarning;
      }
  }


  // 3. Update Display Based on Mode
  switch ((Mode)mode) { // Cast mode to enum for switch
      case Mode::STANDARD: // Use enum constant
          // Update AHI if needed
          if (forceAhiUpdate) {
               updateAHI(currentRoll, currentPitch);
          }

          // Update Wind displays if needed
          if (forceWindUpdate) {
              LarusWindChanged = false; // Reset flag only when update happens
              updateWindDisplay(currentWindDirInst, currentWindSpeedInst, currentWindDirAvg, currentWindSpeedAvg, currentYaw);
              updateBestEnergyDisplay(currentWindSpeedAvg, currentWindDirAvg, currentWindSpeedInst, currentWindDirInst, currentYaw);
           }

          // Update Flap display if needed
          if (forceWbkUpdate) {
              wbkChanged = false; // Reset flag only when update happens
              updateWbkDisplay(currentWbkValue, currentWbkWarning);
          }
          break;

      case Mode::INFO: // Use enum constant
          // Update info display periodically (Demo mode doesn't affect this currently)
          static unsigned long lastInfoUpdate = 0;
          if (millis() - lastInfoUpdate > 500) { // Update every 500ms
              updateInfoDisplay();
              lastInfoUpdate = millis();
          }
          break;

      case Mode::CAL:
          static unsigned long lastCalUpdate = 0;
          static uint16_t calRecordedValues[8] = {0};
          
          if (calConfirmPressed) {
              calConfirmPressed = false;
              
              if (calState == CalState::INIT) {
                  calState = CalState::WAIT_S1;
              } else if (calState >= CalState::WAIT_S1 && calState <= CalState::WAIT_L) {
                  int idx = (int)calState - 1;
                  calRecordedValues[idx] = SensorValue;
                  
                  calState = (CalState)((int)calState + 1);
                  
                  if (calState == CalState::DONE) {
                      calculateAndSaveThresholds(calRecordedValues);
                      updateCalDisplay(calState, SensorValue); // Show 'Done' briefly
                      delay(2000); 
                      
                      mode = (uint16_t)Mode::STANDARD;
                      ModeChanged = true;
                  }
              }
          }
          
          if (millis() - lastCalUpdate > 100 && calState != CalState::DONE) { // 10Hz UI update
              updateCalDisplay(calState, SensorValue);
              lastCalUpdate = millis();
          }
          break;
  }
}