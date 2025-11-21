#include <Arduino.h>
#include "indicators.h"

const int LED_BLINK_PIN = 12;
static unsigned long last = 0;
static bool ledState = false;

void indicators_init() {
  pinMode(LED_BLINK_PIN, OUTPUT);
  digitalWrite(LED_BLINK_PIN, LOW);
}

void indicators_update() {
  unsigned long now = millis();
  if (now - last < 500) return;
  last = now;
  ledState = !ledState;
  digitalWrite(LED_BLINK_PIN, ledState ? HIGH : LOW);
}