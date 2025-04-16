#include "button_handler.h"

// Extern global variables/objects needed/updated
extern ace_button::AceButton button;
extern volatile bool ModeChanged;
extern uint16_t mode;
extern bool wbkChanged; // To force redraw on mode change
extern bool LarusWindChanged; // To force redraw on mode change

void initButton() {
    pinMode(0, INPUT_PULLUP); // Initialize button pin
    button.setEventHandler(handleEvent);
    Serial.println("Button initialized.");
}

void checkButton() {
    button.check();
}

void handleEvent(ace_button::AceButton* /* button */, uint8_t eventType, uint8_t /* buttonState */) {
    switch (eventType) {
        case ace_button::AceButton::kEventPressed:
            ModeChanged = true; // Signal that mode has changed
            mode = (mode + 1) % 2; // Cycle through modes 0 and 1 for now
            // Reset flags to force redraws in the new mode
            wbkChanged = true;
            LarusWindChanged = true;
            Serial.printf("Button pressed, new mode: %d\n", mode);
            break;
        case ace_button::AceButton::kEventReleased:
            // Optional: Add action on release if needed
            break;
        // Add other events like kEventLongPressed if required
    }
}