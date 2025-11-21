#include <Arduino.h>
#include "ultrasonic.h"

#define TRIG_PIN 19
#define ECHO_PIN 18

void ultrasonic_init() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
}

float ultrasonic_read_cm() {
  // Gửi xung
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);
  if (duration == 0) return -1.0f;
  float distance = duration * 0.0343f / 2.0f;
  return distance;
}