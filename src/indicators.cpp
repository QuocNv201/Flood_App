#include <Arduino.h>
#include "indicators.h"

const int LED_BLINK_PIN = 12;

// Trạng thái cảnh báo 
static bool alertActive = false;

void indicators_init() {  
  pinMode(LED_BLINK_PIN, OUTPUT);
  digitalWrite(LED_BLINK_PIN, LOW);
  alertActive = false;
}

void indicators_update() {
  // Nếu không ở chế độ cảnh báo thì không làm gì
  if (!alertActive) return;

  digitalWrite(LED_BLINK_PIN, HIGH); 
  delay(500);                        
  digitalWrite(LED_BLINK_PIN, LOW);  
  delay(500);                         
}

void indicators_alert_on() { 
  alertActive = true;
  // Bật ngay 1 lần để phản hồi tức thì
  digitalWrite(LED_BLINK_PIN, HIGH);
}

void indicators_alert_off() {
  alertActive = false;
  // Tắt ngay
  digitalWrite(LED_BLINK_PIN, LOW);
}