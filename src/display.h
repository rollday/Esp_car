#pragma once

// 初始化OLED显示屏
bool initDisplay();

// 清除显示
void clearDisplay();

// 更新显示 - 显示距离和两个电机速度
void updateDisplay(float distance, bool motorEnabled, bool motorForward, float planarVelocity, float yaw);

// 如果需要保留姿态显示的功能，可以添加这个函数
void updateAttitudeDisplay(float roll, float pitch, float yaw, float velX, float velY);
