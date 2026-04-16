#include "bluetooth_control.h" 
#include "BluetoothSerial.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

BluetoothSerial BT;

// Kết nối L298N
#define IN1 27
#define IN2 26
#define IN3 25
#define IN4 33

// ===== Các hàm điều khiển động cơ =====
void stopMotor() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}

void forward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void backward() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void turnLeft() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void turnRight() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

// ===== Hàm khởi tạo Bluetooth =====
void bt_init() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Tắt Brownout

    BT.begin("ESP32-Car");
    Serial.println("Bluetooth Ready!");

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    stopMotor();
}

// ===== Hàm xử lý Bluetooth trong loop =====
void bt_process() {
    if (BT.available()) {
        char cmd = BT.read();
        Serial.println(cmd);
        
        switch (cmd) {
            case 'F': forward();  break;
            case 'B': backward(); break;
            case 'R': turnRight(); break;
            case 'L': turnLeft(); break;
            default:  stopMotor(); break;
        }
    }
}
