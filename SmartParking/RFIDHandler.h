#pragma once
#include <Arduino.h>

extern String lastUID;
extern String lastPlate;
extern String pendingUID;
extern int    lastBalance;

void setupRFID();
void processRFID();
