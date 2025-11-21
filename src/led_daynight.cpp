#include <Arduino.h>
#include "led_daynight.h"

const int LDR_PIN = 35;    // ADC1_CH7 trên ESP32
const int LED_PIN = 14;    // đèn chính (thay đổi theo mạch)
const int LDR_THRESHOLD = 750;

void led_init() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  // LDR là input analog, không cần pinMode
}

int led_readAnalog() {
  return analogRead(LDR_PIN);
}

void led_update(int adcValue) {
  if (adcValue >= LDR_THRESHOLD) digitalWrite(LED_PIN, HIGH);
  else digitalWrite(LED_PIN, LOW);
}