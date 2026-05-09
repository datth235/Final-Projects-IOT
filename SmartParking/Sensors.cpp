#include "Sensors.h"
#include "Config.h"

// ============================================================
// BIEN TOAN CUC
// ============================================================
bool          slotOccupied[10];
int           freeCount    = 0;
float         lastDistance = -1.0;

const uint8_t IR_PINS[10] = {32, 33, 25, 26, 27, 14, 16, 17, 34, 35};

// ============================================================
// SETUP
// ============================================================
void setupSensors() {
  for (int i = 0; i < 10; i++)
    pinMode(IR_PINS[i], INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

// ============================================================
// IR
// ============================================================
static bool isOccupied(int idx) {
  int value = digitalRead(IR_PINS[idx]);
  return IR_ACTIVE_LOW ? (value == LOW) : (value == HIGH);
}

void readAllSlots() {
  freeCount = 0;
  for (int i = 0; i < 10; i++) {
    slotOccupied[i] = isOccupied(i);
    if (!slotOccupied[i]) freeCount++;
  }
}

// ============================================================
// SIEU AM
// ============================================================
float readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1.0;

  float distance = duration * 0.0343f / 2.0f;
  return (distance > 500.0f) ? -1.0f : distance;
}

bool vehicleInScanZone() {
  return (lastDistance > 0.0f && lastDistance <= RFID_ALLOW_CM);
}
