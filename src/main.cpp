#include <Arduino.h>
#include "led_daynight.h"
#include "servo_radar.h"
#include "ultrasonic.h"
#include "bluetooth_control.h"
#include "indicators.h"
#include "firebase_client.h"

static const float OBSTACLE_NEAR_CM  = 25.0f;
static const float OBSTACLE_CLEAR_CM = 30.0f;
static bool obstacleDetected = false;

// Last valid distance kept so Firebase still gets refreshed when sensor
// returns -1 (out of range) for a brief moment.
static float lastValidDist = 0.0f;

void setup() {
    Serial.begin(115200);

    // Khởi tạo tất cả module
    led_init();
    servo_init();
    ultrasonic_init();
    bt_init();
    indicators_init();

    // Kết nối WiFi và Firebase Realtime Database
    firebase_init();

    Serial.println("System initialized.");
}

void loop() {
    // 1. Đọc cảm biến ánh sáng và điều khiển đèn
    int ldr = led_readAnalog();
    led_update(ldr);
    Serial.printf("Mức độ ánh sáng: %d\n", ldr);

    // 2. Quay servo
    servo_update();

    // 3. Đọc khoảng cách siêu âm
    float d = ultrasonic_read_cm();
    if (d > 0) {
        lastValidDist = d;
        Serial.printf("Khoảng cách vật cản: %.2f cm\n", d);

        // Hysteresis để tránh nhảy trạng thái liên tục khi đo dao động quanh ngưỡng.
        if (!obstacleDetected && d < OBSTACLE_NEAR_CM) {
            obstacleDetected = true;
            indicators_alert_on();
            servo_resume();
            Serial.println("Nguy hiểm!! Khoảng cách vật cản quá gần");
        } else if (obstacleDetected && d > OBSTACLE_CLEAR_CM) {
            obstacleDetected = false;
            indicators_alert_off();
            servo_pause();
            Serial.println("Đã an toàn servo tạm dừng");
        }
    }

    // 4. Gửi dữ liệu qua Bluetooth cho Android app
    if (lastValidDist > 0) {
        bt_send_all_data(lastValidDist, ldr, obstacleDetected,
                         servo_get_angle(), servo_isPaused());
    }

    // Xử lý lệnh từ Bluetooth (dự phòng)
    bt_process();

    // 5. Cập nhật còi cảnh báo
    indicators_update();

    // 6. Firebase: đẩy dữ liệu cảm biến (rate-limited tự động trong hàm này)
    if (lastValidDist > 0) {
        firebase_update_sensors(lastValidDist, ldr, obstacleDetected,
                                servo_get_angle(), servo_isPaused());
    }

    // 7. Firebase: xử lý lệnh điều khiển từ Android app
    // Lệnh đặt góc servo
    int servoCmd = firebase_get_servo_target();
    if (servoCmd >= 0) {
        Serial.printf("[Firebase] Servo command: %d°\n", servoCmd);
        servo_setAngle(servoCmd);
        firebase_clear_servo_command();
    }

    // Lệnh reset cảnh báo
    if (firebase_get_reset_command()) {
        obstacleDetected = false;
        indicators_alert_off();
        firebase_clear_reset_command();
        Serial.println("[Firebase] Alert reset by app");
    }

    delay(40);
}
