#include "RFIDHandler.h"
#include "Config.h"
#include "Utils.h"
#include "CardDB.h"
#include "Sensors.h"
#include "Gate.h"
#include "LCDManager.h"
#include <SPI.h>
#include <MFRC522.h>

// ============================================================
// BIEN TOAN CUC
// ============================================================
String lastUID   = "";
String lastPlate = "";
String pendingUID = "";

static MFRC522       rfid(RFID_SS, RFID_RST);
static String        lastProcessedUID   = "";
static unsigned long lastProcessedUIDAt = 0;

// ============================================================
// SETUP
// ============================================================
void setupRFID() {
  SPI.begin();
  rfid.PCD_Init();
}

// ============================================================
// HELPER
// ============================================================
static void finishRFIDRead() {
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

static bool isSameCardCooldown(const String& uid) {
  return (uid == lastProcessedUID &&
          millis() - lastProcessedUIDAt < RFID_SAME_CARD_COOLDOWN_MS);
}

static void rememberProcessedCard(const String& uid) {
  lastProcessedUID   = uid;
  lastProcessedUIDAt = millis();
}

// ============================================================
// XU LY CHINH
// Quy tac:
//   inside = false => cho xe vao bai
//   inside = true  => cho xe ra bai
//   chua co => dat pendingUID de web dang ky
// ============================================================
void processRFID() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial())   return;

  String uid = uidToString(&rfid.uid);

  if (isSameCardCooldown(uid)) {
    Serial.print("Bo qua the lap lai: ");
    Serial.println(uid);
    finishRFIDRead();
    return;
  }

  if (!vehicleInScanZone()) {
    setCardNotice("Dua xe vao", "<= 5.0cm moi quet", 1800);
    Serial.println("Tu choi quet the: xe chua vao vung <= 5.0 cm.");
    rememberProcessedCard(uid);
    finishRFIDRead();
    return;
  }

  lastUID = uid;
  int idx = findCardIndexByUID(uid);

  if (idx >= 0) {
    lastPlate  = cards[idx].plate;
    pendingUID = "";

    if (cards[idx].inside) {
      // Xe ra
      cards[idx].inside = false;
      saveCardsToNVS();
      openGate();
      setCardNotice(lastPlate, "Xe ra - cong mo", 1800);
      Serial.print("Xe ra khoi bai: ");
      Serial.println(lastPlate);
    } else {
      // Xe vao
      if (freeCount > 0) {
        cards[idx].inside = true;
        saveCardsToNVS();
        openGate();
        setCardNotice(lastPlate, "Xe vao - cong mo", 1800);
        Serial.print("Xe vao bai: ");
        Serial.println(lastPlate);
      } else {
        setCardNotice(lastPlate, "Het cho trong", 1800);
        Serial.print("Tu choi xe vao vi het cho: ");
        Serial.println(lastPlate);
      }
    }
  } else {
    // The chua dang ky
    lastPlate  = "CHUA DANG KY";
    pendingUID = uid;
    setCardNotice("The moi", "Nhap bien so web", 1800);
    Serial.print("The moi, UID cho dang ky: ");
    Serial.println(uid);
  }

  rememberProcessedCard(uid);
  finishRFIDRead();
}
