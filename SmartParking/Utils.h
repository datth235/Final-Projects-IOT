#pragma once
#include <Arduino.h>
#include <MFRC522.h>

// Bat buoc chuoi thanh dung 16 ky tu de in LCD dep hon
String fit16(String text);

// Chuan hoa bien so truoc khi luu / so sanh
String normalizePlate(String text);

// Chuan hoa UID truoc khi so sanh
String normalizeUID(String text);

// Chuyen UID doc tu RC522 thanh chuoi HEX de luu
String uidToString(MFRC522::Uid* uid);

// In dia chi I2C dang hex: "0x26"
String hexAddr(uint8_t addr);

// Escape chuoi de chen vao JSON an toan
String jsonEscape(String text);
