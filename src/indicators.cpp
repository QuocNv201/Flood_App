#include <Arduino.h>
#include "indicators.h"

const int BUZZER_PIN = 12;
const int BUZZER_CHANNEL = 15;
const int BUZZER_PWM_RESOLUTION = 8;
const int BUZZER_FREQUENCY_HZ = 2400;
const unsigned long BUZZER_TOGGLE_INTERVAL_MS = 150;

// Trạng thái cảnh báo
static bool alertActive = false;
static bool buzzerState = false;
static unsigned long lastToggleMs = 0;

static void buzzer_on() {
  ledcWriteTone(BUZZER_CHANNEL, BUZZER_FREQUENCY_HZ);
}

static void buzzer_off() {
  ledcWriteTone(BUZZER_CHANNEL, 0);
}

void indicators_init() {
  ledcSetup(BUZZER_CHANNEL, BUZZER_FREQUENCY_HZ, BUZZER_PWM_RESOLUTION);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

  alertActive = false;
  buzzerState = false;
  lastToggleMs = millis();
  buzzer_off();
}

void indicators_update() {
  // Không cảnh báo: đảm bảo còi tắt hoàn toàn.
  if (!alertActive) {
    if (buzzerState) {
      buzzerState = false;
      buzzer_off();
    }
    return;
  }

  // Còi kêu tít tít theo chu kỳ, không dùng delay để tránh chặn loop.
  const unsigned long now = millis();
  if (now - lastToggleMs >= BUZZER_TOGGLE_INTERVAL_MS) {
    lastToggleMs = now;
    buzzerState = !buzzerState;
    if (buzzerState) {
      buzzer_on();
    } else {
      buzzer_off();
    }
  }
}

void indicators_alert_on() {
  alertActive = true;
  // Bật ngay để người dùng nghe phản hồi tức thì.
  buzzerState = true;
  buzzer_on();
  lastToggleMs = millis();
}

void indicators_alert_off() {
  alertActive = false;
  // Tắt còi ngay khi hết cảnh báo.
  buzzerState = false;
  buzzer_off();
}