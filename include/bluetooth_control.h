#pragma once
#include <Arduino.h>

void bt_init();
void bt_process();

// Gửi dữ liệu từng loại sensor
void bt_send_distance(float distance_cm);
void bt_send_light_level(int ldr_value);
void bt_send_alert_status(bool is_alert);
void bt_send_servo_status(int angle, bool is_paused);

// Gửi tất cả dữ liệu cùng lúc (định dạng: DIST,LIGHT,ALERT,SERVO_ANGLE,SERVO_PAUSED)
void bt_send_all_data(float distance, int light, bool alert, int servo_angle, bool servo_paused);
