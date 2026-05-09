/*
  ============================================================
  SMART PARKING - ESP32 38 PIN
  ============================================================
    Config.h        - hang so, cau hinh pin, struct CardInfo
    Utils.h/.cpp    - ham tien ich chuoi
    Sensors.h/.cpp  - cam bien IR + sieu am
    Gate.h/.cpp     - servo dieu khien cong
    LCDManager.h/.cpp - LCD I2C (scan + hien thi)
    CardDB.h/.cpp   - quan ly the / bien so / NVS
    RFIDHandler.h/.cpp - quet the RFID RC522
    WebUI.h/.cpp    - WiFi AP + trang web quan ly
  ============================================================
*/

#include <Wire.h>
#include "Config.h"
#include "Utils.h"
#include "Sensors.h"
#include "Gate.h"
#include "LCDManager.h"
#include "CardDB.h"
#include "RFIDHandler.h"
#include "WebUI.h"

// ============================================================
// TIMER DOC CAM BIEN / LCD / RFID
// ============================================================
static unsigned long lastSensorRead = 0;
static unsigned long lastRFIDCheck  = 0;
static unsigned long lastLcdUpdate  = 0;

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(300);

  // Cam bien IR + sieu am
  setupSensors();

  // I2C + LCD
  Wire.begin(21, 22);
  setupLCDs();

  // Servo / cong
  setupGate();

  // RFID RC522
  setupRFID();

  // NVS: nap du lieu da luu
  loadCardsFromNVS();
  readAllSlots();
  lastDistance = readDistanceCM();

  // WiFi AP + Web server
  setupWebServer();

  // Thong bao khoi dong
  setCardNotice("AP san sang", apIP, 2500);
  updateCardLCD();
  updateSlotLCD();

  Serial.println();
  Serial.println("=== SMART PARKING READY ===");
  Serial.print("AP SSID: "); Serial.println(AP_SSID);
  Serial.print("AP PASS: "); Serial.println(AP_PASS);
  Serial.print("IP: ");      Serial.println(apIP);
  Serial.print("Da nap ");   Serial.print(cardCount);
  Serial.println(" the.");
  Serial.print("LCD o trong: ");
  Serial.println(lcdSlotAddr ? hexAddr(lcdSlotAddr) : "khong co");
  Serial.print("LCD bien so: ");
  Serial.println(lcdCardAddr ? hexAddr(lcdCardAddr) : "khong co");
  Serial.print("Nguong quet the theo sieu am: <= ");
  Serial.print(RFID_ALLOW_CM, 1);
  Serial.println(" cm");
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  // Xu ly yeu cau web
  handleWebClients();

  // Doc cam bien 250 ms / lan
  if (millis() - lastSensorRead > 250) {
    lastSensorRead = millis();
    readAllSlots();
    lastDistance = readDistanceCM();
  }

  // Quet the RFID 80 ms / lan (muot hon)
  if (millis() - lastRFIDCheck > 80) {
    lastRFIDCheck = millis();
    processRFID();
  }

  // Cap nhat LCD 300 ms / lan
  if (millis() - lastLcdUpdate > 300) {
    lastLcdUpdate = millis();
    updateSlotLCD();
    updateCardLCD();
  }

  // Tu dong dong cong sau GATE_OPEN_MS
  if (gateIsOpen && millis() - gateOpenAt >= GATE_OPEN_MS) {
    closeGate();
    setCardNotice(lastPlate != "" ? lastPlate : "San sang",
                  "Cong da dong", 1500);
  }
}
