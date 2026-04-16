#include <Arduino.h>
#include "led_daynight.h"
#include "servo_radar.h"
#include "ultrasonic.h"
#include "bluetooth_control.h"
#include "indicators.h"

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

    // 2. Quét servo (radar)
    servo_update(); 

    // 3. Đọc khoảng cách siêu âm
   float d = ultrasonic_read_cm();
    if (d > 0) {
        Serial.printf("Khoảng cách vật cản: %.2f cm\n", d);

        // Nếu phát hiện vật cản dưới 25-> bật indicators (cảnh báo)
        if (d < 25.0f) {
            indicators_alert_on();
            servo_pause();
         Serial.printf("Nguy hiểm!! Khoảng cách vật cản quá gần" "\n");
        } else {
            indicators_alert_off();
            servo_resume();
        }
    } 
    // 4. Xử lý Bluetooth
    bt_process();

    // 5. Cập nhật đèn tín hiệu 
    indicators_update();

    delay(40);
  
}
