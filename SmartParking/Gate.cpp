#include "Gate.h"
#include "Config.h"
#include <ESP32Servo.h>

bool          gateIsOpen = false;
unsigned long gateOpenAt = 0;

static Servo gateServo;

void setupGate() {
  gateServo.setPeriodHertz(50);
  gateServo.attach(SERVO_PIN, 500, 2400);
  closeGate();
}

void openGate() {
  gateServo.write(SERVO_OPEN_ANGLE);
  gateIsOpen = true;
  gateOpenAt = millis();
}

void closeGate() {
  gateServo.write(SERVO_CLOSE_ANGLE);
  gateIsOpen = false;
}
