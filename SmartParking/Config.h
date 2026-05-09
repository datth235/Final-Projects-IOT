#pragma once
#include <Arduino.h>
// ============================================================
// CONFIG.H - HANG SO & CAU HINH TOAN CUC
// ============================================================

// --- WiFi AP ---
#define AP_SSID "BaiDoXe_ESP32"
#define AP_PASS "12345678"

// --- LCD ---
#define LCD_COLS 16
#define LCD_ROWS 2
#define SLOT_LCD_PREFERRED_ADDR 0x26
#define CARD_LCD_PREFERRED_ADDR 0x27
#define MAX_I2C_FOUND 8

// --- RFID RC522 ---
#define RFID_SS  5
#define RFID_RST 4
#define RFID_SAME_CARD_COOLDOWN_MS 2000UL

// --- Servo ---
#define SERVO_PIN         13
#define SERVO_CLOSE_ANGLE 90
#define SERVO_OPEN_ANGLE  0
#define GATE_OPEN_MS      3000UL

// --- Sieu am HC-SR04 ---
#define TRIG_PIN      15
#define ECHO_PIN      2
#define RFID_ALLOW_CM 5.0f   // Chi cho quet the khi xe <= 5.0 cm

// --- Cam bien IR ---
// Neu trang thai bi nguoc, doi thanh false
#define IR_ACTIVE_LOW true

// --- NVS / RFID DB ---
#define MAX_CARDS 40

// ============================================================
// STRUCT CARD INFO
// ============================================================
struct CardInfo {
  String uid;
  String plate;
  bool   inside;
};
