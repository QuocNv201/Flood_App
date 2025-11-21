#include <Arduino.h>
#include <ESP32Servo.h>
#include "servo_radar.h"

#define SERVO_PIN 5

static Servo myServo;
static int currentAngle = 0;
static int stepAngle = 1;
static bool forwardDir = true;
static unsigned long lastMove = 0;
static const unsigned long interval = 10; // ms

void servo_init() {
  myServo.attach(SERVO_PIN);
  myServo.write(currentAngle);
}

void servo_update() {
  unsigned long now = millis();
  if (now - lastMove < interval) return;
  lastMove = now;

  if (forwardDir) {
    currentAngle += stepAngle;
    if (currentAngle >= 180) { currentAngle = 180; forwardDir = false; }
  } else {
    currentAngle -= stepAngle;
    if (currentAngle <= 0) { currentAngle = 0; forwardDir = true; }
  }
  myServo.write(currentAngle);
}