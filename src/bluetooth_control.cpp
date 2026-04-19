#include "bluetooth_control.h" 
#include "BluetoothSerial.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

BluetoothSerial BT;

// ===== Hàm khởi tạo Bluetooth =====
void bt_init() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Tắt Brownout

    BT.begin("ESP32-FloodApp");
    Serial.println("Bluetooth Ready!");
}

// ===== Gửi dữ liệu khoảng cách ultrasonic =====
void bt_send_distance(float distance_cm) {
    if (BT.connected()) {
        BT.print("DIST:");
        BT.println(distance_cm);
    }
}

// ===== Gửi dữ liệu ánh sáng =====
void bt_send_light_level(int ldr_value) {
    if (BT.connected()) {
        BT.print("LIGHT:");
        BT.println(ldr_value);
    }
}

// ===== Gửi trạng thái cảnh báo =====
void bt_send_alert_status(bool is_alert) {
    if (BT.connected()) {
        BT.print("ALERT:");
        BT.println(is_alert ? "ON" : "OFF");
    }
}

// ===== Gửi trạng thái servo =====
void bt_send_servo_status(int angle, bool is_paused) {
    if (BT.connected()) {
        BT.print("SERVO:");
        BT.print(angle);
        BT.print(",");
        BT.println(is_paused ? "PAUSE" : "SCAN");
    }
}

// ===== Gửi dữ liệu tổng hợp =====
void bt_send_all_data(float distance, int light, bool alert, int servo_angle, bool servo_paused) {
    if (BT.connected()) {
        BT.print("DATA:");
        BT.print(distance);
        BT.print(",");
        BT.print(light);
        BT.print(",");
        BT.print(alert ? 1 : 0);
        BT.print(",");
        BT.print(servo_angle);
        BT.print(",");
        BT.println(servo_paused ? 1 : 0);
    }
}

// ===== Xử lý lệnh từ Android (dự phòng cho tương lai) =====
void bt_process() {
    // Hiện tại chỉ gửi dữ liệu, không xử lý lệnh
    // Sau này có thể thêm xử lý lệnh từ Android app tại đây
}
