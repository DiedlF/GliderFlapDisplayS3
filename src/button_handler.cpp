#include "button_handler.h"

// Extern global variables/objects needed/updated
extern ace_button::AceButton button;
extern volatile bool ModeChanged;
extern uint16_t mode;
extern bool wbkChanged; // To force redraw on mode change
extern bool LarusWindChanged; // To force redraw on mode change

void initButton() {
    pinMode(0, INPUT_PULLUP); // Initialize button pin
    ace_button::ButtonConfig* config = button.getButtonConfig();
    config->setFeature(ace_button::ButtonConfig::kFeatureLongPress);
    config->setFeature(ace_button::ButtonConfig::kFeatureSuppressAfterLongPress);
    button.setEventHandler(handleEvent);
    Serial.println("Button initialized.");
}

void checkButton() {
    button.check();
}

void handleEvent(ace_button::AceButton* /* button */, uint8_t eventType, uint8_t /* buttonState */) {
    switch (eventType) {
        case ace_button::AceButton::kEventReleased:
            ModeChanged = true; // Signal that mode has changed
            mode = (mode + 1) % 3; // Cycle through 0, 1, 2
            // Reset flags to force redraws in the new mode
            wbkChanged = true;
            LarusWindChanged = true;
            
            if (mode == (uint16_t)Mode::CAL) {
                calState = CalState::INIT; // Reset calibration state on entry
                calConfirmPressed = false;
            }
            Serial.printf("Button released, new mode: %d\n", mode);
            break;
        case ace_button::AceButton::kEventLongPressed:
            if (mode == (uint16_t)Mode::CAL) {
                calConfirmPressed = true;
                Serial.println("Long press confirmed");
            }
            break;
    }
}