#pragma once
#include <Arduino.h>

typedef void (*ButtonCallback)(int buttonIndex);

// 初始化按键模块（包含去抖、上电稳定期等）
void buttonsInit();

// 轮询按键（非阻塞，需在 loop 中周期调用）
void buttonsPoll();

// 注册短按/长按回调
void buttonsSetShortPressHandler(ButtonCallback cb);
void buttonsSetLongPressHandler(ButtonCallback cb);
