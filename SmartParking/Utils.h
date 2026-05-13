#pragma once
#include <Arduino.h>
#include <MFRC522.h>

String fit16(String text);
String normalizePlate(String text);
String normalizeUID(String text);
String normalizeUsername(String text);
String uidToString(MFRC522::Uid* uid);
String hexAddr(uint8_t addr);
String moneyToText(int money);
String jsonEscape(String text);
