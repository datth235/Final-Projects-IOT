#include "Utils.h"

String fit16(String text) {
  while (text.length() < 16) text += " ";
  if (text.length() > 16) text = text.substring(0, 16);
  return text;
}

String normalizePlate(String text) {
  text.trim();
  text.toUpperCase();
  text.replace(" ", "");
  return text;
}

String normalizeUID(String text) {
  text.trim();
  text.toUpperCase();
  text.replace(" ", "");
  return text;
}

String uidToString(MFRC522::Uid* uid) {
  String text = "";
  for (byte i = 0; i < uid->size; i++) {
    if (uid->uidByte[i] < 0x10) text += "0";
    text += String(uid->uidByte[i], HEX);
  }
  text.toUpperCase();
  return text;
}

String hexAddr(uint8_t addr) {
  char buffer[6];
  snprintf(buffer, sizeof(buffer), "0x%02X", addr);
  return String(buffer);
}

String jsonEscape(String text) {
  text.replace("\\", "\\\\");
  text.replace("\"", "\\\"");
  text.replace("\r", "");
  text.replace("\n", " ");
  return text;
}
