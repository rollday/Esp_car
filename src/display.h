#pragma once

// 返回 true 表示初始化成功
bool initDisplay();

// 根据当前电机速度刷新显示
void updateDisplay(int currentSpeedA, int currentSpeedB);
