#ifndef GPS_H
#define GPS_H

#include "globals.h"

#define RX_Butterfly 38 // Define pins here
#define RX_Larus 39

// Declare functions
void initGPS();
void processGPS();

#endif // GPS_H