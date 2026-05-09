#include "CardDB.h"
#include "Utils.h"
#include "Sensors.h"   // freeCount
#include <Preferences.h>
#include <Arduino.h>

CardInfo cards[MAX_CARDS];
int      cardCount = 0;

static Preferences prefs;

// ============================================================
// NVS LOAD / SAVE
// ============================================================
void loadCardsFromNVS() {
  prefs.begin("rfiddb", true);

  cardCount = prefs.getInt("count", 0);
  if (cardCount < 0)        cardCount = 0;
  if (cardCount > MAX_CARDS) cardCount = MAX_CARDS;

  for (int i = 0; i < cardCount; i++) {
    String keyUID    = "u" + String(i);
    String keyPlate  = "p" + String(i);
    String keyInside = "s" + String(i);

    cards[i].uid    = prefs.getString(keyUID.c_str(), "");
    cards[i].plate  = prefs.getString(keyPlate.c_str(), "");
    cards[i].inside = (prefs.getUChar(keyInside.c_str(), 0) == 1);
  }

  prefs.end();
}

void saveCardsToNVS() {
  prefs.begin("rfiddb", false);
  prefs.putInt("count", cardCount);

  for (int i = 0; i < MAX_CARDS; i++) {
    String keyUID    = "u" + String(i);
    String keyPlate  = "p" + String(i);
    String keyInside = "s" + String(i);

    if (i < cardCount) {
      prefs.putString(keyUID.c_str(),  cards[i].uid);
      prefs.putString(keyPlate.c_str(), cards[i].plate);
      prefs.putUChar(keyInside.c_str(), cards[i].inside ? 1 : 0);
    } else {
      prefs.remove(keyUID.c_str());
      prefs.remove(keyPlate.c_str());
      prefs.remove(keyInside.c_str());
    }
  }

  prefs.end();
}

// ============================================================
// TIM KIEM
// ============================================================
int findCardIndexByUID(const String& uid) {
  for (int i = 0; i < cardCount; i++)
    if (cards[i].uid == uid) return i;
  return -1;
}

int findCardIndexByPlate(const String& plate) {
  String plateNorm = normalizePlate(plate);
  for (int i = 0; i < cardCount; i++)
    if (normalizePlate(cards[i].plate) == plateNorm) return i;
  return -1;
}

int countCarsInside() {
  int count = 0;
  for (int i = 0; i < cardCount; i++)
    if (cards[i].inside) count++;
  return count;
}

bool trackingMismatch() {
  return (countCarsInside() != (10 - freeCount));
}

// ============================================================
// CRUD
// ============================================================
bool saveOrUpdateCard(const String& uid,
                      const String& plate,
                      String& err) {
  int idx      = findCardIndexByUID(uid);
  int plateIdx = findCardIndexByPlate(plate);

  if (plateIdx >= 0 && plateIdx != idx) {
    err = "Bien so da duoc gan cho the khac.";
    return false;
  }

  if (idx >= 0) {
    cards[idx].plate = plate;
    saveCardsToNVS();
    return true;
  }

  if (cardCount >= MAX_CARDS) {
    err = "Bo nho the da day.";
    return false;
  }

  cards[cardCount].uid    = uid;
  cards[cardCount].plate  = plate;
  cards[cardCount].inside = false;
  cardCount++;

  saveCardsToNVS();
  return true;
}

bool updateCardPlate(const String& uid,
                     const String& plate,
                     String& err) {
  int idx = findCardIndexByUID(uid);
  if (idx < 0) { err = "Khong tim thay the can sua."; return false; }

  int plateIdx = findCardIndexByPlate(plate);
  if (plateIdx >= 0 && plateIdx != idx) {
    err = "Bien so da duoc gan cho the khac.";
    return false;
  }

  cards[idx].plate = plate;
  saveCardsToNVS();
  return true;
}

bool replaceCardUID(const String& oldUID,
                    const String& newUID,
                    String& err) {
  int oldIdx = findCardIndexByUID(oldUID);
  if (oldIdx < 0) { err = "Khong tim thay the cu."; return false; }

  int newIdx = findCardIndexByUID(newUID);
  if (newIdx >= 0 && newIdx != oldIdx) {
    err = "The moi da ton tai trong he thong.";
    return false;
  }

  cards[oldIdx].uid = newUID;
  saveCardsToNVS();
  return true;
}

bool setInsideState(const String& uid, bool inside, String& err) {
  int idx = findCardIndexByUID(uid);
  if (idx < 0) { err = "Khong tim thay the."; return false; }

  cards[idx].inside = inside;
  saveCardsToNVS();
  return true;
}
