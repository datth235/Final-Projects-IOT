#pragma once
#include <Arduino.h>

extern bool          gateIsOpen;
extern unsigned long gateOpenAt;

// Gan servo, dong cong ngay tu dau
void setupGate();

// Mo cong (servo quay, bat timer tu dong dong)
void openGate();

// Dong cong
void closeGate();
