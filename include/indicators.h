#pragma once

// Khởi tạo module indicators (LED xi-nhan / cảnh báo)
void indicators_init();

// Gọi định kỳ trong loop để cập nhật blink (non-blocking)
void indicators_update();

// Bật/tắt chế độ cảnh báo (khi có vật cản)
void indicators_alert_on();
void indicators_alert_off();

// Thiết lập chu kỳ nháy (ms)
void indicators_setInterval(unsigned long ms);