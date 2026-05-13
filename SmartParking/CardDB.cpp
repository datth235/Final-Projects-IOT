#include "CardDB.h"
#include "Utils.h"
#include "Sensors.h"    // freeCount
#include "UserDB.h"     // findUserIndexByUsername
#include <Preferences.h>

CardInfo cards[MAX_CARDS];
int      cardCount = 0;

static Preferences prefs;

// ============================================================
// NVS
// ============================================================
void loadCardsFromNVS() {
  prefs.begin("rfiddb", true);

  cardCount = prefs.getInt("count", 0);
  if (cardCount < 0)         cardCount = 0;
  if (cardCount > MAX_CARDS) cardCount = MAX_CARDS;

  for (int i = 0; i < cardCount; i++) {
    String ku = "u" + String(i);
    String kp = "p" + String(i);
    String ks = "s" + String(i);
    String kb = "b" + String(i);
    String ko = "o" + String(i);

    cards[i].uid     = prefs.getString(ku.c_str(), "");
    cards[i].plate   = prefs.getString(kp.c_str(), "");
    cards[i].owner   = normalizeUsername(prefs.getString(ko.c_str(), ""));
    cards[i].inside  = (prefs.getUChar(ks.c_str(), 0) == 1);
    cards[i].balance = prefs.getInt(kb.c_str(), 0);
  }

  prefs.end();
}

void saveCardsToNVS() {
  prefs.begin("rfiddb", false);
  prefs.putInt("count", cardCount);

  for (int i = 0; i < MAX_CARDS; i++) {
    String ku = "u" + String(i);
    String kp = "p" + String(i);
    String ks = "s" + String(i);
    String kb = "b" + String(i);
    String ko = "o" + String(i);

    if (i < cardCount) {
      prefs.putString(ku.c_str(), cards[i].uid);
      prefs.putString(kp.c_str(), cards[i].plate);
      prefs.putString(ko.c_str(), cards[i].owner);
      prefs.putUChar(ks.c_str(),  cards[i].inside ? 1 : 0);
      prefs.putInt(kb.c_str(),    cards[i].balance);
    } else {
      prefs.remove(ku.c_str());
      prefs.remove(kp.c_str());
      prefs.remove(ko.c_str());
      prefs.remove(ks.c_str());
      prefs.remove(kb.c_str());
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
  String pn = normalizePlate(plate);
  for (int i = 0; i < cardCount; i++)
    if (normalizePlate(cards[i].plate) == pn) return i;
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
// VALIDATION
// ============================================================
bool isValidOwnerForCard(const String& ownerRaw, String& err) {
  String owner = normalizeUsername(ownerRaw);
  if (owner == "") return true;
  if (findUserIndexByUsername(owner) < 0) {
    err = "Tai khoan chu xe khong ton tai. Hay tao user truoc.";
    return false;
  }
  return true;
}

// ============================================================
// CRUD
// ============================================================
bool saveOrUpdateCard(const String& uid, const String& plate,
                      const String& ownerRaw, String& err) {
  String owner    = normalizeUsername(ownerRaw);
  int idx         = findCardIndexByUID(uid);
  int plateIdx    = findCardIndexByPlate(plate);

  if (!isValidOwnerForCard(owner, err)) return false;
  if (plateIdx >= 0 && plateIdx != idx) {
    err = "Bien so da duoc gan cho the khac.";
    return false;
  }

  if (idx >= 0) {
    cards[idx].plate = plate;
    cards[idx].owner = owner;
    saveCardsToNVS();
    return true;
  }

  if (cardCount >= MAX_CARDS) {
    err = "Bo nho the da day.";
    return false;
  }

  cards[cardCount].uid     = uid;
  cards[cardCount].plate   = plate;
  cards[cardCount].owner   = owner;
  cards[cardCount].inside  = false;
  cards[cardCount].balance = 0;
  cardCount++;
  saveCardsToNVS();
  return true;
}

bool updateCardPlate(const String& uid, const String& plate,
                     const String& ownerRaw, String& err) {
  String owner = normalizeUsername(ownerRaw);
  int idx = findCardIndexByUID(uid);
  if (idx < 0) { err = "Khong tim thay the can sua."; return false; }

  if (!isValidOwnerForCard(owner, err)) return false;

  int plateIdx = findCardIndexByPlate(plate);
  if (plateIdx >= 0 && plateIdx != idx) {
    err = "Bien so da duoc gan cho the khac.";
    return false;
  }

  cards[idx].plate = plate;
  cards[idx].owner = owner;
  saveCardsToNVS();
  return true;
}

bool replaceCardUID(const String& oldUID, const String& newUID, String& err) {
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

bool topUpBalance(const String& uid, int amount, String& err) {
  if (amount <= 0) { err = "So tien nap phai > 0."; return false; }

  int idx = findCardIndexByUID(uid);
  if (idx < 0) { err = "Khong tim thay the."; return false; }

  if (cards[idx].balance > 2000000000 - amount) {
    err = "So du qua lon.";
    return false;
  }

  cards[idx].balance += amount;
  saveCardsToNVS();
  return true;
}

bool deleteCardByUID(const String& uid, String& err) {
  int idx = findCardIndexByUID(uid);
  if (idx < 0) { err = "Khong tim thay the can xoa."; return false; }

  for (int i = idx; i < cardCount - 1; i++)
    cards[i] = cards[i + 1];
  if (cardCount > 0) cardCount--;

  saveCardsToNVS();
  return true;
}

// Duoc goi boi UserDB::deleteUserAccountByName
void resetCardOwner(const String& username) {
  for (int i = 0; i < cardCount; i++)
    if (cards[i].owner == username) cards[i].owner = "";
  saveCardsToNVS();
}
