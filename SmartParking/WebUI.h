#pragma once
#include <Arduino.h>

// Khoi tao WiFi AP va dang ky tat ca route
void setupWebServer();

// Goi trong loop()
void handleWebClients();

// IP cua AP (de in len LCD / Serial)
extern String apIP;
