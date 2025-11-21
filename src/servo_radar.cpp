#include <Arduino.h>
#include <ESP32Servo.h>
#include "servo_radar.h"

#define SERVO_PIN 5

static Servo myServo;
static int currentAngle = 0;
static int stepAngle = 2;
static bool forwardDir = true;
static unsigned long lastMove = 0;
static const unsigned long interval = 3; // ms
static bool paused = false; //  tạm dừng quét

void servo_init() {
  myServo.attach(SERVO_PIN);
  myServo.write(currentAngle);
  lastMove = millis();// khởi tạo thời gian
  paused = false;
}

void servo_update() {
  if (paused) return;
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
void servo_setAngle(int angle) {
  if (angle < 0) angle = 0;
  if (angle > 180) angle = 180;
  currentAngle = angle;
  myServo.write(currentAngle);
}

void servo_pause() {
  paused = true;
  // giữ nguyên currentAngle, servo sẽ không update cho tới resume
}

void servo_resume() {
  paused = false;
  // reset timer để tránh nhảy ngay lập tức
  lastMove = millis();
}

bool servo_isPaused() {
  return paused;
}