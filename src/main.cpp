#include <Arduino.h>
#include "led_daynight.h"
#include "servo_radar.h"
#include "ultrasonic.h"
#include "bluetooth_control.h"
#include "indicators.h"

static const float OBSTACLE_NEAR_CM = 25.0f;
static const float OBSTACLE_CLEAR_CM = 30.0f;
static bool obstacleDetected = false;

void setup() {
    Serial.begin(115200);

    // Khởi tạo tất cả module
    led_init();
    servo_init();
    ultrasonic_init();
    bt_init();
    indicators_init();

    Serial.println("System initialized.");
}

void loop() {
    // 1. Đọc cảm biến ánh sáng và điều khiển đèn
    int ldr = led_readAnalog(); 
    led_update(ldr);
    Serial.printf("Mức độ ánh sáng: " "%d\n", ldr);

    // 2. Quay servo 
    servo_update(); 

    // 3. Đọc khoảng cách siêu âm
   float d = ultrasonic_read_cm();
    if (d > 0) {
        Serial.printf("Khoảng cách vật cản: %.2f cm\n", d);

        // Hysteresis để tránh nhảy trạng thái liên tục khi đo dao động quanh ngưỡng.
        if (!obstacleDetected && d < OBSTACLE_NEAR_CM) {
            obstacleDetected = true;
            indicators_alert_on();
            servo_resume();
            Serial.printf("Nguy hiểm!! Khoảng cách vật cản quá gần\n");
        } else if (obstacleDetected && d > OBSTACLE_CLEAR_CM) {
            obstacleDetected = false;
            indicators_alert_off();
            servo_pause();
            Serial.printf("Đã an toàn servo tạm dừng \n");
        }
    } 
    
    // 4. Gửi dữ liệu qua Bluetooth cho Android app
    // Gửi tất cả dữ liệu: khoảng cách, ánh sáng, trạng thái cảnh báo, góc servo, trạng thái servo
    if (d > 0) {
        bt_send_all_data(d, ldr, obstacleDetected, servo_get_angle(), servo_isPaused());
    }
    
    // Xử lý lệnh từ Android (để trống hiểu tại là sẵn sàng cho tương lai)
    bt_process();

    // 5. Cập nhật còi cảnh báo
    indicators_update();

    delay(40);
  
}
