#include "LCDManager.h"
#include "Config.h"
#include "Utils.h"
#include "Sensors.h"
#include "Gate.h"
#include <Wire.h>

LiquidCrystal_I2C* lcdCard = nullptr;
LiquidCrystal_I2C* lcdSlot = nullptr;
uint8_t lcdCardAddr = 0;
uint8_t lcdSlotAddr = 0;

static uint8_t i2cFound[MAX_I2C_FOUND];
static int     i2cFoundCount = 0;

static String        cardNoticeLine1 = "";
static String        cardNoticeLine2 = "";
static unsigned long cardNoticeUntil = 0;
static unsigned long lastPageFlip    = 0;
static bool          lcdPage         = false;

// Bien ngoai tu SmartParking.ino
extern String pendingUID;
extern String lastPlate;

// ============================================================
// I2C
// ============================================================
static bool isI2CAlive(uint8_t addr) {
  Wire.beginTransmission(addr);
  return (Wire.endTransmission() == 0);
}

static bool isLikelyLcdBackpack(uint8_t addr) {
  return ((addr >= 0x20 && addr <= 0x27) ||
          (addr >= 0x38 && addr <= 0x3F));
}

static void scanI2C() {
  i2cFoundCount = 0;
  Serial.println("\n=== I2C SCAN START ===");

  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      if (i2cFoundCount < MAX_I2C_FOUND)
        i2cFound[i2cFoundCount++] = addr;
      Serial.print("Tim thay I2C tai ");
      Serial.println(hexAddr(addr));
    }
  }

  if (i2cFoundCount == 0) Serial.println("Khong tim thay thiet bi I2C nao.");
  Serial.println("=== I2C SCAN END ===");
}

static uint8_t chooseLCDAddress(uint8_t preferredAddr, uint8_t excludeAddr) {
  if (preferredAddr != 0 && preferredAddr != excludeAddr && isI2CAlive(preferredAddr))
    return preferredAddr;

  for (int i = 0; i < i2cFoundCount; i++) {
    uint8_t addr = i2cFound[i];
    if (addr == excludeAddr) continue;
    if (isLikelyLcdBackpack(addr)) return addr;
  }

  for (int i = 0; i < i2cFoundCount; i++)
    if (i2cFound[i] != excludeAddr) return i2cFound[i];

  return 0;
}

static void initOneLCD(LiquidCrystal_I2C*& lcd, uint8_t addr, const char* label) {
  if (addr == 0) return;
  lcd = new LiquidCrystal_I2C(addr, LCD_COLS, LCD_ROWS);
  lcd->init();
  lcd->backlight();
  lcd->clear();
  lcd->setCursor(0, 0);
  lcd->print(fit16(String(label) + " " + hexAddr(addr)));
  lcd->setCursor(0, 1);
  lcd->print(fit16("Dang khoi dong"));
  Serial.print(label);
  Serial.print(" LCD da gan tai ");
  Serial.println(hexAddr(addr));
}

// ============================================================
// PUBLIC
// ============================================================
void setupLCDs() {
  scanI2C();
  lcdSlotAddr = chooseLCDAddress(SLOT_LCD_PREFERRED_ADDR, 0);
  lcdCardAddr = chooseLCDAddress(CARD_LCD_PREFERRED_ADDR, lcdSlotAddr);

  if (lcdSlotAddr == 0 && lcdCardAddr == 0) {
    Serial.println("Khong gan duoc LCD nao."); return;
  }

  if (lcdSlotAddr != 0) initOneLCD(lcdSlot, lcdSlotAddr, "LCD OTRONG");
  if (lcdCardAddr != 0) initOneLCD(lcdCard, lcdCardAddr, "LCD BIENSO");
  if (lcdSlotAddr == 0) Serial.println("Khong tim thay LCD hien o trong.");
  if (lcdCardAddr == 0) Serial.println("Khong tim thay LCD hien bien so.");
}

void lcdPrint2Lines(LiquidCrystal_I2C* lcd, const String& line1, const String& line2) {
  if (lcd == nullptr) return;
  lcd->setCursor(0, 0); lcd->print(fit16(line1));
  lcd->setCursor(0, 1); lcd->print(fit16(line2));
}

void setCardNotice(const String& line1, const String& line2, unsigned long durationMs) {
  cardNoticeLine1 = line1;
  cardNoticeLine2 = line2;
  cardNoticeUntil = millis() + durationMs;
}

static String buildMapLine(int startIdx, int endIdx) {
  String text = "";
  for (int i = startIdx; i <= endIdx; i++) {
    text += String(i + 1);
    text += (slotOccupied[i] ? "D" : "T");
    if (i < endIdx) text += " ";
  }
  return text;
}

void updateSlotLCD() {
  if (lcdSlot == nullptr) return;
  if (millis() - lastPageFlip > 2000) {
    lastPageFlip = millis();
    lcdPage = !lcdPage;
  }
  String line1 = "Trong:" + String(freeCount) + "/10";
  String line2 = !lcdPage ? buildMapLine(0, 4) : buildMapLine(5, 9);
  lcdPrint2Lines(lcdSlot, line1, line2);
}

void updateCardLCD() {
  if (lcdCard == nullptr) return;
  if (cardNoticeUntil != 0 && (long)(cardNoticeUntil - millis()) > 0) {
    lcdPrint2Lines(lcdCard, cardNoticeLine1, cardNoticeLine2);
    return;
  }

  String line1;
  if (pendingUID != "") {
    line1 = "UID cho dang ky";
  } else if (lastPlate != "") {
    line1 = lastPlate;
  } else {
    line1 = "Cho quet the";
  }

  String line2 = gateIsOpen ? "Cong: MO" : "Cong: DONG";
  lcdPrint2Lines(lcdCard, line1, line2);
}
