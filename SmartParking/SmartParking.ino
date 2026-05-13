/*
  ============================================================
  SMART PARKING - ESP32 38 PIN  (V11)
  ============================================================
  Cau truc project:
    Config.h          - hang so, chan, struct CardInfo / UserAccount
    Utils.h/.cpp      - ham tien ich chuoi
    Sensors.h/.cpp    - cam bien IR + sieu am
    Gate.h/.cpp       - servo dieu khien cong
    LCDManager.h/.cpp - LCD I2C (scan + hien thi)
    CardDB.h/.cpp     - quan ly the / bien so / so du / NVS
    UserDB.h/.cpp     - quan ly tai khoan web / NVS
    RFIDHandler.h/.cpp- quet the RFID, phan luong vao/ra + tinh phi
    WebUI.h/.cpp      - WiFi AP, trang web, phan quyen admin/user

  Tinh nang chinh (v11):
    - Luot vao: BAT BUOC co xe <= 5.0 cm (sieu am)
    - Luot ra : khong can sieu am, tru EXIT_FEE (20.000 VND)
    - Khong du so du -> tu choi mo cong ra
    - Web phan quyen: admin quan ly toan bo, user chi xem
    - Admin nap tien, xoa the, tao/xoa tai khoan user
    - User xem ban do bai xe + xe/the cua chinh minh
  ============================================================
*/

#include <Wire.h>
#include "Config.h"
#include "Utils.h"
#include "Sensors.h"
#include "Gate.h"
#include "LCDManager.h"
#include "CardDB.h"
#include "UserDB.h"
#include "RFIDHandler.h"
#include "WebUI.h"

// ============================================================
// TIMER LOOP
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

  // NVS: nap du lieu cu
  loadCardsFromNVS();
  loadUsersFromNVS();
  readAllSlots();
  lastDistance = readDistanceCM();

  // WiFi AP + Web server
  setupWebServer();

  // Thong bao khoi dong
  setCardNotice("AP san sang", apIP, 2500);
  updateCardLCD();
  updateSlotLCD();

  Serial.println("\n=== SMART PARKING READY ===");
  Serial.print("AP SSID: ");    Serial.println(AP_SSID);
  Serial.print("AP PASS: ");    Serial.println(AP_PASS);
  Serial.print("IP: ");         Serial.println(apIP);
  Serial.print("Da nap ");      Serial.print(cardCount);   Serial.println(" the.");
  Serial.print("Da nap ");      Serial.print(userCount);   Serial.println(" tai khoan user.");
  Serial.print("Phi moi lan ra: "); Serial.println(EXIT_FEE);
  Serial.print("LCD o trong: ");
  Serial.println(lcdSlotAddr ? hexAddr(lcdSlotAddr) : "khong co");
  Serial.print("LCD bien so: ");
  Serial.println(lcdCardAddr ? hexAddr(lcdCardAddr) : "khong co");
  Serial.print("Nguong quet the luc vao / the moi: <= ");
  Serial.print(RFID_ALLOW_CM, 1); Serial.println(" cm");
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

  // Quet RFID 80 ms / lan
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
    setCardNotice(lastPlate != "" ? lastPlate : "San sang", "Cong da dong", 1500);
  }
}
