#pragma once
#include <Arduino.h>

extern bool          slotOccupied[10];
extern int           freeCount;
extern float         lastDistance;
extern const uint8_t IR_PINS[10];

void  setupSensors();
void  readAllSlots();
float readDistanceCM();
bool  vehicleInScanZone();
