#include <Arduino.h>
#include <ESP32Servo.h>
#include "servo_radar.h"

#define SERVO_PIN 5

// ====== GIỮ TỐC ĐỘ CŨ ======
static int stepAngle = 10;
static const unsigned long interval = 20; // ms

// ====== CẤU HÌNH CHO TẤM CHẮN LŨ ======
// HOME: vị trí cũ (thu về)
// END:  vị trí đẩy tấm chắn lên (quay hết hành trình bên phải)
// Bạn chỉnh lại END_ANGLE cho đúng cơ cấu ray (thường 90 hoặc 120, không nhất thiết 180)
static const int HOME_ANGLE = 0;
static const int END_ANGLE  = 180;

// ====== STATE ======
static Servo myServo;
static int currentAngle = HOME_ANGLE;
static int targetAngle  = HOME_ANGLE;

static unsigned long lastMove = 0;
static bool paused = false; // dùng như "đang có nước/đóng chắn" (để main không đổi)

// Trạng thái chuyển động
enum MoveState { IDLE, MOVING_TO_END, MOVING_TO_HOME };
static MoveState state = IDLE;

static inline int clampAngle(int a) {
  if (a < 0) a = 0;
  if (a > 180) a = 180;
  return a;
}

// Đảo chiều quay thực tế của servo so với góc logic.
static inline int toServoWriteAngle(int logicalAngle) {
  return 180 - clampAngle(logicalAngle);
}

void servo_init() {
  myServo.attach(SERVO_PIN);

  currentAngle = clampAngle(HOME_ANGLE);
  targetAngle  = currentAngle;
  myServo.write(toServoWriteAngle(currentAngle));

  lastMove = millis();
  paused = false;
  state = IDLE;
}

// Chỉ chạy khi có lệnh (pause/resume) đặt state.
// Không còn quét liên tục nữa.
void servo_update() {
  unsigned long now = millis();
  if (now - lastMove < interval) return;
  lastMove = now;

  if (state == IDLE) return;

  // Di chuyển từng bước về targetAngle
  if (currentAngle < targetAngle) {
    currentAngle += stepAngle;
    if (currentAngle >= targetAngle) {
      currentAngle = targetAngle;
      state = IDLE; // tới đích -> dừng
    }
  } else if (currentAngle > targetAngle) {
    currentAngle -= stepAngle;
    if (currentAngle <= targetAngle) {
      currentAngle = targetAngle;
      state = IDLE; // tới đích -> dừng
    }
  } else {
    state = IDLE;
  }

  myServo.write(toServoWriteAngle(currentAngle));
}

// Vẫn giữ hàm này (nếu bạn cần set góc thủ công qua Bluetooth)
void servo_setAngle(int angle) {
  angle = clampAngle(angle);
  targetAngle = angle;

  if (targetAngle == currentAngle) {
    state = IDLE;
  } else {
    // tự chọn hướng để chạy tới góc đó
    state = (targetAngle > currentAngle) ? MOVING_TO_END : MOVING_TO_HOME;
  }
}

// Khi "khoảng cách gần" (có nước): bắt đầu quay để đẩy tấm chắn lên, tới END rồi dừng
void servo_resume() {
  // Nếu đã đang ở END hoặc đang chạy tới END thì bỏ qua
  paused = true;               // để servo_isPaused() phản ánh trạng thái "đang đóng"
  targetAngle = END_ANGLE;
  if (currentAngle != targetAngle) state = MOVING_TO_END;
}

// Khi "khoảng cách xa" (hết nước): quay ngược về HOME (vị trí cũ) rồi dừng
void servo_pause() {
  paused = false;              // trạng thái "mở"
  targetAngle = HOME_ANGLE;
  if (currentAngle != targetAngle) state = MOVING_TO_HOME;

  lastMove = millis();
}

bool servo_isPaused() {
  return paused;
}

// Lấy góc servo hiện tại
int servo_get_angle() {
  return currentAngle;
}