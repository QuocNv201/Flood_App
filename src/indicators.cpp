#include <Arduino.h>
#include "indicators.h"

const int LED_BLINK_PIN = 12;

// Internal state
static unsigned long lastMillis = 0;
static bool ledState = false;
static bool alertActive = false;
static unsigned long intervalMs = 500;

void indicators_init() {
  pinMode(LED_BLINK_PIN, OUTPUT);
  digitalWrite(LED_BLINK_PIN, LOW);
  lastMillis = millis();
  ledState = false;
  alertActive = false;
}

void indicators_update() {
  // Chỉ nhấp nháy khi alertActive = true
  if (!alertActive) return;

  unsigned long now = millis();
  if (now - lastMillis >= intervalMs) {
    lastMillis = now;
    ledState = !ledState;
    digitalWrite(LED_BLINK_PIN, ledState ? HIGH : LOW);
  }
}

void indicators_alert_on() {
  if (!alertActive) {
    alertActive = true;
    lastMillis = millis();
    ledState = true;
    digitalWrite(LED_BLINK_PIN, HIGH); // bật ngay
  }
}

void indicators_alert_off() {
  if (alertActive) {
    alertActive = false;
    ledState = false;
    digitalWrite(LED_BLINK_PIN, LOW); // tắt ngay
  }
}

void indicators_setInterval(unsigned long ms) {
  if (ms == 0) return;
  intervalMs = ms;
}