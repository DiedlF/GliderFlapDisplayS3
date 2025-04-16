#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "globals.h" // Includes AceButton

// Declare functions
void initButton();
void checkButton(); // Wrapper for button.check()
void handleEvent(ace_button::AceButton* button, uint8_t eventType, uint8_t buttonState); // Event handler

#endif // BUTTON_HANDLER_H