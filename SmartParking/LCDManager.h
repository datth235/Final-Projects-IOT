#pragma once
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// LCD1 - hien bien so + trang thai cong
extern LiquidCrystal_I2C* lcdCard;
// LCD2 - hien so o trong + ban do 10 o
extern LiquidCrystal_I2C* lcdSlot;

extern uint8_t lcdCardAddr;
extern uint8_t lcdSlotAddr;

// Khoi tao: quet I2C, gan LCD tu dong
void setupLCDs();

// In 2 dong len mot LCD
void lcdPrint2Lines(LiquidCrystal_I2C* lcd,
                    const String& line1,
                    const String& line2);

// Hien thi thong bao tam thoi tren LCD bien so
void setCardNotice(const String& line1,
                   const String& line2,
                   unsigned long durationMs);

// Cap nhat LCD so o trong (goi trong loop)
void updateSlotLCD();

// Cap nhat LCD bien so / cong (goi trong loop)
void updateCardLCD();
