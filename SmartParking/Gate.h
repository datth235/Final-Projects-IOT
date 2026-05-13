#pragma once
#include <Arduino.h>

extern bool          gateIsOpen;
extern unsigned long gateOpenAt;

void setupGate();
void openGate();
void closeGate();
