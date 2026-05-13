#include "RFIDHandler.h"
#include "Config.h"
#include "Utils.h"
#include "CardDB.h"
#include "Sensors.h"
#include "Gate.h"
#include "LCDManager.h"
#include <SPI.h>
#include <MFRC522.h>

String lastUID    = "";
String lastPlate  = "";
String pendingUID = "";
int    lastBalance = 0;

static MFRC522       rfid(RFID_SS, RFID_RST);
static String        lastProcessedUID   = "";
static unsigned long lastProcessedUIDAt = 0;

void setupRFID() {
  SPI.begin();
  rfid.PCD_Init();
}

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
// XU LY RFID
// ============================================================
// Quy tac:
//   LUOT RA  : the inside=true  -> KHONG can sieu am, tru EXIT_FEE
//   LUOT VAO : the inside=false -> CAN sieu am <= RFID_ALLOW_CM
//   THE MOI  : chua dang ky     -> CAN sieu am de tao pendingUID
// ============================================================
void processRFID() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial())   return;

  String uid = uidToString(&rfid.uid);

  if (isSameCardCooldown(uid)) {
    Serial.print("Bo qua the lap lai: "); Serial.println(uid);
    finishRFIDRead();
    return;
  }

  lastUID = uid;
  int idx = findCardIndexByUID(uid);

  if (idx >= 0) {
    lastPlate  = cards[idx].plate;
    lastBalance = cards[idx].balance;
    pendingUID = "";

    // --- LUOT RA ---
    if (cards[idx].inside) {
      if (cards[idx].balance < EXIT_FEE) {
        setCardNotice("So du khong du", "Nap tien tren web", 2000);
        Serial.print("Tu choi xe ra do khong du tien: ");
        Serial.print(cards[idx].plate);
        Serial.print(" | So du: ");
        Serial.println(cards[idx].balance);
      } else {
        cards[idx].balance -= EXIT_FEE;
        cards[idx].inside   = false;
        lastBalance         = cards[idx].balance;
        saveCardsToNVS();
        openGate();
        setCardNotice(cards[idx].plate, "Xe ra - tru 20k", 2000);
        Serial.print("Xe ra khoi bai: ");
        Serial.print(cards[idx].plate);
        Serial.print(" | Tru phi: "); Serial.print(EXIT_FEE);
        Serial.print(" | Con lai: "); Serial.println(cards[idx].balance);
      }
      rememberProcessedCard(uid);
      finishRFIDRead();
      return;
    }

    // --- LUOT VAO ---
    if (!vehicleInScanZone()) {
      setCardNotice("Xe vao can", "<= 5.0cm moi quet", 1800);
      Serial.println("Tu choi vao: chua co xe o vung sieu am.");
      rememberProcessedCard(uid);
      finishRFIDRead();
      return;
    }

    if (freeCount <= 0) {
      setCardNotice(cards[idx].plate, "Het cho trong", 1800);
      Serial.print("Tu choi xe vao vi het cho: "); Serial.println(cards[idx].plate);
      rememberProcessedCard(uid);
      finishRFIDRead();
      return;
    }

    cards[idx].inside = true;
    saveCardsToNVS();
    openGate();
    setCardNotice(cards[idx].plate, "Xe vao - cong mo", 1800);
    Serial.print("Xe vao bai: "); Serial.println(cards[idx].plate);
    lastBalance = cards[idx].balance;

    rememberProcessedCard(uid);
    finishRFIDRead();
    return;
  }

  // --- THE MOI ---
  if (!vehicleInScanZone()) {
    lastPlate  = "CHUA DANG KY";
    lastBalance = 0;
    setCardNotice("The moi can", "<= 5.0cm moi quet", 1800);
    Serial.println("Tu choi the moi: chua co xe o vung sieu am.");
    rememberProcessedCard(uid);
    finishRFIDRead();
    return;
  }

  lastPlate   = "CHUA DANG KY";
  lastBalance = 0;
  pendingUID  = uid;
  setCardNotice("The moi", "Nhap bien so web", 1800);
  Serial.print("The moi, UID cho dang ky: "); Serial.println(uid);

  rememberProcessedCard(uid);
  finishRFIDRead();
}
