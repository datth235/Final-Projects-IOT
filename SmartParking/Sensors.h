#pragma once
#include <Arduino.h>

// Trang thai 10 o do xe (true = co xe)
extern bool slotOccupied[10];

// So o con trong
extern int freeCount;

// Khoang cach do duoc tu sieu am (-1 neu loi)
extern float lastDistance;

// Chan IR cho 10 o
extern const uint8_t IR_PINS[10];

// Khoi tao chan IR va sieu am
void setupSensors();

// Doc trang thai 10 o IR
void readAllSlots();

// Do khoang cach sieu am (cm), tra ve -1 neu qua tam hoac loi
float readDistanceCM();

// Kiem tra xe co trong vung quet the (<= RFID_ALLOW_CM) hay khong
bool vehicleInScanZone();
