#pragma once
#include "Config.h"
#include <Arduino.h>

extern CardInfo cards[MAX_CARDS];
extern int      cardCount;

// NVS
void loadCardsFromNVS();
void saveCardsToNVS();

// Tim kiem
int  findCardIndexByUID(const String& uid);
int  findCardIndexByPlate(const String& plate);
int  countCarsInside();
bool trackingMismatch();

// Validation
bool isValidOwnerForCard(const String& ownerRaw, String& err);

// CRUD
bool saveOrUpdateCard(const String& uid, const String& plate, const String& ownerRaw, String& err);
bool updateCardPlate(const String& uid, const String& plate, const String& ownerRaw, String& err);
bool replaceCardUID(const String& oldUID, const String& newUID, String& err);
bool setInsideState(const String& uid, bool inside, String& err);
bool topUpBalance(const String& uid, int amount, String& err);
bool deleteCardByUID(const String& uid, String& err);

// Duoc goi boi UserDB khi xoa tai khoan
void resetCardOwner(const String& username);
