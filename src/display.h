#pragma once

// 返回 true 表示初始化成功
bool initDisplay();

// 根据当前电机速度刷新显示
void updateDisplay(int currentSpeedA, int currentSpeedB);

// 根据当前距离刷新显示
void updateDisplay(float distance);

// 新增：同时显示距离与两路速度（V: ...... ......）
void updateDisplay(float distance, int currentSpeedA, int currentSpeedB);

// 清屏函数声明
void clearDisplay();
