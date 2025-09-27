#pragma once
#include <Arduino.h>

// 初始化超声波传感器（内部固定引脚）
bool initUltrasonic();

// 读取距离（cm），失败/超时返回 -1.0f
float ultrasonicReadCm();
