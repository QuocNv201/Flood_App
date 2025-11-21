#pragma once

// Khởi tạo/ cập nhật servo radar
void servo_init();
void servo_update(); // gọi định kỳ để quét

// Tạm dừng/quay lại quét servo (hold current angle)
void servo_pause();
void servo_resume();
bool servo_isPaused();

// Đặt servo về 1 góc cụ thể
void servo_setAngle(int angle);