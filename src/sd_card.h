#ifndef SD_CARD_H
#define SD_CARD_H

#include "globals.h" // Includes SD library

// Declare functions
bool initSDCard();
bool openSDFile(); // Made internal to initSDCard

#endif // SD_CARD_H