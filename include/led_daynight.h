#pragma once
void led_init();
int led_readAnalog();   // đọc ADC LDR
void led_update(int adcValue); // bật/tắt LED theo ngưỡng