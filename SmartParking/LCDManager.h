#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

extern LiquidCrystal_I2C* lcdCard;
extern LiquidCrystal_I2C* lcdSlot;
extern uint8_t lcdCardAddr;
extern uint8_t lcdSlotAddr;

void setupLCDs();
void lcdPrint2Lines(LiquidCrystal_I2C* lcd, const String& line1, const String& line2);
void setCardNotice(const String& line1, const String& line2, unsigned long durationMs);
void updateSlotLCD();
void updateCardLCD();
