#pragma once
#include <Arduino.h>

// Bien trang thai RFID (doc boi cac module khac)
extern String lastUID;
extern String lastPlate;
extern String pendingUID;

// Khoi tao SPI + RC522
void setupRFID();

// Goi trong loop() de quet the
void processRFID();
